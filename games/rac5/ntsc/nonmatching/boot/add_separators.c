/*
 * add_separators — retail 0x01ECBEB0, body 432 bytes (0x01ECBEB0..0x01ECC060).
 *
 * Sony's helper behind the `' (grouping) flag of the libc printf family.
 * Signature confirmed at both call sites (_vfprintf_r 0x01ECB648,
 * _vfiprintf_r 0x01ECC83C):
 *
 *     a0 = destination buffer, a1 = first digit, a2 = one past the last
 *     digit, a3 = &length; the return value replaces the caller's digit
 *     pointer (s4 = $v0 after the call).
 *
 * Semantics recovered from the retail code:
 *   - the text start..end is scanned for '.', 'e' or 'E';
 *   - an exponent ('e'/'E') aborts grouping and the input pointer is
 *     returned unchanged (so the caller keeps its own length);
 *   - otherwise the text is rebuilt backwards into an alloca'd block,
 *     inserting ',' every three digits left of the decimal point (or every
 *     three digits of the whole field when there is no point), then
 *     memcpy'd into buf and *len updated.
 *
 * Measurement (pinned SN ProDG 2.95.3 `ee-gcc2953.exe`, -O2 -G0,
 * Ps2EeAs 2.0, ld.exe 2.0, --end 0x01ECC060):
 *     body 432/432 bytes, equality 432/432, trailing 0 bytes: 0 nonzero
 *     *** EXACT ***
 *
 * Two source details carry the exactness and are easy to "fix" wrongly:
 *   - `count++` belongs INSIDE the `if (!(s.dot < p))` block, which is what
 *     puts it in both skip-branch delay slots and rotates the copy loop
 *     (`lbu` hoisted into the back-branch delay slot at 0x01ECC018);
 *   - the closing character is taken through the cursor (`*q = *p`), not
 *     through the parameter (`*start`).  Writing `*start` shifts the tail
 *     load to $a1 and also makes gcc fold `p` onto `start` in the loop-entry
 *     comparison (0x01ECBF18), costing exactly two bytes.
 */
extern void *memcpy(void *, const void *, unsigned int);
extern void *alloca(unsigned int);

char *add_separators(char *buf, char *start, char *end, int *len) {
    int n = end - start + 10;
    char *q = alloca(n) + n - 1;
    struct { int state; char *dot; } s;
    char *p;
    int count = 0;

    if (!buf)
        return 0;
    if (!start)
        return 0;
    if (!end)
        return 0;

    p = start;
    s.state = 0;
    s.dot = end;
    for (; p <= end && s.state != 2; p++) {
        switch (*p) {
        case '.':
            s.dot = p;
            if (!s.state)
                s.state = 1;
            break;
        case 'E':
        case 'e':
            s.state = 2;
            break;
        }
    }

    if (s.state == 2)
        return start;

    for (p = end; start < p; p--) {
        *q-- = *p;
        if (!(s.dot < p)) {
            if (count % 3 == 0 && count != 0) {
                *q-- = ',';
                n++;
            }
            count++;
        }
    }

    *q = *p;
    n -= 10;
    memcpy(buf, q, n);
    *len = n;
    return buf;
}
