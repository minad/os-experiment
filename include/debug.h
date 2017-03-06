#ifndef _DEBUG_H
#define _DEBUG_H

#include <regs.h>
#include <stdio.h>

void panic(const char *, ...) __printf_noreturn(1, 2);
void dump_regs(const regs_t*);
void stack_trace(int);

#ifndef NDEBUG

#define TRACE(fmt, args...) \
    printf(fmt, ##args)

#define DUMP(mem, size) \
    _dump(mem, size)

#define ASSERT(expr) \
    ((expr) ? (void)0 : _assert(#expr, __FILE__, __LINE__, __FUNCTION__, NULL))

#define XASSERT(expr, fmt, args...) \
    ((expr) ? (void)0 : _assert(#expr, __FILE__, __LINE__, __FUNCTION__, fmt, ##args))

void _assert(const char *, const char *, int, const char *,
              const char *, ...) __noreturn;

void _dump(const void*, size_t);

#else

#define TRACE(fmt, args...)
#define DUMP(mem, size)
#define ASSERT(expr)
#define XASSERT(expr, fmt, ...)

#endif

#endif // _DEBUG_H
