/* near-miss: func_L20_00443CF0 (20_yeedil, 100 bytes).
   0x24: retail emits `daddu $a1,$sp,$zero` before the prologue's `sd $s0,0x30($sp)`,
   this cc1 the other way round. Every other word matches. Same wall as the recorded
   func_001334B8 row (a single scheduling word in the prologue). 
   Second wave (20_yeedil, 2026-10-20) re-measured this one and could not move it:
   ten further source shapes for the two locals (char[16], unsigned int[4], float[4],
   a TI128 pair, one 32-byte array used as v and v+16, a 48-byte struct holding all
   three objects, the &v[0] spellings, a `char *p0 = v0` temporary, the declaration
   order of the locals, and the float assigned as a separate statement) all give the
   same 100 bytes with only these two words swapped. The float constant is 1.0471977f
   (0x3F860A93) and is materialised in the entry block, before the frame setup, exactly
   as retail has it. Wall: the prologue interleaving - a lever that pins the local's
   address computation before the `sd $s0,0x30($sp)` prologue save is what is missing.*/
   Fourth wave (20_yeedil, 2026-10-20, worker 01): the two flags of the assignment
   were measured on this function in a file of its own, and each of them moves the
   pair the third wave named, but breaks something else:
     - `-fno-schedule-insns2` puts the retail's `daddu $a1,$sp,$zero` BEFORE the
       prologue's `sd $s0,0x30($sp)` - the pair is right - but the lui/ori/mtc1 of
       1.0471977f sinks below the frame adjust (retail has it in the entry block);
     - `-mastra-keep-frame-order` (patch 0021) also puts the pair right, but it keeps
       the $s0 save in the prologue instead of delaying it to just before the
       `lw $s0,0x68($v0)` that retail has, and it leaves the saves in the prologue
       order instead of retail's $s1, $ra, $s0.
   So the wall is the interaction of the two: retail delays the $s0 save AND lets
   the first call's second argument precede it.  Measured and rejected: the float
   initialised in the declaration or in a separate statement, the state loaded
   before or after the first call, the index in a variable of its own, the two
   locals as TI128, char[16] or unsigned int[4], the declaration order of the
   locals, the arguments as TI128* or char* - the pair never moves without one of
   the flags, and with a flag the constant or the save order breaks.
   The flags are per file: this function needs a unit of its own, it cannot go on
   the shared L20_0043ECB0.c (the flag changes the other bodies of the unit).

#include "common.h"

typedef int TI128 __attribute__((mode(TI)));

extern void func_L20_00443B60(char *, TI128 *, TI128 *, f32 *);
extern void func_L20_00440198(s32, TI128 *, TI128 *, f32);

void func_L20_00443CF0(char *arg0)
{
    char *state = *(char **)(arg0 + 0x68);
    TI128 v0;
    TI128 v1;
    f32 f;

    f = 1.0471977f;
    func_L20_00443B60(arg0, &v0, &v1, &f);
    func_L20_00440198(*(s32 *)(state + 0x21C), &v0, &v1, f);
}

/* Third wave (20_yeedil, 2026-10-20, worker 02): the same one-word-pair wall,
   re-measured.  The candidate below is the two `mode(TI)` locals at sp+0 and
   sp+0x10 with the float at sp+0x20, and it differs from the retail in one
   word pair: the retail sets the first call's second argument up before the
   prologue's `sd $s0,0x30($sp)`, this cc1 after it.  Additional spellings
   measured: the locals as `char[16]`, as one 32-byte struct, declared in the
   other order, the state as `char *` and as `void *`, the argument as `void *`,
   the index in a variable of its own, the float passed as `*(float *)&f`, the
   state load through a `u64` argument, and the state assigned after the first
   call - all give the same 100 bytes with the same two words swapped.  The
   per-file flags `-g`, `-mno-split-addresses`, `-fno-schedule-insns` and
   `-fno-schedule-insns2` were tried on the whole unit: the first two change
   nothing, the last two reorder the whole body.  This is the prologue
   interleaving the compiler notes list as a measured wall. */

#include "include_asm.h"

typedef int TI128 __attribute__((mode(TI)));

extern void func_L20_00443B60(char *, char *, char *, float *);

extern void func_L20_00440198(int, char *, char *, float);

void func_L20_00443CF0(char *arg0) {
    TI128 v1;
    TI128 v2;
    float f = 1.0471977f;
    char *state = *(char **)(arg0 + 0x68);

    func_L20_00443B60(arg0, (char *)&v1, (char *)&v2, &f);
    func_L20_00440198(*(int *)(state + 0x21C), (char *)&v1, (char *)&v2, f);
}
