#include "include_asm.h"

/* Frozen Batch A A04-table-select-76; acknowledged claim 8c4d82cfbf43e138ed35e757fdaeee3e7c4651fa684549ba9d5e8ac22c56d111. */
typedef unsigned char BatchA_A04_u8;

typedef unsigned int BatchA_A04_u32;

extern BatchA_A04_u32 D_001A79F0[] __attribute__((sda));

extern BatchA_A04_u8 D_001395B8[];

INCLUDE_ASM("asm/levels", func_L04_00402D90);

INCLUDE_ASM("asm/levels", func_L04_00403530);

INCLUDE_ASM("asm/levels", func_L04_00404068);

INCLUDE_ASM("asm/levels", func_L04_00404100);

INCLUDE_ASM("asm/levels", func_L04_004044B8);

INCLUDE_ASM("asm/levels", func_L04_004045E0);

INCLUDE_ASM("asm/levels", func_L04_00407078);

INCLUDE_ASM("asm/levels", func_L04_004073E8);

INCLUDE_ASM("asm/levels", func_L04_004077A0);

INCLUDE_ASM("asm/levels", func_L04_0040A090);

/* A04-table-select-76 freshly exact-verified under family reservation */
BatchA_A04_u32 func_L04_0040A1D0(BatchA_A04_u8 *a0) {
    BatchA_A04_u32 v = D_001A79F0[0];
    BatchA_A04_u32 r = 0;
    if (v == 4) r = D_001395B8[(BatchA_A04_u32)a0 + 0x4D];
    else if (v == 0xB) r = D_001395B8[(BatchA_A04_u32)a0 + 0x57];
    return r;
}

INCLUDE_ASM("asm/levels", func_L04_0040A220);

/* shared native body of family A04-table-select: reads the state-selected byte */
unsigned int func_L04_0040A268(unsigned char *a0)
{
    unsigned int v = D_001A79F0[0];
    unsigned int r = 0;

    if (v == 4) r = D_001395B8[(unsigned int)a0 + 0x52];
    else if (v == 0xB) r = D_001395B8[(unsigned int)a0 + 0x5C];
    return r;
}

INCLUDE_ASM("asm/levels", func_L04_0040A2B8);

INCLUDE_ASM("asm/levels", func_L04_0040A310);

INCLUDE_ASM("asm/levels", func_L04_0040A3B8);

INCLUDE_ASM("asm/levels", func_L04_0040A5C0);

INCLUDE_ASM("asm/levels", func_L04_0040A628);

INCLUDE_ASM("asm/levels", func_L04_0040A958);

INCLUDE_ASM("asm/levels", func_L04_0040AC30);

INCLUDE_ASM("asm/levels", func_L04_0040AC98);

INCLUDE_ASM("asm/levels", func_L04_0040ACA8);

INCLUDE_ASM("asm/levels", func_L04_0040ACC8);

INCLUDE_ASM("asm/levels", func_L04_0040AD28);

INCLUDE_ASM("asm/levels", func_L04_0040BA68);

INCLUDE_ASM("asm/levels", func_L04_0040BD50);

INCLUDE_ASM("asm/levels", func_L04_0040BED0);

INCLUDE_ASM("asm/levels", func_L04_0040C0C0);

INCLUDE_ASM("asm/levels", func_L04_0040C368);

INCLUDE_ASM("asm/levels", func_L04_0040C880);

extern void func_L04_0040D148();

extern void func_L04_0040CE30();

void func_L04_0040CDD0(unsigned char *a0)
{
    unsigned char *s = *(unsigned char **)(a0 + 0x68);

    if (*(unsigned int *)(s + 0x4D8) != 0) {
        func_L04_0040D148(a0, -1);
    } else if (*(unsigned int *)(s + 0x768) != 0) {
        func_L04_0040D148(a0, *(unsigned int *)(s + 0x764));
    } else if (*(unsigned char *)(a0 + 0xBC) != 6) {
        func_L04_0040CE30(a0);
    }
}

INCLUDE_ASM("asm/levels", func_L04_0040CE30);

INCLUDE_ASM("asm/levels", func_L04_0040D148);

INCLUDE_ASM("asm/levels", func_L04_0040DCA8);

INCLUDE_ASM("asm/levels", func_L04_0040DF90);

INCLUDE_ASM("asm/levels", func_L04_0040E248);

INCLUDE_ASM("asm/levels", func_L04_0040E460);

INCLUDE_ASM("asm/levels", func_L04_0040EC80);

INCLUDE_ASM("asm/levels", func_L04_0040EF50);

INCLUDE_ASM("asm/levels", func_L04_0040F5D0);
