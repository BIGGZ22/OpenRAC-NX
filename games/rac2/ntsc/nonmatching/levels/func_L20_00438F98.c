/* Near miss from src/levels/20_yeedil/L20_0039C448-g8.c: 1548 bytes; not attempted
   (out of budget), the note is what the head of the body shows.

   The dispatch of this body is the JUMP TABLE of the level's own data - the
   table is at 0x002B57C0 (the label jtbl_002B57C0 in
   build/20_yeedil/asm/data/data_BAC0.data.s, five words of 0x0043xxxx), so
   it is reachable from C the way the common file describes:

       extern void *D_L20_002B57C0[5];

   The retail's head:

       lbu    $v1,0x20($s3)
       sltiu  $v0,$v1,0x5
       beqz   $v0,.L00439574
         lw    $s1,0x68($s3)
       lui    $v0,%hi(jtbl_002B57C0)
       sll    $v1,$v1,2
       addiu  $v0,$v0,%lo(jtbl_002B57C0)
       addu   $v1,$v1,$v0
       lw     $a0,0x0($v1)
       jr     $a0
         nop

   so the C is a computed goto (`goto *D_L20_002B57C0[state];`) with a
   `static void *volatile` array of &&labels to keep the case bodies alive.
   The table is in the level's data, not in the object's .rdata, so the build
   can place it (the build reads its address from the retail's lui/addiu
   pair).

   The body belongs in the -G8 unit: it reads -0x7600($gp) and the two
   globals D_1A7BC8 and D_19B2E8. */
