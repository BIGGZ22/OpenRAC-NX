#include "common.h"

/* The unit saves $ra in a TImode slot (`sq`), so it is built with the
   -mastra-quad-saves profile. */
extern int D_001341E0;

extern int D_001341E4;

extern int D_00134DB8;

extern void func_0011AC40(int arg0);

void func_00116360(void) {
    if (--D_001341E4 == 0) {
        D_001341E0 = -1;
        func_0011AC40(D_00134DB8);
    }
}
