/* Alternative source form for func_L12_004254A8.
   Retained from v11-b4.c, entry b5; declarations preserved.
   Naming/extraction only: this form has not been recompiled or validated
   by the cleanup. See the primary near-miss file for the recorded blocker;
   do not apply another form's verdict to this one. Recheck its file-level
   flags with tools/check.py, then tools/build.py before promotion. */
typedef unsigned char u8;
typedef unsigned short u16;
typedef short s16;
typedef unsigned int u32;
typedef int s32;
typedef float f32;

extern int func_00282948(void *);
extern s32 func_002A7820(s32, s32);
extern s32 func_002E5CF8(s32, s32, void *);
extern s32 D_L12_001B20C0 __attribute__((sda));

extern s32 func_002ACBC8(s32);
extern void func_002A7750(void *, s32, s32);

void func_L12_004254A8(u8 *a0) {
    u8 *s0 = a0;

    if (*(u8 *)(s0 + 0x20) != 0)
        return;
    if (*(s32 *)*(u8 **)(s0 + 0x68) != -1) {
        if (func_002ACBC8((s32)0x00189EA0) != 0) {
            *(u8 *)0x001395DB = 1;
            func_002A7750(s0, 1, -1);
        }
    }
}
