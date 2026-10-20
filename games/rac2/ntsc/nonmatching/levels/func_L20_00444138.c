/* 0 of 1 places match - last line: 20_yeedil: 152 bytes, expected 156
   The walk is recovered from the matched sibling recipe (L01_003CAFE0-g8.c, L04_00422A00-g8.c):
   a `for` over the 0x100-stride object list whose bound D_L20_001B2FA0 is re-read every
   iteration (the body stores, so this cc1 reloads it on both paths) and whose id arm is a
   `(u16)(id - 0x1233) < 2 || id == 0x1231 || id == 0x1232` chain; the loop body and tail match
   word for word, including the bnel with the 0xAA halfword in its delay slot. Three residues:
   (1) retail re-materialises the bound with a fresh `lui $v0,0x1B; lw $v0,0x2FA0($v0)` on the
   storing path where this cc1 keeps the %hi in a caller-saved register and loads `0($t0)`
   (one word short, 152 bytes); (2) retail keeps a `nop` between the loop head's
   `lhu $v1,0xAA($a0)` and the `addiu $v0,$v1,-0x1233`; (3) the bound's two reloads carry two
   different addressing forms in retail (a `$gp` word in the bne's delay slot, the absolute
   word on the storing path) - a second name (`D_L20_001B2FA0_1` as `sda`, this file's form
   uses `nosda` for both) reproduces the `$gp` form but turns the `bne` into a `bnel` and
   costs the same one word. Declaring the bound `sda` outright is worse (144 bytes, the arm
   inlines before the tail). */
#include "common.h"

extern u8 *D_L20_001B2F9C __attribute__((nosda));

extern u8 *D_L20_001B2FA0 __attribute__((nosda));

/* Flags the 0x31/0x34 pair of every live 0x1233/0x1234/0x1231/0x1232 object in the level's
   list run. */
void func_L20_00444138(void)
{
    u8 *p;

    for (p = D_L20_001B2F9C; p <= D_L20_001B2FA0 && *(u8 *)(p + 0x20) != 0xFF;
         p += 0x100) {
        u16 id = *(u16 *)(p + 0xAA);

        if ((u16)(id - 0x1233) < 2 || (s16)id == 0x1231 || (s16)id == 0x1232) {
            *(u8 *)(p + 0x31) = 0;
            *(u16 *)(p + 0x34) = *(u16 *)(p + 0x34) | 1;
        }
    }
}

/* Second wave (20_yeedil, 2026-10-20, worker 02): the same wall, re-measured from
   the retail's own disassembly, with the id arm read as the earlier note has it
   (`(u16)(id - 0x1233) < 2 || (s16)id == 0x1231 || (s16)id == 0x1232`) - the third
   test compares the sign-extended id, not the flags, because the delay slot of
   the beql is annulled on the not-taken path.  The candidate below is 156 bytes,
   the retail's size, and 20 instructions differ:

   - the retail keeps the bound in `$v0` (the `sltu $v0,$v0,$a0` reuses the
     register) and the three constants in `$a3` (0x1231), `$a2` (0x1232) and
     `$a1` (0xFF); the candidate keeps the bound in `$a1` and the constants in
     `$t0`, `$t1`, `$t2`;
   - the retail's first two id tests are branch-likely (`bnel`/`beql` to the
     store path) with the `lhu` of the flags in their delay slots, the third is
     a plain `bne` away with the `$gp` read of the bound in its delay slot; the
     candidate uses plain branches and puts the `$gp` read in the store block;
   - the retail stores the `0` at 0x31 before the `ori` of the flags, the
     candidate after it.

   Measured: the bound needs two names (the retail reads it once through $gp and
   once with an absolute lui/lw), the `$gp` read has to be `volatile` or this
   cc1 drops it as a dead load, and a `volatile` read cannot be split into a
   delay slot, which is what costs the branch-likely shape.  With the bound read
   plainly the branch-likely shape comes back but the `$gp` read is dropped (38
   instructions against the retail's 39).  Also tried: the bound and the flags
   in one variable, in two variables, the flags as `u32`, the base as `u32`, the
   declaration order swapped, the pre-loop test reading the global directly, and
   the dead `$gp` read assigned to the flags instead of the bound.
   The body belongs in the -G8 sibling (it reads a global through $gp). */

#include "include_asm.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef signed char s8;
typedef signed short s16;

/* The bound of the object bank: 0x1AEFF0 + 0x3FB0.  The retail reads it once through $gp
   and once with an absolute lui/lw, so the unit needs two names for the address. */
extern volatile u32 D_L20_001B2FA0 __attribute__((sda));

extern volatile u32 D_L20_001B2FA0_1;

extern u32 D_L20_001B2F9C;

void func_L20_00444138(void) {
    char *base = (char *)D_L20_001B2F9C;
    u32 v = D_L20_001B2FA0_1;
    u16 id;

    if (v < (u32)base) {
        return;
    }
    if (*(u8 *)(base + 0x20) == 0xFF) {
        return;
    }
    do {
        id = *(u16 *)(base + 0xAA);
        if ((u16)(id - 0x1233) < 2 || (s16)id == 0x1231 || (s16)id == 0x1232) {
            v = D_L20_001B2FA0;
            v = *(u16 *)(base + 0x34);
            base[0x31] = 0;
            v |= 1;
            *(u16 *)(base + 0x34) = v;
            v = D_L20_001B2FA0_1;
        }
        base += 0x100;
        if (v < (u32)base) {
            return;
        }
    } while (*(u8 *)(base + 0x20) != 0xFF);
}

/* ---- wave 3 (20_yeedil, 2026-10-20, worker 02): THE RECIPE, three words short ------
 *
 * The body is now recovered: the candidate is the retail's exact 156 bytes and only
 * three words differ, all one register allocation (the `sra` of the sign extension
 * lands in $v1 here and in $v0 in the retail).  What got it there:
 *
 *  1. The two id tests are NOT an `||` chain but three separate tests with `goto`:
 *
 *         if ((u16)(id - 0x1233) < 2) goto store;
 *         if ((s16)id == 0x1231) goto store;
 *         bound = <the $gp read>;
 *         if ((s16)id != 0x1232) goto next;
 *     store:
 *         base[0x31] = 0;
 *         *(u16 *)(base + 0x34) |= 1;
 *         bound = <the absolute read>;
 *     next:
 *
 *     That gives the retail's `bnel`/`beql` for the first two tests (branch-likely,
 *     the flags load in the delay slot) and the PLAIN `bne` for the third one (the
 *     $gp read of the bound in the delay slot).  With the `||` chain the third test
 *     also comes out branch-likely and the $gp read lands elsewhere; with the `goto`
 *     spelling but the $gp read before the tests, the $gp read takes the FIRST test's
 *     delay slot and that test loses its branch-likely form.
 *
 *  2. The bound needs TWO names and TWO flags.  The retail reads it once through $gp
 *     (`lw $v0,0x3FB0($gp)`, in the third test's delay slot) and twice absolutely
 *     (`lui $v0,%hi; lw $v0,%lo($v0)`, at the top and after the store block).  The
 *     $gp form needs the `sda` attribute, the absolute ones `nosda`:
 *
 *         extern u32 D_L20_001B2FA0 __attribute__((sda));        /* the $gp read */
 *         extern u32 D_L20_001B2FA0_abs __attribute__((nosda));  /* the absolute reads */
 *
 *     Two names for one address with the same attribute are merged by the compiler,
 *     and the absolute read then reuses the %hi the loop hoisted out (`lw $v0,0($t0)`
 *     instead of the retail's `lui; lw`).  A `volatile` absolute read keeps the
 *     `lui; lw` form but a volatile load is a single instruction, so the assembler
 *     gives it the $gp form instead.
 *
 *  3. The unit needs `-mno-split-addresses` for the retail's "base == dest" absolute
 *     load (`lui $a0,%hi(D_1B2F9C); lw $a0,%lo(D_1B2F9C)($a0)`).  In the default
 *     profile the compiler splits the address (`lui $v0,%hi; lw $a0,%lo($v0)`), and
 *     the flag changes the other bodies of a shared unit, so this function needs a
 *     unit of its own:
 *
 *         src/levels/20_yeedil/L20_00444138-g8.c   -O2 -G8 -ffunction-sections -mno-split-addresses
 *
 *     The $gp read and the absolute reads must then be DIFFERENT symbols: the
 *     assembler picks the addressing form per symbol (the `sda` one is small data and
 *     goes through $gp), so one name cannot give both forms.
 *
 *  What is left (three words, one register allocation): the retail keeps the sign
 *  extension of the id in $v0 (`sll $v0,$v1,16; sra $v0,$v0,16`, in place) and this
 *  compiler puts it in $v1 (`sll $v0,$v1,16; sra $v1,$v0,16`), so the retail's
 *  `beql $v0,$a3` and `bne $v0,$a2` become `beql $v1,$a3` and `bne $v1,$a2`.  The
 *  reload pass gives the `ashr` the id's register (dead after the `ashl`) instead of
 *  the `ashl`'s own register (dead after the `ashr`).  Measured and not enough: the
 *  id as `s16`/`short`/`int`/`u32`, the cast as `(s16)`/`(short)`/`(int)(short)`/
 *  `(s16)(u16)`, a named `s16` copy of the id, the id assigned before the loop and
 *  reloaded at the bottom, the flags loaded into a named local, the declaration order
 *  of the locals, and the per-file flags `-g`, `-mno-split-addresses`,
 *  `-fno-schedule-insns`, `-fno-schedule-insns2`, `-fexpensive-optimizations`,
 *  `-fno-expensive-optimizations`, `-fno-gcse`, `-fno-crossjumping`, `-fthread-jumps`,
 *  `-foptimize-comparisons`, `-fpeephole2`, `-fsched-interblock`, `-fregmove`,
 *  `-O1`, `-O3`, `-mips3` (all the same three words).  The residue is a reload-pass
 *  choice, not a source shape.
 */

/* The candidate in full, as it stands (the unit's own flags:
 * -O2 -G8 -ffunction-sections -mno-split-addresses; the two names for the
 * bound are D_L20_001B2FA0 (sda, the $gp read) and D_L20_001B2FA0_abs
 * (nosda, the absolute reads), the base D_L20_001B2F9C_abs (nosda)):
 *
 *     void func_L20_00444138(void) {
 *         u8 *base = (u8 *)D_L20_001B2F9C_abs;
 *         u32 bound = D_L20_001B2FA0_abs;
 *
 *         if (bound < (u32)base) {
 *             return;
 *         }
 *         if (*(u8 *)(base + 0x20) == 0xFF) {
 *             return;
 *         }
 *         for (;;) {
 *             u16 id = *(u16 *)(base + 0xAA);
 *
 *             if ((u16)(id - 0x1233) < 2) {
 *                 goto store;
 *             }
 *             if ((s16)id == 0x1231) {
 *                 goto store;
 *             }
 *             bound = D_L20_001B2FA0;
 *             if ((s16)id != 0x1232) {
 *                 goto next;
 *             }
 *         store:
 *             base[0x31] = 0;
 *             *(u16 *)(base + 0x34) |= 1;
 *             bound = D_L20_001B2FA0_abs;
 *         next:
 *             base += 0x100;
 *             if (bound < (u32)base) {
 *                 return;
 *             }
 *             if (*(u8 *)(base + 0x20) != 0xFF) {
 *                 continue;
 *             }
 *             return;
 *         }
 *     }
 *
 * tools/check.py on it (in a -G8 unit of its own, with the flags above):
 *
 *     20_yeedil: instruction at +0x4c differs
 *     func_L20_00444138: 0 of 1 places match (src/levels/20_yeedil/L20_00444138-g8.c)
 *
 * The candidate is the retail's exact 156 bytes; the first difference is the
 * `sra` at +0x4c (the retail keeps the sign extension in $v0, this cc1 in
 * $v1), and with it the two branches that use it (+0x50 and +0x58).
 */
