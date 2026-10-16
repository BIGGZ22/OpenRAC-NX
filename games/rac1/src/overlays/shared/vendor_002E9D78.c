/* Shared level code: func_L00_002E9D78 in a file of its own, built with -fno-schedule-insns
 * (config/file_cflags.txt). GCC 2.95 takes options per translation unit,
 * and the functions of the file it came from do not build with this one
 * (docs/BUILD_FIDELITY.md, "Flags"). */
#include "common.h"
#include "include_asm.h"

extern void func_001F9BF0(float *, float *, float *);
extern float func_L00_002E9B60(char *, float *, float);

void func_L00_002E9D78(char *moby, int arg, float f)
{
    float scratch[4];
    char *data = *(char **)(moby + 0x70);
    func_L00_002E9B60(moby, (FastVecSub(scratch, (float *)arg, (float *)(data + 0x40)), scratch), f);
}
