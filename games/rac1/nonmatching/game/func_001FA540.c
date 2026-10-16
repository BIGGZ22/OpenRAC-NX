/* func_001FA540(dst, m, src): four vectors out of four: dst = m * src, with m
   as four columns (VU0: column 0 times x, plus column 1 times y, plus column 2
   times z, plus column 3 times w, accumulated in ACC in that order). The
   columns are read before the first store, as the VU registers are. */
void func_001FA540(float *dst, float *m, float *src) {
    float c0[4];
    float c1[4];
    float c2[4];
    float c3[4];
    float *end = src + 16;
    int k;

    for (k = 0; k < 4; k++) {
        c0[k] = m[k];
        c1[k] = m[4 + k];
        c2[k] = m[8 + k];
        c3[k] = m[12 + k];
    }
    do {
        float x = src[0];
        float y = src[1];
        float z = src[2];
        float w = src[3];

        for (k = 0; k < 4; k++) {
            float a = c0[k] * x;

            a = a + c1[k] * y;
            a = a + c2[k] * z;
            dst[k] = a + c3[k] * w;
        }
        dst += 4;
        src += 4;
    } while (src != end);
}
