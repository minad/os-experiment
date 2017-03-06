#include <desc.h>
#include <idt.h>
#include <types.h>
#include <stdio.h>
#include <asm.h>
#include <segment.h>
#include <regs.h>
#include <debug.h>
#include <thread.h>

// IDT table
static desc_t idt[64];

// Assembler routines
void ex_divide_error();
void ex_debug_exception();
void ex_nmi();
void ex_breakpoint();
void ex_overflow();
void ex_bounds_check();
void ex_invalid_opcode();
void ex_coprocessor_not_available();
void ex_double_fault();
void ex_coprocessor_segment_overrun();
void ex_invalid_tss();
void ex_segment_not_present();
void ex_stack_exception();
void ex_general_protection();
void ex_page_fault();
void ex_coprocessor_error();
void ex_alignment_check();
void ex_machine_check();
void ex_simd_float();

void int_system_call();

static inline void set_trap(int id, func_t handler)
{
    idt_set(id, handler, DESC_TYPE_TRAP | DESC_PRESENT);
}

static inline void set_user(int id, func_t handler)
{
    idt_set(id, handler, DESC_TYPE_TRAP | DESC_DPL3 | DESC_PRESENT);
}

void idt_init()
{
    /*
     * IDT must be build at runtime because the assembler and linker are
     * to stupid to handle the broken addresses in the descriptors :-(
     */
    
    set_trap(0,  ex_divide_error);
    set_trap(1,  ex_debug_exception);
    set_trap(2,  ex_nmi);
    set_user(3,  ex_breakpoint);
    set_user(4,  ex_overflow);
    set_user(5,  ex_bounds_check);
    set_trap(6,  ex_invalid_opcode);
    set_trap(7,  ex_coprocessor_not_available);
    set_trap(8,  ex_double_fault);
    set_trap(9,  ex_coprocessor_segment_overrun);
    set_trap(10, ex_invalid_tss);
    set_trap(11, ex_segment_not_present);
    set_trap(12, ex_stack_exception);
    set_trap(13, ex_general_protection);
    set_trap(14, ex_page_fault);
    set_trap(16, ex_coprocessor_error);
    set_trap(17, ex_alignment_check);
    set_trap(18, ex_machine_check);
    set_trap(19, ex_simd_float);
    
    idt_set(128, int_system_call, DESC_TYPE_INT | DESC_DPL3 | DESC_PRESENT);

    /* Load idt from linear address (linear = virtual, because segmentation
     * is effectively deactivated.
     */
    set_idtr((uint32_t)idt, sizeof (idt) - 1);
}

void idt_set(int id, func_t handler, int flags)
{
    idt[id] = desc_gate((uint32_t)handler, KERNEL_CS, flags);
}

// General exception handler
void do_exception(const regs_t regs)
{
    dump_regs(&regs);
    thread_exit();
}
