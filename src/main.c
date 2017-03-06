#include <vmem.h>
#include <pmem.h>
#include <pic.h>
#include <pit.h>
#include <stdarg.h>
#include <string.h>
#include <idt.h>
#include <irq.h>
#include <stdio.h>
#include <console.h>
#include <idt.h>
#include <multiboot.h>
#include <cpu.h>
#include <asm.h>
#include <desc.h>
#include <ansicode.h>
#include <section.h>
#include <thread.h>
#include <time.h>
#include <keyboard.h>
#include <timer.h>
#include <syscall.h>

static void init_ctors();
static void init() __noreturn;
static void idle() __noreturn;

char kernel_stack[4096];
char* kernel_stack_end = kernel_stack + sizeof(kernel_stack);

/*
 * Machine state after boot with multiboot:
 * 
 * eax:    Contains the magic value 0x2BADB002
 * ebx:    Contains the 32-bit physical address of the Multiboot information structure 
 * segs:   32-bit segment with an offset of 0 and a limit of 0xFFFFFFFF 
 * A20:    enabled 
 * cr0:    PG cleared, PE set
 * eflags: VM cleared, IF cleared 
 */

void __noreturn kernel_main(uint32_t eax, uint32_t ebx)
{
    char* argv[64];
    int   argc = 64;
    
    puts(CLRSCR "Calling static constructors");
    init_ctors();
    
    puts("Multiboot information:");
    multiboot_init(eax, ebx, &argc, argv);
    
    cpu_detect();
    cpu_dump_info(cpu_get_info());
   
    puts("Initializing PMEM...");
    pmem_init();
    pmem_dump_stats(pmem_get_stats());
    
    puts("Initializing VMEM...");
    vmem_init();
    con_init();
    
    puts("Setting up IDT...");
    idt_init();
   
    puts("Initializing PIC...");
    pic_init();
  
    puts("Initializing keyboard...");
    kbd_init();
   
    puts("Initializing timer...");
    timer_init();

    puts("Initializing threading...");
    thread_init();
   
    irqs_enable();
 
    thread_create(init, "init");
   
    idle();
}

static void init_ctors()
{
    for (func_t* ctor = (func_t*)CTORS_START; *ctor; ++ctor)
    {
	printf("0x%X --> 0x%X", ctor, *ctor);
	(*ctor)();
    }
}

static void __noreturn child()
{
    thread_setpriority(2);
    int y = 0; 
    for (;;)
        con_printf(0, "%s: %d\n", curr_thread->name, y++);
}

static void __noreturn thread_view()
{
    thread_setpriority(20);
    for (;;)
    {
        thread_dump();
	sleep(100);
	//thread_sleep(100);
    }
}

static void __noreturn mem_view()
{
    thread_setpriority(20);
    for (;;)
    {
	pmem_dump_stats(pmem_get_stats());
        thread_sleep(100);
    }
}

static void __noreturn mem_test()
{
    for (;;)
    {
       void* p[256];
       int i;
       for (i = 0; i < 256; ++i)
	    p[i] = malloc(500 * i);
       thread_sleep(1000);
       for (i = 0; i < 256; ++i)
	    free(p[i]);
    }
}

static void __noreturn thread_a() 
{
    double x = 0;
    thread_setpriority(5);
    for (;;)
        con_printf(0, "a: %d\n", (int)((x += 0.1) * 1000));
}

static void __noreturn thread_b() 
{
    double x = 0;
    for (;;)
        con_printf(0, "b: %d\n", (int)((x += 0.1) * 1000));
}

static void __noreturn init()
{
    printf("Freeing init memory: %dK\n", INIT_SIZE >> 10);
    vmem_free(INIT_START, INIT_START + INIT_SIZE);
    pmem_dump_stats(pmem_get_stats());
  	 
    thread_create(thread_view, "thread_view");
    //thread_create(mem_view, "mem_view");
    //thread_create(mem_test, "mem_test");
    
    //for (i = 0; i < 10; ++i)
    //{
    //    char name[32];
    //    snprintf(name, sizeof (name), "chld%d", i);
    //    thread_create(child, name);
    //}
    
    //thread_setpriority(5);
    //x = 0;
    //for (;;)
    //     con_printf(0, "init: %d\n", x++);
   
    thread_create(thread_a, "a");
    thread_create(thread_b, "b");
    
    thread_exit();
}

static void __noreturn idle()
{
    for (;;)
        hlt();
}

void show_time()
{
    struct tm tm;
    gettime(&tm);

    printf("Time:\n"
           "\tsec:   %d\n"
	   "\tmin:   %d\n"
	   "\thour:  %d\n"
	   "\tmday:  %d\n"
	   "\tmon:   %d\n"
	   "\tyear:  %d\n"
	   "\twday:  %d\n"
	   "\tyday:  %d\n"
	   "\ttime:  %d\n",
	   tm.tm_sec, tm.tm_min, tm.tm_hour, tm.tm_mday, tm.tm_mon,
	   tm.tm_year, tm.tm_wday, tm.tm_yday, time(NULL));
}
