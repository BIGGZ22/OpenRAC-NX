/* func_L20_0039BEC8 (1252 bytes, 313 instructions, level 20 only) -- MAPPING,
   not integrated.

   This body reaches small data through $gp (twelve accesses, six of them
   floats), so it belongs in the -G8 unit:
   src/levels/20_yeedil/L20_002DEBE0-g8.c.  The marker is back in place
   there.

   WHAT IT IS.  The second half of the yeedil emitter: the first half is
   func_L20_0039BB18 (which builds a velocity vector and hands it over); this
   one draws the object's trail - it fills two arrays of four positions each
   with a rolling history, moving the old values down and computing the new
   one at the head.

   THE PROLOGUE (0x0039BEC8 .. 0x0039BF38):

     0x0039BEC8  addiu  $sp,$sp,-0x1D0        the frame is 0x1D0
     0x0039BECC  sw     $a0,0x140($sp)       the argument is spilled
     0x0039BED0  sd     $s0,0x170($sp)       the saved GPRs $s0..$s7, $fp, $ra
     0x0039BED4  sd     $s1,0x178($sp)       at 0x170..0x1B8 and the saved
     0x0039BED8  addiu  $s0,$sp,0x90          FPRs $f21, $f20 at 0x1C8, 0x1C0
     0x0039BEDC  sd     $s2,0x180($sp)
     0x00310E04  sd     $s3,0x188($sp)       (the $s0 = sp+0x90 is a pointer
     0x0039BEE4  sd     $s4,0x190($sp)        to the first of the two arrays)
     0x0039BEE8  sd     $s5,0x198($sp)
     0x0039BEEC  sd     $s6,0x1A0($sp)
     0x0039BEF0  sd     $s7,0x1A8($sp)
     0x0039BEF4  sd     $fp,0x1B0($sp)
     0x0039BEF8  sd     $ra,0x1B8($sp)
     0x0039BEFC  swc1   $f21,0x1C8($sp)
     0x0039BF00  swc1   $f20,0x1C0($sp)
     0x0039BF04  lw     $v1,0x140($sp)       the argument again
     0x0039BF08  lw     $v0,0x68($a0)        the object's field at +0x68
     0x0039BF0C  daddu  $a0,$s0,$zero        the first array (sp+0x90)
     0x0039BF10  addiu  $a1,$v1,0xC0        the object's field at +0xC0
     0x0039BF14  jal    func_00302D30       the set-up call
     0x0039BF18  sw     $v0,0x144($sp)       the field is spilled to 0x144
     0x0039BF1C  lw     $v0,0x140($sp)
     0x0039BF20  addiu  $a0,$sp,0xC0        a 16-byte copy of the object
     0x0039BF24  addiu  $v1,$v0,0x10
     0x0039BF28  lq     $v0,0x0($v1)        (the lq/sq pair, the 128-bit
     0x0039BF2C  sq     $v0,0x0($a0)         copy wall)
     0x0039BF30  lw     $a0,-0x384($gp)      = 0x001AEC6C, a small global
     0x0039BF38  daddu  $s3,$zero,$zero      $s3 = 0, the loop counter

   THE FIRST LOOP (0x0039BFC8 .. 0x0039C054), four iterations over the
   history of the first array:

     0x0039BFC8  lwc1   $f1,0x0($s4)        the old x
     0x0039BFCC  andi   $s0,$s3,0x1         the parity of the index
     0x0039BFD0  lwc1   $f0,0x0($s6)        the old y
     0x0039BFD8  swc1   $f1,0x0($s5)        the new x into the array
     0x0039BFDC  addiu  $v1,$gp,-0x380      a table of four ints in the small
     0x0039BFE4  movn   $v0,$zero,$s0       data, indexed by the parity
     0x0039BFE8  addu   $v0,$v0,$v1
     0x0039BFEC  lw     $a0,0x0($v0)        the chosen int
     0x0039BFEC  swc1   $f0,0x0($s7)        the new y into the array
     0x0039C000  jal    func_00303410       the call that draws the segment
     0x0039C00C  jal    func_00302230       a second, tiny call
     0x0039C014  lwc1   $f0,-0x374($gp)     = 0x001AEC7C, a float constant
     0x0039C018  bnez   $s0,...             the parity decides the constant
     0x0039C020  lwc1   $f0,-0x378($gp)     = 0x001AEC78
     0x0039C030  addiu  $s3,$s3,0x1         the counter
     0x0039C034  addiu  $s1,$s1,0x10        the pointers walk on
     0x0039C038  addiu  $s2,$s2,0x4
     0x0039C048  slti   $v0,$s3,0x4         four iterations
     0x0039C050  addiu  $s4,$s4,0x8

   THE SECOND LOOP (0x0039C130 .. 0x0039C1D0) is the same shape for the
   other array, with the extra test `slti $v0,$s3,0x2` and a different
   constant.

   THE TAIL (0x0039C1D4 .. 0x0039C3A8):

     0x0039C1D4  lq     $v0,0x0($a0)        another 128-bit copy
     0x0039C1D8  sq     $v0,0x0($v1)
     0x0039C254  add.s  $f0,$f0,$f1        the value at +0x138 grows
     0x0039C284  jal    func_0032F718       func_002A7878, the random float
     0x0039C298  jal    func_00303410
     0x0039C2B8  jal    func_0032F7E0       func_002A7940, the angle
     0x0039C2D8  jal    func_00300FE8       func_00281EB8
     0x0039C2E4  jal    func_0032F718
     0x0039C348  jal    func_0032F7E0
     0x0039C36C  jal    func_00300FE8
     0x0039C3A4  the restores of $s0..$s7, $fp, $ra and $f20, $f21

   THE CALLS: 26 in all, to 13 distinct callees - func_00302D30, func_002FA720
   (twice), func_00303410 (five times), func_00302230, func_00302AC8,
   func_00300058, func_00302290, func_003396D8, func_00302548 (twice),
   func_00280B90 (twice), func_0032F718 (twice), func_0032F7E0 (three times),
   func_00300FE8 (three times).

   THE $gp ACCESSES (twelve, which is why the body needs -G8; the floats are
   the -0x374/-0x378/-0x33C/-0x308 ones, the ints the -0x384/-0x350/-0x318
   ones):

     0x0039BF30  lw    $a0,-0x384($gp)   = 0x001AEC6C
     0x0039C014  lwc1  $f0,-0x374($gp)   = 0x001AEC7C
     0x0039C020  lwc1  $f0,-0x378($gp)   = 0x001AEC78
     0x0039C10C  lw    $a0,-0x350($gp)   = 0x001AECA0
     0x0039C18C  lwc1  $f12,-0x374($gp)  = 0x001AEC7C
     0x0039C190  lwc1  $f12,-0x33C($gp)  = 0x001AECB4
     0x0039C1A8  lwc1  $f0,-0x378($gp)   = 0x001AEC78
     0x0039C1F8  lwc1  $f0,-0x374($gp)   = 0x001AEC7C
     0x0039C1FC  lwc1  $f0,-0x33C($gp)   = 0x001AECB4
     0x0039C21C  lwc1  $f0,-0x378($gp)   = 0x001AEC78
     0x0039C28C  lw    $a0,-0x318($gp)   = 0x001AECD8
     0x0039C2C8  lwc1  $f12,-0x308($gp)  = 0x001AECE8

   The two `lq`/`sq` pairs at 0x0039BF28 and 0x0039C1D4 are the 128-bit
   copy of the 16-byte object, and the ones at 0x0039C0FC, 0x0039C130 and
   0x0039C234 are the four-float arrays; they are the 128-bit copy wall the
   compiler profile has (see nonmatching/levels/func_L20_0039BB18.c).

*/
