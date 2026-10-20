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

extern char D_00189E20[];

extern float func_00282DF0(char *arg0);

extern float func_00282E80(char *arg0, char *arg1);

extern float func_00283248(float arg0, float arg1);

extern float func_00283180(float arg0);

extern float func_00283198(float arg0);

struct S {
    char pad[52];
    unsigned short flags;
};

INCLUDE_ASM("asm/levels", func_L00_002C2DE8);

INCLUDE_ASM("asm/levels", func_L00_002C30B0);

/* Walks the resident player block: when the player is moving, has a short range and a
 * small health, the state word selects a new sub-state. The sub-state then drives a turn
 * towards a target point, which writes a velocity and a speed into the block.
 * The base is taken from the symbol again after the nested tests, and the sub-state store
 * goes through a second name for the block: the retail bytes keep the block's address in
 * $s1 and rebuild it (addiu $s0,$s1,-0x61E0) on every path that reaches the merge point,
 * and they build it a second time for the store, so both spellings are kept here. */
void func_L00_002C3838(void)
{
    char *p = D_00189E20;
    f32 v;
    f32 dist, range, ang;

    if (*(u8 *)(p + 0x287) != 0) {
        if (0.0f < *(f32 *)(p + 0x31C)) {
            if (*(f32 *)(p + 0x170) < 0.050000004f) {
                dist = func_00282DF0(p + 0xF0);
                if (dist < 0.050000004f) {
                    if (*(f32 *)(p + 0x24B8) < 0.2f) {
                        int state = *(int *)(p + 0x229C);

                        if (state == 0 || state == 2 || state == 4 || state == 10
                            || state == 1) {
                            char *q = D_00189E20;

                            *(short *)(q + 0x20A) = 5;
                        }
                    }
                }
            }
        }
    }
    p = D_00189E20;
    if (*(short *)(p + 0x20A) != 0) {
        dist = func_00282E80(p + 0x80, p + 0x240);
        range = *(f32 *)(p + 0x264) * 0.9f;
        if (dist < range) {
            f32 dx = *(f32 *)(p + 0x80) - *(f32 *)(p + 0x240);
            f32 dy = *(f32 *)(p + 0x84) - *(f32 *)(p + 0x244);

            ang = func_00283248(dx, dy);
            if (dist < *(f32 *)(p + 0x264) * 0.05f)
                ang = *(f32 *)(p + 0x98);
            v = dist - range;
            func_002AA3A8(&v, (f32 *)(p + 0x274), 0.0f, 0.03f, 0.3f, 0.046666667f);
            *(f32 *)(p + 0x100) = func_00283180(ang) * *(f32 *)(p + 0x274);
            *(f32 *)(p + 0x104) = func_00283198(ang) * *(f32 *)(p + 0x274);
            *(f32 *)(p + 0x108) = 0.0f;
        }
    } else {
        *(int *)(p + 0x274) = 0;
    }
}

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

extern unsigned char D_0018C0D4[];

extern void func_L00_002C42E0(int a0, int a1);

/* Picks the variant of the shared loader the current game state selects. */
void func_L00_002C8BA8(int a0)
{
    int a1 = (a0 != 0) ? 2 : 1;

    switch (D_0018C0D4[0]) {
    case 0:
        func_L00_002C42E0(0, a1);
        break;
    case 1:
        func_L00_002C42E0(0x3F, a1);
        break;
    }
}

extern void func_L00_002AF7D0(void);

/* Gates the level transition on the shared block: the state word must be 0, 2 or 3, the byte at
 * +0x22B8 non-zero, the counter at +0x1D8 below 6 and the timer at +0x24B8 below 0.7. */
int func_L00_002C8C00(void)
{
    char *p = D_00189E20_4;
    int state = *(int *)(p + 0x2294);

    if (state != 2 && state != 3 && state != 0)
        return 0;
    if (*(unsigned char *)(p + 0x22B8) == 0)
        return 0;
    if (*(int *)(p + 0x1D8) >= 6)
        return 0;
    if (*(float *)(p + 0x24B8) < 0.7f) {
        if (*(unsigned char *)(p + 0x22BB) != 0)
            return 0;
        func_L00_002AF7D0();
        *(int *)(p + 0xEA4) = 2;
        func_L00_002C42E0(0x22, 1);
        return 1;
    }
    return 0;
}

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

extern int func_L00_0032EF10(int, int, int *);

extern int func_L00_002C3A40(void);

extern int func_L00_0032EFB0(int, int, int, int);

extern int D_00138320[];

/* Looks the object up in the shared table and stops when the lookup fails; otherwise the state
 * machine's offset advances the object and a second lookup asks for a transition when it
 * succeeds.  A set state word, a timer past 0.9 and a state other than the idle one ask for the
 * same transition, and a zero second argument asks for it with another code. */
int func_L00_002C9FC8(int arg0, int arg1)
{
    int index;
    int r;
    char *p;

    if (func_L00_0032EF10(0x40, arg0, &index) == 0)
        return 0;
    r = func_L00_002C3A40();
    if ((D_00138320[0] & 8) && r > 0) {
        if (func_L00_0032EFB0(0x40, 0x1F000, arg0 + r, r) != 0) {
            func_L00_002C42E0(0xB, 1);
            return 1;
        }
    }
    p = D_00189E20_4;
    if (*(short *)(p + 0x1454) != 0) {
        if (0.9f < *(float *)(p + 0x24B8)) {
            if (func_L00_002C3A78() != 2) {
                func_L00_002C42E0(0xB, 1);
                return 1;
            }
        }
    }
    if (arg1 != 0)
        return 0;
    func_L00_002C42E0(7, 1);
    return 1;
}

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
