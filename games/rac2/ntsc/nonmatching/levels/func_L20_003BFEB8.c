/* Near miss from src/levels/20_yeedil/L20_0039C448-g8.c (the -G8 unit).

   The attempt below is 96 instructions against the retail's 99 and the
   shape of the body is right: the walk over the record list (the
   `beql $a0,$s4` sentinel test with the counter in the delay slot, the
   call to f_003305C0, the pointer advanced by 4 in the delay slot of the
   exit branch, the counter and the reload in the delay slot of the loop
   branch), the test of the int at +0x2294 of D_00189E20, the two reads
   of the int at +0x24 of the state, the byte at +0xBC, the two table
   lookups with the 0x80 mask and the stores through D_L20_0021E340 and
   D_L20_0021EFB0.

   What differs, and what was measured:

   - The retail's loop is a `goto` loop whose check block ends in an
     unconditional jump to the body; that is what makes this cc1 emit
     the branch-LIKELY form the retail uses (`beql $a0,$s4` with
     `addiu $s2,$s2,0x1` in the delay slot, `bnel $v0,$zero` with
     `lw $a0,0x0($s0)` in the delay slot).  The attempt's loop block
     ends in the fall-through to the exit and the branches come out
     plain (`beq`, `bnel`).  See the note on func_L20_0039D680: the
     check block has to end in an unconditional jump to the body.

   - The retail keeps the list pointer in one register ($s0) for the
     whole loop; the attempt needs two ($s0 for the first read and $s1
     for the loop), which costs the extra `daddu $s1,$s0,$zero` and one
     more callee-saved register (the frame is 0x40 against the retail's
     0x30).  Every spelling tried (the value read from the state
     instead of from the pointer, a char pointer advanced by 4, the
     value read in the loop only) keeps the copy.

   - The retail's -1 is two different values in two registers: $s4 for
     the list's sentinel (set in the prologue) and $s0 for the "not
     found" marker (set in the delay slot of the D_00189E20 test).  The
     attempt merges them into one, and its `p = (s32 *)-1` comes out as
     the 64-bit -1 (`lui` + `ori`) where the retail has the 32-bit
     `addiu $s0,$zero,-0x1`.

   - The retail's `lw $a3,0x24($s0)`-style loads of the state fields
     are re-read after the call to f_00335728 (the attempt keeps the
     value in a register instead).

   The attempt below is the closest one.
   With the attempt in src/ (the unit's default flags):
     20_yeedil: 380 bytes, expected 396
     func_L20_003BFEB8: 0 of 1 places match
*/
#include "include_asm.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned int u32;
typedef signed int s32;
typedef float f32;

extern s32 func_003305C0(s32, s32);
extern s32 func_00335728(s32);
extern void func_003253A8(char *);
extern void func_003306A0(s32, s32);

/* The retail body reaches D_00189E20 through $at/$lo. */
extern char D_00189E20[] __attribute__((nosda));

/* The retail body reaches D_0019B2E8 through $at/$lo. */
extern char D_0019B2E8[] __attribute__((nosda));

/* The retail body reaches D_L20_001B2DE0 through $at/$lo. */
extern u8 D_L20_001B2DE0[] __attribute__((nosda));

/* The retail body reaches D_L20_0021E340 through $at/$lo. */
extern u8 D_L20_0021E340[] __attribute__((nosda));

/* The retail body reaches D_L20_0021EFB0 through $at/$lo. */
extern u8 D_L20_0021EFB0[] __attribute__((nosda));

/* 0x1AEFF0 - 0x7600 : the retail body reaches this one through $gp. */
extern s32 D_001A79F0 __attribute__((sda));

void func_L20_003BFEB8(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);
    s32 *p = (s32 *)state;
    s32 value = *p;
    s32 count = 0;
    s32 sentinel = -1;
    s32 n;
    s32 a;
    s32 b;
    u32 c;

body:
    if (value == sentinel) {
        goto check;
    }
    if (func_003305C0(value, -1) != 0) {
        goto out;
    }
    count++;
    p++;
    value = *p;
check:
    if (count < 8) {
        goto body;
    }
    if (*(s32 *)(D_00189E20 + 0x2294) == 0x31) {
        goto out;
    }
    n = -1;
    a = *(s32 *)(state + 0x28);
    b = *(s32 *)(state + 0x24);
    if (a == n) {
        goto have_b;
    }
    if (func_00335728(b + 0x80) == 0) {
        goto out;
    }
    b = *(s32 *)(state + 0x24);
have_b:
    if (b == n) {
        goto have_c;
    }
    *(u8 *)(arg0 + 0xBC) = *(u8 *)(state + 0x24);
    c = *(u32 *)(state + 0x20);
have_c:
    if (c != 0xFF) {
        goto other;
    }
    {
        u8 d = *(u8 *)(arg0 + 0xB0);
        s32 e = d & 0xFF;

        *(u8 *)(D_L20_0021E340 + *(s16 *)(arg0 + 0xB2)) = d + 2;
        if (e == (s32)c) {
            goto store;
        }
        if (*(u8 *)(D_L20_001B2DE0 + e) & 0x80) {
            goto done;
        }
        if (*(u8 *)(D_0019B2E8 + (D_001A79F0 << 4) + e) & 0x80) {
            goto done;
        }
    store:
        *(u8 *)(D_L20_0021EFB0 + *(s16 *)(arg0 + 0xB2)) = d + 2;
    }
done:
    func_003253A8(arg0);
    goto out;
other:
    if (c == (s32)n) {
        goto out;
    }
    *(u8 *)(arg0 + 0x20) = *(u8 *)(state + 0x20);
    a = *(s32 *)(state + 0x2C);
    if (a == n) {
        goto out;
    }
    func_003306A0(a, 1);
out:
    return;
}

/* ------------------------------------------------------------------
   Wave 21, worker 08.  A new attempt, with the retail's loop shape.

   The attempt below is 380 bytes against the retail's 396 and the
   loop's SHAPE is now the retail's: the check comes first, the
   sentinel test is a branch-likely with the count increment in its
   delay slot, the list pointer is one register, and the back edge is
   a branch-likely with the reload of the element in its delay slot:

     retail:  beql $a0,$s4,.+0x18 / addiu $s2,$s2,0x1 / jal / addiu $a1,-1 /
              bnez $v0,EXIT / addiu $s0,$s0,0x4 / addiu $s2,$s2,0x1 /
              slti $v0,$s2,0x8 / bnel $v0,$zero,CHECK / lw $a0,0($s0)
     here:    beql $a0,$s5,.+0x1C / addiu $s2,$s2,0x1 / jal / addiu $a1,-1 /
              bnel $v0,$zero,EXIT / ld $s0,0($sp) / addiu $s1,$s1,0x4 /
              addiu $s2,$s2,0x1 / slti $v0,$s2,0x8 / bnel $v0,$zero,CHECK /
              lw $a0,0($s1)

   What is still different, and what was measured:

   - the register allocation: the retail gives $s0 to the list
     pointer, $s1 to the state, $s2 to the count, $s3 to arg0 and $s4
     to the sentinel (-1), and it reuses $s0 for the "not found" marker
     after the loop (`addiu $s0,$zero,-0x1` at 0x3BFF2C).  The attempt
     gives $s0 to the state and $s1 to the pointer, and it needs one
     callee-saved register more than the retail (its frame is 0x40
     against the retail's 0x30).  The order of the two declarations
     (`char *state` then `s32 *p`, or `s32 *p` then `char *state`) does
     not change it: the load lands in the lower register either way.
   - the exit branch of the call: the retail has a plain `bnez $v0` with
     the pointer increment in its delay slot, the attempt a branch-
     likely `bnel $v0,$zero` with the epilogue's restore of $s0 in the
     delay slot.  Writing the test as `if (... == 0) { p++; count++; }
     else { goto out; }` does not change it either.
   - the retail re-reads the fields of the state after the call to
     f_00335728 (`lw $v0,0x24($s1)` at 0x3BFF4C), so the source uses
     the expression `*(s32 *)(state + 0x24)` twice and keeps no
     variable for it; the attempt's `b` variable costs a register.
   - the retail's "not found" marker is a second -1 (it is materialised
     into $s0 after the loop), while the sentinel of the loop test is
     the one of the prologue ($s4).  The attempt has both, but the
     marker lands in $s1 (the pointer's register) instead of $s0.

   The attempt below is the closest one of this wave: the loop and its
   branches are right, the registers are not.
*/

#include "include_asm.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned int u32;
typedef signed int s32;
typedef float f32;

extern s32 func_003305C0(s32, s32);
extern s32 func_00335728(s32);
extern void func_003253A8(char *);
extern void func_003306A0(s32, s32);

extern char D_00189E20[] __attribute__((nosda));
extern char D_0019B2E8[] __attribute__((nosda));
extern u8 D_L20_001B2DE0[] __attribute__((nosda));
extern u8 D_L20_0021E340[] __attribute__((nosda));
extern u8 D_L20_0021EFB0[] __attribute__((nosda));
extern s32 D_001A79F0 __attribute__((sda));

void func_L20_003BFEB8(char *arg0) {
    s32 *p = (s32 *) *(char **)(arg0 + 0x68);
    char *state = (char *)p;
    s32 sentinel = -1;
    s32 count = 0;
    s32 n;
    u32 c;

check:
    if (*p == sentinel) {
        count++;
        goto test;
    }
    if (func_003305C0(*p, -1) != 0) {
        goto out;
    }
    p++;
    count++;
test:
    if (count < 8) {
        goto check;
    }
    if (*(s32 *)(D_00189E20 + 0x2294) == 0x31) {
        goto out;
    }
    n = -1;
    if (*(s32 *)(state + 0x28) == n) {
        goto have_b;
    }
    if (func_00335728(*(s32 *)(state + 0x24) + 0x80) == 0) {
        goto out;
    }
have_b:
    if (*(s32 *)(state + 0x24) == n) {
        goto have_c;
    }
    *(u8 *)(arg0 + 0xBC) = *(u8 *)(state + 0x24);
    c = *(u32 *)(state + 0x20);
have_c:
    if (c != 0xFF) {
        goto other;
    }
    {
        u8 d = *(u8 *)(arg0 + 0xB0);
        s32 e = d & 0xFF;

        *(u8 *)(D_L20_0021E340 + *(s16 *)(arg0 + 0xB2)) = d + 2;
        if (e == (s32)c) {
            goto store;
        }
        if (*(u8 *)(D_L20_001B2DE0 + e) & 0x80) {
            goto done;
        }
        if (*(u8 *)(D_0019B2E8 + (D_001A79F0 << 4) + e) & 0x80) {
            goto done;
        }
    store:
        *(u8 *)(D_L20_0021EFB0 + *(s16 *)(arg0 + 0xB2)) = d + 2;
    }
done:
    func_003253A8(arg0);
    goto out;
other:
    *(u8 *)(arg0 + 0x20) = *(u8 *)(state + 0x20);
    if (*(s32 *)(state + 0x2C) == n) {
        goto out;
    }
    func_003306A0(*(s32 *)(state + 0x2C), 1);
out:
    return;
}
