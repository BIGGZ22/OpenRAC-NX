/* Alternative source form for func_L17_003E4860.
   Retained from lvl6_12_a.c, entry func_L17_003E4860_v1; declarations preserved.
   Naming/extraction only: this form has not been recompiled or validated
   by the cleanup. See the primary near-miss file for the recorded blocker;
   do not apply another form's verdict to this one. Recheck its file-level
   flags with tools/check.py, then tools/build.py before promotion. */
typedef int s32;
typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;
typedef short s16;
typedef char s8;
typedef float f32;

extern s32 D_L17_001C0018;
extern s32 func_2DF570(s32, s32, s32, s32, s32);

extern u8 D_L04_00254E10[];
extern void func_0030C948(u8 *a0, s32 a1, s32 a2, s32 a3);

extern s16 *D_L17_001FB720[];
extern s32 D_L17_001B2C1C __attribute__((sda));

extern s32 func_321F48(s32, s32, s32);
extern u8 *func_31FC88(s32);
extern void func_362DE0(u8 *, s32, s32, s32);
extern void func_325308(s32, s32, s32, f32, f32);
extern void func_3159F0(s32);

extern s32 func_312898(s32 a0, s32 a1, void *a2);
extern void func_2EEFE0(char *a0, char *a1, void *a2);
extern void func_31B920(char *a0, s32 a1, s32 a2);

extern int func_L22_003A48F8(int);

void func_L17_003E4860(s32 a0) {
    s32 i = *(u8 *)(a0 + 0x21);
    s16 *p;
    u8 *base;

    if (i == 0xFF)
        return;
    p = D_L17_001FB720[i];
    if (p == 0)
        return;
    base = (u8 *)D_L17_001B2C1C;
    do {
        u8 *q = base + ((*(u16 *)p & 0x7FFF) << 8);

        if (*(s8 *)(q + 0x20) >= 0 && *(s16 *)(q + 0xAA) == 0xB88)
            *(s16 *)(*(s32 *)(q + 0x68) + 0x406) = 1;
        p++;
    } while (*p >= 0);
}
