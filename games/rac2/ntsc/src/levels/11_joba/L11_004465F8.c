#include "common.h"

extern void func_002B60B0(char *, float, float, float, float);
extern void func_002A7470(char *, int, int, int);
extern void func_002A7750(char *, int, int);
extern void func_002A05A8(char *);
extern void func_002A0488(char *, int, int, int);
extern int func_002B3380(char *, char *, char *);
extern void func_L11_003973F8(char *, int, float, float);

extern void func_002A04B8(char *, int *, int *, int *);

extern void func_00282D30_2(char *dst, char *src, float scale);

extern void func_32C9E0();
extern void func_32C700();
extern void func_324560(char *a0);
extern void func_441978(s32 a0, int a1);
extern void func_323688(char *a0, char *a1);
extern void func_300268(char *a0, int a1, int a2);
extern void func_L11_00449330();

typedef int TI128 __attribute__((mode(TI)));
extern char *func_3224B0(int a0);
extern void func_324F00(char *a0);
extern void func_3362A8(char *a0, s32 a1, int a2, int a3);
extern void func_300180(void *a0);
extern void func_L00_003D7250(char *a0, char *a1, float a2, int a3, int a4, int a5);
extern void func_338398(char *a0, char *a1, float a2, int a3, int a4);
extern void func_32E480(char *a0, char *a1, float f12, float f13, float f14,
                        int a2, int a3, int a4, int a5);
extern void func_2FD7E8(void *a0, int a1, void *a2, void *a3, int a4, int a5,
                        int a6, int a7, float f12);
extern void func_32EB98(void *a0, void *a1);
extern s32 func_441868(s32 a0, s32 a1);
extern s32 func_33C800(char *a0, s32 a1, s32 a2, s32 a3, float f12, float f13);

INCLUDE_ASM("asm/levels", func_L11_004465F8);

INCLUDE_ASM("asm/levels", func_L11_00446C78);

INCLUDE_ASM("asm/levels", func_L11_00446EB8);

INCLUDE_ASM("asm/levels", func_L11_00447018);

INCLUDE_ASM("asm/levels", func_L11_004471A8);

INCLUDE_ASM("asm/levels", func_L11_00447360);

INCLUDE_ASM("asm/levels", func_L11_004475A0);

INCLUDE_ASM("asm/levels", func_L11_004484A8);

void func_L11_00448848(char *a0)
{
    char *s1 = *(char **)(a0 + 0x68);

    func_300180(s1 + 0x364);
    if (*(s32 *)(s1 + 0x354) != 0) {
        func_3362A8(a0, *(s32 *)(s1 + 0x354), 2, 0);
        *(u16 *)((char *)(long)*(s32 *)(s1 + 0x354) + 0x34) &= 0xFFFD;
    }
    if (*(u16 *)(a0 + 0x96) < 0xFFFF)
        *(u16 *)(a0 + 0x96) = *(u16 *)(a0 + 0x96) + 1;
    func_338398(a0, s1 + 0x230, 3.5f, 4, 1);
    func_32E480(a0, a0 + 0x10, 1.5f, 1.0f, 1.0f, 0x4000000, 0, 1, 0);
}

void func_L11_00448908(char *a0)
{
    if (*(s32 *)(*(char **)(a0 + 0x68) + 0x354) != 0) {
        func_32C9E0(a0, 4, 2);
        if (*(u8 *)(a0 + 0x43) != 0)
            func_32C700(a0, 0, 0, 0xA);
    } else {
        func_32C9E0(a0, 5, 2);
        if (*(u8 *)(a0 + 0x43) != 1)
            func_32C700(a0, 1, 0, 0xA);
    }
}

INCLUDE_ASM("asm/levels", func_L11_00448AA8);

s32 func_L11_00448CE0(char *a0)
{
    s32 r = 0;
    u8 x;

    if (*(s32 *)(*(char **)(a0 + 0x68) + 0x354) != 0) {
        x = *(u8 *)(a0 + 0x20);
        if (x == 0xF || x == 4 || *(u8 *)(a0 + 0x95) == 4 || x == 0xE)
            r = 1;
    }
    return r;
}

INCLUDE_ASM("asm/levels", func_L11_00448DF8);

extern float func_0029F530(char *);
extern void func_0029FCC0(char *, int, void *);
extern void func_L00_0031BDC0(void *, void *, void *, void *);
extern void func_00282CF0(void *, void *, void *);
extern float func_00282DF0(void *);
extern int func_002A8C28(char *, void *, int, float, float, float, int, int, int);
extern int func_002E5CF8(int, int, char *);

void func_L11_00449098(char *object)
{
    float start[4], end[4], closest[4], remaining[4], span[4];
    char *state;
    float time, fraction;

    time = func_0029F530(object);
    state = *(char **)(object + 0x68);
    if (19.0f <= time && time <= 31.0f) {
        func_0029FCC0(object, 0, start);
        func_0029FCC0(object, 1, end);
        func_L00_0031BDC0(closest, start, end,
                         *(char **)(state + 0x350) + 0x10);
        func_00282CF0(remaining, closest, end);
        func_00282CF0(span, start, end);
        fraction = func_00282DF0(remaining);
        fraction /= func_00282DF0(span);
        if (fraction < 0.0f)
            fraction = 0.0f;
        if (fraction > 1.0f)
            fraction = 1.0f;
        if (func_002A8C28(object, closest, 0x10001,
                         fraction * 0.35f + 0.15f,
                         (float)*(unsigned char *)(state + 0x34), 1.0f, 0, 1, 0)
            && *(short *)(state + 0x368) == 0) {
            func_002E5CF8(5, 0, object);
            *(short *)(state + 0x368) = 1;
        }
        func_002A8C28(object, end, 0x10001, 0.5f,
                      (float)*(unsigned char *)(state + 0x34), 1.0f, 0, 1, 0);
    }
}

void func_L11_004492A0(char *a0)
{
    char *s0 = *(char **)(a0 + 0x68);
    s32 p = (s32)(long)func_3224B0(0x9D8);

    *(s32 *)(s0 + 0x354) = p;
    if (p == 0)
        return;
    *(u8 *)((char *)(long)p + 0x30) = *(u8 *)(a0 + 0x30);
    *(u16 *)((char *)(long)*(s32 *)(s0 + 0x354) + 0x32) = *(u16 *)(a0 + 0x32);
    *(u8 *)((char *)(long)*(s32 *)(s0 + 0x354) + 0x31) = 1;
    *(u64 *)((char *)(long)*(s32 *)(s0 + 0x354) + 0x38) = *(u64 *)(a0 + 0x38);
    func_324F00((char *)(long)*(s32 *)(s0 + 0x354));
    *(s32 *)((char *)(long)*(s32 *)(s0 + 0x354) + 0x98) = 0;
    *(s32 *)(*(char **)((char *)(long)*(s32 *)(s0 + 0x354) + 0x68) + 0x3C) = 1;
}

INCLUDE_ASM("asm/levels", func_L11_00449330);

void func_L11_004496A0(char *object, int mode, int active) {
    char *state = *(char **)(object + 0x68);
    int colors[3];
    int green, blue;

    *(short *)(state + 0x88) = 0;
    *(short *)(state + 0x366) = 0;
    *(float *)(state + 0x20) = *(short *)(state + 0x24);
    *(short *)(state + 0x36E) = active;
    func_002B60B0(object, 0.0055555556900799274f, 0.10333333909511566f,
                  0.0034906589426100254f, 0.41887909173965454f);
    *(u8 *)(state + 0x6A) = 0;
    *(u8 *)(state + 0x2E) = 1;
    *(u8 *)(object + 0x7F) = 0;
    *(u16 *)(object + 0x34) = (*(u16 *)(object + 0x34) | 0x1000) & 0xFFB9;
    *(int *)(object + 0x98) = *(int *)(*(char **)(object + 0x24) + 0x10);
    if (*(short *)(state + 0x36E) != 0) {
        func_002A7750(object, 4, 2);
        *(u16 *)(object + 0x34) |= 0x100;
        func_002A7470(object, 0, 0, 0);
        func_002A05A8(object);
        func_L11_003973F8(object, 0, 2.0f, 2.0f);
    } else {
        if (*(u8 *)(object + 0x43) != 0)
            func_002A7470(object, 0, 0, 10);
        func_002A7750(object, 3, 1);
        *(u8 *)(object + 0x31) = 1;
        *(u16 *)(object + 0x34) &= 0xFEFE;
    }
    func_L11_004492A0(object);
    *(short *)(state + 0x36C) = mode;
    *(int *)(state + 0x200) = 0;
    func_002B3380(object, state + 0x1F0, state + 0x20);
    if (*(short *)(state + 0x50) != 0) {
        *(short *)(state + 0x50) = 0;
        func_002A0488(object, *(u8 *)(state + 0x54), *(u8 *)(state + 0x55),
                      *(u8 *)(state + 0x56));
        *(short *)(state + 0x52) = 1;
    }
    if (*(char **)(state + 0x354) != 0) {
        func_002A04B8(object, &colors[0], &colors[1], &colors[2]);
        green = colors[1];
        blue = colors[2];
        func_002A0488(*(char **)(state + 0x354), colors[0], green, blue);
    }
}

void func_L11_00449878(char *a0)
{
    char *s2 = a0;
    char *s1;
    TI128 tmp;

    func_32C9E0(a0, 1, 0);
    func_32C700(a0, 0, 0, 3);
    func_324560(a0);
    *(u16 *)(s2 + 0x34) = (*(u16 *)(s2 + 0x34) & 0xEFFF) | 0x47;
    *(u8 *)(s2 + 0x31) = 0;
    *(s32 *)(s2 + 0x98) = 0;
    s1 = *(char **)(s2 + 0x68);
    if (*(s16 *)(s1 + 0x366) == 1)
        func_441978(*(s32 *)(s2 + 0xB8), 0xD98);
    *(s16 *)(s1 + 0x366) = 0;
    *(s32 *)(s1 + 0x350) = 0;
    if (*(s32 *)(s1 + 0x354) != 0) {
        tmp = *(TI128 *)((char *)(long)*(s32 *)(s1 + 0x354) + 0x10);
        func_L11_00449330(s2, &tmp);
    }
    if (*(u8 *)(s1 + 0x2B1) != 0) {
        f32 one = 1.0f;

        *(u8 *)(s1 + 0x2B2) = 0;
        *(f32 *)(s1 + 0x2D8) = one;
        *(f32 *)(s1 + 0x2D4) = one;
        *(f32 *)(s1 + 0x2D0) = one;
        func_323688(s2, s1 + 0x2B0);
        func_300268(s1 + 0x2B0, 0, 0x40);
    }
}

INCLUDE_ASM("asm/levels", func_L11_00449970);

INCLUDE_ASM("asm/levels", func_L11_00449AD8);

INCLUDE_ASM("asm/levels", func_L11_00449C70);

INCLUDE_ASM("asm/levels", func_L11_0044AFC8);

INCLUDE_ASM("asm/levels", func_L11_0044BD68);

void func_L11_0044C0F0(char *a0)
{
    char *p = *(char **)(a0 + 0x68);

    if (*(u16 *)(a0 + 0x96) < 0xFFFF)
        *(u16 *)(a0 + 0x96) = *(u16 *)(a0 + 0x96) + 1;
    func_338398(a0, p + 0x4E0, 4.0f, 7, 1);
    func_32E480(a0, a0 + 0x10, 1.3f, 1.0f, 1.0f, 0x4000000, 0, 1, 0);
}

/* advances the sub-object's ring cursor, hands the new slot of the 0x370 table to the scene
   manager, and refreshes the matching slot of the 0x270 table from the live 0x4A4 record */
INCLUDE_ASM("asm/levels", func_L11_0044C180);

INCLUDE_ASM("asm/levels", func_L11_0044C220);

/* state handler: clears the 0x490 word while the frame byte is out of the 0..3 window, otherwise
   arms the 0x4CA gate through the 0xB8 handle and runs the 100-unit smash into it */
/*W23B*/
INCLUDE_ASM("asm/levels", func_L11_0044C268);
/*W23E*/


void func_L11_0044C328(char *object, int mode, int active) {
    char *state = *(char **)(object + 0x68);
    char **slot;
    char *collision, *health;
    int i;

    *(short *)(state + 0x4CA) = 0;
    *(short *)(state + 0x88) = 0;
    *(u8 *)(state + 0x2E) = 1;
    *(float *)(state + 0x20) = *(short *)(state + 0x24);
    *(short *)(state + 0x4D2) = active;
    func_002B60B0(object, 0.004444444552063942f, 0.13333334028720856f,
                  0.0011635529808700085f, 0.10471977293491364f);
    *(u8 *)(object + 0x7F) = 0;
    *(u16 *)(object + 0x34) = (*(u16 *)(object + 0x34) | 0x1000) & 0xFFB9;
    *(int *)(object + 0x98) = *(int *)(*(char **)(object + 0x24) + 0x10);
    if (*(short *)(state + 0x4D2) != 0) {
        func_002A7750(object, 4, 2);
        *(u16 *)(object + 0x34) |= 0x100;
        func_002A7470(object, 2, 0, 0);
        func_002A05A8(object);
        func_L11_003973F8(object, 0, 2.0f, 2.0f);
    } else {
        func_002A7750(object, 3, 0);
        *(u8 *)(object + 0x31) = 1;
        *(u16 *)(object + 0x34) &= 0xFEFE;
    }
    *(u8 *)(state + 0x6A) = 0;
    collision = state + 0x1F0;
    health = state + 0x20;
    for (i = 0; i < 2; i++) {
        slot = (char **)(state + 0x4A0 + i * 4);
        if (*slot != 0) {
            *(u16 *)(*slot + 0x34) &= 0xFFFE;
            *(u8 *)(*slot + 0x31) = 1;
            *(int *)(*slot + 0x98) = *(int *)(*(char **)(*slot + 0x24) + 0x10);
        }
    }
    *(short *)(state + 0x4D0) = mode;
    *(int *)(state + 0x200) = 0;
    func_002B3380(object, collision, health);
    if (*(short *)(state + 0x50) != 0) {
        *(short *)(state + 0x50) = 0;
        func_002A0488(object, *(u8 *)(state + 0x54), *(u8 *)(state + 0x55),
                      *(u8 *)(state + 0x56));
        *(short *)(state + 0x52) = 1;
    }
}

extern void func_L11_00441978(int, int);
extern void func_0032C700(char *, int, int, int, int, int, int, int,
                         float, float, int, int, int, int);
extern void func_002E5DA0(int, int, char *, int);
extern void func_002A7A00(void *, float, float);
extern void func_00282CC0(void *, void *, void *);
extern void func_002B0A68(char *, int, void *, float, float, float, float, int);

void func_L11_0044C4F0(char *object) {
    char *state = *(char **)(object + 0x68);
    float direction[4];
    char **slot;
    int i;

    if (*(short *)(state + 0x4CA) == 1)
        func_L11_00441978(*(int *)(object + 0xB8), 0xDCF);
    *(short *)(state + 0x4CA) = 0;
    *(int *)(state + 0x490) = 0;
    if (*(u8 *)(object + 0x31) != 0) {
        func_0032C700(object + 0x10, 0, 0x1E, 5, 5, 5, 0x2D, 1,
                     0.5f, 0.0f, 1, 0, 0, 0);
        func_002E5DA0(0, 0, object, 0xDF7);
        func_002A7A00(direction, 0.033333335071802139f, 0.083333335816860199f);
        func_00282CC0(direction, direction, state + 0xD0);
        func_002B0A68(object, 0x7F0, direction, 3.0f, 6.0f, 3.0f, 6.0f, 0xFF);
    }
    slot = (char **)(state + 0x4A0);
    for (i = 1; i >= 0; i--) {
        if (*slot != 0) {
            *(u16 *)(*slot + 0x34) |= 1;
            *(u8 *)(*slot + 0x31) = 0;
            *(int *)(*slot + 0x98) = 0;
        }
        slot++;
    }
    func_002A7470(object, 0, 0, 3);
    func_002A05A8(object);
    func_002A7750(object, 1, 0);
    *(u8 *)(object + 0x31) = 0;
    *(int *)(object + 0x98) = 0;
    *(u16 *)(object + 0x34) = (*(u16 *)(object + 0x34) & 0xEFFF) | 0x47;
}

INCLUDE_ASM("asm/levels", func_L11_0044C688);

void func_L11_0044C818(char *a0)
{
    char *p = *(char **)(a0 + 0x68);

    *(u16 *)(p + 0x4CC) = *(u8 *)(a0 + 0x20);
    *(u16 *)(p + 0x4CE) = *(u8 *)(a0 + 0x95);
    func_32C9E0(a0, 6, 4);
}

INCLUDE_ASM("asm/levels", func_L11_0044C850);

INCLUDE_ASM("asm/levels", func_L11_00450328);

/* Scales four consecutive 16-byte XYZ vectors in place by one scalar through a vector helper. */
void func_L11_00450748(char *p, float scale)
{
    func_00282D30_2(p, p, scale);
    func_00282D30_2(p + 16, p + 16, scale);
    func_00282D30_2(p + 32, p + 32, scale);
    func_00282D30_2(p + 48, p + 48, scale);
}

INCLUDE_ASM("asm/levels", func_L11_00451178);

INCLUDE_ASM("asm/levels", func_L11_00452450);

void func_L11_00452810(char *a0)
{
    char *p = *(char **)(a0 + 0x68);

    if (*(u16 *)(a0 + 0x96) < 0xFFFF)
        *(u16 *)(a0 + 0x96) = *(u16 *)(a0 + 0x96) + 1;
    func_338398(a0, p + 0x2E0, 4.0f, 5, 1);
    func_32E480(a0, a0 + 0x10, 1.3f, 1.0f, 1.0f, 0x4000000, 0, 1, 0);
}

void func_L11_004528A0(char *a0)
{
    char *s1 = *(char **)(a0 + 0x68);
    float f;

    if (*(u8 *)(a0 + 0x95) != 0) {
        func_L00_003D7250(a0, s1 + 0x270, 0.125f, 0x36F6, 0x20000000, 0x20000000);
        f = 0.15f;
        func_L00_003D7250(a0, s1 + 0x280, f, 0x1050B0, 0x30000000, 0x20000000);
        func_L00_003D7250(a0, s1 + 0x290, f, 0x1050B0, 0x10000000, 0x30000000);
    }
}

void func_L11_00452C58(char *object, int mode, int active) {
    char *state = *(char **)(object + 0x68);

    *(u8 *)(state + 0x2E) = 1;
    *(short *)(state + 0x88) = 0;
    *(short *)(state + 0x2D2) = 1;
    *(float *)(state + 0x20) = *(short *)(state + 0x24);
    *(short *)(state + 0x2D8) = active;
    func_002B60B0(object, 0.004444444552063942f, 0.13333334028720856f,
                  0.0011635529808700085f, 0.10471977293491364f);
    if (*(u8 *)(object + 0x43) != 2)
        func_002A7470(object, 2, 0, 0);
    *(u8 *)(object + 0x7F) = 0;
    *(u16 *)(object + 0x34) = (*(u16 *)(object + 0x34) | 0x1000) & 0xFFB9;
    *(int *)(object + 0x98) = *(int *)(*(char **)(object + 0x24) + 0x10);
    if (*(short *)(state + 0x2D8) != 0) {
        func_002A7750(object, 4, 2);
        *(u16 *)(object + 0x34) |= 0x100;
        func_002A7470(object, 2, 0, 0);
        func_002A05A8(object);
        func_L11_003973F8(object, 0, 2.0f, 2.0f);
    } else {
        func_002A7750(object, 3, 0);
        *(u8 *)(object + 0x31) = 1;
        *(u16 *)(object + 0x34) &= 0xFEFE;
    }
    *(short *)(state + 0x2D4) = mode;
    *(u8 *)(state + 0x6A) = 0;
    *(int *)(state + 0x200) = 0;
    func_002B3380(object, state + 0x1F0, state + 0x20);
    if (*(short *)(state + 0x50) != 0) {
        *(short *)(state + 0x50) = 0;
        func_002A0488(object, *(u8 *)(state + 0x54), *(u8 *)(state + 0x55),
                      *(u8 *)(state + 0x56));
        *(short *)(state + 0x52) = 1;
    }
}

void func_L11_00452DF8(char *object) {
    char *state = *(char **)(object + 0x68);
    float direction[4];

    *(short *)(state + 0x2D2) = 0;
    *(int *)(state + 0x2C4) = 0;
    if (*(u8 *)(object + 0x31) != 0) {
        func_0032C700(object + 0x10, 0, 5, 5, 25, 20, 10, 1,
                     0.5f, 0.0f, 1, 0, 0, 0);
        func_002E5DA0(0, 0, object, 0xDF7);
        func_002A7A00(direction, 0.083333335816860199f, 0.11666667461395264f);
        func_00282CC0(direction, direction, state + 0xD0);
        func_002B0A68(object, 0xFF0, direction, 3.0f, 6.0f, 3.0f, 6.0f, 0xFF);
    }
    func_002A7470(object, 0, 0, 3);
    func_002A05A8(object);
    func_002A7750(object, 1, 0);
    *(u8 *)(object + 0x31) = 0;
    *(int *)(object + 0x98) = 0;
    *(u16 *)(object + 0x34) = (*(u16 *)(object + 0x34) & 0xEFFF) | 0x47;
}


INCLUDE_ASM("asm/levels", func_L11_00452F30);

void func_L11_004530C0(char *a0)
{
    func_32C9E0(a0, 6, 3);
}

INCLUDE_ASM("asm/levels", func_L11_004530E0);
