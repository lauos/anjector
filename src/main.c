/*
 * main.c
 * Copyright (C) 2016 Gavin Liu <lbliuyun@gmail.com>
 *
 * Distributed under terms of the MIT license.
 */

#include <stdio.h>
#include <stdlib.h>

#include "anjector.h"
#include "opt_utils.h"
#include "log_utils.h"
#include "proc_utils.h"

int main(int argc, char* argv[]){
    pid_t current_pid, target_pid;
    char *current_name, *target_name, *entry_name, *bridge_path;

    if(0 != init_bridge(argc, argv)){
        LOGE("[+] init bridge failed!\n");
        return 0;
    }

    current_pid = get_current_pid();
    target_pid = get_target_pid();
    target_name = get_target_name();
    entry_name = get_bridge_entry();
    bridge_path = get_bridge_path();
    current_name = find_process_by_pid(current_pid);

    if(0 != do_inject_process(target_pid, bridge_path, entry_name)){
        LOGE("[+] [%s](%d) inject [%s] into process [%s](%d) failed!\n",
                current_name, current_pid,bridge_path, target_name, target_pid);
    } else {
        LOGE("[+] [%s](%d) inject [%s] into process [%s](%d) succeed!\n",
                current_name, current_pid,bridge_path, target_name, target_pid);
    }

    if(NULL != current_name){
        free(current_name); current_name = NULL;
    }

    return 0;
}
