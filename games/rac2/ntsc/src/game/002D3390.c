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

extern void func_0029C960(void);

extern char F7cb419c1_D_001F2840[];

extern int F7cb419c1_FUN_002DE618(int arg);

/* Candidate function: FUN_002d3390 */
unsigned long long func_002D3390(void)
{
    func_0027C540(0);
    func_0029C960();
    func_0027C660();
    return 0;
}

INCLUDE_ASM("asm/game", func_002D33C0);

INCLUDE_ASM("asm/game", func_002D3400);

INCLUDE_ASM("asm/game", func_002D3488);

INCLUDE_ASM("asm/game", func_002D34B8);

INCLUDE_ASM("asm/game", func_002D3538);

INCLUDE_ASM("asm/game", func_002D35B8);

INCLUDE_ASM("asm/game", func_002D36A0);

INCLUDE_ASM("asm/game", func_002D3D88);

INCLUDE_ASM("asm/game", func_002D3FA0);

INCLUDE_ASM("asm/game", func_002D4948);

int func_002D4E10(void)
{
    char *p = F7cb419c1_D_001F2840;

    *(int *)(p + 460) = F7cb419c1_FUN_002DE618(*(int *)(p + 460));
    return 0;
}

INCLUDE_ASM("asm/game", func_002D4E48);

INCLUDE_ASM("asm/game", func_002D4FF0);

INCLUDE_ASM("asm/game", func_002D5038);

INCLUDE_ASM("asm/game", func_002D5048);

INCLUDE_ASM("asm/game", func_002D50F0);

INCLUDE_ASM("asm/game", func_002D5290);

INCLUDE_ASM("asm/game", func_002D52D8);

INCLUDE_ASM("asm/game", func_002D53F0);

INCLUDE_ASM("asm/game", func_002D5480);

INCLUDE_ASM("asm/game", func_002D54F0);

INCLUDE_ASM("asm/game", func_002D5518);
