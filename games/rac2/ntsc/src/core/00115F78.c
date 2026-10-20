#include "common.h"

/* The unit saves $s0, $s1 and $ra in TImode slots (`sq`), so it is built with
   the -mastra-quad-saves profile. func_00116300 and func_00116360 are declared
   with the argument retail passes; they ignore it. */
extern void *D_00133E74;

extern void func_00116300(void *arg0) __asm__("func_00116300");

extern void func_00116E10(void *arg0, int arg1);

extern void func_00116360(void *arg0) __asm__("func_00116360");

void func_00115F78(int arg0) {
    func_00116300(D_00133E74);
    func_00116E10(D_00133E74, arg0);
    func_00116360(D_00133E74);
}
