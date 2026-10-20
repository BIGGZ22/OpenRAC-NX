/* Alternative source form for func_L04_0040ACC8.
   Retained from lvl6_12_d.c, entry func_L04_0040ACC8_v1; declarations preserved.
   Naming/extraction only: this form has not been recompiled or validated
   by the cleanup. See the primary near-miss file for the recorded blocker;
   do not apply another form's verdict to this one. Recheck its file-level
   flags with tools/check.py, then tools/build.py before promotion. */
typedef int s32;
typedef unsigned int u32;
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned long long u64;
typedef float f32;

extern u8 *D_001B1F9C;

extern s32 func_2EE138(void *, void *, void *);
extern void func_2EE5B0(void *, void *, void *);
extern f32 func_2EE268(void *);

extern f32 func_2F36F0(s32);
extern void func_2F9E20(s32, s32);

extern s32 D_L22_0018B6A8_nosda __asm__("D_L22_0018B6A8") __attribute__((nosda));

extern s32 D_L04_001B0914_gp __asm__("D_L04_001B0914") __attribute__((section(".sdata")));
extern s32 D_L04_001B092C_gp __asm__("D_L04_001B092C") __attribute__((section(".sdata")));
extern s32 D_L04_001B0934_gp __asm__("D_L04_001B0934") __attribute__((section(".sdata")));
extern s32 D_L04_001B0938_gp __asm__("D_L04_001B0938") __attribute__((section(".sdata")));

void func_L04_0040ACC8(void) {
    char *v1 = (char *)D_L04_001B0914_gp;
    char *o = *(char **)(v1 + 0x68);

    if (*(s32 *)(o + 0x744) < 5) {
        s32 v = *(s32 *)(o + 0x724);

        D_L04_001B092C_gp = 1;
        if (v != 0) {
            D_L04_001B0938_gp = D_L04_001B0938_gp + 1;
            if (D_L04_001B0938_gp == 3)
                D_L04_001B0934_gp = 1;
        }
    } else if (D_L04_001B0938_gp < 4) {
        D_L04_001B0938_gp = 0;
    }
    *(s32 *)(o + 0x744) = 0;
    *(s32 *)(o + 0x724) = 0;
}
