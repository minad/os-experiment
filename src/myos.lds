// VIRT_TO_PHYS macro
#include <segment.h>

// Start section 
#define SECTION(name) \
    name : AT(VIRT_TO_PHYS(ADDR(name)))

// Define start symbols
#define DEFINE_START(name) \
    _##name##_START = ABSOLUTE(.); \
    _##name##_START_PHYS = VIRT_TO_PHYS(_##name##_START);

// Define end symbols
#define DEFINE_END(name) \
    _##name##_SIZE = ABSOLUTE(.) - _##name##_START;

// Page alignment
#define PAGE_ALIGN ALIGN(4096)

OUTPUT(myos.elf)
OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386")

// Start address must be physical because of the boot loader
start = VIRT_TO_PHYS(kernel_entry);

SECTIONS
{
    . = KERNEL_VIRT;

    DEFINE_START(KERNEL)

    // Text Section
    SECTION(.text)
    {
        DEFINE_START(TEXT)
	
	// Multiboot Header
        LONG(0x1BADB002)        // Magic
        LONG(3)                 // Flags: Page align, Memory info required
        LONG(-(0x1BADB002 + 3)) // Checksum
        
	// Code
        *(.text*)
	
	// Read-only data: Strings, etc.
        *(.rodata*)
        
	// Align because .text will be readonly
        . = PAGE_ALIGN;
	DEFINE_END(TEXT)
    }
 
    // Data Section
    SECTION(.data)
    {
        DEFINE_START(DATA)
        *(.data*)
	DEFINE_END(DATA)
    }

    // BSS Section (zeroed by the boot loader)
    SECTION(.bss)
    {
        DEFINE_START(BSS)
        *(.bss*)
        *(COMMON)
	DEFINE_END(BSS)
    }
   
    // Page aligned because init will be freed
    . = PAGE_ALIGN;
    DEFINE_START(INIT)
    
    // Init Text
    SECTION(.init.text)
    {
        DEFINE_START(INIT_TEXT)
        *(.init.text*)
        DEFINE_END(INIT_TEXT)
    }
   
    // Init Data
    SECTION(.init.data)
    {
        DEFINE_START(INIT_DATA)
        *(.init.data*)
        DEFINE_START(CTORS)
        *(.ctor*)
	LONG(0)
        DEFINE_END(CTORS)
	DEFINE_END(INIT_DATA)
    }

    // Page aligned because init will be freed
    . = PAGE_ALIGN;
    DEFINE_END(INIT)
    
    DEFINE_END(KERNEL)

    // Keep the size down...
    /DISCARD/ : {
        *(.comment*)
        *(.note*)
    }
}
