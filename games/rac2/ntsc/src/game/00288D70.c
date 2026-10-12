#include "common.h"
#include "structs.h"

   /* current buffer */
extern short r1_D_0015EDC4;
extern void func_00288D70(void);
extern int D_001A7B8C __attribute__((sda));

void func_00288D70(void) {
    D_001A7B8C = 1;
}


int func_00288D80(void) {
    return D_001A7B8C;
}
