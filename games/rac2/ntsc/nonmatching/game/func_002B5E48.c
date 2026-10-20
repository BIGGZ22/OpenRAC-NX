/* func_002B5E48 (boot function, 176 bytes; 20 levels, 21 places in all): from
 * src/game/002B5278.c, compiled -O2 -G0 (the file has no cflags line).
 * Near miss: compiles to 176 bytes, the size of retail's; 17 of its 44 words differ from
 * retail (the first attempt left 32). Fixed against that attempt: $s1 = moby, $s0 = moby + 0x60
 * and $s2 = obj + 0x10 now sit in retail's registers, because moby + 0x60 is written inline
 * three times instead of held in a named pointer.
 *
 * What is left comes from ONE cause, the recorded 16-byte copy wall:
 *   - retail has `addiu $s2,$a2,0x10 ; lq $v0,0($s2) ; sq $v0,0($sp)`; this cc1 gives
 *     `lq $v0,0x10($a0)` (`.word 0x78820010` in the diff) and puts the add after it. The
 *     fold is cse's find_best_addr: an address that is a register known to be (plus reg 16)
 *     is rewritten to the plus form whenever the two cost the same, and on MIPS a register and
 *     (plus reg small_int) both cost 1 (ADDRESS_COST), so the tie goes to the plus. It happens for
 *     every mode (a plain int load after `p = o + 16` folds the same way), not only TImode.
 *   - the missing `daddu $a2,$a0,$zero` on entry, and the one-word shift of the prologue after
 *     it, are what the fold does to the allocation: it gives obj a third reference (the lq
 *     address), global-alloc ranks pseudos by refs * floor_log2(refs) / live length, so obj
 *     (3 refs, length 9) is allocated before id (3 refs, length 10) and takes $a0, which
 *     leaves id in $v1. Retail's obj has two references (the entry copy and the add), so it
 *     would rank below id and take $a2, the other register it prefers (the dump lists
 *     preferences 4 and 6 for it). This second step is inferred from the dumps, not measured.
 * Tried without effect on the 17 words (all give the same bytes): the pointer formed as
 * `s2 = obj; s2 += 0x10`, as `(char *)((int)obj + 0x10)`, as `(char *)((TI *)obj + 1)`, through
 * a do-while(0) and through 64-bit arithmetic (`(char *)(int)((long long)obj + 0x10)`); the id
 * read once into a local or twice from memory. Forming the pointer
 * at the top of the function, before the two early returns, gives 172 bytes (the add moves into
 * the first branch delay slot) and still folds.
 * Not tried, because they are outside the rules: the qcopy helpers the notes mention (inline
 * asm) and a volatile source.
 * What would unblock it: a compiler-side change to find_best_addr for TImode (or retail's own
 * address cost), or a source form in which cse does not know the register as (plus obj 16).
 */
typedef int TI128_002B5E48 __attribute__((mode(TI)));
typedef union { TI128_002B5E48 q; float f[4]; } Vec4Q_002B5E48;

extern int func_002C9218(int a0, void *a1, void *a2, float f);
extern void func_00282CF0(void *, void *, void *);
extern void func_00282F20(char *, char *, float);

int func_002B5E48(char *obj, char *moby)
{
    Vec4Q_002B5E48 v;
    char *s2;
    int id;

    id = *(int *)(moby + 0x3C);
    if (id == -1) return 0;
    if (*(unsigned int *)(moby + 0x9C) & 1) return 0;
    s2 = obj + 0x10;
    v.q = *(TI128_002B5E48 *)s2;
    if (func_002C9218(id, &v, s2, *(float *)moby) != 0) {
        func_00282CF0(moby + 0x60, &v, s2);
        func_00282F20(moby + 0x60, moby + 0x60, *(float *)moby);
        *(unsigned int *)(moby + 0x94) |= 0x10;
        return 1;
    }
    return 0;
}
