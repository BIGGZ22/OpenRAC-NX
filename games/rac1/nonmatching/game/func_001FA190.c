/* Writes the 4x4 identity matrix (row-major, four quadword stores) to m. */
void func_001FA190(void *m) {
    float *f = (float *)m;
    f[0] = 1.0f; f[1] = 0.0f; f[2] = 0.0f; f[3] = 0.0f;
    f[4] = 0.0f; f[5] = 1.0f; f[6] = 0.0f; f[7] = 0.0f;
    f[8] = 0.0f; f[9] = 0.0f; f[10] = 1.0f; f[11] = 0.0f;
    f[12] = 0.0f; f[13] = 0.0f; f[14] = 0.0f; f[15] = 1.0f;
}
