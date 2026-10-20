/* func_L20_002CD928 (34964 bytes, 8741 instructions, level 20 only) -- NEAR MISS,
   not integrated.  The attempt belongs in the -G8 unit (the body reaches six
   small-data globals through $gp): the C at the end of this file is written
   for src/levels/20_yeedil/L20_002CD928-g8.c, and the marker in
   src/levels/20_yeedil/L20_002CD928.c is back.

   WAVE 3 (worker 16) added, and this is the summary:

     - WHAT DIFFERS: the frame.  The retail's frame is 0x1A0 (ten saved GPRs,
       five saved FPRs, about forty float spills); the skeleton below has no
       frame at all, because the frame and the register allocation are decided
       by the 62 bodies, not by the dispatch.  tools/check.py says
       "20_yeedil: 92 bytes, expected 34964" and "0 of 1 places match".
     - WHAT IS PROVEN: the dispatch through the level's own jump table is the
       retail's instruction for instruction (lui %hi / sll / addiu %lo / addu /
       lw / jr / nop, after the retail's sltiu and beqz), and the epilogue's
       two bnel tests have the retail's shape.
     - WHAT WAS SOLVED: the "64-bit base".  The retail's `daddu $s6, $v0, $zero`
       is cc1's `move $s6, $v0` (the ee gas assembles `move` as `daddu rd,rs,
       $zero`), so there is no 64-bit value anywhere: the pattern is GCC 2.x's
       split address (the "high" and "low" patterns of config/mips/mips.md),
       and the spelling that makes the compiler produce the retail's shape is
       in the WAVE 3 section below.
     - WHAT WAS SETTLED: the jump table is `jtbl_002B27E0` (the name the
       level's data blob and the level's undefined_syms_auto.txt carry), not
       `D_L20_002B27E0`.
     - WHAT IS STILL MISSING: the 62 bodies (34964 bytes of C), the
       prototypes of the 136 callees (26 of them are already matched; the rest
       are guessed from the call sites), and the arguments of the 746 calls.

   The body map (the 62 bodies, their addresses, sizes, case values, callees,
   float constants, the global's fields they touch and where they branch) is
   in this file: the first map from wave 1 and 2, the extended one from wave 3.

/* func_L20_002CD928 (34964 bytes, 8741 instructions, level 20 only) -- NEAR MISS, not
   integrated.

   What this is: the level's big state machine.  A `switch` on a value read from the
   boot global D_00189E20 (+0x2294) with 149 possible values, 62 distinct bodies, and a
   common tail.  746 calls to 136 distinct functions, 873 float constants.

   State of the attempt: the C below has the right shape (the 149-way dispatch, the
   global reads, the epilogue) and the first body transcribed; the other 61 bodies are
   stubs.  tools/check.py func_L20_002CD928 --diff says

     20_yeedil: 576 bytes, expected 34964
     func_L20_002CD928: 0 of 1 places match (src/levels/20_yeedil/L20_002CD928.c)

   The first difference is the very first instruction: our frame is 0x60, retail's is
   0x1A0, because retail keeps ten GPRs (s0-s7, fp) and five FPRs (f20-f24) live
   across the calls and spills about forty floats to the stack.  The register
   allocation of a 8741-instruction function cannot be reproduced without the whole
   function, so this is not a matter of fixing a line or two.

   Measured, byte for byte, on the parts that are transcribed:
   - the dispatch: our ten instructions have the same opcode sequence as retail's
     (sltiu, beqz, lui, sll, addiu, addu, lw, jr, nop); the registers differ ($v0/$v1
     against $v1/$a0) and retail's branch delay slot holds `sw $v0,0x114($sp)` where
     ours holds `lbu $s5,0x43($s2)`, because retail spills the two saved values to the
     stack in the prologue and we keep them in registers.
   - the first body (cases 0,1,3,4,28,29,30,57,109,111,125, retail 0x002CD9B0): 25 of
     the first 60 instructions are identical (counting a jal to any symbol as equal),
     28 have the same opcode.  The first divergence is retail's `addiu $s2,$zero,0x1`
     (the int variable 1, materialised before the call) against our rematerialisation
     of the constant after the call.

   What the next worker should know (measured with the reconstructed compiler):

   1. THE JUMP TABLE.  The dispatch is GCC's own `casesi` expansion: `sltiu $v1,$a0,
      0x95; beqz $v1,default; lui $v0,%hi(T); sll $v1,$a0,2; addiu $v0,$v0,%lo(T);
      addu $v1,$v1,$v0; lw $a0,0($v1); jr $a0`.  The table T is D_L20_002B27E0, 149
      words of absolute case addresses, in the level's data section.
      A C `switch` cannot be used here: the compiler puts its jump table in the C
      object's .rdata, and tools/build.py loads only `obj(.text.func_X)` of every C
      object and discards the rest, so the table would not be in the image and the
      `lui` would point at a discarded section.  (Worse: check.py would still pass,
      because it reads the symbol's value out of the retail bytes and a compiler-local
      table symbol carries no address constraint.)
      A computed goto through the level's own table gives retail's instructions, but
      GCC then deletes every case body as unreachable: only labels whose address is
      taken (a `&&label` array) survive.  The C below uses both - `goto
      *D_L20_002B27E0[sel]` for the dispatch and a `static void * volatile jt[]` of
      `&&label`s that is never read, so the bodies stay and cost no code.  The volatile
      array lands in the C object's .data, which the build discards, so it is harmless.
      tools/build.py would need a change to place a compiler's .rdata at a fixed
      address before a plain `switch` can be used at all.

   2. THE LAYOUT IS THE SOURCE ORDER OF THE CASES.  With this compiler the bodies of a
      switch are laid out in the order the `case` labels appear in the source, not in
      the order of the case values, and not in a probability order (measured: a switch
      whose cases are written 148,147,...,0 comes out laid out 148,147,...,0, and the
      jump table still lists the cases by value).  Retail's layout therefore gives the
      original source order of the 62 bodies, which is what the table below records.
      A body is shared by several case values where the source wrote them one after
      the other.

   3. THE FILE NEEDS -G8.  Six small-data globals are reached through $gp
      (-0x7C60($gp) = 0x1A7390, -0x7600($gp) = 0x1A79F0, -0x6000($gp) = 0x1A8FF0,
      0xF10($gp) = 0x1AF000, 0x3AD8($gp) = 0x1B2AC8, 0x4748($gp) = 0x1B3738), so the
      body belongs in a `-g8.c` unit (L20_002CD928-g8.c) and not in
      L20_002CD928.c, which builds with -G0.

   4. THE 64-BIT GLOBAL.  Retail keeps the address of D_00189E20 as a 64-bit value:
      `lui $v0,%hi(D); daddu $s6,$v0,$zero` in the prologue and
      `addiu $s0,$s6,%lo(D)` (=-0x61E0) wherever the 32-bit address is wanted.  That
      is GCC's 64-bit `la`: it appears when the pointer's value is needed in 64 bits
      (measured: a pointer cast to unsigned long long gives `lui; daddu; addiu`, a
      pointer only used through 32-bit offsets gives `lui; addiu`).  The source passes
      64-bit arguments (the retail's `daddu $a1,$sp,$zero`, `daddu $a2,$s0,$zero`), so
      the prototypes of the callees matter to the register allocation.

   5. FRAME.  0x1A0, the saved GPRs at 0x120-0x168 (s0, s1, ..., s7, fp, ra) and the
      saved FPRs at 0x170-0x190 (f20, f21, f22, f23, f24, as 4-byte swc1 in descending
      order), the locals at 0x00-0x11F (about forty float spills, the two ints the
      epilogue compares at 0x114 and 0x118).

   6. THE EPILOGUE.  Both tests are "if the value changed, clear a flag":
        if (saved != D_00189E20[0x2298]) D_00189E20[0x1C0] = 0;
        if (state != obj[0x43])         D_00189E20[0x1C4] = 0;
      with `bnel` + the store in the delay slot, as the compiler emits it.

   The structure, from the retail (the jump table at 0x002B27E0 and the body map):

     0x002CD928  prologue: frame 0x1A0, the 64-bit address of D_00189E20 in s6,
                 obj = D_00189E20[0x2290] in fp, sel = D_00189E20[0x2294],
                 saved = D_00189E20[0x2298] and state = obj[0x43] spilled to
                 0x114($sp) and 0x118($sp)
     0x002CD980  the dispatch (see 1.)
     0x002CD9B0  the 62 bodies, in the source order (see the table)
     0x002D6154  the default body: one instruction, `addiu $v1,$s6,%lo(D)`
     0x002D6158  the epilogue (see 6.)
     0x002D6178  the restores and `jr $ra`

   The 62 bodies, the cases each one answers and its size in the retail:


    0x002CD9B0   1392 bytes  cases 0, 1, 3, 4, 28, 29, 30, 57, 109, 111, 125
    0x002CDF20    328 bytes  cases 124
    0x002CE068   3412 bytes  cases 50, 51, 52
    0x002CEDBC    120 bytes  cases 115
    0x002CEE34     68 bytes  cases 114
    0x002CEE78    444 bytes  cases 103, 127
    0x002CF034    712 bytes  cases 53, 54
    0x002CF2FC     48 bytes  cases 133
    0x002CF32C    380 bytes  cases 134
    0x002CF4A8    368 bytes  cases 47
    0x002CF618    168 bytes  cases 138, 139
    0x002CF6C0    256 bytes  cases 35
    0x002CF7C0    804 bytes  cases 36
    0x002CFAE4    236 bytes  cases 37
    0x002CFBD0   2032 bytes  cases 43
    0x002D03C0   1556 bytes  cases 20
    0x002D09D4   1100 bytes  cases 21, 131
    0x002D0E20   1408 bytes  cases 19
    0x002D13A0    336 bytes  cases 119
    0x002D14F0   1244 bytes  cases 31
    0x002D19CC    236 bytes  cases 34
    0x002D1AB8   1816 bytes  cases 2, 112
    0x002D21D0    840 bytes  cases 123
    0x002D2518    536 bytes  cases 46
    0x002D2730    868 bytes  cases 48
    0x002D2A94   3904 bytes  cases 39, 40, 41, 42, 62
    0x002D39D4    180 bytes  cases 23, 24
    0x002D3A88    172 bytes  cases 70, 71
    0x002D3B34    320 bytes  cases 25, 26
    0x002D3C74     24 bytes  cases 118
    0x002D3C8C    292 bytes  cases 6, 44
    0x002D3DB0    184 bytes  cases 116
    0x002D3E68    924 bytes  cases 130
    0x002D4204    596 bytes  cases 8
    0x002D4458   2028 bytes  cases 7, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 27, 56, 69, 72, 91, 102, 110, 129, 137, 146
    0x002D4C44    180 bytes  cases 22
    0x002D4CF8     36 bytes  cases 140
    0x002D4D1C    592 bytes  cases 98
    0x002D4F6C    164 bytes  cases 99
    0x002D5010     20 bytes  cases 60, 100
    0x002D5024    108 bytes  cases 101
    0x002D5090      8 bytes  cases 121
    0x002D5098     36 bytes  cases 120
    0x002D50BC     36 bytes  cases 122
    0x002D50E0    152 bytes  cases 63, 67
    0x002D5178    688 bytes  cases 77
    0x002D5428    148 bytes  cases 75
    0x002D54BC     84 bytes  cases 78
    0x002D5510     24 bytes  cases 65
    0x002D5528     68 bytes  cases 68
    0x002D556C    688 bytes  cases 64, 76
    0x002D581C    108 bytes  cases 66
    0x002D5888    364 bytes  cases 73, 74
    0x002D59F4    188 bytes  cases 141
    0x002D5AB0     84 bytes  cases 145
    0x002D5B04    232 bytes  cases 147
    0x002D5BEC    108 bytes  cases 148
    0x002D5C58    152 bytes  cases 143
    0x002D5CF0    800 bytes  cases 142
    0x002D6010    192 bytes  cases 144
    0x002D60D0    120 bytes  cases 55
    0x002D6148     12 bytes  cases 96, 97
    0x002D6154      0 bytes  cases 5, 32, 33, 38, 45, 49, 58, 59, 61, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 92, 93, 94, 95, 104, 105, 106, 107, 108, 113, 117, 126, 128, 132, 135, 136

     The 36 case values that are not in the table above (5, 32, 33, 38, 45, 49, 58,
     59, 61, 79-90, 92-95, 104-108, 113, 117, 126, 128, 132, 135, 136) have an empty
     body: they fall into the default body at 0x002D6154.

   The C below.  It compiles, it is the right shape, and it is the start of the
   transcription; the 61 stub bodies are where the work is.  The prototypes of the
   callees are guesses where the repo has none (26 of the 136 callees are already
   matched and their prototypes are in src/, the rest are guessed from the call sites).

*/

/* =====================================================================================
   WAVE 2 (worker 17 of the level-20 wave) -- the body map and the jump table.

   The earlier wave (the note above) recovered the shape of the dispatch and the 62
   bodies.  This wave measured the rest: the complete map of the bodies (their
   addresses, sizes, case values, callees and float constants), the source-level
   structure, and -- the important one -- HOW THE LEVEL'S OWN JUMP TABLE CAN BE
   REACHED FROM C.

   1. THE JUMP TABLE IS REACHABLE FROM C.  The earlier wave wrote that "the linker
      cannot place the table" and that a C `switch` puts its own table in the
      object's .rdata, which tools/build.py discards.  That is true for a table the
      COMPILER generates, and it is why func_00118CC8 (nonmatching/core) can never
      land ("needs .rodata at its place").  It is NOT true here: this level's table
      is already in the level's data, at 0x002B27E0, and the build places the level's
      data.  So the C can name it:

          extern void *D_L20_002B27E0[149];
          ...
          goto *D_L20_002B27E0[sel];

      and tools/build.py resolves it: Placement.derive() reads the symbol's value out
      of the retail bytes at the function's place (the retail's `lui %hi(jtbl_002B27E0)`
      / `addiu %lo(jtbl_002B27E0)` pair gives 0x002B27E0), Placement.expected() accepts
      it (a `D_LNN_XXXXXXXX` name is at its own address in its own level), and
      undefined_symbols() writes `D_L20_002B27E0 = 0x002b27e0;` into the linker script.
      Measured with the pinned build.py on the C below:

          the object has R_MIPS_HI16 at +0x20 and R_MIPS_LO16 at +0x28 against
          D_L20_002B27E0, the value derived from the retail bytes is 0x002B27E0 and
          expected('D_L20_002B27E0') is 0x002b27e0 -- the same.

      The table's entries are the absolute addresses of the 62 bodies, so the computed
      goto lands on the right place as soon as the bodies sit at the retail addresses
      (which they do when the function matches).

   2. THE DISPATCH MATCHES THE RETAIL INSTRUCTION FOR INSTRUCTION.  With the
      computed goto, cc1 emits exactly the retail's ten instructions:

        ours                                 retail
        sltiu $v0, $v1, 0x95                 sltiu $v1, $a0, 0x95
        beqz  $v0, <default>                beqz  $v1, .L002D6154
        lui   $v0, %hi(D_L20_002B27E0)      lui   $v0, %hi(jtbl_002B27E0)
        sll   $v1, $v1, 2                   sll   $v1, $a0, 2
        addiu $v0, $v0, %lo(D_L20_002B27E0) addiu $v0, $v0, %lo(jtbl_002B27E0)
        addu  $v1, $v1, $v0                 addu  $v1, $v1, $v0
        lw    $a0, 0($v1)                   lw    $a0, 0x0($v1)
        jr    $a0                           jr    $a0
        nop                                 nop

      Only the register NAMES differ ($v1 against $a0 for the switch value), and that
      is the register allocator's business: the retail's switch value lives in $a0
      because the huge body needs the registers.  A C `switch` produces the same
      instruction sequence but with the table in .rdata, which the build discards.

      The `jt` array of label addresses below is what keeps the 62 bodies alive: GCC
      deletes a label whose address is never taken, and `&&label` is the only way to
      take one.  The array itself lands in the object's .data, which the build
      discards, so it costs no code and no placed bytes.

   3. `sltu` IN cc1's OUTPUT IS `sltiu` IN THE OBJECT.  cc1 writes "sltu $2,$4,149"
      for the range check, but the retail shows `sltiu $v1,$a0,0x95`.  The two agree:
      in this gas (gas/config/tc-mips.c, case M_SLTU_I) `sltu` with an immediate is a
      MACRO that expands to `sltiu` when the immediate fits in a signed 16-bit half.
      The retail has no `sltu` with an immediate anywhere (measured over the whole of
      level 20), and cc1 never writes `sltiu` at all.  So this difference is only in
      the TEXT: compare the objects, not the .s files.  (The same holds for the
      `bltu`/`bgeu` pseudos, which expand to `sltiu` + a branch.)

   4. THE SOURCE-LEVEL STRUCTURE.  The 62 bodies are laid out in the order of their
      case labels, so the retail's layout is the original source order:

        switch (sel) {
        case 0: case 1: case 3: case 4: case 28: case 29: case 30: case 57:
        case 109: case 111: case 125:      <body 0>   break;
        case 124:                          <body 1>   break;
        case 50: case 51: case 52:         <body 2>   break;
        ...                                  ...        ...
        case 96: case 97:                  <body 61>  break;
        }
        <the tail>

      - The 36 case values that have no label (5, 32, 33, 38, 45, 49, 58, 59, 61,
        79-90, 92-95, 104-108, 113, 117, 126, 128, 132, 135, 136) all point at
        0x002D6154, which is the first instruction of the TAIL: the switch has no
        `default:` label and the tail follows it, so "not in the table" and "break"
        land in the same place.  The tail is `addiu $v1,$s6,%lo(D_189E20)`.
      - The epilogue at 0x002D6158 is reached by 45 branches: nearly every body ends
        with `b .L002D6158`, and $v1 (the global's base) is materialised in the
        branch's delay slot (`addiu $v1,$s6,%lo(D_189E20)`), so the epilogue can use
        $v1 without a reload.
      - Two bodies end in a SHARED HUB instead:
          0x002D60C0 (inside body 59)  `jal func_L00_002D34C0; b .L002D6158`  --
              14 branches reach it (bodies 0, 16, 19, 20, 21, 23, 37, 38 and more);
          0x002D5FE4 (inside body 58)  `jal func_L00_002D34C0; jal func_L00_002C7438;
              b .L002D6158` -- 7 branches reach it.
        In C these are the `break`s of a shared block at the end of the switch.
      - Bodies also branch into each other (body 4 into body 6, body 10 into 11,
        body 15 into 31, body 16 into 17, body 26 into 27, body 28 into 52, body 29
        into 56, body 33 into 34), which is how the source's `goto`s look.

   5. THE PROLOGUE AND THE EPILOGUE, as the retail has them:

        0x002CD928  addiu $sp,$sp,-0x1A0 ; lui $v0,%hi(D_189E20) ;
                    sd $s6,0x150($sp) ; sd $s0,0x120($sp) ;
                    daddu $s6,$v0,$zero ; sd $s1,0x128($sp) ;
                    addiu $v0,$v0,%lo(D_189E20) ; sd $s2,0x130($sp) ; ... ;
                    swc1 $f24,0x190($sp) ; ... ; swc1 $f20,0x170($sp) ;
                    lw $a0,0x2294($v0)   (sel) ;
                    lw $fp,0x2290($v0)   (obj) ;
                    lw $v0,0x2298($v0)   (saved) ;
                    sltiu $v1,$a0,0x95 ; sw $v0,0x114($sp) ; lbu $v0,0x43($fp) ;
                    beqz $v1,.L002D6154 ; sw $v0,0x118($sp) (delay slot) ;
                    lui $v0,%hi(jtbl_002B27E0) ; ... ; jr $a0 ; nop
        0x002D6154  addiu $v1,$s6,%lo(D_189E20)        (the tail)
        0x002D6158  lw $a0,0x114($sp) ; lw $v0,0x2298($v1) ;
                    bnel $a0,$v0,.L002D6168 ; sw $zero,0x1C0($v1) ;
                    lbu $v0,0x43($fp) ; lw $a0,0x118($sp) ;
                    bnel $a0,$v0,.L002D6178 ; sw $zero,0x1C4($v1) ;
        0x002D6178  ld $s0,0x120($sp) ... ld $ra,0x168($sp) ;
                    lwc1 $f24,0x190($sp) ... lwc1 $f20,0x170($sp) ;
                    jr $ra ; addiu $sp,$sp,0x1A0 ; nop

      The frame is 0x1A0: the saved GPRs at 0x120-0x168 (s0, s1, ..., s7, fp, ra),
      the saved FPRs at 0x170-0x190 (f20, f21, f22, f23, f24, as 4-byte swc1 in
      descending order) and about forty float spills at 0x00-0x11F.  The two ints the
      epilogue compares live at 0x114($sp) and 0x118($sp): the retail SPILLS `saved`
      and `state` in the prologue and reloads them in the epilogue, because they are
      live across all the calls of the bodies.

      $s6 holds the address of D_00189E20 as a SPLIT 64-bit constant: $v0 holds the
      %hi (0x00190000) and `daddu $s6,$v0,$zero` zero-extends it, and every use adds
      the %lo with a 32-bit `addiu $rd,$s6,%lo(D_189E20)` (380 such uses).  The C
      spelling for that is NOT known yet: a pointer cast to unsigned long long gives
      `lui; addiu; dsll; dsrl` and a literal gives `dli`, neither of which is the
      retail's split form.  The next worker should try to find it.

   6. THE FILE NEEDS -G8.  Six small-data globals are reached through $gp (all six
      are loads): -0x7C60 = 0x001A7390, -0x7600 = 0x001A79F0 (both boot resident),
      -0x6000 = 0x001A8FF0, 0xF10 = 0x001AF000, 0x3AD8 = 0x001B2AC8, 0x4748 =
      0x001B3738 (the last four are level 20's own data).  So the body belongs in
      L20_002CD928-g8.c, with the four level globals declared
      `extern int D_L20_001A8FF0 __attribute__((sda));` and the boot ones
      `extern int D_001A7390 __attribute__((sda));`.

   7. THE MEASUREMENT.  The C below is the skeleton: the prologue, the dispatch and
      the epilogue, with the 62 bodies as stubs.  tools/check.py says

          20_yeedil: 92 bytes, expected 34964
          func_L20_002CD928: 0 of 1 places match (src/levels/20_yeedil/L20_002CD928.c)

      The first difference is the FIRST instruction: the retail's frame is 0x1A0 and
      it saves ten GPRs and five FPRs, the skeleton has no frame at all.  That is not
      a detail of the prologue: the frame and the register allocation are decided by
      the bodies (the 40 float spills, the ten live GPRs and the five live FPRs), so
      the prologue cannot be matched before the bodies are written.  What the
      skeleton does prove is the dispatch (point 2) and the epilogue's shape: the
      skeleton's epilogue is `lw $v0,0x2298($a1) ; bnel $a3,$v0,... ; sw $zero,
      0x1C0($a1) ; lbu $v0,0x43($a2) ; bnel $t0,$v0,... ; sw $zero,0x1C4($a1)`,
      which is the retail's shape (the bnel with the store in its delay slot) with
      the spilled values in registers instead of on the stack.

   8. THE COMPLETE BODY MAP.  63 bodies in the layout order (= the source order of
      the case labels); body 62 is the default body (the tail and the epilogue, the
      36 case values with no label).  The `calls` are in the order of the retail and
      the `floats` are the constants the body materialises with `lui`/`ori`/`mtc1`.

   THE COMPLETE BODY MAP:

  body  0  0x002cd9b0   1392 bytes  cases 0 1 3 4 28 29 30 57 109 111 125
      calls  : func_L00_002AA478, func_L00_002B54F8, func_L00_002B54F8, func_L00_002B9F00, func_L00_002B39F0, func_00283098, func_L00_002B3EF8, func_L00_002CAF20, func_L00_002B3EF8, func_00282C48, func_L00_002D2168, func_00282C48, func_L00_002D2168, func_L00_002A9E30, func_L00_002B41B8, func_00282C88, func_L00_002B94D8, func_L00_002D34C0, func_L20_002C7588
      floats : 0x3F333333 (0.7), 0x3D088889 (0.0333333), 0x406CCCCD (3.7), 0x3EB33333 (0.35), 0x403A2B22 (2.90888), 0x3E4CCCCD (0.2), 0x3B03126F (0.002), 0x3D8F5C29 (0.07), 0x3DEE4BB0 (0.116355), 0x3BBEA2F4 (0.00581777), 0x4190BF49 (18.0934), 0x3F0CCCCD (0.55), 0x3BF5C290 (0.0075), 0x3F8CCCCD (1.1), 0x39F79499 (0.000472222), 0x3BA11BFE (0.00491667), 0x3C75C290 (0.015)
      what   : the common camera/FOV setup: func_L00_002AA478(15,30), func_L00_002B54F8(0.7,0) or (0.5,1/30) on the g[0x1492] test, func_L00_002B9F00(3.7,0,0.35), func_L00_002B39F0(0,1); then the sub-blocks on g[0x2294] (== 1, == 29, == 3, == 4), the moby update func_L00_002B3EF8, the g[0x1A4] pair func_00282C48/func_L00_002D2168, func_L00_002A9E30 and func_L00_002D34C0; ends in the shared hub 0x002d60c0
  body  1  0x002cdf20    328 bytes  cases 124
      calls  : func_0029EFD0, func_002A0F48, func_002E5CF8, func_00283CF0, func_002A0F48, func_00282E20, func_L00_002EE4D8, func_L00_002C9228
      floats : 0x3F6B851F (0.92)
      what   : a one-shot setup (func_0029EFD0(0x52A), a sound, a 0.92 scale on the camera); ends at the epilogue
  body  2  0x002ce068   3412 bytes  cases 50 51 52
      calls  : func_L00_002B9BB0, func_L00_002D6900, func_002E59F8, func_00282978, func_L00_002AA938, func_L00_002AA938, func_L00_002AA938, func_L00_002AABE0, func_002FD1B8, func_002AA898, func_00283BE0, func_L00_002B39F0, func_L00_002B3EF8, func_00282948, func_00282948, func_0029FCC0, func_00283098, func_00282CC0, func_00282F20, func_00282CC0, func_002BF728, func_002A77E0, func_002A7820, func_00282978, func_0029FCC0, func_00283098, func_00282CC0, func_002A7878, func_002A7878, func_002A7878, func_L00_00323F28, func_00283CF0, func_002A7878, func_002BF728, func_L00_002B41B8, func_L00_002B41B8, func_00283B98, func_002AF000, func_L00_002A9F30, func_002AF000, func_00282DF0, func_002AA2E8, func_002AA2E8, func_00282F20, func_L20_002C7588, func_00283180, func_002AA898, func_L00_002AB0C0, func_L00_002AB0D8, func_L00_002AB0F0
      floats : 0x3ECCCCCD (0.4), 0xBE99999A (-0.3), 0x3C0EFA36 (0.00872665), 0x3C3EA2F4 (0.0116355), 0x3E4CCCCD (0.2), 0x3CCCCCCD (0.025), 0x3EB33333 (0.35), 0x3FAE40F2 (1.36136), 0x44BB8000 (1500), 0xBFAE40F2 (-1.36136), 0x3C75C28F (0.015), 0x3BE56042 (0.007), 0x3DA3D70A (0.08), 0x3DB2B8C4 (0.0872665), 0x3DCCCCCE (0.1), 0x3DEEEEF0 (0.116667), 0x3D3F258D (0.0466667), 0x3D4CCCCE (0.05), 0x3E19999A (0.15), 0x3CCCCCCE (0.025), 0x3D088889 (0.0333333), 0x46C4E000 (25200), 0x3CF5C28F (0.03), 0xBE19999A (-0.15), 0xBC4CCCCE (-0.0125), 0x422B6DB6 (42.8571), 0x45C4E000 (6300), 0x4613A800 (9450), 0x3B6CA864 (0.00361111), 0x3B11A2B4 (0.00222222), 0x3AFEDCBB (0.00194444), 0x3A91A2B4 (0.00111111), 0x3D99999A (0.075), 0x3E2E147B (0.17), 0x3CA6CE95 (0.0203622), 0x3C1374BC (0.009), 0x3E6147AE (0.22), 0x3D23D70A (0.04), 0x3CA3D70A (0.02), 0x3FB33333 (1.4), 0x3F5F66F4 (0.872665), 0xBF5F66F4 (-0.872665), 0x3F8CCCCD (1.1)
      what   : the big moby/level update: 49 calls, a pi/1.36136 trigonometry block, a loop over the objects at g[0xA38], the g[0x1B0] counter, the g[0xC38] vector maths and the func_L00_002AB0C0/D8/F0 trio; ends at the tail
  body  3  0x002cedbc    120 bytes  cases 115
      calls  : func_L00_002AA938, func_00282E20, func_L00_002EE4D8, func_L20_002C7588
      floats : 0x3F6B851F (0.92)
      what   : a short body: func_L00_002AA938, func_00282E20, the 0.92 scale and func_L20_002C7588; ends at the epilogue
  body  4  0x002cee34     68 bytes  cases 114
      calls  : func_L00_002AA478, func_00282E20, func_L00_002EE4D8
      floats : 0x3F6B851F (0.92)
      what   : the shortest body: func_L00_002AA478(15,30), func_00282E20 and the 0.92 scale; jumps into body 6 (0x002cf2e4)
  body  5  0x002cee78    444 bytes  cases 103 127
      calls  : func_00282DF0, func_002AA2E8, func_00282F20, func_00282948, func_0029FCC0, func_00283098, func_00282CC0, func_00282F20, func_00282CC0, func_002BF728, func_002A7820, func_002A7820, func_002AA2E8
      floats : 0x3A91A2B4 (0.00111111), 0x3ECCCCCD (0.4), 0x3CCCCCCE (0.025), 0x3D088889 (0.0333333), 0x46C4E000 (25200), 0xBCCCCCCE (-0.025), 0x39DA740E (0.000416667)
      what   : the g[0xA3C] timer: 0.00111111 steps, the 25200 constant, func_002A7820(5) and (0x14); ends at the tail
  body  6  0x002cf034    712 bytes  cases 53 54
      calls  : func_L00_002AA478, func_L00_002AA750, func_L00_002AA570, func_L00_002AA938, func_L00_002B39F0, func_L00_002B3EF8, func_00282948, func_00282C48, func_L00_002B41B8, func_L00_002A9F30, func_00283180, func_00283198, func_00282C88, func_L00_002AA938, func_L00_002B94D8, func_L20_002C7588, func_L00_002C9228
      floats : 0x3CCCCCCE (0.025), 0x3C888889 (0.0166667), 0x3F19999A (0.6), 0x3D4CCCCE (0.05), 0x3BE56042 (0.007), 0x3DA3D70A (0.08), 0x3DB2B8C4 (0.0872665), 0x3F490FDC (0.785398), 0x3AB60B61 (0.00138889), 0x3A91A2B4 (0.00111111), 0x3A98EAD6 (0.00116667)
      what   : the moby spawn: func_L00_002AA478/750/570, the 0.6/0.05/0.007/0.08/0.0872665 block, the pi/4 (0.785398) turn and the g[0x98] angle maths; ends at the epilogue
  body  7  0x002cf2fc     48 bytes  cases 133
      calls  : (none)
      floats : (none)
      what   : an empty body (48 bytes of frame bookkeeping only); ends at the epilogue
  body  8  0x002cf32c    380 bytes  cases 134
      calls  : func_00282D30, func_002AA3A8, func_00282D30, func_00282CC0, func_00282F20, func_L00_002B8F00, func_00282F20, func_L00_002D3B58, func_00282C88, func_00283098, func_L00_002B3EF8
      floats : 0x3E4CCCCD (0.2), 0x3E99999A (0.3), 0x3CF5C28F (0.03), 0x3BE56042 (0.007), 0x3ECCCCCD (0.4), 0x3CCCCCCD (0.025), 0x3E5DDDDF (0.216667), 0x3CA3D70A (0.02), 0x3D0EFA37 (0.0349066)
      what   : the g[0x24B8]/g[0x1FB4] damage block: 0.2/0.3/0.03/0.007, the 0.0349066 (2 degrees) and func_L00_002B3EF8; ends at the epilogue
  body  9  0x002cf4a8    368 bytes  cases 47
      calls  : func_L00_002B54F8, func_L00_002B54F8, func_L00_002B9F00, func_L00_002B39F0, func_L00_002B41B8, func_00282C88, func_L00_002B94D8, func_L00_002D34C0, func_L00_002B4308, func_L00_002D34C0, func_L20_002B7A48, func_L00_002D2168
      floats : 0x3F333333 (0.7), 0x3D088889 (0.0333333), 0x3EB33333 (0.35), 0x406CCCCD (3.7), 0x3BA11BFE (0.00491667), 0x3BE38E39 (0.00694444), 0x3C75C290 (0.015)
      what   : the FOV block (0.7, 3.7, 0.35) plus func_L00_002B41B8, func_L00_002B94D8, func_L00_002D34C0 and func_L20_002B7A48; ends at the epilogue
  body 10  0x002cf618    168 bytes  cases 138 139
      calls  : func_L00_002B9F00, func_L00_002B41B8, func_00282C88, func_L00_002B94D8, func_L00_002D34C0, func_L00_002B4308, func_L00_002D34C0
      floats : 0x3EB33333 (0.35), 0x406CCCCD (3.7), 0x3BA11BFE (0.00491667), 0x3BC83FB8 (0.00611111), 0x3C75C290 (0.015)
      what   : the same FOV block as body 9 without the 0.7 part; ends at the epilogue
  body 11  0x002cf6c0    256 bytes  cases 35
      calls  : func_L00_002B9F00, func_L00_002B41B8, func_00282C88, func_L00_002B94D8, func_L00_002D34C0, func_L00_002B4308, func_L00_002D34C0, func_00283248, func_L00_002B3EF8
      floats : 0x3EB33333 (0.35), 0x406CCCCD (3.7), 0x3BA11BFE (0.00491667), 0x3BC83FB8 (0.00611111), 0x3C75C290 (0.015), 0x3CA3D70A (0.02), 0x3E19999A (0.15), 0x3DD67752 (0.10472)
      what   : the FOV block plus the g[0x10]/g[0x14] angle maths (0.02, 0.15, 0.10472 = pi/30); ends at the epilogue
  body 12  0x002cf7c0    804 bytes  cases 36
      calls  : func_00282E48, func_002AA2E8, func_00283248, func_00283C80, func_00283B98, func_L00_002B3EF8, func_002AA2E8, func_002A88A0, func_002AA800, func_00282E80, func_00283248, func_00283B98, func_002AA800, func_00283B98, func_00283180, func_00283B98, func_00283198, func_00282CC0, func_00282CF0, func_00282F20
      floats : 0x3EE66667 (0.45), 0x3CB60B61 (0.0222222), 0x3FC90FDB (1.5708), 0x40490FDB (3.14159), 0x3CF5C28F (0.03), 0x3E19999A (0.15), 0x3DFA35E0 (0.122173), 0x3C3F258C (0.0116667), 0x3FF33333 (1.9), 0xBBBF258C (-0.00583333), 0x3D831007 (0.0639954), 0x3FC90FDC (1.5708), 0x3DDC6C69 (0.107629), 0xBFC90FDC (-1.5708), 0xBF11EB85 (-0.57), 0x3C03FB73 (0.00805556)
      what   : the g[0xAA0] aim/rotation block: pi/2 and pi constants, the 1.9/-0.00583333 and -1.5708/-0.57 clamps; ends at the epilogue
  body 13  0x002cfae4    236 bytes  cases 37
      calls  : func_002AA2E8, func_00282D30, func_00282E48, func_00282CC0, func_00282CF0, func_00282F20, func_00282CC0, func_00282CF0
      floats : 0x3E2AAAAB (0.166667), 0x3F666666 (0.9), 0x3B888889 (0.00416667)
      what   : a short lerp block (0.166667, 0.9, 0.00416667) over the g[0x9CC] vector; ends at the epilogue
  body 14  0x002cfbd0   2032 bytes  cases 43
      calls  : func_L00_002D2168, func_00283C80, func_00283B98, func_002AA898, func_L00_002B3EF8, func_00282CF0, func_00282F20, func_00282C88, func_00283410, func_00283698, func_002830C0, func_00282C10, func_00283248, func_00283248, func_002AA898, func_002AA898, func_L00_002B39F0, func_00283180, func_00283198, func_00282CC0, func_00282E80, func_L00_002D34C0, func_L00_002D3B00, func_00282CF0, func_L00_002D34C0, func_00282DF0, func_00282D30, func_00282F20, func_00282CC0, func_00282DF0, func_002AA2E8, func_00282F20, func_00283B98, func_00283180, func_00283198, func_00282CC0, func_00282CC0, func_00282CF0, func_00282F20, func_00282CC0, func_00282CF0, func_002AA3A8, func_00282DF0, func_00282F20, func_00282CC0, func_00282E20, func_002AA3A8, func_00283180, func_00283198, func_00282CC0, func_00282C10, func_L00_002B5590, func_L00_002D2168, func_L00_002D2168, func_L00_002A9E30
      floats : 0x3E99999A (0.3), 0x40490FDB (3.14159), 0x3CA3D70A (0.02), 0x3DA0D97D (0.0785398), 0x3E6B851F (0.23), 0x3D23D70A (0.04), 0x3E4CCCCD (0.2), 0x3E23D409 (0.159989), 0x3B656042 (0.0035), 0x3DAAAAAB (0.0833333), 0x3FC8F5C3 (1.57), 0x3C360B61 (0.0111111), 0x3C81B4E8 (0.0158333), 0x3983126F (0.00025), 0x3B888889 (0.00416667), 0xBFC90FDC (-1.5708), 0x3E19999A (0.15), 0x3E6EEEF0 (0.233333), 0x3D408312 (0.047)
      what   : the largest trigonometry body: pi, 0.0785398 (pi/40), the g[0xAE4]/g[0xAEC] vector maths, the 0.0035/0.0833333 timing and func_L00_002B5590; ends at the tail
  body 15  0x002d03c0   1556 bytes  cases 20
      calls  : func_00282E80, func_L00_002B41B8, func_00283180, func_00283198, func_L00_002B41B8, func_L00_002D3BD8, func_00282CF0, func_L00_002D3A58, func_L00_002B41B8, func_00282F20, func_L00_002D3BD8, func_00282CC0, func_L00_002B41B8, func_L00_002D3BD8, func_L00_002B3EF8, func_L00_002B75A0, func_L00_002D3778, func_00282F20, func_002A8C00, func_00277F48, func_L00_002B3998, func_L00_002D6900, func_L00_002D3B00, func_002AA2E8, func_L00_002D34C0, func_L00_002D3B00, func_L00_002D3B00
      floats : 0x3D99999A (0.075), 0x3A91A2B4 (0.00111111), 0x3A11A2B4 (0.000555556), 0x3BE38E39 (0.00694444), 0x3B888889 (0.00416667), 0x3D4CCCCD (0.05), 0x3E4CCCCD (0.2), 0x3E90777C (0.282162), 0x3F860A93 (1.0472), 0x3E0369D0 (0.128333), 0x3D8369D0 (0.0641667), 0x3FE66666 (1.8), 0x3DB7FA89 (0.0898333), 0x45AFDF66 (5627.92), 0x3ECCCCCD (0.4), 0x41CD999A (25.7), 0x3DCCCCCD (0.1), 0x3D2AAAAB (0.0416667), 0x3D13E93F (0.0361111), 0xBF1DDDDE (-0.616667), 0xBF955556 (-1.16667)
      what   : the g[0x80]/g[0x98] timing body: 0.075/0.00111111/0.000555556 steps, the 1.0472 (pi/3) and 0.282162 constants; ends at the tail
  body 16  0x002d09d4   1100 bytes  cases 21 131
      calls  : func_L00_002B9B38, func_L00_002B9B38, func_00283BE0, func_002AA738, func_L00_002B3EF8, func_L00_002B75A0, func_L00_0041E958, func_L00_002B3998, func_00283180, func_00283198, func_L00_002BA508, func_L00_002B3998, func_00283B98, func_00283180, func_00283198, func_L00_002B41B8, func_L00_002B4208
      floats : 0x408BA059 (4.36332), 0x3D6978D5 (0.057), 0x3E4CCCCD (0.2), 0x3E90777C (0.282162), 0x3F060A93 (0.523599), 0x3F490FDC (0.785398), 0x3E32B8C4 (0.174533), 0x3EB2B8C4 (0.349066), 0x3D088889 (0.0333333), 0x3CB17E4B (0.0216667), 0x40490FDB (3.14159), 0x3D2AAAAB (0.0416667), 0x3D4CCCCE (0.05), 0x3C286420 (0.0102778), 0x3BFEDCBB (0.00777778), 0x47C34F80 (99999), 0x3BE38E39 (0.00694444), 0x3C75C290 (0.015)
      what   : the 4.36332 (250 degrees) body: 0.523599/0.785398/0.174533/0.349066 angle steps, the 99999 sentinel and func_L00_0041E958; ends at the tail
  body 17  0x002d0e20   1408 bytes  cases 19
      calls  : func_L00_002B9598, func_00282E80, func_00282E48, func_00282C48, func_002AA2E8, func_L00_002B9B38, func_00283BE0, func_00282C48, func_L00_002B3EF8, func_L00_002B75A0, func_L00_002B41B8, func_L00_002B42D8, func_L00_002B4208, func_L00_002B4208, func_L00_002D34C0, func_L00_002D34C0, func_L00_002B9F00
      floats : 0x3F4CCCCD (0.8), 0x3F19999A (0.6), 0x3FD9999A (1.7), 0x3FA66666 (1.3), 0x3FF33333 (1.9), 0x3F8E38E4 (1.11111), 0x3D962FCA (0.0733333), 0x3D4CCCCE (0.05), 0x3D99999A (0.075), 0x3DC28F5C (0.095), 0x3D7C9631 (0.0616667), 0x3E4CCCCD (0.2), 0x3D4CCCCD (0.05), 0x3E90777C (0.282162), 0x3F490FDC (0.785398), 0x3C286420 (0.0102778), 0x3BFEDCBB (0.00777778), 0x47C34F80 (99999), 0x3BE38E39 (0.00694444), 0x3C75C290 (0.015), 0x406CCCCD (3.7), 0x3EB33333 (0.35)
      what   : the 0.8/0.6/1.7/1.3/1.9 scaling body with the 99999 sentinel and func_L00_002B42D8; ends at the tail
  body 18  0x002d13a0    336 bytes  cases 119
      calls  : func_002AA2E8, func_00282F20, func_L00_002D34C0, func_L00_002D3B58, func_00282D30, func_00283180, func_00283198, func_00283B98, func_L00_002B3EF8
      floats : 0x3BDA740E (0.00666667), 0x3BE38E39 (0.00694444), 0x40490FDB (3.14159), 0x3D4CCCCD (0.05), 0x3E4CCCCD (0.2), 0x3E90777C (0.282162)
      what   : a short pi/0.05 body with the g[0x110] vector maths; ends at the epilogue
  body 19  0x002d14f0   1244 bytes  cases 31
      calls  : func_L00_002B3EF8, func_L00_002B75A0, func_L00_00322A98, func_L00_00322AC0, func_L00_00322AC0, func_L00_00322AC0, func_L00_00322BE0, func_00283180, func_00283198, func_L00_002B9598, func_L00_002B41B8, func_L00_002B4208, func_L00_002B4208, func_00283180, func_00283198, func_002A8C00, func_00283180, func_00283198, func_00282CC0, func_00277F48, func_L00_002BA138, func_00283B98, func_00283180, func_00283198, func_00282CC0, func_00277F48, func_L00_002BA138, func_00283B98, func_00283180, func_00283198, func_00282CC0, func_00277F48, func_L00_002BA138, func_L00_002B9F00, func_L20_002C7588
      floats : 0x3D4CCCCD (0.05), 0x3E4CCCCD (0.2), 0x3E90777C (0.282162), 0x3F5F66F4 (0.872665), 0x3E99999A (0.3), 0x3EAAAAAB (0.333333), 0x3E111112 (0.141667), 0x3CA3D70B (0.02), 0x3D582D83 (0.0527778), 0x3CCCCCCD (0.025), 0x47C34F80 (99999), 0x3F4CCCCD (0.8), 0x3F0CCCCD (0.55), 0x3FA66666 (1.3), 0x45AFDF66 (5627.92), 0xBF5F66F4 (-0.872665), 0x406CCCCD (3.7), 0x3EB33333 (0.35), 0x3BC83FB8 (0.00611111), 0x3C75C290 (0.015)
      what   : the 0.333333/0.141667/0.0527778 body with func_L00_00322A98/AC0/BE0 and the 5627.92 constant; ends at the epilogue
  body 20  0x002d19cc    236 bytes  cases 34
      calls  : func_L00_002B3EF8, func_L00_002B75A0, func_L00_002B41B8, func_L00_002B4208
      floats : 0x3D23D70A (0.04), 0x3E4CCCCD (0.2), 0x3E90777C (0.282162), 0x3F5F66F4 (0.872665), 0x3C088889 (0.00833333), 0x3C1F49F5 (0.00972222), 0x47C34F80 (99999), 0x3C75C290 (0.015)
      what   : a short body (0.04, 0.00833333, 99999) with func_L00_002B75A0 and func_L00_002B4208; ends at the epilogue
  body 21  0x002d1ab8   1816 bytes  cases 2 112
      calls  : func_L00_002AA570, func_L00_002AA750, func_L00_002AA938, func_00282C48, func_L00_0032F038, func_L00_0032F038, func_00283C80, func_00282C48, func_00282C48, func_00282C48, func_L00_002B3D10, func_L00_002ADEA0, func_L00_002B54F8, func_L00_002B5590, func_00282C88, func_L00_002B35A0, func_L00_002B3EF8, func_00282C48, func_L00_002B3F68, func_L00_002B7B80, func_L00_002B8CB8, func_00282C48, func_00282C48, func_L00_002B41B8, func_L00_002B4208, func_L00_002B4208, func_L00_002B42D8, func_L00_002B42D8, func_L00_002B4208, func_L20_002C7588, func_L00_002B9F00
      floats : 0x3C888889 (0.0166667), 0x3C088889 (0.00833333), 0x3CE81B50 (0.0283333), 0x40060A93 (2.0944), 0x3FF5BE0D (1.91986), 0x3F9C61AB (1.22173), 0x3DB2B8C4 (0.0872665), 0x3D8EFA36 (0.0698132), 0x3E4CCCCD (0.2), 0x3F333333 (0.7), 0x3C3F258C (0.0116667), 0x3CA3D70A (0.02), 0x3E2E147B (0.17), 0x3E29C921 (0.165806), 0x3F666666 (0.9), 0x3F490FDC (0.785398), 0x3CEF078D (0.0291784), 0x3DCB17E5 (0.0991667), 0x3BA3D70A (0.005), 0x3BB60B61 (0.00555556), 0x40278D37 (2.61799), 0x3B9ABCDF (0.00472222), 0x3F1C61AB (0.610865), 0x3ECCCCCD (0.4), 0x47C34F80 (99999), 0x3EB33333 (0.35), 0x3BE38E39 (0.00694444), 0x3C75C290 (0.015)
      what   : the big 2.0944 (120 degrees) body: 1.91986/1.22173/0.0872665/0.0698132 angles, func_L00_002B35A0, func_L00_002B3F68, func_L00_002B7B80; ends at the tail
  body 22  0x002d21d0    840 bytes  cases 123
      calls  : func_L00_002B9BB0, func_L00_002B39F0, func_L00_002A9D38, func_L00_002B3DC0, func_L00_002B41B8, func_L00_002B4208, func_L00_002D3778, func_002AA458, func_002AA3A8, func_00282D30, func_00282CC0, func_00277F48, func_00282D30, func_002ABB78, func_002A8F70, func_L00_002AB0A8, func_L00_002AB0D8, func_L00_002AB0F0
      floats : 0x3E444445 (0.191667), 0x3E2E147B (0.17), 0x3CA6CE95 (0.0203622), 0x3DCCCCCD (0.1), 0x3E4CCCCD (0.2), 0x3CE38E39 (0.0277778), 0x3B888889 (0.00416667), 0x47C34F80 (99999), 0x3D23D70A (0.04), 0x3E99999A (0.3), 0x3F11EB85 (0.57), 0x3C8B4396 (0.017), 0x3CCCCCCD (0.025), 0x3CF5C28F (0.03), 0x3E8A3D71 (0.27), 0x3FD9999A (1.7), 0xBFD9999A (-1.7)
      what   : the 0.191667/0.17 body with func_L00_002B9BB0, func_L00_002AB0A8/D8/F0 and the 99999 sentinel; ends at the epilogue
  body 23  0x002d2518    536 bytes  cases 46
      calls  : func_L00_002B9598, func_L00_002AB0A8, func_L00_002AB0C0, func_L00_002AB0D8, func_L00_002AB0F0, func_00282C48, func_00282C48, func_00283248, func_00283BE0, func_00283B98, func_00283180, func_00283198
      floats : 0x3E4CCCCD (0.2), 0x3D23D70A (0.04), 0x3CA3D70A (0.02), 0x3FB33333 (1.4), 0xBFB33333 (-1.4), 0xBE0F5C29 (-0.14), 0x3E99999A (0.3), 0xBE6147AE (-0.22), 0x3ECCCCCD (0.4), 0xBDB851EC (-0.09), 0x3DA3D70A (0.08), 0x40490FDB (3.14159), 0x40DF66F4 (6.98132), 0x3EDF66F4 (0.436332), 0x3BE38E39 (0.00694444), 0x3D2AAAAB (0.0416667)
      what   : the rotation body: 1.4/-1.4/-0.14/-0.22, pi and 6.98132 (400 degrees); ends at the tail
  body 24  0x002d2730    868 bytes  cases 48
      calls  : func_L00_002B9BB0, func_L00_002B9BB0, func_L00_002AA938, func_L00_002AA938, func_002A7878, func_002A7878, func_002A7878, func_00282D30, func_002A7878, func_002A7878, func_002A7878, func_002A7878, func_L00_00338D28, func_L00_002B3EF8, func_00283C80, func_00282C88, func_L00_002B94D8, func_002AA898, func_002AA898, func_L00_002AB0C0, func_L00_002AB0D8, func_L00_002AB0F0
      floats : 0xBC3F258C (-0.0116667), 0x3C3F258C (0.0116667), 0x3ECCCCCD (0.4), 0xBDCCCCCD (-0.1), 0x3F666666 (0.9), 0x3C888889 (0.0166667), 0x3D088889 (0.0333333), 0x4713A800 (37800), 0x4793A800 (75600), 0x3E99999A (0.3), 0x3D8EFA37 (0.0698132), 0x3D072B02 (0.033), 0x3AFEDCBB (0.00194444), 0x3C638E39 (0.0138889), 0xBC360B61 (-0.0111111), 0x3CBEA2F4 (0.0232711), 0x3BE56042 (0.007), 0x3E2E147B (0.17), 0x3CA6CE95 (0.0203622), 0x3E4CCCCD (0.2), 0x3D031007 (0.0319977), 0x3D0F5C29 (0.035), 0x3C1374BC (0.009), 0x3E6147AE (0.22), 0x3D23D70A (0.04), 0x3CA3D70A (0.02), 0x40066666 (2.1)
      what   : the 37800/75600 timing body: the g[0xA38]/-0.0116667 steps, 2.1 and the func_L00_00338D28 call; ends at the tail
  body 25  0x002d2a94   3904 bytes  cases 39 40 41 42 62
      calls  : func_L00_002B9BB0, func_002E59F8, func_002A78D8, func_00283B98, func_002A7878, func_00283180, func_00283198, func_002A7878, func_002BE278, func_L00_00341FA8, func_L00_00341FA8, func_L00_002B6560, func_L00_00341820, func_L00_00341820, func_L00_00341820, func_L00_00341820, func_00283248, func_00283BE0, func_L00_002AB0A8, func_00282E20, func_00283248, func_L00_002AB0F0, func_L00_002AB0C0, func_L00_002AB0D8, func_00282CF0, func_00283098, func_00283248, func_00283BE0, func_00283BE0, func_00283B98, func_00283B98, func_00283B98, func_00283B98, func_L00_003417B8, func_00282CF0, func_00282DF0, func_00283248, func_00282E20, func_00283248, func_00283B98, func_L00_002B3EF8, func_00283248, func_00283C80, func_L00_002B3EF8, func_00282CF0, func_00282D30, func_00282CF0, func_00282D30, func_00282CC0, func_00282CF0, func_00282CF0, func_00282CF0, func_00282CF0, func_00282DF0, func_00282F20, func_L00_002B6560, func_L00_002B39F0, func_00283BE0, func_002AA898, func_002AA2E8, func_L00_00341820, func_00282E80, func_00283248, func_002AA2E8, func_002AA2E8, func_00282CF0, func_00282CF0, func_00282DF0, func_002AA3A8, func_00282DF0, func_00282F20, func_00282CC0, func_002A78D8, func_00283B98, func_00283180, func_00283198, func_002A8C00, func_00277F48
      floats : 0x3F060A93 (0.523599), 0x3DEEEEF0 (0.116667), 0x3D888889 (0.0666667), 0x3D4CCCCE (0.05), 0x3DAAAAAB (0.0833333), 0xBC088889 (-0.00833333), 0x4479C000 (999), 0x3F88F5C2 (1.07), 0x3BC83FB8 (0.00611111), 0x3DCCCCCD (0.1), 0x3EF0A3D7 (0.47), 0x3F490FDC (0.785398), 0xBF490FDC (-0.785398), 0x3CF5C28F (0.03), 0x3E4CCCCD (0.2), 0x3E99999A (0.3), 0x3C8B4396 (0.017), 0x3CDD2F1B (0.027), 0x3FC90FDC (1.5708), 0xBFC90FDC (-1.5708), 0x3E0EFA36 (0.139626), 0xBD8EFA36 (-0.0698132), 0xBF51FF7F (-0.820305), 0x3F169696 (0.588235), 0x3F3BA867 (0.733038), 0x3A83126F (0.001), 0x3C75C28F (0.015), 0x3E19999A (0.15), 0x3DD67752 (0.10472), 0x3D23D70A (0.04), 0x3CAAAAAB (0.0208333), 0x3C888889 (0.0166667), 0x3BD159E3 (0.00638889), 0x3E4CCCCE (0.2), 0x3B888889 (0.00416667), 0x3E860A93 (0.261799), 0xBDA2F982 (-0.0795775), 0xBE44980A (-0.191986), 0x3EC391D0 (0.381972), 0xBDFA35DE (-0.122173), 0x3B11A2B4 (0.00222222), 0x3A11A2B4 (0.000555556), 0xBD2AAAAB (-0.0416667), 0x3EB2B8C4 (0.349066), 0x3F1C61AB (0.610865), 0x3F19999A (0.6)
      what   : the largest body (3904 bytes, 5 cases): 34 calls, the 0.523599/1.5708/-0.820305/0.261799 trigonometry, the 999 sentinel, func_L00_00341FA8/func_L00_00341820/func_L00_003417B8; ends at the tail
  body 26  0x002d39d4    180 bytes  cases 23 24
      calls  : func_00282CF0, func_00283248, func_L00_002B3EF8, func_002AA800, func_00282CF0, func_002AC9F8
      floats : 0x3D23D70A (0.04), 0x3E4CCCCD (0.2), 0x3DD67752 (0.10472), 0x3DA0D97D (0.0785398), 0x3D888889 (0.0666667)
      what   : the 0.04/0.10472/0.0785398 body with func_002AC9F8; ends at the epilogue
  body 27  0x002d3a88    172 bytes  cases 70 71
      calls  : func_00282CF0, func_00283248, func_L00_002B3EF8, func_00282CF0, func_002AC9F8, func_002AA2E8
      floats : 0x3E4CCCCD (0.2), 0x3CA3D70A (0.02), 0x3DA0D97D (0.0785398), 0x3CE81B50 (0.0283333), 0x3D4CCCCE (0.05)
      what   : the 0.02/0.0283333 body with func_002AC9F8; ends at the epilogue
  body 28  0x002d3b34    320 bytes  cases 25 26
      calls  : func_00282CF0, func_00283248, func_L00_002B3EF8, func_00282CF0, func_00282C88, func_00282DD8, func_00282DD8, func_00282F20, func_00282CC0, func_L00_002ACF88, func_L00_002ACF88, func_00282CF0, func_00283CF0
      floats : 0x3CA3D70A (0.02), 0x3E4CCCCD (0.2), 0x3DA0D97D (0.0785398)
      what   : the 0.02/0.0785398 body with func_L00_002ACF88 and func_00283CF0; ends at the epilogue
  body 29  0x002d3c74     24 bytes  cases 118
      calls  : (none)
      floats : 0x3DBBBBBC (0.0916667)
      what   : an 24-byte body: the 0.0916667 constant and the frame fix-up only; ends at the epilogue
  body 30  0x002d3c8c    292 bytes  cases 6 44
      calls  : func_002AA898, func_002AA898, func_L00_002AA0F0, func_L00_002B3D10, func_L00_002B6BC8, func_L00_002B7B80, func_L00_002B54F8
      floats : 0x3EDF66F4 (0.436332), 0x3CA3D70A (0.02), 0x3E99999A (0.3), 0x3D32B8C4 (0.0436332), 0x3F333333 (0.7)
      what   : the 0.436332/0.0436332 (25 degrees) body with func_L00_002B6BC8 and func_L00_002B54F8; ends at the epilogue
  body 31  0x002d3db0    184 bytes  cases 116
      calls  : func_002AA2E8, func_002AA2E8, func_L00_002AA0F0, func_00282E20, func_L00_002EE4D8, func_L00_002D34C0, func_L00_002D3B00, func_L00_002D3B58
      floats : 0x3B1EDD20 (0.00242407), 0x3F666666 (0.9), 0xBF555556 (-0.833333)
      what   : the 0.9/-0.833333 body with func_L00_002AA0F0 and func_L00_002D3B00; ends at the epilogue
  body 32  0x002d3e68    924 bytes  cases 130
      calls  : func_L00_002B9BB0, func_L00_0031D620, func_L00_00325FC8, func_00290010, func_002AA2E8, func_L00_002D38A0, func_002AA2E8, func_002AA2E8, func_L00_002D34C0, func_L00_002D3B00, func_L00_002D3B58, func_L00_002D3B00, func_L00_002D3B58, func_L00_002D3B00, func_L00_002D3B58, func_002AA3A8, func_L00_002D3B00, func_L00_002B7B80, func_L00_002B39F0, func_L00_002B3EF8, func_L00_002B6198
      floats : 0x3CA3D70A (0.02), 0x3998825C (0.000290888), 0x3DCCCCCD (0.1), 0x3CCCCCCD (0.025), 0x3BF5C290 (0.0075), 0x3DCCCCCE (0.1), 0xBE4CCCCE (-0.2), 0x3F99999A (1.2), 0x3F86BCA1 (1.05263), 0x3CDD2F1B (0.027), 0x3E99999A (0.3), 0x3DAAAAAB (0.0833333), 0x3E4CCCCD (0.2), 0x3E4CCCCE (0.2), 0x3DB2B8C4 (0.0872665), 0x3AB60B61 (0.00138889), 0x3A11A2B4 (0.000555556)
      what   : the 0.000290888 body: 17 calls, the g[0x1B0] counter and the 1.05263 constant; ends at the epilogue
  body 33  0x002d4204    596 bytes  cases 8
      calls  : func_L00_002ADEA0, func_002E59F8, func_L00_002B9BB0, func_002E59F8, func_L00_002B9BB0, func_L00_002D3B00, func_L00_002B39F0, func_L00_002B3EF8, func_L00_002B41B8, func_L00_002B4208, func_L00_002B7B80, func_L00_002D3960, func_002AA3A8, func_L00_002D34C0, func_002AA2E8
      floats : 0x3D4CCCCE (0.05), 0x3DAAAAAB (0.0833333), 0x3F3851EC (0.72), 0x3E0369D0 (0.128333), 0x3F7C84B7 (0.9864), 0x3CCCCCCD (0.025), 0x3E99999A (0.3), 0x3E567752 (0.20944), 0x3AFEDCBB (0.00194444), 0x3B888889 (0.00416667), 0x47C34F80 (99999), 0x3E911112 (0.283333), 0x3D888889 (0.0666667)
      what   : the 0.72/0.9864/0.20944 body with func_L00_002D3960 and the 99999 sentinel; ends at the epilogue
  body 34  0x002d4458   2028 bytes  cases 7 9 10 11 12 13 14 15 16 17 18 27 56 69 72 91 102 110 129 137 146
      calls  : func_L00_002B3998, func_L00_002B9B88, func_L00_002B39F0, func_L00_002B39F0, func_L00_002AABE0, func_002FD1B8, func_L00_002AA938, func_L00_002B9598, func_00283180, func_00283198, func_L00_002B3EF8, func_L00_002B4208, func_L00_002B4208, func_L00_002B41B8, func_L00_002B94D8, func_L00_002B54F8, func_L00_002D3798, func_L00_002D3798, func_L00_002D38A0, func_L00_002D3798, func_002761F8, func_L00_002B6BC8, func_00282E20, func_00282F70, func_L00_002D3B00, func_L00_002B7B80, func_L00_002B8930, func_L00_00322BE0, func_L00_00322BE0, func_L00_002D36E0, func_00277F48, func_00282D30, func_002ABB78, func_002A8F70, func_L00_002AAE00, func_L00_002B6560, func_L00_002AC578
      floats : 0x3DCB17E5 (0.0991667), 0x3ECCCCCD (0.4), 0x3EB33333 (0.35), 0x3DEEEEF0 (0.116667), 0x3C3F258C (0.0116667), 0x3C75C28F (0.015), 0x3E4CCCCD (0.2), 0x3E80157C (0.250164), 0x47C34F80 (99999), 0x3D088889 (0.0333333), 0x3B91A2B4 (0.00444444), 0x3C71357A (0.0147222), 0x3F733333 (0.95), 0x3D7C9631 (0.0616667), 0x3DCCCCCE (0.1), 0x3A83126F (0.001), 0x3F4CCCCD (0.8), 0x3E99999A (0.3), 0x3F666666 (0.9), 0x3CCCCCCE (0.025)
      what   : the 21-case body (2028 bytes): 32 calls, the 0.250164/0.0147222/0.0616666 constants, func_L00_002D3798/func_L00_002D36E0 and func_00277F48; ends at the tail
  body 35  0x002d4c44    180 bytes  cases 22
      calls  : func_L00_002D3A58, func_L00_002B9F00, func_L00_002D3BD8
      floats : 0x3A91A2B4 (0.00111111), 0x3B360B61 (0.00277778), 0x406CCCCD (3.7), 0x3EB33333 (0.35)
      what   : the 0.00277778 body with func_L00_002D3A58 and func_L00_002D3BD8; ends at the epilogue
  body 36  0x002d4cf8     36 bytes  cases 140
      calls  : (none)
      floats : 0xBEC44445 (-0.383333)
      what   : a 36-byte body: the -0.383333 constant and the frame fix-up only; ends at the epilogue
  body 37  0x002d4d1c    592 bytes  cases 98
      calls  : func_00282CF0, func_00283248, func_00282E20, func_00283248, func_L00_002B3EF8, func_L00_002B3EF8, func_L00_002B3EF8, func_L00_002B7B80, func_L00_002B41B8, func_L00_002B4208, func_L20_002C7588
      floats : 0x3E4CCCCD (0.2), 0x3C888889 (0.0166667), 0x3C75C290 (0.015), 0x3CF5C28F (0.03), 0x3E99999A (0.3), 0x3DBEA2F4 (0.0930842), 0x3E060A93 (0.1309), 0x3BA3D70A (0.005), 0x3BB60B61 (0.00555556), 0x47C34F80 (99999), 0x3BE38E39 (0.00694444)
      what   : the 0.0930842/0.1309 body with the 99999 sentinel; ends at the tail
  body 38  0x002d4f6c    164 bytes  cases 99
      calls  : func_00282CF0, func_00282DB0, func_00282C88, func_00282C88, func_L00_002B94D8, func_L20_002C7588
      floats : 0x3BF5C290 (0.0075), 0x3BE38E39 (0.00694444), 0x3C75C290 (0.015)
      what   : the 0.0075/0.00694444 body with func_00282DB0; ends at the epilogue
  body 39  0x002d5010     20 bytes  cases 60 100
      calls  : func_00282C88
      floats : (none)
      what   : a 20-byte body: func_00282C88 and the frame fix-up only; ends at the epilogue
  body 40  0x002d5024    108 bytes  cases 101
      calls  : func_00282C88
      floats : 0xBB888889 (-0.00416667), 0xBC888889 (-0.0166667)
      what   : the -0.00416667/-0.0166667 body with func_00282C88; ends at the epilogue
  body 41  0x002d5090      8 bytes  cases 121
      calls  : func_L00_002BA220
      floats : (none)
      what   : an 8-byte body: func_L00_002BA220 and nothing else; ends at the epilogue
  body 42  0x002d5098     36 bytes  cases 120
      calls  : func_00282C88
      floats : 0xBC888889 (-0.0166667)
      what   : a 36-byte body: the -0.0166667 constant and func_00282C88; ends at the epilogue
  body 43  0x002d50bc     36 bytes  cases 122
      calls  : func_00282C88
      floats : 0xBBBF258C (-0.00583333)
      what   : a 36-byte body: the -0.00583333 constant and func_00282C88; ends at the epilogue
  body 44  0x002d50e0    152 bytes  cases 63 67
      calls  : func_L00_002B9F00, func_L00_002B39F0, func_L00_002B41B8, func_00282C88, func_L20_002C7588
      floats : 0x3EB33333 (0.35), 0x406CCCCD (3.7), 0x3B656042 (0.0035), 0x3BE38E39 (0.00694444), 0x3C75C290 (0.015)
      what   : the 0.0035/0.00694444 body with func_L00_002B9F00 and func_L00_002B39F0; ends at the epilogue
  body 45  0x002d5178    688 bytes  cases 77
      calls  : func_0029FCC0, func_0029FCC0, func_00282CC0, func_00282D30, func_002A8C28, func_002AA2E8, func_L00_002B3EF8, func_L00_002B75A0, func_L00_002B41B8, func_L00_002B42D8, func_L00_002B4208, func_L00_002B4208, func_L00_002B4308, func_L00_002B9F00
      floats : 0x3D6EEEF0 (0.0583333), 0x3E4CCCCD (0.2), 0x3D4CCCCD (0.05), 0x3E90777C (0.282162), 0x3F490FDC (0.785398), 0x3C286420 (0.0102778), 0x3BFEDCBB (0.00777778), 0x47C34F80 (99999), 0x3BE38E39 (0.00694444), 0x3C75C290 (0.015), 0x3F333333 (0.7), 0x3E99999A (0.3)
      what   : the 0.0583333 body with func_002A8C28, func_L00_002B42D8 and the 99999 sentinel; ends at the epilogue
  body 46  0x002d5428    148 bytes  cases 75
      calls  : func_L00_002B39F0, func_L00_002B3EF8, func_L00_002B41B8, func_L00_002B4208, func_L00_002D3960
      floats : 0x3D088889 (0.0333333), 0x3E14EF4E (0.145444), 0x3D23D70A (0.04), 0x3E4CCCCD (0.2), 0x3AFEDCBB (0.00194444), 0x3B888889 (0.00416667), 0x47C34F80 (99999), 0x3CC49BA7 (0.024)
      what   : the 0.145444 body with func_L00_002D3960 and the 99999 sentinel; ends at the epilogue
  body 47  0x002d54bc     84 bytes  cases 78
      calls  : func_L00_002B6198, func_L00_002D34C0
      floats : 0x3DBBBBBC (0.0916667), 0x3BE38E39 (0.00694444), 0x3BA3D70A (0.005)
      what   : the 0.0916667/0.005 body with func_L00_002B6198; ends at the epilogue
  body 48  0x002d5510     24 bytes  cases 65
      calls  : (none)
      floats : 0x3D4CCCCE (0.05)
      what   : a 24-byte body: the 0.05 constant and the frame fix-up only; ends at the epilogue
  body 49  0x002d5528     68 bytes  cases 68
      calls  : func_L00_002AA0F0
      floats : 0x3E567752 (0.20944), 0x3BB60B61 (0.00555556)
      what   : the 0.20944/0.00555556 body with func_L00_002AA0F0; ends at the epilogue
  body 50  0x002d556c    688 bytes  cases 64 76
      calls  : func_L00_002B39F0, func_L00_002B3EF8, func_L00_002B41B8, func_00282C48, func_L00_0032F038, func_L00_0032F038, func_00283C80, func_00282C48, func_00282C48, func_00282C48, func_00282C48, func_L00_002B4208, func_L00_002B4208, func_L20_002C7588
      floats : 0x3D4CCCCE (0.05), 0x3E29C921 (0.165806), 0x3D072B02 (0.033), 0x3E479297 (0.194895), 0x3D178D50 (0.037), 0x3E19999A (0.15), 0x3BA3D70A (0.005), 0x3BB60B61 (0.00555556), 0x3C3F258C (0.0116667), 0x3FDF66F4 (1.74533), 0x3F4CCCCD (0.8), 0x40278D37 (2.61799), 0x3F9C61AB (1.22173), 0x3DB2B8C4 (0.0872665), 0x3D8EFA36 (0.0698132), 0x3CADD6F3 (0.0212207), 0x47C34F80 (99999), 0x3C75C290 (0.015)
      what   : the 1.74533 (100 degrees) body: 0.165806/0.194895/0.0872665/0.0698132, func_L00_0032F038 and the 99999 sentinel; ends at the epilogue
  body 51  0x002d581c    108 bytes  cases 66
      calls  : func_00282E20, func_L00_002EE4D8
      floats : 0x3F6B851F (0.92), 0x3F7D70A4 (0.99)
      what   : the 0.92/0.99 body with func_00282E20 and func_L00_002EE4D8; ends at the epilogue
  body 52  0x002d5888    364 bytes  cases 73 74
      calls  : func_00282CF0, func_00283248, func_L00_002B3EF8, func_00282CF0, func_00282C88, func_00282DD8, func_00282DD8, func_00282F20, func_00282CC0, func_L00_002ACF88, func_L00_002ACF88, func_00282CF0, func_00283CF0, func_00282F20
      floats : 0x3FA66666 (1.3), 0x3CA3D70A (0.02), 0x3E4CCCCD (0.2), 0x3DA0D97D (0.0785398)
      what   : the 1.3 body with func_L00_002ACF88 and func_00283CF0; ends at the epilogue
  body 53  0x002d59f4    188 bytes  cases 141
      calls  : func_L00_002B9F00, func_L18_002C4000, func_L00_002B39F0, func_L00_002B41B8, func_00282C88, func_L20_002C7588
      floats : 0x406CCCCD (3.7), 0x3EB33333 (0.35), 0x3B656042 (0.0035), 0x3BE38E39 (0.00694444), 0x3C75C290 (0.015)
      what   : the 0.0035/0.00694444 body with the level's func_L18_002C4000; ends at the epilogue
  body 54  0x002d5ab0     84 bytes  cases 145
      calls  : func_L20_002C5A48, func_0027D330
      floats : (none)
      what   : an 84-byte body: func_L20_002C5A48 and func_0027D330 only; ends at the epilogue
  body 55  0x002d5b04    232 bytes  cases 147
      calls  : func_L00_002B41B8, func_L00_002B4208, func_L00_002B4308, func_L00_002B9F00, func_L18_00428738, func_L20_002D7730
      floats : 0x3C286420 (0.0102778), 0x3BFEDCBB (0.00777778), 0x47C34F80 (99999), 0x3BE38E39 (0.00694444), 0x3C75C290 (0.015), 0x406CCCCD (3.7), 0x3EB33333 (0.35)
      what   : the 0.0102778/0.00777778 body with func_L18_00428738 and func_L20_002D7730; ends at the epilogue
  body 56  0x002d5bec    108 bytes  cases 148
      calls  : func_L00_002B6198, func_L00_002D34C0
      floats : 0x3DBBBBBC (0.0916667), 0x3BE38E39 (0.00694444), 0x3BA3D70A (0.005)
      what   : the 0.0916667/0.005 body with func_L00_002B6198; ends at the epilogue
  body 57  0x002d5c58    152 bytes  cases 143
      calls  : func_L18_002C4000, func_L00_002B39F0, func_L00_002B6BC8, func_L00_002D34C0
      floats : 0x3DC44445 (0.0958333), 0x3B5A740E (0.00333333), 0xBE2AAAAB (-0.166667)
      what   : the 0.0958333/-0.166667 body with the level's func_L18_002C4000 and func_L00_002B6BC8; ends at the epilogue
  body 58  0x002d5cf0    800 bytes  cases 142
      calls  : func_L00_002B39F0, func_L18_002C4000, func_L00_002B35A0, func_L00_002B3EF8, func_L00_002B41B8, func_00282C48, func_L00_0032F038, func_L00_0032F038, func_00283C80, func_00282C48, func_00282C48, func_00282C48, func_00282C48, func_L00_002B42D8, func_L00_002B42D8, func_L20_002C7588, func_L00_002D34C0, func_L00_002B4308
      floats : 0x3DC44445 (0.0958333), 0x3DDC6C69 (0.107629), 0x3D072B02 (0.033), 0x3E0BFFAB (0.136717), 0x3D178D50 (0.037), 0x3E19999A (0.15), 0x3BA3D70A (0.005), 0x3BB60B61 (0.00555556), 0x3C3F258C (0.0116667), 0x3FDF66F4 (1.74533), 0x3F4CCCCD (0.8), 0x40278D37 (2.61799), 0x3F9C61AB (1.22173), 0x3DB2B8C4 (0.0872665), 0x3D8EFA36 (0.0698132), 0x3CADD6F3 (0.0212207), 0x3BE38E39 (0.00694444), 0x3C75C290 (0.015)
      what   : the 800-byte 1.74533 body with func_L18_002C4000 and func_L00_0032F038; its tail at 0x002d5fe4 is a shared hub (7 branches reach it); ends at the epilogue
  body 59  0x002d6010    192 bytes  cases 144
      calls  : func_00282E20, func_L00_002EE4D8, func_L00_002D34C0, func_L00_002D34C0
      floats : 0x3F6B851F (0.92), 0x3F7D70A4 (0.99), 0x3BC83FB8 (0.00611111), 0x3B83126F (0.004)
      what   : the 192-byte body with the 0.92/0.99/0.00611111 constants; its tail at 0x002d60c0 is the hub 14 other bodies branch to (func_L00_002D34C0 then the epilogue); ends at the epilogue
  body 60  0x002d60d0    120 bytes  cases 55
      calls  : func_L00_002B3EF8, func_L00_002B3EF8
      floats : 0x3E0C49BA (0.137), 0x3E50823A (0.203622), 0x3DCCCCCD (0.1), 0x3ED0823A (0.407244)
      what   : the 0.137/0.203622/0.407244 body with func_L00_002B3EF8; ends at the epilogue
  body 61  0x002d6148     12 bytes  cases 96 97
      calls  : func_00282C88
      floats : (none)
      what   : a 12-byte body: func_00282C88 and the frame fix-up only; ends at the epilogue
  body 62  0x002d6154    108 bytes  cases 5 32 33 38 45 49 58 59 61 79 80 81 82 83 84 85 86 87 88 89 90 92 93 94 95 104 105 106 107 108 113 117 126 128 132 135 136
      calls  : (none)
      floats : (none)
      what   : the default body: the 36 case values with no label land here; it is the tail (addiu $v1,$s6,%lo(D_189E20)) and the epilogue (the two bnel tests and the restores)

   9. THE EXTENDED BODY MAP (wave 3).  For each of the 63 bodies (the 62 bodies
      plus the default): the number of instructions, the number of calls, the
      offsets the body reads or writes in the boot global D_00189E20 (the ones
      >= 0x2000), the offsets it uses on the objects it drives (the smaller
      ones), and where the body's branches go: into another body (the source's
      `goto`), into one of the two shared hubs, into the tail or into the
      epilogue.

  body  0  0x002CD9B0   1392 bytes   348 insns   19 calls  cases [0, 1, 3, 4, 28, 29, 30, 57, 109, 111, 125]
        global fields: [8848, 8852, 8885, 8889, 8891, 9388, 9400, 31840]
        other offsets: [0, 65, 66, 67, 216, 220, 224, 372, 380, 416, 420, 432, 440, 536, 796, 828, 846, 3104, 5266, 5271]
        ends in      : body 58 + 0x2e4, the hub 0x2d60c0, the tail, the epilogue
  body  1  0x002CDF20    328 bytes    82 insns    8 calls  cases [124]
        global fields: [8848]
        other offsets: [0, 35, 48, 49, 50, 52, 248, 1956]
        ends in      : the epilogue
  body  2  0x002CE068   3412 bytes   853 insns   50 calls  cases [50, 51, 52]
        global fields: [8848, 8852, 8856, 8864, 9280, 9400, 9408, 12528, 15064, 18248]
        other offsets: [0, 4, 8, 24, 32, 88, 116, 128, 132, 136, 140, 148, 152, 170, 224, 272, 276, 280, 296, 300, 416, 420, 432, 436, 440, 476, 804, 808, 868, 872, 980, 984, 1044, 1048, 2592, 2604, 2608, 2616, 2624, 2626, 2634, 2636, 2638, 2640, 2644, 3116, 3128, 4880]
        ends in      : the tail
  body  3  0x002CEDBC    120 bytes    30 insns    4 calls  cases [115]
        global fields: []
        other offsets: [248, 440]
        ends in      : the epilogue
  body  4  0x002CEE34     68 bytes    17 insns    3 calls  cases [114]
        global fields: []
        other offsets: [248]
        ends in      : body 6 + 0x2b0
  body  5  0x002CEE78    444 bytes   111 insns   13 calls  cases [103, 127]
        global fields: [8848]
        other offsets: [0, 24, 32, 228, 248, 440, 2608, 2620]
        ends in      : the tail
  body  6  0x002CF034    712 bytes   178 insns   17 calls  cases [53, 54]
        global fields: [8852]
        other offsets: [152, 240, 244, 248, 372, 424, 432, 436, 468, 1944, 2634, 3104, 3116, 3128]
        ends in      : the epilogue
  body  7  0x002CF2FC     48 bytes    12 insns    0 calls  cases [133]
        global fields: []
        other offsets: [0, 6192]
        ends in      : the epilogue
  body  8  0x002CF32C    380 bytes    95 insns   11 calls  cases [134]
        global fields: [9400]
        other offsets: [4, 436, 1984, 8112, 8116]
        ends in      : the epilogue
  body  9  0x002CF4A8    368 bytes    92 insns   12 calls  cases [47]
        global fields: [8848]
        other offsets: [66, 96, 606, 5266]
        ends in      : the tail, the epilogue
  body 10  0x002CF618    168 bytes    42 insns    7 calls  cases [138, 139]
        global fields: []
        other offsets: [432, 846, 2832]
        ends in      : body 11 + 0xa4
  body 11  0x002CF6C0    256 bytes    64 insns    9 calls  cases [35]
        global fields: []
        other offsets: [16, 20, 416, 432, 846, 2724]
        ends in      : the epilogue
  body 12  0x002CF7C0    804 bytes   201 insns   20 calls  cases [36]
        global fields: [8856]
        other offsets: [0, 8, 16, 20, 32, 36, 40, 136, 152, 232, 248, 368, 416, 550, 928, 2720, 2724, 2732, 2736, 2740]
        ends in      : the epilogue
  body 13  0x002CFAE4    236 bytes    59 insns    8 calls  cases [37]
        global fields: []
        other offsets: [0, 248, 2724, 2736, 2748, 2752]
        ends in      : the epilogue
  body 14  0x002CFBD0   2032 bytes   508 insns   55 calls  cases [43]
        global fields: [8848, 9400]
        other offsets: [0, 4, 8, 12, 16, 20, 24, 52, 66, 67, 104, 136, 152, 176, 180, 184, 208, 212, 216, 240, 244, 248, 272, 416, 440, 796, 846, 2772, 2780, 2784, 2788, 2796, 2800, 2804, 2812, 2814, 2820, 2824, 2828, 3104, 3112]
        ends in      : the tail, the epilogue
  body 15  0x002D03C0   1556 bytes   389 insns   27 calls  cases [20]
        global fields: [8848, 24576]
        other offsets: [0, 24, 28, 40, 41, 42, 152, 196, 240, 244, 248, 280, 364, 384, 392, 416, 432, 436, 440, 506, 548, 796, 846, 3044, 3048, 3052, 3068, 3072, 3116, 3128, 4665, 5264, 5266, 5269, 5270, 5275]
        ends in      : body 31 + 0xa8, the tail
  body 16  0x002D09D4   1100 bytes   275 insns   17 calls  cases [21, 131]
        global fields: [8848, 8852, 8864, 8872, 8899, 31968]
        other offsets: [0, 24, 67, 152, 240, 244, 368, 416, 432, 440, 846, 3044, 3048, 3052, 3068, 3104, 3116, 3128, 4665, 5266]
        ends in      : body 17 + 0x58, body 56 + 0x5c, the hub 0x2d60c0
  body 17  0x002D0E20   1408 bytes   352 insns   17 calls  cases [19]
        global fields: [8899]
        other offsets: [0, 4, 32, 152, 416, 432, 440, 846, 3044, 3048, 3052, 3060, 3068, 3104, 3116, 3128, 4665, 5266]
        ends in      : the epilogue
  body 18  0x002D13A0    336 bytes    84 insns    9 calls  cases [119]
        global fields: []
        other offsets: [240, 244, 248, 296, 416, 436, 796, 3056]
        ends in      : the epilogue
  body 19  0x002D14F0   1244 bytes   311 insns   35 calls  cases [31]
        global fields: [8848, 8852]
        other offsets: [0, 12, 24, 25, 26, 48, 52, 56, 64, 68, 72, 152, 170, 240, 244, 376, 388, 416, 432, 440, 846, 3044, 3048, 3052, 4640, 5266]
        ends in      : the hub 0x2d5fe4, the hub 0x2d60c0
  body 20  0x002D19CC    236 bytes    59 insns    4 calls  cases [34]
        global fields: []
        other offsets: [0, 416, 432, 846, 3048, 3052]
        ends in      : the hub 0x2d5fe4, the hub 0x2d60c0
  body 21  0x002D1AB8   1816 bytes   454 insns   31 calls  cases [2, 112]
        global fields: [8856, 8889, 8890, 8899, 9400, 9404, 9412]
        other offsets: [0, 152, 372, 416, 424, 432, 532, 846, 1032, 1036, 1039, 1040, 2656, 2672, 5204, 5208, 5268]
        ends in      : the hub 0x2d5fe4, the hub 0x2d60c0
  body 22  0x002D21D0    840 bytes   210 insns   18 calls  cases [123]
        global fields: [8848, 9400]
        other offsets: [0, 96, 432, 440, 456, 632, 792]
        ends in      : the tail
  body 23  0x002D2518    536 bytes   134 insns   12 calls  cases [46]
        global fields: []
        other offsets: [67, 96, 152, 248, 272, 276, 280, 372, 424, 448, 624, 628, 632, 846]
        ends in      : body 47 + 0x3c, the hub 0x2d60c0
  body 24  0x002D2730    868 bytes   217 insns   22 calls  cases [48]
        global fields: [8848, 30208]
        other offsets: [0, 4, 8, 16, 20, 24, 128, 132, 136, 152, 248, 272, 280, 416, 2880, 2884]
        ends in      : the tail
  body 25  0x002D2A94   3904 bytes   976 insns   78 calls  cases [39, 40, 41, 42, 62]
        global fields: [8848, 8852, 8890, 9260, 9400]
        other offsets: [0, 16, 20, 24, 28, 32, 36, 40, 48, 52, 56, 66, 67, 80, 84, 88, 104, 116, 128, 132, 136, 152, 240, 244, 248, 252, 256, 260, 264, 268, 276, 280, 284, 368, 416, 440, 444, 456, 632, 796, 1180, 1496, 1568, 1572, 1584, 1588, 1592, 1600, 1604, 1608, 1612, 1616, 1620, 1624, 1628, 1632, 1636, 1648, 1652, 1656, 1660, 1664, 1684, 1692, 1696, 1704, 1708, 1736, 4680]
        ends in      : the tail, the epilogue, body 62 + 0x8
  body 26  0x002D39D4    180 bytes    45 insns    6 calls  cases [23, 24]
        global fields: [8899]
        other offsets: [136, 400, 404, 416]
        ends in      : body 27 + 0x88
  body 27  0x002D3A88    172 bytes    43 insns    6 calls  cases [70, 71]
        global fields: []
        other offsets: [136, 400, 404, 416]
        ends in      : the epilogue
  body 28  0x002D3B34    320 bytes    80 insns   13 calls  cases [25, 26]
        global fields: [8852]
        other offsets: [400, 404, 416, 3000]
        ends in      : body 52 + 0x144, the tail
  body 29  0x002D3C74     24 bytes     6 insns    0 calls  cases [118]
        global fields: []
        other offsets: [812]
        ends in      : body 56 + 0x34
  body 30  0x002D3C8C    292 bytes    73 insns    7 calls  cases [6, 44]
        global fields: [8852, 8856]
        other offsets: [0, 160, 164, 480, 796, 2688]
        ends in      : body 31 + 0x74
  body 31  0x002D3DB0    184 bytes    46 insns    8 calls  cases [116]
        global fields: [8892]
        other offsets: [2688, 2696, 2700, 2704, 2708]
        ends in      : the epilogue
  body 32  0x002D3E68    924 bytes   231 insns   21 calls  cases [130]
        global fields: [8848, 9400]
        other offsets: [152, 416, 420, 792, 796, 1456, 1470]
        ends in      : the epilogue
  body 33  0x002D4204    596 bytes   149 insns   15 calls  cases [8]
        global fields: [8848, 9272, 9276, 9416, 9455]
        other offsets: [116, 136, 272, 1168, 4920]
        ends in      : body 34 + 0x7dc
  body 34  0x002D4458   2028 bytes   507 insns   37 calls  cases [7, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 27, 56, 69, 72, 91, 102, 110, 129, 137, 146]
        global fields: [8848, 8852, 8884, 9400, 9468, 31916]
        other offsets: [0, 128, 132, 136, 152, 240, 244, 248, 376, 432, 436, 440, 792, 842, 1076, 1112, 1168, 1180, 1181, 1182, 1188, 1196, 1280, 1284, 1312, 1316, 1320, 3128, 4920, 5204, 5208, 5266]
        ends in      : the epilogue
  body 35  0x002D4C44    180 bytes    45 insns    3 calls  cases [22]
        global fields: []
        other offsets: [440, 846, 5271]
        ends in      : body 59 + 0xa4
  body 36  0x002D4CF8     36 bytes     9 insns    0 calls  cases [140]
        global fields: []
        other offsets: [0, 248]
        ends in      : the tail
  body 37  0x002D4D1C    592 bytes   148 insns   11 calls  cases [98]
        global fields: [8884, 9400]
        other offsets: [0, 4, 416, 432, 846, 6316]
        ends in      : the hub 0x2d5fe4, the hub 0x2d60c0
  body 38  0x002D4F6C    164 bytes    41 insns    6 calls  cases [99]
        global fields: []
        other offsets: [5458]
        ends in      : the hub 0x2d5fe4, the hub 0x2d60c0
  body 39  0x002D5010     20 bytes     5 insns    1 calls  cases [60, 100]
        global fields: []
        other offsets: []
        ends in      : the epilogue
  body 40  0x002D5024    108 bytes    27 insns    1 calls  cases [101]
        global fields: []
        other offsets: [136, 248, 820, 6336]
        ends in      : the tail
  body 41  0x002D5090      8 bytes     2 insns    1 calls  cases [121]
        global fields: []
        other offsets: []
        ends in      : (falls through to the next body)
  body 42  0x002D5098     36 bytes     9 insns    1 calls  cases [120]
        global fields: []
        other offsets: [8]
        ends in      : the tail
  body 43  0x002D50BC     36 bytes     9 insns    1 calls  cases [122]
        global fields: []
        other offsets: [8]
        ends in      : the tail
  body 44  0x002D50E0    152 bytes    38 insns    5 calls  cases [63, 67]
        global fields: []
        other offsets: [432, 846]
        ends in      : the hub 0x2d5fe4, the hub 0x2d60c0
  body 45  0x002D5178    688 bytes   172 insns   14 calls  cases [77]
        global fields: [8848, 8899]
        other offsets: [4, 28, 32, 248, 280, 416, 432, 846, 3044, 3048, 3052, 3068, 3116, 3128]
        ends in      : the epilogue
  body 46  0x002D5428    148 bytes    37 insns    5 calls  cases [75]
        global fields: []
        other offsets: []
        ends in      : the hub 0x2d60c0
  body 47  0x002D54BC     84 bytes    21 insns    2 calls  cases [78]
        global fields: []
        other offsets: [416, 432, 812]
        ends in      : the epilogue
  body 48  0x002D5510     24 bytes     6 insns    0 calls  cases [65]
        global fields: []
        other offsets: []
        ends in      : body 57 + 0x3c
  body 49  0x002D5528     68 bytes    17 insns    1 calls  cases [68]
        global fields: []
        other offsets: [248]
        ends in      : the hub 0x2d60c0
  body 50  0x002D556C    688 bytes   172 insns   14 calls  cases [64, 76]
        global fields: [9400]
        other offsets: [372, 416, 424, 432, 846, 1036, 1039]
        ends in      : body 58 + 0x2e4, the hub 0x2d60c0
  body 51  0x002D581C    108 bytes    27 insns    2 calls  cases [66]
        global fields: []
        other offsets: [248, 606, 846]
        ends in      : body 59 + 0x80, body 59 + 0xa4
  body 52  0x002D5888    364 bytes    91 insns   14 calls  cases [73, 74]
        global fields: [8852]
        other offsets: [0, 400, 404, 416, 3000, 3104]
        ends in      : the tail, the epilogue
  body 53  0x002D59F4    188 bytes    47 insns    6 calls  cases [141]
        global fields: [8852]
        other offsets: [432, 846]
        ends in      : the hub 0x2d5fe4, the hub 0x2d60c0, the epilogue
  body 54  0x002D5AB0     84 bytes    21 insns    2 calls  cases [145]
        global fields: [8852, 8860, 8897, 9376]
        other offsets: [480, 3856]
        ends in      : the tail, the epilogue
  body 55  0x002D5B04    232 bytes    58 insns    6 calls  cases [147]
        global fields: [8852]
        other offsets: [248, 280, 432, 846]
        ends in      : the tail, the epilogue
  body 56  0x002D5BEC    108 bytes    27 insns    2 calls  cases [148]
        global fields: [8852]
        other offsets: [416, 432, 812]
        ends in      : the epilogue
  body 57  0x002D5C58    152 bytes    38 insns    4 calls  cases [143]
        global fields: [8852]
        other offsets: [248, 480]
        ends in      : the epilogue
  body 58  0x002D5CF0    800 bytes   200 insns   18 calls  cases [142]
        global fields: [8852, 9400]
        other offsets: [0, 152, 372, 424, 432, 846, 1036, 1039, 5204, 5208]
        ends in      : the hub 0x2d60c0, the epilogue
  body 59  0x002D6010    192 bytes    48 insns    4 calls  cases [144]
        global fields: [8852]
        other offsets: [248, 846]
        ends in      : the epilogue
  body 60  0x002D60D0    120 bytes    30 insns    2 calls  cases [55]
        global fields: [8848]
        other offsets: [67]
        ends in      : the epilogue
  body 61  0x002D6148     12 bytes     3 insns    1 calls  cases [96, 97]
        global fields: []
        other offsets: []
        ends in      : (falls through to the next body)
  body 62  0x002D6154     40 bytes    10 insns    0 calls  cases [5, 32, 33, 38, 45, 49, 58, 59, 61, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 92, 93, 94, 95, 104, 105, 106, 107, 108, 113, 117, 126, 128, 132, 135, 136]
        global fields: [8856]
        other offsets: [67, 276, 280, 288, 448, 452]
        ends in      : (falls through to the next body)

/* =====================================================================================
   WAVE 3 (worker 16 of the level-20 wave) -- THE 64-BIT BASE IS SOLVED, THE JUMP
   TABLE'S SPELLING IS SETTLED, AND THE BODY MAP IS EXTENDED.

   1. `daddu $s6, $v0, $zero` IS NOT A 64-BIT MOVE: IT IS cc1's `move $s6, $v0`.

   The earlier waves read the retail's

       lui    $v0, %hi(D_189E20)      0x002CD92C
       daddu  $s6, $v0, $zero         0x002CD938
       addiu  $v0, $v0, %lo(D_189E20) 0x002CD940
       ...
       addiu  $s0, $s6, %lo(D_189E20) 0x002CD9B0   (380 such uses)

   as a "split 64-bit constant": $s6 holding the %hi zero-extended into 64 bits,
   every use adding the %lo with a 32-bit addiu.  That reading is wrong, and the
   wrongness is worth a line of the report: cc1's `move` is assembled by the ee
   gas (2.9-ee) as `daddu rd, rs, $zero`, so the retail's `daddu` is a plain
   32-bit register copy.

   MEASURED (the compiler in build/compiler/tools, the object read with
   tools/elf.py): the C `move $4, $16` comes out of cc1 as the text `move $4,$16`
   and out of the assembler as the word 0x2D020020 = `daddu $a0, $s0, $zero`.
   The same holds for every `move` between two general registers.

   So $s6 holds the 32-bit %hi of D_00189E20 (0x00190000) and every use adds
   the %lo with a 32-bit addiu.  There is no 64-bit anything in this pattern.

   2. THE PATTERN IS GCC 2.x's SPLIT ADDRESS, and the compiler source says so.

   In the compiler this repository uses (build/sdk-compiler-build/source/ee/gcc,
   the ee-gcc 2.9 the game was built with):

     config/mips/mips.h:3180 (LEGITIMIZE_ADDRESS)
         if (mips_split_addresses && mips_check_split (X, MODE))
           X = gen_rtx (LO_SUM, Pmode,
                        copy_to_mode_reg (Pmode, gen_rtx (HIGH, Pmode, X)), X);
     config/mips/mips.md:5327  the "high" pattern
         (set (reg:SI) (high:SI (immediate)))  ->  "lui\t%0,%%hi(%1) # high"
     config/mips/mips.md:5337  the "low" pattern
         (set (reg:SI) (lo_sum:SI (reg:SI) (immediate))) -> "addiu\t%0,%1,%%lo(%2) # low"

   So the retail's three instructions are the "high" pattern, a copy of its
   result into the register the split keeps, and the "low" pattern:

       lui    $v0, %hi(D_189E20)      # the "high" pattern, into a temporary
       daddu  $s6, $v0, $zero         # move $s6, $v0 : the %hi into the register
                                     #   the split keeps (a callee-saved one)
       addiu  $s0, $s6, %lo(D_189E20) # the "low" pattern: the address, at each use

   The split is enabled because the file is built with -O2 (mips.c:4495 wants
   `optimize`, `TARGET_GAS` and `TARGET_SPLIT_ADDRESSES`).

   3. THE SPELLING: HOW TO MAKE THE COMPILER KEEP THE %hi AND RE-ADD THE %lo.

   The retail keeps the %hi in a callee-saved register and re-does the "low" at
   each of its 380 uses.  The compiler only does that when

     (a) the address of the global is used as a VALUE (a function argument) at
         several call sites, so it is not a memory operand that can take the
         %lo as a displacement, and
     (b) those call sites are in DIFFERENT basic blocks, so each one is a
         separate reload of the address pseudo (in the same block the compiler
         CSEs the whole address into one register), and
     (c) the function is big enough that the %hi stays live across the calls,
         so the compiler puts the "high" into a callee-saved register and copies
         it there (that copy is the retail's `daddu $s6, $v0, $zero`).

   MEASURED REPRODUCTION (a function with twelve call sites in twelve blocks,
   each passing the address of the global to a function; cc1 -O2 -G8, the
   object read with tools/elf.py):

       lui    $2, %hi(D_00189E20)      # the "high" into a temporary
       addiu  $17, $2, %lo(D_00189E20) # the "low": the pointer variable, once
       ...
       daddu  $19, $2, $zero           # move $19,$2 : the %hi into $s3
       ...
       addiu  $4, $19, %lo(D_00189E20) # the "low" again, at each use

   which is the retail's instruction for instruction.  Compare the retail's own
   prologue in func_L20_002DEBE0 (0x002DEBE0):

       lui    $v0,%hi(D_189E20)
       addiu  $s0,$v0,%lo(D_189E20)    # the pointer variable
       daddu  $s3,$v0,$zero            # the %hi into a callee-saved register
       ...
       addiu  $a0,$s3,%lo(D_189E20)    # the address, as an argument

   So the source of this function is (in the shape the compiler sees it):

       char *g = D_00189E20;          /* the pointer variable: one "low" */
       ...
       some_function(D_00189E20);     /* the address as a value: a "low" each time */
       ...
       some_function(g + 0x2290);      /* the pointer variable: the offset folded */

   The same shape is in func_L00_002A9600 (0x002BC500), in func_L20_002DEBE0 and
   in 58 other places of level 20 (measured: the pattern
   `daddu $sN, $vM, $zero` followed by `addiu $sK, $sN, %lo(sym)`).

   SPELLINGS MEASURED AND FOUND NOT TO GIVE THE RETAIL'S SHAPE (the compiler
   folds the %lo into the displacement, or materialises the whole address once):
   a 32-bit pointer variable used in one block; the same pointer passed to two
   functions in one block; a volatile pointer (the address is spilled and
   reloaded from the stack); the address in a loop; the address as the only
   argument of one call; a `unsigned long long` local holding the address (a
   conversion, not an address: the compiler emits `la` = lui+addiu, then
   `dsll`/`dsra`); a `long long` local; a pointer to a 64-bit type; a 64-bit
   global holding the address; the address of a 64-bit global; a DI-mode
   pointer typedef (the compiler rejects it as a pointer); the address in a
   struct member; the address in an array element.  What is needed is (a)+(b)
   above: several call sites, in different blocks, with the address as the
   argument.

   4. THE JUMP TABLE'S SPELLING: `jtbl_002B27E0`, NOT `D_L20_002B27E0`.

   The level's own data blob defines the table (build/20_yeedil/asm/data/
   BAC0.data.s, `dlabel jtbl_002B27E0` at 0x002B27E0) and the level's
   config/undefined_syms_auto.txt carries `jtbl_002B27E0 = 0x2B27E0;`, so the
   linker script always has that name.  A `D_L20_002B27E0` in C would only get
   into the linker script through the values the build derives from the retail
   bytes of the functions it PLACES (tools/build.py, Placement.derive and
   undefined_symbols()), so it would not resolve while this function is a near
   miss.  The C below uses `jtbl_002B27E0`, and the object gets R_MIPS_HI16 and
   R_MIPS_LO16 against it (measured with tools/elf.py), which the build
   resolves to 0x002B27E0.

   5. THE MEASUREMENT OF THE SKELETON.

   The C at the end of this file is the skeleton: the prologue, the dispatch
   through the level's own table, the 62 stub bodies, the two shared hubs and
   the epilogue.  tools/check.py func_L20_002CD928 --diff says

       20_yeedil: 92 bytes, expected 34964
       func_L20_002CD928: 0 of 1 places match (src/levels/20_yeedil/L20_002CD928.c)

   What the skeleton proves:

     - THE DISPATCH IS THE RETAIL'S INSTRUCTION FOR INSTRUCTION.  Ours, at
       0x002CD948:

           lui   $v0, %hi(jtbl_002B27E0)
           sll   $v1, $v1, 2
           addiu $v0, $v0, %lo(jtbl_002B27E0)
           addu  $v1, $v1, $v0
           lw    $a0, 0($v1)
           jr    $a0
           nop

       against the retail's, at 0x002CD994: the same seven instructions, the
       same order, the same operands (only the register names of the switch
       value differ, and that is the register allocator's business).  The
       `sltiu $v0,$v1,0x95` and the `beqz` to the tail in front of them are the
       retail's too.

     - THE EPILOGUE'S SHAPE IS RIGHT: ours is
       `lw $v0,0x2298($a1) ; bnel $a3,$v0,... ; sw $zero,0x1C0($a1) ;
        lbu $v0,0x43($a2) ; bnel $t0,$v0,... ; sw $zero,0x1C4($a1)`,
       the retail's two `bnel` tests with the store in the delay slot.

   What it cannot show: the frame.  The retail's frame is 0x1A0 with ten saved
   GPRs (0x120-0x168) and five saved FPRs (0x170-0x190) and about forty float
   spills; the skeleton has no frame at all, because the frame and the register
   allocation are decided by the 62 bodies (8741 instructions), not by the
   dispatch.  The first difference in the diff is therefore the very first
   instruction: `addiu $sp,$sp,-0x1A0` against our `lui $v0,%hi(D_00189E20)`.

   6. THE EXTENDED BODY MAP.

   The map below adds to the one above, for each of the 63 bodies (62 bodies
   plus the default): the number of instructions, the number of calls, the
   offsets the body reads and writes in the boot global D_00189E20 (the ones
   >= 0x2000), the offsets it uses on the objects it drives (the smaller
   ones), and where the body's branches go: into another body (the source's
   `goto`), into one of the two shared hubs, into the tail or into the
   epilogue.


#include "common.h"

/* The attempt belongs in the -G8 unit: func_L20_002CD928 reaches six
   small-data globals through $gp (-0x7C60 = 0x001A7390, -0x7600 = 0x001A79F0
   boot resident; -0x6000 = 0x001A8FF0, 0xF10 = 0x001AF000, 0x3AD8 =
   0x001B2AC8, 0x4748 = 0x001B3738 level 20's own data), so the body belongs in
   L20_002CD928-g8.c and not in L20_002CD928.c, which builds with -G0. */

/* The level's own jump table: 149 words of absolute case addresses, in the
   level's data section at 0x002B27E0.  The level's data blob and the level's
   config/undefined_syms_auto.txt both spell it jtbl_002B27E0, so that is the
   name that links. */
extern void *jtbl_002B27E0[149];

/* The boot's global struct, at 0x00189E20. */
extern char D_00189E20[];

void func_L20_002CD928(void) {
    /* The label array keeps the 62 case bodies alive: cc1 deletes a label
       whose address is never taken.  Its address is never taken, so it costs
       no code, and it lands in the .data the build discards. */
    static void * volatile jt[62] = {
        &&b0, &&b1, &&b2, &&b3, &&b4, &&b5, &&b6, &&b7, &&b8, &&b9, &&b10, &&b11, &&b12, &&b13,
        &&b14, &&b15, &&b16, &&b17, &&b18, &&b19, &&b20, &&b21, &&b22, &&b23, &&b24, &&b25,
        &&b26, &&b27, &&b28, &&b29, &&b30, &&b31, &&b32, &&b33, &&b34, &&b35, &&b36, &&b37,
        &&b38, &&b39, &&b40, &&b41, &&b42, &&b43, &&b44, &&b45, &&b46, &&b47, &&b48, &&b49,
        &&b50, &&b51, &&b52, &&b53, &&b54, &&b55, &&b56, &&b57, &&b58, &&b59, &&b60, &&b61
    };
    char *g = D_00189E20;
    int sel = *(int *)(g + 0x2294);
    char *obj = *(char **)(g + 0x2290);
    int saved = *(int *)(g + 0x2298);
    u8 state = *(u8 *)(obj + 0x43);

    if ((unsigned)sel < 149) {
        goto *jtbl_002B27E0[sel];
    }
    goto tail;

b0:
    /* cases 0 1 3 4 28 29 30 57 109 111 125 -- retail 0x002cd9b0, 1392 bytes */
    goto tail;
b1:
    /* cases 124 -- retail 0x002cdf20, 328 bytes */
    goto tail;
b2:
    /* cases 50 51 52 -- retail 0x002ce068, 3412 bytes */
    goto tail;
b3:
    /* cases 115 -- retail 0x002cedbc, 120 bytes */
    goto tail;
b4:
    /* cases 114 -- retail 0x002cee34, 68 bytes */
    goto tail;
b5:
    /* cases 103 127 -- retail 0x002cee78, 444 bytes */
    goto tail;
b6:
    /* cases 53 54 -- retail 0x002cf034, 712 bytes */
    goto tail;
b7:
    /* cases 133 -- retail 0x002cf2fc, 48 bytes */
    goto tail;
b8:
    /* cases 134 -- retail 0x002cf32c, 380 bytes */
    goto tail;
b9:
    /* cases 47 -- retail 0x002cf4a8, 368 bytes */
    goto tail;
b10:
    /* cases 138 139 -- retail 0x002cf618, 168 bytes */
    goto tail;
b11:
    /* cases 35 -- retail 0x002cf6c0, 256 bytes */
    goto tail;
b12:
    /* cases 36 -- retail 0x002cf7c0, 804 bytes */
    goto tail;
b13:
    /* cases 37 -- retail 0x002cfae4, 236 bytes */
    goto tail;
b14:
    /* cases 43 -- retail 0x002cfbd0, 2032 bytes */
    goto tail;
b15:
    /* cases 20 -- retail 0x002d03c0, 1556 bytes */
    goto tail;
b16:
    /* cases 21 131 -- retail 0x002d09d4, 1100 bytes */
    goto tail;
b17:
    /* cases 19 -- retail 0x002d0e20, 1408 bytes */
    goto tail;
b18:
    /* cases 119 -- retail 0x002d13a0, 336 bytes */
    goto tail;
b19:
    /* cases 31 -- retail 0x002d14f0, 1244 bytes */
    goto tail;
b20:
    /* cases 34 -- retail 0x002d19cc, 236 bytes */
    goto tail;
b21:
    /* cases 2 112 -- retail 0x002d1ab8, 1816 bytes */
    goto tail;
b22:
    /* cases 123 -- retail 0x002d21d0, 840 bytes */
    goto tail;
b23:
    /* cases 46 -- retail 0x002d2518, 536 bytes */
    goto tail;
b24:
    /* cases 48 -- retail 0x002d2730, 868 bytes */
    goto tail;
b25:
    /* cases 39 40 41 42 62 -- retail 0x002d2a94, 3904 bytes */
    goto tail;
b26:
    /* cases 23 24 -- retail 0x002d39d4, 180 bytes */
    goto tail;
b27:
    /* cases 70 71 -- retail 0x002d3a88, 172 bytes */
    goto tail;
b28:
    /* cases 25 26 -- retail 0x002d3b34, 320 bytes */
    goto tail;
b29:
    /* cases 118 -- retail 0x002d3c74, 24 bytes */
    goto tail;
b30:
    /* cases 6 44 -- retail 0x002d3c8c, 292 bytes */
    goto tail;
b31:
    /* cases 116 -- retail 0x002d3db0, 184 bytes */
    goto tail;
b32:
    /* cases 130 -- retail 0x002d3e68, 924 bytes */
    goto tail;
b33:
    /* cases 8 -- retail 0x002d4204, 596 bytes */
    goto tail;
b34:
    /* cases 7 9 10 11 12 13 14 15 16 17 18 27 56 69 72 91 102 110 129 137 146
       -- retail 0x002d4458, 2028 bytes */
    goto tail;
b35:
    /* cases 22 -- retail 0x002d4c44, 180 bytes */
    goto tail;
b36:
    /* cases 140 -- retail 0x002d4cf8, 36 bytes */
    goto tail;
b37:
    /* cases 98 -- retail 0x002d4d1c, 592 bytes */
    goto tail;
b38:
    /* cases 99 -- retail 0x002d4f6c, 164 bytes */
    goto tail;
b39:
    /* cases 60 100 -- retail 0x002d5010, 20 bytes */
    goto tail;
b40:
    /* cases 101 -- retail 0x002d5024, 108 bytes */
    goto tail;
b41:
    /* cases 121 -- retail 0x002d5090, 8 bytes */
    goto tail;
b42:
    /* cases 120 -- retail 0x002d5098, 36 bytes */
    goto tail;
b43:
    /* cases 122 -- retail 0x002d50bc, 36 bytes */
    goto tail;
b44:
    /* cases 63 67 -- retail 0x002d50e0, 152 bytes */
    goto tail;
b45:
    /* cases 77 -- retail 0x002d5178, 688 bytes */
    goto tail;
b46:
    /* cases 75 -- retail 0x002d5428, 148 bytes */
    goto tail;
b47:
    /* cases 78 -- retail 0x002d54bc, 84 bytes */
    goto tail;
b48:
    /* cases 65 -- retail 0x002d5510, 24 bytes */
    goto tail;
b49:
    /* cases 68 -- retail 0x002d5528, 68 bytes */
    goto tail;
b50:
    /* cases 64 76 -- retail 0x002d556c, 688 bytes */
    goto tail;
b51:
    /* cases 66 -- retail 0x002d581c, 108 bytes */
    goto tail;
b52:
    /* cases 73 74 -- retail 0x002d5888, 364 bytes */
    goto tail;
b53:
    /* cases 141 -- retail 0x002d59f4, 188 bytes */
    goto tail;
b54:
    /* cases 145 -- retail 0x002d5ab0, 84 bytes */
    goto tail;
b55:
    /* cases 147 -- retail 0x002d5b04, 232 bytes */
    goto tail;
b56:
    /* cases 148 -- retail 0x002d5bec, 108 bytes */
    goto tail;
b57:
    /* cases 143 -- retail 0x002d5c58, 152 bytes */
    goto tail;
b58:
    /* cases 142 -- retail 0x002d5cf0, 800 bytes; its tail at 0x002d5fe4 is the
       shared hub seven other bodies branch to */
    goto hub1;
b59:
    /* cases 144 -- retail 0x002d6010, 192 bytes; its tail at 0x002d60c0 is the
       shared hub fourteen other bodies branch to */
    goto hub0;
b60:
    /* cases 55 -- retail 0x002d60d0, 120 bytes */
    goto tail;
b61:
    /* cases 96 97 -- retail 0x002d6148, 12 bytes */
    goto tail;

hub1:
    /* 0x002d5fe4: jal func_L00_002D34C0 ; jal func_L00_002C7438 ; b tail */
    goto tail;
hub0:
    /* 0x002d60c0: jal func_L00_002D34C0 ; b tail */
    goto tail;

tail:
    if (saved != *(int *)(g + 0x2298)) {
        *(int *)(g + 0x1C0) = 0;
    }
    if (state != *(u8 *)(obj + 0x43)) {
        *(int *)(g + 0x1C4) = 0;
    }
}
