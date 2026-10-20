/* 0 of 4 places match - candidate is 68 bytes against 64.
   Everything matches but the exit: retail branches `bltz $a0,<epilogue>` with `daddu $v0,$zero,$zero`
   in the delay slot, so the "not found" arm needs no block of its own and the load arm falls
   straight through to `ld $ra`. This cc1 materialises the 0 in a block of its own and adds a `b`
   over it (68 bytes), or spills the value to $s0 when the zero is a variable initialiser (76 bytes).
   Tried: `if (slot < 0) return 0;` then `return X;`, the inverted `if (slot >= 0) return X;`,
   `slot < 0 ? 0 : X`, `value = 0; if (slot >= 0) value = X; return value;`, and the struct form of
   the record against its byte-offset form. Its twin func_L05_0037AC40 fails identically (field 0x10
   against 0x14). */

int func_L05_0037AC00(void)
{
    int slot = func_L05_0037AA88(6);

    return slot < 0 ? 0 : D_002A1800[slot].value;
}
