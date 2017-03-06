#ifndef _VMEM_H
#define _VMEM_H

#include <types.h>

/*
 * Initialize memory management
 */

void vmem_init() __init;

/*
 * Allocate/free single page of virtual memory
 */

bool vmem_alloc(uint32_t start, uint32_t end, int flags);
void vmem_free(uint32_t start, uint32_t end);
bool vmem_alloc_page(uint32_t addr, int flags);
void vmem_free_page(uint32_t addr);

/*
 * Map physical memory directly. These functions are really low-level;
 * there's no checking done if the physical pages are free.
 * This can be used for VGA or DMA.
 * 
 * You should use:
 *    
 *    vmem_map(start, end, phys_start, PAGE_WRITE);
 *    pmem_reserve(phys_start, phys_start + (end - start));
 */

bool vmem_map(uint32_t start, uint32_t end, uint32_t phys_start, int flags);
void vmem_unmap(uint32_t start, uint32_t end);
bool vmem_map_page(uint32_t addr, uint32_t page, int flags);
uint32_t vmem_unmap_page(uint32_t addr);

#endif // _VMEM_H
