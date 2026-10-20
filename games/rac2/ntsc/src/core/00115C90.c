#include "common.h"

/* The unit saves $ra, $s0 and $s1 in TImode slots (`sq`), so it is built with
   the -mastra-quad-saves profile. The call's failure value is the unsigned 0xFFFFFFFF, and the
   body below runs when the call returns it */
extern int D_00133E78;

extern u32 func_0011B270(int arg0);

u32 func_00115C90(int *arg0, u32 arg1) {
    u32 result;

    D_00133E78 = 0;
    result = func_0011B270(arg1);
    if (result == 0xFFFFFFFF) {
        if (D_00133E78 != 0)
            *arg0 = D_00133E78;
    }
    return result;
}
