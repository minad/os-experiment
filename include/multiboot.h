#ifndef _MULTIBOOT_H
#define _MULTIBOOT_H

#include <types.h>

/*
 * Multiboot flags
 */
enum
{
    MULTIBOOT_MAGIC            = 0x2BADB002,
    MULTIBOOT_MEM              = 0x001,
    MULTIBOOT_BOOT_DEVICE      = 0x002,
    MULTIBOOT_CMDLINE          = 0x004,
    MULTIBOOT_MODS             = 0x008,
    MULTIBOOT_AOUT_SYMS        = 0x010,
    MULTIBOOT_ELF_SECTS        = 0x020,
    MULTIBOOT_MMAP             = 0x040,
    MULTIBOOT_DRIVES           = 0x080,
    MULTIBOOT_CONFIG_TABLE     = 0x100,
    MULTIBOOT_BOOT_LOADER_NAME = 0x200,
};

/*
 * Multiboot info structure
 */
typedef struct multiboot_info_s
{
    uint32_t  flags;
    uint32_t  mem_lower;
    uint32_t  mem_upper;
    uint32_t  boot_device;
    char*     cmdline;
    uint32_t  mods_count;
    void*     mods_addr; 
    uint32_t  syms[4];
    uint32_t  mmap_length;
    uint32_t  map_addr;
    uint32_t  drives_length;
    void*     drives_addr;
    void*     config_table;
    char*     boot_loader_name;
} multiboot_info_t __packed;

// Load multiboot information
void multiboot_init(uint32_t eax, uint32_t ebx, int* argc, char** argv);
const multiboot_info_t* multiboot_get();

#endif // _MULTIBOOT_H
