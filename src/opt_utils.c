#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "opt_utils.h"
#include "proc_utils.h"
#include "log_utils.h"

#define OPTIONS "hn:p:b:e:"

typedef struct _inject_bridge{
    char name[256];
    char path[256];
    char entry[256];
    pid_t pid;
    pid_t ppid;
} inject_bridge;

static struct _inject_bridge bridge;

pid_t get_target_pid(){
    return bridge.pid;
}

pid_t get_current_pid(){
    return bridge.ppid;
}

char* get_target_name(){
    return bridge.name;
}

char* get_bridge_path(){
    return bridge.path;
}

char* get_bridge_entry(){
    return bridge.entry;
}

void display_usage(){
    LOGD("Usage:\n");
    LOGD("  -n name_of_process\n");
    LOGD("  -p pid_of_process\n");
    LOGD("  -b path_to_bridge\n");
    LOGD("  -e entry_of_bridge\n");
    LOGD("  -h\n");
}

int init_bridge(int argc, char* argv[]){
    int opt;
    while((opt = getopt(argc, argv, OPTIONS)) != -1){
        //LOGD("next option=[%d](%s)\n", optind, argv[optind]);
        switch(opt){
            case 'n':
                LOGD("[-] option n : %s\n", optarg);
                strncpy(bridge.name, optarg, strlen(optarg));
                break;
            case 'p':
                LOGD("[-] option p : %s\n", optarg);
                bridge.pid = atoi(optarg);
                break;
            case 'b':
                LOGD("[-] option b : %s\n", optarg);
                strncpy(bridge.path, optarg, strlen(optarg));
                break;
            case 'e':
                LOGD("[-] option e : %s\n", optarg);
                strncpy(bridge.entry, optarg, strlen(optarg));
                break;
            case ':':
                LOGD("[-] valid option %c\n", optopt);
                break;
            case 'h':
            case '?':
                display_usage();
                break;
            default :
                break;
        }
    }

    if('\0' == bridge.path[0]){
        display_usage();
        return -1;
    } else if('\0' == bridge.name[0]){
        if(0 == bridge.pid){
            display_usage();
            return -1;
        } else {
            const char* process = NULL;
            process = find_process_by_pid(bridge.pid);
            strncpy(bridge.name, process, strlen(process));
        }
    } else {
        if(0 == bridge.pid){
            bridge.pid = find_pid_by_process(bridge.name);
        }
    }

    bridge.ppid = getpid();

    LOGD("[-] ppid=%d pid=%d name=%s path=%s func=%s\n", bridge.ppid, bridge.pid, bridge.name, bridge.path, bridge.entry);

    if (access(bridge.path, R_OK) < 0) {
        LOGE("[-] %s is not readable\n", bridge.path);
        return -1;
    }

    return 0;
}
