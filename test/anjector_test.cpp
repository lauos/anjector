#include <stdio.h>
#include <unistd.h>
#include <libgen.h>
#include <signal.h>
#include <gtest/gtest.h>

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

// No syscall
TEST(AnjectorTest, NoSysCall) {
    int ret;
    pid_t target_pid;
    target_pid = fork();
    if(target_pid == 0){
        execl(tracee, basename(tracee), NULL);
    } else if (target_pid > 0){
        sleep(1);
        ret = do_inject_process(target_pid, bridge_path, bridge_entry);
        EXPECT_EQ(0, ret);
        ret = kill(target_pid, 0);
        EXPECT_EQ(0, ret);
        if(0 == ret){
            kill(target_pid, SIGKILL);
        }
    }
}

// Auto restart syscall through restart_syscall
TEST(AnjectorTest, Sleep) {
    int ret;
    pid_t target_pid;
    target_pid = fork();
    if(target_pid == 0){
        execl(tracee_sleep, basename(tracee_sleep), NULL);
    } else if (target_pid > 0){
        sleep(1);
        ret = do_inject_process(target_pid, bridge_path, bridge_entry);
        EXPECT_EQ(0, ret);
        ret = kill(target_pid, 0);
        EXPECT_EQ(0, ret);
        if(0 == ret){
            kill(target_pid, SIGKILL);
        }
    }
}


// Auto restart syscall
TEST(AnjectorTest, Select) {
    int ret;
    pid_t target_pid;
    target_pid = fork();
    if(target_pid == 0){
        execl(tracee_select, basename(tracee_select), NULL);
    } else if (target_pid > 0){
        sleep(1);
        ret = do_inject_process(target_pid, bridge_path, bridge_entry);
        EXPECT_EQ(0, ret);
        ret = kill(target_pid, 0);
        EXPECT_EQ(0, ret);
        if(0 == ret){
            kill(target_pid, SIGKILL);
        }
    }
}
 
// Do not restart syscall
TEST(AnjectorTest, EpollWait) {
    int ret;
    pid_t target_pid;
    target_pid = fork();
    if(target_pid == 0){
        execl(tracee_epoll, basename(tracee_epoll), NULL);
    } else if (target_pid > 0){
        sleep(1);
        ret = do_inject_process(target_pid, bridge_path, bridge_entry);
        EXPECT_EQ(0, ret);
        ret = kill(target_pid, 0);
        EXPECT_EQ(0, ret);
        if(0 == ret){
            kill(target_pid, SIGKILL);
        }
    }
}

// Auto restart syscall
TEST(AnjectorTest, SelectTimeout) {
    int ret;
    pid_t target_pid;
    target_pid = fork();
    if(target_pid == 0){
        execl(tracee_select_timeout, basename(tracee_select_timeout), NULL);
    } else if (target_pid > 0){
        sleep(1);
        ret = do_inject_process(target_pid, bridge_path, bridge_entry);
        EXPECT_EQ(0, ret);
        ret = kill(target_pid, 0);
        EXPECT_EQ(0, ret);
        if(0 == ret){
            kill(target_pid, SIGKILL);
        }
    }
}
 
// Do not restart syscall
TEST(AnjectorTest, EpollWaitTimeout) {
    int ret;
    pid_t target_pid;
    target_pid = fork();
    if(target_pid == 0){
        execl(tracee_epoll_timeout, basename(tracee_epoll_timeout), NULL);
    } else if (target_pid > 0){
        sleep(1);
        ret = do_inject_process(target_pid, bridge_path, bridge_entry);
        EXPECT_EQ(0, ret);
        ret = kill(target_pid, 0);
        EXPECT_EQ(0, ret);
        if(0 == ret){
            kill(target_pid, SIGKILL);
        }
    }
}
