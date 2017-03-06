#ifndef _PMEM_H
#define _PMEM_H

#include <types.h>

/*
 * Memory statistics
 */
typedef struct pmem_stats_s
{
    // All values in pages
    int total;
    int free;
    int used;
    int upper;
    int lower;
    int kernel;
} pmem_stats_t;

/*
 * General functions 
 */

void pmem_init() __init;
const pmem_stats_t* pmem_get_stats();
void pmem_dump_stats(const pmem_stats_t*);
void pmem_dump_map();

/*
 * Manage physical memory
 */

uint32_t pmem_alloc_page();
void     pmem_free_page(uint32_t);
void     pmem_alloc_region(uint32_t, uint32_t);
void     pmem_free_region(uint32_t, uint32_t);

/*
 * Usage counters for physical pages which must be
 * allocated
 * 
 * pmem_inc() increments usage counter
 * pmem_dec() decrements usage counter
 */

int pmem_inc(uint32_t);
int pmem_dec(uint32_t);

#endif // _PMEM_H
