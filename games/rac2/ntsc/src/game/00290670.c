#include "common.h"
#include "structs.h"

   /* current buffer */
extern short r1_D_0015EDC4;
extern void func_00290670(void);
extern int D_001A9018 __attribute__((sda));

void func_00290670(void) {
    D_001A9018 = 1;
}


INCLUDE_ASM("asm/game", func_00290680);

INCLUDE_ASM("asm/game", func_00290718);

extern int D_001A901C __attribute__((sda));

void func_00290720(void) {
    D_001A901C = 0;
}

extern int D_001A9020 __attribute__((sda));

void func_00290728(void) {
    D_001A9020 = 0;
}
