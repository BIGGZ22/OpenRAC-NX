/* Alternative source form for func_L00_002FC058.
   A two-case `switch` instead of the if/else-if chain: it lands on retail's 184 bytes
   and gives the switch dispatch layout (both cases branch forward, case 1 out of line),
   but this cc1 then drops the `slti $v0,$v1,2` range test retail carries between the two
   `beq`s, so the dispatch is eight bytes short and case 1 sits at +0x28, not +0x34.
   Kept beside the primary near-miss (levels/func_L00_002FC058.c), which records the
   `sda` declarations, the `!= 0` polarities and the remaining blocker.
   Naming/extraction only: this form has not been recompiled or validated. */
#include "include_asm.h"

extern int ref_L00_002FC058_00 __attribute__((sda));
extern int ref_L00_002FC058_01 __attribute__((sda));
extern int ref_L00_002FC058_02 __attribute__((sda));
extern char D_00138320[];

extern int func_L00_002FC018(void);
extern void func_L00_002FC020(void);
extern void func_L00_002FC028(void);

void func_L00_002FC058(void)
{
    int st = ref_L00_002FC058_00;
    int v;

    switch (st) {
    case 1:
        v = ref_L00_002FC058_01;
        ref_L00_002FC058_00 = 2;
        ref_L00_002FC058_02 = v;
        break;
    case 2:
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
        break;
    }
}
