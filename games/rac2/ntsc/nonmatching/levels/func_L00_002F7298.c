/* 0 of 27 places match - last line: 00_aranos_tutorial: instruction at +0xc differs
   Table slot picker over the 0x0027DD00 block: scan i = 0..12 for the record whose
   word at 0x64 + i*0x90 equals the first argument; when none matches return 1;
   when the record's word at 0x68 (through the block + the first argument) is set
   return 1; otherwise clamp the record's 0x7C word up to the second argument
   (`slt` + `movz`, i.e. a max) and return 0.
   Retail's own shape is odd and needs care: the first iteration is peeled
   (`lw $v1,0x64($v0)` before the loop with the offset folded), the found path
   tests `i < 13` again, and the block is materialised twice - `$a2` keeps the
   unadjusted 0x280000 and gets `addiu $a2,$a2,-0x2300` after the loop. There is
   also a dead `mult $a1,$v1` (0x00A32018) between the divisor setup and the
   `addu $v0,$a2,$a0`, which no C spelling here reproduces.
   Measured: the do/while-with-break form below stops at +0xc (the peeled first
   comparison). Blocker: the peeled first iteration plus the dead multiply. */
typedef int s32;

extern s32 D_L00_0027DD00[];

s32 func_L00_002F7298(s32 key, s32 value)
{
    s32 i;

    for (i = 0; i < 13; i++) {
        if (*(s32 *)((char *)D_L00_0027DD00 + 0x64 + i * 0x90) == key)
            break;
    }
    if (i >= 13)
        return 1;
    if (*(s32 *)((char *)D_L00_0027DD00 + key + 0x68) != 0)
        return 1;
    {
        s32 *slot = (s32 *)((char *)D_L00_0027DD00 + key + 0x7C);

        *slot = *slot < value ? value : *slot;
    }
    return 0;
}
