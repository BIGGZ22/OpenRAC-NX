/* from src/core/001163B0.c: C that compiles to 188 bytes where retail has 112
 * - and the gap is the compiler, not the source.
 *
 * A decimal counter advanced in place, right to left: with a zero length
 * nothing happens and 1 is returned; the last digit is set to '0' when it is
 * '5' or above, the 9s below it are cleared, and the first digit that is not a
 * 9 is incremented (returning the new character) or 0 is returned when that
 * digit is a 9.
 *
 * Retail's control flow is a peeled loop: the length test at +0, the digit
 * test at +0x10, one `sb` of '0' at +0x20, then the 9s loop as a
 * `beql $v0,$a2` back edge at +0x3C, the digit at +0x4C and the two returns
 * at +0x58 and +0x60.
 *
 * The first instruction is `blez $a1, 0x001184D0` - a branch into the next
 * function, func_001184D0 (the SDK's strrev, libc.a/vfprintf.obj in
 * config/datamine/ntsc-u-sdk-matches.tsv), whose result is returned: a
 * sibling call. The rebuilt compiler cannot make one: gcc/calls.c's
 * expand_call keeps `|| 1` under TARGET_ABI_UNSPECIFIED, and
 * -mastra-sibcall (tools/compiler/patches/0022-sibcall-default-off.patch)
 * only replaces the `mips_astra_no_sibcall` term, so the call stays a `jal`
 * with a frame, a `sd $ra` and a `ld $ra` - 76 bytes more than retail. The
 * same C with the call left as a call is what the body below is; the shape
 * of the body (the digit test, the 9s loop, the increment) is measured.
 *
 * With the call written as a plain call the candidate is 188 bytes: the
 * `jal`+frame prologue, then the digit test `slti $v0,$v0,0x35` and the
 * 9s loop, but the loop is not peeled the way retail has it and the final
 * increment returns the character where retail returns it as an int.
 */
extern char *func_001184D0(char *arg0);

int func_00118460(char *buf, int len) {
    if (len <= 0)
        return func_001184D0(buf);
    len--;
    if (buf[len] < '5')
        return 1;
    buf[len] = '0';
    for (;;) {
        len--;
        if (len <= 0)
            break;
        if (buf[len] != '9')
            break;
        buf[len] = '0';
    }
    buf += len;
    if (*buf == '9')
        return 0;
    *buf = *buf + 1;
    return *buf;
}
