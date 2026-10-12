#include "include_asm.h"

typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;
typedef unsigned short u16;
typedef signed int s32;
typedef unsigned int u32;
typedef float f32;

extern s32 func_L12_0041E8E8(void *);
extern void func_L12_0041E8F8(void *);
extern s32 func_00282948(void *);
extern void func_002A7750(void *, s32, s32);
extern void func_0029F338(void *);
extern void func_0032C700(void *, s32, s32, s32, s32, s32, s32, s32,
                          s32, s32, s32, s32, f32, f32);
extern void func_00282F20(f32 *, void *, f32);
extern void func_002B0A68(void *, s32, f32 *, s32, f32, f32, f32, f32);

/* level 12 data reached through $gp (small-data) */
extern int D_L12_001A79F0 __attribute__((sda));
extern u8 *D_L12_001B2E9C __attribute__((sda));
extern int D_L12_001B18F0 __attribute__((sda));

/* level 12 data reached absolutely: out of the $gp window */
extern u8 D_L12_0021EDF0[0x1000];
extern u8 D_L12_0019B4A8[0x4000];

typedef struct Actor {
    char pad00[0x18];
    f32 f18;              /* 0x18 */
    char pad1c[0x04];
    u8 f20;               /* 0x20 */
    char pad21[0x13];
    u16 f34;              /* 0x34 */
    char pad36[0x32];
    void *f68;            /* 0x68 */
    char pad6c[0x2c];
    u32 f98;              /* 0x98 */
    char pad9c[0x16];
    u16 fB2;              /* 0xB2 */
    char padB4[0x0c];
    char fC0[0x40];       /* 0xC0 */
} Actor;

typedef struct Node {
    s32 f0;               /* 0x00 */
    f32 f4;               /* 0x04 */
    f32 f8;               /* 0x08 */
    s32 fC;               /* 0x0C */
} Node;

void func_L12_004163E8(Actor *self)
{
    Node *s1 = self->f68;
    u16 f;
    u8 *p;
    u8 *q;

    switch (self->f20) {

    case 0:
        self->f20 = 1;
        s1->f8 = self->f18;
        f = self->fB2;
        q = D_L12_0021EDF0 + (s16)f;
        if (q[0x464] != 0)
            goto E;
        p = D_L12_0019B4A8 + ((s16)f >> 1) + (D_L12_001A79F0 << 10);
        if (f & 1) {
            if (((*(s8 *)p >> 4) & 7) != 0)
                goto E;
            return;
        } else {
            if ((*p & 7) == 0)
                return;
            goto E;
        }

    case 1:
        if (s1->f0 != -1) {
            if (func_L12_0041E8E8(D_L12_001B2E9C + (s1->f0 << 8)) == 0)
                return;
        }
        self->f20 = 2;
        s1->fC = 0x50;
        return;

    case 4:
        s1->f4 = s1->f4 + 0.0013888889f;
        self->f18 = self->f18 + s1->f4;
        if (s1->f0 != -1)
            *(f32 *)((u8 *)D_L12_001B2E9C + (s1->f0 << 8) + 0x18) += s1->f4;
        if (s1->f8 + 8.0f < self->f18)
            self->f20 = 5;
        return;

    case 2:
        if (func_00282948(&s1->fC) != 0)
            return;
        func_002A7750(self, 3, -1);
        s1->fC = D_L12_001B18F0;
        return;

    case 3:
        if (func_00282948(&s1->fC) == 0) {
            if (s1->fC == D_L12_001B18F0 / 2) {
                if (s1->f0 != -1)
                    *(u8 *)((u8 *)D_L12_001B2E9C + (s1->f0 << 8) + 0x20) = 4;
            }
            return;
        }
        {
            u8 *t = (u8 *)D_L12_001B2E9C + (s1->f0 << 8);
            f32 local[4];
            f32 a = 3.0f;
            f32 b = 6.0f;

            func_0032C700(t + 0x10, 1, 0xE, 0xA, 0xF, 0xA, 0x14, 1,
                          1, 1, 0, 0, 2.0f, 20.0f);
            self->f98 = 0;
            self->f34 = (self->f34 | 0x41) & 0xEFFF;
            func_L12_0041E8F8(t);
            func_00282F20(local, self->fC0, -0.1f);
            func_002B0A68(self, 0x3F, local, 0xFF, a, b, a, b);
            if (s1->f0 != -1) {
                *(u16 *)((u8 *)D_L12_001B2E9C + (s1->f0 << 8) + 0x34) &= 0xEFFF;
                func_0029F338((u8 *)D_L12_001B2E9C + (s1->f0 << 8));
            }
        }
        f = self->fB2;
        p = D_L12_0019B4A8 + ((s16)f >> 1) + (D_L12_001A79F0 << 10);
        if (f & 1) {
            if (((*p >> 4) & 7) != 7)
                *p += 0x10;
        } else {
            if ((*p & 7) != 7)
                *p += 1;
        }
        goto E;

    case 5:
        f = self->fB2;
        p = D_L12_0019B4A8 + ((s16)f >> 1) + (D_L12_001A79F0 << 10);
        if (f & 1) {
            if (((*p >> 4) & 7) != 7)
                *p += 0x10;
        } else {
            if ((*p & 7) != 7)
                *p += 1;
        }
        goto E;
    }

E:
    func_0029F338(self);
}
