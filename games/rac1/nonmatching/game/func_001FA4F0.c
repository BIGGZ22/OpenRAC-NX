/* out (3 columns of 4 floats) = A * B, where A is the 4x3 block at src (its three
   columns are vf4..vf6) and B the block at b (three columns vf1..vf3). Each lane:
   out[j][i] = (A0[i]*B[j][0] + A1[i]*B[j][1]) + A2[i]*B[j][2], as the VU0 does it. */
void func_001FA4F0(float *out, const float *src, const float *b) {
    int j, i;
    for (j = 0; j < 3; j++) {
        for (i = 0; i < 4; i++) {
            float t = src[i] * b[4 * j + 0];
            t = t + src[4 + i] * b[4 * j + 1];
            out[4 * j + i] = t + src[8 + i] * b[4 * j + 2];
        }
    }
}
