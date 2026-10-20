/* from src/core/001163B0.c: 120 bytes, matching retail instruction for instruction
   except one register assignment */
/*
 * In-place string reverse (config/datamine names strrev at 0x1184D0 in the SDK's
 * vfprintf.obj; func_00118548 calls it on its way out). Retail counts the length
 * with `while (s[++len])`, decrements, then swaps s[i] with s[len] while both
 * counters close on each other. The body below, under the *game* compiler
 * (`-O2 -G2 -ffunction-sections`, no `sdk` prefix, in a file of its own),
 * compiles to exactly 120 bytes: same instructions, same order, same three
 * `nop`s in the first loop, same `bnel` with the increment in its delay slot,
 * same frame-less prologue and same `jr $ra`/`daddu $v0` - with one difference,
 * in the whole function: the base pointer. Retail copies the parameter to $t1
 * and counts in $t0; this cc1 copies it to $t0 and counts in $t1.
 *
 * Measured witnesses for the tie (wave 4 added 32 scratch bodies, compiled in
 * four batches; only tools/check.py decides a match):
 *  - the same C under the SDK profile (`sdk -O2 -G2 -ffunction-sections`, the
 *    unit's own line) loses `bnel`, the delay-slot increment and the whole
 *    register order (112 bytes) - this unit is a game-compiler unit;
 *  - `int len = 0; int i = 0;` with the block's `i = 0;` dropped flips the base
 *    to $t1 and the rest of the body then agrees word for word, but the two
 *    initialisations are then emitted in the entry block (a3 in the beqz delay
 *    slot becomes t0) and it is 120 bytes of the wrong layout;
 *  - adding the block's `i = 0;` back makes the compiler delete the entry one
 *    before register allocation, and the base returns to $t0 (exact 120-byte
 *    layout, wrong register);
 *  - `do { len++; } while (s[len])`, `for (len = 1; s[len]; len++)`, a `char *p
 *    = arg0;` base, and both declaration orders all give the same 120 bytes with
 *    $t0 / $t1.
 *
 * Wave 4 adds these, all measured on the same 120-byte shape:
 *  - the tie is decided by ONE knob: whether the counter is live in the entry
 *    block. In every body where `i` is born before the first `beqz` (both
 *    initialisations written in the entry, in either declaration order, with or
 *    without the block's `i = 0;` kept) the base is $t1 and `i` is $t0 -
 *    retail's registers - but the entry then carries the extra move, and reorg
 *    pulls *that* move into the `beqz` delay slot instead of the `len = 0` retail
 *    has there, shifting the body by one instruction;
 *  - `register int i`, `register char c`, `register char *arg0`, the comma
 *    declaration `int len = 0, i = 0;`, `int i = (int)0;`, a redundant
 *    `i = i - i;` in the block and `register char *p = arg0;` all change nothing
 *    at all (byte-identical to the plain body) - the keyword never reaches this
 *    tie;
 *  - a *use* of `i` in the entry is enough on its own: with the block's `i = 0;`
 *    kept and the guard written `if (arg0[i] != 0)`, the base goes to $t1 and `i`
 *    stays $t0 while the initialisation still lands in the `blez` delay slot -
 *    right registers, right slot - but the `arg0[i]` addressing costs an
 *    `addu`/`lb` pair retail does not have (5 words differ; that is a different
 *    program, not a placement difference);
 *  - so the closest bodies are 8 words apart in both directions: retail's
 *    registers with one move in the wrong place, or retail's layout with the two
 *    registers swapped. In the 9-word body everything from the `blez` to the
 *    `jr $ra` is already right; in the 8-word one only the `beqz` slot and the
 *    loop entry differ.
 *
 * Wave 5 adds twenty more bodies, all measured on the same 120-byte shape:
 * the swap written in the three orders (`c = arg0[i]` first, `c = arg0[len]`
 * first, the two stores first), `unsigned char` for the temporary, the swap
 * through `char *p`/`char *q`, `*arg0` and `*(arg0 + ++len)` for the length
 * scan, the counter initialised in the entry, the counter's scope moved into
 * the block, and the declaration order permuted. None of them moves the
 * base pointer out of $t0 while keeping retail's delay-slot layout: the
 * bodies that put the base in $t1 (the counter live in the entry block) shift
 * the body by one instruction, and the bodies that keep retail's layout
 * leave the base in $t0. The tie stands.
 */
char *func_001184D0(char *arg0) {
    int len = 0;
    int i;
    char c;

    if (arg0[0] != 0) {
        while (arg0[++len] != 0) {
        }
    }
    len--;
    if (len > 0) {
        i = 0;
        do {
            c = arg0[i];
            arg0[i] = arg0[len];
            arg0[len] = c;
            i++;
            len--;
        } while (i < len);
    }
    return arg0;
}
