#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main (int argc, char* argv[]){
    int i = 0;
    while (1){
        i ++;
        printf("[TRACEE] Hello %d\n", i);
        sleep(2);
    }
    return 0;
}
