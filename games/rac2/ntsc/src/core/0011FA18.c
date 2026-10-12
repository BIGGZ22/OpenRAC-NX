#include "common.h"
#include "structs.h"

extern void r1_func_0011D4A0(void);

/* rac1-decomp src/core/0011DD68.c: func_0011DD98 */
/* Tail call: retail is `j r1_func_0011D4A0` + nop, with no frame at all.
   2.9-ee emits it natively for a void function that ends in a call. */
void func_0011FA18(void) {
    r1_func_0011D4A0();
}
