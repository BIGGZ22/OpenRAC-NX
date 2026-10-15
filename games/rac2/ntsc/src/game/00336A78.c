#include "include_asm.h"

extern int func_00336A40(void *);

extern int func_0027C878_2(int);

extern void func_0027F640(float, float, float, int, int, int, int, int);

typedef int s32;

typedef unsigned char u8;

typedef float f32;

typedef unsigned int u32;

extern void D_00115E38(char *assertion, int line, char *file);

extern char D_L00_001ADDD8[];

extern char D_L00_001ADDF8[];

typedef struct { unsigned int f0, f4, f8, f12, f16, f20; } T;

extern char D_L00_001ADDD8_1[];

extern char D_L00_001ADE20[];

extern void D_00115E38_1(char *a0, int a1, char *a2);

extern int func_L00_002E9100(char *, int);

extern void func_00115AC0_1(void *, int, int);

extern void func_00337E98_1(void *, int, int, float, float);

/* Reads a float through a pointer field; returns when it is zero or when a word field is zero, otherwise calls a helper, adjusts a float loaded from another pointer field by half or all of the helper's result depending on a small mode word, and finally calls a second helper with three pointers and three floats taken from the object and from two further pointer fields. */
void func_00336A78(char *o)
{
    float f20;
    int s4;
    int r;
    int s2v;
    char *p0;
    char *p1;

    if (*(float *)(*(char **)(o + 16)) == 0.0f) {
        return;
    }
    if (*(int *)(o + 64) == 0) {
        return;
    }
    s4 = func_00336A40(o);
    f20 = *(float *)(*(char **)(o));
    switch (*(int *)(o + 68)) {
    case 0:
        break;
    case 1:
        f20 = f20 - (float)(s4 >> 1);
        break;
    case 2:
        f20 = f20 - (float)s4;
        break;
    }
    p0 = *(char **)(o);
    p1 = *(char **)(o + 4);
    s2v = *(int *)(*(char **)(o + 12));
    r = func_0027C878_2(*(int *)(o + 56));
    func_0027F640(f20, *(float *)(p0 + 4), *(float *)(p1), s2v, *(int *)(o + 64),
            s4, r, *(int *)(o + 52));
}

/* Complete boot bodies from liolu PRs #72 and #75; refreshed in one combined batch. */
void func_00336B90(u8 *a0, s32 a1) { *(s32 *)(a0 + 0x40) = a1; }

void func_00336B98(char *object, int value)
{
    unsigned int limit = *(unsigned int *)(object + 0x40);
    unsigned int clamped;
    if (limit < (unsigned int)value) clamped = limit;
    else clamped = value;
    if ((float)limit == 0.0f) {
        **(float **)(object + 4) = 0.0f;
        return;
    }
    **(float **)(object + 4) = ((float)clamped / (float)*(unsigned int *)(object + 0x40)) * (float)*(unsigned int *)(object + 0x3C);
}


/* Measured target: byte-identical across all 27 levels. */
void func_00336CC0(u8 *a0, s32 a1) { *(s32 *)(a0 + 0x38) = a1; }

void func_00336CC8(u8 *a0,s32 a1) {
 (*(f32 **)(a0+4))[1]=(f32)a1;
}

void func_00336CE0(u8 *object, s32 value) {
    *(s32 *)(object + 0x44) = value;
}

extern void func_00300618(void *);

void func_00336CE8(char *object)
{
    int count = *(int *)(object + 0x18);
    int i;
    int *row = (int *)(object + 0x1C);
    for (i = 0; i < count; i++) {
        row[i * 2 + 1] += (int)object;
        func_00300618((void *)row[i * 2 + 1]);
    }
}

s32 func_00336D50(u8 *a0,s32 a1) {
 s32 count=*(s32 *)(a0+0x18); s32 i,out=0;u8 *base=a0+0x1c;
 for(i=0;i<count;i++) {
  s32 *p=(s32 *)((i<<3)+(u32)base);
  if(p[0]==a1) {out=p[1];break;}
 }
 return out;
}

void func_00336DA0(void)
{
}

void *func_00336DA8(void *first, void *second) { (void) first; return second; }

void *func_00336DB0(void *object) {
    return object;
}

/* shared native body of family 5b6bba31 */
void func_00336DB8(int *p, unsigned int a1, int a2, int a3)
{
    if (a1 < 4)
        D_00115E38(D_L00_001ADDD8, 37, D_L00_001ADDF8);
    p[1] = a3;
    p[2] = a1;
    p[4] = 0;
    p[5] = 0;
    p[3] = 0;
    p[0] = a2;
}

INCLUDE_ASM("asm/game", func_00336E38);

/* a bump/free-list allocator. `p` (the parameter, moved to `$a1`) is a six-word */
void *func_00336E40(T *p)
{
    unsigned int off, size;
    void *r;

    if (p->f20 != 0)
    {
        unsigned int *x = (unsigned int *)p->f20;

        p->f20 = x[0];
        p->f16 = p->f16 + 1;
        return x;
    }
    off = p->f12;
    size = p->f8;
    if (p->f4 < off + size)
    {
        D_00115E38_1(D_L00_001ADDD8_1, 83, D_L00_001ADE20);
        return 0;
    }
    r = (void *)(p->f0 + off);
    p->f12 = off + size;
    p->f16 = p->f16 + 1;
    return r;
}

void func_00336ED0(u8 *a0, u8 *a1) {
    *(u8 **)(a1 + 0) = *(u8 **)(a0 + 0x14);
    *(u8 **)(a0 + 0x14) = a1;
    *(s32 *)(a0 + 0x10) = *(s32 *)(a0 + 0x10) - 1;
}

extern int D_001ADB10 __attribute__((sda));

int func_00336EF0(void) {
    return D_001ADB10;
}



INCLUDE_ASM("asm/game", func_00336F20);

INCLUDE_ASM("asm/game", func_00337210);

extern void *func_003361A8(void *);

void *func_00337BD8(char *object) {
    func_003361A8(object + 0x10);
    func_003361A8(object + 0x5C);
    return object;
}

INCLUDE_ASM("asm/game", func_00337C10);

INCLUDE_ASM("asm/game", func_00337E28);

/* Initialises an object: two float stores through the pointer at +0, clears byte +423, stores an int at +440, calls a callee with (this+168, -1) storing its result at +468 and the address at +472, sets +432 to 1 and writes 1.0f through the pointer at +4. */
void func_00337E30(char *p, int a1, int a2, float f12, float f13)
{
    char *q = p + 168;

    *(float *)(*(int *)p) = f12;
    *(float *)(*(int *)p + 4) = f13;
    p[423] = 0;
    *(int *)(p + 440) = a2;
    *(int *)(p + 472) = (int)q;
    *(int *)(p + 468) = func_L00_002E9100(q, -1);
    *(int *)(p + 432) = 1;
    *(float *)(*(int *)(p + 4)) = 1.0f;
}

INCLUDE_ASM("asm/game", func_00337E98);

/* clamps a caller-supplied terminator value (-1 meaning unset) to 360, then runs two callees on two sub-objects of one handle, the second with a float pair */
void func_00338040(int object, int value, int count)
{
    int limit = (count == -1) ? 360 : count;
    int inner = object + 8124;

    func_00115AC0_1((void *)(object + 8292), value, 255);
    func_00337E98_1((void *)inner, 1, limit, 255.0f, 80.0f);
}

INCLUDE_ASM("asm/game", func_003380A8);

INCLUDE_ASM("asm/game", func_003380B0);

INCLUDE_ASM("asm/game", func_003384C0);

INCLUDE_ASM("asm/game", func_00338718);
