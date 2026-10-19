/* functional: medium. fcheck: unchecked (level/overlay function; fcheck runs boot-executable functions only; OpenRAC tools/fcheck_gate.py, 2026-10-18). Written from the assembly, not compiled. Retail does this in VU0 single-precision arithmetic (its own rounding and denormal flush), which plain float subtraction may differ from in the last bit; the reads, writes and w-lane preservation are the same. */
void func_L00_001FF270(int unused, float *a, float *b) {
    a[0] = a[0] - b[0];
    a[1] = a[1] - b[1];
    a[2] = a[2] - b[2];
}
