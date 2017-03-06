#ifndef _PIC_H
#define _PIC_H

#include <types.h>
#include <io.h>

enum
{
    // PIC interrupt base address
    PIC_INTBASE = 0x20,

    // PIC ports
    PIC1 = 0x20,
    PIC2 = 0xA0,

    // End of interrupt command
    PIC_EOI = 0x20,
};

// Initialize controller
void pic_init() __init;

// Enable/Disable IRQ
void pic_irq_enable(int);
void pic_irq_disable(int);

// Send EOI
static inline void pic_irq_end(int irq)
{
    outb(PIC1, PIC_EOI);
    if (irq >= 8)
        outb(PIC2, PIC_EOI);
}

#endif // _PIC_H

