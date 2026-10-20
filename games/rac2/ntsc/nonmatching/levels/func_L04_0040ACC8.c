/* 0 of 2 places match - last line: 04_barlow: 104 bytes, expected 92
   The body is right (walks the 0x1924 record, bumps the 0x1948 counter to 3 and latches the
   0x1944 flag under 5, clears the counter under 4, then zeroes the record's 0x744/0x724
   words) but every global access comes out ABSOLUTE here - `lui $v1,%hi / lw $v1,%lo($v1)` -
   where retail has the bare `$gp` form (`lw $v1,0x1924($gp)`), which is three instructions'
   worth of difference, so the file is 12 bytes over.
   Measured with the W4 recipe, with and without the -G8 unit:
     extern s32 D_L04_001B0914_gp __asm__("D_L04_001B0914") __attribute__((section(".sdata")));
   still emits `lui $v1,%hi(D_L04_001B0914) / lw $v1,%lo(...)($v1)` under -O2 -G8.  The
   existing matched bodies in L04_00402D90-g8.c only reach $gp for STORES that land in a
   delay slot (.set nomacro), so this function needs whatever makes cc1 choose GPREL16 for a
   plain load/store outside a slot - the recipe as measured here does not.  The four
   displacements name the symbols by address: 0x1924 -> D_L04_001B0914,
   0x193C -> D_L04_001B092C, 0x1944 -> D_L04_001B0934, 0x1948 -> D_L04_001B0938. */

typedef int s32;

extern s32 D_L04_001B0914_gp __asm__("D_L04_001B0914") __attribute__((section(".sdata")));
extern s32 D_L04_001B092C_gp __asm__("D_L04_001B092C") __attribute__((section(".sdata")));
extern s32 D_L04_001B0934_gp __asm__("D_L04_001B0934") __attribute__((section(".sdata")));
extern s32 D_L04_001B0938_gp __asm__("D_L04_001B0938") __attribute__((section(".sdata")));

void func_L04_0040ACC8(void) {
    char *v1 = (char *)D_L04_001B0914_gp;
    char *o = *(char **)(v1 + 0x68);

    if (*(s32 *)(o + 0x744) < 5) {
        s32 v = *(s32 *)(o + 0x724);

        D_L04_001B092C_gp = 1;
        if (v != 0) {
            D_L04_001B0938_gp = D_L04_001B0938_gp + 1;
            if (D_L04_001B0938_gp == 3)
                D_L04_001B0934_gp = 1;
        }
    } else if (D_L04_001B0938_gp < 4) {
        D_L04_001B0938_gp = 0;
    }
    *(s32 *)(o + 0x744) = 0;
    *(s32 *)(o + 0x724) = 0;
}
