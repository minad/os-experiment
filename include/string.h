#ifndef _STRING_H
#define _STRING_H

#include <types.h>

/*
 * POSIX buffer routines
 */

void *memccpy(void *, const void *, int, size_t);
int memcmp(const void *, const void *, size_t);
void *memcpy(void *, const void *, size_t);
void *memset(void *, int, size_t);
void *memchr(const void *, int, size_t);
void *memmove(void *, const void *, size_t);

/*
 * POSIX string routines
 */

char *strcat(char *, const char *);
char *strchr(const char *, int);
int strcmp(const char *, const char *);
char *strcpy(char *, const char *);
size_t strcspn(const char *, const char *);
size_t strlen(const char *);
char *strncat(char *, const char *, size_t);
int strncmp(const char *, const char *, size_t);
char *strncpy(char *, const char *, size_t);
char *strpbrk(const char *, const char *);
char *strrchr(const char *, int);
size_t strspn(const char *, const char *);
char *strstr(const char *, const char *);

char* strdup(const char*);

/*
 * Non-POSIX string routines
 */

int stricmp(const char *, const char *);
char *strlwr(char *);
int strnicmp(const char *, const char *, size_t);
char *strsep(char **, const char *); // strtok replacement
char *strupr(char *);

/*
 * String to integer
 */

long atol(const char *str);
int atoi(const char *str);
long strtol(const char *str, char **end, int base);

#endif // _STRING_H

