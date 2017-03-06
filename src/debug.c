#include <debug.h>
#include <math.h>
#include <stdio.h>
#include <ctype.h>
#include <asm.h>
#include <ansicode.h>

void __printf_noreturn(1, 2) panic(const char *format, ...)
{
    char buffer[1024];
    va_list argptr;

    irqs_disable();

    va_start(argptr, format);
    vsnprintf(buffer, sizeof (buffer), format, argptr);
    va_end(argptr);

    buffer[sizeof (buffer) - 1] = 0;
    printf(FG_RED "Kernel panic:" NOCOLOR " %s\n", buffer);
    stack_trace(10);
    
    for (;;);
}

void dump_regs(const regs_t* r)
{
    static const char* int_name[] =
    {
        "Divide Error",
        "Debug Exception",
        "Non-maskable Interrupt",
        "Breakpoint",
        "Overflow",
        "Bounds Check",
        "Invalid Opcode",
        "Coprocessor not available",
        "Double Fault",
        "Coprocessor Segment Overrun",
        "Invalid TSS",
        "Segment not present",
        "Stack Exception",
        "General Protection",
        "Page Fault",
        "Unknown", // Reserved
        "Coprocessor Error",
        "Alignment Check",
        "Machine Check",
        "SIMD Floating Point Exception",
    }; 

    printf(FG_LTWHITE "Interrupt %d (%s):" NOCOLOR " Error code = %d\n"
           " eax=%08X ebx=%08X ecx=%08X edx=%08X\n"
           " ebp=%08X esi=%08X edi=%08X\n"
           " eip=%08X cs=%04hX ds=%04hX es=%04hX fs=%04hX gs=%04hX\n"
           " eflags=%08X user_ss=%04hX user_esp=%08X\n",
           r->int_nr, (r->int_nr > 19 ? "Unknown" : int_name[r->int_nr]), r->error_code,
           r->eax, r->ebx, r->ecx, r->edx,
           r->ebp, r->esi, r->edi,
           r->eip, r->cs, r->ds, r->es, r->fs, r->gs,
           r->eflags, r->user_ss, r->user_esp);
}

void stack_trace(int num_frames)
{
    // Stack frame pointer is stored in ebp
    uint32_t *fp = (uint32_t*)get_reg(ebp);
    
    puts(FG_LTWHITE "Stack Trace:" NOCOLOR);
    while (fp && num_frames-- > 0)
    {
	// The return address is pushed before the frame pointer
	printf("\t0x%08X\n", *(fp + 1));

	// Go to Previous frame pointer.
	// Address is on the stack
        fp = (uint32_t*)(*fp);
    }

    if (fp)
	puts("\t...");
}

#ifndef NDEBUG

void _dump(const void* data, size_t size)
{
    char *p, *q, *end;
    p = (char*)data;
    end = p + size;
    
    while (p < end)
    {
        for(q = p; q < p + 16; ++q)
            printf(FG_LTWHITE "%02X " NOCOLOR, *q & 0xFF);
        putchar('\t');
        
        for(q = p; q < p + 16; ++q)
            putchar(isprint(*q) ? *q : '.');
        putchar('\n');
 
        p += 16;
    }
}

void __noreturn _assert(const char *expr, const char *file, int line,
                         const char *func, const char *fmt, ...) 
{
    printf("Assertion failed: ");
    
    if (fmt)
    {
        va_list ap;
        char buffer[1024];
        va_start(ap, fmt);
        snprintf(buffer, sizeof (buffer), fmt, ap);
        va_end(ap);
        printf("\n\t%s\n\t", buffer);
    }
    
    printf("%s, File %s, Line %d, Function %s\n",
           expr, file, line, func);

    panic("Assertion failed");
}

#endif

