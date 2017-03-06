#ifndef _ASM_H
#define _ASM_H

#include <types.h>

enum
{
    EFLAGS_CF   = (1<< 0), // Carry Flag
    EFLAGS_PF   = (1<< 2), // Parity Flag
    EFLAGS_AF   = (1<< 4), // Auxillary carry Flag
    EFLAGS_ZF   = (1<< 6), // Zero Flag
    EFLAGS_SF   = (1<< 7), // Sign Flag
    EFLAGS_TF   = (1<< 8), // Trap Flag
    EFLAGS_IF   = (1<< 9), // Interrupt Flag
    EFLAGS_DF   = (1<<10), // Direction Flag
    EFLAGS_OF   = (1<<11), // Overflow Flag
    EFLAGS_IOPL = (2<<12), // IOPL mask
    EFLAGS_NT   = (1<<14), // Nested Task
    EFLAGS_RF   = (1<<16), // Resume Flag
    EFLAGS_VM   = (1<<17), // Virtual Mode
    EFLAGS_AC   = (1<<18), // Alignment Check
    EFLAGS_VIF  = (1<<19), // Virtual Interrupt Flag
    EFLAGS_VIP  = (1<<20), // Virtual Interrupt Pending
    EFLAGS_ID   = (1<<21), // CPUID detection flag
};

static inline uint32_t eflags_get()
{
    uint32_t eflags;
    __asm__ __volatile__ (
        "pushfl    \n\t"
        "popl    %0\n\t"
        : "=r" (eflags));
    return eflags;
}

static inline void eflags_set(uint32_t eflags)
{
    __asm__ __volatile__ (
        "pushl %0\n\t"
        "popfl   \n\t"
        : : "r" (eflags));
}

// Return register by name
#define get_reg(name) \
({ \
    uint32_t name; \
    __asm__ __volatile__ ("movl %%" #name ", %0" : "=r" (name)); \
    name; \
})

// Set register by name
#define set_reg(name, val) \
__asm__ __volatile__ ("movl %0, %%" #name : : "r" (val))

// Set segment register
#define set_seg(name, val) \
__asm__ __volatile__ ("movw %0, %%" #name : : "a" ((uint16_t)val))

// Set code segment register
#define set_cs(val) \
__asm__ __volatile__ ("ljmp %0, $1f\n1:" : : "i" (val));

// Set IDTR (interrupt descriptor table register)
static inline void set_idtr(uint32_t base, uint16_t limit)
{
    uint8_t idtr[6];
    *(uint16_t*)(idtr)     = limit;
    *(uint32_t*)(idtr + 2) = base;
    __asm__ __volatile__ ("lidt (%0)" : : "g" (idtr));
}

// Set GDTR (global descriptor table register)
static inline void set_gdtr(uint32_t base, uint16_t limit)
{
    uint8_t gdtr[6];
    *(uint16_t*)(gdtr)     = limit;
    *(uint32_t*)(gdtr + 2) = base;
    __asm__ __volatile__ ("lgdt (%0)" : : "g" (gdtr));
}

// Set task register
static inline void set_tr(uint16_t sel)
{
    __asm__ __volatile__ ("ltr %0" : :"r" (sel));
}

// Enable paging and set base address
static inline void enable_paging(uint32_t base)
{
    __asm__ __volatile__ (
        "movl %0, %%cr3          \n\t"
        "movl %%cr0, %%eax       \n\t"
        "orl  $0x80000000, %%eax \n\t"
        "movl %%eax, %%cr0       \n\t"
        "jmp  1f; 1:\n" : : "r" (base) : "%eax");
}

// Invalidate TLB for a address (translation lookaside buffer)
static inline void invalidate_tlb(uint32_t addr)
{
#if defined(__i486__) || defined(__i586__) || defined(__i686__)
    __asm__ __volatile__ ("invlpg (%0)" : : "r" (addr));
#else
    __asm__ __volatile__ (
        "movl %%cr3, %%eax\n\t"
        "movl %%eax, %%cr3\n\t" : : : "%eax");
#endif
}

// Are interrupts enabled
static inline bool irqs_enabled()
{
    return ((eflags_get() & EFLAGS_IF) != 0);
}

// Disable interrupts
static inline void irqs_disable()
{
    __asm__ __volatile__ ("cli");
}

// Enable interrupts
static inline void irqs_enable()
{
    __asm__ __volatile__ ("sti");
}

// Save interrupts
static inline void irqs_save(bool* enabled)
{
    *enabled = irqs_enabled();
    if (*enabled)
        irqs_disable();
}

// Restore interrupts
static inline void irqs_restore(bool enabled)
{
    if (enabled)
        irqs_enable();
}

// Force reboot by triple fault
static inline void force_reboot()
{
    set_idtr(0, 0);
    __asm__ __volatile__ ("int3; hlt");
}

static inline void hlt()
{
    __asm__ __volatile__ ("hlt");
}

#define fp_save(buf) \
__asm__ __volatile__ ("fnsave %0" : : "m" (buf))

#define fp_restore(buf) \
__asm__ __volatile__ ("frstor %0" : : "m" (buf))

static inline void fp_init()
{
    __asm__ __volatile__ ("fninit");
}

#endif // _ASM_H
