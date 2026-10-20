#include "common.h"

/* The unit saves $ra and $fp in TImode slots (`sq $ra,0x20($sp)`,
   `sq $fp,0x10($sp)`) and spills every named argument to the frame, so it is
   built with the -mastra-quad-saves profile at -O0: the three arguments live
   at 0, 4 and 8 of the frame, the global is re-read for the call, and the
   32-bit result of func_00117278 is widened to the long return. */

extern void *D_00133E74;

extern int func_00117278(void *arg0, int arg1, int arg2, int arg3);

long func_001175F0(int arg0, int arg1, int arg2) {
    return func_00117278(D_00133E74, arg0, arg1, arg2);
}
