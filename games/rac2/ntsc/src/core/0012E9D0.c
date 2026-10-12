#include "common.h"
#include "structs.h"

extern long r1_func_0012AAC8();

extern long r1_func_0012AAC8(void *, int);

extern int r1_func_0012AAA8(void *, int);

/* rac1-decomp src/core/0012A2F0.c: func_0012ABB0 */
/* Get one bit, as func_0012E980. */
int func_0012E9D0(void *arg0) {
    int r;
    r = r1_func_0012AAA8(arg0, 1);
    r1_func_0012AAC8(arg0, 1);
    return r;
}
