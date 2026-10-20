/* func_L20_002DEBE0 (28208 bytes, 7052 instructions, level 20 only) -- MAPPING,
   not integrated.

   WHAT THIS BODY IS NOT.  The brief for this wave describes this function as
   "a 149-way dispatch through the level's own jump table D_L20_002B27E0 over
   62 bodies" with "a prologue with a 0x1A0 frame, 746 calls to 136 distinct
   callees, 873 float constants, 6 $gp accesses, the epilogue's two stores".
   Those numbers belong to func_L20_002CD928 (34964 bytes, the level's state
   machine), whose mapping is already in nonmatching/levels/func_L20_002CD928.c.
   The evidence: all 149 entries of the jump table D_L20_002B27E0 point
   inside func_L20_002CD928's own body (0x002CD9B0 .. 0x002D6154) and none
   inside this one; and the frame, the call count, the float constants and
   the $gp accesses of THIS body are the ones measured below.

   So this file records the MAPPING of func_L20_002DEBE0: what the function
   is, where its parts are, and what a C attempt would have to reproduce.
   It is the yeedil level's main update function: a long straight-line
   sequence of calls with early exits, driven by the level global
   D_00189E20 (the boot's global, at 0x00189E20, reached here as a 64-bit
   address in $s0 and $s3).

   THE PROLOGUE (0x002DEBE0 .. 0x002DEC8C):

     0x002DEBE0  addiu  $sp,$sp,-0x110        the frame is 0x110
     0x002DEBE4  lui    $v0,%hi(D_189E20)     the 64-bit address of the global
     0x002DEBE8  sd     $s0,0xA0($sp)        the saved GPRs at 0xA0..0xE8:
     0x002DEBF0  sd     $s3,0xB8($sp)        $s0 $s1 $s2 $s3 $s4 $s5 $s6
     0x002DEBF4  addiu  $s0,$v0,%lo(D_189E20)   $s7 $fp $ra (0xA0..0xE8)
     0x002DEBF8  sd     $s1,0xA8($sp)        the saved FPRs f20..f23 at
     0x002DEBFC  daddu  $s3,$v0,$zero        0xF0..0x108 (swc1, descending)
     0x002DEC00  sd     $s2,0xB0($sp)
     0x002DEC04  sd     $s4,0xC0($sp)
     0x002DEC08  sd     $s5,0xC8($sp)
     0x002DEC0C  sd     $s6,0xD0($sp)
     0x002DEC10  sd     $s7,0xD8($sp)
     0x002DEC14  sd     $fp,0xE0($sp)
     0x002DEC18  sd     $ra,0xE8($sp)
     0x002DEC1C  swc1   $f23,0x108($sp)
     0x002DEC20  swc1   $f22,0x100($sp)
     0x002DEC24  swc1   $f21,0xF8($sp)
     0x002DEC28  swc1   $f20,0xF0($sp)

   $s0 is the 32-bit address of the global (`addiu $s0,$v0,%lo(D_189E20)`)
   and $s3 its 64-bit address (`daddu $s3,$v0,$zero`): the two forms of GCC's
   `la`, exactly as the near miss of func_L20_002CD928 records.

   THE FOUR VALUES THE PROLOGUE READS OUT OF THE GLOBAL AND KEEPS ON THE
   STACK (the locals at 0x78..0x84, in the order they are read):

     0x78  D_00189E20[0x2290]   the object the level drives (11 reads later)
     0x7C  D_00189E20[0x229C]   the "other" object (6 reads)
     0x80  D_00189E20[0x2298]   the saved value (5 reads)
     0x84  object[0x43]         the object's state byte

   The prologue then calls func_002C5D00(1) and branches to the exit at
   0x002E5958 when it returns non-zero.

   THE BODY.  440 calls to 83 distinct callees, in one long sequence with
   early exits; the exits are the only branches out of the sequence, and they
   all land in the last 0x1B0 bytes of the function.  The phases (the
   address range, the number of calls and the callees each phase uses):

     0x002DEC00 - 0x002DEFFF   17 calls  the level's own set-up: func_002C5D00,
                                     func_002DE870, func_002DCC80, func_002DC628,
                                     func_002DC4C8, func_002DC418 (the six
                                     "is this level live" tests, each with an
                                     exit), then func_0030AD58, func_00320D18,
                                     func_00320C30, func_002DEA50, func_002DEB10
     0x002DF000 - 0x002DF3FF   18 calls  the first update: func_0032F2D0 (x4),
                                     func_002B7B08, func_002B7A48, func_002DDD18,
                                     func_002D7730, func_002E6090, func_00301ED0,
                                     func_0032F6C0, func_002E5A10, func_002C0C30,
                                     func_002C2560, func_002C6B20
     0x002DF400 - 0x002DF7FF   15 calls  func_002DD800, func_00344460, func_002C6B20,
                                     func_00303358, func_00302408, func_002C6360,
                                     func_002B8DE0, func_0045E410, func_002C6E40,
                                     func_002DDCB0, func_00301F00
     0x002DF800 - 0x002DFBFF    9 calls  func_002E6090, func_00344460, func_002D7730,
                                     func_002B7A48
     0x002DFC00 - 0x002DFFFF   15 calls  func_002D7730, func_002E6090, func_002B7A48,
                                     func_002C6340, func_002E6938, func_003033C8,
                                     func_002BCD30
     0x002E0000 - 0x002E03FF   13 calls  func_003033C8, func_00344460, func_002DE340,
                                     func_002D7730, func_002E6090, func_002B7A48,
                                     func_002E63F8
     0x002E0400 - 0x002E07FF   12 calls  func_002DE5C8, func_00344460, func_002C6B20,
                                     func_002DD800, func_0045E410, func_003023C0,
                                     func_002B7A48, func_002E6090, func_002D7730
     0x002E0800 - 0x002E0BFF   18 calls  func_002D7730, func_002C6360, func_003021D0,
                                     func_002DE758, func_00335558, func_002B7A48,
                                     func_003023C0, func_002E6090, func_002CCCD8
     0x002E0C00 - 0x002E0FFF   16 calls  func_00302390, func_002BE008, func_00326528,
                                     func_00351618, func_00302440, func_00303358,
                                     func_002DD8E8, func_002D7730, func_00344460
     0x002E1000 - 0x002E13FF   17 calls  func_00375180, func_002D7730, func_002B7A48,
                                     func_002E6090, func_0045E410, func_002DE340,
                                     func_00344460, func_0033E7D8, func_002BCD30
     0x002E1400 - 0x002E17FF   14 calls  func_002E6090, func_002D7730, func_002B7A48,
                                     func_002E7608, func_002F3518, func_00302858,
                                     func_00302870, func_00302260, func_002F24B8
     0x002E1800 - 0x002E1BFF   13 calls  func_00302548, func_002D7730, func_00344460,
                                     func_002E6090, func_00331F18, func_003309C0
     0x002E1C00 - 0x002E1FFF   19 calls  the busiest phase: func_00358028, func_002C6B20,
                                     func_00303358, func_00303270, func_003578A0,
                                     func_003022D0, func_00302260, func_00302858,
                                     func_00302870, func_002C0768, func_002E7A28,
                                     func_002E7CC0, func_002BCD30 and five more
     0x002E2000 - 0x002E23FF   15 calls  func_002BCD30, func_002D7730, func_002E6090,
                                     func_002E7CC0, func_003021D0, func_002C6B20,
                                     func_002BFE88, func_00344460, func_00303358
     0x002E2400 - 0x002E27FF   28 calls  func_00303270, func_00303358, func_00302858,
                                     func_00302870, func_002BFE88, func_002C6B20,
                                     func_00344460, func_002D7730, func_002C61E0,
                                     func_002E6090
     0x002E2800 - 0x002E2BFF   14 calls  func_002B8DA0, func_002D7730, func_002E6090,
                                     func_002E7A80, func_00335558, func_002C0C30,
                                     func_002DD800
     0x002E2C00 - 0x002E2FFF   19 calls  func_002C61E0, func_002E7A28, func_002D7730,
                                     func_002E6090, func_002E7170, func_002C6B20,
                                     func_00303358, func_002B7A48
     0x002E3000 - 0x002E33FF   13 calls  func_002E7170, func_002D6E90, func_00344460,
                                     func_002C0C30, func_002D7730, func_00302390
     0x002E3400 - 0x002E37FF   17 calls  func_002E7CC0, func_002BCD30, func_002CCCD8,
                                     func_002BDAE0, func_002B8DA0, func_002E7170,
                                     func_003033C8, func_002B7B08, func_002E6090,
                                     func_003329E8, func_002E6938
     0x002E3800 - 0x002E3BFF   14 calls  func_002B7B08, func_003033C8, func_002E6090,
                                     func_002D7730, func_003022D0, func_00344460,
                                     func_00303358, func_002DE0D0
     0x002E3C00 - 0x002E3FFF   11 calls  func_003329E8, func_00344460, func_002E7A28,
                                     func_003021D0, func_002D7730, func_002C0C30,
                                     func_002E6090, func_002B7A48, func_002DC390
     0x002E4000 - 0x002E43FF   20 calls  func_002E6090, func_0032F718, func_0045E410,
                                     func_00344460, func_002C6B20, func_002D7730,
                                     func_003033C8, func_002E7CC0, func_002E6938
     0x002E4400 - 0x002E47FF   13 calls  func_002BCD30, func_003033C8, func_002E6090,
                                     func_00344460, func_002C6B20, func_00303358,
                                     func_002D7730
     0x002E4800 - 0x002E4BFF    6 calls  func_00344460, func_002C61E0, func_002E6090
     0x002E4C00 - 0x002E4FFF   29 calls  func_002C6B20, func_002BFE88, func_002D7730,
                                     func_00344460, func_00303358, func_00303270,
                                     func_00302858, func_00302870
     0x002E5000 - 0x002E53FF   23 calls  func_00344460, func_00303358, func_002D7730,
                                     func_002E6090, func_0045E410, func_002C6B20,
                                     func_002C6360, func_002B91F0, func_002E7CC0,
                                     func_002BCD30, func_003033C8, func_002E6938
     0x002E5400 - 0x002E57FF   11 calls  func_00344460, func_002C6B20, func_00303358,
                                     func_002D7730, func_002C61E0, func_002E6090,
                                     func_002B7A48
     0x002E5800 - 0x002E5BFF   11 calls  the tail: func_002E6090, func_002D7730,
                                     func_002B7A48, func_00344460, func_002DE340,
                                     func_002B9610, func_002DDCB0 (the last one is
                                     in the epilogue)

   THE SIX EXITS.  Every branch out of the sequence lands in the last 0x1B0
   bytes; the number of branches that reach each:

     0x002E591C   196 branches   the main exit (the level is done)
     0x002E5958   107 branches   the "the other object changed" exit
     0x002E5954    62 branches   the same as 0x002E5958 (one branch further)
     0x002E595C    24 branches   the "the saved value changed" exit
     0x002E58B8     9 branches   the "the camera is too high" exit
     0x002E5960     4 branches   the same as 0x002E595C

   THE EPILOGUE (0x002E595C .. 0x002E5A10):

     0x002E595C  lw    $v0,0x229C($s0)      the other object
     0x002E5960  beql  $a0,$v0,...          if the object changed:
     0x002E5964  lw    $a0,0x22A0($s0)
     0x002E5968  jal   func_002B9610          call func_002B9610(object)
     0x002E5970  lw    $a0,0x22A0($s0)      the state (0x3C = 60, 0x3D = 61)
     0x002E5974  addiu $v0,$zero,0x3C
     0x002E5980  lw    $v1,0x2294($s0)      the level's own state
     0x002E598C  bne   $a0,$v0,...
     0x002E5994  lw    $v0,0x1854($s0)
     0x002E599C  lw    $a0,0x80($sp)       the saved value
     0x002E59A0  jal   func_002DDCB0          if (D[0x1854]) func_002DDCB0(saved)
     0x002E59A8  addiu $v1,$s3,-0x61E0
     0x002E59B0  lw    $v0,0x2298($v1)
     0x002E59B4  bnel  $a0,$v0,...
     0x002E59B8  sw    $zero,0x1C0($v1)     if the value changed, D[0x1C0] = 0
     0x002E59BC  lw    $a0,0x78($sp)       the object
     0x002E59C0  lbu   $v0,0x43($a0)       its state byte
     0x002E59C4  lw    $a0,0x84($sp)       the saved state
     0x002E59C8  bnel  $a0,$v0,...
     0x002E59CC  sw    $zero,0x1C4($v1)     if the state changed, D[0x1C4] = 0
     0x002E59D0  the restores of $s0..$s7, $fp, $ra and f20..f23
     0x002E5A08  jr $ra ; addiu $sp,$sp,0x110

   The two stores at the end are the same "if the value changed, clear the
   flag" shape as the epilogue of func_L20_002CD928.

   THE $gp ACCESSES (seven, so this body belongs in the -G8 unit):

     0x002DED04  lw $v0,-0x603C($gp)      = 0x001A8FB4
     0x002DED1C  lw $v1, 0x3AD4($gp)      = 0x001B2AC4
     0x002E1B28  lw $v1,-0x7600($gp)      = 0x001A79F0
     0x002E2CD0  lw $v1,-0x7600($gp)      = 0x001A79F0
     0x002E5834  sw $zero,-0x65A8($gp)    = 0x001A8A48
     0x002E585C  sw $zero,-0x65A8($gp)    = 0x001A8A48
     0x002E5874  sw $zero,-0x65A8($gp)    = 0x001A8A48

   THE FLOAT CONSTANTS: 369 materialisations (`lui`+`ori`+`mtc1`), 76 distinct
   values, from 0.000833333 (0x3A5A740F) to 999.0f (0x4479C000), among them
   0.016 (0x3C83126F), 0.0333333 (0x3D088889), 0.2 (0x3E4CCCCD, 13 times), 0.3 (0x3E99999A, 12),
   0.5 (0x3F000000), pi/4 (0x3F490FDC, 6), pi/2 (0x3FC90FDB, 3 and 0x3FC90FDC,
   5), 3pi/4 (0x4016CBE5, 4), 5 degrees (0x3DB2B8C4, 5), 10 degrees
   (0x3E2E147B, 5) and -pi/2 (0xBFC90FDC, 5).

   WHAT A C ATTEMPT WOULD HAVE TO REPRODUCE.  The frame (0x110), the ten
   saved GPRs and the four saved FPRs, the 64-bit address of D_00189E20 in
   $s3 and the 32-bit one in $s0, the four values the prologue spills to
   0x78..0x84, the 440 calls in their order, the six exits and the
   epilogue's two conditional stores.  With 7052 instructions and 83 callees
   this is a transcription of a whole level update, not a body a single wave
   can finish; the map above is what the next worker needs to start.

   The $gp accesses also decide the unit: the three small globals are
   reached through $gp, so the body belongs in L20_002DEBE0-g8.c (built with
   -G8) and not in L20_002DEBE0.c.

*/
