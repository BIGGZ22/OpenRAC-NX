/* 0 of 27 places match - last line: 00_aranos_tutorial: instruction at +0x44 differs
   43 of 45 instructions: retail loads the fade value into $f1 (via $at) and issues
   `addiu $a0,$a0,0x18E0` before the $gp load; here it is $f0 and the addiu follows the load.
   Below: the whole -G8 sibling source (declarations included). */

#include "include_asm.h"

/* Small-data members of the resident state, addressed through $gp. */
extern int D_001A7CD0 __attribute__((sda));
extern int D_001A7CD4 __attribute__((sda));
extern float D_001A7CCC __attribute__((sda));

/* Materialised absolutely in the retail bytes, so it stays out of the small-data area.
   Two names: the retail recomputes the address at each site instead of keeping it live
   across the call. */
extern float D_001B18E0 __attribute__((nosda));
extern float D_001B18E0_1 __attribute__((nosda));

extern void func_0031D8A0(float *dst, float value, float scale);

/* walks a two-step fade of one resident float and drops the enable flag once it is finished */
void func_L00_002A7958(void)
{
    float f;

    if (D_001A7CD0 == 0)
        return;
    if (D_001A7CD4 == 0) {
        f = 1.0f;
        func_0031D8A0(&D_001B18E0, f, D_001A7CCC);
        if (1.0f == D_001B18E0_1)
            D_001A7CD4 = 1;
        return;
    }
    if (D_001A7CD4 == 1) {
        D_001A7CD4 = 2;
        return;
    }
    if (D_001A7CD4 == 2) {
        f = 0.0f;
        func_0031D8A0(&D_001B18E0, f, D_001A7CCC);
        if (0.0f == D_001B18E0_1)
            D_001A7CD0 = 0;
    }
}
