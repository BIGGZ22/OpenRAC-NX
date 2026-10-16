/* Writes the 3x4 identity as three quadwords (vf0 is the fixed (0,0,0,1)):
 * rows (1,0,0,0), (0,1,0,0), (0,0,1,0). */
void func_001FA168(float *m) {
    m[0] = 1.0f;  m[1] = 0.0f;  m[2] = 0.0f;  m[3] = 0.0f;
    m[4] = 0.0f;  m[5] = 1.0f;  m[6] = 0.0f;  m[7] = 0.0f;
    m[8] = 0.0f;  m[9] = 0.0f;  m[10] = 1.0f; m[11] = 0.0f;
}
