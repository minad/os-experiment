#include <timer.h>
#include <pit.h>
#include <pic.h>
#include <idt.h>
#include <pool.h>
#include <regs.h>
#include <thread.h>
#include <irq.h>
#include <desc.h>
#include <thread.h>

static pool_t timer_pool;
static ullong ticks = 0;
static list_t timer_list = LIST_INIT(timer_list);

void irq_timer();
static void trigger_expired();

void __init timer_init()
{
    pool_init(&timer_pool, 20, sizeof (timer_t));

    idt_set(PIC_INTBASE + IRQ_TIMER, irq_timer, DESC_TYPE_INT | DESC_PRESENT);
    pit_set(PIT_CHANNEL_TIMER, PIT_MODE_RATEGEN, 1000);
    pic_irq_enable(IRQ_TIMER);
}

void timer_add(callback_t call, void* arg, int interval)
{
    ullong expires = ticks + interval;
    list_t* p;
    timer_t* t;
    bool irq_status;

    irqs_save(&irq_status);
    
    for (p = timer_list.next; p != &timer_list; p = p->next)
    {
        t = LIST_OBJECT(p, timer_t, list_entry);
	if (t->expires > expires)
	    break;
    }
	
    t = (timer_t*)pool_get(&timer_pool);
    t->expires = ticks + interval;
    t->call    = call;
    t->arg     = arg;
    list_add(p, &t->list_entry);

    irqs_restore(irq_status);
}

// Timer handler
void do_irq_timer(const regs_t regs)
{
    pic_irq_end(IRQ_TIMER);
    
    ++ticks;
    trigger_expired();
    thread_tick();
}

// Trigger expired timers
static void trigger_expired()
{
    list_t* p = timer_list.next;
    while (p != &timer_list)
    {
	timer_t* t = LIST_OBJECT(p, timer_t, list_entry);
        if (t->expires >= ticks)
	    break;
	t->call(t->arg);

	p = p->next;
	
	list_delete(&t->list_entry);
	pool_release(&timer_pool, t);
    }
}
