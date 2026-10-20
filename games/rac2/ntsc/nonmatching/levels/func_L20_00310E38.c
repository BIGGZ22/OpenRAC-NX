/* func_L20_00310E38 (1212 bytes, 303 instructions, level 20 only) -- MAPPING,
   not integrated.

   This body reaches small data through $gp (seven accesses), so it belongs in
   the -G8 unit: src/levels/20_yeedil/L20_002DEBE0-g8.c.  The marker is back
   in place there.

   WHAT IT IS.  The yeedil level's per-object update: it walks an array of
   objects, marks the ones that need a re-init, and then runs the level's
   "one shot" set-up when the level's own state asks for it.  It is not a
   dispatch and has no jump table: one loop over the objects, then a straight
   tail.

   THE PROLOGUE (0x00310E38 .. 0x00310E94):

     0x00310E38  addiu  $sp,$sp,-0xE0        the frame is 0xE0
     0x00310E3C  sd     $s1,0x88($sp)        the saved GPRs $s0..$s7 and $ra
     0x00310E40  sd     $s7,0xB8($sp)        at 0x80..0xC0, and $f20 at 0xD0
     0x00310E44  daddu  $s1,$zero,$zero      $s1 = the loop counter (0)
     0x00310E48  sd     $s0,0x80($sp)
     0x00310E4C  sd     $s2,0x90($sp)
     0x00310E50  sd     $s3,0x98($sp)
     0x00310E54  sd     $s4,0xA0($sp)
     0x00310E58  sd     $s5,0xA8($sp)
     0x00310E5C  sd     $s6,0xB0($sp)
     0x00310E60  sd     $ra,0xC0($sp)
     0x00310E64  jal    func_00310BD0        the level's set-up, called first
     0x00310E68  swc1   $f20,0xD0($sp)
     0x00310E6C  lui    $v1,%hi(D_1BDCC0)   the 64-bit address of the level
     0x00310E70  daddu  $s7,$v1,$zero       state (D_1BDCC0)
     0x00310E74  addiu  $v1,$v1,%lo(D_1BDCC0)
     0x00310E78  lh     $v0,0x44($v1)       the level's counter at +0x44
     0x00310E7C  blezl  $v0,...             if it is <= 0, skip the whole loop
     0x00310E80  lw     $v1,-0x7600($gp)     = 0x001A79F0, the object array
     0x00310E84  daddu  $s3,$v1,$zero       $s3 = its 64-bit address
     0x00310E88  addiu  $s6,$zero,0xE29     the three type constants:
     0x00310E8C  addiu  $s5,$zero,0xD54       0xE29, 0xD54, 0xFA0
     0x00310E90  addiu  $s4,$zero,0xFA0
     0x00310E94  addiu  $s2,$s3,0x18C       $s2 = &array[0x18C]

   THE LOOP (0x00310E98 .. 0x00310F20).  It walks the pointers at
   $s3+0x18C, and for each object:

     0x00310E98  lw     $s0,0x0($s2)        the object pointer
     0x00310E9C  beqz   $s0,...             skip a null one
     0x00310EA0  lh     $v0,0xAA($s0)       its type at +0xAA
     0x00310EA4  bne    $v0,$s6,...         0xE29 -> the "reset" path
     0x00310EAC  lhu    $v0,0x34($s0)       its flags at +0x34
     0x00310EB0  ori    $v0,$v0,0x800       set the 0x800 flag
     0x00310EB4  sh     $v0,0x34($s0)
     0x00310EBC  bne    $v0,$s5,...         0xD54 -> the other path
     0x00310EC4  jal    func_00375EE0       a per-object call
     0x00310ECC  beql   $s0,$zero,...       a null object -> the end
     0x00310ED4  lh     $v0,0xAA($s0)
     0x00310ED8  bnel   $v0,$s4,...         0xFA0 -> the end
     0x00310EE0  lw     $a1,0x68($s0)       its field at +0x68
     0x00310EE4  beqz   $a1,...             no field -> skip
     0x00310EEC  lui    $a2,0x80AAAAD2      two float constants in a2/a3
     0x00310EF4  lui    $a3,0x80396464
     0x00310EFC  mtc1   $at,$f12            0x3D7A35E0
     0x00310F00  addiu  $a1,$a1,0xC
     0x00310F08  jal    func_0033AE48       the "spawn" call
     0x00310F14  addiu  $s1,$s1,0x1        the counter
     0x00310F18  slt     $v0,$s1,$v0         against the level's counter
     0x00310F1C  bnez   $v0,0x00310E98      the loop back edge

   AFTER THE LOOP (0x00310F24 .. 0x003112C8).  The level's own state decides
   what happens next:

     0x00310F24  addiu  $v0,$a1,-0x4        the state at +0x30 is 0 or 1?
     0x00310F28  sltiu  $v0,$v0,0x2
     0x00310F2C  beqz   $v0,0x00311144       yes -> a different path
     0x00310F34  lw     $a2,0x19C($a0)
     0x00310F44  daddu  $s5,$v0,$zero       $s5 = the 64-bit D_00189E20
     0x00310F60  bne    $a1,$v0,...         the state 4 -> another path
     0x00310F68  sw     $v0,0x5C0($v1)      D_00189E20[0x5C0] = the field
     0x00310F74  lw     $s0,-0x5BBC($gp)    three small globals
     0x00310F7C  lw     $s3,-0x5BB8($gp)
     0x00310F80  lw     $s0,-0x5BB4($gp)
     0x00310F88  lw     $s3,-0x5BB0($gp)
     0x00310F8C  addiu  $s4,$s7,%lo(D_1BDCC0)
     0x00310F94  lw     $v1,0x34($s4)       the level's state at +0x34
     0x003110A8  por    $v0,$zero,$zero     a 128-bit zero for the local
     0x003110C4  sq     $v0,0x70($sp)
     0x003110E0  jal    func_00302728       the local at sp+0x70 is set up
     0x003110E4  jal    func_00302260       and copied
     0x003110F4  jal    func_00302290
     0x00311118  jal    func_00302548
     0x0031112C  jal    func_00420850
     0x003111DC  jal    func_0033AE90
     0x00311238  jal    func_00302260
     0x00311280  jal    func_0041A018       the level's big set-up call

   THE CALLS: 20 in all, to 17 distinct callees - func_00310BD0, func_00375EE0,
   func_0033AE48, func_00301FE8, func_00326488, func_00302D50, func_00326528,
   func_00302548, func_00302260, func_003B4240, func_003B4440, func_002FAE78,
   func_00302728, func_00302290, func_00420850, func_0033AE90, func_0041A018.

   THE $gp ACCESSES (seven, which is why the body needs -G8):

     0x00310E80  lw $v1,-0x7600($gp)   = 0x001A79F0
     0x00310F74  lw $s0,-0x5BBC($gp)   = 0x001A9434
     0x00310F7C  lw $s3,-0x5BB8($gp)   = 0x001A9438
     0x00310F80  lw $s0,-0x5BB4($gp)   = 0x001A943C
     0x00310F88  lw $s3,-0x5BB0($gp)   = 0x001A9440
     0x003111CC  lw $v1,-0x5BC0($gp)   = 0x001A9430
     0x00311210  lw $v0,-0x5BC0($gp)   = 0x001A9430

   The 128-bit zero at 0x003110A8 (`por $v0,$zero,$zero ; sq $v0,0x70($sp)`)
   is the same 128-bit zero store the compiler profile folds to `sq $zero`
   whenever the register is dead after the store (see the note in
   nonmatching/levels/func_L20_0039BB18.c): here the zero feeds only one
   store, so the por is likely to be lost the same way.

*/
