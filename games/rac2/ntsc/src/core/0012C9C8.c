#include "common.h"
#include "structs.h"

extern void func_0012C3B0(void *);

extern int func_0012C680(void *, int);

extern void func_0012C788(void *, int);

extern void func_0012C9C8(void *);

extern int func_0012CA48(void *);

extern int func_0012C878(void *, int);

extern void func_0012CFA0(void *);

/* rac1-decomp src/core/00125630.c: func_00128BA8 */
void func_0012C9C8(void *arg0) {
    int pad;

    func_0012C3B0(arg0);
    pad = -(*(volatile int *)0x10002020 & 7) & 7;
    if (pad != 0) {
        func_0012C788(arg0, pad);
    }
    while (func_0012C680(arg0, 0x18) != 1) {
        func_0012C788(arg0, 8);
    }
}

/* rac1-decomp src/core/00125630.c: func_00128C28 */
int func_0012CA48(void *arg0) {
    *(int *)((char *)arg0 + 0x1B4) = func_0012C878(arg0, 5);
    if (func_0012C878(arg0, 1) != 0) {
        func_0012C878(arg0, 1);
        func_0012C788(arg0, 7);
        func_0012CFA0(arg0);
    }
    return 0;
}

INCLUDE_ASM("asm/core", func_0012CAB0);

INCLUDE_ASM("asm/core", func_0012CBC0);

INCLUDE_ASM("asm/core", func_0012CC88);

INCLUDE_ASM("asm/core", func_0012CDB0);
