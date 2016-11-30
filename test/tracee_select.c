#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char*argv[]){
    printf("[TRACEE] Begin to %s\n", "select");
    char buf[10] = {0};
    fd_set rdfds;
    int ret;
    FD_ZERO(&rdfds);
    FD_SET(1, &rdfds);

#ifdef TIME_OUT
    struct timeval tv;
    tv.tv_sec = 10;
    tv.tv_usec = 1000;
    ret = select(1 + 1, &rdfds, NULL, NULL, &tv);
#else
    ret = select(1 + 1, &rdfds, NULL, NULL, NULL);
#endif

    if(ret < 0){
        perror("[TRACEE] ");
    } else if (ret == 0){
        printf("[TRACEE] Timeout!\n");
    } else{
        printf("[TRACEE] Ret=%d\n", ret);
    }

	return 0;
}
