/* Body map for func_L20_002C57D0 (level 20, 504 bytes, 126 instructions).  No C
   attempt yet: this worker spent its budget on the near miss of the same file
   (func_L20_002B7A48) and measured the address shapes this body needs.  Not
   linked, not counted.

   The retail (read out of build/reference/20_yeedil.elf at 0x002C57D0):

     addiu $sp,$sp,-0x30 / sd $s0..$s4 / sd $ra        the frame, five callee
                                                        saved registers
     lui   $s0,%hi(D_189E20)                            the hero block
     daddu $s1,$a0,$zero / daddu $s2,$a2,$zero /
     daddu $s3,$a1,$zero                               the three int
                                                        arguments, sign
                                                        extended to 64 bit
     jal f_002C2560
       daddu $s4,$s0,$zero                              the %hi of D copied
                                                        into $s4
     addiu $s0,$s0,%lo(D_189E20)                       the address in the lui
                                                        register itself
     addiu $v0,$zero,1
     jal f_002C4D20
       sb   $v0,0x22B6($s0)                            D[0x22B6] = 1
     lw   $v0,0xC18($s0)
     addiu $a0,$s0,0x18E0
     jal f_00338100
       sw   $v0,0x2290($s0)                            D[0x2290] = D[0xC18]
     jal f_00338100
       addiu $a0,$s0,0x1A30
     lw   $a0,0x2290($s0)
     addiu $v0,$zero,-1 / addiu $v1,$zero,4
     bne  $s1,$v1,.L584C
       sw   $v0,0x9C($a0)                             the pointer's 0x9C = -1
     lbu  $v0,0x22B4($s0)
     beq  $v0,$s1,.L5854                              the state byte against
                                                        the first argument
     .L584C: jal f_002E6D90
     .L5854: jal f_002B9610
             jal f_002B7FD0
     addiu $v0,$s4,%lo(D_189E20)                       the address again, the
                                                        %lo added at the use
     andi $a0,$s1,0xFF
     sw   $s2,0x2290($v0)                              D[0x2290] = the third
                                                        argument
     addiu $v1,$zero,1
     sb   $s1,0x22B4($v0)                              D[0x22B4] = the first
                                                        argument
     bne  $a0,$v1,.L58C4
       sw   $s2,0xC14($v0)                             D[0xC14] = the third
                                                        argument
     lui  $v1,(0x140000 >> 16) / lw $v0,-0x7CE0($v1)   the global at
                                                        0x00138320
     andi $v0,$v0,0x10 / beqz $v0,.L58C4
     lui  $a2,%hi(f_0030C700) / lui $a3,%hi(f_0030C748) /
     lui  $t0,%hi(f_0030CC50) / addiu $a0,$zero,0x23 /
     addiu $a2/$a3/$t0 with their %lo
     daddu $a1,$zero,$zero / daddu $t1,$zero,$zero
     jal f_0030AD58
       daddu $t2,$zero,$zero
     jal f_0030AD08
       addiu $a0,$zero,3
     .L58C4: jal f_002C63F0
       addiu $s0,$s4,%lo(D_189E20)
     lwc1 $f0,0x258($s0) / lwc1 $f1,0x25C($s0) /
     swc1 $f0,0x250($s0) / swc1 $f1,0x254($s0)         the two floats moved
                                                        down two words
     lw   $v1,0x2290($s0)
     lui  $at,(0x3F800000 >> 16) / mtc1 $at,$f0        1.0f
     lhu  $v0,0x34($v1) / ori $v0,$v0,6 / sh $v0,0x34($v1)
     swc1 $f0,0xC24($s0)
     jal f_002E63E0
       swc1 $f0,0xC20($s0)
     jal f_0032E5DC
       lw   $a0,0x2290($s0)
     addiu $v0,$zero,1
     beq  $s1,$v0,.L5928
       sh   $s3,0x24F2($s0)
     addiu $v0,$zero,4
     beq  $s1,$v0,.L594C
       daddu $a0,$s3,$zero
     b    .L5980
     .L5928: lhu $v0,0x24CC($s0) / lw $v1,0x24D0($s0) /
             sh $v0,0x24D4($s0) / slti $a0,$v1,5 /
             bnez $a0,.L597C
               sw $v1,0x24CC($s0)
             addiu $v0,$zero,4
             b .L597C
               sw $v0,0x24CC($s0)
     .L594C: addiu $a0,$s0,0x80 / addiu $v1,$s2,0x10 /
             lq $v0,0x0($v1) / sq $v0,0x0($a0)          the first 128 bit copy
             addiu $a1,$s2,0xF0 / addiu $v1,$s0,0x90 /
             lq $v0,0x0($a1) / sq $v0,0x0($v1)          the second one
             lhu $a0,0x24CC($s0) / addiu $v0,$zero,3 /
             sw $v0,0x24CC($s0) / sh $a0,0x24D4($s0)
     .L597C: daddu $a0,$s3,$zero
     .L5980: jal f_002D7730
               addiu $a1,$zero,2
             addiu $v1,$zero,4
             bne $s1,$v1,.L59AC
               ld $s0,0x0($sp)
             lui $at,(0x40A00000 >> 16) / mtc1 $at,$f12   5.0f
             addiu $a0,$zero,0xF
             jal f_002E6090
               daddu $a1,$zero,$zero
             ld $s0,0x0($sp)
     .L59AC: the epilogue, $s0..$s4 and $ra restored

   What the body needs from the compiler:

   - the address of D_00189E20 in TWO shapes out of one lui: the address in
     the lui register itself ("lui $s0,%hi(D)" + "addiu $s0,$s0,%lo(D)") and
     the %hi copied into $s4 with the %lo added at every use
     ("daddu $s4,$s0,$zero" + "addiu $x,$s4,%lo(D)").  Measured on this
     compiler: a pointer to the symbol used as a memory base gives
     "lui $h,%hi(S)" + "addiu $b,$h,%lo(S)"; the same pointer rematerialised
     into the base register of the load gives the retail's form; a 64 bit
     value of the symbol gives "daddu $h2,$h,$zero" + "addiu $x,$h2,%lo(S)";
     a 32 bit pointer that is only ever copied gives the same.  A `long` is
     64 bit in this ABI, a pointer is 32 bit.
   - the 128 bit copies are in the "plain register address" shape (the
     address in a register, the lq and the sq at offset 0), which is the
     shape that matches.
   - the float constants are 1.0f (0x3F800000) and 5.0f (0x40A00000); the
     "mtc1 $at,$f" pair is what the retail uses, so the constant has to be
     spelled exactly (0.005f is 0x3EBD70A4 and 0.7f is 0x3F333333).
   - the stores in the delay slots of the calls (D[0x22B6] = 1,
     D[0x2290] = D[0xC18], D[0xC14] = the third argument) are the retail's
     shape: a store in a delay slot writes the previous call's result.
 */
