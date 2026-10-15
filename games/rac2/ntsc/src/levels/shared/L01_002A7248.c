#include "include_asm.h"

typedef struct { char *p[6]; } Sep6;

extern Sep6 D_L01_001AA550;

extern char D_L01_001AA568[];

extern char D_L01_001AA578[];

extern char D_L01_001AA588[];

extern int func_00115DA8_2(char *dst, char *fmt, ...);

extern char D_L01_00189E20[];

extern char D_L01_002637A0[];

extern int D_L01_001A8F40;

extern char D_L01_00189E20_sda[] __attribute__((sda));

INCLUDE_ASM("asm/levels", func_L01_002A7248);

extern void func_002C7CA8(void);
extern void func_002B4070(void);
extern void func_002ADDC8(void);
extern void func_002C37C8(void);
extern void func_002AEB80(void);
extern void func_002B9878(void);
extern void func_002A7740(void);
extern void func_002AB1C8(void);
extern void func_002A9520(void);
extern void func_002A9310(void);
extern void func_002FDB00(void *a0);
extern void func_002AB328(void);
extern void func_0030F444(int a0);

extern char D_L01_00189E20[];

void func_L01_002A84D8(void) {
    char *p;

    func_002C7CA8();
    func_002B4070();
    func_002ADDC8();
    func_002C37C8();
    func_002AEB80();
    p = D_L01_00189E20;
    if (*(unsigned char *)(p + 0x22B4) != 0) {
        func_002B9878();
        func_002A7740();
        if (*(unsigned char *)(p + 0x22B4) != 0) {
            func_002AB1C8();
            func_002A9520();
            func_002A9310();
            func_002FDB00(p + 0x80);
            func_002AB328();
            func_0030F444(*(int *)(p + 0x2290));
        }
    }
}

INCLUDE_ASM("asm/levels", func_L01_002A8C48);


extern void func_002A8EE0(int a0, int a1);
extern char D_L01_00139568[];

void func_L01_002A9298(void) {
    char *b = D_L01_00189E20;
    int i = *(int *)(b + 0x1248);
    unsigned char c;
    unsigned char *t;

    if (i < 0)
        return;
    c = *(unsigned char *)(D_L01_00139568 + i);
    t = (unsigned char *)D_L01_002637A0 + c * 0xE0;
    if (t[4] == 0)
        return;
    if (*(int *)(b + 0x1220) == 0)
        return;
    func_002A8EE0(*(int *)(b + 0x1220), *(int *)(b + 0x2290));
}

INCLUDE_ASM("asm/levels", func_L01_002ABF90);

INCLUDE_ASM("asm/levels", func_L01_002AE128);

INCLUDE_ASM("asm/levels", func_L01_002B0318);

INCLUDE_ASM("asm/levels", func_L01_002B3458);

INCLUDE_ASM("asm/levels", func_L01_002B35E8);

INCLUDE_ASM("asm/levels", func_L01_002B3788);

INCLUDE_ASM("asm/levels", func_L01_002B3D28);

INCLUDE_ASM("asm/levels", func_L01_002B46A0);


INCLUDE_ASM("asm/levels", func_L01_002B4DA0);

INCLUDE_ASM("asm/levels", func_L01_002B5EA0);

INCLUDE_ASM("asm/levels", func_L01_002B6948);

INCLUDE_ASM("asm/levels", func_L01_002BE0E0);

extern unsigned char D_0018C0D4[];

extern void func_002BEB90(unsigned long, unsigned long);

void func_L01_002C1988(unsigned long x) {
    unsigned long mode = 1;
    if (x)
        mode = 2;
    if (D_0018C0D4[0] == 0)
        func_002BEB90(0, mode);
}

INCLUDE_ASM("asm/levels", func_L01_002C2CF0);

INCLUDE_ASM("asm/levels", func_L01_002C81D0);

extern void *D_0018C0B0;

void func_L01_002C8990(void) {
    *(unsigned short *)((char *)D_0018C0B0 + 0x34) |= 1;
}

void func_L01_002C89A8(void) {
    *(unsigned short *)((char *)D_0018C0B0 + 0x34) &= 0xFFFE;
}

extern float D_L01_00189F90;

void func_L01_002C98B8(void) {
    if (D_L01_00189F90 < 0.045f)
        func_002BEB90(0, 1);
    else
        func_002BEB90(3, 1);
}

INCLUDE_ASM("asm/levels", func_L01_002E1150);

INCLUDE_ASM("asm/levels", func_L01_002E1510);

INCLUDE_ASM("asm/levels", func_L01_002E1670);

INCLUDE_ASM("asm/levels", func_L01_002E1680);


INCLUDE_ASM("asm/levels", func_L01_002E2608);

INCLUDE_ASM("asm/levels", func_L01_002EABA8);

INCLUDE_ASM("asm/levels", func_L01_002F0E88);

INCLUDE_ASM("asm/levels", func_L01_002F14E8);

INCLUDE_ASM("asm/levels", func_L01_002F1588);

INCLUDE_ASM("asm/levels", func_L01_002F1F70);

INCLUDE_ASM("asm/levels", func_L01_002F1FD0);

INCLUDE_ASM("asm/levels", func_L01_00304270);

INCLUDE_ASM("asm/levels", func_L01_00310BC0);

extern unsigned short *D_L01_001FB020[];

#define MACRO_ADDR __attribute__((section(".sdata")))

extern unsigned short *r1_D_L01_001ABFC0[];

extern char *r1_D_L01_00160058 MACRO_ADDR;

extern int r1_D_L01_00160058_m __asm__("r1_D_L01_00160058") MACRO_ADDR;

/* rac1-decomp src/overlays/shared/mobyutil_0026E8E0.c: func_L01_0026F090 */
// Walks a list of moby indices and sets the state byte of each one that is non-negative.
void func_L01_003115E8(int list, int state)
{
    unsigned short *p;
    unsigned short *q = (unsigned short *)r1_D_L01_001ABFC0[list];
    if (q != 0) {
        p = q;
        do {
            char *m = (char *)(((*p & 0x7FFF) << 8) + r1_D_L01_00160058_m);
            if (m[0x20] >= 0) {
                m[0x20] = state;
            }
        } while (*(short *)p++ >= 0);
    }
}

INCLUDE_ASM("asm/levels", func_L01_00311C18);

INCLUDE_ASM("asm/levels", func_L01_003120A8);

void func_L01_003128C0(char *p) {
    *(int *)(p + 0x70) = 0;
    *(float *)(p + 0x74) = 0.2f;
}

INCLUDE_ASM("asm/levels", func_L01_003161F0);

INCLUDE_ASM("asm/levels", func_L01_003165E8);


extern void func_002E3CD8(void *a0, void *a1);
extern void func_002E3F60(void *a0, void *a1);
extern void func_002E34A0(void *a0, void *a1, void *a2);
extern void func_L01_002E3940(void *a0, void *a1, void *a2);
extern void func_002E3470(void *a0, void *a1, void *a2);

void func_L01_00316C48(void *a0, void *a1, void *a2, void *a3, void *t0) {
    float p[0x10];
    float q[0x10];
    float v[4];

    func_002E3CD8(p, a3);
    func_002E3F60(p, p);
    func_002E3CD8(q, t0);
    func_002E34A0(v, a0, a2);
    func_L01_002E3940(v, v, p);
    func_L01_002E3940(v, v, q);
    func_002E3470(a1, v, a2);
}

INCLUDE_ASM("asm/levels", func_L01_00318638);

extern void func_002E3918(void *a0, void *a1, void *a2);
extern void func_002E3470(void *a0, void *a1, void *a2);

void func_L01_0031A278(char *a0, float *a1, float f12, float f13, float f14) {
    a1[0] = f12;
    a1[1] = f13;
    a1[2] = f14;
    func_002E3918(a1, a1, a0 + 0xC0);
    func_002E3470(a1, a1, a0 + 0x10);
}

INCLUDE_ASM("asm/levels", func_L01_0031AB20);

INCLUDE_ASM("asm/levels", func_L01_0031B300);

extern float func_002E3A60(int a0, int a1);
extern void func_002E4600(int a0, int a1, float f12);

void func_L01_0031BC68(int a0, int a1) {
    float v = func_002E3A60(a0, a1);
    func_002E4600(a0, a1, v * 0.5f + 0.5f);
}

INCLUDE_ASM("asm/levels", func_L01_0031EF98);

INCLUDE_ASM("asm/levels", func_L01_0031F080);

INCLUDE_ASM("asm/levels", func_L01_003200C8);

INCLUDE_ASM("asm/levels", func_L01_003208D0);

INCLUDE_ASM("asm/levels", func_L01_003209F8);


extern float func_002E3B10(float f12, float f13);
extern void func_003212C0(char *a0, char *a1, float f12);

void func_L01_00321178(char *a0, char *a1) {
    float v = func_002E3B10(*(float *)(a1 + 0) - *(float *)(a0 + 0x10),
                            *(float *)(a1 + 4) - *(float *)(a0 + 0x14));
    func_003212C0(a0, a1, v);
}

extern char *func_00321F08(int a0);
extern void func_003140B0(char *a0, char *a1, int a2, float f12, float f13,
                          float f14, float f15);

int func_L01_00322288(int a0, float f12) {
    char *p = func_00321F08(a0);
    if (p == 0)
        return 0;
    func_003140B0((char *)a0 + 0xF8, p + 0xB4, 0, f12, *(float *)(p + 0x14),
                  *(float *)(p + 0x18), *(float *)(p + 0x1C));
    return 1;
}

INCLUDE_ASM("asm/levels", func_L01_00322358);

INCLUDE_ASM("asm/levels", func_L01_00323310);

/* shared native body of family d1b66aa7f0ecd20d */
void func_L01_003242C8(char *dst, int x, int idx)
{
    Sep6 t;
    t = D_L01_001AA550;
    if (x > 999999) {
        int m = x / 1000000;
        int w = x % 1000000;
        func_00115DA8_2(dst, D_L01_001AA568, m, t.p[idx % 6], w / 1000,
                          t.p[idx % 6], w % 1000);
    } else if (x >= 1000)
        func_00115DA8_2(dst, D_L01_001AA578, x / 1000,
                          t.p[idx % 6], x % 1000);
    else
        func_00115DA8_2(dst, D_L01_001AA588, x);
}

INCLUDE_ASM("asm/levels", func_L01_00324408);

INCLUDE_ASM("asm/levels", func_L01_00324410);

INCLUDE_ASM("asm/levels", func_L01_00324550);


INCLUDE_ASM("asm/levels", func_L01_003245E0);


extern char D_00139A28[];

void func_L01_003246A0(int a) {
    char *base = D_00139A28;
    char *p;
    base = base + 0xC;
    p = base + a * 16;
    if (*(int *)p == 0)
        *(int *)p = 1;
}

extern int func_003246F8(void);

void func_L01_003246D0(void) {
    func_L01_003246A0(func_003246F8());
}

INCLUDE_ASM("asm/levels", func_L01_003246F8);


INCLUDE_ASM("asm/levels", func_L01_00326DD8);

INCLUDE_ASM("asm/levels", func_L01_0032EF38);

INCLUDE_ASM("asm/levels", func_L01_00331CA0);

INCLUDE_ASM("asm/levels", func_L01_00331E08);

INCLUDE_ASM("asm/levels", func_L01_003377C8);


INCLUDE_ASM("asm/levels", func_L01_00337888);

INCLUDE_ASM("asm/levels", func_L01_00337A60);

INCLUDE_ASM("asm/levels", func_L01_00337B80);

INCLUDE_ASM("asm/levels", func_L01_0034EA48);

extern char D_00188660[];

int func_L01_00355BA0(int index, int value) {
    char *p = D_00188660 + index * 0x70;
    *(unsigned short *)(p + 0x80) = value;
    return 1;
}

INCLUDE_ASM("asm/levels", func_L01_00367F78);

extern int func_003711D8(int a0);
extern unsigned char D_L01_001395B8[];

void func_L01_00371228(int a0, int a1) {
    int i = func_003711D8(a0);
    if (i >= 0)
        D_L01_001395B8[i] |= (unsigned char)(1 << a1);
}

INCLUDE_ASM("asm/levels", func_L01_003801B8);

INCLUDE_ASM("asm/levels", func_L01_003808C0);

INCLUDE_ASM("asm/levels", func_L01_0038EF40);

INCLUDE_ASM("asm/levels", func_L01_003907B8);

extern void func_003907B8(int a0, int a1, int a2);

void func_L01_003909E0(int a0, int a1, int a2, int a3) {
    unsigned short *p = D_L01_001FB020[a2];
    if (p != 0) {
        do {
            func_003907B8(*p & 0x7FFF, a1, a3);
        } while (*(short *)p++ >= 0);
    }
}

INCLUDE_ASM("asm/levels", func_L01_00390A50);

void func_L01_00390C60(char *a0)
{
    char *v0 = *(char **)(a0 + 0x68);
    char *v1;
    unsigned char a1;

    if (v0 == 0)
        return;
    v1 = *(char **)v0;
    if (v1 == 0)
        return;
    a1 = *(unsigned char *)(v1 + 0x20);
    if (a1 == 0xFE)
        return;
    if (a1 == 0xFD)
        return;
    if (*(unsigned short *)(v1 + 0x34) & 1)
        *(unsigned short *)(a0 + 0x34) |= 0x41;
    else
        *(unsigned short *)(a0 + 0x34) &= 0xFFBE;
}

INCLUDE_ASM("asm/levels", func_L01_00394148);

INCLUDE_ASM("asm/levels", func_L01_00394240);
