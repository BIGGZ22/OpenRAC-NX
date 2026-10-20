#include "include_asm.h"

typedef int TI128 __attribute__((mode(TI)));

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef short s16;
typedef int s32;

INCLUDE_ASM("asm/levels", func_L18_003F80A0);

INCLUDE_ASM("asm/levels", func_L18_003F8A98);

extern void func_0033A0B0(u8 *arg0, u8 *arg1);
extern void func_0033DD38(u8 *arg0, u8 *arg1, s32 arg2, float arg3, float arg4);
extern void func_00340BD8(u8 *arg0, u8 *arg1, s32 arg2, s32 arg3, float arg4);
extern void func_00337920(u8 *arg0);
extern float func_00308A98();
extern u8 D_001C97C0[];

void func_L18_003F8C08(u8 *a0) {
    u8 *s0 = a0;
    u8 *s1 = *(u8 **)(s0 + 0x68);
    func_0033A0B0(a0, s1 + 0x50);
    func_0033DD38(s0, s1 + 0x680, 1, 0.029999999329447746f,
                  0.30000001192092896f);
    func_0033DD38(s0, s1 + 0x600, 0, 0.029999999329447746f,
                  0.30000001192092896f);
    func_00340BD8(s0, s1 + 0x770, 2, 1, 2.5f);
    if (*(u16 *)(s0 + 0x96) <= 0xFFFEu) {
        *(u16 *)(s0 + 0x96) = *(u16 *)(s0 + 0x96) + 1;
    }
    *(u16 *)(s1 + 0x766) = 0;
    *(u16 *)(s1 + 0x768) = 0;
    *(u16 *)(s1 + 0x76A) = 0;
    if (*(u8 *)(s0 + 0x31) != 0) {
        if (func_00308A98(s0 + 0x10, D_001C97C0) < 24.0f) {
            func_00337920(s0);
            *(u8 *)(s0 + 0x6F) = 0x16;
        }
    }
}

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

extern int *D_L18_001FE480[];

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

extern float func_00308FB0(float arg0, float arg1);
extern float func_003099E8(float arg0, float arg1);

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

s32 func_L18_00432718(u8 *a0, float *a1) {
    float f = func_00308FB0(a1[0] - *(float *)(a0 + 0x10),
                            a1[1] - *(float *)(a0 + 0x14));
    u8 *v1 = *(u8 **)(a0 + 0x68);
    if (*(s16 *)(v1 + 0x7FC) != 0) {
        return func_003099E8(*(float *)(*(u8 **)(v1 + 0x7C0) + 0xF8), f) <
               0.03490658849477768f;
    }
    return func_003099E8(*(float *)(*(u8 **)(v1 + 0x7C0) + 0xF8), f) <
           0.00872664712369442f;
}

INCLUDE_ASM("asm/levels", func_L18_004327B0);

extern void func_002B60B0(u8 *arg0, float, float, float, float);

void func_L18_00432910(u8 *a0) {
    func_002B60B0(a0, 0.0055555557f, 0.05333334f, 0.027925272f, 0.8377582f);
}

void func_L18_00432960(u8 *a0) {
    func_002B60B0(a0, 0.1388889f, 0.18333334f, 0.027925272f, 0.8377582f);
}

extern u8 D_00190000[];
extern void func_002E59F8(s32 arg0);
extern void func_002E5CF8(s32 arg0, s32 arg1, u8 *arg2);
extern void func_0029F338(void *arg0);
extern void func_0032C700(void *arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5,
                           s32 arg6, s32 arg7, float arg8, float arg9, s32 arg10, s32 arg11,
                           s32 arg12, s32 arg13);

void func_L18_004329B0(u8 *a0) {
    u8 *s0;
    s32 a1;
    s0 = *(u8 **)(a0 + 0x68);
    a1 = *(s32 *)(s0 + 0x7F0);
    if (a1 != -1) {
        u8 *p = D_00190000 - 0x79A0 + a1 * 0x70;
        if (*(u32 *)(p + 0x88) != (u32)a0) goto end;
        if (p[0x74] == 0) goto end;
        func_002E59F8(a1);
    }
end:
    *(s32 *)(s0 + 0x7F0) = -1;
    func_0032C700(a0 + 0x10, 1, 0x19, 0x14, 0x14, 0x14, 0x2D, 1, 1.0f, 20.0f, 1, 1, 0, 0);
    func_002E5CF8(6, 0, a0);
    if (*(u32 *)(s0 + 0x7C0)) {
        func_0029F338(*(void **)(s0 + 0x7C0));
        *(u32 *)(s0 + 0x7C0) = 0;
    }
    if (*(u32 *)(s0 + 0x7C4)) {
        func_0029F338(*(void **)(s0 + 0x7C4));
        *(u32 *)(s0 + 0x7C4) = 0;
    }
    if (*(u32 *)(s0 + 0x7C8)) {
        func_0029F338(*(void **)(s0 + 0x7C8));
        *(u32 *)(s0 + 0x7C8) = 0;
    }
    func_0029F338(a0);
}

void func_L18_00432AD0(u8 *a0) {
    u8 *s0;
    if (*(u8 *)(a0 + 0x20) != 4) return;
    s0 = *(u8 **)(a0 + 0x68);
    func_002A7750(a0, 6, 1);
    func_002A7750(a0, 5, -1);
    *(u16 *)(s0 + 0x7FA) = 0;
}

void func_L18_00432B30(u8 *object, int reason) {
    u8 *state;

    if (object[0x20] == 2)
        return;
    state = *(u8 **)(object + 0x68);
    *(int *)(state + 0x7EC) = reason;
    *(u16 *)(object + 0x34) = (*(u16 *)(object + 0x34) & 0xFFBE) | 0x1000;
    *(int *)(object + 0x98) = *(int *)(*(u8 **)(object + 0x24) + 0x10);
    *(u16 *)(object + 0x34) &= 0xFFFD;
    *(float *)(state + 0x7E8) = 0.5f;
    *(float *)(object + 0x2C) = *(float *)(*(u8 **)(object + 0x24) + 0x24) * 0.5f;
    if ((*(u8 **)(state + 0x7C0))) {
        *(u16 *)((*(u8 **)(state + 0x7C0)) + 0x34) &= 0xFFBE;
        *(u16 *)((*(u8 **)(state + 0x7C0)) + 0x34) |= 0x1000;
        *(int *)((*(u8 **)(state + 0x7C0)) + 0x98) = *(int *)(*(u8 **)((*(u8 **)(state + 0x7C0)) + 0x24) + 0x10);
        *(int *)((*(u8 **)(state + 0x7C0)) + 0x98) = 0;
        *(u16 *)((*(u8 **)(state + 0x7C0)) + 0x34) &= 0xFFFD;
        *(float *)((*(u8 **)(state + 0x7C0)) + 0x2C) = *(float *)(*(u8 **)((*(u8 **)(state + 0x7C0)) + 0x24) + 0x24) *
                                 *(float *)(state + 0x7E8);
    }
    if ((*(u8 **)(state + 0x7C4))) {
        *(u16 *)((*(u8 **)(state + 0x7C4)) + 0x34) &= 0xFFBE;
        *(u16 *)((*(u8 **)(state + 0x7C4)) + 0x34) |= 0x1000;
        *(int *)((*(u8 **)(state + 0x7C4)) + 0x98) = *(int *)(*(u8 **)((*(u8 **)(state + 0x7C4)) + 0x24) + 0x10);
        *(int *)((*(u8 **)(state + 0x7C4)) + 0x98) = 0;
        *(u16 *)((*(u8 **)(state + 0x7C4)) + 0x34) &= 0xFFFD;
        *(float *)((*(u8 **)(state + 0x7C4)) + 0x2C) = *(float *)(*(u8 **)((*(u8 **)(state + 0x7C4)) + 0x24) + 0x24) *
                                 *(float *)(state + 0x7E8);
    }
    if ((*(u8 **)(state + 0x7C8))) {
        *(u16 *)((*(u8 **)(state + 0x7C8)) + 0x34) &= 0xFFBE;
        *(u16 *)((*(u8 **)(state + 0x7C8)) + 0x34) |= 0x1000;
        *(int *)((*(u8 **)(state + 0x7C8)) + 0x98) = *(int *)(*(u8 **)((*(u8 **)(state + 0x7C8)) + 0x24) + 0x10);
        *(int *)((*(u8 **)(state + 0x7C8)) + 0x98) = 0;
        *(u16 *)((*(u8 **)(state + 0x7C8)) + 0x34) &= 0xFFFD;
        *(float *)((*(u8 **)(state + 0x7C8)) + 0x2C) = *(float *)(*(u8 **)((*(u8 **)(state + 0x7C8)) + 0x24) + 0x24) *
                                 *(float *)(state + 0x7E8);
    }
    func_002A7750(object, 2, 1);
    *(u16 *)(object + 0x34) &= 0xEFFF;
    *(short *)(state + 0x7FA) = 0;
}


void func_L18_00432D08(u8 *a0) {
    u8 *s0;
    if (*(u8 *)(a0 + 0x20) != 2) return;
    s0 = *(u8 **)(a0 + 0x68);
    func_002A7750(a0, 3, 1);
    *(u16 *)(a0 + 0x34) = *(u16 *)(a0 + 0x34) | 0x1000;
    *(u16 *)(s0 + 0x7FA) = 0;
}

INCLUDE_ASM("asm/levels", func_L18_00432D60);

s32 func_L18_00432E28(u8 *a0) {
    s32 v = *(u8 *)(a0 + 0x20);
    s32 r = 0;
    if ((v & 0xFF) == 8) goto one;
    if ((v & 0xFF) == 13) goto one;
    if ((u32)(v - 9) >= 4) goto done;
    if (*(u8 *)(a0 + 0x94) != 8) goto done;
one:
    r = 1;
done:
    return r;
}

extern void func_00335388(u8 *arg0, s32 arg1, s32 arg2, s32 arg3);

void func_L18_00432E78(u8 *a0, s32 a1, s32 a2, s32 a3) {
    u8 *s3 = *(u8 **)(a0 + 0x68);
    if (*(u8 *)(a0 + 0x43) != a1) {
        func_00335388(a0, a1, a2, a3);
    }
    if (*(u8 **)(s3 + 0x7C0) != 0) {
        if (*(u8 *)(*(u8 **)(s3 + 0x7C0) + 0x43) != a1) {
            func_00335388(*(u8 **)(s3 + 0x7C0), a1, a2, a3);
        }
    }
    if (*(u8 **)(s3 + 0x7C4) != 0) {
        if (*(u8 *)(*(u8 **)(s3 + 0x7C4) + 0x43) != a1) {
            func_00335388(*(u8 **)(s3 + 0x7C4), a1, a2, a3);
        }
    }
}

extern void func_L18_00433020(u8 *a0);
extern void func_L18_00432FB8(u8 *a0);
extern void func_L18_00433088(u8 *a0, s32 a1);
extern void func_L18_00433168(u8 *a0, s32 a1);

void func_L18_00432F18(u8 *a0) {
    u8 *s1 = *(u8 **)(a0 + 0x68);
    if (*(u8 **)(s1 + 0x7C0) != 0) {
        func_L18_00433020(a0);
        if ((*(u16 *)(*(u8 **)(s1 + 0x7C0) + 0x7C) & 2) != 0) {
            func_L18_00432FB8(a0);
        }
    }
    if (*(u8 **)(s1 + 0x7C4) == 0) return;
    func_L18_00433088(a0, 0);
    func_L18_00433088(a0, 1);
    func_L18_00433168(a0, 2);
    func_L18_00433168(a0, 3);
    func_L18_00433168(a0, 4);
}

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
