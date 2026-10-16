/* NON_MATCHING func_L18_002F1444 -- src/overlays/l18_veldin2/vendor_002A8400.c
 * Best so far: SIZE ours 200 / retail 204, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   (not run). Landing needs line 137 changed to a 4-byte MACRO_ADDR declaration (2D8140 reads it through
 *   `*(char **)&D_L18_00160058`, which would still work with a cast).
 *   2. Missing `nop` (4 bytes) before the inner loop: retail's function starts at 0x...444 (4 mod 8; it is the
 *   interior of one function with func_L18_002F1420, which jumps into it with $9 = list, $12 = a0), so the
 *   .p2align 3 of the inner loop pads one nop. Standalone gcc aligns the function to 8, so no pad. The pair
 *   should be written as ONE C function (2F1420 + 2F1444, 228 bytes): table lookup D_L18_001AC540[a1], return 1
 *   if null, then this body ($12 = a0 is `val`, which is the uninitialised value stored at data+0x16C).
 *   Not verified combined (try_func only swaps one function).
 */
#include "common.h"

int func_L18_002F1444(int a0, int a1, void *src, int a3, int a4, short *list) {
    int ret = 1;
    int val = a0; /* FIX: retail's $12 = a0 (set by func_L18_002F1420 at 002F1438) */
again:
    {
        unsigned char *moby = (unsigned char *)D_L18_00160058_m +  /* FIX: moby pool pointer is 0x160058 (retail 002F1450), not 0x15F6A8 */ ((*(unsigned short *)list & 0x7FFF) << 8);
        if (*(short *)(moby + 0xA6) == 0x54B && moby[0x20] == 1) {
            unsigned char *data = *(unsigned char **)(moby + 0x78);
            int n;
            ret = 0;
            qcopy(moby + 0x10, src);
            n = 0;
            do {
                qcopy(data + 0x10 + n * 16, moby + 0x10);
                n++;
            } while ((float)n < 4.0f);
            *(int *)(data + 0x16C) = val;
            moby[0x31] = 1;
            *(unsigned short *)(moby + 0x34) &= 0xFFFE;
                *(int *)(moby + 0x94) = *(int *)(*(unsigned char **)(moby + 0x24) + 0x10);
            moby[0x20] = 2;
        }
    }
    if (*list++ >= 0) goto again;
    return ret;
}
