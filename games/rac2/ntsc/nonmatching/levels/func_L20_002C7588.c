/* Body map for func_L20_002C7588 (level 20, 876 bytes, 219 instructions).  No C
   attempt yet: this worker spent its budget on the near miss of the same file
   (func_L20_002B7A48) and measured the address shapes this body needs.  Not
   linked, not counted.

   The retail (read out of build/reference/20_yeedil.elf at 0x002C7588), the
   head of it:

     addiu $sp,$sp,-0x90
     sd $s0..$s4 / sd $ra
     swc1 $f20,0x70($sp) / swc1 $f21,0x78($sp) /
     swc1 $f22,0x80($sp)                              three float registers
                                                        spilled in the frame
     lui $v0,%hi(D_189F10)
     addiu $s0,$v0,%lo(D_189F10)                       the second hero block
     daddu $s1,$a0,$zero                               the first argument
     daddu $s2,$v0,$zero                               the %hi of D
     daddu $a0,$s0,$zero
     jal f_002E7980
       swc1 $f20,0x70($sp)
     lui $at,(0x3ADA740F >> 16) / ori $at,... / mtc1 $at,$f1
     nop
     c.lt.s $f0,$f1
     nop
     bc1tl .L002C78CC                                  the branch-likely
                                                        float branch
       ld $s0,0x40($sp)
     addiu $a0,$s0,-0xF0
     lui $at,(0x3EBD70A4 >> 16) / ori $at,... / mtc1 $at,$f21   0.005f
     lw $v0,0x229C($a0)
     lui $at,(0x3F000000 >> 16) / mtc1 $at,$f22         1.0f
     addiu $v0,$v0,-0x11
     sltiu $v0,$v0,2
     beql $v0,$zero,.L002C762C
       lbu $a0,0x22B4($a0)
     mtc1 $zero,$f21
     lui $at,(0x3F333333 >> 16) / ori $at,... / mtc1 $at,$f22   0.7f
     b .L002C766C
     .L002C762C: addiu $v0,$zero,1
                 bne $a0,$v0,.L002C7658
     ...

   What the body needs from the compiler:

   - the address of D_00189F10 in two shapes out of one lui: the address in a
     register ("addiu $s0,$v0,%lo(D)") and the %hi copied out
     ("daddu $s2,$v0,$zero") with the %lo added at every use.  Measured on
     this compiler: a pointer to the symbol used as a memory base gives
     "lui $h,%hi(S)" + "addiu $b,$h,%lo(S)"; the same pointer rematerialised
     into the base register of the load gives the retail's form; a 64 bit
     value of the symbol gives "daddu $h2,$h,$zero" + "addiu $x,$h2,%lo(S)";
     a 32 bit pointer that is only ever copied gives the same.  A `long` is
     64 bit in this ABI, a pointer is 32 bit.
   - the float constants must be spelled exactly: 0.005f is 0x3EBD70A4,
     0.7f is 0x3F333333, 1.0f is 0x3F000000 and the first one is 0x3ADA740F.
   - the float branch is the branch-LIKELY form ("c.lt.s" + "bc1tl"); a
     negated comparison with the operands swapped selects its polarity
     ("if (!(a > b)) {} else { x = 1; }" emits "c.lt.s b,a" + "bc1tl", the
     plain "if (a < b)" emits "c.lt.s a,b" + "bc1fl").
 */
