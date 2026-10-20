/* 0 of 23 places match - last line: 01_oozla: instruction at +0x2c differs
   Right size (96 bytes) and 21 of the 24 words match.  Two positions resist:
   (a) +0x2C/+0x34 -- retail finishes $a1 (`lui $a1,0x5; ori $a1,$a1,0x360B`) and puts the
   `addiu $a0,$zero,0x47` of the first func_3717C8 call in the jal delay slot; this cc1
   emits the a0 move before the jal and sinks the `ori` into the slot instead.
   (b) +0x50 -- retail `or $a1,$v0,$a1` (constant register first), this cc1
   `or $a1,$a1,$v0`.
   Loads and constants are right: the boot word 0x001A72E0 must be written as the literal
   `*(int *)0x001A72E0` (the symbol form costs the a1 base register and 4 more words), the
   per-level pointer and its $gp slot are ref_ names (the slot needs `sda`: this file is
   -G0), and the value genuinely shifts arithmetic (`sra`) and ORs 0x01000000 in.
   Tried and identical: symbol vs literal load, `|` swapped, a local for the loaded value,
   `v |=`, a local for the first argument, two-step shifts.  A scheduling/operand-order
   tie this cc1 breaks the other way. */

#include "include_asm.h"

extern void func_3533B8(void);
extern void func_353150(void);
extern void func_353428(void);
extern void func_3717C8(int, int);

/* per level: the pointer handed to the first setter and the $gp slot it lands in */
extern int ref_L01_002F1F70_00 __attribute__((sda));
extern char ref_L01_002F1F70_01[];

void func_L01_002F1F70(void)
{
    func_3533B8();
    ref_L01_002F1F70_00 = (int)ref_L01_002F1F70_01;
    func_353150();
    func_353428();
    func_3717C8(0x47, 0x5360B);
    func_3717C8(0x4E, 0x01000000 | (*(int *)0x001A72E0 >> 13));
}
