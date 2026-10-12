#include "include_asm.h"

typedef int s32;

extern void F2d5993bb_FUN_00282A60(char *p, int a1, int a2);

extern void F2d5993bb_FUN_00282AB0(char *p, int a1, int a2);

extern int F2d5993bb_FUN_0029AEB0(char *p, int a1);

typedef int BatchA_A08_s32;

extern BatchA_A08_s32 D_L00_001A904C[] __attribute__((sda));

extern int D_L00_001A904C_6 __attribute__((sda));

extern void func_003380B0(int *p, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9, int a10);

s32 func_0029AE78(s32 *a0) {
 s32 n=8;
 while(a0[0]!=0) {
  n+=8; n+=a0[1]; n=(n+3)&-4;
  a0+=4;
 }
 return n+8;
}

INCLUDE_ASM("asm/game", func_0029AEB0);

INCLUDE_ASM("asm/game", func_0029AF58);

int func_0029AF98(char *out, int mult, int *recs)
{
    int off = 0;
    char *p = out + 8;
    char *r;
    int v;
    int w;

    if (*(int *)recs != 0) {
        r = (char *)recs;
        do {
            off += 8;
            v = *(int *)(r + 0) + mult * *(int *)(r + 4);
            *(int *)(p + 0) = *(int *)(r + 8);
            *(int *)(p + 4) = *(int *)(r + 4);
            p += 8;
            if (*(int *)(r + 8) == 6000)
                F2d5993bb_FUN_00282A60(p, 0, *(int *)(r + 4));
            else
                F2d5993bb_FUN_00282AB0(p, v, *(int *)(r + 4));
            w = *(int *)(r + 4);
            r += 16;
            p += w;
            off += w;
            p = (char *)(((int)p + 3) & ~3);
            off = (off + 3) & ~3;
        } while (*(int *)r != 0);
    }
    off += 8;
    *(int *)(p + 4) = 0;
    *(int *)(p + 0) = -1;
    v = F2d5993bb_FUN_0029AEB0(out + 8, off);
    *(int *)(out + 4) = v;
    *(int *)(out + 0) = off;
    return off + 8;
}

INCLUDE_ASM("asm/game", func_0029B0B0);

INCLUDE_ASM("asm/game", func_0029B168);

INCLUDE_ASM("asm/game", func_0029B3E0);

INCLUDE_ASM("asm/game", func_0029B628);

INCLUDE_ASM("asm/game", func_0029B658);

INCLUDE_ASM("asm/game", func_0029B698);

INCLUDE_ASM("asm/game", func_0029B6D0);

INCLUDE_ASM("asm/game", func_0029B710);

INCLUDE_ASM("asm/game", func_0029B720);

/* A08-sda-offset-40 freshly exact-verified under family reservation */
void func_0029B758(BatchA_A08_s32 a0, BatchA_A08_s32 a1) {
    BatchA_A08_s32 base = D_L00_001A904C[0];
    BatchA_A08_s32 *p;
    if (base == 0) return;
    p = (BatchA_A08_s32 *)(base + 0x38000);
    p[0x7368/4] = a0;
    p[0x736C/4] = a1;
}

INCLUDE_ASM("asm/game", func_0029B780);

INCLUDE_ASM("asm/game", func_0029B7C0);

INCLUDE_ASM("asm/game", func_0029B810);

INCLUDE_ASM("asm/game", func_0029B848);

INCLUDE_ASM("asm/game", func_0029B858);

/* Loads a 32-bit global pointer from a fixed data address; if non-null, tail-forwards it (offset by 0x36F28) plus ten incoming arguments to an overlay-local callee. */
void func_0029B888(int a0, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9)
{
    int value = D_L00_001A904C_6;

    if (value != 0)
        func_003380B0((int *)(value + 0x36F28), a0, a1, a2, a3, a4, a5, a6, a7, a8, a9);
}

INCLUDE_ASM("asm/game", func_0029B910);

INCLUDE_ASM("asm/game", func_0029BA58);

INCLUDE_ASM("asm/game", func_0029BB30);
