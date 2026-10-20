/* Alternative source form for func_L02_003FC1B0.
   Retained from v11-b1.c, entry c3; declarations preserved.
   Naming/extraction only: this form has not been recompiled or validated
   by the cleanup. See the primary near-miss file for the recorded blocker;
   do not apply another form's verdict to this one. Recheck its file-level
   flags with tools/check.py, then tools/build.py before promotion. */
typedef unsigned char u8;
typedef signed char s8;
typedef unsigned short u16;
typedef short s16;
typedef unsigned int u32;
typedef int s32;

extern int func_00282948(void *);
extern s32 func_002A7820(s32, s32);
extern s32 func_002E5CF8(s32, s32, void *);
extern u8 *D_L12_001B20C4;
extern u8 *D_L12_001B20C4_ns __attribute__((nosda));
extern s32 D_L12_001B20C0 __attribute__((sda));

extern s32 func_002ACBC8(void *);
extern void func_002A7750(void *, s32, s32);

s32 func_L02_003FC1B0(u8 *a0, s32 a1) {
    s32 *v1 = *(s32 **)(a0 + 0x68);

    if (a1 == 0xA44)
        return 0xA44;
    if (a1 >= 0xA45) {
        if (a1 == 0xC3B)
            return ++v1[0x16];
        if (a1 >= 0xC3C) {
            if (a1 == 0xC3E)
                return ++v1[0x17];
            return 0xC3E;
        }
        if (a1 == 0xB22)
            return ++v1[0x15];
        return 0xB22;
    }
    if (a1 == 0x9CA)
        return ++v1[0x14];
    return 0x9CA;
}
