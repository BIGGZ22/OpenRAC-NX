/* Abandoned attempt - func_L05_004427A8, 128 bytes, 4 places.
   Best result: 128 bytes = exact size, 32 instructions in the retail order,
   every instruction shape identical. The ONLY difference is which GPR holds
   each value: retail $v0,$v1,$a0 (the three byte fields), $a1 (the word),
   f0..f3 (the four floats, already identical); this attempt gives $a2,$a3,$v0,$v1
   because each byte/word load materialises its address in a separate register
   while the retail reuses the destination register
   (`lui $v0,%hi ; lbu $v0,%lo($v0)`).
   Measured: plain externs materialise the address; `__attribute__((sda))` makes
   cc1 emit a bare symbol, which GNU as expands with $at in ordinary flow and to
   the one-instruction $gp form in a delay slot. Neither gives the retail's
   destination-register scratch. The same symbol is a load here and a store in
   the mirrored func_L05_00442720, so a per-symbol attribute cannot express the
   difference. Layout of the two mirrored records:
     0x1B2000 +0x00..+0x02 bytes, +0x04/+0x08/+0x0C/+0x10 floats, +0x14 int
     0x1B2024 int (($gp) 0x3024, reached in the delay slot)
     0x1B3630.. same layout; +0x44 doubles as the word 0x1B3644 */
#include "include_asm.h"

extern unsigned char D_L05_001B3630;
extern unsigned char D_L05_001B3631;
extern unsigned char D_L05_001B3632;
extern float D_L05_001B3634 __attribute__((sda));
extern float D_L05_001B3638 __attribute__((sda));
extern float D_L05_001B363C __attribute__((sda));
extern float D_L05_001B3640 __attribute__((sda));
extern int D_L05_001B3644;
extern unsigned char D_L05_001B2000;
extern unsigned char D_L05_001B2001;
extern unsigned char D_L05_001B2002;
extern float D_L05_001B2004 __attribute__((sda));
extern float D_L05_001B2008 __attribute__((sda));
extern float D_L05_001B200C __attribute__((sda));
extern float D_L05_001B2010 __attribute__((sda));
extern int D_L05_001B2024 __attribute__((sda));

void func_L05_004427A8(void)
{
    unsigned int b0 = D_L05_001B3630;
    unsigned int b1 = D_L05_001B3631;
    unsigned int b2 = D_L05_001B3632;
    float f0 = D_L05_001B3638;
    float f1 = D_L05_001B3640;
    float f2 = D_L05_001B3634;
    float f3 = D_L05_001B363C;
    unsigned int w = D_L05_001B3644;

    D_L05_001B2000 = b0;
    D_L05_001B2001 = b1;
    D_L05_001B2002 = b2;
    D_L05_001B2008 = f0;
    D_L05_001B2010 = f1;
    D_L05_001B2004 = f2;
    D_L05_001B200C = f3;
    D_L05_001B2024 = w;
}
