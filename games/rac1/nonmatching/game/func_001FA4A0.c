/* Writes the 4x4 matrix whose first three rows are the transpose of the three vectors at in
   (in[0..3], in[4..7], in[8..11]; their fourth lanes are ignored), with the last row (0, 0, 0, 1).
   Each lane is 0 + the source lane, as the VU0 adds are, and the w lanes of rows 0-2 are 1 - 1. */
void func_001FA4A0(float *out, float *in) {
    float a0 = 0.0f + in[0];
    float a1 = 0.0f + in[1];
    float a2 = 0.0f + in[2];
    float b0 = 0.0f + in[4];
    float b1 = 0.0f + in[5];
    float b2 = 0.0f + in[6];
    float c0 = 0.0f + in[8];
    float c1 = 0.0f + in[9];
    float c2 = 0.0f + in[10];

    out[0] = a0;
    out[1] = b0;
    out[2] = c0;
    out[3] = 1.0f - 1.0f;
    out[4] = a1;
    out[5] = b1;
    out[6] = c1;
    out[7] = 1.0f - 1.0f;
    out[8] = a2;
    out[9] = b2;
    out[10] = c2;
    out[11] = 1.0f - 1.0f;
    out[12] = 0.0f;
    out[13] = 0.0f;
    out[14] = 0.0f;
    out[15] = 1.0f;
}
