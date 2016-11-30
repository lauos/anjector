#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/epoll.h>

int main(int argc, char*argv[]){
    printf("[TRACEE] Begin to %s\n", "epoll");
    
    int epfd, nfds;
    int i = 0;
    int time_out;
    struct epoll_event ev, events[5];

    epfd = epoll_create(1);
    ev.data.fd = STDIN_FILENO;
    ev.events = EPOLLIN | EPOLLET;

    epoll_ctl(epfd, EPOLL_CTL_ADD, STDIN_FILENO, &ev);

#ifdef TIME_OUT
    time_out = 10000;
#else
    time_out = -1;
#endif
    nfds = epoll_wait(epfd, events, 5, time_out);
    if(nfds < 0){
        perror("[TRACEE]");
    } else if(nfds == 0){
        printf("[TRACEE] Timeout!\n");
    } else{
        printf("[TRACEE] Ret = %d\n", nfds);
    }

    for(i = 0; i < nfds; i ++){
        if(events[i].data.fd == STDIN_FILENO){
            printf("[TRACEE] Accept\n");
        }
    }
    while(1){printf("[TRACEE] Timeout!\n");}

	return 0;
}
