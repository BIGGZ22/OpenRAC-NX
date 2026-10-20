/* func_L16_00395ED0 - 532 bytes, 2 place(s), level 16_snivelak, retail at
   0x00395ed0. Screened with tools/w10_screen.py: 16 call(s)
   [func_002AA548@00395F4C, func_002833F0@00395F5C, func_00283248@00395F80,
   func_00283248@00395F98, func_00282E20@00395FA8, func_00282E20@00395FB8,
   func_002AA150@00395FDC, func_00283180@00395FF8, func_00283198@00396008,
   func_00283098@00396054, func_00282CC0@00396064, func_00283810@00396084,
   func_00283958@00396090, func_002AB6D8@003960A0, func_00335370@003960A8,
   func_003353C8@003960B0]; data addresses materialised [0x39410000,
   0x3c350000, 0x1b0000, 0x1b0000, 0x1b0000, 0x1b0000, 0x1b0000, 0x1b0000,
   0x1b0000, 0x1b0000]; float constants [0x3f800000, 0x1b0000, 0x1b0000];
   $gp-relative small data: no   saves: -; callee-saved registers saved [-];
   words plain C cannot emit [LQ/SQ 78620000@0x0170, LQ/SQ 7C820000@0x0174].
   Head: addiu $sp, $sp, -0xE0 ; addiu $v1, $zero, -0x1 ; sd $s1, 0x98($sp)
   ; sd $s2, 0xA0($sp) ; sd $ra, 0xA8($sp) ; swc1 $f24, 0xD0($sp). Not
   matched in this wave: the 128-bit copy needs an `lq`/`sq` pair; plain C
   reaches it only through a `mode(TI)` local, and this cc1 folds the
   destination offset into the `sq` where retail materialises the pointer
   first. What would unblock it: the exact source expression tree, so this
   cc1 picks the same registers in the same order. */
