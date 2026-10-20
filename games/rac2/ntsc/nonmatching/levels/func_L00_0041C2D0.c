/* func_L00_0041C2D0 - 2224 bytes, 27 place(s), level 00_aranos_tutorial,
   retail at 0x0041c2d0. Screened with tools/w10_screen.py: 56 call(s)
   [func_0029EFD0@0041C318, func_002B37E8@0041C398, func_00282D30@0041C418,
   func_00282CC0@0041C428, func_00282D30@0041C444, func_00282CC0@0041C454,
   func_00282D30@0041C470, func_00282CC0@0041C480, func_00282CC0@0041C494,
   func_002761F8@0041C4AC, func_00282CF0@0041C4C4, func_00282D30@0041C4D8,
   func_00282CC0@0041C4E8, func_00282C88@0041C510, func_00283B98@0041C578,
   func_002AF000@0041C590, func_L00_002D3798@0041C5A8,
   func_00282C88@0041C5B0, func_002833F0@0041C5BC, func_002833F0@0041C5F0,
   func_00283738@0041C600, func_00283738@0041C624, func_00283658@0041C630,
   func_002AB6D8@0041C640, func_00283678@0041C650, func_00277F48@0041C698,
   func_002ABB78@0041C6D8, func_002AB1F0@0041C6F4, func_002AFFD8@0041C710,
   func_00282D30@0041C724, func_00282CC0@0041C738, func_002AF580@0041C7C4,
   func_002AFFD8@0041C7EC, func_00282E48@0041C7FC, func_002ACEC0@0041C848,
   func_00282D30@0041C874, func_00282CC0@0041C884, func_00282D30@0041C8B0,
   func_00282CC0@0041C8C0, func_002AFFD8@0041C930, func_002ACEC0@0041C94C,
   func_L00_0032A088@0041C980, func_0029F338@0041C994,
   func_L00_0032A0B8@0041C9A4, func_L00_00322A98@0041CA24,
   func_L00_00322AC0@0041CA4C, func_L00_00322AC0@0041CA74,
   func_L00_00322AC0@0041CA90, func_002AB668@0041CAA4,
   func_002ACDE8@0041CAB4, func_002ACDE8@0041CAC4, func_002FED90@0041CAE0,
   func_002AFFD8@0041CB10, func_00282DB0@0041CB1C, func_00282D30@0041CB30,
   func_00282CF0@0041CB40]; data addresses materialised [0xbf9c0000,
   0x140000, 0x260000, 0xffff0000, 0xff0a0000, 0x190000, 0x3e990000,
   0x140000, 0x3c880000, 0x260000, 0x1c0000, 0x190000, 0x3dfa0000, 0x1c0000,
   0x190000, 0x1c0000, 0x190000, 0x3e320000, 0x3eb20000, 0x3d080000,
   0x3edf0000, 0x1c0000, 0x190000, 0x1b0000, 0x190000, 0x190000, 0x190000,
   0xbedf0000, 0x1b0000, 0x1b0000, 0x1b0000, 0x1b0000, 0x1b0000]; float
   constants [0x3f000000, 0x3f400000, 0x190000, 0x3f800000, 0x41f00000,
   0x3f800000, 0x42800000, 0x42800000, 0x3f800000, 0x1b0000]; $gp-relative
   small data: no   saves: -; callee-saved registers saved [-]; words plain
   C cannot emit [LQ/SQ 7A420000@0x022c, LQ/SQ 7C620000@0x0230, LQ/SQ
   7AA20000@0x0820, LQ/SQ 7C620000@0x0824]. Head: addiu $sp, $sp, -0x120 ;
   sd $s1, 0xC8($sp) ; sd $s0, 0xC0($sp) ; daddu $s1, $a0, $zero ; sd $s2,
   0xD0($sp) ; daddu $s0, $a3, $zero. Not matched in this wave: the 128-bit
   copy needs an `lq`/`sq` pair; plain C reaches it only through a
   `mode(TI)` local, and this cc1 folds the destination offset into the `sq`
   where retail materialises the pointer first. What would unblock it: the
   exact source expression tree, so this cc1 picks the same registers in the
   same order. */
