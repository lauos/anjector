# Register

1. Header File : 
`$NDK_HOME/platforms/android-24/arch-arm64/usr/include/asm/ptrace.h`

2. Structure Define:

```
struct user_pt_regs {
 __u64 regs[31];
 __u64 sp;
 __u64 pc;
 __u64 pstate;
};
```
