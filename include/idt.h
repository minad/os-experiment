#ifndef _IDT_H
#define _IDT_H

#include <types.h>

void idt_init() __init;
void idt_set(int id, func_t handler, int flags);

#endif // _IDT_H
