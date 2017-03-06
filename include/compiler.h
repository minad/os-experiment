#ifndef _COMPILER_H
#define _COMPILER_H

// Non-returning
#define __noreturn __attribute__ ((noreturn))

// Packed structure
#define __packed //__attribute__ ((packed))

// Printf format
#define __printf(a, b)          __attribute__ ((format (printf, a, b)))
#define __printf_noreturn(a, b) __attribute__ ((format (printf, a, b), noreturn))

// Init sections
#define __init     __attribute__ ((__section__ (".init.text")))
#define __initdata __attribute__ ((__section__ (".init.data")))

// Branch prediction
#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#endif // _COMPILER_H
