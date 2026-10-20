#include "include_asm.h"

/* This unit is built with -G8: every body here reaches a global through $gp.
   The markers below are the functions of L20_0039C448.c whose retail bodies access
   data through $gp; they are written in this file, which the build compiles with -G8. */

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned int u32;
typedef signed int s32;
typedef float f32;

/* 0x1AEFF0 + 0x39F0 : this symbol is written through $gp (gas emits the
 * gp-relative form because the store sits in the bc1f delay slot), so the
 * file has to be compiled with -G8 and the symbol marked sda. */
extern float D_L20_001B29E0 __attribute__((sda));

/* 0x1AEFF0 + 0x39F0 again, under a second name: gcc must not see the two
 * stores as one dead store, and gas expands this one to lui $at / swc1. */
extern float D_L20_001B29E0_1;

void func_L20_004095D0(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);
    float v = *(float *)(state + 0xC) * 1.2f;

    D_L20_001B29E0 = v;
    if (v > 1.0f) {
        D_L20_001B29E0_1 = 1.0f;
    }
}

INCLUDE_ASM("asm/levels", func_L20_0039D680);

INCLUDE_ASM("asm/levels", func_L20_0039E368);

INCLUDE_ASM("asm/levels", func_L20_0039E5F0);

extern char *D_L20_001FBB20[];

extern int D_L20_001B2FB8;

extern char *D_L20_001B2F9C;

extern char *D_L20_001B308C __attribute__((sda));

extern void func_0032F5B0(void *, int, int);

void func_L20_0039E890(int arg0, float arg1) {
    char *s1 = D_L20_001FBB20[arg0];
    int s4 = arg0 & 0xFF;
    float f20 = arg1;

    if (s4 == 0xFF) {
        goto end;
    }
    if (D_L20_001B2FB8 < arg0) {
        goto end;
    }
    if (s1 == 0) {
        goto end;
    }
    for (;;) {
        char *s0 = (char *)D_L20_001B2F9C + ((*(u16 *)s1 & 0x7FFF) << 8);

        if (s0 == 0) {
            goto end;
        }
        if (*(u8 *)(s0 + 0x20) == 1) {
            u16 v0 = *(u16 *)(s0 + 0x34);
            char *a3 = *(char **)(s0 + 0x24);

            v0 = (u16)((v0 & 0xFFBE) | 0x1000);
            *(u16 *)(s0 + 0x34) = v0;
            *(int *)(s0 + 0x98) = *(int *)(a3 + 0x10);
            func_0032F5B0(s0, 2, -1);
            if (0.0f < f20) {
                char *v1 = *(char **)(s0 + 0x68);

                *(f32 *)((char *)D_L20_001B308C + 0x38 + (*(int *)(v1 + 0x84) << 7)) = f20;
            }
        }
        s1 += 2;
        if ((arg0 & 0xFF) == 0xFF) {
            goto end;
        }
        if (D_L20_001B2FB8 < arg0) {
            goto end;
        }
        if (s1 == 0) {
            return;
        }
        if (*(s16 *)(s1 - 2) >= 0) {
            continue;
        }
        if (s1 == D_L20_001FBB20[arg0]) {
            continue;
        }
        break;
    }
end:
    return;
}

INCLUDE_ASM("asm/levels", func_L20_0039E9F8);

INCLUDE_ASM("asm/levels", func_L20_0039ED78);

INCLUDE_ASM("asm/levels", func_L20_003A0320);

INCLUDE_ASM("asm/levels", func_L20_003BFEB8);

INCLUDE_ASM("asm/levels", func_L20_003C0048);

INCLUDE_ASM("asm/levels", func_L20_00408D98);

INCLUDE_ASM("asm/levels", func_L20_00438F98);

/* func_L20_004395B0 reads three floats through $gp; its near miss is in
   nonmatching/levels/func_L20_004395B0.c and belongs in this -G8 unit. */
INCLUDE_ASM("asm/levels", func_L20_004395B0);
