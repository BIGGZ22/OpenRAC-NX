/* func_L10_00425888 (164 bytes, 1 place) - the tail matches exactly; the three delay-slot
 * loads are what is left.  `check.py` has NOT been run on this row (docker-free ruler only):
 * scratch/wslcmp.py says "10_hrugis_cloud: 176 bytes, expected 164" for the form below, and
 * 160 bytes for the 8-bit spelling recorded as an additional form.
 *
 * What is settled, measured: (a) the row belongs in a -G8 unit - the same body in the
 * default -G0 profile gives `lui $at` + absolute for the $gp read (this is the sixth wave's
 * "the unit's -G0 profile" cause), and only there does `__attribute__((sda))` reach
 * GPREL16; (b) a plain `extern u8 X;` in a -G8 unit is still addressed absolutely, so
 * `sda` on exactly one of the four objects and nothing on the other three is the right
 * reading of retail's `lw $v0,0x23A0($gp)` plus three `lui/lw` pairs; (c) the tail is exact
 * with the base held as an object (`extern u8 D_L10_001395B8[];` and `D[i] = ...`): the
 * literal spelling folds 0x66/0x67/0x68/0x69 into four `lui $at` pairs and comes out short.
 * The residue: retail's delay slots for the first three clamps hold
 * `addiu $v0,$zero,0x7F`; this cc1 puts a duplicate load of the NEXT clamp's object there
 * instead (`@0xc reloc 7 D_L10_001B1394`), 3 words of pure delay-slot filling.  The 8-bit
 * `if (D >= 128) D = 127;` spelling avoids the duplicates (160 bytes) but replaces
 * `lw`/`slti`/`sw` with `lb`/`bgez`/`sb`.
 * To land it: a way to make this cc1 fill those three delay slots from the fall-through
 * block instead of from the target block.
 */
typedef unsigned char u8;
typedef signed int s32;

extern u8 D_L10_001B1390 __attribute__((sda));
extern u8 D_L10_001B1394;
extern u8 D_L10_001B1398;
extern u8 D_L10_001B139C;

extern u8 D_L10_001395B8[];

void func_L10_00425888(void) {
    if (*(s32 *)&D_L10_001B1390 > 0x7F) *(s32 *)&D_L10_001B1390 = 0x7F;
    if (*(s32 *)&D_L10_001B1394 > 0x7F) *(s32 *)&D_L10_001B1394 = 0x7F;
    if (*(s32 *)&D_L10_001B1398 > 0x7F) *(s32 *)&D_L10_001B1398 = 0x7F;
    if (*(s32 *)&D_L10_001B139C > 0x7F) *(s32 *)&D_L10_001B139C = 0x7F;

    D_L10_001395B8[0x66] = D_L10_001B1390;
    D_L10_001395B8[0x67] = D_L10_001B1394;
    D_L10_001395B8[0x68] = D_L10_001B1398;
    D_L10_001395B8[0x69] = D_L10_001B139C;
}
