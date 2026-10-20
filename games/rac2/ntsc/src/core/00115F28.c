#include "common.h"

/* The unit saves $ra, $s0 and $s1 in TImode slots (`sq`), so it is built with
   the -mastra-quad-saves profile. D_00133E74 is re-read after every call
   because a callee may store to it, which is what retail does. */
extern void *D_00133E74;

extern void func_00116300(void *arg0);

extern int func_001166C8(void *arg0, int arg1);

extern void func_00116360(void *arg0);

int func_00115F28(int arg0) {
    int result;

    func_00116300(D_00133E74);
    result = func_001166C8(D_00133E74, arg0);
    func_00116360(D_00133E74);
    return result;
}
