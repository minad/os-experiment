#include <pic.h>
#include <irq.h>
#include <debug.h>

enum
{
    // Initialization Command Word Flags 1
    ICW1_ICW4       = 0x01, // ICW4 used
    ICW1_SINGLE     = 0x02, // Single mode
    ICW1_INTERVAL4  = 0x04, // Call address interval 4
    ICW1_LEVEL      = 0x08, // Level triggered mode
    ICW1_INIT       = 0x10, // Initialization
    
    // Initialization Command Words Flags 4
    ICW4_8086       = 0x01, // 8086/88 (MCS-80/85) mode
    ICW4_AUTO       = 0x02, // Auto EOI */
    ICW4_BUF_SLAVE  = 0x08, // Buffered mode/slave
    ICW4_BUF_MASTER = 0x0C, // Buffered mode/master
    ICW4_SFNM       = 0x10, // Special fully nested
};

static uint16_t irq_mask = 0xFFFF;

static inline void set_irq_mask()
{
    outb(PIC1 + 1, irq_mask);
    outb(PIC2 + 1, irq_mask >> 8);
    TRACE("New IRQ mask: %016b\n", irq_mask);
}

void __init pic_init()
{
    // ICW1: Initialization
    outb(PIC1, ICW1_INIT | ICW1_ICW4);
    outb(PIC2, ICW1_INIT | ICW1_ICW4);
    
    // ICW2: Set interrupt base addresses
    outb(PIC1 + 1, PIC_INTBASE);
    outb(PIC2 + 1, PIC_INTBASE + 8);

    // ICW3: Set master and slave
    outb(PIC1 + 1, 1 << IRQ_CASCADE); // Master (IRQ2 connected to Slave)
    outb(PIC2 + 1, IRQ_CASCADE);      // Slave

    // ICW4: 
    outb(PIC1 + 1, ICW4_8086);
    outb(PIC2 + 1, ICW4_8086);
    
    set_irq_mask();
}

void pic_irq_enable(int irq)
{
    ASSERT(irq < 16);
    
    irq_mask &= ~(1 << irq);
    
    // Unmask cascaded irq
    if (irq > 8)
        irq_mask &= ~(1 << IRQ_CASCADE);

    set_irq_mask();
}

void pic_irq_disable(int irq)
{
    ASSERT(irq < 16);

    irq_mask |= 1 << irq;
    
    // Mask cascaded irq
    if ((irq_mask >> 8) == 0xFF)
        irq_mask |= 1 << IRQ_CASCADE;

    set_irq_mask();
}

