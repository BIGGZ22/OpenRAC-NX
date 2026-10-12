#include "common.h"
#include "structs.h"

extern void r1_func_0012C468(void *, void *);

extern int func_0012C680(void *, int);

extern void func_0012C788(void *, int);

extern void r1_func_0012C430(void *, char *, int);

extern void func_0012C9C8(void *);

extern int func_0012CA48(void *);

extern char r1_D_00153900[];

extern char r1_D_00153928[];

extern int func_0012B780(void *, int, int *, int *, void *);

extern int func_0012B568(void *);

/* rac1-decomp src/core/00125630.c: func_00127960 */
/* sliceA0 (libmpeg.a:mpc.o): ISO/IEC 13818-2 6.2.4 slice_header(), the
 * hardware-VLC variant. nextStartCode, then peek 32 bits: if it isn't a
 * slice_start_code (0x101..0x1AF), report it (r1_func_0012C430) and return 2.
 * Otherwise consume the 32 bits, run sliceB (IPU setup) and
 * mbAddressIncrement, stash the raw increment at *arg3 (in case of a
 * fault), and if the fault flag (+0x11C, cleared at entry) got set, report
 * it (r1_func_0012C468) and return 1. On full success, compute the starting
 * macroblock address from the slice's row (low byte of the start code)
 * and the picture's mb width (+0x12C), overwrite *arg3 with 1, latch
 * +0x1B0, and zero eight fields of the *arg4 output struct.
 */
int func_0012B780(void *arg0, int arg1, int *arg2, int *arg3, void *arg4) {
    char *s = (char *)arg0;
    char *p5 = (char *)arg4;
    unsigned int code;
    int sliceB_result;
    int mbInc;
    int row;

    *(int *)(s + 0x11C) = 0;
    func_0012C9C8(s);
    code = func_0012C680(s, 0x20);

    if ((unsigned int)(code - 0x101) >= 0xAF) {
        r1_func_0012C430(s, r1_D_00153900, code);
        return 2;
    }

    func_0012C788(s, 0x20);
    sliceB_result = func_0012CA48(s);
    mbInc = func_0012B568(s);
    *arg3 = mbInc;

    if (*(int *)(s + 0x11C) != 0) {
        r1_func_0012C468(s, r1_D_00153928);
        return 1;
    }

    row = (sliceB_result << 7) + (int)(code & 0xFF) - 1;
    *arg2 = row * (*(int *)(s + 0x12C)) + mbInc - 1;
    *arg3 = 1;
    *(int *)(s + 0x1B0) = 1;
    *(int *)(p5 + 0x14) = 0;
    *(int *)(p5 + 0x10) = 0;
    *(int *)(p5 + 0x4) = 0;
    *(int *)(p5 + 0x0) = 0;
    *(int *)(p5 + 0x1C) = 0;
    *(int *)(p5 + 0x18) = 0;
    *(int *)(p5 + 0xC) = 0;
    *(int *)(p5 + 0x8) = 0;
    return 0;
}
