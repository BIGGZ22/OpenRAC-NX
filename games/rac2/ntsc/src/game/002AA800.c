#include "include_asm.h"

extern float func_002AA738 (float x, int a1, float y);

extern float func_00283B98_14 (float x, float y);

extern unsigned int func_00283CF0_5(float x);

extern void Fa5914967_FUN_00282F20(float *a, float *b, float c);

extern void func_00282CF0_13(float *, int, int);

extern void func_00283698_4(float *, int);

extern void func_002830C0_7(float *, float *, float *);

typedef int s32;

typedef unsigned char u8;

/* lot14 -- measured bodies with sizes recorded in the catalogue. */
extern u8 D_0018C0B0[];

extern float func_00283198_7(float x);

extern void func_00282D30_18(void *a0, int a1, float x);

extern float func_00283180_8(float x);

#ifndef RAC2_T_V4_F876460D6
#define RAC2_T_V4_F876460D6
typedef int V4_F876460d6 __attribute__((mode(TI)));
#endif

extern void func_00282C88_3(char *p);

extern float func_00283248_6(float a, float b);

extern void func_00283410_7(char *dst, char *src);

extern void func_00283788_4(char *a, char *b, char *c);

extern void func_00283430(char *dst, char *src);

extern float F500f9762_FUN_00283CE0(int index);

extern int F500f9762_FUN_00283CF0(float value);

extern void F500f9762_FUN_002A04B8(int first, char *a, char *b, char *c);

extern int D_L02_001B359C __attribute__((sda));

extern float func_002A7878(float, float);

#ifndef RAC2_T_V4_F904CC63B
#define RAC2_T_V4_F904CC63B
typedef __attribute__((mode(TI))) int V4_F904cc63b;
#endif

extern char *F904cc63b_FUN_002AD0B0(char *p);

extern void F904cc63b_FUN_00283410(V4_F904cc63b *dst, char *src);

extern void F904cc63b_FUN_00282CC0(V4_F904cc63b *dst, V4_F904cc63b *a, char *b);

extern void F904cc63b_FUN_00282CF0(V4_F904cc63b *dst, V4_F904cc63b *a, char *b);

extern void F904cc63b_FUN_00283698(V4_F904cc63b *dst, V4_F904cc63b *src);

extern void F904cc63b_FUN_00283098(V4_F904cc63b *dst, V4_F904cc63b *a, V4_F904cc63b *b);

/* Family 0e7bb6a8908d30dc -- 244 bytes, 28 placements (1 boot + 27 levels). */

extern char *F0e7bb6a8_FUN_002AD0B0(char *a);

extern void F0e7bb6a8_FUN_00283410(char *dst, char *src);

extern void F0e7bb6a8_FUN_00283698(char *dst, char *src);

extern void F0e7bb6a8_FUN_00282CF0(char *dst, char *a, char *b);

extern void F0e7bb6a8_FUN_002830C0(char *a, char *b, char *c);

extern void F0e7bb6a8_FUN_00283788(char *a, char *b, char *c);

extern void F0e7bb6a8_FUN_002AB6D8(char *a, char *b);

extern float func_00283B98_11(float x);

extern float func_00283198_13(float x);

extern float func_00283198_8(float x);

extern float func_00283180_9(float x);

extern float func_00283B98_8(float x, float y);

extern float func_00283180_10(float);

extern float func_00283198_10(float);

extern float Fb5729173_FUN_00282E48(char *a, char *b);

extern float Fb5729173_FUN_00283248(float x, float y);

extern float Fb5729173_FUN_00283C80(float x, float y);

extern float Fb5729173_FUN_00282E80(char *a, char *b);

extern int Fb5729173_FUN_002ABB78(char *a);

extern void F10bf3d29_FUN_00283698(char *a, char *b);

extern void F10bf3d29_FUN_00283098(char *a, char *b, char *c);

/* clamps a float read through a pointer to plus/minus the second float argument, passes it through two helpers and stores the result back */
void func_002AA800 (float *p, int a1, float fx, float fl)
{
  float v;

  v = func_002AA738 (fx, a1, p[0]);
  if (fl < v)
    v = fl;
  else if (v < -fl)
    v = -fl;
  v = func_00283B98_14 (p[0], v);
  p[0] = v;
  func_002AA738 (fx, a1, v);
}

INCLUDE_ASM("asm/game", func_002AA898);

INCLUDE_ASM("asm/game", func_002AAA00);

INCLUDE_ASM("asm/game", func_002AAC80);

/* Scales four float arguments by 255, converts each through the same routine, masks each result to a byte and packs the four bytes into one word, low byte first. */
int func_002AAE98(float r, float g, float b, float a)
{
    unsigned int cr = func_00283CF0_5(r * 255.0f) & 0xff;
    unsigned int cg = func_00283CF0_5(g * 255.0f) & 0xff;
    unsigned int cb = func_00283CF0_5(b * 255.0f) & 0xff;
    unsigned int ca = func_00283CF0_5(a * 255.0f) & 0xff;
    unsigned int packed = cr;

    packed |= cg << 8;
    packed |= cb << 16;
    packed |= ca << 24;
    return packed;
}

void func_002AAF40(int *a, int *b, int *c, int mask) {
    int x, y;
    if (mask & 1) { x = *b; y = *a; *a = x; *b = y; }
    if (mask & 2) { x = *c; y = *b; *b = x; *c = y; }
    if (mask & 4) { x = *a; y = *c; *c = x; *a = y; }
}

/* Scales four float arguments by 255, converts each through the same routine, masks each result to a byte and packs the four bytes into one word, low byte first. */


int func_002AAFA0(unsigned int color, int mask)
{
    int tmp[3];
    int mid;
    unsigned int a;
    int result;

    if (mask == 0)
        return color;

    tmp[0] = color & 0xFF;
    mid = color & 0xFF00;
    tmp[1] = mid >> 8;
    tmp[2] = (color >> 16) & 0xFF;
    a = color >> 24;
    func_002AAF40(tmp, tmp + 1, tmp + 2, mask);
    result = a << 24;
    result |= tmp[2] << 16;
    result |= tmp[1] << 8;
    result |= tmp[0];
    return result;
}

void func_002AB028(float (*m)[4])
{
    int k;
    int i;
    float v[4];

    for (k = 0; k < 3; k++) {
        v[3] = 0.0f;
        for (i = 0; i < 3; i++)
            v[i] = m[i][k];
        Fa5914967_FUN_00282F20(v, v, 1.0f);
        for (i = 0; i < 3; i++)
            m[i][k] = v[i];
    }
}

/* Initialises a 96-byte stack workspace, calls one routine on its first window with two of the incoming integers, fills the third window from a second routine, combines the three windows with a third routine, and returns one float of the middle window indexed by the fourth integer. */
float func_002AB0E8(int a0, int a1, int a2, int a3)
{
	float v[24];
	float *m = v + 4;

	func_00282CF0_13(v, a2, a0);
	func_00283698_4(v + 8, a1);
	func_002830C0_7(m, v, v + 8);
	return m[a3];
}

extern void func_00282F20(float *, float);
extern float func_00282DB0(void *, void *);
extern void func_00282D30(void *, void *, float);
extern void func_00282CF0(void *, void *, void *);

void func_002AB168(float *out, float *vector, float *axis, float amount)
{
    float scaled[4];
    float unit[4];
    float dot;
    ((void (*)(float *, float *, float))func_00282F20)(unit, axis, 1.0f);
    dot = func_00282DB0(unit, vector);
    func_00282D30(scaled, unit, dot * amount);
    func_00282CF0(out, vector, scaled);
}

s32 func_002AB1F0(u8 *a0) {
 if(a0==0) return 0;
 if((*(unsigned short *)(a0+0x34)&0x20)!=0) return (*(s32 **)(a0+0x68))[0];
 return 0;
}

s32 func_002AB220(u8 *a0) {
 if(a0==0) return 0;
 if((*(unsigned short *)(a0+0x34)&0x20)!=0) return (*(s32 **)(a0+0x68))[4];
 return 0;
}

INCLUDE_ASM("asm/game", func_002AB250);

INCLUDE_ASM("asm/game", func_002AB378);

extern void func_00283658(void *, void *);
extern void func_002AB378(void *, void *);
extern void func_00283678(void *, void *);

void func_002AB600(void *a0, void *a1) {
    unsigned char local[0x40];
    func_00283658(local, a1);
    func_002AB378(a0, local);
    func_00283678(a1, local);
}

void func_002AB650(u8 *a0) {
    u8 *p = *(u8 **)D_0018C0B0;
    *(long *)(a0 + 0x38) = *(long *)(p + 0x38);
}

/* Halves the float argument and calls one helper with it, forwards the second and first arguments plus that helper result to a second helper, and stores a third helper result into the field at offset 12 of the first argument. */
void func_002AB668(void *a0, int a1, float x)
{
    float h = x * 0.5f;
    float t = func_00283198_7(h);

    func_00282D30_18(a0, a1, t);
    *(float *)((char *)a0 + 12) = func_00283180_8(h);
}

INCLUDE_ASM("asm/game", func_002AB6D0);

/* Copies 64 bytes of a source record onto the stack, normalises the last element of the copy to 1.0, queries three pairwise float angles from the copy after two scratch-matrix builds, and writes the three angles into the caller's output record. */
void func_002AB6D8(float *src, float *out)
{
    V4_F876460d6 tmp[4];
    float m[16];
    float local[4];
    float a, b, c;

    *(V4_F876460d6 *)(m + 0) = *(V4_F876460d6 *)(src + 0);
    *(V4_F876460d6 *)(m + 4) = *(V4_F876460d6 *)(src + 4);
    *(V4_F876460d6 *)(m + 8) = *(V4_F876460d6 *)(src + 8);
    *(V4_F876460d6 *)(m + 12) = *(V4_F876460d6 *)(src + 12);
    func_00282C88_3((char *)(m + 12));
    m[15] = 1.0f;
    a = func_00283248_6(m[0], m[1]);
    local[0] = 0.0f;
    local[1] = 0.0f;
    local[2] = -a;
    func_00283410_7((char *)tmp, (char *)local);
    func_00283788_4((char *)m, (char *)tmp, (char *)m);
    b = func_00283248_6(m[0], -m[2]);
    local[0] = 0.0f;
    local[2] = 0.0f;
    local[1] = -b;
    func_00283430((char *)tmp, (char *)local);
    func_00283788_4((char *)m, (char *)tmp, (char *)m);
    c = func_00283248_6(m[5], m[6]);
    out[2] = a;
    out[1] = b;
    out[0] = c;
    local[1] = 0.0f;
    local[0] = -c;
    local[2] = 0.0f;
}

void func_002AB800(int first, char *object)
{
    char *s0 = object;
    short s1 = *(short *)(s0 + 0);
    unsigned short total;
    float limit;
    int result;

    if (s1 != 0 && *(short *)(s0 + 2) == 0)
        return;
    total = *(unsigned short *)(s0 + 12);
    *(short *)(s0 + 2) = 0;
    *(short *)(s0 + 0) = total;
    if (s1 != 0) {
        limit = F500f9762_FUN_00283CE0(*(short *)(s0 + 14));
        result = F500f9762_FUN_00283CF0((float)*(short *)(s0 + 12) * ((float)s1 / limit));
        *(short *)(s0 + 0) = (short)result;
        if ((short)result <= 0)
            *(short *)(s0 + 0) = 1;
    } else {
        int a, b, c;
        F500f9762_FUN_002A04B8(first, (char *)&a, (char *)&b, (char *)&c);
        *(char *)(s0 + 4) = (unsigned char)a;
        *(char *)(s0 + 5) = (unsigned char)b;
        *(char *)(s0 + 6) = (unsigned char)c;
    }
}

INCLUDE_ASM("asm/game", func_002AB8C0);

INCLUDE_ASM("asm/game", func_002ABB10);

INCLUDE_ASM("asm/game", func_002ABB18);

INCLUDE_ASM("asm/game", func_002ABB70);

extern char *g_mobyTableBase __attribute__((sda));
extern char *g_mobyTableEnd __attribute__((sda));

int func_002ABB78(char *object)
{
    if (object != 0)
        goto check;
fail:
    return 0;
check:
    if ((unsigned int)object < (unsigned int)g_mobyTableBase)
        goto fail;
    if ((unsigned int)g_mobyTableEnd < (unsigned int)object)
        goto fail;
    return (unsigned int)(*(unsigned short *)(object + 0xAA) - 500) < 41;
}

INCLUDE_ASM("asm/game", func_002ABBB8);

INCLUDE_ASM("asm/game", func_002AC728);

INCLUDE_ASM("asm/game", func_002AC9F0);

extern void func_00282F20(float *, float);
extern float func_00282DF0(float *);
void func_002AC9F8(float *p, float threshold) { if (threshold < func_00282DF0(p)) ((void (*)(float *, float *, float))func_00282F20)(p, p, threshold); }

INCLUDE_ASM("asm/game", func_002ACA48);

extern int D_L02_001B359C;

/* The subtraction uses the corresponding global in each program. */
void func_002ACA50(int param_1, short *param_2, short param_3)
{
    int i;

    param_1 = (param_1 - D_L02_001B359C) << 8 >> 16;
    for (i = 1; i <= param_2[0]; i++) {
        if (param_2[i] == param_1) return;
    }
    if (param_2[0] < param_3) {
        param_2[0] = param_2[0] + 1;
        param_2[param_2[0]] = param_1;
    }
}

/* shared native body of family bef45185f2ede0ad */
void func_002ACAD0(int param_1, unsigned short *param_2)
{
    int i;

    for (i = 1; i <= (short)param_2[0]; i++) {
        if (D_L02_001B359C + ((short)param_2[i] << 8) == param_1) {
            param_2[i] = param_2[(short)param_2[0]];
            param_2[0] = param_2[0] - 1;
            return;
        }
    }
}

/* Adds the result of a two-argument helper, called three times with the same arguments, into each of three consecutive floats of a vector. */
void func_002ACB48(float *vector, float scale)
{
    float negative = -scale;

    vector[0] = vector[0] + func_002A7878(negative, scale);
    vector[1] = vector[1] + func_002A7878(negative, scale);
    vector[2] = vector[2] + func_002A7878(negative, scale);
}

extern char *D_001B1C4C __attribute__((sda));
extern void func_00283098(char *, char *, char *);

int func_002ACBC8(void *point, int index)
{
    float local[4];
    float transformed[4];
    char *record;

    if (index != -1) {
        record = D_001B1C4C + (index << 7);
        func_00282CF0(local, point, record + 0x30);
        local[3] = 0.0f;
        func_00283098((char *)transformed, (char *)local, record + 0x40);
        if (transformed[0] >= -1.0f && transformed[0] <= 1.0f &&
            transformed[1] >= -1.0f && transformed[1] <= 1.0f &&
            transformed[2] >= -1.0f && transformed[2] <= 1.0f)
            return 1;
    }
    return 0;
}

INCLUDE_ASM("asm/game", func_002ACCA0);

INCLUDE_ASM("asm/game", func_002ACCA8);

INCLUDE_ASM("asm/game", func_002ACD30);

typedef struct { int selected, reserved04, reserved08, count; } MatchRange;
extern MatchRange ref_L00_002ACD38_00;
extern float ref_L00_002ACD38_01[][4];
extern int func_00283D80(void *, void *, void *);
int func_002ACD38(void *object)
{
    int found = 0;
    int i;
    for (i = ref_L00_002ACD38_00.count - 1; i >= 0; i--) {
        if (func_00283D80(object, ref_L00_002ACD38_01[i], ref_L00_002ACD38_01[i + 2])) {
            found = i + 1;
            break;
        }
    }
    return found == ref_L00_002ACD38_00.selected;
}

INCLUDE_ASM("asm/game", func_002ACDC8);

INCLUDE_ASM("asm/game", func_002ACDE8);

extern void func_00282F20(float *p, float x);
extern float func_00282C10(float x);

void func_002ACE78(float *p, float x) { func_00282F20(p, x); p[3] = func_00282C10(1.0f - x * x); }

INCLUDE_ASM("asm/game", func_002ACEC0);

INCLUDE_ASM("asm/game", func_002ACF68);

INCLUDE_ASM("asm/game", func_002AD0A8);

s32 func_002AD0B0(u8 *a0) {
 if(a0==0) return 0;
 if((*(unsigned short *)(a0+0x34)&0x20)==0) return 0;
 return (*(s32 **)(a0+0x68))[2];
}

INCLUDE_ASM("asm/game", func_002AD0E0);

INCLUDE_ASM("asm/game", func_002AD250);

int func_002AD330(char *a0, char *a1, char *a2, char *a3)
{
    V4_F904cc63b b0[4];
    V4_F904cc63b b1[1];
    V4_F904cc63b b2[4];
    V4_F904cc63b b3[4];
    char *p;

    p = F904cc63b_FUN_002AD0B0(a1);
    if (p == 0)
    {
        *(V4_F904cc63b *)a3 = 0;
        return 0;
    }
    F904cc63b_FUN_00283410(b0, p);
    F904cc63b_FUN_00282CC0(b1, (V4_F904cc63b *)a2, p + 16);
    F904cc63b_FUN_00282CF0(b1, b1, a1 + 16);
    if (*(int *)(p + 60) & 2)
    {
        F904cc63b_FUN_00283410(b3, p + 32);
        F904cc63b_FUN_00283698(b2, b3);
        F904cc63b_FUN_00283098(b1, b1, b2);
        F904cc63b_FUN_00283098(b1, b1, (V4_F904cc63b *)(a1 + 192));
    }
    else
    {
        F904cc63b_FUN_00283098(b1, b1, b0);
    }
    F904cc63b_FUN_00282CC0(b1, b1, a1 + 16);
    F904cc63b_FUN_00282CF0((V4_F904cc63b *)a3, b1, a2);
    return 1;
}

INCLUDE_ASM("asm/game", func_002AD470);

int func_002AD5F8(char *unused, char *obj, char *arg2, char *arg3, char *arg4, char *arg5)
{
    char buf0[64];
    char buf1[16];
    char buf2[64];
    char *p;

    p = F0e7bb6a8_FUN_002AD0B0(obj);
    if (p == 0)
        return 0;
    if (*(int *)(p + 60) & 0x40) {
        F0e7bb6a8_FUN_00283410(buf0, obj + 240);
        F0e7bb6a8_FUN_00283698(buf0, buf0);
    } else {
        F0e7bb6a8_FUN_00283698(buf0, obj + 192);
    }
    F0e7bb6a8_FUN_00282CF0(buf1, arg2, obj + 16);
    F0e7bb6a8_FUN_002830C0(arg4, buf1, buf0);
    F0e7bb6a8_FUN_00283410(buf2, arg3);
    F0e7bb6a8_FUN_00283788(buf2, buf0, buf2);
    F0e7bb6a8_FUN_002AB6D8(buf2, arg5);
    return 1;
}

INCLUDE_ASM("asm/game", func_002AD6F0);

INCLUDE_ASM("asm/game", func_002AD7B8);

INCLUDE_ASM("asm/game", func_002AD860);

INCLUDE_ASM("asm/game", func_002AD980);

INCLUDE_ASM("asm/game", func_002ADB78);

INCLUDE_ASM("asm/game", func_002AE728);

INCLUDE_ASM("asm/game", func_002AE730);

INCLUDE_ASM("asm/game", func_002AE838);

INCLUDE_ASM("asm/game", func_002AE8C0);

INCLUDE_ASM("asm/game", func_002AEAE0);

extern float func_0029F530(void *);
extern float func_002AEAE0(int, float);
int func_002AEB60(char *object, float threshold)
{
    float current = func_002AEAE0(4, func_0029F530(object));
    float difference = current - threshold;
    float step = func_002AEAE0(4, *(float *)(object + 0x48) * *(float *)(object + 0x4C));
    if (threshold <= current)
        return difference < step;
    return 0;
}

INCLUDE_ASM("asm/game", func_002AEBF0);

void func_002AEC18(void *a0, int a1) { extern void func_002A0488(void *, unsigned char, unsigned char, unsigned char); int high = a1 >> 16; int middle = a1 >> 8; func_002A0488(a0, (unsigned char)a1, (unsigned char)middle, (unsigned char)high); }

INCLUDE_ASM("asm/game", func_002AEC48);

/* Branches on a float argument: when it is positive it rewrites a float in place with a helper result and sets a field at offset 24 to (helper2 * scale) + offset; otherwise it subtracts a float through a third pointer, rewrites that pointer with (helper2 * scale) and adds it back to the field. */
void func_002AEE70(float *a0, float *a1, float *a2,
                                        float b, float unused, float c)
{
    if (c > 0.0f) {
        float r = func_00283B98_11(*a1);

        *a1 = r;
        a0[6] = func_00283198_13(r) * b + c;
    } else {
        float r = func_00283B98_11(*a1);

        *a1 = r;
        a0[6] = a0[6] - *a2;
        *a2 = func_00283198_13(*a1) * b;
        a0[6] = a0[6] + *a2;
    }
}

/* Scales two owner words by sampled unit-vector factors derived from two float inputs, then normalises each input in place. */
void func_002AEF28(float *out, float *p, float *q,
                                        float k1, float k2, float k3)
{
    out[60] = k1 * func_00283198_8(*p) * func_00283198_8(*q);
    out[61] = k1 * func_00283198_8(*p) * func_00283180_9(*q);
    *p = func_00283B98_8(*p, k2);
    *q = func_00283B98_8(*q, k3);
}

INCLUDE_ASM("asm/game", func_002AEFF0);

/* Writes three floats built from two one-float helpers and the three incoming floats: f(b)*a*f(c), g(b)*a*f(c) and g(c)*a. */
void func_002AF000(float *p, float a, float b, float c)
{
    p[0] = func_00283180_10(b) * a * func_00283180_10(c);
    p[1] = func_00283198_10(b) * a * func_00283180_10(c);
    p[2] = func_00283198_10(c) * a;
}

INCLUDE_ASM("asm/game", func_002AF0A0);

INCLUDE_ASM("asm/game", func_002AF0A8);

INCLUDE_ASM("asm/game", func_002AF1D0);

float func_002AF2E8(char *a0, char *a1, int *a2,
                                       float p1, float p2, float p3, float p4)
{
    float best = p4;
    float hi = p3;
    float radius;
    float reach = p2;
    float far;
    float near;
    float tmp;

    *a2 = 0;
    far = Fb5729173_FUN_00282E48(a0, a1 + 16);
    if (reach < far) {
        *a2 = 1;
    }

    reach = 0.0f;
    radius = Fb5729173_FUN_00283C80(Fb5729173_FUN_00283248(((float *)(a1 + 16))[0] - ((float *)a0)[0],
                             ((float *)(a1 + 16))[1] - ((float *)a0)[1]), p1);
    near = Fb5729173_FUN_00282E80(a0, a1 + 16);
    tmp = Fb5729173_FUN_00283C80(Fb5729173_FUN_00283248(near, ((float *)(a1 + 16))[2] - ((float *)a0)[2]), 0.0f);

    if (reach < hi) {
        if (hi < radius) {
            *a2 = 1;
        }
    }
    if (0.0f < best) {
        if (best < tmp) {
            *a2 = 1;
        }
    }

    reach = far + radius * far;
    if (Fb5729173_FUN_002ABB78(a1) != 0) {
        reach = reach + 8.0f;
    }
    return reach;
}

INCLUDE_ASM("asm/game", func_002AF460);

INCLUDE_ASM("asm/game", func_002AF580);

INCLUDE_ASM("asm/game", func_002AFD70);

void func_002AFD88(float x, float y, float z, char *a0, char *a1)
{
    float buf[3];
    buf[0] = x;
    buf[1] = y;
    buf[2] = z;
    F10bf3d29_FUN_00283098((char *)buf, (char *)buf, a0 + 0xC0);
    F904cc63b_FUN_00282CC0((V4_F904cc63b *)a1, (V4_F904cc63b *)a1, (char *)buf);
}

void func_002AFDD8(char *a0, char *a1, char *a2, char *a3)
{
    char buf[64];

    if (a3 == 0) {
        F10bf3d29_FUN_00283698(buf, a0 + 192);
        F10bf3d29_FUN_00283098(a1, a2, buf);
    } else {
        F10bf3d29_FUN_00283098(a1, a2, a3);
    }
}

INCLUDE_ASM("asm/game", func_002AFE40);

extern void func_00282F70(char *, char *, float);
extern void func_00283098(char *, char *, char *);

void func_002AFE58(char *object, char *out, char *in, char *transform, float scale)
{
    func_002AFDD8(object, out, in, transform);
    func_00282F70(out, out, scale);
    func_00283098(out, out, object + 0xC0);
}

void func_002AFEB8(char *object, char *out, char *in, char *transform, float z)
{
    func_002AFDD8(object, out, in, transform);
    *(float *)(out + 8) = z;
    func_00283098(out, out, object + 0xC0);
}
