/* Near miss from src/levels/20_yeedil/L20_00449178.c: 200 bytes, retail 204.
   This unit has no $gp access, so the body belongs here (-G0), not in the
   -G8 sibling.

   Everything matches but one instruction: the retail writes the 128-bit zero
   of the stack buffer with a materialised register
       por $v0, $zero, $zero ; sq $v0, 0x20($sp)
   while this cc1's rac2_fold_zero_ti_store pass folds that store to
       sq $zero, 0x20($sp)
   (4 bytes short).  The pass folds whenever the zero register is dead after
   the store, and it is dead here.

   Measured this wave (2026-10-20): the frame (0x50), the two saved
   registers, the 0.3f materialisation, the two float calls, the 14-argument
   func_0032C700 call with its four stack words, the branch-likely test and
   the epilogue are all the retail instructions.  The union spelling (float
   f[4] / TI128 q) is what puts the float at sp+0x28 and keeps the buffer
   address out of a callee-saved register, as the retail has it.

   Spellings tried for the zero, all of which fold to sq $zero: `v.q = 0`,
   `v = 0`, `v = {0}`, `*(volatile TI128 *)&v = 0`, the same store through a
   pointer, and a struct with a TI128 member.  The por survives only when the
   zero feeds two stores (measured: two stores of the same zero keep
   `por $2,$0,$0` and two `sq $2`), and the retail has one store.  The same
   wall is recorded for func_L00_002A75D8, func_L00_003ACC88,
   func_L13_0040A2C8 and func_L04_00422D40.

   Measured further this wave (2026-10-20, worker 10 of the level 20 wave):
   the fold is not the only mechanism - the shape of the store itself is
   decided before the pass.  The RTL of the store is
       (set (mem/s:TI (plus:SI (reg:SI 1 at) (const_int 32)) 0) (const_int 0))
   (the RTL dump, cc1 -dall, file x.i.regmove), i.e. the source is a
   TImode CONSTANT zero, and the movti_internal pattern of
   gcc/config/mips/mips.md has no alternative for a constant zero but a
   register: the reload then gives it register 0 and the pattern prints
   `sq $0,32($sp)`.  With two uses of the same constant the reload has to
   keep it in one register and the pattern's "d" alternative prints
   `por $2,$0,$0` (por rd,$zero,rs) followed by the two `sq $2`.  So the
   register form needs the value to be in a register, and this cc1 only
   puts a TImode constant there when the value has two or more uses.
   Further spellings measured, all of them `sq $0`: the union as a local, in
   a global, in a struct, in an array, the store through a pointer, through
   a volatile pointer, `*(TI128 *)&v = 0`, a TI128 local (`TI128 z = 0;`),
   a union with a float array, a TI128 value returned by a call, and the
   same store with the union at a non-zero stack offset.  A function that
   RETURNS a TI128 does keep the por (`por $2,$0,$0`, the return value in
   $v0/$v1), which is the only way measured to get the materialised zero
   into a register with one use.

   The wall is therefore the same for every body of this shape, and no
   matched function in the repository has a `por` + `sq` zero store (checked
   over every level's retail assembly against the matched functions in
   src/). */

#include "include_asm.h"

typedef int TI128 __attribute__((mode(TI)));

extern void func_00283098(void *, void *, void *);
extern void func_00282CC0(void *, void *, void *);
extern void func_002E59F8(int);
extern void func_0032C700(void *a0, int a1, int a2, int a3, int a4, int a5,
                           int a6, int a7, float f12, float f13, int s1, int s2,
                           int s3, int s4);

void func_L20_0044AB68(char *arg0) {
    union {
        TI128 q;
        float f[4];
    } v;
    char *state;

    v.q = 0;
    v.f[2] = 0.3f;
    func_00283098(&v, &v, arg0 + 0xC0);
    func_00282CC0(&v, &v, arg0 + 0x10);
    func_0032C700(&v, 0, 0x19, 0x14, 0x14, 0x14, 0x2D, 0, 0.666f, 20.0f, 1, 0,
                  0, 0);
    state = *(char **)(arg0 + 0x68);
    if (*(int *)(state + 0x1B8) != -1) {
        func_002E59F8(*(int *)(state + 0x1B8));
        *(int *)(state + 0x1B8) = -1;
    }
}

   ------------------------------------------------------------------
   Third attempt (wave w21, worker 10): the same wall, measured again
   from the compiler side, with one addition.  The RTL of the store
   after reload is
       (insn:TI ... (set (mem:TI (reg:SI 4 a0) 0) (reg:TI 0 $0)))
   (cc1 -dR, file x.i.sched2), i.e. the source of the store is the ZERO
   REGISTER, not a pseudo: `rac2_fold_zero_ti_store` never fires on this
   body (its guard sees register 0 referenced by the next instruction,
   `daddu $s0,$a0,$zero`, so the register reads as live), and the
   `sq $zero` comes from `movti_internal`'s store alternative itself
   (gcc/config/mips/mips.md: alternative 4, `sq %1,%0`, with %1 the zero
   register).  The retail's `por $v0,$zero,$zero` + `sq $v0,0x20($sp)` is
   alternative 0 (`por %0,$0,%1`), which needs the source of the store to
   be a real register - and this cc1 only puts a TImode constant in a
   register when the constant has two or more uses.  Measured again: one
   store folds to `sq $0`; two stores of the same constant keep
   `por $2,$0,$0` and two `sq $2` (one store too many for this body).
   A `volatile` store, a `volatile` pointer, a TI128 local, a union, a
   struct member and a store through a pointer were re-measured: all
   `sq $0`.  The wall stands; the body needs a compiler change, not a
   different spelling.
