#include "include_asm.h"

typedef int s32;

typedef unsigned char u8;

typedef float f32;

/* The campaign: call-bearing bodies reproduced by the reconstructed toolchain
   (2.9-ee + refined gas) -- sd saves in 8-byte slots,
   no sibcall, the s32 return prototype determines v0 versus v1, and || chains
   with a shared exit. Measurements and evidence: docs/COMPILER-NOTES.md. */

/* func_002A77E0 : random draw bounded by the argument */
extern s32 func_001163B0(void);

extern f32 func_002A7940_5(void);

extern f32 func_002A7878_10(f32, f32);

extern void func_002AF000_2(s32, f32, f32, f32);

extern float Fc3394786_FUN_002A7878(void);

extern float Fc3394786_FUN_002A7940(void);

extern float Fc3394786_FUN_00283180(float);

extern float Fc3394786_FUN_00283198(float);

extern f32 func_00283180_3 (f32);

struct family_27f9_object {
    unsigned char reserved[248];
    float heading;
};

extern float F27f9be8e_FUN_00283BE0(float p, float x);

extern float F27f9be8e_FUN_00282C48(float t);

extern float F27f9be8e_FUN_00283B98(float x, float y);

typedef struct { f32 x,y,z,w; } MmiPoint;

/* The 16-byte store is written through a volatile quad pointer. Without the
   qualifier the compiler moves the quad store into the return's delay slot,
   which the retail build does not do; the qualifier is what keeps the store
   ahead of the return. The alternative idiom used by the RAC1 corpus is inline
   assembly, which the candidate gate refuses. */
/* The copy itself goes through a 128-bit integer type, not the aggregate:
   the reconstructed 2.9-ee compiler reaches lq/sq only through TImode (the
   aggregate path builds ld/sd pairs or a memcpy call). Same bytes as the
   retail (`lq $v0,0($a3); sq $v0,0($a0)`). */

typedef struct { int a, b, c, d; } __attribute__((aligned(16))) Quad16;

typedef int TI128 __attribute__((mode(TI)));

#ifndef RAC2_T_V4_F5621621C
#define RAC2_T_V4_F5621621C
typedef int V4_F5621621c __attribute__((mode(TI)));
#endif

extern float F5621621c_FUN_00283248(float a, float b);

extern float F5621621c_FUN_00283180(float a);

extern float F5621621c_FUN_00283198(float a);

extern void F5621621c_FUN_002A8E20(int v, void *rec);

#ifndef RAC2_T_VEC16_F5621621C
#define RAC2_T_VEC16_F5621621C
typedef struct {
    float x;
    float y;
    float z;
    float w;
} Vec16_F5621621c;
#endif

#ifndef RAC2_T_NODE_F5621621C
#define RAC2_T_NODE_F5621621C
typedef struct {
    char pad[16];
    float f16;
    float f20;
} Node_F5621621c;
#endif

#ifndef RAC2_T_REC_F5621621C
#define RAC2_T_REC_F5621621C
typedef struct {
    float f0;
    float f4;
    float f8;
    float f12;
    int i16;
    int i20;
    signed char b24;
    signed char b25;
    unsigned short h26;
    float f28;
    int i32;
} Rec_F5621621c;
#endif

extern float Ff454dd77_FUN_00282C48(float a);

#ifndef RAC2_T_OBJ_FF454DD77
#define RAC2_T_OBJ_FF454DD77
typedef struct {
    float f0;
} Obj_Ff454dd77;
#endif

extern void F8cb0f4c0_FUN_002AA198(float *, float);

extern float F8cb0f4c0_FUN_00282C48(float);

#ifndef RAC2_T_V4_FC6BD459E
#define RAC2_T_V4_FC6BD459E
typedef int V4_Fc6bd459e __attribute__((mode(TI)));
#endif

extern void func_00282CF0_16(float *local, void *a1, void *a0);

extern float func_00282DF0_4(float *local);

extern void func_002AA198_2(void *a2, float r, float f1, float f2, float f3);

extern void func_00282F20_12(float *local, float *local2, float v);

extern void func_00282CC0_32(void *a0, void *a0b, float *local);

extern float func_00282C48_9(float t);

void func_002A7750(u8 *a0,u8 a1,s32 a2) {
 u8 old=a0[0x20];a0[0x20]=a1;a0[0x94]=old;*(short *)(a0+0x96)=0;
 a0[0xbe]&=~1;
 if(a2!=-1){a0[0x95]=(u8)a2;a0[0xbe]&=~2;}
}

/* Measured target: byte-identical across all 27 levels. */
f32 func_002A7790(f32 a0) { return a0 * a0; }

f32 func_002A7798(f32 a0) {
 return 1.0f-(1.0f-a0)*(1.0f-a0);
}

INCLUDE_ASM("asm/game", func_002A77C0);

s32 func_002A77E0(s32 a0) {
    return (func_001163B0() >> 16 & 0x7fff) % a0;
}

s32 func_002A7820(s32 a0, s32 a1) {
    return (func_001163B0() >> 16 & 0x7fff) % ((a1 - a0) + 1) + a0;
}

f32 func_002A7878(f32 a0, f32 a1) {
    s32 r = func_001163B0();
    return a0 + (f32)(r >> 16 & 0x7FFF) * (a1 - a0) * 3.0517578125e-05f;
}

f32 func_002A78D8(f32 a0, f32 a1) {
    s32 r = func_001163B0();
    f32 result = a0 + (f32)(r >> 16 & 0xFFF) * (a1 - a0) * 0.000244140625f;
    if ((r >> 16 & 1) != 0) result = -result;
    return result;
}

f32 func_002A7940(void) {
    return (f32)((func_001163B0() >> 16 & 0xfff) - 0x800) * 0.0015339808f;
}

/* draws two bounded random angles and an interval random value from the two float arguments, then forwards the pointer, the interval draw and both angles to a fourth callee */
void func_002A7988(s32 a0, f32 a1, f32 a2) {
    f32 x = func_002A7940_5();
    f32 y = func_002A7940_5();
    f32 z = func_002A7878_10(a1, a2);
    func_002AF000_2(a0, z, x, y);
}

void func_002A7A00(float *out)
{
    float back = Fc3394786_FUN_002A7878();
    float a = Fc3394786_FUN_002A7940();
    float b = Fc3394786_FUN_002A7940();
    float value;

    value = Fc3394786_FUN_00283180(a) * Fc3394786_FUN_00283198(b);
    value = value * back;
    out[0] = value;
    value = Fc3394786_FUN_00283198(a) * Fc3394786_FUN_00283198(b);
    value = value * back;
    out[1] = value;
    out[2] = Fc3394786_FUN_00283180(b) * back;
}

float func_002A7AA8(float a, float b, float c, float d, float t) {
    float p = (d - c) - (a - b);
    float q = (a - b) - p;
    float t2 = t * t;
    float t3 = t2 * t;
    return p * t3 + q * t2 + (c - a) * t + b;
}

INCLUDE_ASM("asm/game", func_002A7AE0);

/* Eased scalar interpolation between a and b: returns a for t==0.0, b for t==1.0, otherwise a + (b-a)*((1-cos(t*pi))*0.5). */
f32 func_002A7C00 (f32 a, f32 b, f32 t)
{
  f32 u;
  if (t == 0.0f)
    return a;
  if (t == 1.0f)
    return b;
  u = (1.0f - func_00283180_3 (t * 3.1415927f)) * 0.5f;
  return a + (b - a) * u;
}

INCLUDE_ASM("asm/game", func_002A7C98);

float func_002A7CA0(struct family_27f9_object *a0,
                                         float *a1,
                                         float p1, float p2, float p3, float p4)
{
    float t;
    float v;
    float f3;
    float out;

    t = F27f9be8e_FUN_00283BE0(p1, a0->heading);
    v = t * 6.3661971092224121093750e+00f;
    if (v > 1.0f) {
        v = 1.0f;
    } else {
        if (v < -1.0f) {
            v = -1.0f;
        }
    }
    f3 = a1[0];
    a1[0] = f3 + (p2 * v - p3 * f3);
    if (p4 != 0.0f) {
        if (a1[0] > p4) {
            a1[0] = p4;
        } else {
            if (a1[0] < -p4) {
                a1[0] = -p4;
            }
        }
    }
    if (F27f9be8e_FUN_00282C48(t) < a1[0]) {
        a1[0] = F27f9be8e_FUN_00282C48(t);
    } else {
        if (a1[0] < -F27f9be8e_FUN_00282C48(t)) {
            a1[0] = -F27f9be8e_FUN_00282C48(t);
        }
    }
    out = F27f9be8e_FUN_00283B98(a0->heading, a1[0]);
    a0->heading = out;
    return out;
}

extern float func_00283248(float, float);
float func_002A7E08(struct family_27f9_object *object, float *target, float *value,
                   float speed, float damping, float limit)
{
    float angle = func_00283248(target[4] - ((float *)object)[4],
                               target[5] - ((float *)object)[5]);
    return func_002A7CA0(object, value, angle, speed, damping, limit);
}

INCLUDE_ASM("asm/game", func_002A7E90);

INCLUDE_ASM("asm/game", func_002A7EA0);

INCLUDE_ASM("asm/game", func_002A8238);

INCLUDE_ASM("asm/game", func_002A8240);

INCLUDE_ASM("asm/game", func_002A84E0);

extern unsigned short *D_001EE5A0[];
extern char *D_001B1B5C __attribute__((sda));

/* Counts the entries of the 0x7FFF-masked id list at table[index] whose class
   byte (0x20) is not value; an entry with the high bit of the id set ends the
   list, and value -1 counts every entry with a class byte below 0x80. */
int func_002A8508(int index, int value) {
    unsigned short *list;
    unsigned short *entry;
    short key;
    char *object;
    int count;
    if (index == -1)
        return 0;
    list = D_001EE5A0[index];
    count = 0;
    if (list == 0)
        return 0;
    entry = list;
    do {
        object = (char *)(((*entry & 0x7FFF) << 8) + (unsigned int)D_001B1B5C);
        if (object[0x20] >= 0) {
            if (value == -1 || (unsigned char)object[0x20] != value)
                count++;
        }
        key = *entry++;
    } while (key >= 0);
    return count;
}

#define MACRO_ADDR __attribute__((section(".sdata")))

extern unsigned short *r1_D_L01_001ABFC0[];

extern char *r1_D_L01_00160058 MACRO_ADDR;

/* rac1-decomp src/overlays/shared/mobyutil_0026E8E0.c: func_L01_0026F040 */
// Set byte 0xBC of each entry in a negative-terminated index list.
void func_002A8598(int i, int v) {
    unsigned short *t = r1_D_L01_001ABFC0[i];
    unsigned short *p;
    if (t != 0) {
        p = t;
        do {
            r1_D_L01_00160058[(*p & 0x7FFF) * 256 + 0xBC] = v;
        } while (*(short *)p++ >= 0);
    }
}

INCLUDE_ASM("asm/game", func_002A85F0);

INCLUDE_ASM("asm/game", func_002A8600);

INCLUDE_ASM("asm/game", func_002A86E8);

INCLUDE_ASM("asm/game", func_002A8850);

int func_002A8860(int *p, int b) {
    int w = *p;
    int v = (w >> 24) - b;
    if (v < 0) v = 0;
    *p = (w & 0xFFFFFF) | (v << 24);
    return v == 0;
}

INCLUDE_ASM("asm/game", func_002A8898);

INCLUDE_ASM("asm/game", func_002A88A0);

INCLUDE_ASM("asm/game", func_002A89A8);

extern void func_002A89A8(void *, int, float);

void func_002A8A20(void *object) {
    func_002A89A8(object, 32, 0.5f);
}

INCLUDE_ASM("asm/game", func_002A8A48);

s32 func_002A8A50(const f32 *a0,const MmiPoint *a1,s32 a2) {
    s32 i;
    for(i=0;i<a2;i++) {
        f32 x=a1[i].x,y=a1[i].y;
        if((a1[(i+1)%a2].x-x)*(a0[1]-y)-(a1[(i+1)%a2].y-y)*(a0[0]-x)>0.0f) return i+1;
    }
    return 0;
}

int func_002A8AF0(float *p, float *v, int n) {
    int r = 0;
    int i = 0;
    int k;
    for (k = 0; k < n; k++) {
        float y1 = v[k * 4 + 1];
        i++;
        if (i == n) i = 0;
        if ((y1 < p[1] && p[1] <= v[i * 4 + 1]) || (v[i * 4 + 1] < p[1] && p[1] <= y1)) {
            if (v[k * 4] + (p[1] - v[k * 4 + 1]) / (v[i * 4 + 1] - v[k * 4 + 1]) * (v[i * 4] - v[k * 4]) < p[0]) {
                r = !r;
            }
        }
    }
    return r;
}

INCLUDE_ASM("asm/game", func_002A8BC0);

void func_002A8BD0(char *a0, int a1, int a2, float a3) { extern void func_00282C88(void *); *(int *)(a0 + 0x10) = a1; *(int *)(a0 + 0x14) = a2; *(float *)(a0 + 0x1C) = a3; *(int *)(a0 + 0x20) = 0; func_00282C88(a0); }

void func_002A8C00(char *a, int b, int c, float d, Quad16 *q) {
    *(int *)(a + 0x10) = b;
    *(int *)(a + 0x14) = c;
    *(float *)(a + 0x1C) = d;
    *(int *)(a + 0x20) = 1;
    *(volatile TI128 *)a = *(TI128 *)q;
}

INCLUDE_ASM("asm/game", func_002A8C28);

extern void func_0029FCC0(void *, void *, float *);
extern int func_002A8C28(void *, float, float *, int, float, float, int, int, int);

int func_002A8D70(float x, float y, float z, void *object, void *source, int mode, int first, int second, int third)
{
    float vector[4];
    func_0029FCC0(object, source, vector);
    return func_002A8C28(object, x, vector, mode, y, z, first, second, third);
}

INCLUDE_ASM("asm/game", func_002A8E18);

INCLUDE_ASM("asm/game", func_002A8E20);

INCLUDE_ASM("asm/game", func_002A8F70);

INCLUDE_ASM("asm/game", func_002A90C8);

INCLUDE_ASM("asm/game", func_002A90F0);

INCLUDE_ASM("asm/game", func_002A90F8);

extern void func_002837D0(void *, void *, void *);
extern void func_00283898(void *, int, float);

void func_002A91F0(void *a0, float *a1) {
    float v2[4];
    float v1[4];
    float v0[4];
    func_00283898(v0, 0, a1[0]);
    func_00283898(v1, 1, a1[1]);
    func_00283898(v2, 2, a1[2]);
    func_002837D0(a0, v0, v1);
    func_002837D0(a0, a0, v2);
}

INCLUDE_ASM("asm/game", func_002A9288);

INCLUDE_ASM("asm/game", func_002A93B0);

extern void func_002A93B0(void *);

void func_002A9548(void *object) {
    func_002A93B0(object);
}

extern int func_00283D38(int, int, float);

int func_002A9568(int first, int second, float amount) {
    return func_00283D38(first, second, amount);
}



INCLUDE_ASM("asm/game", func_002A9640);

INCLUDE_ASM("asm/game", func_002A9698);

INCLUDE_ASM("asm/game", func_002A96A0);

void func_002A9850(void *a0, Vec16_F5621621c *src, int *p, int n,
                                        int t0, int t1, int t2, int t3,
                                        float f1, float f2, float f3)
{
    Vec16_F5621621c vec;
    Vec16_F5621621c *vp = &vec;
    Rec_F5621621c rec;
    int i;
    int *q;

    *(V4_F5621621c *)&vec = *(V4_F5621621c *)src;
    if (n <= 0) return;
    for (q = p, i = n; i != 0; i--, q++) {
        Node_F5621621c *v = (Node_F5621621c *)*q;
        float t;
        if ((int)*q == t0) continue;
        rec.i16 = (int)a0;
        rec.h26 = *(unsigned short *)((char *)a0 + 170);
        rec.i20 = t1;
        rec.f28 = f1;
        rec.i32 = t1;
        rec.b24 = t2;
        rec.b25 = t3;
        t = F5621621c_FUN_00283248(v->f16 - vp->x, v->f20 - vp->y);
        rec.f0 = F5621621c_FUN_00283180(t) * f2;
        rec.f4 = F5621621c_FUN_00283198(t) * f2;
        rec.f8 = f3;
        rec.f12 = 5.6279248046875000000000e3f;
        F5621621c_FUN_002A8E20(*q, &rec);
    }
}

INCLUDE_ASM("asm/game", func_002A99A0);

INCLUDE_ASM("asm/game", func_002AA130);

f32 func_002AA140(f32 a, f32 b, f32 t) { return a + (b - a) * t; }

float func_002AA150(float a0, float a1, float a2)
{
    return F27f9be8e_FUN_00283B98(a0,
            F27f9be8e_FUN_00283BE0(a1, a0) * a2);
}

void func_002AA198(Obj_Ff454dd77 *o, float a, float b, float c, float d)
{
    if (Ff454dd77_FUN_00282C48(a) < o->f0)
        o->f0 = Ff454dd77_FUN_00282C48(a);
    else if (o->f0 < -Ff454dd77_FUN_00282C48(a))
        o->f0 = -Ff454dd77_FUN_00282C48(a);

    o->f0 = o->f0 + (b * a - c * o->f0);
    if (0.0f < d) {
        if (d < o->f0)
            o->f0 = d;
        else if (o->f0 < -d)
            o->f0 = -d;
    }

    if (Ff454dd77_FUN_00282C48(a) < o->f0)
        o->f0 = Ff454dd77_FUN_00282C48(a);
    else if (o->f0 < -Ff454dd77_FUN_00282C48(a))
        o->f0 = -Ff454dd77_FUN_00282C48(a);
}

f32 func_002AA2E8(f32 *value, f32 target, f32 step) {
    f32 diff = target - *value;
    if (step < diff) diff = step;
    else if (diff < -step) diff = -step;
    *value = *value + diff;
    return Ff454dd77_FUN_00282C48(target - *value);
}

f32 func_002AA340(s32 *a0, s32 a1, s32 a2)
{
    s32 value;
    s32 step = a1 - *a0;
    if (a2 < step)
        step = a2;
    else if (step < -a2)
        step = -a2;
    value = *(volatile s32 *)a0 + step;
    *a0 = value;
    return (f32)func_00282C30(a1 - value);
}

INCLUDE_ASM("asm/game", func_002AA3A0);

float func_002AA3A8(float *a0, float *a1, float f1, float f2,
                                         float f3, float f4)
{
    float s = f1;
    float u = f4;
    float t;
    float r;

    F8cb0f4c0_FUN_002AA198(a1, s - a0[0]);
    t = s - (a0[0] + a1[0]);
    a0[0] = a0[0] + a1[0];
    r = F8cb0f4c0_FUN_00282C48(t);
    u = u * 0.01f;
    if (r < u) {
        a0[0] = s;
        a1[0] = 0.0f;
        return a1[0];
    }
    return t;
}

/* shared native family */
float func_002AA458(void *a0, void *a1, void *a2, float f1, float f2, float f3)
{
    float local[4];
    float r;
    float u;

    func_00282CF0_16(local, a1, a0);
    r = func_00282DF0_4(local);
    func_002AA198_2(a2, r, f1, f2, f3);
    func_00282F20_12(local, local, *(float *)a2);
    func_00282CC0_32(a0, a0, local);
    r = r - *(float *)a2;
    u = func_00282C48_9(r);
    if (!(u < f3 * 0.01f))
        return r;
    *(V4_Fc6bd459e *)a0 = *(V4_Fc6bd459e *)a1;
    *(float *)a2 = 0.0f;
    return *(float *)a2;
}

INCLUDE_ASM("asm/game", func_002AA548);

extern float func_00283BE0(void);
extern float func_00282C48(float);

f32 func_002AA738(s32 a0)
{
    f32 base = func_00283BE0();

    if (a0 == 0 || base * (f32)a0 > 0.0f)
        return base;
    {
        f32 zero = 0.0f;

        if (func_00282C48(base) <= 0.001745329471305012702941895f)
            return zero;
        if (base > zero)
            return base - 6.2831859588623046875f;
        return base + 6.2831859588623046875f;
    }
}
