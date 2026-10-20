/* Near miss from src/levels/20_yeedil/L20_002D6488.c (level 20, yeedil).

   func_L20_002D6488 (2044 bytes) - the level's per-frame update of the state
   block D_00189E20: it clears a row of small fields through f_00301F00 /
   f_00301ED0, walks a list of objects (the table at D_001FBEE0), walks the
   four records at D_00267060, rebuilds the frame list, and clears the flags.

   MEASURED (tools/check.py func_L20_002D6488 --diff, -O2 -G8):

     20_yeedil: 2024 bytes, expected 2044
     func_L20_002D6488: 0 of 1 places match (src/levels/20_yeedil/L20_002D6488.c)

   2024 bytes against the retail's 2044 (20 bytes, five instructions, short).
   Only 9 of the 511 instructions are identical (the frame adjustment, the
   nops, the jr/epilogue).  The first difference is the second instruction of
   the body:

     retail: 002D6488 addiu $sp,$sp,-0x80 / 002D648C lui $v1,0x19
     here:   002D6488 addiu $sp,$sp,-0x80 / 002D648C lui $v0,0x0

   WHAT BLOCKS THE MATCH

   1. THE 64-BIT "la" OF THE GLOBAL.  The retail keeps TWO forms of the
      address of D_00189E20 in callee-saved registers:

        $s0 = the 32-bit address   (lui $v1,%hi(D) / addiu $s0,$v1,%lo(D))
        $s5 = the 64-bit %hi        (daddu $s5,$v1,$zero)

      and every field access through the second form costs two instructions:

        addiu $v1,$s5,%lo(D_189E20) ; lh $v0,0x1454($v1)

      This compiler does not produce that form at all: it materialises the
      address once ($19 = the full address) and accesses it with a 16-bit
      displacement, or, when the accesses are written as direct symbol
      references, it folds the offset into the symbol (lui %hi(D+off) /
      op %lo(D+off)(reg)).  The retail's form is gcc's medlow anchor with a
      64-bit base; the same shape appears in the boot's f_002AB1C8
      (build/01_oozla/asm: lui $v0,%hi(D_189E20) / daddu $s0,$v0,$zero /
      addiu $v0,$v0,%lo(D_189E20)) and in the near miss
      nonmatching/levels/func_L20_002CD928.c, so the compiler can do it -
      the source construct that asks for it is still unknown.  Tried and
      measured: a pointer cast to unsigned long long, a 64-bit pointer type,
      an unsigned long long local, a pointer kept across a call, a volatile
      pointer, and -mno-split-addresses / -mabi=64 / -mips3 / -mcpu=r5900.
      All of them give "la" (lui+addiu) plus dsll/dsra, never daddu+addiu.

   2. THE ANCHOR REGISTER.  When the accesses are direct symbol references
      this compiler anchors the symbol in $fp (the frame pointer: it saves
      and restores $fp but never uses it as a frame pointer) and derives the
      addresses with "addiu $19,$fp,%lo(D)".  The retail has no $fp at all:
      its anchor is $s5, a general callee-saved register.

   3. THE FRAME POINTER.  This compiler allocates a frame pointer for the
      function (it saves $fp at 0x60 and $ra at 0x68); the retail saves $ra at
      0x60 and no $fp, with the two 16-byte locals at 0x00 and 0x10 of the
      frame (the calls take $sp and $sp+0x10).

   4. THE 128-BIT COPIES.  The retail materialises both pointers before the
      copy (addiu $v1,$v0,0x2D0 / addiu $a0,$v0,0xD0 / lq / sq); this
      compiler folds the offset into the lq/sq (ld/sd with a displacement).
      The known escapes (a plain register address, a pointer loaded from
      memory, a static __inline__ copy helper) were tried; a static
      __inline__ helper of two pointer parameters still folds.

   5. THE LOOP SHAPES.  The retail's two loops are rotated (the exit test at
      the bottom, the entry branching over the body), which is the goto form
      (see nonmatching/levels/func_L20_0039D680.c).  The goto form used here
      still lets this compiler duplicate the exit test in front of the object
      loop and to reload the list entry at the top instead of the bottom.

   6. THE UNKNOWN PROTOTYPES.  The 20 callees have no prototype in the
      repository; the ones that decide the register allocation were guessed:
      f_0033438/s32 (the result is tested), f_00302408/float (the result is
      compared with 15.37), f_0032F680/s32 (the result is tested), f_002E7A28
      /float (the result is compared with 0.0), f_00331398(void*,void*,s32,
      void*,void*,float) - the retail passes 0x04000000 in $a2 and 1.0f in
      $f12, so the third argument is an int and the float is the last one.

   THE FILE NEEDS -G8.  The retail body reaches three globals through $gp:
     -0x60F0($gp) = 0x001A8F00 (D_001A8F00, an int, also read absolutely),
      0x39F4($gp) = 0x001B29E4 (D_001B29E4, a float, also read and written
      absolutely),
      0x3AD8($gp) = 0x001B2AC8 (D_001B2AC8, an int, also read absolutely).
   So the body belongs in a -g8.c unit (L20_002D6488-g8.c) and not in
   L20_002D6488.c, which builds with -G0.  Both functions of this file need
   it: func_L20_002D7730 reaches 0x4748($gp) = 0x001B3738, -0x5FFC($gp) =
   0x001A8FF4, -0x7318($gp) = 0x001A7CD8, -0x7314($gp) = 0x001A7CDC and
   -0x7C60($gp) = 0x001A7390.

   THE STRUCTURE, from the retail (build/20_yeedil/asm/5B0B0.s):

     0x002D6488  the prologue: frame 0x80, the saved GPRs at 0x20-0x60 (s0,
                 s1, s2, s3, s4, s5, s6, s7, ra), $f20 at 0x70, the address
                 of D_00189E20 in $s0 (32-bit) and its %hi in $s5 (64-bit)
     0x002D64BC  if (D_00189E20[0x22BD] == 0) f_002CC4D0()
     0x002D64D0  four counters (0x1B8, 0x1BC, 0x1C0, 0x1C4) are incremented
     0x002D6500  34 calls: 33 to f_00301F00 and one to f_00301ED0, each with
                 &D_00189E20[0x1D0 .. 0x226] in $a0 (the delay slot)
     0x002D6610  if ((*(int *)(D_00138180 + 0x1A0) & 4) == 0) D[0x20C] = 0
     0x002D662C  if (D[0x21C] != 0) f_002BA910()
     0x002D6644  if (D[0x22B8] == 0) D[0x1D8] = 0; else D[0x1D8]++
     0x002D6658  D[0x1C8]/D[0x1CC] are incremented and the other one cleared,
                 the choice depending on *(int *)(D_00138180 + 0x1D4)
     0x002D6690  if (D[0x22C3] && !D[0x229C] && D[0x340] && D[0x170] <
                 0.0016666668f && D[0x184] < 0.0016666668f)
                 *(long long *)(D + 0x80) = *(long long *)(D + 0x2030)
     0x002D6700  if (D[0x1454] == 0) D[0x1E8] = 0; else D[0x1E8]++
     0x002D6718  the flag block: D[0x24CC], D_001A8F00, D[0x22B4] == 4,
                 D[0x2294] == 0x6F, D_00138180[0x1A4] & 0x10, then
                 f_L20_002D7730(0x91, 1); then D_001B29E0/D_001B29E4 and the
                 counters at 0x22A and 0x228
     0x002D6824  the object loop over the table at D_001FBEE0 (the entries
                 are 4-byte pointers): f_003343B8(p), the flag at p[0x24]
                 and its 0x46, the D_001B2AC8 test against (p >> 8) & 7,
                 f_0032F680(4), f_00302408(D_00189EA0, p + 0x10) and, if the
                 result is below 15.37, f_00302290, f_00302548 (1.7f) and
                 f_00331398
     0x002D692C  four calls to f_00301ED0 on the records at D_00267060
     0x002D6954  if (D[0x34E] != 0): D[0x24DC] = 9999.0f, f_002E7A28(D+0xF0),
                 and, when D[0x23C8] == 0x20, eight rounds over the array at
                 D + 0x1D90 (the index is (d + 0x1F) & 31), f_00302290,
                 f_00302260, then f_003022D0 (0.125f) and f_00302390, and
                 the counter at D[0x202]
     0x002D6A94  f_002D6C88, then f_002E7608 on D_00189F00 (0.4f for the
                 state 1, 0.7f for the state 4) and the pair at D[0x24A4]
     0x002D6B8C  the pair at D[0xD0]/D[0xE0]/D[0x2D0] and f_002E76A0(0.5f)
     0x002D6BE4  the pointer at D[0x5C0] and the one at D[0x33C], the flag
                 at +0x34 and the state at D[0x1248]
     0x002D6C30  if (D[0x88] >= 1.0f) f_002C61E0()
     0x002D6C58  the epilogue

   The C below is semantically the retail's; it is the starting point for the
   next attempt, not a match.

*/
/* The level's state block, in the boot's resident data at D_00189E20, and the
   second resident block at D_00138180. */

   ------------------------------------------------------------------
   WAVE 3 (worker 14, 20 October 2026): THE "SPLIT 64-BIT la" IS SOLVED
   ------------------------------------------------------------------

   The wave-2 note above called the split 64-bit "la" the wall that stops
   this body.  It is not a wall: the shape is this compiler's own "split
   address" (gcc 2.9's mips_check_split / the movsi expand), and the
   SOURCE SPELLING that produces it is known.  A matched witness is in the
   repository: src/levels/shared/L00_002C2DE8.c, func_L00_002C3838 (27 of 27
   places match), whose retail body is

       lui    $v1, %hi(D_189E20)
       addiu  $s0, $v1, %lo(D_189E20)      <- the plain 32-bit address
       ...
       daddu  $s1, $v1, $zero             <- the 64-bit %hi in a callee-saved reg
       ...
       addiu  $s0, $s1, %lo(D_189E20)      <- the address re-made at each use
       lwc1   $f0, 0x170($s0)

   and whose C is

       char *p = D_00189E20;              /* the plain address, in $s0 */
       ...
           char *q = D_00189E20;          /* a SECOND name for the same address */
           *(short *)(q + 0x20A) = 5;

   THE RULE: a global's address used through ONE C name is materialised once
   ("lui %hi" + "addiu %lo" into one register, the plain 32-bit address); the
   SAME global's address used through a SECOND C name is materialised in the
   split form: the %hi half lives in a callee-saved register (the compiler
   copies it with "daddu $sN, $vN, $zero" - gas's "move") and the %lo half is
   added again at every use ("addiu $vN, $sN, %lo(SYM)").  The field offset
   then rides along as the 16-bit displacement of the load/store, exactly as
   the retail has it.  "Two names for the same pointer" is the same trick the
   compiler notes already record for the missing "daddu"; here it is what
   produces the whole split form.

   In this body the retail keeps both forms of D_00189E20 at once:
     $s0 = the plain address  (the early part: the four counters, the 34
           calls, 0x20C, 0x21C, 0x22B8, 0x1D8)
     $s5 = the %hi half       (everything from 0x1C8/0x1CC onwards)
   and the same for D_00138180 ($s1 = its %hi half) and D_00189EA0
   ($s4 = its %hi half, $s2 = the address re-made from it).

   HOW THE SPLIT FORM IS REACHED IN CODE: gcc 2.9's movsi expand splits a
   move of a plain SYMBOL_REF into HIGH + LO_SUM (mips.md, "movsi"), the
   reload's LEGITIMIZE_ADDRESS does the same for a memory address
   (mips.h), and the "low" insn (mips.md) prints the LO_SUM as
   "addiu %0,%1,%lo(%2)".  The %hi half is a separate pseudo, so it can be
   given a callee-saved register while the LO_SUM is rematerialised at every
   use.  So the split form needs the address to be re-materialised per use
   rather than kept in one register - which is what the second C name makes
   the compiler do.

   MEASURED ON THIS BODY (tools/check.py func_L20_002D6488 --diff, the C
   below, in a -G8 unit):

     20_yeedil: 1916 bytes, expected 2044
     func_L20_002D6488: 0 of 1 places match
     12 of 511 instructions identical (the wave-2 attempt: 9 of 511, 2024
     bytes) - the first difference is still the prologue.

   WHAT STILL STOPS THE MATCH (after the split form is understood):

   1. THE PROLOGUE.  The retail's frame is 0x80: it saves $s0-$s7 and $ra
      (0x20-0x68) and $f20 at 0x70.  The attempt below saves $s0-$s6 and
      $ra and puts $f20 at 0x60, so its frame is 0x70: one callee-saved
      register too few.  The retail's $s6 holds the constant 0x16 (the
      object-kind test in the object loop) and its $s7 holds a copy of the
      state block's address (used for the 0x2290 field), and the compiler
      has to be pushed into using both.
   2. THE REGISTER ORDER.  The retail's order is $s0 the plain address,
      $s1 the %hi of D_00138180, $s2 the address of D_00189EA0, $s3 the
      object table, $s4 the %hi of D_00189EA0, $s5 the %hi of D_00189E20,
      $s6 the constant, $s7 the address copy - which is not the order the
      code creates the values in, so the pseudo numbering (and with it the
      callee-saved assignment) has to be forced from the source.
   3. THE 128-BIT COPIES (the wave-2 wall 4, unchanged): the retail
      materialises both pointers ("addiu $a0,$v1,0x2030" / "addiu $v1,
      $v1,0x80" / "lq" / "sq"), this compiler folds the offset into the
      lq/sq.
   4. THE TWO LOOPS (the wave-2 wall 5, unchanged): the object loop and the
      frame loop are rotated (the test at the bottom, the entry branching
      over the body) and the retail duplicates the table increment into
      three delay slots.
   5. THE UNKNOWN PROTOTYPES (the wave-2 wall 6, unchanged).

   A NOTE ON "addu" vs "addiu": the compiler's register+immediate add is
   printed as "addu" in the .s, but gas turns it into the retail's
   "addiu" when the immediate fits in 16 bits (checked on the matched
   func_L20_0039D7B0: the compiler prints "addu $4,$17,648" and the object
   holds 0x26240288 = "addiu $a0,$s1,0x288").  So the retail's
   "addiu $a0,$s0,0x1E0" is no obstacle.

   THE FILE NEEDS -G8: the body reaches D_001A8F00 (-0x60F0($gp)),
   D_L20_001B29E4 (0x39F4($gp)) and D_L20_001B2AC8 (0x3AD8($gp)) through
   $gp, so the body belongs in a -g8 unit (L20_002D6488-g8.c) and not in
   L20_002D6488.c, which builds with -G0.  The attempt below was compiled
   in such a unit.


/* The C of the first attempt (wave 2) is kept below the new one, for the record.
   ------------------------------------------------------------------ */

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned int u32;
typedef signed int s32;
typedef float f32;

extern char D_00189E20[];
extern char D_00138180[];
extern char D_00189EA0[];
extern char D_00189F00[];
extern char *D_001FBEE0[];
extern char D_00267060[];

extern u8 D_001A7AFF;
extern int D_001A8F00 __attribute__((sda));
extern int D_001A8F00_1 __attribute__((nosda));
extern int D_001B2AC8 __attribute__((sda));
extern float D_001B29E0;
extern float D_001B29E4 __attribute__((sda));
extern float D_001B29E4_1;

extern void func_002CC4D0(void);
extern void func_00301F00(void *);
extern void func_00301ED0(void *);
extern void func_002BA910(void);
extern s32 func_003343B8(void *);
extern s32 func_0032F680(s32);
extern f32 func_00302408(void *, void *);
extern void func_00302290(void *, void *, void *);
extern void func_00302548(void *, void *, f32);
extern void func_00331398(void *, void *, s32, void *, void *, f32);
extern f32 func_002E7A28(void *);
extern void func_00302228(void *);
extern void func_00302260(void *, void *, void *);
extern void func_003022D0(void *, void *, f32);
extern f32 func_00302390(void *);
extern void func_002D6C88(void);
extern void func_002E7608(void *, s32, f32, f32);
extern void func_002E76A0(f32, void *);
extern void func_002C61E0(void);

void func_L20_002D6488(void)
{
    char *s0 = D_00189E20;
    char *s1 = D_00138180;
    char *s2 = D_00189EA0;
    char *s3;
    char *p;
    s32 i;
    s32 v0;
    f32 buf[8];
    f32 f1v;
    f32 f20v = 15.37f;

    if (s0[0x22BD] == 0) {
        func_002CC4D0();
    }

    *(s32 *)(D_00189E20 + 0x1B8) += 1;
    *(s32 *)(D_00189E20 + 0x1BC) += 1;
    *(s32 *)(D_00189E20 + 0x1C0) += 1;
    *(s32 *)(D_00189E20 + 0x1C4) += 1;

    func_00301F00(s0 + 0x1E0);
    func_00301F00(s0 + 0x1DC);
    func_00301F00(s0 + 0x1D2);
    func_00301F00(s0 + 0x1E4);
    func_00301F00(s0 + 0x1F8);
    func_00301F00(s0 + 0x1FE);
    func_00301ED0(s0 + 0x1D4);
    func_00301F00(s0 + 0x210);
    func_00301F00(s0 + 0x1D0);
    func_00301F00(s0 + 0x1F4);
    func_00301F00(s0 + 0x1F6);
    func_00301F00(s0 + 0x204);
    func_00301F00(s0 + 0x206);
    func_00301F00(s0 + 0x1F0);
    func_00301F00(s0 + 0x1EC);
    func_00301F00(s0 + 0x1EE);
    func_00301F00(s0 + 0x208);
    func_00301F00(s0 + 0x200);
    func_00301F00(s0 + 0x20E);
    func_00301F00(s0 + 0x20A);
    func_00301F00(s0 + 0x214);
    func_00301F00(s0 + 0x212);
    func_00301F00(s0 + 0x1FC);
    func_00301F00(s0 + 0x216);
    func_00301F00(s0 + 0x1FA);
    func_00301F00(s0 + 0x218);
    func_00301F00(s0 + 0x1F2);
    func_00301F00(s0 + 0x21A);
    func_00301F00(s0 + 0x21C);
    func_00301F00(s0 + 0x1DE);
    func_00301F00(s0 + 0x1E6);
    func_00301F00(s0 + 0x222);
    func_00301F00(s0 + 0x224);
    func_00301F00(s0 + 0x226);

    if (!(*(s32 *)(s1 + 0x1A0) & 4)) {
        *(u16 *)(D_00189E20 + 0x20C) = 0;
    }

    if (*(s16 *)(D_00189E20 + 0x21C) != 0) {
        func_002BA910();
    }

    if (s0[0x22B8] == 0) {
        *(s32 *)(D_00189E20 + 0x1D8) = 0;
    } else {
        *(s32 *)(D_00189E20 + 0x1D8) += 1;
    }

    if (*(s32 *)(s1 + 0x1D4) == 0) {
        *(s32 *)(D_00189E20 + 0x1C8) += 1;
        *(s32 *)(D_00189E20 + 0x1CC) = 0;
    } else {
        *(s32 *)(D_00189E20 + 0x1CC) += 1;
        *(s32 *)(D_00189E20 + 0x1C8) = 0;
    }

    if (s0[0x22C3] != 0 && *(s32 *)(D_00189E20 + 0x229C) == 0 && *(s32 *)(D_00189E20 + 0x340) != 0) {
        if (*(f32 *)(D_00189E20 + 0x170) < 0.0016666668f) {
            if (*(f32 *)(D_00189E20 + 0x184) < 0.0016666668f) {
                *(long long *)(D_00189E20 + 0x80) = *(long long *)(D_00189E20 + 0x2030);
            }
        }
    }

    if (*(s16 *)(D_00189E20 + 0x1454) == 0) {
        *(s32 *)(D_00189E20 + 0x1E8) = 0;
    } else {
        *(s32 *)(D_00189E20 + 0x1E8) += 1;
    }

    if (*(s32 *)(D_00189E20 + 0x24CC) != 0) {
        if (D_001A8F00 != 0) {
            goto after_flag;
        }
        if (s0[0x22B4] == 4) {
            if (*(s32 *)(D_00189E20 + 0x2294) == 0x6F) {
                goto load_flag;
            }
            if (!(*(s32 *)(s1 + 0x1A4) & 0x10)) {
                goto load_flag;
            }
            *(s32 *)(D_00189E20 + 0x24CC) = 0;
            *(u16 *)(D_00189E20 + 0x1E0) = 0;
            func_L20_002D7730(0x91, 1);
        }
        goto check_flag;
    }
load_flag:
    v0 = D_001A8F00_1;
check_flag:
    if (v0 != 0) {
        goto after_flag;
    }
    if (0.0f < D_001B29E0) {
        *(u16 *)(D_00189E20 + 0x22A) += 1;
    } else {
        *(u16 *)(D_00189E20 + 0x22A) = 0;
    }
    f1v = D_001B29E4;
    if (*(s16 *)(D_00189E20 + 0x22A) >= 0x79) {
        *(int *)&D_001B29E0 = 0;
        *(u16 *)(D_00189E20 + 0x22A) = 0;
        f1v = D_001B29E4_1;
    }
    if (0.0f < f1v) {
        *(u16 *)(D_00189E20 + 0x228) += 1;
    } else {
        *(u16 *)(D_00189E20 + 0x228) = 0;
    }
    if (*(s16 *)(D_00189E20 + 0x228) >= 0x79) {
        *(u16 *)(D_00189E20 + 0x228) = 0;
        *(int *)&D_001B29E4 = 0;
    }
after_flag:

    if (*(s16 *)(D_00189E20 + 0x224) == 0) {
        goto end_objects;
    }

    s3 = (char *)D_001FBEE0;
    p = *(char **)s3;
    if (p == 0) {
        goto end_objects;
    }
object_loop:
    if (func_003343B8(p) == 0) {
        if (*(s32 *)(p + 0x24) != 0) {
            if (*(s16 *)(*(s32 *)(p + 0x24) + 0x46) == 0x16) {
                goto have_value;
            }
        }
        goto next;
    }
have_value:
    if ((D_001B2AC8 & 7) != (((s32)p >> 8) & 7)) {
        if (func_0032F680(4) != 0) {
            goto next;
        }
    }
    {
        f32 r = func_00302408(s2, p + 0x10);

        if (r < f20v) {
            func_00302290(buf, p + 0x10, s2);
            func_00302548(buf, buf, 1.7f);
            func_00331398(p, *(void **)(D_00189E20 + 0x2290), 0x04000000, s2, buf, 1.0f);
        }
    }
next:
    s3 += 4;
    p = *(char **)s3;
    if (p != 0) {
        goto object_loop;
    }
end_objects:

    for (s3 = D_00267060; s3 < D_00267060 + 0x1C0; s3 += 0x70) {
        func_00301ED0(s3);
    }

    s0 = D_00189E20;
    if (*(s16 *)(D_00189E20 + 0x34E) == 0) {
        goto no_frames;
    }
    *(f32 *)(D_00189E20 + 0x24DC) = 9999.0f;
    {
        f32 r = func_002E7A28(D_00189E20 + 0xF0);

        if (r < 0.0f) {
            goto no_frames;
        }
    }
    if (*(s32 *)(D_00189E20 + 0x23C8) == 0x20) {
        s32 x;
        char *arr = s0 + 0x1D90;

        func_00302228(buf);
        i = 0;
        x = *(s32 *)(D_00189E20 + 0x23C4);
frame_loop:
        {
            s32 d;
            s32 a;
            s32 b;
            s32 c;
            s32 e;
            s32 p1;
            s32 p2;

            d = x - i;
            a = d + 0x1F;
            b = d + 0x1E;
            c = d + 0x3D;
            e = d + 0x3E;
            if (b < 0) {
                c = b;
            }
            if (a < 0) {
                e = a;
            }
            p1 = a - ((e >> 5) << 5);
            p2 = b - ((c >> 5) << 5);
            func_00302290(buf + 4, arr + (p1 << 4), arr + (p2 << 4));
            func_00302260(buf, buf, buf + 4);
        }
        i++;
        x = *(s32 *)(D_00189E20 + 0x23C4);
        if (i < 8) {
            goto frame_loop;
        }
        func_003022D0(buf, buf, 0.125f);
        {
            f32 r = func_00302390(buf);

            if (r < 0.016666668f) {
                *(u16 *)(D_00189E20 + 0x202) += 1;
            } else {
                *(u16 *)(D_00189E20 + 0x202) = 0;
            }
        }
    }
no_frames:

    func_002D6C88();

    if (s0[0x22B4] == 1) {
        func_002E7608(D_00189F00 + 0xE0, 1, 0.0f, 0.4f);
    } else if (s0[0x22B4] == 4) {
        func_002E7608(D_00189F00, 1, 0.0f, 0.7f);
    }

    if ((D_001B2AC8 & 3) == 0) {
        if (s0[0x22B4] == 1) {
            *(f32 *)(D_00189E20 + 0x24A4) = 2.125f;
            *(f32 *)(D_00189E20 + 0x24A8) = 1.25f;
        } else if (s0[0x22B4] == 4) {
            if (D_001A7AFF) {
                *(f32 *)(D_00189E20 + 0x24A4) = 14.0f;
                *(f32 *)(D_00189E20 + 0x24A8) = 4.5f;
            } else {
                *(f32 *)(D_00189E20 + 0x24A4) = 4.25f;
                *(f32 *)(D_00189E20 + 0x24A8) = 2.25f;
            }
        }
    }

    {
        f32 v = *(f32 *)(D_00189E20 + 0x31C);

        if (v < 4.0f) {
            *(long long *)(D_00189E20 + 0xD0) = *(long long *)(D_00189E20 + 0x2D0);
            func_002E76A0(0.5f, s0 + 0xD0);
        } else {
            *(long long *)(D_00189E20 + 0xD0) = *(long long *)(D_00189E20 + 0xE0);
        }
    }

    {
        s32 ptr = *(s32 *)(D_00189E20 + 0x5C0);

        if (ptr != 0) {
            if (!(*(u16 *)(ptr + 0x34) & 0x1000)) {
                *(s32 *)(D_00189E20 + 0x5C0) = 0;
            } else if (ptr == *(s32 *)(D_00189E20 + 0x33C)) {
                *(s32 *)(D_00189E20 + 0x5C0) = 0;
            }
        }
    }
    if (*(s32 *)(D_00189E20 + 0x5C0) != 0) {
        if (*(s32 *)(D_00189E20 + 0x1248) == 0x2E) {
            *(s32 *)(D_00189E20 + 0x5C0) = 0;
        }
    }

    if (!(*(f32 *)(D_00189E20 + 0x88) < 1.0f)) {
        func_002C61E0();
    }
}

/* ------------------------------------------------------------------
   The wave-3 attempt: the same body with the split-address form
   (a second C name for every global the retail re-materialises) and the
   -G8 unit it needs.
   ------------------------------------------------------------------ */

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned int u32;
typedef signed int s32;
typedef float f32;

#ifndef RAC2_T_V4_FBFDC087A
#define RAC2_T_V4_FBFDC087A
typedef int V4_Fbfdc087a __attribute__((mode(TI)));
#endif

/* the level's state block and the second resident block */
extern char D_00189E20[];
extern char D_00138180[];
extern char D_00189EA0[];
extern char D_00189F00[];

/* the level's own data */
extern char *D_L20_001FBEE0[];
extern char D_L20_00267060[];

/* 0x001A7AFF: read absolutely. */
extern u8 D_001A7AFF;

/* 0x001A8F00: read through $gp (-0x60F0) and absolutely. */
extern int D_001A8F00 __attribute__((sda));
extern int D_001A8F00_1 __attribute__((nosda));

/* 0x001B2AC8: read through $gp (0x3AD8) and absolutely. */
extern int D_L20_001B2AC8 __attribute__((sda));
extern int D_L20_001B2AC8_1 __attribute__((nosda));

/* 0x001B29E0: read and written absolutely. */
extern float D_L20_001B29E0;

/* 0x001B29E4: read through $gp (0x39F4) and written absolutely. */
extern float D_L20_001B29E4 __attribute__((sda));
extern float D_L20_001B29E4_1 __attribute__((nosda));

extern void func_002CC4D0(void);
extern void func_00301F00(void *);
extern void func_00301ED0(void *);
extern void func_002BA910(void);
extern s32 func_003343B8(void *);
extern s32 func_0032F680(s32);
extern f32 func_00302408(void *, void *);
extern void func_00302290(void *, void *, void *);
extern void func_00302548(void *, void *, f32);
extern void func_00331398(void *, void *, s32, void *, void *, f32);
extern f32 func_002E7A28(void *);
extern void func_00302228(void *);
extern void func_00302260(void *, void *, void *);
extern void func_003022D0(void *, void *, f32);
extern f32 func_00302390(void *);
extern void func_002D6C88(void);
extern void func_002E7608(void *, s32, f32, f32);
extern void func_002E76A0(f32, void *);
extern void func_002C61E0(void);
extern void func_L20_002D7730(s32, s32);

void func_L20_002D6488(void)
{
    char *p = D_00189E20;
    char *r = D_00138180;
    char *s = D_00189EA0;
    char **u;
    char *s2 = D_00189EA0;
    char *q = D_00189E20;
    s16 kind = 0x16;
    char *w;
    char *t;
    s32 i;
    s32 n;
    s32 x;
    f32 buf[4];
    f32 buf2[4];
    f32 f20v = 15.37f;
    f32 f1v;

    if (p[0x22BD] == 0) {
        func_002CC4D0();
    }

    *(s32 *)(p + 0x1B8) += 1;
    *(s32 *)(p + 0x1BC) += 1;
    *(s32 *)(p + 0x1C0) += 1;
    *(s32 *)(p + 0x1C4) += 1;

    func_00301F00(p + 0x1E0);
    func_00301F00(p + 0x1DC);
    func_00301F00(p + 0x1D2);
    func_00301F00(p + 0x1E4);
    func_00301F00(p + 0x1F8);
    func_00301F00(p + 0x1FE);
    func_00301ED0(p + 0x1D4);
    func_00301F00(p + 0x210);
    func_00301F00(p + 0x1D0);
    func_00301F00(p + 0x1F4);
    func_00301F00(p + 0x1F6);
    func_00301F00(p + 0x204);
    func_00301F00(p + 0x206);
    func_00301F00(p + 0x1F0);
    func_00301F00(p + 0x1EC);
    func_00301F00(p + 0x1EE);
    func_00301F00(p + 0x208);
    func_00301F00(p + 0x200);
    func_00301F00(p + 0x20E);
    func_00301F00(p + 0x20A);
    func_00301F00(p + 0x214);
    func_00301F00(p + 0x212);
    func_00301F00(p + 0x1FC);
    func_00301F00(p + 0x216);
    func_00301F00(p + 0x1FA);
    func_00301F00(p + 0x218);
    func_00301F00(p + 0x1F2);
    func_00301F00(p + 0x21A);
    func_00301F00(p + 0x21C);
    func_00301F00(p + 0x1DE);
    func_00301F00(p + 0x1E6);
    func_00301F00(p + 0x222);
    func_00301F00(p + 0x224);
    func_00301F00(p + 0x226);

    if (!(*(s32 *)(r + 0x1A0) & 4)) {
        *(u16 *)(p + 0x20C) = 0;
    }

    if (*(s16 *)(p + 0x21C) != 0) {
        func_002BA910();
    }

    if (p[0x22B8] == 0) {
        *(s32 *)(p + 0x1D8) = 0;
    } else {
        *(s32 *)(p + 0x1D8) += 1;
    }

    if (*(s32 *)(r + 0x1D4) == 0) {
        n = *(s32 *)(q + 0x1CC);
        *(s32 *)(q + 0x1C8) = 0;
        *(s32 *)(q + 0x1CC) = n + 1;
    } else {
        n = *(s32 *)(q + 0x1C8);
        *(s32 *)(q + 0x1CC) = 0;
        *(s32 *)(q + 0x1C8) = n + 1;
    }

    if (q[0x22C3] != 0 && *(s32 *)(q + 0x229C) == 0 && *(s32 *)(q + 0x340) != 0) {
        if (*(f32 *)(q + 0x170) < 0.0016666668f) {
            if (*(f32 *)(q + 0x184) < 0.0016666668f) {
                *(V4_Fbfdc087a *)(q + 0x80) = *(V4_Fbfdc087a *)(q + 0x2030);
            }
        }
    }

    if (*(s16 *)(q + 0x1454) == 0) {
        *(s32 *)(q + 0x1E8) = 0;
    } else {
        *(s32 *)(q + 0x1E8) += 1;
    }

    if (*(s32 *)(q + 0x24CC) != 0) {
        if (D_001A8F00 != 0) {
            goto after_flag;
        }
        if (q[0x22B4] == 4) {
            if (*(s32 *)(q + 0x2294) == 0x6F) {
                goto load_flag;
            }
            if (!(*(s32 *)(r + 0x1A4) & 0x10)) {
                goto load_flag;
            }
            *(s32 *)(q + 0x24CC) = 0;
            *(u16 *)(q + 0x1E0) = 0;
            func_L20_002D7730(0x91, 1);
        }
        goto check_flag;
    }
load_flag:
    n = D_001A8F00_1;
check_flag:
    if (n != 0) {
        goto after_flag;
    }
    if (0.0f < D_L20_001B29E0) {
        *(u16 *)(q + 0x22A) += 1;
    } else {
        *(u16 *)(q + 0x22A) = 0;
    }
    f1v = D_L20_001B29E4;
    if (*(s16 *)(q + 0x22A) >= 0x79) {
        D_L20_001B29E0 = 0;
        *(u16 *)(q + 0x22A) = 0;
        f1v = D_L20_001B29E4_1;
    }
    if (0.0f < f1v) {
        *(u16 *)(q + 0x228) += 1;
    } else {
        *(u16 *)(q + 0x228) = 0;
    }
    if (*(s16 *)(q + 0x228) >= 0x79) {
        *(u16 *)(q + 0x228) = 0;
        D_L20_001B29E4_1 = 0;
    }
after_flag:

    if (*(s16 *)(q + 0x224) == 0) {
        goto end_objects;
    }

    w = q;
    u = D_L20_001FBEE0;
    t = *u;
    if (t == 0) {
        goto end_objects;
    }
object_loop:
    if (func_003343B8(t) == 0) {
        if (*(s32 *)(t + 0x24) != 0) {
            if (*(s16 *)(*(s32 *)(t + 0x24) + 0x46) == kind) {
                goto have_value;
            }
        }
        goto next;
    }
have_value:
    if ((D_L20_001B2AC8 & 7) != (((s32)t >> 8) & 7)) {
        if (func_0032F680(4) != 0) {
            goto next;
        }
    }
    {
        f32 f = func_00302408(s2, t + 0x10);

        if (f < f20v) {
            func_00302290(buf, t + 0x10, s2);
            func_00302548(buf, buf, 1.7f);
            func_00331398(t, *(void **)(w + 0x2290), 0x04000000, s2, buf, 1.0f);
        }
    }
next:
    u += 4;
    t = *u;
    if (t != 0) {
        goto object_loop;
    }
end_objects:

    for (w = D_L20_00267060; w < D_L20_00267060 + 0x1C0; w += 0x70) {
        func_00301ED0(w);
    }

    if (*(s16 *)(q + 0x34E) == 0) {
        goto no_frames;
    }
    *(f32 *)(q + 0x24DC) = 9999.0f;
    {
        f32 f = func_002E7A28(q + 0xF0);

        if (f < 0.0f) {
            goto no_frames;
        }
    }
    if (*(s32 *)(q + 0x23C8) == 0x20) {
        func_00302228(buf);
        i = 0;
        x = *(s32 *)(q + 0x23C4);
frame_loop:
        {
            s32 d;
            s32 a;
            s32 b;
            s32 c;
            s32 e;
            s32 p1;
            s32 p2;

            d = x - i;
            a = d + 0x1F;
            b = d + 0x1E;
            c = d + 0x3D;
            e = d + 0x3E;
            if (b < 0) {
                c = b;
            }
            if (a < 0) {
                e = a;
            }
            p1 = a - ((e >> 5) << 5);
            p2 = b - ((c >> 5) << 5);
            func_00302290(buf2, q + 0x1D90 + (p1 << 4), q + 0x1D90 + (p2 << 4));
            func_00302260(buf, buf, buf2);
        }
        i++;
        x = *(s32 *)(q + 0x23C4);
        if (i < 8) {
            goto frame_loop;
        }
        func_003022D0(buf, buf, 0.125f);
        {
            f32 f = func_00302390(buf);

            if (f < 0.016666668f) {
                *(u16 *)(q + 0x202) += 1;
            } else {
                *(u16 *)(q + 0x202) = 0;
            }
        }
    }
no_frames:

    func_002D6C88();

    if (q[0x22B4] == 1) {
        func_002E7608(D_00189F00 + 0xE0, 1, 0.0f, 0.4f);
    } else if (q[0x22B4] == 4) {
        func_002E7608(D_00189F00, 1, 0.0f, 0.7f);
    }

    if ((D_L20_001B2AC8 & 3) == 0) {
        if (q[0x22B4] == 1) {
            *(f32 *)(q + 0x24A4) = 2.125f;
            *(f32 *)(q + 0x24A8) = 1.25f;
        } else if (q[0x22B4] == 4) {
            if (D_001A7AFF) {
                *(f32 *)(q + 0x24A4) = 14.0f;
                *(f32 *)(q + 0x24A8) = 4.5f;
            } else {
                *(f32 *)(q + 0x24A4) = 4.25f;
                *(f32 *)(q + 0x24A8) = 2.25f;
            }
        }
    }

    {
        f32 f = *(f32 *)(q + 0x31C);

        if (f < 4.0f) {
            *(V4_Fbfdc087a *)(q + 0xD0) = *(V4_Fbfdc087a *)(q + 0x2D0);
            func_002E76A0(0.5f, q + 0xD0);
        } else {
            *(V4_Fbfdc087a *)(q + 0xD0) = *(V4_Fbfdc087a *)(q + 0xE0);
        }
    }

    {
        s32 ptr = *(s32 *)(q + 0x5C0);

        if (ptr != 0) {
            if (!(*(u16 *)(ptr + 0x34) & 0x1000)) {
                *(s32 *)(q + 0x5C0) = 0;
            } else if (ptr == *(s32 *)(q + 0x33C)) {
                *(s32 *)(q + 0x5C0) = 0;
            }
        }
    }
    if (*(s32 *)(q + 0x5C0) != 0) {
        if (*(s32 *)(q + 0x1248) == 0x2E) {
            *(s32 *)(q + 0x5C0) = 0;
        }
    }

    if (!(*(f32 *)(q + 0x88) < 1.0f)) {
        func_002C61E0();
    }
}
