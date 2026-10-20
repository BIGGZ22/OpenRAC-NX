/* Near miss from src/levels/20_yeedil/L20_002B7A48.c: the build refuses this body at its
   place — 180 bytes, expected 192. Kept as a starting point;
   not linked, not counted. */

/* The retail body is the "which screen does the front end hand back" lookup of
   D_00189E20: the state byte at 0x22B4 answers 0 (state 1) or 0xD (state 4),
   otherwise the resident table entry of the current selection is looked up and
   the 0x24CC/0x1492 fields of the selection decide between 0x51, 0x67 and 0.

   The C below is semantically the retail's.  It compiles to 180 bytes
   (45 instructions) against the retail's 192 (48).  The whole tail, from the
   "lw $v0,0x24CC($a0)" on, matches instruction for instruction; only the first
   14 instructions differ:

     1. the retail keeps the address of D_00189E20 in the lui register ("lui
        $v0,%hi" then "addiu $v0,$v0,%lo") and re-materialises the 64 bit form
        for the later loads ("addiu $v1,$a2,%lo", $a2 = "daddu $a2,$v0,$zero");
        this build copies the address into $a1 once and uses it for every load;
     2. the retail compares the state byte with 1 first ("beq $a1,$v1") and
        keeps the "return 0" as a separate two instruction block between the
        two tests ("b" + "daddu $v0,$zero,$zero", reached by the branch);
        this build inverts the first test ("bne") and merges that block into
        the fall-through;
     3. the retail's prologue starts with the lui of D_00189E20, this build
        starts with the stack adjustment.

   Tried (tools/check.py --diff on each):
     - "if (state == 1) ... else if (state == 4)" instead of two ifs: gives the
       retail's branch targets (+5) but then the compiler emits a branch-likely
       ("beql") for the state == 4 test and duplicates that test;
     - a switch on the state byte: 188 bytes, the byte lands in $v1 instead of
       $a1 and the constant 1 in $v0;
     - "a0 == 0 || ..." and two separate "a0 = D_00189E20" assignments: the
       branch-likely form appears and the body grows to 184/196 bytes;
     - the state byte in a local variable, the two tests in the other order,
       the common block written with "== 1" instead of "!= 1": no change, or
       worse.

   The sibling func_L18_002C2D48 (level 18) is the same function without the
   "state == 1" test and is matched; its C is the shape used here.  The
   function at 0x2C0C30 is the shared func_L00_002ADEA0, and 0x139568 is
   resident boot data, so it is D_00139568 (not D_L20_00139568). */

/* ---- second attempt (level-20 wave, 2026-10-20) ---------------------------------------
 * The state byte is read into a local and dispatched with a `switch` instead of two
 * `if`s, and the "return 0x51 / 0x67" tail is unchanged.  Measured: still 180 bytes
 * against the retail's 192, and the same 14 instructions differ at the top -- the
 * `switch` does not restore retail's forward `beq` for the state == 1 test, and the
 * branch-likely form appears for the state == 4 test.  Kept as the starting point;
 * not linked, not counted.
 */

/* ---- third attempt (level-20 wave, worker w20-12) ------------------------------------
 * What the wave measured about the retail's head, with the compiler's own assembly
 * (build/compiler's cc1 -O2, "lui $2,%hi(D)" / "addiu $5,$2,%lo(D)" / "move $6,$2"):
 *
 *   - the retail materialises D_00189E20 TWICE out of ONE "lui":
 *       lui   $v0, %hi(D_189E20)      # the hi
 *       daddu $a2, $v0, $zero         # the sign-extended hi, for the "hi + %lo" form
 *       addiu $v0, $v0, %lo(D_189E20) # the address in the lui register itself
 *     The "lui + %lo in one register" form is what cc1 gives an ADDRESS use of the
 *     symbol (a pointer variable, a memory base); the "sign-extended hi + %lo at each
 *     use" form is what it gives a VALUE use of it (the assignment "a0 = D_00189E20"
 *     in the matched level-18 twin func_L18_002C2D48, whose cc1 output is
 *       lui $2,%hi(D) ; addiu $5,$2,%lo(D) ; move $6,$2
 *     and whose retail is
 *       lui $v0,%hi(D) ; addiu $a1,$v0,%lo(D) ; daddu $a2,$v0,$zero).
 *     So the retail's level-20 body uses the symbol as an ADDRESS for the state byte
 *     at 0x22B4 and as a VALUE for the fields at 0x22BA and 0x24EC - the two forms
 *     must come from two different source expressions.  A "long" (64-bit) base variable
 *     ("long base = (long)D_00189E20") makes cc1 keep the value form, but it then
 *     sign extends the narrowed value with "dsll32/dsra32" where the retail has none.
 *   - the retail's "lui" is the FIRST instruction of the function (before the frame),
 *     the level-18 twin's comes after it.
 *   - the retail's first test is a plain "beq" to a two-instruction block that ends in
 *     "b .L002B7AFC" + "daddu $v0,$zero,$zero" (the return 0), the level-18 twin has
 *     no such block.  A switch on the state byte keeps the "beq" but puts the case
 *     block far away and duplicates the jump over it (188 bytes); two ifs invert the
 *     first test (180 bytes); a `goto` to a shared label keeps the forward `beq` but
 *     moves the block to the end of the function (180 bytes).
 *
 * The attempt below is the level-18 shape with the extra "state == 1" test: 180 bytes
 * against the retail's 192, with the whole tail from the "jal f_002C0C30" on equal to
 * the retail's instruction for instruction.  What is left is the head: the order of
 * the two materialisations of D_00189E20, the register the address lands in, and the
 * branch shape of the two state tests.
 *
 * The alternative below it makes the body the retail's SIZE (192 bytes) with the
 * 64-bit base; the tail still differs (cc1 then sign extends the narrowed value with
 * "dsll32/dsra32" where the retail has none).
 */

/* Standalone declarations recovered from src/levels/20_yeedil/L20_002B7A48.c. */
typedef unsigned char      u8;

typedef signed short       s16;

typedef signed int         s32;

extern char D_00189E20[];

extern u8 D_00139568[];

extern char D_L20_0026D970[];

extern s32 func_L00_002ADEA0(s32 index);

typedef struct {
    u8 pad[0x28];
    s32 v;
} Entry;

s32 func_L20_002B7A48_attempt1(char *a0)
{
    char *s0 = D_00189E20;

    if (*(u8 *)(s0 + 0x22B4) == 1)
        return 0;
    if (*(u8 *)(s0 + 0x22B4) == 4)
        return 0xD;
    if (a0 != 0 && *(u8 *)(s0 + 0x22BA) != 0 && *(s16 *)(s0 + 0x24EC) == 0) {
        s32 idx = func_L00_002ADEA0(0);
        return ((Entry *)(D_L20_0026D970 + D_00139568[idx] * 0xE0))->v;
    }
    a0 = D_00189E20;
    if (*(s32 *)(a0 + 0x24CC) != 1)
        return *(u8 *)(a0 + 0x1492) ? 0x67 : 0;
    return 0x51;
}

s32 func_L20_002B7A48_attempt2(char *a0)
{
    char *s0 = D_00189E20;
    u8 state = *(u8 *)(s0 + 0x22B4);

    switch (state) {
    case 1:
        return 0;
    case 4:
        return 0xD;
    }
    if (a0 != 0 && *(u8 *)(s0 + 0x22BA) != 0 && *(s16 *)(s0 + 0x24EC) == 0) {
        s32 idx = func_L00_002ADEA0(0);
        return ((Entry *)(D_L20_0026D970 + D_00139568[idx] * 0xE0))->v;
    }
    a0 = D_00189E20;
    if (*(s32 *)(a0 + 0x24CC) != 1)
        return *(u8 *)(a0 + 0x1492) ? 0x67 : 0;
    return 0x51;
}

s32 func_L20_002B7A48_attempt3(char *a0)
{
    char *s0 = D_00189E20;
    u8 state = *(u8 *)(s0 + 0x22B4);

    if (state == 1)
        return 0;
    if (state == 4)
        return 0xD;
    if (a0 != 0 && *(u8 *)(s0 + 0x22BA) != 0 && *(s16 *)(s0 + 0x24EC) == 0) {
        s32 idx = func_L00_002ADEA0(0);
        return ((Entry *)(D_L20_0026D970 + D_00139568[idx] * 0xE0))->v;
    }
    a0 = D_00189E20;
    if (*(s32 *)(a0 + 0x24CC) != 1)
        return *(u8 *)(a0 + 0x1492) ? 0x67 : 0;
    return 0x51;
}

s32 func_L20_002B7A48_longbase(char *a0)
{
    long base = (long)D_00189E20;
    u8 state = D_00189E20[0x22B4];

    if (state == 1)
        return 0;
    if (state == 4)
        return 0xD;
    if (a0 != 0 && *(u8 *)((char *)base + 0x22BA) != 0 && *(s16 *)((char *)base + 0x24EC) == 0) {
        s32 idx = func_L00_002ADEA0(0);
        return ((Entry *)(D_L20_0026D970 + D_00139568[idx] * 0xE0))->v;
    }
    a0 = (char *)base;
    if (*(s32 *)(a0 + 0x24CC) != 1)
        return *(u8 *)(a0 + 0x1492) ? 0x67 : 0;
    return 0x51;
}

/* ---- fourth attempt (level-20 wave 3, worker w21-11, 2026-10-20) ----------------------
 * What this worker measured about the retail's head (the retail read out of
 * build/reference/20_yeedil.elf at 0x002B7A48, 48 instructions):
 *
 *   002B7A48  lui        $v0, %hi(D_189E20)
 *   002B7A4C  addiu      $sp, $sp, -0x10
 *   002B7A50  sd         $ra, 0x0($sp)
 *   002B7A54  daddu      $a2, $v0, $zero        <- the %hi copied out of the lui
 *   002B7A58  addiu      $v0, $v0, %lo(D_189E20) <- the address IN the lui register
 *   002B7A5C  addiu      $v1, $zero, 0x1
 *   002B7A60  lbu        $a1, 0x22B4($v0)        <- the state byte
 *   002B7A64  beq        $a1, $v1, .L7A7C       <- "state == 1" is a FORWARD branch
 *   002B7A68    addiu     $v0, $zero, 0x4
 *   002B7A6C  beq        $a1, $v0, .L7AFC       <- "state == 4"
 *   002B7A70    addiu     $v0, $zero, 0xD
 *   002B7A74  b          .L7A84                 <- and the common block follows
 *   002B7A78    nop
 *   002B7A7C  b          .L7AFC                 <- the "return 0" block, out of line
 *   002B7A80    daddu     $v0, $zero, $zero
 *   002B7A84  beqz       $a0, .L7AD8
 *   002B7A88    addiu     $v1, $a2, %lo(D_189E20) <- the fields' base, the %lo per use
 *   002B7A8C  lbu        $v0, 0x22BA($v1)
 *   002B7A90  beqz       $v0, .L7ADC
 *   002B7A94    addiu     $a0, $a2, %lo(D_189E20) <- "a0 = D", the %lo per use
 *   002B7A98  lh         $v0, 0x24EC($v1)
 *   002B7A9C  bnez       $v0, .L7AE0
 *   002B7AA0    addiu     $v1, $zero, 0x1
 *
 * So the retail keeps THREE materialisations of D_00189E20 out of ONE lui:
 * the address in the lui register itself (the state byte), the %hi copied into
 * $a2 with the %lo added at each use (the fields' base and the "a0 = D"), and
 * the "a0 = D" a second time on the a0 == 0 path.
 *
 * Measured with this compiler (build/compiler/tools/cc1 -O2 -G0):
 *
 *  - a POINTER to the symbol used as a memory base is materialised
 *      "lui $h,%hi(S)" + "addiu $b,$h,%lo(S)"  (the hi in one register, the
 *    address in another) - the "F1" form; the same pointer used once and
 *    rematerialised into the load's base register gives the retail's "F2" form
 *    "lui $h,%hi(S)" + "addiu $h,$h,%lo(S)";
 *  - a 64-BIT VALUE of the symbol (a `long`, which is 64 bit in this ABI, or a
 *    pointer to a 64 bit type) is materialised "daddu $h2,$h,$zero" (the hi
 *    sign extended) + "addiu $x,$h2,%lo(S)" at each use - the retail's "F3"
 *    form.  A 32 bit pointer that is only ever COPIED (like "a0 = D_00189E20")
 *    gets the same F3 form;
 *  - `long long *p = (long long *)D_00189E20;` changes the branch polarity of
 *    the state test: with it the "state == 1" test stays a forward `beq` and
 *    the "return 0" stays a separate two instruction block, which is what the
 *    retail has; with a plain `char *` the compiler inverts the test (`bne`)
 *    and merges the block into the fall-through.  A `switch` on the state byte
 *    also keeps the forward `beq`;
 *  - cc1 folds two references to the same symbol into one materialisation
 *    whatever their type and whatever block they are in (measured with
 *    `char *`, `unsigned char *`, `long long *` and `long *` bases, with the
 *    second assignment in the same block and in a later one, and with the
 *    first base in a nested scope).  The retail has the state byte on a
 *    different materialisation from the fields, so its source must use two
 *    expressions cc1 cannot fold - the one spelling that is known to stop the
 *    fold is a CALL between them, and there is no call there.
 *
 * The attempt below is the best measured: 180 bytes against the retail's 192,
 * with the tail from "beqz $a0" on equal to the retail's instruction for
 * instruction.  What still differs is the head: the retail keeps the address
 * in the lui register and re-materialises the %lo per use (F2 + F3), this
 * build keeps one address in $a1 (F1) and re-materialises the %lo only for the
 * "a0 = D"; and the retail's "state == 1" branch is a forward `beq` to an
 * out-of-line "return 0" block, this build inverts it to `bne`.
 *
 * Not linked, not counted.
 */

typedef long long ll;

s32 func_L20_002B7A48_attempt4(char *a0)
{
    ll *S = (ll *)D_00189E20;
    ll *P;

    u8 state = ((u8 *)S)[0x22B4];

    if (state == 1)
        return 0;
    if (state == 4)
        return 0xD;
    P = (ll *)D_00189E20;
    if (a0 != 0 && ((u8 *)P)[0x22BA] != 0 && *(s16 *)((u8 *)P + 0x24EC) == 0) {
        s32 idx = func_L00_002ADEA0(0);
        return ((Entry *)(D_L20_0026D970 + D_00139568[idx] * 0xE0))->v;
    }
    a0 = (char *)D_00189E20;
    if (*(s32 *)(a0 + 0x24CC) != 1)
        return *(u8 *)(a0 + 0x1492) ? 0x67 : 0;
    return 0x51;
}
