#ifndef _THREAD_H
#define _THREAD_H

#include <types.h>
//#include <vmem.h>
#include <list.h>
#include <asm.h>

enum
{
    THREAD_PRIO_MAX = 20,
    THREAD_PRIO_MIN = 1,
    THREAD_PRIO_DEF = 10,

    THREAD_STATE_RUNNING = 0,
    THREAD_STATE_SLEEP   = 1,
};

typedef struct tss_s
{
    uint32_t    back, _pad0;
    uint32_t    esp0;
    uint16_t    ss0,  _pad1;
    uint32_t    esp1;
    uint16_t    ss1,  _pad2;
    uint32_t    esp2;
    uint16_t    ss2,  _pad3;
    uint32_t    cr3;
    uint32_t    eip;
    uint32_t    eflags;
    uint32_t    eax;
    uint32_t    ecx;
    uint32_t    edx;
    uint32_t    ebx;
    uint32_t    esp;
    uint32_t    ebp;
    uint32_t    esi;
    uint32_t    edi;
    uint16_t    es,   _pad4;
    uint16_t    cs,   _pad5;
    uint16_t    ss,   _pad6;
    uint16_t    ds,   _pad7;
    uint16_t    fs,   _pad8;
    uint16_t    gs,   _pad9;
    uint16_t    ldt,  _padA;
    uint16_t    trap, bitmap;
} tss_t __packed;

typedef struct fp_state_s
{
    uint32_t    cwd;
    uint32_t    swd;
    uint32_t    twd;
    uint32_t    fip;
    uint32_t    fcs;
    uint32_t    foo;
    uint32_t    fos;
    uint8_t     st_space[80];
} fp_state_t;

typedef struct thread_s
{
    // Layout important because it's used in asm
    uint32_t esp;  // + 0
    uint32_t esp0; // + 4
    uint32_t cr3;  // + 8
    // END: Layout important

    // Parents, children of this thread
    struct thread_s* parent;
    list_t           children_list;

    // Virtual memory data
    //vmem_data_t vmem;

    // Thread state
    int state;

    // Scheduling data
    int priority;
    int timeslice;

    // Runtime
    int usertime, systime;
    int curr_usertime, curr_systime;

    // Lists which contain the thread
    list_t thread_entry;
    list_t prio_entry;
    list_t child_entry;
    list_t hash_entry;

    char name[256];
    int  pid;

    // Floating point state
    bool       fp_inited;
    fp_state_t fp_state;
} thread_t __packed;

typedef struct runqueue_s
{
    list_t prio_list[THREAD_PRIO_MAX];
    int    num_running;
} runqueue_t;

void thread_init() __init;
void thread_create(func_t, const char* name);
void thread_sleep(int);
void thread_setpriority(int);
thread_t* thread_by_pid(int);
void thread_tick();
void thread_dump();

extern thread_t* curr_thread;

static inline void critical_enter() { irqs_disable(); }
static inline void critical_leave() { irqs_enable(); }
/*
static inline void critical_enter()
{
    if (unlikely(!curr_thread))
	return;
    if (curr_thread->critical_level == 0)
    {
        curr_thread->critical_enabled = irqs_enabled();
	irqs_disable();
    }
    ++curr_thread->critical_level;
}

static inline void critical_leave()
{
    if (unlikely(!curr_thread))
	return;
    if (--curr_thread->critical_level == 0 && curr_thread->critical_enabled)
        irqs_enable();
}
*/
#endif
