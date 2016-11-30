#ifndef _PTRACE_ARCH_H_
#define _PTRACE_ARCH_H_

#include <linux/elf.h> // For NT_PRSTATUS
#include <sys/uio.h>   // For struct iovec 

#include "ptrace_utils.h"

__BEGIN_DECLS

static inline long arch_get_next_ins_addr(regs_t *regs){
    return regs->rip;
}

static inline long arch_get_return_value(regs_t *regs){
    return regs->rax;
}

static inline long arch_get_stack_pointer(regs_t *regs){
    return regs->rsp;
}

static inline int arch_set_syscall_no(regs_t *regs, long new_syscall){
    regs->rax = new_syscall;
    return 0;
}

void  arch_dump_all_regs(regs_t* regs);
int   arch_do_with_interrupt(pid_t pid, regs_t* regs);
long  arch_get_syscall_no(pid_t pid, regs_t* regs);
int   arch_set_syscall_params(regs_t* regs, void** params, ssize_t nparams);
int   arch_set_params(pid_t pid, regs_t* regs, void** param, ssize_t nargs);
int   arch_set_next_and_ret_addr(pid_t pid, regs_t* regs, void* addr);

__END_DECLS

#endif
