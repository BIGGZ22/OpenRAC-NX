/* func_L20_002D7730 (level 20, yeedil) - A MAP, NOT AN ATTEMPT.

   Nothing was written for this body: at 19548 bytes (4887 instructions) it
   is far past what a single worker should sink a wave into, and the split
   64-bit address wall (see func_L20_002D6488.c in this directory, the note
   above the "wave 3" section) had to be understood first.  What follows is
   the map, so the next attempt starts from the structure and not from zero.

   THE SHAPE.  277 basic blocks, 238 calls to 62 distinct callees, and NOT
   ONE back edge: the body is a straight-line dispatch, not a loop.  It is
   the level's message handler: func_L20_002DC390 (in L20_002D6488.c) picks
   a variant of it and calls it with (0,1), (0x3F,1) or (0x8D,1), and the
   body itself calls itself four more times.

   THE ARGUMENTS.  $a0 = the message/state id, $a1 = the mode.  In the
   prologue: $s1 = the first argument, $s2 = the second.

   THE PROLOGUE (0x002D7730).
     - frame 0xA0; the GPR saves $s0-$s6 at 0x50,0x58,0x60,0x68,0x70,0x78,
       0x80, $ra at 0x88; $f21 at 0x98 and $f20 at 0x90; the locals at
       0x00-0x50.
     - lui $v0,%hi(D_189E20) ; daddu $s4,$v0,$zero ; addiu $v0,$v0,%lo(D_189E20)
       - the split form: $s4 keeps the %hi half and the plain address is
       re-made at every use ("addiu $s0,$s4,%lo(D_189E20)"), exactly the
       shape the note in func_L20_002D6488.c describes.

   THE ENTRY.
     1. 0x002D776C: the state D[0x229C]; if it is not 0x14, go to .L002D7794.
     2. 0x002D7774: when the state is 0x14, $a0 must be 0x61 and $a1 must be
        2, else the function returns 0 (.L002DC35C).
     3. .L002D7794 (0x002D7794): the mode word D_001A8F00.  If it is 2 then
        $a1 must be 2 (else return 0); if it is 6 the body falls through as
        well; any other value falls through to .L002D77BC.
     4. .L002D77BC (0x002D77BC): the flag D[0x24CC]; when it is set the id
        dispatch is skipped (.L002D7848).  Otherwise the state is read
        again: 2 and 7 lead into the id test, every other state goes on at
        .L002D784C with $t0 = 0.
     5. THE ID DISPATCH (0x002D77E0-0x002D7840), a chain of "beq $s1,$v0":
        0x6, 0x39, 0x43, 0x91, 0x5F, 0x74, 0x78, 0x79, 0x7C, 0x67 and 0x7D
        all reach .L002D7844, the common continuation (which sets $s0 to
        the address of D_00189E20); 0x7F and 0x61 return 0.
     6. A SECOND DISPATCH at 0x002D9380: the ids 0xE and 0x38 reach
        .L002D9398.

   THE JOIN POINTS (the labels many branches fall into).
     .L002DC15C  x67  the tail of most cases: "jal f_002E6090" (the sound
                        helper) with $a0 = the previous result, then
                        .L002DC20C.
     .L002DC20C  x38  the shared tail: when $s6 is 0 and D[0x22B7] is 0,
                        D[0x231C] = 1; then the tests on D[0x348],
                        D[0x1298] (0x13) and D[0x2304], which set
                        D[0x2320] = 1.
     .L002DA7DC  x16, .L002DBCF4 x7, .L002DB378 x5, .L002DBCBC x3.

   THE EXITS.
     .L002DC35C  x8   the "return 0" exit: "daddu $v0,$zero,$zero".
     0x002DC318-0x002DC354  the normal end: the counters D[0x1B8],
                        D[0x1C0], D[0x1BC] and D[0x1C4] are cleared and
                        the function returns 1 ("addiu $v0,$zero,0x1"),
                        then the epilogue at 0x002DC35C-0x002DC38C.

   THE CALLS (238 in all, 62 distinct).
     f_002E6090 x40  the sound/effect helper, the most called.
     f_002B7A48 x20, f_002B7B10 x10, f_002CCCD8 x9, f_00302920 x8,
     f_00337CF8 x8, f_003023C0 x7, f_00302228 x7, f_0032F2D0 x7,
     f_00335558 x7, f_003033C8 x7, f_002C9268 x7, f_002EA8A0 x6,
     f_002C2560 x5, f_002CA6F0 x5, f_002C6340 x5, f_00303358 x4,
     f_00303270 x4, f_002C0BF8 x4, f_002B7B08 x4, f_00338100 x3, ...
     f_002D7730 x4   the body is recursive: (0,0) at 0x002D86C8 (when $a1
                      is 0 and a helper returned 0x51), (0x39,1) at
                      0x002D99B0, (0x94,1) at 0x002DBCF4, (0x16,1) at
                      0x002DBF50.
     f_002DC390 x1   at 0x002D7950: the variant picker, which calls this
                      function back.

   THE DATA.  D_00189E20 (the state block, x23), D_18C750 and D_18C360
   (x12 each), D_00268F20 (x10), D_00268D90 (x8), D_001B3730/D_001B3738,
   D_00189F10, D_0018A210, D_001A8F00 (the mode word), D_00189F80,
   D_002690B0, D_001A9010, D_001A8F20, D_001B9E80.  The level's own data
   needs the D_L20_ names; the boot-resident data keeps the D_ names.

   THE $gp ACCESSES, so the body belongs in a -G8 unit
   (L20_002D7730-g8.c): -0x7C60($gp) = 0x001A7390 (x3), -0x5FFC($gp) =
   0x001A8FF4 (x1), 0x4748($gp) = 0x001B3738 (x1).  The symbols that are
   read both absolutely and through $gp need two names (sda and nosda).

   THE WALL FOR THE NEXT ATTEMPT.  The same split-address form as in
   func_L20_002D6488 (the %hi half in a callee-saved register, the address
   re-made at every use), so the same recipe applies: give every global the
   retail re-materialises a second C name.  Beyond that the body is 4887
   instructions of dispatch, and it wants its own -G8 unit; it is not a
   wave-3-sized job.
*/
