#include "include_asm.h"

/*
 * Disassembled at 0x002cd220
 * Signature: int FUN_002cd220(void)
 * Behavior:
 *   - Saves RA on stack
 *   - Calls func_0027C540(0)
 *   - Calls func_0029C138()
 *   - Calls func_0027C660()
 *   - Restores RA from stack
 *   - Returns 0
 */
extern void func_0027C540(long);

extern void func_0027C660(void);

extern void func_0029C450(void);

void func_0027C540(long);

void func_0027C660(void);

/* 
 * Function: func_002D2EE0
 * 
 * Disassembly summary:
 * - Saves return address on stack
 * - Calls func_0027C540(0) (with zero argument)
 * - Calls func_0029C7A0()
 * - Calls func_0027C660()
 * - Restores return address
 * - Returns 0
 *
 * Notes:
 * - The function follows standard MIPS ABI prologue/epilogue.
 * - Delay slots are filled with NOP or argument setup as seen in disassembly.
 * - No local variables used.
 * - All external function addresses come from the assigned externals list.
 */

extern void func_0027C540(long arg0);

/* External functions declared with exact aliases from task.externals */
extern void func_0027C540(long param_1);

extern int F750245c6_D_001A7340 __attribute__((sda));

extern void F750245c6_FUN_0028FC48(int a, int b, int c, int d, char *e, int f);

long long func_002CD8C0(void)
{
    func_0027C540(0LL);
    func_0029C450();
    func_0027C660();
    return 0LL;
}

INCLUDE_ASM("asm/game", func_002CD8F0);

INCLUDE_ASM("asm/game", func_002CD950);

INCLUDE_ASM("asm/game", func_002CDA20);

INCLUDE_ASM("asm/game", func_002CDAB0);

INCLUDE_ASM("asm/game", func_002CDB30);

INCLUDE_ASM("asm/game", func_002CDD88);

INCLUDE_ASM("asm/game", func_002CE5B8);

INCLUDE_ASM("asm/game", func_002CE5C8);

INCLUDE_ASM("asm/game", func_002CE5D0);

INCLUDE_ASM("asm/game", func_002CEBD0);

void func_002CF0A0(int a0, char *a1)
{
    int h, k, p, m;
    k = a0 / 2 + 5;
    h = F750245c6_D_001A7340 / 2;
    m = h - k;
    p = h + k;
    F750245c6_FUN_0028FC48(m - 2, 312, p + 4, 314, a1, 0);
    F750245c6_FUN_0028FC48(m - 2, 333, p + 4, 335, a1, 0);
    F750245c6_FUN_0028FC48(m - 2, 313, m, 334, a1, 0);
    F750245c6_FUN_0028FC48(p + 2, 313, p + 4, 334, a1, 0);
}

INCLUDE_ASM("asm/game", func_002CF188);

INCLUDE_ASM("asm/game", func_002CF1D0);

INCLUDE_ASM("asm/game", func_002CF2B8);

INCLUDE_ASM("asm/game", func_002CF340);

INCLUDE_ASM("asm/game", func_002CF5B0);

INCLUDE_ASM("asm/game", func_002CF7F0);

INCLUDE_ASM("asm/game", func_002CFBE0);

INCLUDE_ASM("asm/game", func_002CFBE8);

INCLUDE_ASM("asm/game", func_002CFDA8);

INCLUDE_ASM("asm/game", func_002D01C8);

INCLUDE_ASM("asm/game", func_002D01D0);

INCLUDE_ASM("asm/game", func_002D0568);

INCLUDE_ASM("asm/game", func_002D08C8);

INCLUDE_ASM("asm/game", func_002D0930);

INCLUDE_ASM("asm/game", func_002D0B98);

INCLUDE_ASM("asm/game", func_002D0F00);

INCLUDE_ASM("asm/game", func_002D0FA0);

INCLUDE_ASM("asm/game", func_002D1228);

INCLUDE_ASM("asm/game", func_002D1590);

INCLUDE_ASM("asm/game", func_002D16C8);

INCLUDE_ASM("asm/game", func_002D1970);

INCLUDE_ASM("asm/game", func_002D1CB0);

INCLUDE_ASM("asm/game", func_002D1D20);

INCLUDE_ASM("asm/game", func_002D1E70);

INCLUDE_ASM("asm/game", func_002D2010);

INCLUDE_ASM("asm/game", func_002D2018);

INCLUDE_ASM("asm/game", func_002D2188);

INCLUDE_ASM("asm/game", func_002D23D8);

INCLUDE_ASM("asm/game", func_002D23F8);

INCLUDE_ASM("asm/game", func_002D2538);

INCLUDE_ASM("asm/game", func_002D2830);

INCLUDE_ASM("asm/game", func_002D2838);

INCLUDE_ASM("asm/game", func_002D2918);

INCLUDE_ASM("asm/game", func_002D2BD8);

INCLUDE_ASM("asm/game", func_002D2C00);

INCLUDE_ASM("asm/game", func_002D2C30);
