#ifndef _STDIO_H
#define _STDIO_H

#include <stdarg.h>
#include <types.h>
#include <console.h>

int vsnprintf(char *, size_t, const char *, va_list);
int snprintf(char *, size_t, const char *, ...) __printf(3, 4);

int vprintf(const char *, va_list);
int printf(const char *, ...) __printf(1, 2);
int puts(const char*);
int putchar(int);

// Console functions
int con_vprintf(int, const char *, va_list);
int con_printf(int, const char *, ...) __printf(2, 3);
int con_puts(int, const char*);
//int con_putchar(int, int); in console.h

#endif // _STDIO_H
