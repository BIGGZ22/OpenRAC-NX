/* 0 of 4 places match - candidate is 68 bytes against 64. Twin of func_L05_0037AC00 (see its
   near-miss for the full note); the only difference between the two is the record field read at the
   end, 0x10 here against 0x14 there. Same failure: retail fills the `bltz` delay slot with
   `daddu $v0,$zero,$zero` and branches to the shared epilogue, this cc1 gives the zero arm its own
   block plus a `b` over it. */

int func_L05_0037AC40(void)
{
    int slot = func_L05_0037AA88(6);

    return slot < 0 ? 0 : D_002A1800[slot].limit;
}
