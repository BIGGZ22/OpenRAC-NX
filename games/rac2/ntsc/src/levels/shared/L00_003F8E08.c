#include "include_asm.h"

/* Measured partial layouts; original type and game roles are unknown. */
typedef struct Rac2Native_9cdc323a4d0c2fbd_AnalysisOwner {
    unsigned char prefix_00[0x68];
    void *context_68;
} Rac2Native_9cdc323a4d0c2fbd_AnalysisOwner;

typedef struct Rac2Native_9cdc323a4d0c2fbd_AnalysisConditionalContext {
    unsigned char prefix_00[0x10];
    float vector_10[4];
    unsigned int field_20;
    unsigned int active_24;
} Rac2Native_9cdc323a4d0c2fbd_AnalysisConditionalContext;

extern void func_002FDBD8_6(float scale, Rac2Native_9cdc323a4d0c2fbd_AnalysisConditionalContext *context,
                                    float *vector, int mode);

extern int func_002A77E0_3(int x);

INCLUDE_ASM("asm/levels", func_L00_003F8E08);

INCLUDE_ASM("asm/levels", func_L00_003F9040);

INCLUDE_ASM("asm/levels", func_L00_003F9368);

extern void func_31D8A0(void *, float, float);
extern void func_2E6CF0(void *, void *);
extern int func_2EDD48(void *);
extern void func_31A658(void *, int, int);
extern void func_L00_003F9570(void);

void func_L00_003F94F8(char *self)
{
    char *s0 = *(char **)(self + 0x68);

    if (*(unsigned char *)(self + 0x20) == 1) {
        func_31D8A0(s0, 1.0f, *(float *)(s0 + 4));
        func_2E6CF0((void *)func_L00_003F9570, self);
        if (func_2EDD48(s0 + 8) != 0) {
            func_31A658(self, 0, -1);
        }
    }
}

INCLUDE_ASM("asm/levels", func_L00_003F9570);

INCLUDE_ASM("asm/levels", func_L00_003F9A30);

INCLUDE_ASM("asm/levels", func_L00_003FA858);

INCLUDE_ASM("asm/levels", func_L00_003FBF78);

void func_L00_003FC100(void)
{
}

INCLUDE_ASM("asm/levels", func_L00_003FC108);

INCLUDE_ASM("asm/levels", func_L00_003FC228);

INCLUDE_ASM("asm/levels", func_L00_003FC468);

extern char D_L00_001B8DC0[];

extern void func_00282CF0(char *, char *, char *);
extern float func_00283248(float, float);
extern float func_002A7878(float, float);
extern float func_002A78D8(float, float);
extern int func_00283CF0(float);
extern float func_00283B98(float, float);
extern void func_002AF000(float *, float, float, float);
extern void func_002C16C0(float *, float *, int, int, unsigned int, int,
                          float, float, float, float, float);

/* Fifteen times over: take the angle of the constant vector at D_L00_001B8DC0 and the point,
   build a direction from a random step and a random magnitude, jitter the point by a random
   small amount on each axis and hand the result to the spawn helper with 210000.0f, 500.0f,
   1.0f, 0.0f and 0.0f; the original game role is unknown. */
void func_L00_003FC820(float *pos)
{
    float v[4];
    float dir[4];
    float out[4];
    float ang;
    float angle;
    float scale;
    float mag;
    float speed;
    int id;
    int i;

    func_00282CF0((char *)v, D_L00_001B8DC0, (char *)pos);
    ang = func_00283248(v[0], v[1]);

    for (i = 14; i >= 0; i--) {
        scale = func_002A7878(0.5f, 1.0f);
        id = func_00283CF0(func_002A78D8(1.0f, 2.0f));
        func_002A7878(0.05f, 0.2f);
        speed = 210000.0f;
        mag = func_002A7878(0.0f, 1.396263599395752f);
        angle = func_00283B98(ang, func_002A78D8(0.0f, 1.396263599395752f));
        func_002AF000(dir, scale * 0.01666666753590107f, angle, mag);
        out[0] = pos[0] + func_002A78D8(0.0f, 0.1f);
        out[1] = pos[1] + func_002A78D8(0.0f, 0.1f);
        out[2] = pos[2] + func_002A78D8(0.0f, 0.1f);
        func_002C16C0(out, dir, 30, 127, 0x007F7F7F, id,
                      func_002A7878(0.05f, 0.2f) * speed, 500.0f, 1.0f, 0.0f, 0.0f);
    }
}

INCLUDE_ASM("asm/levels", func_L00_003FCA40);

INCLUDE_ASM("asm/levels", func_L00_003FCB70);

INCLUDE_ASM("asm/levels", func_L00_003FCDB0);

INCLUDE_ASM("asm/levels", func_L00_003FD178);

extern int func_2EDD48(void *);
extern void func_310440(void *);
extern void func_2E6EF8(void *, void *);
extern void func_L00_003FD700(void);

void func_L00_003FD6B0(char *self)
{
    if (func_2EDD48(*(void **)(self + 0x68)) != 0) {
        func_310440(self);
    } else {
        func_2E6EF8((void *)func_L00_003FD700, self);
    }
}

INCLUDE_ASM("asm/levels", func_L00_003FD700);

INCLUDE_ASM("asm/levels", func_L00_003FDB20);

INCLUDE_ASM("asm/levels", func_L00_003FDE48);

extern int func_L00_003FED60(int, int, int);

int func_L00_003FE2D0(int a0, int a1, int a2)
{
    int r = func_L00_003FED60(a1, a2, *(short *)(a0 + 0xAA) == 0xE9E);

    if (r != 0) {
        *(int *)(r + 0xB8) = a0;
    }
    return r;
}

extern float func_002A7878(float, float);
extern int func_002A7820(int, int);
extern void func_002A7A00(char *, float, float);
extern char *func_002BDCE8(float, float, float, float, char *, int, char *,
                     unsigned int, int);
extern void func_00282F20(void *, void *, float);
extern void func_00312B70(void *, void *, void *, void *);
extern char ref_L00_003FE310_00[];

/* Twenty rounds that spawn a particle at the second argument: each round draws a radius
   (70000..150000) and a byte (10..127), builds a three-float direction on the stack and
   fills the new record's short and byte fields; the level's own data address is then
   handed to the vector routine with the third argument and a scale of 1.5, and to the
   level-object starter with the second argument. */
void func_L00_003FE310(void *a0, void *a1, void *a2)
{
    char local[16];
    float f;
    int i;
    unsigned int v;
    unsigned int w;
    unsigned int x;
    char *r;

    for (i = 19; i >= 0; i--) {
        func_002A77E0_3(2);
        func_002A77E0_3(2);
        f = func_002A7878(70000.0f, 150000.0f);
        v = func_002A7820(10, 127);
        w = (v << 8) | 0x7f000000U;
        x = (v << 16) | w;
        v = x | v;
        func_002A7A00(local, 1.6666668670950457453728e-05f,
                6.6666672937572002410889e-03f);
        r = func_002BDCE8(0.3f, 1.0f, 1.01f, f, a1, 0, local, v, 1);
        if (r != 0) {
            char *q = r + 32;

            *(short *)(r + 10) = 120;
            *(int *)(q + 4) = 2;
            *(char *)(q + 10) = 127;
            *(char *)(q + 11) = *(unsigned char *)(r + 10);
        }
    }
    func_00282F20(ref_L00_003FE310_00, a2, 1.5f);
    func_00312B70(ref_L00_003FE310_00, a1, 0, 0);
}

/* When the context flag at offset 0x24 is nonzero, pass scale 1.0, the context, its vector at offset 0x10 and mode zero to the bound helper; original game role is unknown. */
void func_L00_003FE490(Rac2Native_9cdc323a4d0c2fbd_AnalysisOwner *owner)
{
    Rac2Native_9cdc323a4d0c2fbd_AnalysisConditionalContext *context = owner->context_68;
    if (context->active_24 != 0) {
        func_002FDBD8_6(1.0f, context, context->vector_10, 0);
    }
}

INCLUDE_ASM("asm/levels", func_L00_003FE4C8);

INCLUDE_ASM("asm/levels", func_L00_003FE8D0);

INCLUDE_ASM("asm/levels", func_L00_003FE920);

INCLUDE_ASM("asm/levels", func_L00_003FEAC8);

INCLUDE_ASM("asm/levels", func_L00_003FEBA0);

INCLUDE_ASM("asm/levels", func_L00_003FED60);

INCLUDE_ASM("asm/levels", func_L00_003FEEA0);

INCLUDE_ASM("asm/levels", func_L00_003FF530);

/* takes the float array at +104 and writes four values returned by a call with argument 255, then four float constants (-1.0, -2.25, 1.25, 2.5) at +16..+28, a descending count 255/191/127/63 into the ints at +44..+32 and 2.5/3.0 pairs at +48..+60 */
void func_L00_003FF7B8(char *self)
{
    float *p = *(float **)(self + 104);
    float *q = p;
    int *w;
    int c;
    int i;
    int j;

    for (i = 3; i >= 0; i--)
    {
        *q = (float)func_002A77E0_3(255);
        q++;
    }

    p[4] = -1.0000000000000000000000e+00f;
    p[5] = -2.2500000000000000000000e+00f;
    p[6] = 1.2500000000000000000000e+00f;
    p[7] = 2.5000000000000000000000e+00f;

    j = 3;
    w = (int *)(p + 11);
    c = 255;
    for (; j >= 0; j--)
    {
        *w = c;
        w--;
        c -= 64;
    }

    p[12] = 2.5000000000000000000000e+00f;
    p[13] = 3.0000000000000000000000e+00f;
    p[15] = 2.5000000000000000000000e+00f;
    p[14] = 3.0000000000000000000000e+00f;
}

INCLUDE_ASM("asm/levels", func_L00_003FF890);
