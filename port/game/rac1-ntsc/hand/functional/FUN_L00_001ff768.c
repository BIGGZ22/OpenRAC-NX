/* Carried from the PAL decompilation's nonmatching/functional/func_L00_001FF718.c (games/rac1) through the version
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
/* functional: high. Checked against the US assembly (config/us/overlays/asm/FUN_L00_001ff768.s): vmulx.xyzw by s, vftoi0 (truncation), pminw/pmaxw to 0x7FFF / -0x8000, ppach of the low halves. Clamping in float first gives the same integers as the clamp after truncation; not checked against hardware. Scales the four floats at v by s, truncates to int, clamps each to -0x8000..0x7FFF and packs them as four 16-bit values (x lowest) into the 64-bit result. */
long FUN_L00_001ff768(float *v, float s) {
    float f[4];
    unsigned long r = 0;
    int i;
    f[0] = v[0] * s;
    f[1] = v[1] * s;
    f[2] = v[2] * s;
    f[3] = v[3] * s;
    for (i = 0; i < 4; i++) {
        int n;
        if (f[i] >= 32767.0f) n = 32767;
        else if (f[i] <= -32768.0f) n = -32768;
        else n = (int)f[i];
        r |= (unsigned long)(unsigned short)n << (16 * i);
    }
    return (long)r;
}
