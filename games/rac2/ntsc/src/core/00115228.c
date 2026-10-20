#include "common.h"

/* $ra is saved in a TImode slot (`sq`), so this unit is built with the
   -mastra-quad-saves profile. func_00115210 is declared with the argument
   retail passes; its definition in 00115200.c ignores it. */
extern void *D_00133E74;

extern void func_00115210(void *arg0) __asm__("func_00115210");

void func_00115228(void) {
    func_00115210(D_00133E74);
}
