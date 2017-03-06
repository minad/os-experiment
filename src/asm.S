#include <segment.h>

/**********************************************************
 * Initial GDTR
 * (GDTR will be reloaded later when paging is on)
 */
 
.data

.extern gdt

initial_gdtr:
        .word   GDT_ENTRIES * 8 - 1
        .long   VIRT_TO_PHYS(gdt)

/**********************************************************
 * Kernel entry
 */

.text

.extern kernel_stack_end, kernel_main

.global kernel_entry
kernel_entry:
        // Load new kernel gdt
        lgdt    VIRT_TO_PHYS(initial_gdtr)
               
        // Set new segment selectors
        ljmp    $KERNEL_CS, $1f
1:      movw    $KERNEL_DS, %cx
        movw    %cx, %ds
        movw    %cx, %es
        movw    %cx, %ss
        xorw    %cx, %cx // fs and gs are unused
        movw    %cx, %fs
        movw    %cx, %gs
        
        // Set-up kernel stack
        movl    $kernel_stack_end, %ecx 
        movl    %ecx, %esp
       
        // Frame pointer for stack trace 
        xorl    %ebp, %ebp
       
        // Push multiboot infos and call kernel
        pushl   %ebx
        pushl   %eax
        call    kernel_main

/**********************************************************
 * Threading stuff
 */

#define THREAD_ESP  0
#define THREAD_ESP0 4
#define THREAD_CR3  8
#define TSS_ESP0    8

.extern curr_thread
.extern system_tss
.extern last_fp_thread

/**********************************************************
 * Thread switch
 * 
 * void thread_switch(thread_t* next_thread);
 *
 * 1. Everything is stored on the stack of curr_thread
 * 2. Switch to stack of next_thread
 * 3. Restore registers from stack
 */

.global thread_switch
thread_switch:
        // eax, ecx and edx are saved by GCC
        popl    %eax // return address
        popl    %ecx // next_thread

        // Save all registers
        pushfl
        pushl   %cs
        pushl   %eax      // eip
        subl    $12, %esp // error_code, int_nr, eax
        pushl   %ebx
        subl    $8, %esp  // ecx, edx
        pushl   %ebp
        pushl   %esi
        pushl   %edi
        pushl   %ds
        pushl   %es
        pushl   %fs
        pushl   %gs

        // Save current stack pointer
        movl    curr_thread, %eax
        movl    %esp, THREAD_ESP(%eax)

        // Argument next_thread
        movl    %ecx, curr_thread
        
        // Restore stack of next thread
        jmp     thread_restore
    
/**********************************************************
 * Interrupt stubs with thread switch
 *
 * 1. Everything is stored on the stack of curr_thread
 * 2. Kernel segments are loaded
 * 3. Kernel handler is called
 * 4. Switch to stack of curr_thread (can have been changed)
 * 5. Restore registers from stack
 */

interrupt_stub:
        // Save all registers
        xchgl   %eax, (%esp) // Handler address
        pushl   %ebx
        pushl   %ecx
        pushl   %edx
        pushl   %ebp
        pushl   %esi
        pushl   %edi
        pushl   %ds
        pushl   %es
        pushl   %fs
        pushl   %gs

        // Save current stack pointer
        movl    curr_thread, %ebx
        movl    %esp, THREAD_ESP(%ebx)

        // Kernel segments (ds and es are used)
        movw    $KERNEL_DS, %bx
        movw    %bx, %ds
        movw    %bx, %es

        // Call handler
        call    *%eax
       
.global thread_restore
thread_restore:
        movl    curr_thread, %eax
        
	// Save kernel stack of new thread in system tss
        movl    THREAD_ESP0(%eax), %ebx
        movl    %ebx, system_tss+TSS_ESP0
        
        // Load PDBR of new thread if it's different
        // This check is done because switching the pdbr is slow.
        movl    THREAD_CR3(%eax), %ebx
        movl    %cr3, %ecx
        cmpl    %ebx, %ecx
        je      1f
        movl    %ebx, %cr3
1:

	// Set emulate bit (bit 2)
	cmpl    %eax, last_fp_thread
	je	2f
	movl    %cr0, %ebx
	orl     $4, %ebx
	movl    %ebx, %cr0
2:

	// Restore registers
        movl    THREAD_ESP(%eax), %esp
	popl    %gs
        popl    %fs
        popl    %es
        popl    %ds
        popl    %edi
        popl    %esi
        popl    %ebp
        popl    %edx
        popl    %ecx
        popl    %ebx
        popl    %eax
        
        // Pop error code and interrupt nr
        addl    $8, %esp
        iret

/*
 * Stub macros
 */

// Interrupt without error code
#define INT(nr, name, handler) \
.global name;                  \
.extern handler;               \
name:                          \
        pushl   $0;            \
        pushl   $nr;           \
        pushl   $handler;      \
        jmp     interrupt_stub

// Interrupt with error code
#define INT_EC(nr, name, handler) \
.global name;                     \
.extern handler;                  \
name:                             \
        pushl   $nr;              \
        pushl   $handler;         \
        jmp     interrupt_stub
 
#define EX(nr, name) \
    INT(nr, ex_##name, do_exception)

#define EX_EC(nr, name) \
    INT_EC(nr, ex_##name, do_exception)

#define IRQ(nr, name) \
    INT(nr, irq_##name, do_irq_##name)

/* 
 * Exception stubs
 */

EX    ( 0, divide_error)
EX    ( 1, debug_exception)
EX    ( 2, nmi)
EX    ( 3, breakpoint)
EX    ( 4, overflow)
EX    ( 5, bounds_check)
EX    ( 6, invalid_opcode)
INT   ( 7, ex_coprocessor_not_available, switch_fp_state)
EX_EC ( 8, double_fault)
EX    ( 9, coprocessor_segment_overrun)
EX_EC (10, invalid_tss)
EX_EC (11, segment_not_present)
EX_EC (12, stack_exception)
EX_EC (13, general_protection)
EX_EC (14, page_fault)
/*INT_EC(14, ex_page_fault, do_page_fault)*/
EX    (16, coprocessor_error)
EX    (17, alignment_check)
EX    (18, machine_check)
EX    (19, simd_float)

/*
 * IRQ stubs
 */

IRQ(33, timer)
IRQ(34, keyboard)

/*
 * System calls
 */

INT(128, int_system_call, do_system_call)
