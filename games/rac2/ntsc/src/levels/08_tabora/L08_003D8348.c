#include "include_asm.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed short s16;
typedef int s32;
typedef unsigned int u32;
typedef float f32;

/* the level's object-handle table entry: 0x100 bytes, indexed by the handle */
typedef struct {
    u8 pad00[0x20];
    u8 f20;
    u8 f21;
    u8 pad22[0x24 - 0x22];
    void *f24;
    u8 pad28[0x2C - 0x28];
    f32 f2C;
    u8 pad30[0x68 - 0x30];
    void *f68;
    u8 pad6C[0x95 - 0x6C];
    u8 f95;
    u16 f96;
    u8 pad98[0xBC - 0x98];
    u8 fBC;
    u8 padBD[0x100 - 0xBD];
} Ent;

/* what an entry points at; only the queue flag and its sound handle are named here */
typedef struct {
    u8  pad00[0x20];
    f32 f20;
    void *f24;
    u8  pad28[0x2C - 0x28];
    f32 f2C;
    u8  pad30[0x50 - 0x30];
    u8  f50[0x88 - 0x50];
    u16 f88;
    u8  pad8A[0xE4 - 0x8A];
    f32 fE4;
    u8  padE8[0x144 - 0xE8];
    s32 f144;
    u8  pad148[0x150 - 0x148];
    f32 f150[4];
    f32 f160;
    f32 f164;
    f32 f168;
    u8  pad16C[0x189 - 0x16C];
    u8  f189;
    u8  pad18A[0x18C - 0x18A];
    u8  f18C;
    u8  pad18D[0x1B0 - 0x18D];
    f32 f1B0;
    f32 f1B4;
    u8  f1B8[0x1BA - 0x1B8];
    s16 f1BA;
    s16 f1BC;
    u16 f1BE;
    u8  pad1C0[0x1CC - 0x1C0];
    s32 f1CC;
    f32 f1D0;
    f32 f1D4;
    s32 f1D8;
    s32 f1DC;
    u8  f1E0[0x254 - 0x1E0];
    s32 f254;
    u8  pad258[0x40E - 0x258];
    u16 f40E;
    u8  pad410[0x440 - 0x410];
    u8  f440[0x448 - 0x440];
    s32 f448;
    u8  f44C[0x484 - 0x44C];
    s32 f484;
    s32 f488;
    u8  f48C[8];
} Moby;

/* the 16-byte block at Ent+0x10 the driver works on */
typedef struct {
    f32 f0;
    f32 f4;
    f32 f8;
    f32 fC;
} Quad;

typedef int V4 __attribute__((mode(TI)));

/* a sub-object hung off Ent::f24 */
typedef struct {
    u8  pad00[0x24];
    f32 f24;
} Sub24;

extern s32 func_2F9A60();
extern void func_2F9A90();
extern void func_328A38(s32, s32);
extern s32 func_327898(s32, s32);
extern s32 func_2EA300(void *, void *, s32, s32, s32);
extern void func_3347E0(Ent *, void *, s32, s32, f32);
extern s32 func_3357E8(Ent *, void *, s32);
extern void func_329EC0(Ent *, f32);
extern f32 func_3278F0(f32, f32);
extern f32 func_2FA3C8(f32);
extern f32 func_2FA3E0(f32);
extern void func_2F9DF0(void *, void *, void *);
extern void func_2F9E20(void *, void *, void *);
extern void func_2FA0B8(void *, void *, f32);
extern void func_L08_00422B98(Ent *);
extern f32 D_L08_001BFF68;
extern f32 D_00189EA8;
extern char D_00189EA0[];

extern f32 func_L08_00422850(Quad *, f32, f32, f32, f32);
extern void func_L08_00422900(Ent *);
extern void func_L08_00422AD8(Ent *);

INCLUDE_ASM("asm/levels", func_L08_003D8348);

/* the entry's sound handle and both of the moby's own handles are released */
void func_L08_003D8A28(Ent *a0)
{
    Moby *s0 = a0->f68;
    s32 v0, v;

    if (a0->f21 != 0xFF) func_328A38(a0->f21, 1);
    if (s0->f484 != -1) func_328A38(s0->f484, 1);
    v = s0->f488;
    v0 = 1;
    if (v == -1) {
        v0 = 1;
    } else {
        func_328A38(s0->f484, 1);
        v0 = 1;
    }
    a0->fBC = v0;
}

INCLUDE_ASM("asm/levels", func_L08_003D8AB0);

INCLUDE_ASM("asm/levels", func_L08_003D8CA8);

extern void func_002B60B0(u8 *, float, float, float, float);
extern int func_00282948(void *);
extern float func_002A7878(float, float);
extern float func_00282E80(void *, void *);
extern int func_002B4FF0(u8 *, void *, float);
extern float func_00282DF0(void *);
extern float func_00282C48(float);

int func_L08_003D9038(u8 *object, float accel, float unused1, float damping,
                     float speed, float unused4, float turn, float friction) {
    u8 *state = *(u8 **)(object + 0x68);
    int result;
    float amount;

    func_002B60B0(object, speed * 0.00027777778450399637f,
                  turn * 0.01666666753590107f,
                  accel * 4.848137450608192e-06f,
                  damping * 0.0002908882452175021f);
    *(float *)(state + 0x154) = friction;
    if (func_00282948(state + 0x474) != 0) {
        *(float *)(state + 0x46C) = -*(float *)(state + 0x46C);
        *(float *)(state + 0x470) = *(float *)(state + 0x46C) * 0.01745329424738884f;
        *(int *)(state + 0x474) = func_002A7878(30.0f, 90.0f);
    }
    if (*(int *)(state + 0x48C) != 0 ||
        (object[0x20] == 8 && object[0x95] != 0) ||
        func_00282E80(object + 0x10, state + 0x3F0) < 5.0f)
        *(float *)(state + 0x470) = 0;
    result = func_002B4FF0(object, state + 0x3F0, *(float *)(state + 0x470));
    amount = func_00282DF0(state + 0x1C0);
    amount += func_00282C48(*(float *)(state + 0x1D4));
    if (result & 15) {
        func_00282948(state + 0x448);
        amount = func_00282C48(*(float *)(state + 0x1D4)) + 0.010000000707805157f;
    }
    *(float *)(object + 0x48) = amount * 29.999998092651367f;
    if (*(float *)(object + 0x48) > 1.5f)
        *(float *)(object + 0x48) = 1.5f;
    else if (*(float *)(object + 0x48) < 0.0f)
        *(float *)(object + 0x48) = 0;
    return result;
}

/* the countdown runs while the entry is flagged, then the timer field saturates */
void func_L08_003D9208(Ent *a0)
{
    Moby *s0 = a0->f68;
    u16 v1;

    if (a0->fBC != 0) {
        a0->fBC = 0;
        if (s0->f448 < 200) {
            s0->f448 = func_327898(300, 360);
            s0->f40E = 12;
        }
    }
    v1 = a0->f96;
    if (s0->f254 == 2) {
        if (func_2F9A60(&s0->f448)) s0->f40E = 3;
    }
    v1 = a0->f96;
    if (v1 <= 0xFFFDu) a0->f96 = v1 + 1;
    if (a0->f20 == 8) {
        if (a0->f95 == 0) func_2F9A60(&s0->f48C);
    }
    func_2F9A60(&s0->f440);
    func_2F9A60(&s0->f44C);
}

INCLUDE_ASM("asm/levels", func_L08_003D92D8);

INCLUDE_ASM("asm/levels", func_L08_003D9798);

INCLUDE_ASM("asm/levels", func_L08_003D9F68);

INCLUDE_ASM("asm/levels", func_L08_003F6D30);

INCLUDE_ASM("asm/levels", func_L08_003F6E18);

INCLUDE_ASM("asm/levels", func_L08_003F76C0);

INCLUDE_ASM("asm/levels", func_L08_003F77C0);

extern u32 D_L08_001B9E04;

/* 128-bit copies reach lq/sq only through TImode. */
typedef int TQ_003F79E0 __attribute__((mode(TI)));

/* func_L08_003F79E0: 124 bytes against retail's 104; the four bound constants take
   different FPRs and the frame grows. Deposited in nonmatching/levels/. */
INCLUDE_ASM("asm/levels", func_L08_003F79E0);

INCLUDE_ASM("asm/levels", func_L08_003F7A48);

INCLUDE_ASM("asm/levels", func_L08_0040E9B8);

INCLUDE_ASM("asm/levels", func_L08_0040F9D0);

INCLUDE_ASM("asm/levels", func_L08_0040FAE8);

INCLUDE_ASM("asm/levels", func_L08_0041D808);

INCLUDE_ASM("asm/levels", func_L08_00422190);

extern f32 func_002A7940(void);
extern s32 func_002A7820(s32, s32);
extern f32 func_00283180(f32);
extern f32 func_00283198(f32);
extern void func_00282CC0(void *, void *, void *);
extern void func_L01_0032EF38(void *, int, int, float, float, int);

/* five rounds: each picks a direction, two spreads and a life time */
void func_L08_00422428(Ent *a0)
{
    Moby *s3 = a0->f68;
    f32 position[4];
    f32 angle, radius, first, second;
    int life, i;

    for (i = 4; i >= 0; i--) {
        angle = func_002A7940();
        radius = func_002A7878(0.0f, 0.8f);
        first = func_002A7878(0.0f, 1.5f);
        second = func_002A7878(1.5f, 5.7f);
        life = func_002A7820(100, 180);
        position[0] = func_00283180(angle) * radius;
        position[1] = func_00283198(angle) * radius;
        position[2] = 0.0f;
        func_00282CC0(position, position, (char *)a0 + 0x10);
        position[2] = s3->f1B0 + 0.025f;
        func_L01_0032EF38(position, 0x38002028, 0x2020, first, second, life);
    }
}

INCLUDE_ASM("asm/levels", func_L08_00422578);

INCLUDE_ASM("asm/levels", func_L08_00422850);

INCLUDE_ASM("asm/levels", func_L08_00422900);

/* the entry's own two handles and its sound handle are released */
void func_L08_00422AD8(Ent *a0)
{
    Moby *s0 = a0->f68;
    s32 v0, v;

    if (a0->f21 != 0xFF) func_328A38(a0->f21, 1);
    if (s0->f1D8 != -1) func_328A38(s0->f1D8, 1);
    v = s0->f1DC;
    v0 = 1;
    if (v == -1) {
        v0 = 1;
    } else {
        func_328A38(s0->f1D8, 1);
        v0 = 1;
    }
    a0->fBC = v0;
}

/* the queued sound of the entry is released and the flag cleared */
void func_L08_00422B60(Ent *a0)
{
    Moby *s0 = a0->f68;

    if (s0->f18C != 0) {
        func_2F9A60(s0->f1CC);
        s0->f18C = 0;
    }
}

INCLUDE_ASM("asm/levels", func_L08_00422B98);

/* the swing angle is folded into the moby's frame and the subobject is ticked */
void func_L08_00422EE0(Ent *a0)
{
    Moby *s0 = a0->f68;
    Quad local;
    f32 f21 = -3.1415927f;
    f32 f20 = 3.1415927f;
    f32 t;

    t = func_3278F0(f21, f20);
    s0->f160 = func_2FA3C8(t) * (f32)s0->f189;
    t = func_3278F0(f21, f20);
    s0->f164 = func_2FA3E0(t) * (f32)s0->f189;
    s0->f168 = 0.0f;
    func_2F9DF0((char *)s0 + 0x160, (char *)s0 + 0x160, (char *)s0 + 0x150);
    s0->f168 = s0->f168 + 1.7f;
    if (s0->f1BC != 0) {
        func_2F9E20(&local, (char *)s0 + 0x160, (void *)D_00189EA0);
        *(s32 *)((char *)&local + 8) = 0;
        func_2FA0B8(&local, &local, 8.0f);
        func_2F9DF0((char *)s0 + 0x160, (char *)s0 + 0x160, &local);
    }
    func_L08_00422B98(a0);
}

INCLUDE_ASM("asm/levels", func_L08_00423010);

INCLUDE_ASM("asm/levels", func_L08_004233E0);

INCLUDE_ASM("asm/levels", func_L08_004237F0);

/* the driver is primed, the entry follows its mount and then its own update runs */
void func_L08_004238A0(Ent *a0)
{
    Moby *s0 = a0->f68;
    Sub24 *q;
    f32 f0;
    s32 r;

    func_3347E0(a0, s0->f1E0, 2, 1, 2.5f);
    func_L08_00422900(a0);
    q = a0->f24;
    f0 = q->f24 * 0.5f;
    a0->f2C = f0;
    r = func_3357E8(a0, s0->f50, 0);
    if (r != 0) {
        if (s0->f20 <= 0.0f) {
            s0->f88 |= 2;
            s0->fE4 = 0.8377582f;
        } else {
            s0->f1BE = 330;
        }
        func_L08_00422B60(a0);
        func_L08_00422AD8(a0);
    }
}
