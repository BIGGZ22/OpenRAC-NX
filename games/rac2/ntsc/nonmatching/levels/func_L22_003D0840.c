/* Near miss from src/levels/22_dobbo_orbit/L22_00358778.c — 1 word (4 bytes) differ.
 *
 * Kept as a starting point; not linked, not counted.
 *
 * The body below compiles to retail's exact size (60 bytes) and instruction for
 * instruction, except the one `daddu` that copies the parameter, where GCC
 * allocates $v1 and retail has $a3:
 *
 *   003D0844  daddu $v1, $a0, $zero   |  daddu $a3, $a0, $zero
 *
 * Two things the retail bytes settle, both easy to get backwards:
 *   - `bnez $v0` at 0x3D0850 carries `ld $ra, 0x0($sp)` in its delay slot, so the
 *     branch leaves the early return already half done.
 *   - `sh $v0, 0x34($a3)` at 0x3D086C is in the delay slot of `jal`, so it runs
 *     *before* the call. Reading the halfword, OR-ing 0x41 and storing it are all
 *     ahead of func_31FBB8, not after it.
 *
 * An earlier draft here passed four arguments to func_31FBB8 and came out 64
 * bytes; the call takes three.
 *
 * Ten wordings were tried and all produce $v1: `if (...) { }` and early-return
 * shapes, `|=` against a separate temporary and against a stored value, a `u16 *`
 * alias, and the whole body with `s32` in place of `char *`. Getting $a3 needs
 * whatever makes GCC treat $a3 as an ordinary temporary rather than an incoming
 * argument register, which this source shape does not: the call takes exactly one
 * pointer, so $a0-$a2 are seeded first and $v1 is the next free hard register.
 *
 * Not yet tried: a wording where the copy is not a plain assignment (a cast
 * through a wider type, or the parameter being read once before it is copied).
 */

/* Standalone declarations recovered from src/levels/22_dobbo_orbit/L22_00358778.c. */
typedef unsigned char u8;

typedef unsigned short u16;

typedef int s32;

extern void func_31FBB8();

void func_L22_003D0840(s32 a0) {
    s32 a3 = a0;
    if (*(u8 *)(a3 + 0x20) == 0) {
        u16 v = *(u16 *)(a3 + 0x34) | 0x41;
        *(u16 *)(a3 + 0x34) = v;
        func_31FBB8(a3, 1, -1);
    }
}