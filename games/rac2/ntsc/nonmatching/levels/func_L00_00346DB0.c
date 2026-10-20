/* func_L00_00346DB0 (204 bytes, 27 places) -- NEAR MISS, not integrated.  Needs the -G8 unit
   (the resident word is read as lw -0x5FA4($gp)).
   Object-slot probe: reads the flag word at D_L00_00138180+0x1C4, and for the
   0x10 bit either stores the 0xE0 word of the sub-object, else falls through the
   0x900 bit into func_L00_0030D5C8 plus a $gp-gated callback into func_003415B8.
   The whole 204-byte body is instruction-exact except the three-way select that
   ends the 0x10 arm: retail branches (beqz + addiu $v1,zero,-1 in the delay slot
   + a shared daddu $v1,zero,zero) where this cc1 always emits movn.  Measured on
   nine spellings (if/else-if/else, else with r = -1 then a conditional r = 0,
   defaults on either side, early returns, a goto, a char* result, and both a
   64-bit result and a 64-bit select variable): all 204 bytes except the last,
   all differing at +0x18 only.  Same wall for its two twins, deposited beside it. */
extern char D_L00_00138180[];
extern char D_L00_001FEC40[];
extern int D_L00_001A904C __attribute__((sda));

extern void func_L00_0030D5C8(void);
extern char *func_003415B8(char *);
extern void func_002D5E98(void *);

int func_L00_00346DB0(void)
{
    char *g = D_L00_00138180;
    int ret = 0;
    int v = *(int *)(g + 0x1C4);

    if (v & 0x10) {
        char *p = D_L00_001FEC40;
        int x = *(int *)(*(char **)(p + 0x14) + 0xE0);
        int r = 0;

        if (x != 0) {
            *(int *)(p + 0x18) = x;
        } else if (*(int *)(p + 0x134) == 0) {
            r = -1;
        }
        return r;
    }
    if (v & 0x900) {
        ret = 1;
    } else {
        func_L00_0030D5C8();
        v = *(int *)(g + 0x1C4);
        if ((v & 0x40) && D_L00_001A904C != 0) {
            char *o = func_003415B8((char *)D_L00_001A904C + 0x3BE08);
            char tmp[8];

            *(unsigned short *)(tmp + 2) = *(unsigned short *)(o + 8);
            *(int *)(tmp + 4) = *(int *)(o + 0xC);
            func_002D5E98(tmp);
        }
    }
    return ret;
}
