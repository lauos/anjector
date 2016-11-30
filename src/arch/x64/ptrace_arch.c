#include <stdio.h>
#include <sys/syscall.h>

#include "ptrace_arch.h"
#include "log_utils.h"

#define TOTAL_PT_REGS   21
#define PARAM_REGS      4

// Copy from kernel
#define ERESTARTSYS	512
#define ERESTARTNOINTR	513
#define ERESTARTNOHAND	514	/* restart if no handler.. */
#define ERESTART_RESTARTBLOCK 516 /* restart by calling sys_restart_syscall */

void  arch_dump_all_regs(regs_t *regs){
    LOGD("-----------------REGS------------------\n");

    LOGD("| [R15]      = %016lx |\n", regs->r15);
    LOGD("| [R14]      = %016lx |\n", regs->r14);
    LOGD("| [R13]      = %016lx |\n", regs->r13);
    LOGD("| [R12]      = %016lx |\n", regs->r12);

    LOGD("| [RBP]      = %016lx |\n", regs->rbp);
    LOGD("| [RBX]      = %016lx |\n", regs->rbx);
    LOGD("| [R11]      = %016lx |\n", regs->r11);
    LOGD("| [R10]      = %016lx |\n", regs->r10);

    LOGD("| [R09]      = %016lx |\n", regs->r9);
    LOGD("| [R08]      = %016lx |\n", regs->r8);
    LOGD("| [RAX]      = %016lx |\n", regs->rax);
    LOGD("| [RCX]      = %016lx |\n", regs->rcx);

    LOGD("| [RDX]      = %016lx |\n", regs->rdx);
    LOGD("| [RSI]      = %016lx |\n", regs->rsi);
    LOGD("| [RDI]      = %016lx |\n", regs->rdi);
    LOGD("| [ORIG_RAX] = %016lx |\n", regs->orig_rax);

    LOGD("| [RIP]      = %016lx |\n", regs->rip);
    LOGD("| [CS]       = %016lx |\n", regs->cs);
    LOGD("| [EFLAGS]   = %016lx |\n", regs->eflags);
    LOGD("| [RSP]      = %016lx |\n", regs->rsp);

    LOGD("| [SS]       = %016lx |\n", regs->ss);

    LOGD("----------------------------------------\n");
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
            regs->rax = regs->orig_rax;
            regs->rip -= 2;
            break;
        case -ERESTART_RESTARTBLOCK:
            LOGD("[+] syscall %ld interruped, do something with the interrupt\n",
                    arch_get_syscall_no(pid, regs));
            regs->rax = __NR_restart_syscall;
            regs->rip -= 2;
            break;
        default:
            break;
    }
    return 0;
}

long arch_get_syscall_no(pid_t pid, regs_t* regs){
    return regs->orig_rax;
}

int arch_set_params(pid_t pid, regs_t* regs, void** params, ssize_t nparams){
    int i = 0;

    // Fill register with the first 4 params
    for(; i < nparams && i < PARAM_REGS; i ++){
        switch(i){
            case 0:
                regs->rcx = (long)params[i];
                break;
            case 1:
                regs->rdx = (long)params[i];
                break;
            case 2:
                regs->r8 = (long)params[i];
                break;
            case 3:
                regs->r9 = (long)params[i];
                break;
            default:
                break;
        }
    }
    
    //Push remain params onto stack(from right to left)
    if(i < nparams){
        ptrace_push_stack(pid, regs, (void*)&params[i], (nparams - i) * WORD_LEN);
    }

    return 0;
}

int arch_set_next_and_ret_addr(pid_t pid, regs_t* regs, void* addr){
    long ret_addr = 0;    
    regs->rip = (long)(addr);

    // Put return address NULL onto stack
    ptrace_push_stack(pid, regs, (void*)&ret_addr, sizeof(ret_addr));
    
    return 0;
}

int arch_set_syscall_params(regs_t* regs, void** params, ssize_t nparams){
    int i = 0;

    for(; i < nparams && i < SYSCALL_REGS; i ++){
        switch(i){
            case 0:
                regs->rdi = (long)params[i];
                break;
            case 1:
                regs->rsi = (long)params[i];
                break;
            case 2:
                regs->rdx = (long)params[i];
                break;
            case 3:
                regs->r10 = (long)params[i];
                break;
            case 4:
                regs->r8 = (long)params[i];
                break;
            case 5:
                regs->r9 = (long)params[i];
                break;
            default:
                break;
        }
    }
    
    return 0;
}
