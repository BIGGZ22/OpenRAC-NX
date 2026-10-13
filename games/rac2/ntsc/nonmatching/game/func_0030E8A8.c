#include "include_asm.h"

INCLUDE_ASM("asm/game", func_00308600);

void func_00308768(void) { }

void func_00308DB8(void) { }

void func_003092A0(void) { }

void func_00309718(void) { }

INCLUDE_ASM("asm/game", func_0030A188);

void func_0030A348(void) { }

INCLUDE_ASM("asm/game", func_0030BA38);

void func_0030BBA0(void) { }

INCLUDE_ASM("asm/game", func_0030C188);

void func_0030C440(void) { }

INCLUDE_ASM("asm/game", func_0030CAA0);

INCLUDE_ASM("asm/game", func_0030CAD0);

void func_0030CCD0(void) { }

INCLUDE_ASM("asm/game", func_0030D430);

void func_0030D5B8(void) { }

INCLUDE_ASM("asm/game", func_0030DB88);

INCLUDE_ASM("asm/game", func_0030DE88);

void func_0030E030(void) { }

INCLUDE_ASM("asm/game", func_0030E6F8);

INCLUDE_ASM("asm/game", func_0030E700);

typedef int TI128 __attribute__((mode(TI)));

extern char D_00189E20_1[];
extern char D_001BAFA0_1[];

extern void func_002FE8D8(char *, char *);
extern void func_00282CF0(char *, char *, char *);
extern void func_00282CC0(char *, char *, char *);
extern void func_00282D30(char *, char *, float);
extern float func_00282DB0(char *, char *);
extern float func_00282DF0(char *);
extern void func_00282F20(char *, char *, float);
extern int func_002761F8(char *, char *, int, int, int);
extern void func_002AA3A8(float *, char *, float, float, float, float);
extern void func_002A8240(char *, char *, int, float, float, float);

void func_0030E8A8(char *a0)
{
    char *obj = a0;
    char *e = *(char **)(obj + 0x68);
    char *d = D_00189E20_1;
    short flag = *(short *)(d + 0x202);
    float vec[10][4];
    float f;
    int mode;
    float l3;
    float l2;
    float step;
    float acc;
    float x;
    int r;

    if (*(int *)(e + 0x44) == 0 && flag == 0)
        return;

    func_002FE8D8(obj + 0x10, (char *)vec);
    acc = 0.0f;
    func_00282CF0((char *)vec[1], obj + 0x10, d + 0x80);
    func_00282D30((char *)vec[3], (char *)vec, func_00282DB0((char *)vec[1], (char *)vec));
    func_00282CF0((char *)vec[2], (char *)vec[1], (char *)vec[3]);
    l3 = func_00282DF0((char *)vec[3]);
    l2 = func_00282DF0((char *)vec[2]);
    func_00282D30((char *)vec[4], (char *)vec, -2.5f);
    func_00282CC0((char *)vec[4], obj + 0x10, (char *)vec[4]);
    func_00282D30((char *)vec[5], (char *)vec, 0.5f);
    func_00282CC0((char *)vec[5], obj + 0x10, (char *)vec[5]);
    r = func_002761F8((char *)vec[5], (char *)vec[4], 2, 0, 0);
    if (r != 0) {
        *(TI128 *)((char *)vec[6]) = *(TI128 *)D_001BAFA0_1;
        mode = 0;
    } else {
        mode = 1;
    }
    if (mode == 0) {
        func_00282CF0((char *)vec[8], (char *)vec[6], obj + 0x10);
        func_00282D30((char *)vec[7], (char *)vec, func_00282DB0((char *)vec[8], (char *)vec));
        acc = func_00282DF0((char *)vec[7]);
    }
    if (*(int *)(e + 0x44) == 0) {
        if (!(l2 < 1.0f))
            return;
        if (!(l3 < 1.3f))
            return;
        *(int *)(e + 0x44) = 1;
    }
    *(TI128 *)((char *)vec[8]) = *(TI128 *)((char *)vec[2]);
    if (l2 - 1.4f > 0.0f)
        f = 0.0f;
    else
        f = l2 - 1.4f;
    if (flag == 0)
        f = 0.0f;
    step = 0.02f;
    func_002AA3A8(&f, e + 0x48, 0.0f, step, 0.3f, 0.066666666f);
    if (f == 0.0f && *(float *)(e + 0x48) == 0.0f && acc < step)
        *(int *)(e + 0x44) = 0;
    if (l2 < *(float *)(e + 0x48))
        *(float *)(e + 0x48) = l2;
    if (*(float *)(e + 0x48) < l2)
        func_00282F20((char *)vec[8], (char *)vec[8], *(float *)(e + 0x48));
    func_002A8240(obj, (char *)vec[8], 0, 0.5f, 0.4f, 0.0f);
    if (mode == 0) {
        if (func_00282DB0((char *)vec[7], (char *)vec) > 0.0f) {
            *(TI128 *)(obj + 0x10) = *(TI128 *)((char *)vec[6]);
            *(float *)(e + 0x4C) = 0.0f;
        } else {
            x = *(float *)(e + 0x4C) - 0.0066666668f;
            *(float *)(e + 0x4C) = x;
            func_00282D30((char *)vec[8], (char *)vec, x);
            func_00282CC0(obj + 0x10, (char *)vec[8], obj + 0x10);
            func_00282CF0((char *)vec[9], (char *)vec[6], obj + 0x10);
            if (func_00282DB0((char *)vec[9], (char *)vec) > 0.0f) {
                *(TI128 *)(obj + 0x10) = *(TI128 *)((char *)vec[6]);
                *(float *)(e + 0x4C) = 0.0f;
            }
        }
    } else {
        x = *(float *)(e + 0x4C) - 0.0066666668f;
        *(float *)(e + 0x4C) = x;
        func_00282D30((char *)vec[8], (char *)vec, x);
        func_00282CC0(obj + 0x10, (char *)vec[8], obj + 0x10);
    }
}

void func_0030ECC8(void) { }

INCLUDE_ASM("asm/game", func_0030F408);
