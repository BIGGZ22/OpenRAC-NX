#include "include_asm.h"

/* Big level-side array: keep the absolute materialisation (`lui+addiu`), never $gp. */
extern unsigned char D_L03_00189E20[] __attribute__((nosda));

extern float D_L03_001AEF64 __attribute__((sda));

extern int D_L03_001B2548;
extern int D_L03_001A9028;
extern int D_L03_001A9024;
extern int D_L03_001C63A8[] __attribute__((nosda));

extern void func_2EF020(unsigned char *, unsigned char *, float);

extern void func_2EEFB0(unsigned char *, unsigned char *, unsigned char *);

void func_L03_0038A9D0(unsigned char *a0, unsigned char *a1) {
    unsigned char *q = a1;
    unsigned char *p = a0;

    func_2EF020(q, p + 0xC0, D_L03_001AEF64);
    func_2EEFB0(q, q, p + 0x10);
}

extern int D_L03_001AEF60 __attribute__((sda));

extern float D_L03_00237F40[] __attribute__((nosda));

extern float func_31BAE8(float, float);

void func_L03_0038B238(void) {
    float *p;
    int i;

    if (D_L03_001AEF60 >= D_L03_001B2548)
        return;
    p = D_L03_00237F40;
    for (i = 15; i >= 0; i--) {
        p[0] = func_31BAE8(0.0f, 1.0f);
        p[1] = func_31BAE8(0.0f, 1.0f);
        p[2] = func_31BAE8(0.0f, 1.0f);
        p[3] = 1.0f;
        p += 4;
    }
    D_L03_001AEF60 = D_L03_001B2548;
}

void func_L03_0032AD10(int a0) {
    if (D_L03_001B2548 != D_L03_001A9028) {
        D_L03_001A9028 = D_L03_001B2548;
        D_L03_001A9024 = 0;
    }
    if (D_L03_001A9024 < 0x20) {
        D_L03_001C63A8[D_L03_001A9024] = a0;
        D_L03_001A9024 = D_L03_001A9024 + 1;
    }
}

/* A 16-byte vector: the engine's VU0 helpers take and return these. */
typedef struct {
    float x;
    float y;
    float z;
    float w;
} Vec4;

extern void func_00282CF0(void *, void *, void *);
extern void func_00283698(void *, void *);
extern void func_00283098(void *, void *, void *);
extern float func_00283248(float, float);
extern float func_00282E20(void *);

extern float D_L03_001B0040 __attribute__((sda));
extern float D_L03_001B0044 __attribute__((sda));
extern float D_L03_001B0048 __attribute__((sda));
extern float D_L03_001B004C __attribute__((sda));

/* Builds a vector with the engine's VU0 helpers, trims its z by the constant the state at
   0xAA names, and stores the two clamped, halved angles in the moby at 0x68. */
void func_L03_003C4A10(unsigned char *a0, void *a1) {
    Vec4 v;
    Vec4 buf[4];
    float f1;
    float f20;
    void *s2;

    if (*(short *)(a0 + 0xAA) == 0x93C) {
        if (a0[0x43] == 1)
            return;
    }

    s2 = *(void **)(a0 + 0x68);

    func_00282CF0(&v, a1, a0 + 0x10);
    func_00283698(buf, a0 + 0xC0);
    func_00283098(&v, &v, buf);

    switch (*(short *)(a0 + 0xAA)) {
    case 0x964:
        v.z -= D_L03_001B0040;
        break;
    case 0x93C:
        v.z -= D_L03_001B0044;
        break;
    case 0x940:
        v.z -= D_L03_001B0048;
        break;
    case 0xA59:
        v.z -= D_L03_001B004C;
        break;
    }

    f20 = func_00283248(v.x, v.y);
    f1 = -func_00283248(func_00282E20(&v), v.z);

    if (f20 > 1.5707964f)
        f20 = 1.5707964f;
    else if (f20 < -1.5707964f)
        f20 = -1.5707964f;
    if (f1 > 0.7853982f)
        f1 = 0.7853982f;
    else if (f1 < -0.7853982f)
        f1 = -0.7853982f;

    f20 = f20 * 0.5f;
    f1 = f1 * 0.5f;

    *(float *)(s2 + 0x2A8) = f20;
    *(float *)(s2 + 0x2A4) = f1;
    *(float *)(s2 + 0x324) = f1;
    *(float *)(s2 + 0x328) = f20;
}

/* The group count is read through $gp here; the group table is absolute and the entity base is
   the bare `lui`/`lw` pair (`sda` sized over 8 bytes in a -G8 unit). */
extern int D_L03_001B2A38 __attribute__((sda));

extern unsigned char *D_L03_001B2A1C[3] __attribute__((sda));

extern unsigned short *D_L03_001FB5A0[];

extern void func_00282C88(void *);

extern void func_00283410(void *, void *);

/* Walks the entities of the object's group (the others of its team at +0x95, not in state 0xFE or
   0xFD), puts each one's offset from the object into the frame that faces the point a1, and
   counts -1 when its y is then positive, +1 otherwise; returns the sum clamped to -5..5. */
int func_L03_003C58F0(unsigned char *a0, float *a1) {
    float v[4];
    float t[4];
    float m[16];
    int n;
    unsigned short *q;
    unsigned char *e;

    if (a0[0x21] == 0xFF)
        return 0;
    func_00282C88(v);
    n = 0;
    v[2] = func_00283248(a1[0] - *(float *)(a0 + 0x10), a1[1] - *(float *)(a0 + 0x14));
    func_00283410(m, v);
    func_00283698(m, m);
    for (q = D_L03_001FB5A0[a0[0x21]];
         a0[0x21] != 0xFF && a0[0x21] <= D_L03_001B2A38 && q != 0 &&
         ((q != 0 && (short)q[-1] >= 0) || q == D_L03_001FB5A0[a0[0x21]]);
         q++) {
        e = D_L03_001B2A1C[0] + ((*q & 0x7FFF) << 8);
        if (e == 0)
            break;
        if (e[0x20] == 0xFE)
            continue;
        if (e[0x20] == 0xFD)
            continue;
        if (e == a0)
            continue;
        if (e[0x95] != a0[0x95])
            continue;
        func_00282CF0(t, e + 0x10, a0 + 0x10);
        func_00283098(t, t, m);
        if (t[1] > 0.0f)
            n--;
        else
            n++;
    }
    if (n >= 6)
        n = 5;
    else if (n <= -6)
        n = -5;
    return n;
}
