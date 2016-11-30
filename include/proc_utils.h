#ifndef _PROC_UTILS_
#define _PROC_UTILS_

#include <sys/types.h>

__BEGIN_DECLS

char* find_process_by_pid(pid_t pid);
pid_t find_pid_by_process(const char *process);

void* get_module_base(pid_t pid, const char* module_name);
char* get_module_name(pid_t pid, void* addr);
void* get_remote_addr(pid_t remote_pid, void* local_addr);

__END_DECLS

#endif //_PROC_UTILS_H_
