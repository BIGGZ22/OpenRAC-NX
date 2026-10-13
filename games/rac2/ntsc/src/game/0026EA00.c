#include "include_asm.h"

/* Family 34e74db3063e7a24 — 108 bytes, 2 placements (18_damosel, 6_notak).
   Same shape as be569dc253cd5555: walk the resident function-pointer table at
   0x1B2180, call each entry while the resident count at 0x1B21C0 says there is
   one, then clear the count.  Small-data profile: the count is a 4-byte scalar,
   so cc1 prints the bare-symbol macro and gas expands every site on its own. */
extern int D_L06_001B21C0 __attribute__((sda));

extern void (*D_L06_001B2180[])(void);

extern float func_00282C48(float t);

extern float func_00283BE0_3(float, float);

extern float func_00282C48_5(float);

extern void func_00283B98_6(float, float);

typedef unsigned char u8;

typedef unsigned short u16;

typedef struct {u8 prefix[0x190];u8 *object;} NativeObjectRefView;

extern NativeObjectRefView D_L24_001B8580;

extern void func_00282CF0_9(float *p);

extern float func_00282E20_4(float *p);

extern float func_00283248_7(float a, float b);

extern void func_00283BE0_5(float a, float b);

extern float func_00282DF0_3(float *p);

extern void Fdb046c5d_FUN_002742E8(void *);

extern void Fdb046c5d_FUN_00274128(void *);

extern int Fdb046c5d_FUN_00274500(void *);

extern void Fdb046c5d_FUN_00282F20(float, void *, void *);

extern void Fdb046c5d_FUN_00282DD8(void *, void *, void *);

extern void Fdb046c5d_FUN_00274A88(void *, void *, void *);

extern void Fdb046c5d_FUN_002AFFD8(void *, void *, int);

extern int Fdb046c5d_FUN_00274BA8(void *, void *, void *, float, float);

extern int Fdb046c5d_FUN_002E5CF8(int, int, void *);

extern void Fdb046c5d_FUN_00274768(void *, int, void *);

extern void Fdb046c5d_FUN_002E5F00(int, void *);

extern char Fdb046c5d_D_001BAFC0[];

INCLUDE_ASM("asm/game", func_0026EA00);

INCLUDE_ASM("asm/game", func_0026EAC8);

extern int func_002A77E0(int);

void func_0026EB98(int *first, int *second, int *third) {
    switch (func_002A77E0(3)) {
    case 0:
        *first = 0;
        *second = 1;
        *third = 2;
        break;
    case 1:
        *first = 1;
        *second = 0;
        *third = 2;
        break;
    case 2:
        *first = 2;
        *second = 1;
        *third = 0;
        break;
    }
    if (func_002A77E0(2) != 0) {
        int value = *second;
        *second = *third;
        *third = value;
    }
}

INCLUDE_ASM("asm/game", func_0026EC58);

INCLUDE_ASM("asm/game", func_0026EDE8);

unsigned int func_0026F710(void)
{
    return 0;
}

void func_0026F718(void)
{
}

void func_0026F720(void)
{
}

void func_0026F728(void)
{
}

unsigned int func_0026F730(void)
{
    return 0;
}

unsigned int func_0026F738(void)
{
    return 0;
}

unsigned int func_0026F740(void)
{
    return 0;
}

unsigned int func_0026F748(void)
{
    return 0;
}

void func_0026F750(void)
{
}

void func_0026F758(void)
{
}

void func_0026F760(void)
{
}

void func_0026F768(void)
{
}

void func_0026F770(void)
{
}

void func_0026F778(void)
{
}

void func_0026F780(void)
{
}

void func_0026F788(void)
{
}

void func_0026F790(void)
{
}

void func_0026F798(void)
{
}

unsigned int func_0026F7A0(void)
{
    return 0;
}

void func_0026F7B0(void)
{
}

unsigned int func_0026F7B8(void)
{
    return 0;
}

unsigned int func_0026F7C0(void)
{
    return 0;
}

unsigned int func_0026F7C8(void)
{
    return 0;
}

unsigned int func_0026F7D0(void)
{
    return 0;
}

unsigned int func_0026F7D8(void)
{
    return 0;
}

unsigned int func_0026F7E0(void)
{
    return 0;
}

unsigned int func_0026F7E8(void)
{
    return 0;
}

unsigned int func_0026F7F0(void)
{
    return 0;
}

unsigned int func_0026F7F8(void)
{
    return 0;
}

unsigned int func_0026F800(void)
{
    return 0;
}

void func_0026F808(void)
{
}

INCLUDE_ASM("asm/game", func_0026F810);

INCLUDE_ASM("asm/game", func_0026FC70);

INCLUDE_ASM("asm/game", func_0026FC78);

INCLUDE_ASM("asm/game", func_0026FE48);

void func_0026FEB8(const char *format,...) {
}

INCLUDE_ASM("asm/game", func_0026FEF0);

INCLUDE_ASM("asm/game", func_0026FF08);

INCLUDE_ASM("asm/game", func_00270018);

INCLUDE_ASM("asm/game", func_002701A8);

INCLUDE_ASM("asm/game", func_002701B0);

/* shared native body of family 34e74db3063e7a24 */
void func_00270210(void)
{
    int i;

    for (i = 0; i < D_L06_001B21C0; i++) {
        D_L06_001B2180[i]();
    }

    D_L06_001B21C0 = 0;
}

INCLUDE_ASM("asm/game", func_00270280);

INCLUDE_ASM("asm/game", func_002702B8);

/* Integrates one float field with a spring-damper step, clamps it to a symmetric limit, then lets a helper raise or lower it, returning a bias plus the field. */
float func_002702C8(float *p, float a, float b, float c, float d, float lim)
{
    float t = b - a;
    float x = *p;
    float r;

    x += c * t - d * x;
    *p = x;
    if (lim != 0.0f) {
        if (x > lim) *p = lim;
        else if (x < -lim) *p = -lim;
    }
    r = func_00282C48(t);
    if (r < *p) {
        *p = func_00282C48(t);
    } else {
        float s = -func_00282C48(t);
        if (*p < s) {
            *p = -func_00282C48(t);
        }
    }
    return a + *p;
}

/* Advances a stored float by a scaled helper result, then clamps it against a positive limit, then clamps it again with a second helper and finally rescales it through a third helper. */
void func_002703B0(char *obj, float p1, float p2, float p3, float p4, float p5)
{
    float r;
    float t;
    float d;
    float neg;

    r = func_00283BE0_3(p2, p1);
    t = *(float *)obj;
    d = p3 * r - p4 * t;
    t = t + d;
    *(float *)obj = t;

    if (p5 != 0.0f) {
        if (t > p5) {
            *(float *)obj = p5;
        } else {
            neg = -p5;
            if (t < neg)
                *(float *)obj = neg;
        }
    }

    if (func_00282C48_5(r) < *(float *)obj) {
        *(float *)obj = func_00282C48_5(r);
    } else if (*(float *)obj < -func_00282C48_5(r)) {
        *(float *)obj = -func_00282C48_5(r);
    }

    func_00283B98_6(p1, *(float *)obj);
}

/* sets the referenced object halfword at 0x7e and clears its byte at 0x7d */
void func_002704D0(void) {
 *(u16 *)(D_L24_001B8580.object+0x7e)=1;
 D_L24_001B8580.object[0x7d]=0;
}

INCLUDE_ASM("asm/game", func_002704F0);

typedef void (*ObjectDispatch)(void *);
typedef struct { ObjectDispatch entries[5]; } ObjectDispatchRecord;
extern ObjectDispatchRecord D_0026E900[];

void func_00270550(char *object) {
    ObjectDispatch callback = D_0026E900[*(short *)(object + 0x8C)].entries[2];
    if (callback != 0)
        callback(object);
}

INCLUDE_ASM("asm/game", func_00270598);

INCLUDE_ASM("asm/game", func_00270838);

void func_00270A00(char *object) {
    ObjectDispatch callback = D_0026E900[*(short *)(object + 0x8C)].entries[4];
    if (callback != 0)
        callback(object);
}

INCLUDE_ASM("asm/game", func_00270A48);

INCLUDE_ASM("asm/game", func_00270B58);

INCLUDE_ASM("asm/game", func_00270D50);

INCLUDE_ASM("asm/game", func_00270E30);

INCLUDE_ASM("asm/game", func_00270EA8);

INCLUDE_ASM("asm/game", func_00270EE0);

INCLUDE_ASM("asm/game", func_00271130);

INCLUDE_ASM("asm/game", func_002712D8);

INCLUDE_ASM("asm/game", func_002718B8);

INCLUDE_ASM("asm/game", func_00271958);

INCLUDE_ASM("asm/game", func_00271B90);

INCLUDE_ASM("asm/game", func_00271E88);

INCLUDE_ASM("asm/game", func_00271FD8);

INCLUDE_ASM("asm/game", func_00272198);

INCLUDE_ASM("asm/game", func_002721C8);

INCLUDE_ASM("asm/game", func_00272280);

INCLUDE_ASM("asm/game", func_00272368);

INCLUDE_ASM("asm/game", func_00272540);

/* Fills a three-float local through two setup calls, stores a two-argument float helper result into a field, then stores a second helper result into two more fields and returns zero. */
int func_00272550(float *a0)
{
    float local[3];
    float t;

    func_00282CF0_9(local);
    a0[1] = func_00283248_7(func_00282E20_4(local), local[2]);

    t = func_00283248_7(local[0], local[1]);
    func_00283BE0_5(t, a0[0]);
    a0[0] = t;

    a0[2] = func_00282DF0_3(local);

    return 0;
}

INCLUDE_ASM("asm/game", func_002725C8);

INCLUDE_ASM("asm/game", func_002725E0);

INCLUDE_ASM("asm/game", func_002729A0);

INCLUDE_ASM("asm/game", func_00272CB0);

INCLUDE_ASM("asm/game", func_00273310);

INCLUDE_ASM("asm/game", func_00273318);

INCLUDE_ASM("asm/game", func_00273418);

INCLUDE_ASM("asm/game", func_00273598);

INCLUDE_ASM("asm/game", func_002735A0);

INCLUDE_ASM("asm/game", func_00273730);

INCLUDE_ASM("asm/game", func_00273978);

INCLUDE_ASM("asm/game", func_00273B70);

int func_00273D10(char *obj)
{
    float buf[4];
    float out[4];
    char *p;
    char *s1;
    unsigned char k;
    int r;
    int s4;
    int s5;

    p = *(char **)(obj + 104);
    k = *(unsigned char *)(p + 92);
    if (k >= 3)
        return 1;
    if (k != 0)
        Fdb046c5d_FUN_002742E8(obj);
    else
        Fdb046c5d_FUN_00274128(obj);

    s5 = Fdb046c5d_FUN_00274500(obj);
    if (s5 == -1)
        return 0;

    s1 = Fdb046c5d_D_001BAFC0;
    s4 = -1;
    Fdb046c5d_FUN_00282F20(1.0f, s1, s1);
    Fdb046c5d_FUN_00282DD8(buf, s1, p);
    Fdb046c5d_FUN_00274A88(obj, p + 16, buf);
    Fdb046c5d_FUN_002AFFD8(obj + 16, out, 1);
    r = Fdb046c5d_FUN_00274BA8(p, s1, out, *(float *)(p + 56), *(float *)(p + 60));
    if (r == 0) {
        if (*(unsigned char *)(p + 94) != 255 && *(unsigned char *)(p + 92) == 0)
            s4 = Fdb046c5d_FUN_002E5CF8(*(unsigned char *)(p + 94), 0, obj);
        Fdb046c5d_FUN_00274768(obj, s5, out);
    } else {
        if (*(unsigned char *)(p + 94) != 255)
            s4 = Fdb046c5d_FUN_002E5CF8(*(unsigned char *)(p + 94), 0, obj);
    }
    if (s4 != -1)
        Fdb046c5d_FUN_002E5F00(s4, p + 32);
    return 0;
}

INCLUDE_ASM("asm/game", func_00273E98);
