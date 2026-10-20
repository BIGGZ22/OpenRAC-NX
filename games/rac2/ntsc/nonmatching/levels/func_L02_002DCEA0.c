/* 0 of 3 places match - candidate is 64 bytes against 72; the two missing words are pipeline nops.
   With the record at 0x1BA3C0 declared as a struct, every instruction matches: `lui $v0,0x1C;
   addiu $v0,$v0,-0x5C40`, `mtc1 $zero,$f1; lwc1 $f0,0x26C($v0); c.eq.s $f0,$f1; nop; bc1f` to the
   epilogue, `lui $at,0x3F80; mtc1 $at,$f0; sw $a0,0x270($v0)`, the reciprocal and the two stores
   `swc1 $f0,0x26C($v0)` / `swc1 $f1,0x268($v0)`. Retail keeps two `nop`s between the mode store and
   the `div.s` (its branch is 0x9 long, this one 0x7); no source shape tried produces them.
   Three separate extern floats instead of one base + offsets give three base materialisations
   (64 bytes, wrong shape); `char base[] + offset` gives a derived `addiu $v1,$v0,0x26C`.
   Unblocking it needs either those two nops or a decision that they are not C-reachable. */

void func_L02_002DCEA0(int mode, float duration)
{
    if (D_L02_001BA3C0.value == 0.0f) {
        D_L02_001BA3C0.mode = mode;
        D_L02_001BA3C0.value = 1.0f;
        D_L02_001BA3C0.step = 1.0f / duration;
    }
}
