#include <stdio.h>
#include <unistd.h>
#include <libgen.h>
#include <signal.h>

#include "anjector.h"
#include "proc_utils.h"
#include "log_utils.h"

const char *tmp_path = "/data/local/tmp";
const char *bridge_path = "/data/local/tmp/libbridge.so";
const char *bridge_entry = "bridge_entry";
const char *tracee = "/data/local/tmp/tracee";
const char *tracee_sleep = "/data/local/tmp/tracee_sleep";
const char *tracee_select = "/data/local/tmp/tracee_select";
const char *tracee_epoll = "/data/local/tmp/tracee_epoll";
const char *tracee_select_timeout = "/data/local/tmp/tracee_select_timeout";
const char *tracee_epoll_timeout = "/data/local/tmp/tracee_epoll_timeout";

int main (int argc, char* argv[]){
    pid_t target_pid;
    target_pid = fork();
    if(target_pid == 0){
        execl(tracee, basename(tracee), NULL);
    } else if (target_pid > 0){
        sleep(1);
        do_inject_process(target_pid, bridge_path, bridge_entry);
    }
    return 0;
}
