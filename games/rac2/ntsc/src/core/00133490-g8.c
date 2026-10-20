#include "common.h"

/* One $gp-relative global (gp - 0x7B64): this unit is built with -G8 by its
   file name and the symbol is declared sda. Boot-time switch - with the flag
   clear the handler runs, otherwise the 0x37 command goes to func_00132DF0
   with zero arguments. */
extern int D_001A748C __attribute__((sda));

extern int func_00125620(void);

extern void func_00132DF0(int arg0, int arg1, int arg2, int arg3, int arg4);

int func_00133640(void) {
    if (D_001A748C == 0)
        return func_00125620();
    func_00132DF0(0x37, 0, 0, 0, 0);
    return 1;
}

