#ifndef _MALLOC_H
#define _MALLOC_H 1

#include <types.h>

/*
 * Kernel memory allocation
 */

void* malloc(size_t);
void* calloc(size_t);
void* realloc(void*, size_t);
void  free(void*);

#endif
