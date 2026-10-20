/* 0 of 27 places match - last line: 00_aranos_tutorial: instruction at +0x8 differs
   Candidate is exactly 276 bytes and instruction-for-instruction the same sequence;
   this cc1 swaps the resident-state base and the looked-up record between $s0 and $s1
   (retail: base $s0, record $s1), which also moves the func_2D2168 argument to $v0.
   Eleven shapes tried (record in a variable read before/after the call, the record
   expression duplicated, the +0x28 load in the call's delay slot, a pointer variable for
   each base) always give the same swap: the allocation wall, not a source difference.
   The 0.025 constant is retail's 0x3CCCCCCE, which no plain 0.025f literal gives. */

#include "common.h"

extern char D_00189E20_36[];
extern u8 D_00139568[];
extern char D_00262570[];
extern int func_2ADEA0(int);
extern void func_2AF7D0(void);
extern int func_2C42E0(int, int, void *);
extern void func_2D2168(void *, int, float);
extern void func_320410(void *, float);

/* Starting a screen: the two counters and the timer are tested, the hero's table entry is
   matched, then a record is looked up and its moby is armed. */
int func_L00_002CAF20(void)
{
    char *s0 = D_00189E20_36;
    int idx;
    char *entry;
    void *s1;

    if (*(u8 *)(s0 + 0x22B8) == 0)
        return 0;
    if (*(u8 *)(s0 + 0x22BA) == 0)
        return 0;
    if (*(short *)(s0 + 0x24EC) != 0)
        return 0;
    idx = func_2ADEA0(0);
    if (idx != *(int *)(s0 + 0x24B4)) {
        func_2AF7D0();
        return 0;
    }
    entry = D_00262570 + D_00139568[idx] * 224;
    if (func_2C42E0(0, 0, entry) == 0)
        return 0;
    s1 = *(void **)(entry + 0x28);
    func_320410(s0 + 0x160, 0.0250000015f);
    if (0.0250000015f < *(float *)(s0 + 0x1B4))
        *(float *)(s0 + 0x1B4) = 0.0250000015f;
    func_2D2168(s1, 0, 11.0f);
    if (*(int *)(s0 + 0xE98) != 0)
        *(int *)(s0 + 0xEA4) = 1;
    *(void **)(s0 + 0x24B0) = s1;
    return 1;
}
