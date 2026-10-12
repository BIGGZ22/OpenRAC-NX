#include "include_asm.h"
#include "common.h"

typedef int TI128 __attribute__((mode(TI)));

extern void func_002A7750(char *arg, int mode, int value);

extern void func_L00_003FF7B8_1(char *arg);

extern void func_L00_003FF890(char *arg);

extern float func_00282E80_3(char *arg, char *table);

extern float func_00282C48_10(float value);

extern void func_002B0A18(int arg, int value);

extern void func_00288308(int arg);

extern void func_002AD860(int arg);

extern void func_0029F338_4(char *arg);

extern char D_00189EA0_2[];

extern unsigned char D_001A7B2E __attribute__((sda));

/* two-state update on a record: state 0 marks a byte with 255 and either runs a shared two-call teardown when a global latch byte is set and a sub-object field is non-zero, or plays a mode/value pair plus a call; state 1 runs a setup call and two float queries against a stored table and, when both are below 2.0, plays three fixed calls and either tears down or writes 120 into a sub-object field */
void func_L00_003FFA70(char *arg)
{
    int *object = *(int **)(arg + 104);
    unsigned char state = *(unsigned char *)(arg + 32);

    switch (state) {
    case 0:
        *(unsigned char *)(arg + 48) = 255;
        if (D_001A7B2E != 0 && object[17] != 0) {
            func_0029F338_4((char *)object[17]);
            func_0029F338_4(arg);
            return;
        } else {
            func_002A7750(arg, 1, -1);
            func_L00_003FF7B8_1(arg);
        }
        break;
    case 1:
        func_L00_003FF890(arg);
        if (func_00282E80_3(arg + 16, D_00189EA0_2) < 2.0f) {
            if (func_00282C48_10(*(float *)(arg + 24) - *(float *)(D_00189EA0_2 + 8)) < 2.0f) {
                func_002B0A18(4658, -1);
                func_00288308(54);
                func_002AD860(54);
                if (object[17] != 0) {
                    func_0029F338_4((char *)object[17]);
                    func_0029F338_4(arg);
                    return;
                }
                object[16] = 120;
            }
        }
        break;
    }
}

INCLUDE_ASM("asm/levels", func_L00_003FFBA0);

INCLUDE_ASM("asm/levels", func_L00_004003F0);

INCLUDE_ASM("asm/levels", func_L00_004005B0);

INCLUDE_ASM("asm/levels", func_L00_00400F30);


INCLUDE_ASM("asm/levels", func_L00_00401168);

INCLUDE_ASM("asm/levels", func_L00_00401668);

INCLUDE_ASM("asm/levels", func_L00_00401D50);

INCLUDE_ASM("asm/levels", func_L00_00402068);

INCLUDE_ASM("asm/levels", func_L00_004029E8);


extern void func_002FDBD8(void *obj, float f, void *p, void *q);
typedef struct __attribute__((packed)) { unsigned char b[8]; } Blob8;
extern Blob8 D_L00_001B0DB8 __attribute__((nosda));

void func_L00_00402DD8(char *arg)
{
    int *o = *(int **)(arg + 104);
    if (o[9] != 0) {
        if (o[11] == 0) {
            func_002FDBD8(o, 1.0f, (char *)o + 16, 0);
        } else {
            Blob8 tmp = D_L00_001B0DB8;
            func_002FDBD8(o, 1.0f, (char *)o + 16, &tmp);
        }
    }
}

INCLUDE_ASM("asm/levels", func_L00_00402E50);

INCLUDE_ASM("asm/levels", func_L00_00403730);

extern int D_L00_001B2C6C __attribute__((nosda));

INCLUDE_ASM("asm/levels", func_L00_00403BF8);

extern int D_L00_001B2C30[] __attribute__((nosda));

INCLUDE_ASM("asm/levels", func_L00_00403C30);

extern int func_0031A728(int range);
extern void func_L00_00403900(int value);
extern void func_L00_00403C68(int value);

INCLUDE_ASM("asm/levels", func_L00_00403CB8);

extern void func_002EE108(char *a, int *b, char *c);
extern void func_0037F9A0(char *a, char *b);
extern void func_0037E1D8(int a, int *b, int c);
extern int func_002EDD48(char *a);
extern void func_002E6EF8(void (*f)(char *), char *arg);
extern void func_0037E1B8(int a);
extern void func_00310440(char *arg);
extern void func_L00_00404168(char *arg);
extern void func_L00_00403F90(char *arg, void *p);

void func_L00_00403D58(char *arg)
{
    int *o = *(int **)(arg + 104);
    unsigned char state = *(unsigned char *)(arg + 32);
    TI128 tmp;

    switch (state) {
    case 0:
        arg[32] = 1;
    case 1:
        qcopy(&tmp, arg + 16);
        func_002EE108(arg + 16, o, arg + 16);
        func_0037F9A0(arg + 16, arg + 16);
        func_0037E1D8(o[7], o, 0);
        func_L00_00403F90(arg, &tmp);
        if (func_002EDD48((char *)o + 16) != 0)
            arg[32] = 2;
        break;
    case 2:
        func_0037E1B8(o[7]);
        func_00310440(arg);
        return;
    default:
        break;
    }
    func_002E6EF8(func_L00_00404168, arg);
}

INCLUDE_ASM("asm/levels", func_L00_00403F90);

INCLUDE_ASM("asm/levels", func_L00_00404168);

INCLUDE_ASM("asm/levels", func_L00_00404368);

INCLUDE_ASM("asm/levels", func_L00_004045E8);

INCLUDE_ASM("asm/levels", func_L00_00404B48);

extern TI128 D_L00_001BF220;
extern void func_00310440(char *arg);
extern int func_002DE4B0(void *a, void *b, int c, void *d, int e);
extern void func_0035F450(int a, int b, void *c, int d);

void func_L00_00404DD8(char *arg, void *a1)
{
    if (*(float *)(arg + 16) < 2.0f || *(float *)(arg + 16) > 1021.0f ||
        *(float *)(arg + 20) < 2.0f || *(float *)(arg + 20) > 1021.0f ||
        *(float *)(arg + 24) < 2.0f || *(float *)(arg + 24) > 1021.0f) {
        func_00310440(arg);
        return;
    }
    {
        char *p = arg + 16;

        if (func_002DE4B0(a1, p, 0, arg, 0) != 0) {
            qcopy(p, &D_L00_001BF220);
            arg[32] = 2;
            func_0035F450(5, 0, arg, 3705);
        }
    }
}

INCLUDE_ASM("asm/levels", func_L00_00404ED8);

INCLUDE_ASM("asm/levels", func_L00_004051C8);

INCLUDE_ASM("asm/levels", func_L00_00405808);

INCLUDE_ASM("asm/levels", func_L00_00406040);

INCLUDE_ASM("asm/levels", func_L00_00406150);

INCLUDE_ASM("asm/levels", func_L00_00406FB0);

INCLUDE_ASM("asm/levels", func_L00_004073D8);

INCLUDE_ASM("asm/levels", func_L00_004079C8);

INCLUDE_ASM("asm/levels", func_L00_00407C18);

INCLUDE_ASM("asm/levels", func_L00_00407E38);

INCLUDE_ASM("asm/levels", func_L00_00408800);

INCLUDE_ASM("asm/levels", func_L00_00408E58);

/* shared native body of family af5b3bc3a678ed5c */
void func_L00_00409420(char *object)
{
    char *o = object;
    int *q = *(int **)(o + 104);

    if (*(unsigned char *)(o + 32))
        return;

    q[2] = 64;
    q[4] = 50;
    q[0] = 0;
    q[3] = -1;
    q[5] = 0;
    *(unsigned char *)(o + 32) = 1;
    q[6] = -1;

    *(float *)(o + 44) = *(float *)(*(int *)(o + 36) + 36) * 0.8f;
}

INCLUDE_ASM("asm/levels", func_L00_00409480);

INCLUDE_ASM("asm/levels", func_L00_00409558);
