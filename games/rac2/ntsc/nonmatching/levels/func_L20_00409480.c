#include "include_asm.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed short s16;
typedef signed int s32;
typedef unsigned int u32;

extern char *D_L20_001FBB20[];

extern char *D_L20_001B2F9C __attribute__((sda));

extern int D_L20_001B2FB8;

extern void func_L09_003F73E8(char *);

extern void func_L09_003F77D0(char *);

extern void func_0029F338(int);

/* Near miss from src/levels/20_yeedil/L20_0039C448.c: 336 bytes; 332-byte candidate; 0/1.
    The walk, the three calls, the frame (0x50) and the globals are all right; two
    instructions are missing and the register allocation differs:
      - retail keeps `index & 0xFF` in a temporary for the first test and copies it to $s4
        only later (`andi $a1,$s2,0xFF; beq $a1,$a0; ...; daddu $s4,$a1,$zero`), this cc1
        copies it to $s4 at once (`andi $s4,$s2,0xFF; beq $s4,$a0`).  Two C names for the
        value do not change it.
      - retail's loop back-edges are the branch-likely forms with the reload of the record
        word in the delay slot (`bgezl $v0,.L00409508 / lhu $v0,0($s1)` and
        `beql $s1,$v0,.L00409508 / lhu $v0,0($s1)`), this cc1 keeps the word in $v1 across
        the loop and uses plain `bgez`/`beq`, so it is two instructions shorter.
    The `u16 **tbl` spelling of the table element is what puts the address of the element in
    a callee-saved register ($s3), as retail has it; a plain `D_L20_001FBB20[index]` leaves it
    in $a1, which the calls clobber.

    Wave 20 (this worker) measured further and got the body down to the retail's shape:
      - the `index & 0xFF` split IS reproducible: write the expression twice, once in the
        first test and once in the test at the end of the loop.  Then this cc1 emits
        `andi $a1,$s2,0xFF` for the first test and copies it into $s4 in the delay slot of
        the `beqz $s1` (`daddu $s4,$a1,$zero`), exactly as the retail does.
      - the loop back-edges come out as the retail's branch-likely pairs with the reload of
        the record word in the delay slot (`bgezl $v0,body / lhu $v0,0($s1)` and
        `bnel $s1,$v0,exit / lhu $v0,0($s1)`) when the loop is written as a `do { ... }
        while (1)` whose two continue cases are joined with `||` and the reload of the word
        sits inside the `if`.
      - the retail keeps the three constants 0xD0F, 0xFE and 0xFD in the callee-saved
        registers $fp, $s7 and $s6 (it materialises them once, before the loop), so they
        have to be variables in the source, not literals: with them as literals this cc1
        compares against an immediate inside the loop.  With them as variables this cc1
        still puts 0xD0F in a scratch register and re-materialises it inside the loop, and
        it puts 0xFE in $fp and 0xFD in $s7 where the retail has 0xFE in $s7 and 0xFD in $s6
        - the register order follows the order in which the values are first used, and
        this cc1 allocates the callee-saved registers the other way round.
      - what is left is the block order: the retail lays the loop out as [preheader][body]
        [latch], this cc1 rotates it to [preheader][latch][body] and jumps into the body,
        which also flips the polarity of the second back-edge (`beql` against the retail's
        `beql`, but the exit is a branch here and the fall-through there).  The rotation
        happens as soon as the body has more than one block (the calls sit in their own
        block); with a one-block body the loop comes out in the retail's shape.
      - the table element's address: the retail computes it into $v1 and copies it into
        $s3 in the delay slot of the `beqz $s1` (`addu $v1,$v1,$v0; ...; daddu $s3,$v1,$zero`),
        so the address is written twice in the source - once for the first load and once
        for the comparison in the loop - and this cc1 keeps the two computations apart
        (an extra `daddu $s6,$v1,$zero`).
      - the order of the saves is the retail's `sd $s0,0x0($sp); sd $s3,0x18($sp)` against
        this cc1's `sd $s3,0x18($sp); sd $s0,0x0($sp)` (the "save-order rotation" wall).
    The candidate below is the closest measured (332 bytes against the retail's 336): the
    two missing instructions are the retail's `addiu $fp,$zero,0xD0F` and
    `addiu $s6,$zero,0xFD`, which this cc1 replaces with an immediate inside the loop and
    with a register of its own. */
void func_L20_00409480(s32 index) {
    u16 *rec = (u16 *)D_L20_001FBB20[index];
    char *p;
    u16 w;
    s16 magic = 0xD0F;
    u8 fe = 0xFE;
    u8 fd = 0xFD;

    if ((index & 0xFF) == 0xFF) {
        return;
    }
    if (D_L20_001B2FB8 < index) {
        return;
    }
    if (rec == 0) {
        return;
    }
    w = *rec;
    do {
        p = D_L20_001B2F9C + ((w & 0x7FFF) << 8);
        if (p == 0) {
            break;
        }
        if (*(s16 *)(p + 0xAA) == magic && *(u8 *)(p + 0x20) != fe && *(u8 *)(p + 0x20) != fd) {
            *(u8 *)(*(int *)(p + 0x68) + 0x20) = 0;
            func_L09_003F73E8(p);
            func_L09_003F77D0(p);
            func_0029F338((int)p);
        }
        if ((index & 0xFF) == 0xFF) {
            break;
        }
        rec++;
        if (D_L20_001B2FB8 < index) {
            break;
        }
        if (rec == 0) {
            break;
        }
        if (*(s16 *)(rec - 1) >= 0 || rec == (u16 *)D_L20_001FBB20[index]) {
            w = *rec;
            continue;
        }
        break;
    } while (1);
}

/* ------------------------------------------------------------------
   Third attempt (worker 06, level-20 wave of 2026-10-20).  Two new
   findings and one that is still open.

   NEW 1 - the save order is the order in which the callee-saved
   registers are ALLOCATED, and it follows the order in which the
   values are first set in the source.  The earlier draft assigned the
   three constants (0xD0F, 0xFE, 0xFD) at the top of the function, so
   the compiler hoisted them into the pre-header and they took the
   registers $fp and $s7 before the loop's own values.  Assigning them
   AFTER the three early exits (where the retail materialises them,
   0x4094F0-0x4094FC) reproduces the retail's save order exactly:
   s2, s0, s3, s4, s5, s6, s7, fp, ra, s1.  That alone brought the
   candidate from 332 to 336 bytes - the retail's size - and fixed the
   frame offsets.

   NEW 2 - the loop's layout.  The retail lays the loop out as
   [pre-header][body][latch] with the pre-header falling into the
   body; a `do { } while (1)` with `break`/`continue` is laid out as
   [pre-header][latch][body] with the pre-header JUMPING to the body.
   Writing the loop's exits as `return` instead of `break` (the exits
   are the function's exits anyway) gives the retail's layout: the body
   comes first and the latch (the block with the two back-edges) comes
   last.  With `break` the compiler rotates the loop; with `return` it
   does not.

   NEW 3 - the store in the body is a WORD store (`sw $zero,0x20($v0)`),
   not a byte store, so the source is `*(int *)(*(int *)(p + 0x68) + 0x20) = 0;`.

   STILL OPEN - three things:
     - the `rec += 2` belongs in the delay slot of the first latch
       test (`beq $s4,$s5` / `addiu $s1,$s1,0x2`); this cc1 puts the
       `lui` of D_1B2FB8 there and moves the increment after the
       D_1B2FB8 test.  The source order is the same as the retail's
       (the test, then the increment), so this is the delay-slot
       scheduler's choice;
     - the second back-edge is a `beql` to the body in the retail
       (`beql $s1,$v0,.L00409508` with the reload of the record word in
       the delay slot) and a plain `bne` to the exit here, which
       splits the reload into a block of its own (four bytes over);
     - the register allocation: the retail keeps the table element's
       ADDRESS in $s3 (a copy of the address in $v1) and reloads the
       element through it in the loop, while this cc1 keeps the table's
       BASE in $s7 and recomputes the element's address inside the
       loop.

   The candidate below is the closest measured (340 bytes against the
   retail's 336, 41 instructions of 84 differ).  ------------------------------------------------------------------ */

extern char *D_L20_001FBB20[];
extern char *D_L20_001B2F9C __attribute__((sda));
extern int D_L20_001B2FB8;
extern void func_L09_003F73E8(char *);
extern void func_L09_003F77D0(char *);
extern void func_0029F338(int);

void func_L20_00409480(s32 index) {
    u16 *rec = (u16 *)D_L20_001FBB20[index];
    char *p;
    u16 w;
    s16 magic;
    u8 fe;
    u8 fd;

    if ((index & 0xFF) == 0xFF) {
        return;
    }
    if (D_L20_001B2FB8 < index) {
        return;
    }
    if (rec == 0) {
        return;
    }
    magic = 0xD0F;
    fe = 0xFE;
    fd = 0xFD;
    w = *rec;
    do {
 top:
        p = D_L20_001B2F9C + ((w & 0x7FFF) << 8);
        if (p == 0) {
            return;
        }
        if (*(s16 *)(p + 0xAA) == magic && *(u8 *)(p + 0x20) != fe && *(u8 *)(p + 0x20) != fd) {
            *(int *)(*(int *)(p + 0x68) + 0x20) = 0;
            func_L09_003F73E8(p);
            func_L09_003F77D0(p);
            func_0029F338((int)p);
        }
        if ((index & 0xFF) == 0xFF) {
            return;
        }
        rec++;
        if (D_L20_001B2FB8 < index) {
            return;
        }
        if (rec == 0) {
            return;
        }
        if (*(s16 *)(rec - 1) >= 0) {
            w = *rec;
            goto top;
        }
        if (rec == (u16 *)D_L20_001FBB20[index]) {
            w = *rec;
            goto top;
        }
        return;
    } while (1);
}
