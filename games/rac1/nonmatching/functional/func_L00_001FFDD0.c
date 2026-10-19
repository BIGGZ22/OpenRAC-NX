/* functional: medium. fcheck: unchecked (level/overlay function; fcheck runs boot-executable functions only; OpenRAC tools/fcheck_gate.py, 2026-10-18). dst[0..15] = a[0..15] + b[0..15], element by element, as the four VU0 vadd.xyzw groups do; VU0 rounds the float sum in the last bit where C may differ. */
void func_L00_001FFDD0(float *dst, float *a, float *b) {
    int i;
    for (i = 0; i < 16; i++) {
        dst[i] = a[i] + b[i];
    }
}
