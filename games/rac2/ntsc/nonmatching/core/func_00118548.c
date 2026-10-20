/* from src/core/001163B0.c: C that compiles to 156 bytes where retail has 156
 * - the same size, but 19 words differ.
 *
 * Itoa, and the unsigned arithmetic is settled: retail divides with `divu`
 * and negates with `negu` + `movz $a0,$v1,$v0` (v0 = base ^ 10), which only
 * the unsigned spellings produce - a signed `int value` compiles to `div` and
 * `movn`. The retained draft is the unsigned one.
 *
 * What is left is the shape of the negation and one scheduling choice, both
 * compiler-side. Retail resolves the "negative and base 10" case without a
 * copy of the parameter: it keeps the value in $a0 (the parameter's own
 * register) and if-converts the inner `if (base == 10)` into
 * `negu $v1,$a0; xori $v0,$a2,0xA; movz $a0,$v1,$v0`, so the "then" value
 * (the negation) sits in $v1 and the select picks it into $a0. This cc1
 * either
 *   - copies the parameter to $a3 first and selects with the mirror image
 *     `movn $a3,$a0,$v0` ("if (base != 10) keep the original"), 160 bytes
 *     (the `-value` spelling, the previous draft), or
 *   - keeps the value in $a0 and leaves the inner `if` a branch, computing
 *     the negation as `nor $v0,$zero,$a0; addiu $a0,$v0,1` (the
 *     `~value + 1` spelling): exactly 156 bytes, the retail's size, and the
 *     loop from 0x118588 on matches retail word for word.
 * The retained draft is the 156-byte one. Its two remaining differences from
 * retail: the negation is a branch plus two instructions where retail has
 * the if-converted `negu`+`movz` (three instructions, one of them a select),
 * and retail keeps the `sq $ra,0($sp)` frame save in the delay slot of the
 * sign test while this cc1 emits it before the test.
 *
 * Profiles: the unit's own line is `sdk -O2 -G2 -ffunction-sections` and the
 * SDK compiler gives the same 160 bytes with the same shape; the game
 * compiler (`-O2 -G2 -ffunction-sections`, with `-mastra-quad-saves` for the
 * `sq`/`lq` frame slots retail uses) is the one that reproduces retail's
 * frame save. This unit therefore needs the game compiler, like the rest of
 * the vfprintf.obj group.
 */
extern char D_00134640[];

extern char *func_001184D0(char *arg0);

char *func_00118548(unsigned int value, char *buf, unsigned int base) {
    unsigned int neg = value >> 31;
    int len = 0;

    if (neg) {
        if (base == 10)
            value = ~value + 1;
    }
    do {
        buf[len++] = D_00134640[value % base];
        value /= base;
    } while (value != 0);
    if (neg)
        buf[len++] = '-';
    buf[len] = '\0';
    return func_001184D0(buf);
}
