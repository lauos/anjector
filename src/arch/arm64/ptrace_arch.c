/*
 * ptrace_arch.c
 * Copyright (C) 2016 Gavin Liu <lbliuyun@gmail.com>
 *
 * Distributed under terms of the MIT license.
 */

#include <stdio.h>

#include "ptrace_arch.h"
#include "log_utils.h"

#define COMPAT_PSR_T_BIT       (1u << 5) 
#define TOTAL_PT_REGS          34
#define PARAM_REGS             8

void  arch_dump_all_regs(regs_t *regs){
    int i = 0;
    LOGD("------------------REGS----------------------\n");
    for (i = 0; i < TOTAL_PT_REGS - 3; i++) {  
        LOGD("| X[%02d]    = %016llx |\n", i, regs->regs[i]);
    }

    LOGD("| [SP]     = %016llx |\n", regs->sp);
    LOGD("| [PC]     = %016llx |\n", regs->pc);
    LOGD("| [PSTATE] = %016llx |\n", regs->pstate);
    LOGD("--------------------------------------------\n");
}

int arch_do_with_interrupt(pid_t pid, regs_t *regs){
    //When tracer's attach interrupt syscall, kernel prepare restart syscall
    //before notify tracer, so tracer can not see the internal syscall return 
    //value, so we just ignore ERESTART_RESTARTBLOCK, and do nothing.
    return 0;
}

long arch_get_syscall_no(pid_t pid, regs_t* regs){
    return regs->regs[8];
}

int arch_set_params(pid_t pid, regs_t* regs, void** params, ssize_t nparams){
    int i = 0;

    // Fill X0-X7 with the first 8 params
    for(; i < nparams && i < PARAM_REGS; i ++){
        regs->regs[i] = (long)params[i];
    }
    
    //Push remain params onto stack(from right to left)
    if(i < nparams){
        ptrace_push_stack(pid, regs, (void*)&params[i], (nparams - i) * WORD_LEN);
    }

    return 0;
}

int arch_set_next_and_ret_addr(pid_t pid, regs_t* regs, void* addr){
    long ret_addr = 0;    
    regs->pc = (long)(addr);

    //Whrn run in thumb or arm compat
    if(regs->pc & 1){
        //Thumb, PC align and CPSR T flag set 1
        regs->pc &= (~1u);
        regs->pstate |= COMPAT_PSR_T_BIT;
    } else {
        //ARM, CPSR T flag set 0
        regs->pstate &= (~COMPAT_PSR_T_BIT);
    }

    // Set func return addr 0, then the target process will catch a SIGSEGV signal
    regs->regs[30] = ret_addr;
    return 0;
}

int arch_set_syscall_params(regs_t* regs, void** params, ssize_t nparams){
    int i = 0;

    // Fill X0-X5 with the params
    for(; i < nparams && i < SYSCALL_REGS; i ++){
        regs->regs[i] = (long)params[i];
    }
    
    return 0;
}
