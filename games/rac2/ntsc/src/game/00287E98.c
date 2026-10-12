#include "common.h"
#include "structs.h"

   /* current buffer */
extern short r1_D_0015EDC4;
extern void func_00287E98(void);
extern int D_001A8A64 __attribute__((sda));

void func_00287E98(void) {
    D_001A8A64 = 1;
}


void func_00287EA8(void) {
    D_001A8A64 = 0;
}

INCLUDE_ASM("asm/game", func_00287EB0);

INCLUDE_ASM("asm/game", func_00287F60);

INCLUDE_ASM("asm/game", func_00287FF8);
