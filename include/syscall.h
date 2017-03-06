#ifndef _SYSCALL_H
#define _SYSCALL_H

#include <regs.h>

#define SYSCALL_sleep 0

#define _SYSCALL_0(type, name)                         \
type name()                                            \
{                                                      \
    long res;                                          \
    __asm__ __volatile__ ("int $128"                   \
	: "=a" (res)                                   \
	: "0" (SYSCALL_##name));                       \
    return (type)res;                                  \
}


#define _SYSCALL_1(type, name, t1, a1)                 \
type name(t1 a1)                                       \
{                                                      \
    long res;                                          \
    __asm__ __volatile__ ("int $128"                   \
	: "=a" (res)                                   \
	: "0" (SYSCALL_##name),"b" ((long)(a1)));      \
    return (type)res;                                  \
}


#define _SYSCALL_2(type, name, t1, a1, t2, a2)         \
type name(t1 a1, t2 a2)                                \
{                                                      \
    long res;                                          \
    __asm__ __volatile__ ("int $128"                   \
	: "=a" (res)                                   \
	: "0" (SYSCALL_##name),"b" ((long)(a1)),"c" ((long)(a2))); \
    return (type)res;                                  \
}

#define _SYSCALL_3(type, name, t1, a1, t2, a2, t3, a3) \
type name(t1 a1, t2 a2, t3 a3)                         \
{                                                      \
    long res;                                          \
    __asm__ __volatile__ ("int $128"                   \
	: "=a" (res)                                   \
	: "0" (SYSCALL_##name),"b" ((long)(a1)),"c" ((long)(a2)), \
		  "d" ((long)(a3)));                   \
    return (type)res;                                  \
}

_SYSCALL_1(void, sleep, int, ticks)

#endif // _SYSCALL_H
