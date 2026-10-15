/* Writes a 4x4 matrix with s on the first three diagonal entries and
   1 in the last entry: m = diag(s, s, s, 1), row by row (VU0 layout). */
void func_001FA1C0(float *m, float s)
{
    m[0] = 0.0f + s;
    m[1] = 0.0f;
    m[2] = 0.0f;
    m[3] = 0.0f;
    m[4] = 0.0f;
    m[5] = 0.0f + s;
    m[6] = 0.0f;
    m[7] = 0.0f;
    m[8] = 0.0f;
    m[9] = 0.0f;
    m[10] = 0.0f + s;
    m[11] = 0.0f;
    m[12] = 0.0f;
    m[13] = 0.0f;
    m[14] = 0.0f;
    m[15] = 1.0f;
}
