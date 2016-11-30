#ifndef _OPT_UTILS_H_
#define _OPT_UTILS_H_

#include <stdio.h>

__BEGIN_DECLS

pid_t get_target_pid();

pid_t get_current_pid();

char* get_target_name();

char* get_bridge_path();

char* get_bridge_entry();

int init_bridge(int argc, char* argv[]);


__END_DECLS

#endif //_OPT_UTILS_H_
