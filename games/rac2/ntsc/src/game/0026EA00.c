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

extern char D_001BAEC0[];
extern int D_001A72DC[] __attribute__((sda));
extern void func_0029D4D0(int, void *);
extern int func_0011AEA0(int);
extern void func_002FC1D8(void);
extern void func_002FCB00(void *, int, int, int);
extern void func_0027B858(void);
extern void func_00284FC0(void);
extern void func_00285338(void);
extern void func_002FC280(void);
extern void func_002FC3C0(int);
extern int func_001244B8(int, int);
extern int func_001261F0(int);

INCLUDE_ASM("asm/game", func_0026EA00);

extern void func_002FCD18(void);
extern void func_00284E18(int, int, int);
extern void func_00284EA8(void);
extern void func_00284F40(int);
extern void func_002FCDA8(void);

void func_0026EAC8(int arg)
{
    char *state = D_001BAEC0;
    int *data;
    func_0029D4D0(arg, *(void **)(state + 20));
    func_0011AEA0(0);
    func_002FC1D8();
    func_002FCD18();
    func_00284E18(0, 0, 0);
    func_00284EA8();
    func_00284F40(0);
    data = *(int **)(state + 20);
    func_002FCB00(data + 4, D_001A72DC[0], data[0], data[1]);
    func_0027B858();
    func_00284FC0();
    func_00285338();
    func_0011AEA0(0);
    func_002FC280();
    func_002FC3C0(1);
    func_001244B8(0, 0);
    func_001261F0(0);
    func_002FCDA8();
}

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

typedef struct { char b[8]; } CdClock;
extern int func_001256D8(CdClock *);
extern void func_001163A0(unsigned int);
void func_0026EC58(unsigned short *out)
{
    CdClock clock;
    int first, second, third;
    int low, high;
    int seed;
    func_001256D8(&clock);
    seed = (unsigned char)clock.b[1] + 1;
    seed *= (unsigned char)clock.b[3] + 1;
    seed *= (unsigned char)clock.b[2] + 1;
    seed *= (unsigned char)clock.b[5] + 1;
    seed *= (unsigned char)clock.b[6] + 1;
    func_001163A0(seed);
    func_0026EB98(&first, &second, &third);
    out[0x20/2] = out[0x08/2] = (unsigned short)first + 0xb9;
    out[0x28/2] = out[0x10/2] = (unsigned short)second + 0xb9;
    out[0x30/2] = out[0x18/2] = (unsigned short)third + 0xb9;
    if (func_002A77E0(2)) {
        low = 0xb3;
        high = 0xb6;
    } else {
        low = 0xb6;
        high = 0xb3;
    }
    func_0026EB98(&first, &second, &third);
    out[0x02/2] = (unsigned short)first + low;
    out[0x06/2] = (unsigned short)second + low;
    out[0x0c/2] = (unsigned short)third + low;
    out[0x12/2] = out[0x02/2];
    out[0x16/2] = out[0x06/2];
    out[0x1c/2] = out[0x0c/2];
    out[0x22/2] = out[0x02/2];
    out[0x26/2] = out[0x06/2];
    out[0x2c/2] = out[0x0c/2];
    func_0026EB98(&first, &second, &third);
    out[0x04/2] = (unsigned short)first + high;
    out[0x0a/2] = (unsigned short)second + high;
    out[0x0e/2] = (unsigned short)third + high;
    out[0x14/2] = out[0x04/2];
    out[0x1a/2] = out[0x0a/2];
    out[0x1e/2] = out[0x0e/2];
    out[0x24/2] = out[0x04/2];
    out[0x2a/2] = out[0x0a/2];
    out[0x2e/2] = out[0x0e/2];
}

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

typedef struct { char pad[0x32C]; int base; int offset; int count; } Table26FE48;
extern Table26FE48 D_0014B540;
extern char D_001C5540[];
extern int D_001A730C __attribute__((sda));
extern unsigned long long D_001A7468 __attribute__((sda));
extern int func_002B7C80(char *, int, int);
extern void func_0026FC78(char *, unsigned long long *, int, int);

void func_0026FE48(void)
{
    unsigned long long local[4];
    char *data = D_001C5540;
    func_002B7C80(data, D_0014B540.offset + D_0014B540.base, D_0014B540.count);
    func_0026FC78(data, local, D_001A730C, 0x3FFC00);
    *(volatile unsigned long long *)&D_001A7468 = local[0];
}

void func_0026FEB8(const char *format,...) {
}

INCLUDE_ASM("asm/game", func_0026FEF0);

INCLUDE_ASM("asm/game", func_0026FF08);

INCLUDE_ASM("asm/game", func_00270018);

INCLUDE_ASM("asm/game", func_002701A8);

extern void func_00282B50(void *, void *, int);
extern char *D_001B5390;
extern char D_001B7550[];
extern char D_001B7C30[];

void func_002701B0(void)
{
    func_00282B50(D_001B7550, D_001B5390, 0xA0);
    func_00282B50(D_001B7C30, D_001B7C30 - 0x500, 0x280);
    *(char **)(D_001B7550 + 0x70) = D_001B7C30;
}

/* shared native body of family 34e74db3063e7a24 */
void func_00270210(void)
{
    int i;

    for (i = 0; i < D_L06_001B21C0; i++) {
        D_L06_001B2180[i]();
    }

    D_L06_001B21C0 = 0;
}

extern char D_001B5750[];

/* Finds the record whose halfword at 0x86 equals the argument, in the table of
   0xA0-byte records that starts at 0x1B5750, and returns it; returns 0 once
   the table is exhausted.  The end of the table is held in an int so that the
   comparison is the signed one the retail does (slt, not sltu). */
char *func_00270280(int a0)
{
    char *p = D_001B5750;
    int limit = (int)p + 0x1E00;

    for (;;) {
        if (*(short *)(p + 0x86) != a0) {
            p += 0xA0;
            if ((int)p >= limit) {
                return 0;
            }
        } else {
            return p;
        }
    }
}

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

extern char D_001B53A0[];
extern void *func_00302A20(void *);
extern void func_0029F338(void *);

void func_002704F0(char *p)
{
    char *base = D_001B53A0;
    if (*(short *)(p + 0x86) == 0) {
        if (*(void **)(base + 0xC4) == 0)
            *(void **)(base + 0xC4) = func_00302A20(base - 0x60);
    } else {
        if (*(void **)(base + 0xC4) != 0) {
            func_0029F338(*(void **)(base + 0xC4));
            *(void **)(base + 0xC4) = 0;
        }
    }
}

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

extern void func_00282CF0(float *dst, float *a, float *b);
extern float func_00282DB0(float *a, float *b);
extern float func_00282DF0(float *a);
extern void func_00282F20(float *dst, float *a, float len);
extern float func_002831B0(float x);
extern void func_002ACEC0(float *dst, float *vec, float *axis, float angle);

/* The angles of a target seen from a camera: the difference of the two points is
   split along the axis and across it, the yaw of the first direction around the
   axis and the pitch of the difference after it, with the length of the
   difference beside them. A zero length becomes 0.0001 before the arcsine. */
void func_00270B58(float *out, float *p0, float *p1, float *dir0, float *dir1, float *axis)
{
    float diff[4];
    float proj[4];
    float perp[4];
    float unit[4];
    float rotated[4];
    float d1, d2, d3, d4, d5;
    float lenPerp, lenDiff;
    float angle1, angle2;
    float yaw, pitch;

    func_00282CF0(diff, p0, p1);
    d1 = func_00282DB0(diff, axis);
    func_00282F20(proj, axis, d1);
    func_00282CF0(perp, diff, proj);

    d2 = func_00282DB0(dir0, perp);
    lenPerp = func_00282DF0(perp);
    if (lenPerp == 0.0f) {
        lenPerp = 0.0001f;
    }
    angle1 = func_002831B0(d2 / lenPerp);
    yaw = 1.57079637f - angle1;

    func_00282F20(unit, perp, 1.0f);
    d3 = func_00282DB0(dir1, unit);
    if (d3 < 0.0f) {
        yaw = -yaw;
    }
    out[0] = yaw;

    func_002ACEC0(rotated, dir0, axis, yaw);
    d4 = func_00282DB0(rotated, diff);
    lenDiff = func_00282DF0(diff);
    if (lenDiff == 0.0f) {
        lenDiff = 0.0001f;
    }
    angle2 = func_002831B0(d4 / lenDiff);
    pitch = -(1.57079637f - angle2);

    func_00282F20(unit, diff, 1.0f);
    d5 = func_00282DB0(axis, unit);
    if (d5 < 0.0f) {
        pitch = 1.57079637f - angle2;
    }
    out[1] = pitch;

    out[2] = func_00282DF0(diff);
}

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
