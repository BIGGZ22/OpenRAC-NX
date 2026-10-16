/* Stores four 16-byte rows at m: the (0,0,0,1) vector in vf4 and its
 * vmr32 rotations (y,z,w,x) in vf5, vf6, vf7. Row 3 (+0x30) is written
 * first, row 0 (+0x00) last. */
void func_00125358(float *m)
{
    char *p = (char *)m;
    float *r;

    r = (float *)(p + 0x30);
    r[0] = 0.0f; r[1] = 0.0f; r[2] = 0.0f; r[3] = 1.0f;
    r = (float *)(p + 0x20);
    r[0] = 0.0f; r[1] = 0.0f; r[2] = 1.0f; r[3] = 0.0f;
    r = (float *)(p + 0x10);
    r[0] = 0.0f; r[1] = 1.0f; r[2] = 0.0f; r[3] = 0.0f;
    r = (float *)(p + 0x00);
    r[0] = 1.0f; r[1] = 0.0f; r[2] = 0.0f; r[3] = 0.0f;
}
