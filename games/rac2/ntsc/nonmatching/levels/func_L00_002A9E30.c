/* func_L00_002A9E30, 252 bytes, 27 placements.  ABANDON - FP scheduling and register-order
   wall.
   Steps a fade clock and publishes it: when the resident threshold (+0xC38) is below arg0 the
   step is the plain difference, else a rate (2.0, or 0.5 divided by the resident scale at
   +0xC24 while the mode word +0xC2C is clear) times a frame count read through the resident
   owner (+0x2290 -> byte +0x43 -> word +0x24 -> +0x48 -> byte +0x10) minus the threshold, and
   arg0 is added; then, while 0.0 < arg1, the published value is (arg2*step)/(arg1*scale) with
   the scale the resident one or the owner's field at +0x5C when the mode word is set, and it
   is either stored into the clock at +0xC20 or handed to func_31D8A0 with arg3.
   Candidate below is 240 bytes (three instructions short); the whole arithmetic sequence is
   instruction-exact, including the 4-float argument convention ($f12-$f15) and the
   movn-based signed `% 4` neighbour. Remaining:
   - retail keeps the resident base as a completed address in $v1 (`lui/addiu`) and holds its
     high half in $a1 for the second block; this cc1 keeps the high half + displacements;
   - the two FP branches come out with the likely-ness inverted (retail `bc1f` at +0x50 and
     `bc1fl` at +0xf8, the candidate `bc1fl` and `bc1f`);
   - the two `nop`s retail carries between `c.eq.s $f15,$f0` and the `div.s` are missing.
   Unblocking: a spelling that makes cc1 complete the address into a register at the top. */

#include "include_asm.h"

extern char D_00189E20_2[];
extern void func_31D8A0(float *dst, float value, float scale);

void func_L00_002A9E30(float arg0, float arg1, float arg2, float arg3)
{
    char *base = D_00189E20_2;
    float step;
    float threshold = *(float *)(base + 0xC38);

    if (threshold < arg0) {
        step = arg0 - threshold;
    } else {
        float rate = 2.0f;

        if (*(int *)(base + 0xC2C) == 0)
            rate = 0.5f / *(float *)(base + 0xC24);
        {
            unsigned char *owner = *(unsigned char **)(base + 0x2290);
            int index = owner[0x43];
            char *entry = *(char **)(*(char **)(owner + 0x24) + index * 4 + 0x48);
            unsigned char count = *(unsigned char *)(entry + 0x10);

            step = arg0 + (rate * (float)count - threshold);
        }
    }

    if (0.0f < arg1) {
        float scale = *(float *)(base + 0xC24);

        if (*(int *)(base + 0xC2C) != 0)
            scale = *(float *)*(char **)(*(char **)(base + 0x2290) + 0x5C);
        {
            float value = (arg2 * step) / (arg1 * scale);

            if (arg3 == -1.0f)
                *(float *)(base + 0xC20) = value;
            else
                func_31D8A0((float *)(base + 0xC20), value, arg3);
        }
    }
}
