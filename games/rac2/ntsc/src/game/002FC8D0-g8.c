#include "include_asm.h"

extern int ref_L00_00300248_00 __attribute__((sda));
extern unsigned char D_0018C0B0[];
extern void func_002AD250(int, int, int, int);

void func_00300248(int a0, int a1) {
    int state = ref_L00_00300248_00;
    if (state != 0)
        func_002AD250(*(int *)D_0018C0B0, state, a1, a0);
}
