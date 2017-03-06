#include <desc.h>
#include <asm.h>
#include <segment.h>

/* Global descriptor table
 * (filled with temporary descriptors)
 */
desc_t gdt[GDT_ENTRIES] =
{
    DESC_ZERO_INIT,
    DESC_SEG_INIT(PHYS_OFFSET, 0xFFFFF, DESC_TYPE_CODE | DESC_READ | DESC_PRESENT),
    DESC_SEG_INIT(PHYS_OFFSET, 0xFFFFF, DESC_TYPE_DATA | DESC_WRITE | DESC_PRESENT),
};

void __init gdt_setup_flat()
{
    // New linear address (because we've paging now)
    set_gdtr((uint32_t)gdt, 8 * GDT_ENTRIES);
    
    // 4 GB Kernel
    gdt[1] = desc_seg(0, 0xFFFFF, DESC_TYPE_CODE | DESC_READ | DESC_PRESENT);
    gdt[2] = desc_seg(0, 0xFFFFF, DESC_TYPE_DATA | DESC_WRITE | DESC_PRESENT);
    
    // 4 GB User
    gdt[3] = desc_seg(0, 0xFFFFF, DESC_TYPE_CODE | DESC_READ | DESC_DPL3 | DESC_PRESENT);
    gdt[4] = desc_seg(0, 0xFFFFF, DESC_TYPE_DATA | DESC_WRITE | DESC_DPL3 | DESC_PRESENT);

    // Reload segments (because of the descriptor cache)
    set_cs(KERNEL_CS);
    set_seg(ds, KERNEL_DS);
    set_seg(es, KERNEL_DS);
    set_seg(ss, KERNEL_DS);
}

void gdt_set(int id, desc_t desc)
{
    gdt[id] = desc;
}
