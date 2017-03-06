#ifndef _SECTION_H
#define _SECTION_H

#include <types.h>

/*
 * Section constants
 *
 * They must be prototyped as char[]!
 */

/*
 * Kernel
 */

extern char _KERNEL_START[];
extern char _KERNEL_START_PHYS[];
extern char _KERNEL_SIZE[];
#define KERNEL_START      ((uint32_t)_KERNEL_START)
#define KERNEL_START_PHYS ((uint32_t)_KERNEL_START_PHYS)
#define KERNEL_SIZE       ((uint32_t)_KERNEL_SIZE)

/*
 * Text section
 */
extern char _TEXT_START[];
extern char _TEXT_SIZE[];
#define TEXT_START ((uint32_t)_TEXT_START)
#define TEXT_SIZE  ((uint32_t)_TEXT_SIZE)

/*
 * Init text section
 */
extern char _INIT_TEXT_START[];
extern char _INIT_TEXT_SIZE[];
#define INIT_TEXT_START ((uint32_t)_INIT_TEXT_START)
#define INIT_TEXT_SIZE  ((uint32_t)_INIT_TEXT_SIZE)

/*
 * Init section
 */

extern char _INIT_START[];
extern char _INIT_SIZE[];
#define INIT_START ((uint32_t)_INIT_START)
#define INIT_SIZE  ((uint32_t)_INIT_SIZE)

/*
 * Init constructors section
 */

extern char _CTORS_START[];
#define CTORS_START ((uint32_t)_CTORS_START)

#endif // _SECTION_H
