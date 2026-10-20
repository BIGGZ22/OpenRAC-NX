/* Body map for func_L20_002C5A48 (level 20, 584 bytes, 146 instructions).  No C
   attempt yet: this worker spent its budget on the near miss of the same file
   (func_L20_002B7A48) and measured the address shapes this body needs.  Not
   linked, not counted.

   The retail (read out of build/reference/20_yeedil.elf at 0x002C5A48):

     addiu $sp,$sp,-0x20 / sd $s0,$s1,$s2 / sd $ra
     lui $v0,%hi(D_189E20)
     addiu $s0,$v0,%lo(D_189E20)                       the hero block
     daddu $s2,$v0,$zero                               the %hi of D, for the
                                                        %lo at each use
     addiu $v0,$zero,1
     lbu  $v1,0x22B4($s0)
     bne  $v1,$v0,.L5AB4                               the state byte against 1
       daddu $s1,$a0,$zero                             the first argument
     lw   $v0,0x24CC($s0) / lh $v1,0x24D4($s0) /
     lw   $a0,0x1860($s0)
     sw   $v0,0x24D0($s0)
     beqz $a0,.L5A9C
       sw  $v1,0x24CC($s0)
     jal f_003253A8
     sw   $zero,0x1860($s0)
     .L5A9C: lw $a0,0x1864($s0)
       beql $a0,$zero,.L5AB8
         addiu $s0,$s2,%lo(D_189E20)
       jal f_003253A8
       sw $zero,0x1864($s0)
     .L5AB4: addiu $s0,$s2,%lo(D_189E20)
     .L5AB8: addiu $v1,$zero,4
       lbu  $v0,0x22B4($s0)
       bne  $v0,$v1,.L5B50                             the state byte against 4
     jal f_003338F0
       lw   $a0,0x2290($s0)
     beql $v0,$zero,.L5AE8
       lw   $a1,0xC18($s0)
     lwc1 $f0,0x24CC($s0) / cvt.s.w $f0,$f0 / swc1 $f0,0x0($v0)
     lw   $a1,0xC18($s0)
     .L5AE8: addiu $a0,$s0,0x80 / addiu $v1,$a1,0x10 /
             lq $v0,0x0($v1) / sq $v0,0x0($a0)          the first 128 bit copy
             addiu $a1,$a1,0xF0 / addiu $v1,$s0,0x90 /
             lq $v0,0x0($a1) / sq $v0,0x0($v1)          the second one
     lh   $a0,0x24D4($s0)
     bnez $s1,.L5B20
       sw  $a0,0x24CC($s0)
     daddu $a0,$zero,$zero
     jal f_00320D50
       daddu $a1,$zero,$zero
     .L5B20: jal f_002C59C8
     lw   $v0,0x3A4($s0)
     addiu $a0,$zero,-0x2 / addiu $a1,$zero,-0x3 /
     addiu $v1,$zero,1
     and  $v0,$v0,$a0
     sh   $v1,0x34E($s0)
     and  $v0,$v0,$a1
     sb   $zero,0x22B5($s0)
     sw   $v0,0x3A4($s0)
     sw   $zero,0x3A0($s0)
     .L5B50: jal f_002B7FD0
       addiu $s0,$s2,%lo(D_189E20)
     lui $a2,%hi(f_0030B818) / lui $a3,%hi(f_0030B868) /
     lui $t0,%hi(f_0030C138) / daddu $t1/$t2,$zero,$zero /
     addiu $a2/$a3/$t0 with their %lo / addiu $a0,$zero,0x23 /
     daddu $a1,$zero,$zero
     jal f_0030AD58
       sb $zero,0x22B4($s0)
     jal f_0030AD08
       addiu $a0,$zero,3
     lw   $v1,0xC14($s0)
     beqz $v1,.L5BA8
     lhu  $v0,0x34($v1) / andi $v0,$v0,0xFFF9 / sh $v0,0x34($v1)
     .L5BA8: jal f_002B9610
       sw $zero,0xC14($s0)
     lw   $a0,0xC18($s0) / addiu $a1,$s0,0x80 /
     sw   $a0,0x2290($s0)
     addiu $v1,$a0,0x10
     lq   $v0,0x0($a1) / sq $v0,0x0($v1)              the third 128 bit copy
     lui  $at,(0x3F800000 >> 16) / mtc1 $at,$f0        1.0f
     sw   $zero,0x9C($a0)
     jal f_002E63E0
       swc1 $f0,0xC24($s0)
     lw   $a0,0x2290($s0)
     addiu $v1,$zero,0xFF
     lbu  $v0,0x42($a0)
     beq  $v0,$v1,.L5C0C
     lui  $a3,%hi(D_1B5600)
     lbu  $a2,0x40($a0)
     daddu $a1,$v0,$zero
     jal f_002B79B0
       addiu $a3,$a3,%lo(D_1B5600)
     lw   $v1,0x2290($s0)
     sw   $v0,0x58($v1)
     lw   $a0,0x2290($s0)
     .L5C0C: lui $a3,%hi(D_1B5C00) / addiu $a3,$a3,%lo(D_1B5C00) /
             lbu $a1,0x43($a0) / jal f_002B79B0
               lbu $a2,0x41($a0)
     lw   $v1,0x2290($s0)
     addiu $a0,$zero,0x61
     sw   $v0,0x5C($v1)
     lw   $v0,0x2294($s0)
     bnel $v0,$a0,.L5C54
       daddu $a0,$zero,$zero
     lui $v1,%hi(D_001A8F00) / lw $v1,%lo(D_001A8F00)($v1)
     addiu $v0,$zero,2
     beq  $v1,$v0,.L5C5C
       addiu $v0,$zero,6
     beq  $v1,$v0,.L5C5C
       daddu $a0,$zero,$zero
     .L5C54: jal f_002D7730
       addiu $a1,$zero,1
     .L5C5C: addiu $v0,$s2,%lo(D_189E20)
     addiu $a0,$zero,4
     lbu  $v1,0x22B4($v0)
     bne  $v1,$a0,.L5C7C
       ld   $s0,0x0($sp)
     jal f_002EDDA0
     ld   $s0,0x0($sp)
     .L5C7C: the epilogue

   What the body needs from the compiler:

   - the address of D_00189E20 in two shapes out of one lui: the address in a
     register ("lui $v0,%hi(D)" + "addiu $s0,$v0,%lo(D)") and the %hi copied
     into $s2 with the %lo added at each use ("daddu $s2,$v0,$zero" +
     "addiu $x,$s2,%lo(D)").  Measured on this compiler: a pointer to the
     symbol used as a memory base gives "lui $h,%hi(S)" + "addiu $b,$h,%lo(S)";
     the same pointer rematerialised into the base register of the load gives
     the retail's form; a 64 bit value of the symbol gives "daddu $h2,$h,$zero"
     + "addiu $x,$h2,%lo(S)"; a 32 bit pointer that is only ever copied gives
     the same.  A `long` is 64 bit in this ABI, a pointer is 32 bit.
   - the 128 bit copies are in the "plain register address" shape (the address
     in a register, the lq and the sq at offset 0), which is the shape that
     matches.
   - the float constant is 1.0f (0x3F800000), and the int-to-float conversion
     is "lwc1" + "cvt.s.w" + "swc1" (the value stays in the FPR).
 */
