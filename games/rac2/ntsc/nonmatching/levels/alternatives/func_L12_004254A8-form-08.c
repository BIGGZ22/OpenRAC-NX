/* Alternative source form for func_L12_004254A8.
   Retained from v11-b6.c, entry b8; declarations preserved.
   Naming/extraction only: this form has not been recompiled or validated
   by the cleanup. See the primary near-miss file for the recorded blocker;
   do not apply another form's verdict to this one. Recheck its file-level
   flags with tools/check.py, then tools/build.py before promotion. */
typedef unsigned char u8;
typedef unsigned int u32;
typedef int s32;

extern s32 func_002ACBC8(s32);
extern void func_002A7750(void *, s32, s32);
extern u8 D_L12_00189EA0[];
extern u8 D_L12_001395DB;

void func_L12_004254A8(u8 *a0) {
    s32 *p;

    if (*(u8 *)(a0 + 0x20) != 0)
        return;
    p = *(s32 **)(a0 + 0x68);
    if (p[0] == -1)
        return;
    if (func_002ACBC8((s32)D_L12_00189EA0) == 0)
        return;
    D_L12_001395DB = 1;
    func_002A7750(a0, 1, -1);
}
