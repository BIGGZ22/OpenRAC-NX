/* Carried from the PAL decompilation's nonmatching/functional/func_L00_0025C710.c (games/rac1) through the version
 * map (../../README.md): functional C, not matched C. Names and addresses are this version's. */
#include "types.h"
#include "qcopy.h"
#include "qzero.h"
#ifndef MACRO_ADDR
#define MACRO_ADDR
#endif
#ifndef NOT_SDA
#define NOT_SDA
#endif
#ifndef SDATA
#define SDATA(symbol) __asm__(#symbol)
#endif
/* functional: high. Copies a and b to the stack first (as retail does), then out[i] = func_L00_0025B6A8(a[i], b[i], t) for the four floats, in order; behaviour is exact, only the register choice differs. */
typedef int V128Interp_c25b6b8 __attribute__((mode(TI)));
extern float func_L00_0025B6A8(float a, float b, float t);

void FUN_L00_0025b6b8(void *out_, void *a_, void *b_, float t) {
    float *out = (float *)out_;
    V128Interp_c25b6b8 va = *(V128Interp_c25b6b8 *)a_;
    V128Interp_c25b6b8 vb = *(V128Interp_c25b6b8 *)b_;
    float *pa = (float *)&va;
    float *pb = (float *)&vb;
    {float value = func_L00_0025B6A8(pa[0], pb[0], t); out[0] = value;}
    {float value = func_L00_0025B6A8(pa[1], pb[1], t); out[1] = value;}
    {float value = func_L00_0025B6A8(pa[2], pb[2], t); out[2] = value;}
    {float value = func_L00_0025B6A8(pa[3], pb[3], t); out[3] = value;}
}
