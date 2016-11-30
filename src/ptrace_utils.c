#include <string.h>
#include <errno.h>

#include "ptrace_arch.h"
#include "ptrace_utils.h"
#include "log_utils.h"

typedef union _data_unit{
    long val;
    char ptr[WORD_LEN];
} data_unit;

void dump_all_regs(regs_t* regs) {
    arch_dump_all_regs(regs);
}

// Dump len * sizeof(long)
void dump_some_stack(pid_t pid, regs_t* regs, int len){
    int i = 0;
    long word;
    long sp = 0;

    sp = arch_get_stack_pointer(regs);
    for(; i < len; i++){
        ptrace_read_data(pid, &word, (void*)(sp + i * WORD_LEN) , WORD_LEN);
        LOGD("[%08lx] %08lx\n", sp + i * WORD_LEN, word);
    }
}

int do_with_interrupt(pid_t pid, regs_t* regs){
    return arch_do_with_interrupt(pid, regs);
}

int wait_for_signal(pid_t pid, int signo){
    int status;
    if(pid != waitpid(pid, &status, WUNTRACED)){
        LOGE("[PTRACE] wait for signal failed\n");
        return -1;
    }

    if (WIFEXITED(status)){
        if(WEXITSTATUS(status) != signo){
            LOGE("[PTRACE] get signal %d, but need %d\n", WEXITSTATUS(status), signo);
            return -1;
        }
    } else if (WIFSIGNALED(status)){
        if(WTERMSIG(status) != signo){
            LOGE("[PTRACE] get signal %d, but need %d\n", WTERMSIG(status), signo);
            return -1;
        }
    } else if (WIFSTOPPED(status)){
        if(WSTOPSIG(status) != signo){
            LOGE("[PTRACE] get signal %d, but need %d\n", WSTOPSIG(status), signo);
            ptrace_continue(pid, 0);
        }
    } else {
        LOGE("[PTRACE] something error\n");
        return -1;
    }
    return 0;
}

int ptrace_push_stack(pid_t pid, regs_t *regs, void *data, ssize_t len){
    long sp = arch_get_stack_pointer(regs);
    sp -= len;    
    ptrace_write_data(pid, data, (void*)sp, len);     
    return 0;
}

long get_next_ins_addr(regs_t *regs){    
    return arch_get_next_ins_addr(regs);
}    

long get_return_value(regs_t *regs){
    return arch_get_return_value(regs);
}    

long get_syscall_no(pid_t pid, regs_t* regs){
    return  arch_get_syscall_no(pid, regs);
}

int ptrace_trace_me(){
    if(ptrace(PTRACE_TRACEME, 0, NULL, NULL) == -1){
        LOGE("[PTRACE] trace me error (%d) : %s\n", errno, strerror(errno));
        return -1;
    }
    return 0;
}

int ptrace_attach(pid_t pid){
    if(ptrace(PTRACE_ATTACH, pid, NULL, NULL) == -1){
        LOGE("[PTRACE] attach error (%d) : %s\n", errno, strerror(errno));
        return -1;
    }
    return 0;
}

int ptrace_detach(pid_t pid){
    if(ptrace(PTRACE_DETACH, pid, NULL, NULL) == -1){
        LOGE("[PTRACE] detach error (%d) : %s\n", errno, strerror(errno));
        return -1;
    }
    return 0;
}

int ptrace_continue(pid_t pid, int signo){
    if(ptrace(PTRACE_CONT, pid, NULL, signo) == -1){
        LOGE("[PTRACE] continue error (%d) : %s\n", errno, strerror(errno));
        return -1;
    }
    return 0;
}

int ptrace_syscall(pid_t pid){
    if(ptrace(PTRACE_SYSCALL, pid, NULL, NULL) == -1){
        LOGE("[PTRACE] continue error (%d) : %s\n", errno, strerror(errno));
        return -1;
    }
    return 0;
}

int ptrace_get_regs(pid_t pid, regs_t* regs){
#if defined(__LP64__)
    // kernel version >= 2.6.34
	int regset = NT_PRSTATUS;
	struct iovec io_vec;
		
	io_vec.iov_base = regs;
	io_vec.iov_len = sizeof(*regs);
    if (ptrace(PTRACE_GETREGSET, pid, (void*)(long)regset, &io_vec) == -1) {    
#else
    if(ptrace(PTRACE_GETREGS, pid, NULL, regs) == -1){
#endif
        LOGE("[PTRACE] get regs error (%d) : %s\n", errno, strerror(errno));
        return -1;
    }
    return 0;
}

int ptrace_set_regs(pid_t pid, regs_t* regs){
#if defined(__LP64__)
    // kernel version >= 2.6.34
	int regset = NT_PRSTATUS;
	struct iovec io_vec;
		
	io_vec.iov_base = regs;
	io_vec.iov_len = sizeof(*regs);
    if (ptrace(PTRACE_SETREGSET, pid, (void*)(long)regset, &io_vec) == -1) {    
#else
    if(ptrace(PTRACE_SETREGS, pid, NULL, regs) == -1){
#endif
        LOGE("[PTRACE] set regs error (%d) : %s\n", errno, strerror(errno));
        return -1;
    }
    return 0;
}

int ptrace_read_data(pid_t pid, void* buf, void* addr, size_t len){
    char* dest =  (char*)buf;
    char* src = (char*)addr;
    union _data_unit word = {0};
    size_t cnt = len / WORD_LEN;  // numbers of word
    size_t rem = len % WORD_LEN;  // remain bytes, maybe less than a word
    size_t i = 0;
    
    // First, read cnt * WORD_LEN bytes
    for(; i < cnt; i ++){
        word.val = ptrace(PTRACE_PEEKDATA, pid, src, NULL);
        memcpy(dest, word.ptr, WORD_LEN);
        src += WORD_LEN;
        dest += WORD_LEN;
    }

    // Then, do with remain bytes
    if(rem > 0){
        word.val = ptrace(PTRACE_PEEKDATA, pid, src, NULL);
        memcpy(dest, word.ptr, rem);
    }

    return 0;
}

int ptrace_write_data(pid_t pid, void* buf, void* addr, size_t len){
    char* src = (char*)buf;
    char* dest = (char*)addr;
    union _data_unit word = {0};
    size_t cnt = len / WORD_LEN;
    size_t rem = len % WORD_LEN;
    size_t i = 0;
    
    //First, write cnt * WORD_LEN bytes
    for(; i < cnt; i ++){
        memcpy(word.ptr, src, WORD_LEN);
        ptrace(PTRACE_POKEDATA, pid, dest, word.val);
        src += WORD_LEN;
        dest += WORD_LEN;
    }

    //Then, do with remain bytes
    if(rem > 0){
        //Read next word to write
        word.val = ptrace(PTRACE_PEEKDATA, pid, dest, NULL);
        //Replace some bytes head of the word with remain data
        for(i = 0; i < rem; i ++){
            word.ptr[i] = *src;
            src ++;
        }

        //Then, write into tracee back
        ptrace(PTRACE_POKEDATA, pid, dest, word.val);
    }

    return 0;
}

int ptrace_invoke_syscall(pid_t pid, regs_t* regs, long syscall, void** args, size_t nargs){
    long pc;
    if(0 != arch_set_syscall_params(regs, args, nargs)){
        LOGE("[PTRACE] set params error (%d) : %s\n", errno, strerror(errno));
        return -1;
    }

    if(0 != arch_set_syscall_no(regs, syscall)){
        LOGE("[PTRACE] set params error (%d) : %s\n", errno, strerror(errno));
        return -1;
    }

    if(0 != arch_set_next_and_ret_addr(pid, regs, (void*)get_next_ins_addr(regs))){
        LOGE("[PTRACE] set next and ret addr error (%d) : %s\n", errno, strerror(errno));
        return -1;
    }

    if(ptrace_set_regs(pid, regs) == -1){
        return -1;
    }
    if(ptrace_continue(pid, 0) == -1){
        return -1;
    }
 
    if(wait_for_signal(pid, SIGSEGV) == -1){
        return -1;
    }
    return 0;
}

int ptrace_call_func(pid_t pid, regs_t* regs, void* addr, void** args, size_t nargs){
    if(0 != arch_set_params(pid, regs, args, nargs)){
        LOGE("[PTRACE] set params error (%d) : %s\n", errno, strerror(errno));
        return -1;
    }

    if(0 != arch_set_next_and_ret_addr(pid, regs, addr)){
        LOGE("[PTRACE] set next and ret addr error (%d) : %s\n", errno, strerror(errno));
        return -1;
    }

    // dump_all_regs(regs);

    if(ptrace_set_regs(pid, regs) == -1){
        return -1;
    }
    if(ptrace_continue(pid, 0) == -1){
        return -1;
    }
 
    if(wait_for_signal(pid, SIGSEGV) == -1){
        return -1;
    }
    return 0;
}

