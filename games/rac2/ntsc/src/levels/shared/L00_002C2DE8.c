#include "include_asm.h"

extern void func_L00_002B39F0(int arg0, float arg1);

extern float func_L00_002B9B38(char *arg0, int arg1);

extern float func_00283BE0(float arg0, float arg1);

extern float func_00283C80(float arg0, float arg1);

extern char D_00189E20_4[];

typedef unsigned char u8;

typedef int s32;

typedef float f32;

extern u8 D_00189E20_7[];

extern void func_002AA3A8(f32 *a, f32 *b, f32 c, f32 d, f32 e, f32 f);

extern f32 func_00282C48_1(f32 v);

extern void func_002AA2E8(f32 *p, f32 v, f32 w);

#ifndef RAC2_T_RES_F458C670E
#define RAC2_T_RES_F458C670E
typedef struct {
    u8 pad000[0x88];
    f32 f088;
    u8 pad08C[0x1B8 - 0x08C];
    s32 i1B8;
    u8 pad1BC[0x330 - 0x1BC];
    f32 f330;
    u8 pad334[0x790 - 0x334];
    f32 f790;
    f32 f794;
    u8 pad798[4];
    f32 f79C;
    f32 f7A0;
    u8 pad7A4[0x9B0 - 0x7A4];
    f32 f9B0;
    u8 pad9B4[0xA3C - 0x9B4];
    f32 fA3C;
    u8 padA40[0x2294 - 0xA40];
    s32 i2294;
    u8 pad2298[4];
    s32 i229C;
    u8 pad22A0[4];
    s32 i22A4;
    u8 pad22A8[8];
    s32 i22B0;
} Res_F458c670e;
#endif

extern char D_00189E20_29[];

extern void func_0029F338_5(int);

struct S {
    char pad[52];
    unsigned short flags;
};

INCLUDE_ASM("asm/levels", func_L00_002C2DE8);

INCLUDE_ASM("asm/levels", func_L00_002C30B0);

INCLUDE_ASM("asm/levels", func_L00_002C3838);

INCLUDE_ASM("asm/levels", func_L00_002C3A40);

/* reads a 16-bit state word out of a fixed global block and, when it is non-zero, maps a second byte of that block onto a return code 0/1/2/3; otherwise it runs a reset call, builds a float either from a query on a sub-object of the block or from two floats stored in the block, and classifies it against a stored angle */
int func_L00_002C3A78(void)
{
    char *p = D_00189E20_4;
    float value;
    int result;
    int state;

    result = 2;
    if (*(short *)(p + 5204) != 0) {
        state = *(unsigned char *)(p + 5207);
        if (state == 0)
            result = 0;
        else if (state == 3)
            result = 3;
        else if (state == 2)
            result = 2;
        else
            result = 1;
    } else {
        func_L00_002B39F0(0, 1.0f);
        if (*(unsigned char *)(p + 8899) != 0)
            value = func_L00_002B9B38(p + 400, 0);
        else
            value = func_00283BE0(*(float *)(p + 416), *(float *)(p + 152));
        if (func_00283C80(value, 3.1415929794311523437500e+00f) < 7.8539824485778808593750e-01f)
            result = 3;
        else if (func_00283C80(value, -1.5707964897155761718750e+00f) < 7.8539824485778808593750e-01f)
            result = 1;
        else if (func_00283C80(value, 1.5707964897155761718750e+00f) < 7.8539824485778808593750e-01f)
            result = 0;
    }
    return result;
}

INCLUDE_ASM("asm/levels", func_L00_002C3BB0);

INCLUDE_ASM("asm/levels", func_L00_002C3E18);

INCLUDE_ASM("asm/levels", func_L00_002C40C0);

INCLUDE_ASM("asm/levels", func_L00_002C42E0);

INCLUDE_ASM("asm/levels", func_L00_002C8BA8);

INCLUDE_ASM("asm/levels", func_L00_002C8C00);

INCLUDE_ASM("asm/levels", func_L00_002C8CB0);

INCLUDE_ASM("asm/levels", func_L00_002C8E10);

/* Drives one effect state: branches on several resource fields, decays and integrates two float components through three float helpers, and writes back the summed value. */
void func_L00_002C9228(void)
{
    f32 old, v, r, sum;

    if (((Res_F458c670e *)D_00189E20_7)->i1B8 >= 11
        || ((Res_F458c670e *)D_00189E20_7)->f790 < 0.0f
        || ((Res_F458c670e *)D_00189E20_7)->i22A4 == 17
        || ((Res_F458c670e *)D_00189E20_7)->i22B0 == 17) {
        if (((Res_F458c670e *)D_00189E20_7)->i2294 == 53) {
            func_002AA3A8(&((Res_F458c670e *)D_00189E20_7)->f790,
                    &((Res_F458c670e *)D_00189E20_7)->fA3C,
                    0.0f, 0.03f, 0.3f, 0.025000002f);
            ((Res_F458c670e *)D_00189E20_7)->f794 = 0.0f;
        } else {
            old = ((Res_F458c670e *)D_00189E20_7)->f790;
            v = ((Res_F458c670e *)D_00189E20_7)->f794;
            ((Res_F458c670e *)D_00189E20_7)->f794 =
                v - (old * 0.005f + v * 0.045f);
            r = func_00282C48_1(old);
            if (r < 0.001f) {
                if (((Res_F458c670e *)D_00189E20_7)->f794 < 0.0001f)
                    ((Res_F458c670e *)D_00189E20_7)->f794 = 0.0f;
            }
        }
    }
    sum = ((Res_F458c670e *)D_00189E20_7)->f790
        + ((Res_F458c670e *)D_00189E20_7)->f794;
    ((Res_F458c670e *)D_00189E20_7)->f790 = sum;
    if (((Res_F458c670e *)D_00189E20_7)->i2294 == 53) {
        if (sum > 0.0f) {
            if (((Res_F458c670e *)D_00189E20_7)->f794 > 0.0f)
                ((Res_F458c670e *)D_00189E20_7)->f794 = 0.0f;
            func_002AA2E8(&((Res_F458c670e *)D_00189E20_7)->f790,
                    0.0f, 0.011666667f);
        }
    }
    if (((Res_F458c670e *)D_00189E20_7)->i2294 != 124) {
        f32 x = ((Res_F458c670e *)D_00189E20_7)->f330;
        if (((Res_F458c670e *)D_00189E20_7)->i229C == 22)
            x = ((Res_F458c670e *)D_00189E20_7)->f9B0;
        func_002AA3A8(&((Res_F458c670e *)D_00189E20_7)->f79C,
                &((Res_F458c670e *)D_00189E20_7)->f7A0,
                x, 0.027f, 0.3f, 0.0f);
    }
    ((Res_F458c670e *)D_00189E20_7)->f088 =
        (((Res_F458c670e *)D_00189E20_7)->f79C + (-0.12f))
        + ((Res_F458c670e *)D_00189E20_7)->f790;
}

INCLUDE_ASM("asm/levels", func_L00_002C9448);

INCLUDE_ASM("asm/levels", func_L00_002C9FC8);

INCLUDE_ASM("asm/levels", func_L00_002CA0B0);

/* On a byte-addressable root base: if the word at +6228 is set, calls a helper with it and clears it, then clears bits 0 and 6 of the 16-bit field at +52 of each object pointed to by the base words at +4880 and +4884. */
void func_L00_002CA478(void)
{
    char *g = D_00189E20_29;
    int v;
    struct S *p;
    struct S *q;

    v = *(int *)(g + 6228);
    if (v != 0) {
        func_0029F338_5(v);
        *(int *)(g + 6228) = 0;
    }
    p = *(struct S **)(g + 4880);
    if (p != 0)
        p->flags &= ~0x41;
    q = *(struct S **)(g + 4884);
    if (q != 0)
        q->flags &= ~0x41;
}

INCLUDE_ASM("asm/levels", func_L00_002CA4E0);

INCLUDE_ASM("asm/levels", func_L00_002CA898);

INCLUDE_ASM("asm/levels", func_L00_002CAB08);
