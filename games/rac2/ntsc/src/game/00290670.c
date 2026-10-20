#include "common.h"
#include "structs.h"

   /* current buffer */
extern short r1_D_0015EDC4;
extern void func_00290670(void);
extern int D_001A9018 __attribute__((sda));

void func_00290670(void) {
    D_001A9018 = 1;
}


extern int D_001A9014 __attribute__((sda));
extern int D_001A9018 __attribute__((sda));
extern void func_0027DDC0(int, int, int, int);

void func_00290680(void)
{
    int n;
    int r;

    if (D_001A9018 != 0) {
        D_001A9018 = 0;
        n = D_001A9014 + 1;
        D_001A9014 = n;
        if (n >= 11)
            D_001A9014 = 10;
    } else {
        n = D_001A9014 - 1;
        D_001A9014 = n;
        if (n < 0)
            D_001A9014 = 0;
    }
    r = (int)((float)D_001A9014 * 0.1f * 48.0f);
    if (r != 0)
        func_0027DDC0(0, 0, 0, r);
}

INCLUDE_ASM("asm/game", func_00290718);

extern int D_001A901C __attribute__((sda));

void func_00290720(void) {
    D_001A901C = 0;
}

extern int D_001A9020 __attribute__((sda));

void func_00290728(void) {
    D_001A9020 = 0;
}
