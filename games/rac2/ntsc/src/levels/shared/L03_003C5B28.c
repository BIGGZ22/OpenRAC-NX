#include "include_asm.h"

typedef unsigned char u8;

typedef unsigned int u32;

typedef int s32;

/* Three measured byte-state cases and width-specific field views.
   The correspondence name UpdateMoby_2426 does not establish an original
   class, enumeration, allocation or field-name declaration. */
extern void func_002A7750(u8 *, u8, s32);

extern u32 func_002B38B8(u8 *);

typedef float f32;

typedef struct {
    u32 field00;
    u32 field04;
    f32 field08;
    f32 field0C;
} NativeCallbackTimer;

extern NativeCallbackTimer D_L04_001B98E0;

extern f32 D_L04_001B1C88 __attribute__((sda));

/* Four-byte pointer in the resident image (0x001B2A1C).  The retail reads it
   with a per-site `lui $r,%hi` + `lw $r,%lo($r)` pair, which is gas's expansion
   of the bare-symbol macro cc1 emits only for an `sda` symbol. */
extern u8 *D_L03_001B2A1C __attribute__((sda));

INCLUDE_ASM("asm/levels", func_L03_003C5B28);

INCLUDE_ASM("asm/levels", func_L03_003C5CE8);

INCLUDE_ASM("asm/levels", func_L03_003C5DC8);

INCLUDE_ASM("asm/levels", func_L03_003C5EE8);

INCLUDE_ASM("asm/levels", func_L03_003C9FB8);

int func_L03_003CA138(unsigned char *p) { return p[0x20] == 1; }

INCLUDE_ASM("asm/levels", func_L03_003CA148);

void func_L03_003CA220(u8 *a0, f32 a1) {
    *(f32 *)(*(u8 **)(a0 + 0x68) + 0xA0) = a1;
}

/* func_L03_003CA230 lives in the -g8 sibling: its first call reads a float
   constant through $gp at 0x1300. */

int func_L03_003CA280(unsigned char *p) { return p[0x20] == 1; }

INCLUDE_ASM("asm/levels", func_L03_003CA290);

INCLUDE_ASM("asm/levels", func_L03_003CA4B0);

void func_L03_003CA9F8(u8 *entity) {
    switch (entity[0x20]) {
    case 0:
        func_002A7750(entity, 1, -1);
        *(u32 *)(entity + 0x98) = 0;
        *(unsigned short *)(entity + 0x34) =
            (*(unsigned short *)(entity + 0x34) | 0x41) & 0xEFFF;
        break;
    case 1:
        if (func_002B38B8(entity) != 0) {
            func_002A7750(entity, 2, -1);
            *(u32 *)(entity + 0x98) =
                *(u32 *)(*(u8 **)(entity + 0x24) + 0x10);
        }
        break;
    case 2:
        if (func_002B38B8(entity) == 0) {
            func_002A7750(entity, 1, -1);
            *(u32 *)(entity + 0x98) = 0;
        }
        break;
    }
}

extern int func_002ACBC8(u8 *, u32);

u32 func_L03_003CAAC8(u8 *a0, u8 *a1) {
    return func_002ACBC8(a1, *(u32 *)(*(u8 **)(a0 + 0x68) + 0x60)) != 0;
}

/* func_L03_003CAAF0 lives in the -g8 sibling: its first call reads a float
   constant through $gp at 0x1354. */

INCLUDE_ASM("asm/levels", func_L03_003CAB80);

extern f32 func_00283B98(f32, f32);

void func_L03_003CC1D8(u8 *a0) {
    if (*(u8 *)(a0 + 0x31) == 0)
        return;
    *(f32 *)(a0 + 0xF4) = func_00283B98(*(f32 *)(a0 + 0xF4),
                                        0.10471977293491364f);
    *(f32 *)(a0 + 0xF8) = func_00283B98(*(f32 *)(a0 + 0xF8),
                                        0.04654212296009064f);
}

INCLUDE_ASM("asm/levels", func_L03_003CC238);

INCLUDE_ASM("asm/levels", func_L03_003CC478);

INCLUDE_ASM("asm/levels", func_L03_003D04E0);

INCLUDE_ASM("asm/levels", func_L03_003D09D0);

INCLUDE_ASM("asm/levels", func_L03_003D0AE0);

INCLUDE_ASM("asm/levels", func_L03_003D0FC8);

INCLUDE_ASM("asm/levels", func_L03_003D9578);

INCLUDE_ASM("asm/levels", func_L03_003D9820);

INCLUDE_ASM("asm/levels", func_L03_003E21B0);

INCLUDE_ASM("asm/levels", func_L03_003E26C8);

INCLUDE_ASM("asm/levels", func_L03_003E2968);

INCLUDE_ASM("asm/levels", func_L03_003E2C08);

INCLUDE_ASM("asm/levels", func_L03_003E2D98);

INCLUDE_ASM("asm/levels", func_L03_003E2EC8);

INCLUDE_ASM("asm/levels", func_L03_003E3018);

INCLUDE_ASM("asm/levels", func_L03_003E3308);

INCLUDE_ASM("asm/levels", func_L03_003E3430);

INCLUDE_ASM("asm/levels", func_L03_003E3678);

INCLUDE_ASM("asm/levels", func_L03_003E3750);

INCLUDE_ASM("asm/levels", func_L03_003E3800);

INCLUDE_ASM("asm/levels", func_L03_003E3A00);

INCLUDE_ASM("asm/levels", func_L03_003E4148);

INCLUDE_ASM("asm/levels", func_L03_003E4588);

INCLUDE_ASM("asm/levels", func_L03_003E4680);

extern u8 *func_0029EFD0(s32);

extern void func_002B37E8(u8 *, u8 *);

extern void func_002833F0(u8 *, u8 *);

void func_L03_003E5070(u8 *a0) {
    u8 *q = *(u8 **)(a0 + 0x68);
    u8 *p = func_0029EFD0(0xB28);

    if (p == 0)
        return;
    func_002B37E8(p, a0);
    func_002833F0(p + 0xC0, p + 0xF0);
    *(unsigned short *)(p + 0x34) =
        (*(unsigned short *)(p + 0x34) | 0x100);
    *(u32 *)(q + 0x58) = (u32)p;
}

/* func_L03_003E50E0 lives in the -g8 sibling: its first call reads a float
   constant through $gp at 0x1CB0. */

INCLUDE_ASM("asm/levels", func_L03_003E5170);

void func_L03_003E5218(u8 *a0) {
    *(u32 *)(*(u8 **)(a0 + 0x68) + 0x60) = 1;
}

INCLUDE_ASM("asm/levels", func_L03_00408068);

INCLUDE_ASM("asm/levels", func_L03_00409248);

INCLUDE_ASM("asm/levels", func_L03_00409AB0);

INCLUDE_ASM("asm/levels", func_L03_00409C38);

extern f32 func_00282C48(f32);

int func_L03_00409D28(u8 *a0) {
    u8 *q = *(u8 **)(a0 + 0x68);

    return func_00282C48(*(f32 *)(a0 + 0x18) - *(f32 *)(q + 0x8)) < 0.01f;
}

INCLUDE_ASM("asm/levels", func_L03_00419578);

u32 func_L03_004197A8(u8 *a0) {
    u8 *p = *(u8 **)(a0 + 0x68);

    if (*(u8 *)(a0 + 0x20) == 2)
        return *(u32 *)(p + 0x10);
    return 0;
}

/* func_L03_004197C8 lives in the -g8 sibling: its first call reads a float
   constant through $gp at 0x2AA0. */

void func_L03_00419818(u8 *a0, u32 a1) {
    u8 *p = *(u8 **)(a0 + 0x68);
    s32 n = *(s32 *)(p + 0x10);
    *(u32 *)(p + 0x1C) = a1;
    n--;
    *(s32 *)(p + 0x10) = n;
    if (n == 0)
        func_002A7750(a0, 3, -1);
}

INCLUDE_ASM("asm/levels", func_L03_00419850);

extern void func_0029F338(u8 *);

extern u32 func_L03_00329EF0(u8 *, u8 *, u8 *, s32);

void func_L03_004199F0(u8 *a0) {
    u8 *q = *(u8 **)(a0 + 0x68);

    if (*(u8 *)(a0 + 0x20) == 0) {
        if (q == 0) {
            func_0029F338(a0);
        } else {
            *(u32 *)(q + 0x480) =
                func_L03_00329EF0(a0, q, q + 0x80, 0x20);
            func_002A7750(a0, 1, -1);
        }
    }
}

INCLUDE_ASM("asm/levels", func_L03_00419A60);

extern void func_L02_0032E998(u8 *, u8 *, u8 *, u8 *, s32, u32, u32);

void func_L03_00419B10(u32 a0, u8 *a1, u8 *a2, u8 *a3, u32 x4, u32 x5) {
    u8 *q = *(u8 **)((u8 *)D_L03_001B2A1C + (a0 << 8) + 0x68);

    func_L02_0032E998(a1, a2, a3, q + 0x80, *(s32 *)(q + 0x480), x4, x5);
}

s32 func_L03_00419B58(u32 a0, s32 a1) {
    u8 *q = *(u8 **)((u8 *)D_L03_001B2A1C + (a0 << 8) + 0x68);
    u8 *p;

    if (a1 != -1) {
        p = q + (a1 << 5);
        return *(s32 *)(p + 0x80);
    }
    return -1;
}

extern void func_L02_0032EDD8(u8 *, u8 *, s32, s32 *);

void func_L03_00419B90(u32 a0, u8 *a1, u8 *a2) {
    u8 *q = *(u8 **)((u8 *)D_L03_001B2A1C + (a0 << 8) + 0x68);

    func_L02_0032EDD8(a1, q + 0x80, *(s32 *)(q + 0x480), (s32 *)a2);
}

void func_L03_00419C00();

void func_L03_00419BD8(u32 a0) {
    func_L03_00419C00((u8 *)D_L03_001B2A1C + (a0 << 8));
}

INCLUDE_ASM("asm/levels", func_L03_00419C00);

INCLUDE_ASM("asm/levels", func_L03_00419C58);

INCLUDE_ASM("asm/levels", func_L03_0041B848);

INCLUDE_ASM("asm/levels", func_L03_00424C40);

INCLUDE_ASM("asm/levels", func_L03_00425030);

INCLUDE_ASM("asm/levels", func_L03_00425108);

INCLUDE_ASM("asm/levels", func_L03_0042B6F8);

INCLUDE_ASM("asm/levels", func_L03_0042B868);

void func_L03_0042B908(u8 *a0, u32 a1) {
    if (a0 != 0) {
        u8 *p = *(u8 **)(a0 + 0x68);
        *(u32 *)(p + 0x2C) = (s32)a1 >> 24;
        *(u32 *)(p + 0x40) = a1 & 0xFFFFFF;
    }
}

INCLUDE_ASM("asm/levels", func_L03_0042B930);

typedef struct {
    int a;
    int b;
    float c;
    int d;
    void (*e)(void);
} S_2fa6d8_r0043AC40;

extern S_2fa6d8_r0043AC40 r1_D_L02_00167680_FBB10 __asm__("r1_D_L02_00167680");

extern void r1_func_L02_002FBB48(void);

typedef struct {
    int a;
    int b;
    float c;
    float d;
} S_r0043AC40;

extern S_r0043AC40 r1_D_L02_00167680;

extern void r1_func_L02_002FBB48();

/* rac1-decomp src/overlays/shared/vendor_002A5218.c: func_L02_002FBB10 */
/* Adapted from Lombyte (MIT) for PAL by OpenRAC's tools/port.py: src/overlays/shared/gameplay_vendor_002ec308.c, FUN_L02_002fa6d8. */
void func_L03_0043AC40(void) {
    r1_D_L02_00167680_FBB10.a = 0;
    r1_D_L02_00167680_FBB10.b = 0;
    r1_D_L02_00167680_FBB10.c = 10000.0f;
    r1_D_L02_00167680_FBB10.d = 0;
    r1_D_L02_00167680_FBB10.e = r1_func_L02_002FBB48;
}

extern f32 D_L03_001B9BEC;

extern void func_L00_00424A88(s32, f32, f32);

extern void func_L03_002D84F0(void *);

extern void func_L03_0043ACD8(void);

void func_L03_0043AC78(void) {
    if (D_L03_001B9BEC != 0.0f) {
        func_L00_00424A88(1, D_L03_001B9BEC * 0.35f, 0.005f);
        func_L03_002D84F0(func_L03_0043ACD8);
    }
}

extern void func_L03_0043ACD8(void);

INCLUDE_ASM("asm/levels", func_L03_0043AD30);

/* rac1-decomp src/game/stub.c: func_001E97C8 */
/* Takes an argument (callers pass one, e.g. r1_func_001EC210) and ignores
   it -- the unused parameter costs no codegen, still 0/8. */
int func_L03_0043AF30(void *arg0) {
    return 0;
}

INCLUDE_ASM("asm/levels", func_L03_00454888);

void func_L03_00456E58(void)
{
}

void func_L03_004586F0(void)
{
}
