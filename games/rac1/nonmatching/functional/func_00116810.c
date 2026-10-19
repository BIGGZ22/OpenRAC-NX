/* functional: medium. fcheck: SAME on 3 calls (OpenRAC tools/fcheck_gate.py, 2026-10-18). Plain byte-wise strlen. Retail (MMI quadword code) returns the same count, but it reads aligned 8/16-byte blocks, so its read pattern differs from this loop; results match for every valid NUL-terminated string. */
unsigned int func_00116810(const char *s) {
    const char *p = s;
    while (*p != 0) {
        p++;
    }
    return (unsigned int)(p - s);
}
