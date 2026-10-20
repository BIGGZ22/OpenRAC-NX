/* from src/core/001232F0.c: 216 bytes against retail's 228, twelve bytes short.
 *
 * libgcc frame.c: find_fde, transcribed from the same public source the
 * qualified compiler is built from (gnu-ee-binutils-gcc-1.1, gcc/frame.c).
 * The object walk, the lazy frame_init func_00123A00, the binary search and
 * every instruction of them are identical to retail's. What is missing is
 * retail's null test between the loop and the search:
 *
 *   retail  ...pc < pc_end test fails to repeat the loop, then
 *           bnel $s0,$zero,+0x14 ; lw $a3,0x10($s0)   # if (ob != 0) goto body
 *           b  <return 0> ; daddu $v0,$zero,$zero
 *   ours    falls straight through into `hi = ob->count`, which the compiler
 *           hoists into the branch delay slot with no test at all.
 *
 * The test is the source's `if (ob == 0) return 0;` after the loop. Both
 * qualified profiles delete it: on the break edge the loop's own `ob != 0`
 * test dominates, so the redundancy eliminator removes the second test
 * (retail's compiler kept it, and this is a compiler behaviour, not a source
 * shape - the loop is a top-tested `for (ob = objects; ob; ob = ob->next)`
 * whose back edges branch to that test, exactly as in the source).
 *
 * Tried: hoisting `hi = ob->count` out of the for-init into its own statement
 * after the check (no change); the default profile -O2 -G0 (wrong frame,
 * 0x20 against retail's 0x30, and the search body stops matching); the
 * sdk -O2 -g2 -gstabs profile (prologue order changes). -fno-delete-null-
 * pointer-checks is rejected by cc1 as an invalid option.
 *
 * What would unblock it: a compiler/flag that does not fold the second test,
 * or confirmation that this object is one the qualified tree builds with a
 * different profile. `objects` is retail's D_00141838, `struct object` is
 * frame.h's (pc_begin, pc_end, fde_begin, fde_array, count, next).
 *
 * Verified: tools/check.py func_00123B40 --diff: boot: 216 bytes, expected
 * 228.
 */

typedef struct
{
  u32 length;
  s32 CIE_delta;
  void *pc_begin;
  u32 pc_range;
} fde;

struct object
{
  void *pc_begin;
  void *pc_end;
  fde *fde_begin;
  fde **fde_array;
  u32 count;
  struct object *next;
};

extern struct object *D_00141838;

extern void func_00123A00(struct object *);

fde *func_00123B40(void *pc) {
    struct object *ob;
    u32 lo, hi;

    for (ob = D_00141838; ob; ob = ob->next) {
        if (ob->pc_begin == 0)
            func_00123A00(ob);
        if (pc >= ob->pc_begin && pc < ob->pc_end)
            break;
    }

    if (ob == 0)
        return 0;

    hi = ob->count;
    for (lo = 0; lo < hi;) {
        u32 i = (lo + hi) / 2;
        fde *f = ob->fde_array[i];

        if (pc < f->pc_begin)
            hi = i;
        else if (pc >= f->pc_begin + f->pc_range)
            lo = i + 1;
        else
            return f;
    }

    return 0;
}
