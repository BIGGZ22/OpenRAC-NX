/* func_L04_00455418, 220 bytes, 8 placements.  Nearest: 208 bytes (-3 instructions), the
   first difference at offset 0x14.
   Wraps its float argument into the [a0+0x10, a0+0x14) band using the a0+0xE4 period
   (x - period * (int)(x / period)), then walks three thresholds: below the first returns 0.0f,
   below the second returns func_L04_004554F8((v - d) / d), below the third returns 1.0f, and
   above it returns 1.0f - func_L04_004554F8((v - e) / *(a0+0x1C)).
   Measured gaps: (1) retail carries two FPR hazard nops between lwc1 $f4,0x10($a0) and the
   div.s that this cc1 does not insert; (2) the annulled bc1fl slot at +0x68 carries
   sub.s $f3,$f3,$f0 in retail where the load of *(a0+0x18) is hoisted here, which also costs
   a word in the second block.  Four spellings (named int round-trip, inline cast, copied
   argument, no named temporaries) all give 208 bytes. */

#include "include_asm.h"

typedef float f32;
typedef int s32;

extern f32 func_L04_004554F8(f32 value);

f32 func_L04_00455418(char *a0, f32 x)
{
    f32 v = x;
    f32 span = *(f32 *)(a0 + 0xE4);
    f32 lo = *(f32 *)(a0 + 0x10);
    s32 n = (s32)(v / span);

    v = v - (f32)n * span;
    if (v < lo)
        return 0.0f;
    v = v - lo;
    {
        f32 d = *(f32 *)(a0 + 0x14);
        if (v < d) {
            v = v - d;
            return func_L04_004554F8(v / d);
        }
    }
    {
        f32 e = *(f32 *)(a0 + 0x18);
        if (v < e)
            return 1.0f;
        v = v - e;
        return 1.0f - func_L04_004554F8(v / *(f32 *)(a0 + 0x1C));
    }
}
