#include "include_asm.h"

typedef struct {
    int f0;
    char pad_04[0x34];
    short f38;
    char pad_3A[2];
    int f3C;
} Entry_r002F46C8;

extern Entry_r002F46C8 D_L00_0027D848[];

extern char D_0018C958[];

extern short D_L00_0027D880[];

extern void func_2F4490(int, int);

extern int r1_func_0011CC38();

void func_L00_002F4400(void)
{
}

INCLUDE_ASM("asm/levels", func_L00_002F4510);

void func_L00_002F45B8(void) {
    char *base = D_0018C958;
    int *field = (int *)(base + 8);
    unsigned int mask = 0x7FFFFFFF;
    int i = 0xA0;

    for (; i >= 0; i--) {
        *field &= mask;
        field += 3;
    }
}

INCLUDE_ASM("asm/levels", func_L00_002F46C8);

void func_L00_002F4720(void) {
    D_L00_0027D880[0] = 0;
}

void func_L00_002F51B0(void)
{
}

/* Only the second mode triggers the spawn. */
void func_L00_002F5890(int a0) {
    if (a0 == 2)
        func_2F4490(0xA01, 0x58);
}

INCLUDE_ASM("asm/levels", func_L00_002F59A8);

INCLUDE_ASM("asm/levels", func_L00_002F62D8);

INCLUDE_ASM("asm/levels", func_L00_002F6B90);

INCLUDE_ASM("asm/levels", func_L00_002F6ED0);

INCLUDE_ASM("asm/levels", func_L00_002F7298);

INCLUDE_ASM("asm/levels", func_L00_002F74F0);

/* rac1-decomp src/core/00119D88.c: func_0011CCB0 */
/* Returns its callee's value: retail keeps the frame and calls, and
   2.9-ee would tail-call it (a bare `j`) if it were void. */
int func_L00_002F7568(void) {
    return r1_func_0011CC38();
}

int func_L00_002F7588(void) {
    return 0;
}

int func_L00_002F9858(void) {
    return 0;
}

INCLUDE_ASM("asm/levels", func_L00_002FACF8);

INCLUDE_ASM("asm/levels", func_L00_002FB8F8);

INCLUDE_ASM("asm/levels", func_L00_002FBF58);
