/* 0 of 1 places match - last line: 06_notak: instruction at +0x2c differs
   The object's live test: run the shared setup into a local, ask the shared helper
   whether the object is live, and answer with the level's own table.
   Retail (112 bytes): the helper's answer is tested, and when it is non-zero the
   level's word at 0x001BFC98 is compared with the second argument; the function
   answers 1 when the two differ and 0 otherwise.
   Wave 9 (p1) re-measured it with the right semantics (the draft above answers 1
   when the two are EQUAL, so its branch target is the other way round):
     0045A4DC  bne  $v1,$s1,one   |  beq  $v1,$s1,zero
     0045A4E0  addiu $v0,$zero,1   |  addiu $v0,$zero,1
     0045A4E4  daddu $v0,$zero,0   |  daddu $v0,$zero,0
   The block order is the same; only the branch sense differs, and this cc1 always
   inverts the second test so that the "return 1" block is the branch target.
   Tried: `&&` and `||` spellings, two separate ifs (this cc1 if-converts those
   into xor/sltiu, 116 bytes), a goto to a shared zero block, and the data read
   through a pointer (that gives a load through a register instead of the retail's
   absolute form).  The level's word must be read as `D_L06_001BFC98` (the symbol
   itself), not through a cast: a cast makes this cc1 emit `lui; lw $v1,0($reg)`.
   The 0x34e half-word and the 0x98 word also come out in the retail's order only
   with `*(u16 *)(s0 + 0x34) |= 0x41;` before `*(s32 *)(s0 + 0x98) = 0;`.
   The eighth argument's address of the other call is 0x001A8F20 in level 17 and
   the same shape here.  Blocker: the branch sense of the second test. */
int func_L06_0045A490(char *a0, int a1, float f)
{
    char buf[16];
    float x = f;

    func_0029FCC0(a0, 0, buf);
    if (func_00277258(buf, 0x21, a0, x) != 0) {
        if (D_L06_001BFC98[0] == a1) {
            return 1;
        }
    }
    return 0;
}
