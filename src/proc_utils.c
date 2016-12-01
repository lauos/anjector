/*
 * proc_utils.c
 * Copyright (C) 2016 Gavin Liu <lbliuyun@gmail.com>
 *
 * Distributed under terms of the MIT license.
 */

#include <stdio.h>
#include <stdlib.h> // For strtoul
#include <string.h>
#include <errno.h>
#include <dirent.h> // For opendir

#include "proc_utils.h"
#include "log_utils.h"

#define ADDR_LEN_MAX ((2 * 2 * sizeof(long)) + 1)

#define BY_WNAME ((void*)(-1))
#define BY_NAME ((void*)(-2))

typedef struct _module_info{
    char name[256];
    void* start;
    void* end;
} module_info;

static char* nexttok(char **strp) {
	char *tok = strsep(strp, " ");
	return tok == NULL ? "" : tok;
}

//7fb9e89cc000-7fb9e89cf000 r-xp 00000000 fe:00 23539       /lib64/libdl-2.19.so
static int find_module_info_internal(pid_t pid, struct _module_info *module, void* addr){
    int len, ret = -1;
	FILE *fp = NULL;
	char line[1024] = {0};
	char *address, *proms, *ptr, *p;

	if (pid < 0) {
		// current process
		snprintf(line, sizeof(line), "/proc/self/maps");
	} else {
		snprintf(line, sizeof(line), "/proc/%d/maps", pid);
	}

	fp = fopen(line, "r");
	if (fp != NULL) {
		while (fgets(line, sizeof(line), fp)) {
			ptr = line;
			address = nexttok(&ptr); // skip address
			proms = nexttok(&ptr); // skip proms
            nexttok(&ptr); // skip offset
            nexttok(&ptr); //skip dev
            nexttok(&ptr); //skip inode

            // skip blank
			while(*ptr != '\0') {
				if(*ptr == ' '){
					ptr++;
                }
				else{
					break;
			    }
            }

            // get path
			p = ptr;
			while(*p != '\0') {
				if(*p == '\n'){
					*p = '\0';
                }
				p++;
			}

			// 64-bit: 7fb9e89cc000-7fb9e89cf000
            // 32-bit: 4016a000-4016b000
            len = strlen(address);
			if(len <= ADDR_LEN_MAX) {
                // replace '-' by '\0'
				address[len/2] = '\0';
                
                // get start and end adress
				module->start = (void*)strtoul(address, NULL, 16);
				module->end   = (void*)strtoul(address + (len/2) + 1, NULL, 16);

				//LOGD("[%p-%p] %s | %p\n", module->start, module->end, ptr, addr);

                if(addr == BY_WNAME){
				    if(strncmp(module->name, ptr, strlen(module->name)) == 0) {
					    ret = 0;
                        break;
				    }
                } else if(addr == BY_NAME){
				    if(strstr(module->name, ptr) != NULL) {
					    ret = 0;
                        break;
                    }
                } else {
				    if(addr > module->start && addr < module->end) {
					    strcpy(module->name, ptr);
					    ret = 0;
                        break;
				    }
			    }
			}
		}
		fclose(fp);
    } else {
        LOGE("[+] open %s failed : %s\n", line, strerror(errno));
    }

	return ret;
}

static int find_module_info_by_address(pid_t pid, struct _module_info* module, void* addr) {
    return find_module_info_internal(pid, module, addr);
}

static int find_module_info_by_name(pid_t pid, struct _module_info* module) {
    return find_module_info_internal(pid, module, (void*)(-1));
}

void* get_module_base(pid_t pid, const char* module_name){
    struct _module_info module = {0};
    
    strncpy(module.name, module_name, strlen(module_name));

    if(find_module_info_by_name(pid, &module) < 0){
		LOGI("[+] find_module_info_by_name failed\n");
    }

    return module.start;
}

char* get_module_name(pid_t pid, void* addr) {
    struct _module_info module = {0};

    if(find_module_info_by_address(pid, &module, addr) < 0){
		LOGI("[+] find_module_info_by_address failed\n");
    }

    return strdup(module.name); 
}

void* get_remote_addr(pid_t remote_pid, void* local_addr){
    void* ret = NULL;
    struct _module_info current_module = {0};
    struct _module_info target_module = {0};

	if(find_module_info_by_address(-1, &current_module, local_addr) < 0) {
		LOGI("[+] find_module_info_by_address failed\n");
		return NULL;
	}

	LOGI("[+] local module is %s\n", current_module.name);

    strncpy(target_module.name, current_module.name, strlen(current_module.name));

	if(find_module_info_by_name(remote_pid, &target_module) < 0) {
		LOGI("[+] find_module_info_by_name failed \n");
		return NULL;
	}

	ret = (void *)((uintptr_t)target_module.start + (uintptr_t)local_addr - (uintptr_t)current_module.start);
    return ret;
}

char* find_process_by_pid(pid_t pid) {
	FILE* f;
	char path[255];
	char buffer[255];

	snprintf(path, sizeof(path), "/proc/%d/cmdline", pid);
	if ((f = fopen(path, "r")) == NULL) {
		return NULL;
	}

	if (fgets(buffer, sizeof(buffer), f) == NULL) {
		return NULL;
	}

	fclose(f);
	return strdup(buffer);

}

pid_t find_pid_by_process(const char *process){
	int id;
	pid_t pid = -1;
	DIR* dir;
	FILE *fp;
	char filename[32];
	char cmdline[256];
	struct dirent * entry;

    if (process == NULL){
		return -1;
    }

	dir = opendir("/proc");
    if (dir == NULL){
		return -1;
    }

	while((entry = readdir(dir)) != NULL){
		id = atoi(entry->d_name);
		if (id != 0){
			sprintf(filename, "/proc/%d/cmdline", id);
			fp = fopen(filename, "r");
			if (fp != NULL){
				fgets(cmdline, sizeof(cmdline), fp);
				fclose(fp);
				if (strncmp(process, cmdline, strlen(process)) == 0){
					pid = id;
					break;
				}
			}
		}
	}

	closedir(dir);
	return pid;
}
