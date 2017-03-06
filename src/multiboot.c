#include <segment.h>
#include <string.h>
#include <multiboot.h>
#include <debug.h>
#include <stdio.h>

static multiboot_info_t info;
static char cmdline[512];

void __init multiboot_init(uint32_t eax, uint32_t ebx, int* argc, char** argv)
{
    char* p;
    int   max_argc;

    // No multiboot loader?
    if (eax != MULTIBOOT_MAGIC)
         panic("No multiboot loader: 0x%8X\n", eax);
    
    /*
     * Copy multiboot structure, cmdline and
     * memory mapping info to kernel space.
     */
    
    memcpy(&info, (void*)PHYS_TO_VIRT(ebx), sizeof (info));
    
    if (info.flags & MULTIBOOT_BOOT_LOADER_NAME)
        printf(" Boot Loader: %s\n", PHYS_TO_VIRT(info.boot_loader_name));
        
    if (info.flags & MULTIBOOT_CMDLINE)
    {
        strncpy(cmdline, PHYS_TO_VIRT(info.cmdline), sizeof (cmdline));
        printf(" Cmdline: %s\n", cmdline);
    }
    
    // These fields aren't loaded from physical memory
    // so they're invalid
    info.flags &=
        ~MULTIBOOT_CMDLINE      &
	~MULTIBOOT_MMAP         &
        ~MULTIBOOT_MODS         &
        ~MULTIBOOT_AOUT_SYMS    &
        ~MULTIBOOT_ELF_SECTS    &
        ~MULTIBOOT_DRIVES       &
        ~MULTIBOOT_CONFIG_TABLE & 
        ~MULTIBOOT_BOOT_LOADER_NAME;

    // Parse command line
    p = cmdline;
    max_argc = *argc;
    *argc = 0;
    while (*argc < max_argc - 1)
    {
        argv[*argc] = strsep(&p, " ");
	if (!argv[*argc])
	    break;
	++*argc;
    }
    argv[*argc] = NULL;
}

const multiboot_info_t* multiboot_get()
{
    return &info;
}

