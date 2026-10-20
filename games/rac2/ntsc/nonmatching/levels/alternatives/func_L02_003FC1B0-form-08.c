/* Alternative source form for func_L02_003FC1B0.
   Retained from v11-b5.c, entry c8; declarations preserved.
   Naming/extraction only: this form has not been recompiled or validated
   by the cleanup. See the primary near-miss file for the recorded blocker;
   do not apply another form's verdict to this one. Recheck its file-level
   flags with tools/check.py, then tools/build.py before promotion. */
typedef unsigned char u8;
typedef unsigned short u16;
typedef short s16;
typedef unsigned int u32;
typedef int s32;

extern s32 func_002ACBC8(s32);
extern void func_002A7750(void *, s32, s32);
extern u8 D_L12_00189EA0[];
extern u8 D_L12_001395DB;

s32 func_L02_003FC1B0(u8 *a0, s32 a1) {
    s32 *v1 = *(s32 **)(a0 + 0x68);

    if (a1 == 0xA44)
        return 0xA44;
    if (a1 < 0xA45) {
        if (a1 == 0x9CA) {
            v1[0x14]++;
            return v1[0x14];
        }
        return 0x9CA;
    }
    if (a1 == 0xC3B) {
        v1[0x16]++;
        return v1[0x16];
    }
    if (a1 < 0xC3C) {
        if (a1 == 0xB22) {
            v1[0x15]++;
            return v1[0x15];
        }
        return 0xB22;
    }
    if (a1 == 0xC3E) {
        v1[0x17]++;
        return v1[0x17];
    }
    return 0xC3E;
}
