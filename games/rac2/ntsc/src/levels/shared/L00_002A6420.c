#include "include_asm.h"

typedef unsigned char u8;

extern void func_L00_002A61C8(int arg0, int arg1, int arg2, int arg3);

extern void func_002E6CF0(void *routine, void *state);

extern void func_00323270(void *a0, void *a1, void *a2, float f12, float f13, float f14);

extern void func_L00_002A7A50();

extern int func_L00_002B3998_1(float value);

#ifndef RAC2_T_RESIDENTSTATE_FD160FB9C
#define RAC2_T_RESIDENTSTATE_FD160FB9C
typedef struct {
    u8 pad000[0x1b8];
    int f1B8;
    u8 pad1bc[0xc2c - 0x1bc];
    int fC2C;
    u8 padc30[0x149d - 0xc30];
    u8 b149D;
    u8 pad149e[0x2290 - 0x149e];
    u8 *p2290;
    int f2294;
} ResidentState_Fd160fb9c;
#endif

extern ResidentState_Fd160fb9c D_00189E20_16;

extern int D_001A79F0_3 __attribute__((sda));

extern int func_L00_002D67A8_1(int);

extern char D_00189E20_2[];

extern float func_002AA2E8(float *p, float a, float b);

extern char D_00189E20_24[];

extern int func_00283CF0_6(float value);

extern float func_00283CC8(float *buffer, float value);

#ifndef RAC2_T_V4_FBFDC087A
#define RAC2_T_V4_FBFDC087A
typedef int V4_Fbfdc087a __attribute__((mode(TI)));
#endif

extern char D_00189EB0[];

extern void func_00283410_9(float *m, char *p);

extern void func_002830C0_8(float *dst, float *src, float *m);

extern void func_00283788_6(float *m, float *m2, float *e);

extern void func_00282CF0_14(float *c, float *a, float *b);

extern void func_00282CC0_29(char *a, char *b, float *c);

extern void func_002AB6D8_4(float *m, char *p);

extern void func_00282C88_2(void *);

extern void func_L00_002A9FE8_1(void *, float, float, float);

extern unsigned char D_0018C0D4[];

#ifndef RAC2_T_S36_F39E097FB
#define RAC2_T_S36_F39E097FB
typedef struct { int w[9]; } S36_F39e097fb;
#endif

extern int D_0018C0B0_1;

extern char D_0018AA70[];

extern S36_F39e097fb D_L00_001A7F68;

extern void func_L00_003116A8_1(int, int, S36_F39e097fb *, char *);

extern char D_00189E20_20[];

extern float func_002A7878_6(float a0, float a1);

extern void func_002C19D8(float *a0, float *a1, int a2, float a3, float a4);

extern int func_002A77E0_2(char *a0);

extern void func_L00_002D3638(char *target, int mode, float value, float zero, float scale);

extern int func_002761F8_2(char *first, char *second, int mode, int flag, int extra);

extern float func_00282E48_4(char *source, short *table);

extern short D_L00_001BF220_2[];

extern float D_0018A084;


typedef int s32;

typedef float f32;

extern void func_L00_002D36E0(f32, f32, f32, f32 *, s32);

typedef unsigned int u32;

extern u8 D_00189E20_1[];

typedef struct {
    u8 prefix[0x1220];
    u32 field1220;
    u8 gap[0x20];
    s32 field1244, field1248;
} NativeIndexedFieldsView;

typedef struct {
    u8 pad0000[0x2294];
    u32 kind;
    u32 unknown2298;
    u32 mode;
} ResidentFlags2294;

/* Reads a resident mode word and the kind byte of a pointer field, then for kind 3, 4 or 20 tests two float thresholds through a predicate and triggers a four-argument action with selector zero or one. */
void func_L00_002A6420(void)
{
    int selector;

    if (D_00189E20_16.f2294 == 2) {
        if (D_00189E20_16.f1B8 < 15)
            return;
    }

    selector = D_00189E20_16.b149D;

    if (D_00189E20_16.f2294 == 2) {
        if (D_00189E20_16.f1B8 == 22)
            func_L00_002A61C8(selector, 0, 1, 30);
    }

    switch (D_00189E20_16.p2290[67]) {
    case 3:
        if (D_00189E20_16.fC2C != 0)
            return;
        if (func_L00_002B3998_1(49.5f) != 0)
            func_L00_002A61C8(selector, 0, 1, 30);
        if (func_L00_002B3998_1(17.0f) != 0)
            func_L00_002A61C8(selector, 1, 1, 30);
        return;
    case 4:
        if (D_00189E20_16.fC2C != 0)
            return;
        if (func_L00_002B3998_1(12.5f) != 0)
            func_L00_002A61C8(selector, 0, 1, 30);
        if (func_L00_002B3998_1(1.0f) != 0)
            func_L00_002A61C8(selector, 1, 1, 30);
        return;
    case 20:
        if (D_00189E20_16.fC2C != 0)
            return;
        if (func_L00_002B3998_1(8.0f) != 0)
            func_L00_002A61C8(selector, 0, 1, 30);
        if (func_L00_002B3998_1(21.0f) != 0)
            func_L00_002A61C8(selector, 1, 1, 30);
        return;
    }
}

INCLUDE_ASM("asm/levels", func_L00_002A6708);

INCLUDE_ASM("asm/levels", func_L00_002A67F8);

INCLUDE_ASM("asm/levels", func_L00_002A6DD0);

/* reads a global mode value; for 8, 9 or 19 probes two engine queries and returns 1 if either reports true, else 0 */
int func_L00_002A7000(void)
{
    int v = D_001A79F0_3;
    int r = 0;
    if (v >= 8) {
        if (v < 10 || v == 19) {
            if (func_L00_002D67A8_1(1) != 0 || func_L00_002D67A8_1(2) != 0)
                r = 1;
        }
    }
    return r;
}

/* shared native body of family 0be97c76a107 */
void func_L00_002A7070(void)
{
    char *base = D_00189E20_2;
    int i, j;

    for (i = 0; i < 7; i++) {
        for (j = 0; j < 3; j++) {
            char *p = *(char **)(base + 4640 + i * 80);

            if (j == 1)
                p = *(char **)(base + 4644 + i * 80);
            else if (j == 2)
                p = *(char **)(base + 4648 + i * 80);
            if (p != 0)
                *(long long *)(p + 56) = *(long long *)(*(char **)(base + 8848) + 56);
        }
    }
}

void func_L00_002A70F0(void)
{
}

INCLUDE_ASM("asm/levels", func_L00_002A70F8);

INCLUDE_ASM("asm/levels", func_L00_002A74A8);

INCLUDE_ASM("asm/levels", func_L00_002A75D8);

/* runs a three-float setup over two resident blocks when the mode is 124, then publishes a
   height once two resident flags are clear */
void func_L00_002A76B8(void)
{
    char *base = D_00189E20_2;

    if (*(int *)(base + 0x2294) == 124)
        func_00323270(*(void **)(base + 0x2290), base + 0x7B0, base + 0x7B4,
                      0.174532949924469f, 0.02617994323372841f, 0.03490659222006798f);
    if (*(short *)(base + 0x6BC) == 0)
        return;
    if (*(int *)(base + 0x660) != 0)
        return;
    *(float *)(*(char **)(base + 0x2290) + 0xF4) = *(float *)(base + 0x6B0);
}

INCLUDE_ASM("asm/levels", func_L00_002A7748);

INCLUDE_ASM("asm/levels", func_L00_002A7958);

/* raises the pending flag once: registers an update routine with the resident pointer */
void func_L00_002A7A10(void)
{
    char *base = D_00189E20_2;

    if (*(short *)(base + 0x2162) == 0) {
        *(short *)(base + 0x2162) = 1;
        func_002E6CF0(func_L00_002A7A50, *(void **)(base + 0x2290));
    }
}

INCLUDE_ASM("asm/levels", func_L00_002A7A50);

INCLUDE_ASM("asm/levels", func_L00_002A7DE8);


INCLUDE_ASM("asm/levels", func_L00_002A7E60);

INCLUDE_ASM("asm/levels", func_L00_002A9460);

INCLUDE_ASM("asm/levels", func_L00_002A9600);

INCLUDE_ASM("asm/levels", func_L00_002A9760);

/* Clamps two float fields of one object into [-1,1] through a repeated helper call and copies the clamped values into a second object. */
void func_L00_002A9D38(float *a0, float *a1, float f12)
{
    char *base = D_00189E20_2;
    float v[2];

    v[0] = -*(float *)(base + 8112);
    v[1] = *(float *)(base + 8116);
    func_002AA2E8(a1 + 2, v[0], f12);
    if (a1[2] > 1.0f) a1[2] = 1.0f;
    if (a1[2] < -1.0f) a1[2] = -1.0f;
    a0[0] = a1[2];
    func_002AA2E8(a1 + 1, v[1], f12);
    if (a1[1] > 1.0f) a1[1] = 1.0f;
    if (a1[1] < -1.0f) a1[1] = -1.0f;
    a0[1] = a1[1];
}

INCLUDE_ASM("asm/levels", func_L00_002A9E30);

/* blends two table entries by the fractional part of a clock value: the integer part is the index, one step back is wrapped over a count unless a flag selects the last slot, and the result is frac*table[index] + (scale-frac)*table[previous] */
float func_L00_002A9F30(float *table, int count)
{
    char *base = D_00189E20_24;
    float buffer[4];
    float t;
    int index;
    int last;
    int previous;
    index = func_00283CF0_6(*(float *)(base + 3128));
    last = count - 1;
    previous = (index + last) % count;
    if (*(int *)(base + 3112) & 2)
        previous = last;
    t = func_00283CC8(buffer, *(float *)(base + 3128));
    return t * table[index] + (*(float *)(base + 3132) - t) * table[previous];
}

/* Copies a 16-byte block pointed to by the first argument into a stack buffer, stores the three float arguments into a 16-byte stack vector, fills two 64-byte matrices through helper routines, applies copies of the buffer through them into two more 16-byte stack vectors, and finishes by passing the destination, the source and a composed vector to a closing routine. */
void func_L00_002A9FE8(V4_Fbfdc087a *src, float x, float y, float z)
{
    char *p = D_00189EB0;
    float q[4];
    float a[4];
    float b[4];
    float c[4];
    float m[16];
    float e[16];
    float v[4];

    *(V4_Fbfdc087a *)q = *src;
    func_00283410_9(m, p);
    func_002830C0_8(a, q, m);
    v[0] = x;
    v[1] = y;
    v[2] = z;
    func_00283410_9(e, v);
    func_00283788_6(m, m, e);
    func_002830C0_8(b, q, m);
    func_00282CF0_14(c, a, b);
    func_00282CC0_29(p - 16, p - 16, c);
    func_002AB6D8_4(m, p);
}

/* runs a setup routine on a 16-byte stack buffer, selects 0.35f or 0.6f into buffer slot 2 from a flag byte in fixed data, then forwards the buffer with three float arguments */
void func_L00_002AA0F0(float a, float b, float c)
{
    float buf[4];

    func_00282C88_2(buf);

    if (D_0018C0D4[0] == 1)
        buf[2] = 0.35f;
    else
        buf[2] = 0.6f;

    func_L00_002A9FE8_1(buf, a, b, c);
}

/* Copies a 36-byte global record onto the stack, substitutes a default id when the argument is zero, and calls one helper with the id, the count 9 and two fixed global addresses. */
void func_L00_002AA178(int a0)
{
    S36_F39e097fb buf;

    if (a0 == 0)
        a0 = D_0018C0B0_1;
    buf = D_L00_001A7F68;
    func_L00_003116A8_1(a0, 9, &buf, D_0018AA70);
}

INCLUDE_ASM("asm/levels", func_L00_002AA200);

/* Bumps a 16-bit counter in a fixed data block and, when the new value exceeds the caller's threshold, builds a three-float jittered position from two globals plus three random calls, hands it to a spawn helper and stores the helper's return value back into the counter. */
void func_L00_002AA478(char *self, int arg1)
{
    float v[3];
    int n;
    char *p;
    float r;

    p = D_00189E20_20;
    n = *(unsigned short *)(p + 2628) + 1;
    *(unsigned short *)(p + 2628) = n;
    if (arg1 < (short)n) {
        v[0] = *(float *)(p + 128) + func_002A7878_6(-0.05f, 0.05f);
        v[1] = *(float *)(p + 132) + func_002A7878_6(-0.05f, 0.05f);
        v[2] = *(float *)(p + 816);
        r = func_002A7878_6(0.4f, 0.6f);
        func_002C19D8(v, (float *)(p + 816), -1, r, 5250.0f);
        *(unsigned short *)(p + 2628) = func_002A77E0_2(self);
    }
}

INCLUDE_ASM("asm/levels", func_L00_002AA570);

INCLUDE_ASM("asm/levels", func_L00_002AA750);

INCLUDE_ASM("asm/levels", func_L00_002AA938);

INCLUDE_ASM("asm/levels", func_L00_002AABE0);

INCLUDE_ASM("asm/levels", func_L00_002AAE00);

typedef struct {
    char reserved00[0xA4];
    float first;
    float second;
    int reservedAC;
} FloatPairRow;
extern FloatPairRow D_L00_0025BE10[4];

void func_L00_002AB0A8(float first, float second) {
    D_L00_0025BE10[0].first = first;
    D_L00_0025BE10[0].second = second;
}

void func_L00_002AB0C0(float first, float second) {
    D_L00_0025BE10[1].first = first;
    D_L00_0025BE10[1].second = second;
}

void func_L00_002AB0D8(float first, float second) {
    D_L00_0025BE10[2].first = first;
    D_L00_0025BE10[2].second = second;
}

void func_L00_002AB0F0(float first, float second) {
    D_L00_0025BE10[3].first = first;
    D_L00_0025BE10[3].second = second;
}

/* scales and blends two 16-byte samples through a helper, and on a successful comparison writes a table-derived value to the caller's output */
int func_L00_002AB108(float *out, float scale, float amount)
{
    char buffer[32];

    func_L00_002D3638(buffer, 1, D_0018A084 - 0.02f, 0.0f, scale);
    func_L00_002D3638(buffer + 16, 1, amount, 0.0f, scale);
    if (func_002761F8_2(buffer, buffer + 16, 2, 0, 0)) {
        if (out)
            *out = func_00282E48_4(buffer, D_L00_001BF220_2);
        return 1;
    }
    return 0;
}

INCLUDE_ASM("asm/levels", func_L00_002AB1D8);

void func_L00_002AB3E0(void)
{
    char *p = D_00189E20_2;
    int i;

    char *q;

    *(int *)(p + 0x1194) = 0;
    *(int *)(p + 0x1198) = 0;
    q = p;
    for (i = 3; i >= 0; i--) {
        *(int *)(q + 0x11B4) = 0;
        *(int *)(q + 0x11B8) = 0;
        q += 0x10;
    }
}


INCLUDE_ASM("asm/levels", func_L00_002AB420);

INCLUDE_ASM("asm/levels", func_L00_002AB948);

INCLUDE_ASM("asm/levels", func_L00_002ABBA0);

INCLUDE_ASM("asm/levels", func_L00_002AC1A0);

INCLUDE_ASM("asm/levels", func_L00_002AC578);

INCLUDE_ASM("asm/levels", func_L00_002ACA08);

INCLUDE_ASM("asm/levels", func_L00_002ACF88);

INCLUDE_ASM("asm/levels", func_L00_002AD868);

INCLUDE_ASM("asm/levels", func_L00_002ADB40);

/* forwards an output pointer, three pinned floating constants and selector one to the typed helper */
void func_L00_002ADCF8(f32 *output) {
 func_L00_002D36E0(0.3f,0.0f,1.34f,output,1);
}

INCLUDE_ASM("asm/levels", func_L00_002ADD30);

/* returns indexed resident word 0x1220 when field 0x1244 equals two, otherwise zero */
u32 func_L00_002ADE68(s32 index) {
 NativeIndexedFieldsView *p=(NativeIndexedFieldsView *)((u8 *)&D_00189E20_1+index*0x50);
 if(p->field1244==2) return p->field1220;
 return 0;
}

/* returns indexed resident word 0x1248 when field 0x1244 equals two, otherwise minus one */
s32 func_L00_002ADEA0(s32 index) {
 NativeIndexedFieldsView *p=(NativeIndexedFieldsView *)((u8 *)&D_00189E20_1+index*0x50);
 if(p->field1244==2) return p->field1248;
 return -1;
}

/* tests whether the indexed resident status equals 12 or 17 */
s32 func_L00_002ADED8(s32 index) {
 s32 value=func_L00_002ADEA0(index);
 if(value==12 || value==17) return 1;
 return 0;
}

/* tests whether the indexed resident status equals 47 */
s32 func_L00_002ADF10(s32 index) {
 return func_L00_002ADEA0(index)==47;
}

/* the indexed resident status is one of four measured values */
s32 func_L00_002ADF38(s32 index)
{
    s32 value = func_L00_002ADEA0(index);

    if (value == 19 || value == 52 || value == 54 || value == 20)
        return 1;
    return 0;
}


/* tests the measured unsigned resident kind and mode alternatives */
s32 func_L00_002ADF80(void)
{
    ResidentFlags2294 *root = (ResidentFlags2294 *)D_00189E20_1;

    if (root->mode == 17 || root->mode == 18
        || root->kind == 0x67 || root->kind == 0x7f
        || root->kind == 0x73 || root->kind == 0x72) {
        return 1;
    }
    return 0;
}

/* returns the provided value, or object byte 0xa9 when the value is 255 */
u32 func_L00_002ADFD0(u8 *object,u32 value) {
    if(value==255) value=object[0xa9];
    return value;
}

INCLUDE_ASM("asm/levels", func_L00_002ADFE8);


INCLUDE_ASM("asm/levels", func_L00_002AE070);


INCLUDE_ASM("asm/levels", func_L00_002AE0C8);
