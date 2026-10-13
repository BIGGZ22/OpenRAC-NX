#include "include_asm.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef short s16;
typedef int s32;

INCLUDE_ASM("asm/levels", func_L18_003F80A0);

INCLUDE_ASM("asm/levels", func_L18_003F8A98);

INCLUDE_ASM("asm/levels", func_L18_003F8C08);

extern void func_00281EB8(void *arg0, s32 arg1, u32 arg2, s32 arg3, float arg4,
                          float arg5, float arg6);
extern void func_0029FCC0(s32 arg0, s32 arg1, void *arg2);

void func_L18_003F9018(u8 *a0) {
    u8 buf[0x10];
    func_0029FCC0((s32)a0, 5, buf);
    func_00281EB8(buf, 0x2E, 0x80FFFFFFu, 1, 0.25f, 0.0f, 0.0f);
    func_00281EB8(buf, 0xC, 0x80F0F020u, 1, 0.25f, 0.0f, 0.0f);
}

INCLUDE_ASM("asm/levels", func_L18_003F90A0);

INCLUDE_ASM("asm/levels", func_L18_003FB308);

INCLUDE_ASM("asm/levels", func_L18_003FB620);

INCLUDE_ASM("asm/levels", func_L18_003FB708);

INCLUDE_ASM("asm/levels", func_L18_003FD858);

INCLUDE_ASM("asm/levels", func_L18_003FDAB8);

INCLUDE_ASM("asm/levels", func_L18_003FDD68);

INCLUDE_ASM("asm/levels", func_L18_0042EBB8);

INCLUDE_ASM("asm/levels", func_L18_0042F308);

INCLUDE_ASM("asm/levels", func_L18_0042F688);

extern void func_002A7750(u8 *arg0, s32 arg1, s32 arg2);

void func_L18_0042F768(u8 *a0) {
    s32 *p;
    if (*(u8 *)(a0 + 0x20) != 1) return;
    p = *(s32 **)(a0 + 0x24);
    *(u16 *)(a0 + 0x34) = (*(u16 *)(a0 + 0x34) & 0xFFBE) | 0x1000;
    *(s32 *)(a0 + 0x98) = p[4];
    func_002A7750(a0, 3, -1);
}

INCLUDE_ASM("asm/levels", func_L18_0042F7B8);

INCLUDE_ASM("asm/levels", func_L18_0042FAA8);

INCLUDE_ASM("asm/levels", func_L18_0042FAE0);

INCLUDE_ASM("asm/levels", func_L18_0042FBC8);

extern void func_L18_004302B0(u8 *a0);
extern void func_L18_004308F0(u8 *a0);
extern void func_L18_00430E58(u8 *a0);
extern void func_L18_00432070(u8 *a0);
extern void func_L18_00432310(u8 *a0);

void func_L18_00430248(u8 *a0) {
    u8 v = *(u8 *)(a0 + 0x20);
    if (v == 0) {
        func_L18_004302B0(a0);
        return;
    }
    if (v == 1) return;
    func_L18_004308F0(a0);
    func_L18_00430E58(a0);
    func_L18_00432070(a0);
    func_L18_00432310(a0);
}

INCLUDE_ASM("asm/levels", func_L18_004302B0);

INCLUDE_ASM("asm/levels", func_L18_004308F0);

INCLUDE_ASM("asm/levels", func_L18_00430E58);

INCLUDE_ASM("asm/levels", func_L18_00432070);

INCLUDE_ASM("asm/levels", func_L18_00432310);

extern float func_00283248(float, float);

void func_L18_00432548(u8 *a0, float *a1, s32 a2) {
    u8 *s0 = *(u8 **)(a0 + 0x68);
    *(u16 *)(s0 + 0x808) = (a2 != 0) ? 2 : 1;
    *(float *)(s0 + 0x7A4) = func_00283248(a1[0] - *(float *)(a0 + 0x10),
                                           a1[1] - *(float *)(a0 + 0x14));
}

extern float func_00282E80(u8 *arg0);

s32 func_L18_00432598(u8 *a0) {
    if (func_00282E80(a0 + 0x10) < 3.8f) return 1;
    return 0;
}

float func_L18_004325D8(u8 *a0) {
    float f = func_00282E80(a0 + 0x10);
    if (*(s16 *)(*(u8 **)(a0 + 0x68) + 0x7FC) != 0) return f - 30.0f;
    return f - 10.0f;
}

INCLUDE_ASM("asm/levels", func_L18_00432630);

extern s32 func_002761F8(void *arg0, void *arg1, s32 arg2, s32 arg3, s32 arg4);

s32 func_L18_00432698(u8 *a0, s32 a1) {
    s32 buf[4];
    func_0029FCC0(*(s32 *)(*(s32 *)(a0 + 0x68) + 0x7C8), 0, buf);
    return func_002761F8(buf, (void *)a1, 0x16, 0, 0) == 0;
}

s32 func_L18_004326E8(s32 a0) {
    u8 *v1 = (u8 *)*(s32 *)(a0 + 0x68);
    s32 r = 0;
    if (*(s16 *)(v1 + 0x7FC) != 0) {
        if (*(s16 *)(v1 + 0x804) < 0x97) goto done;
    }
    r = 1;
done:
    return r;
}

INCLUDE_ASM("asm/levels", func_L18_00432718);

INCLUDE_ASM("asm/levels", func_L18_004327B0);

extern void func_002B60B0(u8 *arg0, float, float, float, float);

void func_L18_00432910(u8 *a0) {
    func_002B60B0(a0, 0.0055555557f, 0.05333334f, 0.027925272f, 0.8377582f);
}

void func_L18_00432960(u8 *a0) {
    func_002B60B0(a0, 0.1388889f, 0.18333334f, 0.027925272f, 0.8377582f);
}

INCLUDE_ASM("asm/levels", func_L18_004329B0);

void func_L18_00432AD0(u8 *a0) {
    u8 *s0;
    if (*(u8 *)(a0 + 0x20) != 4) return;
    s0 = *(u8 **)(a0 + 0x68);
    func_002A7750(a0, 6, 1);
    func_002A7750(a0, 5, -1);
    *(u16 *)(s0 + 0x7FA) = 0;
}

INCLUDE_ASM("asm/levels", func_L18_00432B30);

void func_L18_00432D08(u8 *a0) {
    u8 *s0;
    if (*(u8 *)(a0 + 0x20) != 2) return;
    s0 = *(u8 **)(a0 + 0x68);
    func_002A7750(a0, 3, 1);
    *(u16 *)(a0 + 0x34) = *(u16 *)(a0 + 0x34) | 0x1000;
    *(u16 *)(s0 + 0x7FA) = 0;
}

INCLUDE_ASM("asm/levels", func_L18_00432D60);

INCLUDE_ASM("asm/levels", func_L18_00432E28);

INCLUDE_ASM("asm/levels", func_L18_00432E78);

INCLUDE_ASM("asm/levels", func_L18_00432F18);

INCLUDE_ASM("asm/levels", func_L18_00433088);

INCLUDE_ASM("asm/levels", func_L18_004331D0);

void func_L18_00433430(s32 a0) {
    u8 *v1 = (u8 *)*(s32 *)(a0 + 0x68);
    if (*(s16 *)(v1 + 0x7FC) != 0) return;
    *(u16 *)(v1 + 0x804) = 0;
    *(u16 *)(v1 + 0x806) = 1;
}

void func_L18_00433450(s32 a0) {
    u8 *v0 = (u8 *)*(s32 *)(a0 + 0x68);
    *(u16 *)(v0 + 0x804) = 0;
    *(u16 *)(v0 + 0x806) = 0;
}
