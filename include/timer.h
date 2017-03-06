#ifndef _TIMER_H
#define _TIMER_H

#include <types.h>
#include <list.h>

typedef struct timer_s
{
    callback_t call;
    void*      arg;
    ullong     expires;
    list_t     list_entry;
} timer_t;

void timer_init() __init;
void timer_add(callback_t, void*, int);

#endif
