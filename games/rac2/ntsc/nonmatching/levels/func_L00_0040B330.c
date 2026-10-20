/* 0 of 27 places match - candidate size is exact (104 bytes), one instruction is out of place at
   +0x3c. Retail orders the third call's setup as `lui/addiu $a1` (the 0x255BA0 table), `mtc1
   $zero,$f12`, `sll $v0,6`, `lw $a2,0x1FF0($gp)`, `lui $at,0x3E80; mtc1 $at,$f13`; this cc1 issues
   the `$gp` load before the first `mtc1` and shifts the rest of the block by one slot.
   Everything else is identical: the frame (0x20), the `sd $ra`/`sd $s0` saves, the `lw $s0,0x68($v0)`
   filling the first `jal`'s delay slot, both calls and every argument.
   Needs the -g8 unit (L00_004096B0-g8.c) and `extern int D_L00_001B0FE0 __attribute__((sda));`
   (declared without `sda` it comes out absolute and the body grows to 120 bytes). */

void func_L00_0040B330(char *object)
{
    char local[16];
    char *sub;

    sub = *(char **)(object + 0x68);
    func_L00_0040B2C0(object, local);
    func_002EC608(local, D_L00_001B0FE0);
    func_002EC970(local, D_L00_00255BA0 + *(int *)(sub + 0x2C) * 64, D_L00_001B0FE0, 4, 0.0f, 0.25f);
}
