#ifndef _GDT_H
#define _GDT_H

#include <desc.h>

void gdt_setup_flat();
void gdt_set(int, desc_t);

#endif
