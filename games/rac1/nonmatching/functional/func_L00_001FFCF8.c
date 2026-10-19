/* functional: medium. fcheck: unchecked (level/overlay function; fcheck runs boot-executable functions only; OpenRAC tools/fcheck_gate.py, 2026-10-18). Multiplies the 16 floats at a1 by the float in f12 and stores them at a0, which the VU0 does with vmulx on four quadwords; VU float multiply rounding may differ from C's in the last bit. */
void func_L00_001FFCF8(float *dst, float *src, float s) {
    int i;
    for (i = 0; i < 16; i++) {
        dst[i] = src[i] * s;
    }
}
