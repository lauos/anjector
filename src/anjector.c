/*
 * anjector.c
 * Copyright (C) 2016 Gavin Liu <lbliuyun@gmail.com>
 *
 * Distributed under terms of the MIT license.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dlfcn.h>
#include <limits.h>
#include <sys/mman.h>
#include <sys/syscall.h>

#include "opt_utils.h"
#include "log_utils.h"
#include "proc_utils.h"
#include "ptrace_utils.h"

static int ptrace_call_wrapper(pid_t pid, regs_t* regs, 
        void* addr, void**args, size_t nargs, void** result){
    int ret = -1;
    do{
        if(0 != ptrace_call_func(pid, regs, addr, args, nargs)){
            LOGE("[+] ptrace call func failed\n");
            break;
        }

        if(0 != ptrace_get_regs(pid, regs)){
            LOGE("[+] ptrace get regs failed\n");
            break;
        }
        
        if(result != NULL){
            *result = (void*)get_return_value(regs);
        }

        ret = 0;
    }while(0);

    return ret;
}

static void* get_free_space(pid_t pid, regs_t* regs){
    void* ret = NULL;
#ifdef MMAP
    long params[6] = {0};
	params[0] = 0;	// addr
	params[1] = PAGE_SIZE; // size
	params[2] = PROT_READ | PROT_WRITE | PROT_EXEC;  // prot
	params[3] =  MAP_ANONYMOUS | MAP_PRIVATE; // flags
	params[4] = 0; //fd
	params[5] = 0; //offset

    if(0 != ptrace_invoke_syscall(pid, regs, (long)__NR_mmap2, (void**)params, 6)){
        return ret;
    }

    ptrace_get_regs(pid, regs);
    ret = (void*)get_return_value(regs);
#else
    void *remote_malloc_ptr = get_remote_addr(pid, (void*)malloc);
    long params[1] = {(long)(PAGE_SIZE)};
    if(NULL == remote_malloc_ptr){
        LOGE("[+] get remote malloc addr failed\n");
        return ret;
    }
    LOGD("[+] remote malloc addr : %lx\n", (long)remote_malloc_ptr);

    if(0 != ptrace_call_wrapper(pid, regs, remote_malloc_ptr, (void**)params, 1, &ret)){
        LOGE("[+] malloc failed\n");
        return ret;
    }
#endif
    return ret;
}

static int release_free_space(pid_t pid, regs_t* regs, void* addr){
#ifdef MMAP
    // need to unmmap
#else
    void* remote_free_ptr = get_remote_addr(pid, (void*)free);
    long params[1] = {(long)addr};
        LOGD("[+] remote free addr : %lx\n", (long)remote_free_ptr);
        if(0 != ptrace_call_wrapper(pid, regs, remote_free_ptr, (void**)params, 1, NULL)){
            LOGE("[+] free failed\n");
            return -1;
        }
#endif
    return 0;
}

static int call_bridge_entry(pid_t pid, regs_t *regs, void* handle, 
        const char* entry_name, void* free_space, void* dlsym_ptr){
    int ret = -1;
    int entry_result;
    void *param[2];
    void *entry_ptr;
    do{
        ptrace_write_data(pid, (void*)entry_name, free_space, strlen(entry_name) + 1);
        
        param[0] = handle;
        param[1] = free_space;
        if(-1 == ptrace_call_wrapper(pid, regs, dlsym_ptr, param, 2, &entry_ptr)){
            LOGE("[+] dlsym failed\n");
            break;
        }
        if(NULL == entry_ptr){
            LOGE("[+] can not find entry(%s) \n", entry_name);
            break;
        }
        LOGD("[+] dlsym succeed, get the entry address = (%lx)\n", (long)entry_ptr);
        
        if(-1 == ptrace_call_wrapper(pid, regs, entry_ptr, param, 0, (void**)&entry_result)){
            LOGE("[+] bridge entry exec failed\n");
            break;
        }
        if(0 != entry_result){
            break;
        }

        LOGD("[+] entry exec succeed, get the result = (%d)\n", (int)entry_result);
        ret = 0;
    }while(0);

    return ret;
}

static int do_ptrace_attach(pid_t pid, regs_t *regs){
    int ret = -1;
    long stop_pc;
    char* stop_module;

    do{
        //Attach process
        if(-1 == ptrace_attach(pid)){
            break;
        }

        if(-1 == wait_for_signal(pid, SIGSTOP)){
            break;
        }

        ptrace_get_regs(pid, regs);
        dump_all_regs(regs);
        
        stop_pc = get_next_ins_addr(regs);
        stop_module = get_module_name(pid, (void*)get_next_ins_addr(regs));
        LOGD("[+] attached, stop pc = %lx, module = %s [%lx]\n", stop_pc, stop_module, (long)get_module_base(pid, stop_module));

        // Attached during the syscall, possible interrupt 
        if(0 != do_with_interrupt(pid, regs)){
            break;
        }

        ret = 0; 
    }while(0);

    return ret;
}

int do_inject_process(pid_t pid, const char* path, const char* entry){
    int ret = -1;
    regs_t orig_regs = {0};
    regs_t regs = {0};
    void *remote_dlopen_ptr, *remote_dlsym_ptr, *remote_dlerror_ptr,*remote_dlclose_ptr;
    void *dlopen_handle, *space_base, *error_msg;
    void *param[2]= {0};
    char error_buf[128] = {0};
    do{
        if(pid <= 0 || NULL == path || '\0' == path[0]){
            break;
        }

        // 1. Attach to target process
        if(0 != do_ptrace_attach(pid, &regs)){
            break;
        }
        
        // 2. Backup regs 
        memcpy(&orig_regs, &regs, sizeof(regs_t));
        
        // 3. Get the function address in target process
        remote_dlopen_ptr = get_remote_addr(pid, (void*)dlopen);
        remote_dlsym_ptr = get_remote_addr(pid, (void*)dlsym);
        remote_dlerror_ptr = get_remote_addr(pid, (void*)dlerror);
        remote_dlclose_ptr = get_remote_addr(pid, (void*)dlclose);

        LOGD("[+] remote dlopen addr : %lx\n", (long)remote_dlopen_ptr);
        LOGD("[+] remote dlsym addr : %lx\n", (long)remote_dlsym_ptr);
        LOGD("[+] remote dlclose addr : %lx\n", (long)remote_dlclose_ptr);

        if(NULL == remote_dlopen_ptr || NULL == remote_dlclose_ptr 
                || NULL == remote_dlsym_ptr || NULL == remote_dlerror_ptr){
            LOGE("[+] get remote addr failed\n");
            break;
        }

        // 4. Get free space to hold strings
        space_base = get_free_space(pid, &regs); 
        if(NULL == space_base){
            LOGE("[+] get free space failed\n");
            break;
        }
        LOGD("[+] get space succeed[%lx], have free space to store path\n", (long)space_base);
        
        // 5. Write the path of libbridge.so into the malloced space
        ptrace_write_data(pid, (void*)path, space_base, strlen(path) + 1);
        
        // 6. Call dlopen to load libbridge.so
        param[0] = space_base;
        param[1] = (void*)RTLD_NOW;
        if(0 != ptrace_call_wrapper(pid, &regs, remote_dlopen_ptr, param, 2, &dlopen_handle)){
            LOGE("[+] call dlopen failed\n");
            break;
        }

        if(NULL == dlopen_handle){
            LOGE("[+] dlopen failed, try to call dlerror get error msg\n");
            if(0 != ptrace_call_wrapper(pid, &regs, remote_dlerror_ptr, param, 0, &error_msg)){
                LOGE("[+] call dlerror failed\n");
                break;
            }
            if(NULL == error_msg){
                LOGE("[+] dlerror failed\n");
                break;
            }

            if(0 != ptrace_read_data(pid, error_buf, error_msg, 128)){
                LOGD("[+] dlopen failed : %s\n", error_buf);
            }

            break;
        }
        
        LOGD("[+] dlopen succeed[%lx], the constuctor function already start\n", (long)dlopen_handle);

        // *Optional* Call entry function of bridge
        if(NULL != entry && '\0' != entry[0]){
            if(0 != call_bridge_entry(pid, &regs, dlopen_handle, entry,
                    space_base, remote_dlsym_ptr)){
                LOGE("[+] call bridge entry failed, but doesn't matter\n");
            }
        }

        // 7. Call dlclose to unload libbridge.so
        param[0] = dlopen_handle;
        if(0 != ptrace_call_wrapper(pid, &regs, remote_dlclose_ptr, param, 1, NULL)){
            LOGE("[+] dlclose failed\n");
            break;
        }
        
        LOGD("[+] dlclose succeed, then release malloced space\n");

        // 8. Call free to release the malloced space
        if(0 != release_free_space(pid, &regs, space_base)){
            LOGE("[+] release failed\n");
            break;
        }

        LOGD("[+] release succeed, continue the target\n");

        ret = 0;
    }while(0);
    
    // 9. Continue the stopped process
    dump_all_regs(&orig_regs);
    ptrace_set_regs(pid, &orig_regs);
    ptrace_detach(pid);

    return ret;
}

