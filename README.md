# Anjector 

Stand for *Android Injector*, a framework to inject a shared library into other process. 

# Usage

* -n name of target
* -p pid of target, pid or name should be set.
* -b path of shared library which to be loaded into target process
* -e entry name of shared library, anjector will call it after library was loaded.
* -h program help information

**NOTICE**
* This program must be run under root permission.
* Target process should linked with libc.so

# TODO

* Use target process's svc instruction invoke mmap directly to store constants to be used.
