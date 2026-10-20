/* Alternative source form for func_L04_00415E28.
   Retained from lvl6_12_a.c, entry func_L04_00415E28_v2; declarations preserved.
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

void func_L04_00415E28(void) {
    u8 *p = D_L04_00254E10;
    u8 *end = p + 0xB7C0;

    do {
        if (*(s32 *)(p + 0x144) != 0) {
            s32 c = *(s32 *)(p + 0x138);

            if (c >= 2)
                func_0030C948(p, 0x43, 0x44, 0x45);
        }
        p += 0x150;
    } while (p < end);
}
