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

unsigned long long func_002CD620(void);

extern void func_0029C1D8(void);

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

unsigned long long func_002CD620(void)
{
  func_0027C540(0);
  func_0029C1D8();
  func_0027C660();
  return 0;
}

INCLUDE_ASM("asm/game", func_002CD650);

INCLUDE_ASM("asm/game", func_002CD658);

INCLUDE_ASM("asm/game", func_002CD660);

INCLUDE_ASM("asm/game", func_002CD6F0);

INCLUDE_ASM("asm/game", func_002CD720);

INCLUDE_ASM("asm/game", func_002CD878);
