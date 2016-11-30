#include <stdio.h>

#include "log_utils.h"


// Linker call these
void _init(){
    LOGD("[1] Bridge _init!\n");
}

void __attribute__((constructor)) bridge_ctor(){
    LOGD("[2] Bridge constructor!\n");
}

// Injector use dlsym to call this
int bridge_entry(){
    LOGD("[3] Bridge enter!\n");
    return 0;
}

