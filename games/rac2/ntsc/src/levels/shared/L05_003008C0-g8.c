#include "include_asm.h"

extern int D_L05_001A9CD8;
extern void func_0029BCC8(void);

int func_L05_003251A0(void) {
    int value = D_L05_001A9CD8;
    func_0029BCC8();
    return value;
}

extern int D_L05_001A9CD0;

void func_L05_003263F8(int value) {
    D_L05_001A9CD0 = value + 1;
}

extern int D_L05_001AC4D8 __attribute__((sda));
extern int D_L05_00225DD0[];
extern int D_L05_00225E50[];
extern int D_L05_00225ED0[];
extern int D_L05_00225F50[];

void func_L05_00379E60(int a0, int a1, int a2, int a3) {
    int count = D_L05_001AC4D8;
    D_L05_00225DD0[count] = a0;
    D_L05_00225E50[count] = a1;
    D_L05_00225ED0[count] = a2;
    D_L05_00225F50[count] = a3;
    D_L05_001AC4D8 = count + 1;
}

/* Walk the live object list and return the first one whose id matches. */
INCLUDE_ASM("asm/levels", func_L05_00379ED0);
