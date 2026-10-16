/* Level 15 (Quartu): func_L15_002F9FF8 in a file of its own, built with -fno-force-mem
 * (config/file_cflags.txt). GCC 2.95 takes options per translation unit,
 * and the functions of the file it came from do not build with this one
 * (docs/BUILD_FIDELITY.md, "Flags"). */
#include "common.h"
#include "include_asm.h"

extern char D_0013E633[];
extern char *D_L15_0015F050 MACRO_ADDR;
extern char *D_L15_00167480;
extern void func_L15_002F9D38(void *);

int func_L15_002F9FF8(char *moby)
{
    char *entry = D_L15_0015F050 + (*(short *)(moby + 0x84) << 5);
    char *other = D_L15_00167480;
    char *sub = *(char **)(entry + 0x1C);
    if (*(short *)(other + 0x86) == 0 && *(short *)(sub + 0x20) >= 0 && (unsigned char)D_0013E633[0x2EC1] == 2) {
        func_L15_002F9D38(moby);
    }
    return -1;
}
