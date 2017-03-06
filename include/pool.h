#ifndef _POOL_H
#define _POOL_H

#include <list.h>
#include <types.h>

typedef struct pool_s {
    size_t element_size;
    size_t element_count;
    list_t free_elements;
    list_t blocks;
} pool_t;

void pool_init(pool_t* pool, size_t count, size_t size);
void pool_free(pool_t* pool);
void* pool_get(pool_t* pool);
void* pool_release(pool_t* pool, void* obj);

#endif
