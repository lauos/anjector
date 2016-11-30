#ifndef _ANJECTOR_H_
#define _ANJECTOR_H

__BEGIN_DECLS

int do_inject_process(pid_t pid, const char* path, const char* entry);

__END_DECLS

#endif
