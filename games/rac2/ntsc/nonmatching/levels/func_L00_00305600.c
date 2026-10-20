/* 0 of 27 places match - last line: 00_aranos_tutorial: 176 bytes, expected 168 (8 words over).
   The function is understood: it scans the 0x30-entry gadget-class table of contents at stride
   0x14 (0x0014B540 + 0x4B40; the struct view keeps the member offset a displacement, which
   reproduces `lw $v0,0x4B40($a0)`), returns 1 when the object is absent, and on a hit returns 0
   unless the small-data slot word at 0x001A9670 holds -1, in which case it loads the class
   (func_002FC3C0(1), then func_002946C0(object, *(short *)0x00152CE4, argument)) and returns 1.
   The `!= -1` test reads the small-data word, not the index: the bne's delay slot loads it,
   which is why the register differs from the loop index at that point.
   The eight words left: this cc1 peels the loop's first iteration (it tests entry[1] through the
   base+0x4B54 displacement and materialises the walk pointer afterwards), where retail emits
   `addiu $a0,$a0,0x4B40` straight after the entry[0] test. A shape whose first iteration is not
   peeled is what is needed. Reads a $gp word, so it belongs to the L00_002FC7A0 -g8 unit. */

typedef struct { char gap_L00_00305600[0x4B40]; int entries[0x30 * 5]; } TocBase_L00_00305600;

extern TocBase_L00_00305600 D_0014B540;

extern int D_L00_001A9670 __attribute__((sda));

extern short D_00152CE4;

extern void func_002FC3C0(int);

extern void func_002946C0(int, int, int);

int func_L00_00305600(int object, int argument)
{
    int i = 0;
    int *p;

    if (D_0014B540.entries[0] != object) {
        p = D_0014B540.entries;
        for (i = 1; i < 0x30; i++) {
            p = (int *)((char *)p + 0x14);
            if (*p == object)
                break;
        }
    }
    if (i == 0x30)
        return 1;
    if (D_L00_001A9670 != -1)
        return 0;
    func_002FC3C0(1);
    func_002946C0(object, D_00152CE4, argument);
    return 1;
}
