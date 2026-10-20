/* Alternative source form for func_L14_003F0E98.
   Retained from lvl6_12_d.c, entry func_L14_003F0E98_v1; declarations preserved.
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

void func_L14_003F0E98(u8 *a0) {
    *(u64 *)(a0 + 0x80) = 0x0000FF9000000260ULL;
    *(f32 *)(a0 + 0x28) = -4.0f;
    *(f32 *)(a0 + 0x38) = 4.0f;
    *(f32 *)(a0 + 0x14) = -2.0f;
    *(f32 *)(a0 + 0x34) = 2.0f;
    *(u64 *)(a0 + 0x70) = 0;
    *(s32 *)(a0 + 0x50) = 0;
    *(s32 *)(a0 + 0x54) = 0;
    *(f32 *)(a0 + 0x58) = 1.0f;
    *(s32 *)(a0 + 0x5C) = 0;
    *(s32 *)(a0 + 0x60) = 0;
    *(f32 *)(a0 + 0x64) = 1.0f;
    *(f32 *)(a0 + 0x68) = 1.0f;
    *(f32 *)(a0 + 0x6C) = 1.0f;
    *(f32 *)(a0 + 0x08) = -4.0f;
    *(f32 *)(a0 + 0x18) = 4.0f;
    *(f32 *)(a0 + 0x04) = -2.0f;
    *(f32 *)(a0 + 0x24) = 2.0f;
    *(s32 *)(a0 + 0x00) = 0;
    *(s32 *)(a0 + 0x10) = 0;
    *(s32 *)(a0 + 0x20) = 0;
    *(s32 *)(a0 + 0x30) = 0;
    *(f32 *)(a0 + 0x0C) = 1.0f;
    *(f32 *)(a0 + 0x1C) = 1.0f;
    *(f32 *)(a0 + 0x2C) = 1.0f;
    *(f32 *)(a0 + 0x3C) = 1.0f;
}
