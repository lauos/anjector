#include <stdio.h>
#include <errno.h>
#include <sys/syscall.h>

#include "ptrace_arch.h"
#include "log_utils.h"

#define TOTAL_PT_REGS   18
#define PARAM_REGS      4

void arch_dump_all_regs(regs_t *regs){
    int i = 0;
    LOGD("----------REGS-----------\n");
    for (i = 0; i < TOTAL_PT_REGS; i ++) {  
        if(i < TOTAL_PT_REGS - 7){
            LOGD("| R[%02d]      = %08lx |\n", i, regs->uregs[i]);
        } else{
            switch(i){
                case 11:
                    LOGD("| R[%02d](FP)  = %08lx |\n", i, regs->uregs[i]);
                    break;
                case 12:
                    LOGD("| R[%02d](IP)  = %08lx |\n", i, regs->uregs[i]);
                    break;
                case 13:
                    LOGD("| R[%02d](SP)  = %08lx |\n", i, regs->uregs[i]);
                    break;
                case 14:
                    LOGD("| R[%02d](LR)  = %08lx |\n", i, regs->uregs[i]);
                    break;
                case 15:
                    LOGD("| R[%02d](PC)  = %08lx |\n", i, regs->uregs[i]);
                    break;
                case 16:
                    LOGD("| R[%02d](CPSR)= %08lx |\n", i, regs->uregs[i]);
                    break;
                case 17:
                    LOGD("| R[%02d](ORIG)= %08lx |\n", i, regs->uregs[i]);
                    break;
                default :
                    break;
            }
        }
    }
    LOGD("-------------------------\n");
}

int arch_do_with_interrupt(pid_t pid, regs_t *regs){
    long ret = arch_get_return_value(regs);

    //When internal syscall return -ERESTART_RESTARTBLOCK, 
    //tracer could only see -EINTR
    if(-EINTR == ret){
        LOGD("[+] syscall %ld interruped\n", arch_get_syscall_no(pid, regs));
        // When we set pc to another addr, the syscall restart will be ignore by kernel,
        // so we shoudle do this manually  
        if(regs->ARM_cpsr & PSR_T_BIT){
            regs->ARM_r7 = __NR_restart_syscall - __NR_SYSCALL_BASE;
            regs->ARM_pc -= 2;
        } else {
            regs->ARM_r7 = __NR_restart_syscall;
            regs->ARM_pc -= 4;
        }
    }

    return 0;
}

long  arch_get_syscall_no(pid_t pid, regs_t* regs){
    long syscall = -1;
    long pc = 0;

    //Get the instruction now running
    ptrace_read_data(pid, (void*)&pc, (void*)(regs->ARM_pc - 4), WORD_LEN);
    //LOGD("[+] [ins at (pc - 4)] : %lx\n", pc);

    //1110 1111 0000 0000
    //eabi-> 0xef000000 : swi 0
    if(pc == 0xef000000){
        syscall = regs->ARM_r7;
    } else {
        //1101 1111 0000 0000
        //oabi syscall base 0x900000
        //oabi-> 0xdf000000 | 0x900000 | syscall_num : swi #immed_24
        if((pc & 0x0ff00000) == 0x0f900000){
            syscall = pc & 0x000fffff;
        }
    }
    return syscall;
}

int arch_set_params(pid_t pid, regs_t* regs, void** params, ssize_t nparams){
    int i = 0;

    // Fill R0-R3 with the first 4 params
    for(; i < nparams && i < PARAM_REGS; i ++){
        regs->uregs[i] = (long)params[i];
    }
    
    //Push remain params onto stack(from right to left)
    if(i < nparams){
        ptrace_push_stack(pid, regs, (void*)&params[i], (nparams - i) * WORD_LEN);
    }

    return 0;
}

int arch_set_next_and_ret_addr(pid_t pid, regs_t* regs, void* addr){
    long ret_addr = 0;    
    regs->ARM_pc = (long)(addr);

    //Thumb or ARM
    if(regs->ARM_pc & 1){
        //Thumb, PC align and CPSR T flag set 1
        regs->ARM_pc &= (~1u);
        regs->ARM_cpsr |= PSR_T_BIT;
    } else {
        //ARM, CPSR T flag set 0
        regs->ARM_cpsr &= (~PSR_T_BIT);
    }

    // Set func return addr 0, then the target process will catch a SIGSEGV signal
    regs->ARM_lr = ret_addr;
    return 0;
}

int arch_set_syscall_params(regs_t* regs, void** params, ssize_t nparams){
    int i = 0;

    // Fill R0-R5 with the params
    for(; i < nparams && i < SYSCALL_REGS; i ++){
        regs->uregs[i] = (long)params[i];
    }
    
    return 0;
}
