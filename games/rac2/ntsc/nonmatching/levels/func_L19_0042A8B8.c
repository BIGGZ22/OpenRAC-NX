/* func_L19_0042A8B8, 228 bytes, 1 placement.  NOT RETRIED - the func_L04_00423178 family.
   Same skeleton at level 19: D_001A8F00 == 2, D_001A7A40, the block at 0x001BD740
   (`lui $v1,0x1C` / `addiu $v1,$v1,-0x28C0`) with its +0x44 count, then the +0x18C pointer
   array walked (pointer in $s1 += 4) while the record's +0xAA short is compared with the
   immediate 0xE3E and the publishing scale is f20 = 0x3FF33333 (1.9f); the count is re-read
   at the loop bottom (`lh $v0,0x44($s2)` in both annulled `bnel` slots).
   It is the same address-CSE wall already measured on func_L04_00423178 (retail rematerialises
   each absolute address site by site, this cc1 parks them in callee-saved registers and the
   frame grows), so no C is deposited: read that near miss for the spellings and the frame
   numbers before spending another budget on the family. */
