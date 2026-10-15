/* matrix_mul_vec3: out = m * v for a 4x3 matrix m given by its three columns
   (floats 0..3, 4..7, 8..11), v = (x, y, z, w). xyz get m*v; w gets the
   m-row-w dot v plus v.w (the vf0 = (0,0,0,1) term of the vector unit). */
void func_001F9EC0(void *out, void *v, void *m)
{
    float *o = (float *)out;
    float *vv = (float *)v;
    float *mm = (float *)m;
    float vx = vv[0];
    float vy = vv[1];
    float vz = vv[2];
    float vw = vv[3];
    float acc0, acc1, acc2, acc3;

    acc0 = mm[0] * vx;
    acc1 = mm[1] * vx;
    acc2 = mm[2] * vx;
    acc3 = mm[3] * vx;
    acc0 = acc0 + mm[4] * vy;
    acc1 = acc1 + mm[5] * vy;
    acc2 = acc2 + mm[6] * vy;
    acc3 = acc3 + mm[7] * vy;
    acc0 = acc0 + mm[8] * vz;
    acc1 = acc1 + mm[9] * vz;
    acc2 = acc2 + mm[10] * vz;
    acc3 = acc3 + mm[11] * vz;
    o[0] = acc0 + 0.0f * vw;
    o[1] = acc1 + 0.0f * vw;
    o[2] = acc2 + 0.0f * vw;
    o[3] = acc3 + 1.0f * vw;
}
