#ifndef _IO_H
#define _IO_H

#include <types.h>

static inline void iodelay()
{
    __asm__ __volatile__ ("jmp 1f\n1:\tjmp 1f\n1:");
}

#define DEFINE_FUNCS(mod, type) \
static inline type in##mod(uint16_t p) \
{ \
    type v; \
    __asm__ __volatile__ ("in" #mod " %1, %0" : "=a" (v) : "d" (p)); \
    return v; \
} \
static inline void out##mod(uint16_t p, type v) \
{ \
    __asm__ __volatile__ ("out" #mod " %1, %0" : : "d" (p), "a" (v)); \
} \
static inline void out##mod##_p(uint16_t p, type v) \
{ \
    out##mod(p, v); \
    iodelay(); \
} \
static inline type in##mod##_p(uint16_t p) \
{ \
    type v = in##mod(p); \
    iodelay(); \
    return v; \
}
    
DEFINE_FUNCS(b, uint8_t)
DEFINE_FUNCS(w, uint16_t)
DEFINE_FUNCS(l, uint32_t)

#undef DEFINE_FUNCS

#endif // _IO_H
