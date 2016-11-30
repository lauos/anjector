#ifndef _PTRACE_UTILS_H_
#define _PTRACE_UTILS_H_

#include <sys/ptrace.h>
#include <sys/wait.h>
#include <signal.h>

#define WORD_LEN            (sizeof(long)) //Length of a word
#define SYSCALL_REGS                6      //Args of syscall at most (For arm, syscall func exclude)

#if defined(__aarch64__)
    typedef struct user_pt_regs regs_t;
#else
    typedef struct pt_regs regs_t;
#endif

__BEGIN_DECLS

void dump_all_regs(regs_t* regs);
void dump_some_stack(pid_t pid, regs_t* regs, int len);
int do_with_interrupt(pid_t pid, regs_t* regs);

int wait_for_signal(pid_t pid, int signo);
int ptrace_push_stack(pid_t pid, regs_t *regs, void *data, ssize_t len);

long get_next_ins_addr(regs_t *regs);
long get_return_value(regs_t *regs);
long get_syscall_no(pid_t pid, regs_t* regs);

int ptrace_trace_me();
int ptrace_attach(pid_t pid);
int ptrace_detach(pid_t pid);
int ptrace_syscall(pid_t pid);
int ptrace_continue(pid_t pid, int signo);
int ptrace_get_regs(pid_t pid, regs_t* regs);
int ptrace_set_regs(pid_t pid, regs_t* regs);
int ptrace_read_data(pid_t pid, void* buf, void* addr, size_t len);
int ptrace_write_data(pid_t pid, void* buf, void* addr, size_t len);
int ptrace_call_func(pid_t pid, regs_t* regs, void* addr, void**args, size_t nargs);
int ptrace_invoke_syscall(pid_t pid, regs_t* regs, long syscall, void** args, size_t nargs);

__END_DECLS

#endif //_PTRACE_UTILS_H_
