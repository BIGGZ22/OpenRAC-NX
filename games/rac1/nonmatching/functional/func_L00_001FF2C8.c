/* functional: medium. Written from the assembly, not compiled. Retail multiplies in VU0 single-precision arithmetic (its own rounding and denormal flush), which plain float multiplication may differ from in the last bit; the reads, writes and w-lane preservation are the same. */
void func_L00_001FF2C8(int unused, float s, float *v) {
    v[0] = v[0] * s;
    v[1] = v[1] * s;
    v[2] = v[2] * s;
}
