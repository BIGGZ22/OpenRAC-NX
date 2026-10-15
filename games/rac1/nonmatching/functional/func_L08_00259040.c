/* functional: high. Exact logic of handwritten PS2 VU0 UV shift assembly func_L08_00259040.
 * Offsets count 2D UV float pairs from src to dst by (offset[0], offset[1]).
 */
void func_L08_00259040(float *dst, const float *src, int count, const float *offset) {
    float du = offset[0];
    float dv = offset[1];
    int i;

    for (i = 0; i < count; i++) {
        dst[i * 2 + 0] = src[i * 2 + 0] + du;
        dst[i * 2 + 1] = src[i * 2 + 1] + dv;
    }
}
