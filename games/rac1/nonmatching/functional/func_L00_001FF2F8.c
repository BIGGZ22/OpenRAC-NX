/* functional: medium. fcheck: unchecked (level/overlay function; fcheck runs boot-executable functions only; OpenRAC tools/fcheck_gate.py, 2026-10-18). Multiplies the xyz lanes of two VU0 vectors and writes the 16-byte result; w is copied from a. VU0 float multiply rounds and flushes denormals differently from the C FPU in the last bit, and the VU pipeline nop has no visible effect here. */
void func_L00_001FF2F8(float *dst, const float *a, const float *b) {
    float x = a[0] * b[0];
    float y = a[1] * b[1];
    float z = a[2] * b[2];
    float w = a[3];
    dst[0] = x;
    dst[1] = y;
    dst[2] = z;
    dst[3] = w;
}
