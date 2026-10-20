/* func_L18_003FA180 - 1596 bytes, 3 place(s), level 18_damosel, retail at
   0x003fa180. Screened with tools/w10_screen.py: 0 call(s) [-]; data
   addresses materialised [0x2c0000, 0x200000, 0x1b0000, 0x1b0000, 0x200000,
   0x1b0000, 0x200000, 0x1b0000, 0x1b0000, 0x1b0000, 0x200000, 0x1b0000,
   0x200000, 0x1b0000]; float constants [-]; $gp-relative small data: no
   saves: -; callee-saved registers saved [-]; words plain C cannot emit
   [-]. Head: daddu $t2, $a0, $zero ; sltiu $v0, $a1, 0x8 ; beqz $v0, . + 4
   + (0x66 << 2) ; lw $t1, 0x68($t2) ; lui $v0, 0x2C ; sll $v1, $a1, 2. Not
   matched in this wave: the body is straight-line code whose register
   allocation and the materialisation of its constant addresses have to be
   reproduced word for word. What would unblock it: the exact source
   expression tree, so this cc1 picks the same registers in the same order. */
