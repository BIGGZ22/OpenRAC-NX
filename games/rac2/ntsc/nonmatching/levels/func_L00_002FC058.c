/* 0 of 27 places match - last line: 00_aranos_tutorial: instruction at +0xc differs
   Candidate is 184 bytes, retail's exact size, and the sequence is retail's: the state
   read, the `$gp` counter decrement/reset, both calls of case 2 and the flag tail all
   sit in retail's order with retail's words. Three findings make it stand this close.
   (a) The three `$gp` globals need the SIZED `sda` declaration below; without it this
   cc1 emits a `lui`/%lo pair for every one of them (measured on the plain `extern int`).
   (b) Both conditions branch on `!= 0` - `func_L00_002FC018() != 0` selects the two-call
   arm and `ref_.._02 != 0` takes the early `A = 0` arm - the `== 0` spellings invert the
   blocks and the whole tail.
   (c) `-O2 -G8 -ffunction-sections` (this unit's config/cflags.txt line) is what gives
   `sw $v0,-0x5C80($gp)`; the declaration also needs a real type, an incomplete array
   never takes the gp form.
   WHAT IS LEFT is the dispatch's block layout, not an instruction: retail branches
   FORWARD to the case-1 block at +0x34 (`beq $v1,$v0,.+4+(9<<2)`) and falls through the
   rest of the tree, so case 1 sits after the two `beq`s; this cc1 keeps case 1 as the
   fall-through and emits `bne` inverted, which shifts every following word by the same
   0x20. Both source orders that reach the same instruction set give the same layout.
   27-place shared units: every one of the 27 programs reports the same +0xc. */
#include "include_asm.h"

extern int ref_L00_002FC058_00 __attribute__((sda));
extern int ref_L00_002FC058_01 __attribute__((sda));
extern int ref_L00_002FC058_02 __attribute__((sda));
extern char D_00138320[];

extern int func_L00_002FC018(void);
extern void func_L00_002FC020(void);
extern void func_L00_002FC028(void);

/* state 0 idle, 1 arms the counter from the saved word, 2 counts down and re-arms or
   clears the screen on the 0x10 flag */
void func_L00_002FC058(void)
{
    int st = ref_L00_002FC058_00;
    int v;

    if (st == 1) {
        v = ref_L00_002FC058_01;
        ref_L00_002FC058_00 = 2;
        ref_L00_002FC058_02 = v;
    } else if (st < 2) {
    } else if (st == 2) {
        ref_L00_002FC058_02 = ref_L00_002FC058_02 - 1;
        if (func_L00_002FC018() != 0) {
            func_L00_002FC020();
            func_L00_002FC028();
        } else if (ref_L00_002FC058_02 != 0) {
            if (!(*(int *)D_00138320 & 0x10))
                ref_L00_002FC058_00 = 0;
        } else if (*(int *)D_00138320 & 0x10) {
            ref_L00_002FC058_00 = ref_L00_002FC058_00 + 1;
        } else {
            ref_L00_002FC058_00 = 0;
        }
    }
}
