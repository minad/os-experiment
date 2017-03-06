#include <stdio.h>
#include <regs.h>

void sys_sleep(int ticks)
{
    //thread_sleep2(ticks);
    puts("-----------------------------------------\n");
    //printf("sys_sleep %d\n");
}

typedef long (*syscall_t)(long, long, long, long, long, long);
static syscall_t system_call[] =
{
      sys_sleep,
//    sys_fork,
//    sys_execve,
//    sys_dothis,
//    sys_dothat,
};

#define NR_SYSCALLS (sizeof (system_call) / sizeof (system_call[0]))

// That's the linux syscall convention
#define nr   (regs.eax)
#define arg0 (regs.ebx)
#define arg1 (regs.ecx)
#define arg2 (regs.edx)
#define arg3 (regs.esi)
#define arg4 (regs.edi)
#define arg5 (regs.ebp)
#define ret  (regs.eax)

void do_system_call(regs_t regs)
{
    if (nr > NR_SYSCALLS)
        ret = -1;
    else
        ret = (system_call[nr])(arg0, arg1, arg2, arg3, arg4, arg5);
}
