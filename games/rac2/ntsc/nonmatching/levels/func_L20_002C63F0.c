/* Body map for func_L20_002C63F0 (level 20, 704 bytes, 176 instructions).  No C
   attempt yet: this worker spent its budget on the near miss of the same file
   (func_L20_002B7A48) and measured the address shapes this body needs.  Not
   linked, not counted.

   The retail (read out of build/reference/20_yeedil.elf at 0x002C63F0), the
   head of it:

     addiu $sp,$sp,-0x30 / sd $s0 / sd $s1 / sd $ra
     swc1 $f20,0x20($sp)                              a float register spilled
                                                        in the frame
     lui $v0,%hi(D_189E20)
     addiu $a3,$v0,%lo(D_189E20)                       the hero block
     addiu $v1,$zero,1
     lbu  $a2,0x22B4($a3)                             the state byte
     beq  $a2,$v1,.L6448                               the state byte against 1
       daddu $a0,$v0,$zero                             the %hi of D
     slti $v0,$a2,2
     beql $v0,$zero,.L6438
       addiu $v0,$zero,4
     beqz $a2,.L6474
       addiu $a2,$a0,%lo(D_189E20)                     the address, the %lo at
                                                        the use
     b    .L64AC
       addiu $t0,$zero,4
     .L6438: beq $a2,$v0,.L6474
       addiu $a2,$a0,%lo(D_189E20)
       b .L64AC
       addiu $t0,$zero,4
     .L6448: lui $at,(0x3F199999 >> 16) / ori $at,... / mtc1 $at,$f0   0.6f
             lui $at,(0x3EE66667 >> 16) / ori $at,... / mtc1 $at,$f2   0.35f
             lui $at,(0x3E99999A >> 16) / ori $at,... / mtc1 $at,$f1   0.3f
             b .L649C
               swc1 $f0,0x258($a3)
     .L6474: lui $at,(0x3F4CCCCD >> 16) / ori $at,... / mtc1 $at,$f0   0.8f
             lui $at,(0x3F333333 >> 16) / ori $at,... / mtc1 $at,$f2   0.7f
             lui $at,(0x3EE66666 >> 16) / ori $at,... / mtc1 $at,$f1   0.35f
             swc1 $f0,0x258($a3)
     .L649C: swc1 $f1,0x260($a3) / swc1 $f2,0x25C($a3)
     addiu $a2,$a0,%lo(D_189E20)
     addiu $t0,$zero,4
     .L64AC: lw $a3,0x229C($a2)
             bnel $a3,$t0,.L64E4
               lw $v1,0x2294($a2)
             lh $v0,0x4AC($a2) / lw $v1,0x1B8($a2) /
             slt $v0,$v0,$v1 / beqz $v0,.L6594
               addiu $s0,$a0,%lo(D_189E20)
             lbu $v0,0x49E($a2)
             bnel $v0,$zero,.L6598
               lbu $v0,0x287($s0)
             lwc1 $f0,0x4B8($a2)
             b .L6594
               swc1 $f0,0x25C($a2)
     .L64E4: addiu $v0,$zero,6
             bne $v1,$v0,.L6500
             ...

   What the body needs from the compiler:

   - the address of D_00189E20 in two shapes out of one lui: the address in a
     register ("addiu $a3,$v0,%lo(D)") and the %hi copied out
     ("daddu $a0,$v0,$zero") with the %lo added at every use
     ("addiu $a2,$a0,%lo(D)").  Measured on this compiler: a pointer to the
     symbol used as a memory base gives "lui $h,%hi(S)" + "addiu $b,$h,%lo(S)";
     the same pointer rematerialised into the base register of the load gives
     the retail's form; a 64 bit value of the symbol gives "daddu $h2,$h,$zero"
     + "addiu $x,$h2,%lo(S)"; a 32 bit pointer that is only ever copied gives
     the same.  A `long` is 64 bit in this ABI, a pointer is 32 bit.
   - the float constants must be spelled exactly: 0.6f is 0x3F199999 (not
     0.6f rounded, and NOT 0x3F19999A), 0.35f is 0x3EE66667 (the retail's
     first one) and 0x3EE66666 (the second), 0.3f is 0x3E99999A, 0.8f is
     0x3F4CCCCD and 0.7f is 0x3F333333.
   - the retail builds each float with "lui $at" + "ori $at,$at,lo" + "mtc1",
     so the constants have to be spelled as the exact bit patterns.
 */
