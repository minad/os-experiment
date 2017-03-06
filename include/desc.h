#ifndef _DESC_H
#define _DESC_H

#include <types.h>

enum
{
    /*
     * Descriptor types. Every descriptor must contain one.
     */

    // System segment/gate types (sys-flag=0)
    DESC_TYPE_LDT   = 0x0200,  // LDT (GDT only)
    DESC_TYPE_TASK  = 0x0500,  // Task gate (all tables)
    DESC_TYPE_TSS   = 0x0900,  // Available 32 Bit TSS (GDT only)
    DESC_TYPE_CALL  = 0x0C00,  // 32 Bit Call Gate (GDT or LDT), big flag set
    DESC_TYPE_INT   = 0x0E00,  // 32 Bit Interrupt Gate (IDT only), big flag set
    DESC_TYPE_TRAP  = 0x0F00,  // 32 Bit Trap Gate (IDT only), big flag set
    
    // User segment types (sys-flag=1)
    DESC_TYPE_CODE  = 0x180C,  // Code Segment (GDT or LDT), big flag is set, granularity = 4k
    DESC_TYPE_DATA  = 0x100C,  // Data Segment (GDT or LDT), big flag is set, granularity = 4k
    
    // Any descriptor
    DESC_DPL1       = 0x2000,  // DPL: 1 (System Services)
    DESC_DPL2       = 0x4000,  // 2 (Less Privileged System Services)
    DESC_DPL3       = 0x6000,  // 3 (Applications)
    DESC_PRESENT    = 0x8000,  // Present 
   
    /*
     * Descriptor flags
     */

    // Code segment descriptors
    DESC_CONFORMING = 0x400,   // Conforming
    DESC_READ       = 0x200,   // Read Access
    
    // Data segment descriptors
    DESC_EXPANDDOWN = 0x400,   // Expand down
    DESC_WRITE      = 0x200,   // Write Access
    
    // TSS descriptors
    DESC_BUSY       = 0x200,   // Busy TSS
    
    // Code or data segment descriptors
    DESC_ACCESSED   = 0x100,   // Accessed
};


typedef union desc_u
{
    // Gate Descriptor
    struct
    {
        uint offset_low  : 16; // Offset 15..0
        uint selector    : 16; // Selector
        uint access      : 16; // Dword Count, Type, DPL, Present
        uint offset_high : 16; // Offset 16..31
    } gate __packed;

    // Segment Descriptor
    struct
    {
        uint limit_low  : 16; // Limit 0..15
        uint base_low   : 16; // Base 0..15
        uint base_med   : 8;  // Base 16..23
        uint access     : 8;  // Type, DPL, Present
        uint limit_high : 4;  // Limit 16...19
        uint flags      : 4;  // Available, Big, Granularity
        uint base_high  : 8;  // Base 24..31
    } seg __packed;

} desc_t;

/* 
 * Descriptor initializers:
 *
 * Example 1:
 *
 *     desc_t gdt[] = {
 *         DESC_NULL_INIT,
 *         DESC_SEG_INIT(0, 0xFFFFF, DESC_TYPE_CODE | DESC_READ | DESC_PRESENT)
 *     };
 *
 * Example 2:
 *
 *     idt[0] = desc_gate(divide_error, cs, DESC_TRAP);
 */

#define DESC_GATE_INIT(offset, selector, flags) \
{ .gate = { (offset) & 0xFFFF, (selector) & 0xFFFF, \
	    flags, (offset) >> 16 } }

#define DESC_SEG_INIT(base, limit, flags) \
{ .seg = { (limit) & 0xFFFF, (base) & 0xFFFF, ((base) >> 16) & 0xFF, ((flags) >> 8) & 0xFF, \
           ((limit) >> 16) & 0xF, (flags) & 0xF, (base) >> 24 } }

#define DESC_ZERO_INIT \
DESC_SEG_INIT(0, 0, 0)

static inline desc_t desc_gate(uint32_t offset, uint32_t selector, uint32_t flags)
{
    desc_t gate = DESC_GATE_INIT(offset, selector, flags);
    return gate;
}

static inline desc_t desc_seg(uint32_t base, uint32_t limit, uint32_t flags)
{
    desc_t seg = DESC_SEG_INIT(base, limit, flags);
    return seg;
}

#endif // _DESC_H
