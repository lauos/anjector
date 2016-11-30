# Register

1. Header File : 
`$NDK_HOME/platforms/android-24/arch-x86/usr/include/asm/ptrace.h`

2. Structure Define:

```
struct pt_regs {
 long ebx;
 long ecx;
 long edx;
 long esi;
 long edi;
 long ebp;
 long eax;
 int xds;
 int xes;
 int xfs;
 int xgs;
 long orig_eax;
 long eip;
 int xcs;
 long eflags;
 long esp;
 int xss;
};
```
