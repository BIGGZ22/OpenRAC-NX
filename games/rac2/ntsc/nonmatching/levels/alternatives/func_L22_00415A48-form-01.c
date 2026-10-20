/* Alternative source form for func_L22_00415A48.
   Retained from lvl6_12_d.c, entry func_L22_00415A48_v1; declarations preserved.
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

void func_L22_00415A48(s32 a0) {
    s32 s1 = *(s32 *)(a0 + 0x68);
    s32 s0;

    if (*(u8 *)(a0 + 0x20) == 0) {
        *(u8 *)(a0 + 0x20) = 1;
        *(u8 *)(a0 + 0x30) = 0xFF;
        *(s32 *)s1 = 0x708;
    }
    if (*(u8 *)(a0 + 0x20) != 1)
        return;
    s0 = D_L22_0018B6A8_nosda;
    if (func_2F36F0(s1) == 0)
        return;
    if (*(s32 *)(s0 + 8) != 0)
        return;
    if (*(s32 *)(s0 + 0xC) != 0)
        return;
    func_2F9E20(0x9FD, 0x54);
    *(s32 *)s1 = 0x708;
}
