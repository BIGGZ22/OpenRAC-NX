#include "include_asm.h"

typedef struct { int a, b; } PAIR8;

extern PAIR8 D_L00_001B1458 __attribute__((nosda));

extern int D_L00_001B1460 __attribute__((sda));

extern void func_2EE0D0(void *dst);

extern int func_3411A0(void *a0, void *a1, int a2, float f12);

/* The level's literal pool, read through $gp. */
extern float D_L00_001B151C __attribute__((sda));
extern float D_L00_001B1520 __attribute__((sda));
extern float D_L00_001B1524 __attribute__((sda));
extern float D_L00_001B1528 __attribute__((sda));

extern int func_00282978(short *p);

extern void func_L00_00421A98(char *obj, char *arg1, float *local, unsigned char index);

extern void func_L00_004219D8(char *obj, float *a1, float *a2, float f12, float f13, float f14);

void func_L00_0041ACA0(char *a0, void *a1)
{
    int local[4];
    int sel[2];
    char *p = *(char **)(a0 + 0x68);
    float f;

    func_2EE0D0(local);
    *(PAIR8 *)sel = D_L00_001B1458;
    f = *(float *)(p + 0x50) * 0.025f;
    func_3411A0(a1, local, sel[D_L00_001B1460 % 2], f);
    D_L00_001B1460 = D_L00_001B1460 + 1;
}

/* Updates the type byte and interpolates the two values passed to the helper. */
void func_L00_00421B50(char *obj, char *arg1)
{
    char *base;
    char *p;
    short v;
    float scale;
    float f12;
    float f13;
    float f14;
    float local[3];

    base = *(char **)(obj + 0x70);
    p = base + 0x60;

    if (func_00282978((short *)(base + 0x126)) != 0) {
        *(unsigned char *)(p + 0xC5) = *(unsigned char *)(p + 0xC4);
    }
    v = *(short *)(p + 0xC6);
    scale = (float)v * *(float *)(p + 0xC8);
    func_L00_00421A98(obj, arg1, local, *(unsigned char *)(p + 0xC4));

    if (*(unsigned char *)(p + 0xC4) == 3 ||
        *(unsigned char *)(p + 0xC4) == 5 ||
        *(unsigned char *)(p + 0xC4) == 6 ||
        *(unsigned char *)(p + 0xC4) == 7 ||
        *(unsigned char *)(p + 0xC4) == 1) {
        f12 = D_L00_001B1524 + (D_L00_001B151C - D_L00_001B1524) * scale;
        f13 = D_L00_001B1528 + (D_L00_001B1520 - D_L00_001B1528) * scale;
    } else {
        f12 = D_L00_001B151C;
        f13 = D_L00_001B1520;
    }
    f14 = 0.0f;
    func_L00_004219D8(obj, (float *)(p + 0x90), local, f12, f13, f14);
}
