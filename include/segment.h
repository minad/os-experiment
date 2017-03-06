#ifndef _SEGMENTS_H
#define _SEGMENTS_H

// GDT entries
#define GDT_ENTRIES 8

// Segment selectors
#define KERNEL_CS  0x08
#define KERNEL_DS  0x10
#define USER_CS    0x1B
#define USER_DS    0x23
#define KERNEL_TSS 0x28

/* 
 * There are three phases in memory initialization:
 *     1. After booting:        Identity linear to physical mapping.
 *     2. Temporary kernel gdt: Physical 0x100000 is mapped to linear 0xC0100000
 *                              (with gdt base 0x40000000)
 *     3. Paging is on:         Flat segmentation, Physical 0x100000 is mapped to 0xC0100000
 */

// Kernel start addresses
#define KERNEL_PHYS 0x100000
#define KERNEL_VIRT 0xC0100000

// Conversion offsets (virtual <-> physical)
#define VIRT_OFFSET 0xC0000000
#define PHYS_OFFSET 0x40000000

#define VIRT_TO_PHYS(addr) ((addr) - VIRT_OFFSET + 0) // crazy ld needs '+ 0'
#define PHYS_TO_VIRT(addr) ((addr) + VIRT_OFFSET)

#endif // _SEGMENTS_H
