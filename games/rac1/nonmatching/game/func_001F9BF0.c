/* func_001F9BF0(dst, a, b): VU0 vsub.xyz: dst.xyz = a.xyz - b.xyz; the w of a
   is kept (the 16 bytes are stored whole). Both inputs are read before the store. */
void func_001F9BF0(float *dst, float *a, float *b) {
    float x = a[0] - b[0];
    float y = a[1] - b[1];
    float z = a[2] - b[2];
    float w = a[3];

    dst[0] = x;
    dst[1] = y;
    dst[2] = z;
    dst[3] = w;
}
