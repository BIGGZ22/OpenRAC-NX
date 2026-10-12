#include "common.h"
#include "structs.h"

extern long r1_func_0012AAC8();

extern long r1_func_0012AAC8(void *, int);

extern int r1_func_0012AAA8(void *, int);

/* rac1-decomp src/core/0012A2F0.c: func_0012AB60 */
/* Get n bits: peek them (r1_func_0012AAA8), then consume them
   (r1_func_0012AAC8). */
int func_0012E980(void *arg0, int arg1) {
    int r = r1_func_0012AAA8(arg0, arg1);
    r1_func_0012AAC8(arg0, arg1);
    return r;
}
