#include "include_asm.h"

/* Owner-reviewed Qwen backlog: observed operations, not recovered original object boundaries. */

/* func_00285840: owner ABI/declaration review retained. */
/* extern declaration for global variable referenced in func_00285840 */
extern unsigned int D_001BACC0;

/* func_00285850: owner ABI/declaration review retained. */
extern unsigned int D_001BAD08;

/* func_002858B0: owner ABI/declaration review retained. */
extern int D_001BAD14;

unsigned int func_00285840(void)
{
    return D_001BACC0;
}

unsigned int func_00285850(void)
{
    return D_001BAD08;
}

INCLUDE_ASM("asm/game", func_00285860);

void func_002858B0(int param_1)
{
  D_001BAD14 = param_1;
}

INCLUDE_ASM("asm/game", func_002858C0);

INCLUDE_ASM("asm/game", func_002859C8);

INCLUDE_ASM("asm/game", func_00286738);
