#include <stdio.h>
#include <sys/syscall.h>

#include "ptrace_arch.h"
#include "log_utils.h"

#define TOTAL_PT_REGS   17
#define PARAM_REGS      0

// Copy from kernel
#define ERESTARTSYS	512
#define ERESTARTNOINTR	513
#define ERESTARTNOHAND	514	/* restart if no handler.. */
#define ERESTART_RESTARTBLOCK 516 /* restart by calling sys_restart_syscall */

void  arch_dump_all_regs(regs_t *regs){
    LOGD("----------REGS-----------\n");
    LOGD("| [EBX]      = %08lx |\n", regs->ebx);
    LOGD("| [ECX]      = %08lx |\n", regs->ecx);
    LOGD("| [EDX]      = %08lx |\n", regs->edx);
    LOGD("| [ESI]      = %08lx |\n", regs->esi);

    LOGD("| [EDI]      = %08lx |\n", regs->edi);
    LOGD("| [EBP]      = %08lx |\n", regs->ebp);
    LOGD("| [EAX]      = %08lx |\n", regs->eax);
    LOGD("| [XDS]      = %08x |\n", regs->xds);

    LOGD("| [XES]      = %08x |\n", regs->xes);
    LOGD("| [XFS]      = %08x |\n", regs->xfs);
    LOGD("| [XGS]      = %08x |\n", regs->xgs);
    LOGD("| [ORIG_EAX] = %08lx |\n", regs->orig_eax);

    LOGD("| [EIP]      = %08lx |\n", regs->eip);
    LOGD("| [XCS]      = %08x |\n", regs->xcs);
    LOGD("| [EFLAGS]   = %08lx |\n", regs->eflags);
    LOGD("| [ESP]      = %08lx |\n", regs->esp);
    LOGD("| [XSS]      = %08x |\n", regs->xss);
    LOGD("-------------------------\n");
}

int arch_do_with_interrupt(pid_t pid, regs_t *regs){
    long ret = arch_get_return_value(regs);
    //When tracer's attach interrupt the syscall, tracee notify tracer first,
    //so tracer can see syscall internal error, we should do restart help kernel.
    //When tracee return to execute, syscall has no error, so kernel does not set
    //again
    switch(ret){
        case -ERESTARTSYS:
        case -ERESTARTNOHAND:
        case -ERESTARTNOINTR:
            LOGD("[+] syscall %ld interruped, do something with the interrupt\n",
                    arch_get_syscall_no(pid, regs));
            regs->eax = regs->orig_eax;
            regs->eip -= 2;
            break;
        case -ERESTART_RESTARTBLOCK:
            LOGD("[+] syscall %ld interruped, do something with the interrupt\n",
                    arch_get_syscall_no(pid, regs));
            regs->eax = __NR_restart_syscall;
            regs->eip -= 2;
            break;
        default:
            break;
    }
    return 0;
}

long arch_get_syscall_no(pid_t pid, regs_t* regs){
    return regs->orig_eax;
}

int arch_set_params(pid_t pid, regs_t* regs, void** params, ssize_t nparams){
    //Push all params onto stack(from right to left)
   return  ptrace_push_stack(pid, regs, (void*)&params[0], nparams * WORD_LEN);
}

int arch_set_next_and_ret_addr(pid_t pid, regs_t* regs, void* addr){
    long ret_addr = 0;    
    regs->eip = (long)(addr);
    
    // Put return address NULL onto stack
    ptrace_push_stack(pid, regs, (void*)&ret_addr, sizeof(ret_addr));
    return 0;
}

int arch_set_syscall_params(regs_t* regs, void** params, ssize_t nparams){
    int i = 0;

    for(; i < nparams && i < SYSCALL_REGS; i ++){
        switch(i){
            case 0:
                regs->ebx = (long)params[i];
                break;
            case 1:
                regs->ecx = (long)params[i];
                break;
            case 2:
                regs->edx = (long)params[i];
                break;
            case 3:
                regs->esi = (long)params[i];
                break;
            case 4:
                regs->edi = (long)params[i];
                break;
            case 5:
                regs->ebp = (long)params[i];
                break;
            default:
                break;
        }
    }
    
    return 0;
}
