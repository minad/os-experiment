#include <thread.h>
#include <desc.h>
#include <segment.h>
#include <regs.h>
#include <asm.h>
#include <malloc.h>
#include <string.h>
#include <section.h>
#include <console.h>
#include <gdt.h>
#include <debug.h>
#include <math.h>
#include <timer.h>
#include <stdio.h>
#include <ansicode.h>

// All threads
static list_t thread_list = LIST_INIT(thread_list);
static int num_threads = 0;
static thread_t idle_thread;

// Pid management
enum { PID_HASH_SIZE = 1024 };
static int next_pid = 0;
static list_t pid_hash[PID_HASH_SIZE];

// The runqueues
static runqueue_t runqueue[2],
                  *active = runqueue,
                  *expired = runqueue + 1;

// Current running thread
thread_t *curr_thread = NULL,
         *last_fp_thread = NULL;

// System TSS (for task switches over privilege boundaries)
tss_t system_tss;

void thread_restore();
void thread_switch(thread_t*);
static thread_t* thread_schedule();
static void wakeup_thread(void*);

static inline list_t* get_pid_list(int pid)
{
    return &pid_hash[pid % PID_HASH_SIZE];
}

static inline void enqueue_thread(runqueue_t* q, thread_t* t, int priority)
{
    list_add(&q->prio_list[priority - 1], &t->prio_entry);
    ++q->num_running;
}

static inline void dequeue_thread(runqueue_t* q, thread_t* t)
{
    list_delete(&t->prio_entry);
    --q->num_running;
}

void __init thread_init()
{
    int i;

    // Initialize runqueues
    active->num_running = expired->num_running = 0;
    for (i = 0; i < THREAD_PRIO_MAX; ++i)
    {
	list_init(&active->prio_list[i]);
	list_init(&expired->prio_list[i]);
    }

    for (i = 0; i < PID_HASH_SIZE; ++i)
        list_init(&pid_hash[i]);

    // Create idle thread (from current thread)
    idle_thread.esp0      = 42; // No kernel stack used because idle runs with privilege 0
    idle_thread.cr3       = get_reg(cr3);
    idle_thread.state     = THREAD_STATE_RUNNING;
    idle_thread.pid       = next_pid++;
    idle_thread.priority  = 0;
    //list_init(&idle_thread.vmem.region_list);
    strcpy(idle_thread.name, "idle");

    list_init(&idle_thread.children_list);

    list_add(get_pid_list(idle_thread.pid), &idle_thread.hash_entry);
    list_add(&thread_list, &idle_thread.thread_entry);
    ++num_threads;

    curr_thread = &idle_thread;

    // System TSS
    gdt_set(5, desc_seg((uint32_t)&system_tss, sizeof (system_tss) - 1, DESC_TYPE_TSS | DESC_PRESENT));
    set_tr(KERNEL_TSS);
}

void thread_create(func_t addr, const char* name)
{
    thread_t* t;
    uint32_t* esp = (uint32_t*)((char*)malloc(4096) + 4096);

    *(--esp) = EFLAGS_IF;
    *(--esp) = KERNEL_CS; // cs
    *(--esp) = addr;      // eip
    esp -= 13;            // error_code, int_nr, eax, ebx, ecx, edx, ebp, esi, edi, ds, es, fs, gs

    t = calloc(sizeof (thread_t));
    t->esp  = esp;
    t->esp0 = 42; // No kernel stack used. Thread running with level 0.
                  // So this value is never used.
                  // This value is written to system tss and and
                  // then used during a privilege change (happens never).
    t->cr3       = curr_thread->cr3;
    t->priority  = curr_thread->priority == 0 ? THREAD_PRIO_DEF : curr_thread->priority;
    t->timeslice = t->priority;
    t->state     = THREAD_STATE_RUNNING;
    t->pid       = next_pid++;
    strncpy(t->name, name, sizeof (t->name));

    list_init(&t->children_list);
    t->parent = curr_thread;

    critical_enter();

    // Add new thread
    list_add(&thread_list, &t->thread_entry);
    enqueue_thread(active, t, t->priority);
    ++num_threads;

    // Add child to parent
    list_add(&curr_thread->children_list, &t->child_entry);

    // PID hash
    list_add(get_pid_list(t->pid), &t->hash_entry);

    critical_leave();
}

void thread_sleep(int ticks)
{
    critical_enter();

    timer_add(wakeup_thread, curr_thread, ticks);

    dequeue_thread(active, curr_thread);
    curr_thread->state = THREAD_STATE_SLEEP;

    thread_switch(thread_schedule());

    critical_leave();
}

void thread_sleep2(int ticks)
{
    critical_enter();
    timer_add(wakeup_thread, curr_thread, ticks);

    dequeue_thread(active, curr_thread);
    curr_thread->state = THREAD_STATE_SLEEP;

    curr_thread = thread_schedule();
    critical_leave();
}

// TODO: Stack freen
void thread_exit()
{
    // IRQs will be restored during task switch
    critical_enter();

    dequeue_thread(active, curr_thread);
    list_delete(&curr_thread->thread_entry);

    // Disconnect from parent
    list_delete(&curr_thread->child_entry);

    // Remove from PID hash
    list_delete(&curr_thread->hash_entry);

    free(curr_thread);

    if (last_fp_thread == curr_thread)
	last_fp_thread = NULL;

    curr_thread = thread_schedule();
    thread_restore();
}

void thread_setpriority(int priority)
{
    ASSERT(priority <= THREAD_PRIO_MAX);
    ASSERT(priority >= THREAD_PRIO_MIN);

    critical_enter();
    curr_thread->timeslice = max(THREAD_PRIO_MIN, priority - (curr_thread->priority - curr_thread->timeslice));
    curr_thread->priority = priority;

    // Enqueue with rest of timeslice
    dequeue_thread(active, curr_thread);
    enqueue_thread(active, curr_thread, curr_thread->timeslice);
    critical_leave();
}

thread_t* thread_by_pid(int pid)
{
    list_t* list = get_pid_list(pid), *p;
    for (p = list->next; p != list; p = p->next)
    {
	thread_t* t = LIST_OBJECT(p, thread_t, hash_entry);
	if (t->pid == pid)
	    return t;
    }
    return NULL;
}

// Timer handler
void thread_tick()
{
    if (unlikely(get_reg(cs) == KERNEL_CS))
    {
	++curr_thread->systime;
        ++curr_thread->curr_systime;
    }
    else
    {
	++curr_thread->usertime;
	++curr_thread->curr_usertime;
    }

    if (unlikely(curr_thread == &idle_thread))
        curr_thread = thread_schedule();
    else
    {
	--curr_thread->timeslice;
	if (unlikely(curr_thread->timeslice <= 0))
	{
	    // New timeslice for thread
	    curr_thread->timeslice = curr_thread->priority;

	    // Enqueue in other queue
	    if (unlikely(curr_thread != &idle_thread))
	    {
		dequeue_thread(active, curr_thread);
		enqueue_thread(expired, curr_thread, curr_thread->priority);
	    }
	    curr_thread = thread_schedule();
	}
    }
}

// Switch floating point state
void switch_fp_state(const regs_t regs)
{
    // Clear emulate bit (bit 2)
    set_reg(cr0, get_reg(cr0) & ~4);

    if (last_fp_thread)
        fp_save(last_fp_thread->fp_state);

    if (curr_thread->fp_inited)
        fp_restore(curr_thread->fp_state);
    else
    {
        fp_init();
        curr_thread->fp_inited = true;
    }

    last_fp_thread = curr_thread;
}

static void wakeup_thread(void* arg)
{
    thread_t* t = (thread_t*)arg;

    // Enqueue with rest of timeslice
    enqueue_thread(active, t, t->timeslice);
    t->state = THREAD_STATE_RUNNING;
}

// Simple round-robin scheduler
static thread_t* thread_schedule()
{
    if (unlikely(active->num_running == 0))
    {
        if (unlikely(expired->num_running == 0))
            return &idle_thread;

        // Swap runqueues
        runqueue_t* queue = active;
        active = expired;
        expired = queue;
    }

    for (int i = THREAD_PRIO_MAX - 1; i >= 0; --i)
    {
        if (!list_empty(&active->prio_list[i]))
            return LIST_OBJECT(active->prio_list[i].next, thread_t, prio_entry);
    }

    ASSERT(!"Empty runqueue???");
}

void thread_dump()
{
    list_t* p;
    int idle_time, total_time = 0;

    critical_enter();

    idle_time = idle_thread.curr_usertime + idle_thread.curr_systime;

    for (p = thread_list.next; p != &thread_list; p = p->next)
    {
	thread_t* t = LIST_OBJECT(p, thread_t, thread_entry);
        total_time += t->curr_usertime + t->curr_systime;
        t->curr_usertime = t->curr_systime = 0;
    }

    con_printf(1, CLRSCR "%d Threads, %d%% Usage\n  Name   Pid   Usertime   Systime   Priority   State\n",
	       num_threads, 100 - 100 * idle_time / total_time);
    for (p = thread_list.next; p != &thread_list; p = p->next)
    {
	thread_t* t = LIST_OBJECT(p, thread_t, thread_entry);
	con_printf(1, "%6s %5d %10d %9d %10d %7d\n", t->name, t->pid, t->usertime, t->systime, t->priority, t->state);
    }

    critical_leave();
}
