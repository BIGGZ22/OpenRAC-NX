/* 0 of 2 places match - last line: 03_endako: 88 bytes, expected 92.
   The body is complete: the 16-byte `lq`/`sq` copy onto the frame, the float
   nudge at frame+8, and the 8-argument call (frame, object, *(s32 *)(state +
   0x10) + 2, two $gp values, 0x20, 2, 0xFF) all reproduce.  Two differences:
   this cc1 folds the `object + 0x10` pointer into the load, where retail
   materialises it first, and it issues the three $gp accesses at the top of the
   body instead of interleaved with the call set-up.
   The three cells are one-instruction `$gp` accesses in ordinary flow, so the
   unit must be -G8: measured in a dedicated file with
   `-O2 -G8 -ffunction-sections` (the canonical L03_003C5B28.c is -G0). */

#include "include_asm.h"

typedef int s32;
typedef int TI128 __attribute__((mode(TI)));
typedef float f32;
typedef unsigned char u8;

extern f32 D_L03_001B1A9C __attribute__((sda));
extern s32 D_L03_001B1A94 __attribute__((sda));
extern f32 D_L03_001B1A98 __attribute__((sda));

extern void func_L03_00339128(u8 *, u8 *, s32, s32, f32, s32, s32, s32);

/* copies sixteen bytes of the object onto the frame, nudges the float at +8 and
   hands the frame to the shared query with the measured constants */
void func_L03_00419850(u8 *object)
{
    TI128 frame;
    u8 *state;
    u8 *source = object + 0x10;

    frame = *(TI128 *)source;
    ((f32 *)&frame)[2] = ((f32 *)&frame)[2] + D_L03_001B1A9C;
    state = *(u8 **)(object + 0x68);
    func_L03_00339128((u8 *)&frame, object, *(s32 *)(state + 0x10) + 2,
                      D_L03_001B1A94, D_L03_001B1A98, 0x20, 2, 0xFF);
}
