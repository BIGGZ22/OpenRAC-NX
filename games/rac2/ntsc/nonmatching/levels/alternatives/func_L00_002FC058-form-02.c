/* Alternative source form for func_L00_002FC058.
   The case-1 body hangs off the `else` of a `st != 1` test, so the dispatch is emitted
   first: this is the one spelling that reaches retail's forward `beq $v1,$v0` for case 1
   instead of the inverted `bne`. It lands on 180 bytes - four short - because the case-2
   test still comes out inverted (`bne $v1,$v0`, case 2 inline) where retail branches
   forward to it.
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

    if (st != 1) {
        if (st < 2)
            return;
        if (st == 2) {
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
    } else {
        v = ref_L00_002FC058_01;
        ref_L00_002FC058_00 = 2;
        ref_L00_002FC058_02 = v;
    }
}
