#include "include_asm.h"

typedef float f32;
typedef unsigned char u8;
typedef int s32;
typedef unsigned int u32;
typedef unsigned short u16;
typedef short s16;

extern f32 func_2F9748(f32, f32);
extern s32 func_32A4D0(u8 *, s32);
extern s32 func_31A718(u8 *);
extern s32 func_3248E0(u8 *, s32, s32);
extern void func_337400(s32, s32);
extern s32 func_2AF870(u8 *);
extern void func_327D00(f32 *, f32 *, f32, f32, f32, f32);
extern char D_00189EA0[];
extern f32 D_L13_001B2568 __attribute__((sda));
extern u32 D_L13_001B291C[] __attribute__((sda));
extern u8 D_L13_001B1E60[];
extern void func_2F8A20(u8 *, u8 *, f32);
extern u8 D_0019B2E8[];
extern s32 D_001A79F0 __attribute__((sda));
extern u8 *D_L13_001B2A0C __attribute__((sda));
extern u8 *D_L13_001B291C_2;
extern u8 D_00189E20[];

INCLUDE_ASM("asm/levels", func_L13_00306F38);

INCLUDE_ASM("asm/levels", func_L13_003383B8);

INCLUDE_ASM("asm/levels", func_L13_0035E890);

/* func_L13_0038C328 is in the -g8 sibling (L13_00306F38-g8.c): it addresses its
   three globals through $gp. */

void func_L13_0038C470(u8 *a0) {
    s32 *ptr = *(s32 **)(a0 + 0x68);
    switch (a0[0x20]) {
    case 0:
        a0[0x30] = 0xFF;
        func_3248E0(a0, 1, -1);
        break;
    case 1:
        if (func_32A4D0(D_00189EA0, ptr[0]) != 0) {
            func_337400(ptr[4], ptr[6]);
            func_3248E0(a0, 2, -1);
        }
        break;
    case 2:
        if (func_32A4D0(D_00189EA0, ptr[1]) != 0) {
            func_337400(ptr[5], ptr[7]);
            func_3248E0(a0, 1, -1);
        }
        break;
    }
}

INCLUDE_ASM("asm/levels", func_L13_0038C578);

INCLUDE_ASM("asm/levels", func_L13_0038C810);

INCLUDE_ASM("asm/levels", func_L13_00397600);

INCLUDE_ASM("asm/levels", func_L13_00397A18);

INCLUDE_ASM("asm/levels", func_L13_0039CFF8);

INCLUDE_ASM("asm/levels", func_L13_003D1778);

void func_L13_003E1218(u8 *a0) {
    *(f32 *)(a0 + 0xF4) = func_2F9748(*(f32 *)(a0 + 0xF4), 0.017453296f);
}

void func_L13_003E1250(u8 *a0) {
    *(f32 *)(a0 + 0xF4) = func_2F9748(*(f32 *)(a0 + 0xF4), 0.017453296f);
}

void func_L13_003E1288(u8 *a0) {
    *(f32 *)(a0 + 0xF4) = func_2F9748(*(f32 *)(a0 + 0xF4), 0.029088825f);
}

void func_L13_003E12C0(u8 *a0) {
    *(f32 *)(a0 + 0xF4) = func_2F9748(*(f32 *)(a0 + 0xF4), 0.017453296f);
}

void func_L13_003E12F8(u8 *a0) {
    *(f32 *)(a0 + 0xF4) = func_2F9748(*(f32 *)(a0 + 0xF4), 0.017453296f);
}

INCLUDE_ASM("asm/levels", func_L13_003E8850);

INCLUDE_ASM("asm/levels", func_L13_003EFD58);

INCLUDE_ASM("asm/levels", func_L13_003FE420);

/* The level's lift platform: it picks its next state from the neighbour it hangs on,
   then drives the platform through it. func_002AAA00 takes two pointers, four floats
   (radians) and an integer, in that order. */
extern f32 func_00282E80(void *, void *);
extern f32 func_00282E48(void *, void *);
extern void func_002AAA00(void *, void *, f32, f32, f32, f32, s32);
extern void func_00282F20(void *, void *, f32);
extern void func_00282CC0(void *, void *, void *);
extern u8 D_0012EA0[];

void func_L13_003FE708(u8 *a0) {
    u8 *ptr = *(u8 **)(a0 + 0x68);
    f32 vec[4];

    if (*(s32 *)ptr != 0) {
        if (func_00282E80(a0 + 0x10, D_0012EA0) < 8.0f) {
            if (a0[0x20] == 0) {
                a0[0x20] = 1;
            }
        }
    } else if (*(s32 *)(ptr + 8) != 0) {
        if (func_00282E48(a0 + 0x10, D_0012EA0) < 8.0f) {
            if (a0[0x20] == 0) {
                if (*(s16 *)(D_0012EA0 + 0x2C8) == 1) {
                    a0[0x20] = 2;
                }
            }
        }
    } else {
        if (a0[0x20] == 0) {
            *(u32 *)(a0 + 0x98) = 0;
            a0[0x20] = 3;
        }
    }

    if (a0[0x20] == 1) {
        func_002AAA00(a0 + 0xF8, ptr + 4, -0.78539824f, 0.0017453295f, 0.0017453295f, 0.0349065922f, 0);
    }
    if (a0[0x20] == 2) {
        func_002AAA00(a0 + 0xF8, ptr + 4, -1.5707965f, 0.0017453295f, 0.0017453295f, 0.0349065922f, 0);
    }
    if (a0[0x20] == 4) {
        *(u16 *)(a0 + 0x96) = *(u16 *)(a0 + 0x96) + 1;
        func_00282F20(vec, a0 + 0xD0, -0.16666667f);
        func_00282CC0(a0 + 0x10, a0 + 0x10, vec);
        if (*(u16 *)(a0 + 0x96) >= 11) {
            *(u16 *)(a0 + 0x96) = 0;
            a0[0x20] = 5;
        }
    }
    if (a0[0x20] == 5) {
        u32 c = *(u16 *)(a0 + 0x96) + 1;

        *(u16 *)(a0 + 0x96) = c;
        if ((u16)c >= 31) {
            *(u16 *)(a0 + 0x96) = 0;
            a0[0x20] = 6;
        }
    }
    if (a0[0x20] == 6) {
        *(u16 *)(a0 + 0x96) = *(u16 *)(a0 + 0x96) + 1;
        func_00282F20(vec, a0 + 0xD0, 0.16666667f);
        func_00282CC0(a0 + 0x10, a0 + 0x10, vec);
        if (*(u16 *)(a0 + 0x96) >= 11) {
            a0[0x20] = 0;
        }
    }
}

INCLUDE_ASM("asm/levels", func_L13_003FE970);

INCLUDE_ASM("asm/levels", func_L13_004067F8);

INCLUDE_ASM("asm/levels", func_L13_0041A398);

INCLUDE_ASM("asm/levels", func_L13_0041A550);

void func_L13_0041B260(u8 *a0) {
    s32 *ptr = *(s32 **)(a0 + 0x68);
    u8 *base = D_00189EA0;
    if (func_32A4D0(base, ptr[0]) != 0) D_L13_001B2568 = 30.0f;
    if (func_32A4D0(base, ptr[1]) != 0) D_L13_001B2568 = 95.0f;
}

INCLUDE_ASM("asm/levels", func_L13_0041BDA8);

INCLUDE_ASM("asm/levels", func_L13_0041CD18);

INCLUDE_ASM("asm/levels", func_L13_0041E170);

INCLUDE_ASM("asm/levels", func_L13_0041F8B8);

INCLUDE_ASM("asm/levels", func_L13_00422CB8);

INCLUDE_ASM("asm/levels", func_L13_004230B0);

extern s32 func_002ACBC8(u8 *, s32);
extern u8 D_001395B8[];

void func_L13_00424728(u8 *a0) {
    s32 *ptr = *(s32 **)(a0 + 0x68);
    if (func_002ACBC8((u8 *)D_00189EA0, ptr[2]) != 0) {
        if (D_001395B8[0x25] == 0) {
            D_001395B8[0x25] = 1;
        }
    }
    if (func_002ACBC8((u8 *)D_00189EA0, ptr[3]) != 0) {
        if (D_001395B8[0x24] == 0) {
            D_001395B8[0x24] = 1;
        }
    }
    if (func_002ACBC8((u8 *)D_00189EA0, ptr[0]) != 0) {
        if (D_001395B8[0x27] == 0) {
            D_001395B8[0x27] = 1;
        }
    }
    if (func_002ACBC8((u8 *)D_00189EA0, ptr[1]) != 0) {
        if (D_001395B8[0x28] == 0) {
            D_001395B8[0x28] = 1;
        }
    }
    if (*((u8 *)D_L13_001B291C[0] + (ptr[4] << 8) + 0x20) == 2) {
        if (D_001395B8[0x26] == 0) {
            D_001395B8[0x26] = 1;
        }
    }
}

INCLUDE_ASM("asm/levels", func_L13_004267B0);

/* func_L13_0042BE60: the catalogue entry starts 8 bytes before the body (a stray
   "addiu $sp,$sp,0x190 ; nop" tail). Not a function start — left as retail asm. */

INCLUDE_ASM("asm/levels", func_L13_0042BE60);

extern void func_002E5CF8(s32, s32, u8 *);
extern void func_002AA548(f32 *, f32 *, f32, f32, f32, f32);

void func_L13_0042BEE0(u8 *a0) {
    u8 *ptr = *(u8 **)(a0 + 0x68);
    if (a0[0x20] == 0) {
        *(u16 *)(a0 + 0x34) |= 0x800;
        *(f32 *)(ptr + 0x8) = *(f32 *)(a0 + 0x18);
        *(f32 *)(ptr + 0x4) = *(f32 *)(a0 + 0x18) - 6.0f;
        a0[0x20] = 1;
    }
    if (a0[0x20] == 1) {
        u8 v = *((u8 *)D_L13_001B291C[0] + (*(u32 *)ptr << 8) + 0x20);
        if (v == 2) {
            func_002E5CF8(0, 0, a0);
            a0[0x20] = v;
        }
    }
    if (a0[0x20] == 2) {
        func_002AA548((f32 *)(a0 + 0x18), (f32 *)(ptr + 0xC), *(f32 *)(ptr + 0x4),
                      0.0016666667f, 0.0016666667f, 0.200000018f);
    }
}

extern s32 func_2AF870(u8 *);
extern void func_327D00(f32 *, f32 *, f32, f32, f32, f32);
extern u8 D_L13_001B1E60[];

void func_L13_0042BFD8(u8 *a0) {
    u8 *ptr = *(u8 **)(a0 + 0x68);
    if (a0[0x20] == 0) {
        *(u16 *)(a0 + 0x34) |= 0x800;
        *(f32 *)(ptr + 0x8) = *(f32 *)(a0 + 0x18);
        *(f32 *)(ptr + 0x4) = *(f32 *)(a0 + 0x18) - 6.0f;
        a0[0x20] = 1;
    }
    if (a0[0x20] == 1) {
        u8 v = *((u8 *)D_L13_001B291C[0] + (*(u32 *)ptr << 8) + 0x20);
        if (v == 2) {
            a0[0x20] = v;
            func_2AF870(D_L13_001B1E60);
        }
    }
    if (a0[0x20] == 2) {
        func_327D00((f32 *)(a0 + 0x18), (f32 *)(ptr + 0xC), *(f32 *)(ptr + 0x4),
                    0.0016666667f, 0.0016666667f, 0.200000018f);
    }
}

INCLUDE_ASM("asm/levels", func_L13_0043E8C0);
