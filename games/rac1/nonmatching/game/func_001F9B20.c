/* Clip test of the vector at a0 against |w| (vclipw.xyz vf1, vf1w), returning
 * the six flag bits of that test in the order +x, -x, +y, -y, +z, -z (the low
 * six bits of CLIPFLAG, which cfc2 vi18 & 0x3F reads). */
s32 func_001F9B20(f32 *v)
{
    f32 w = v[3];
    s32 flags = 0;

    if (w < 0.0f)
        w = -w;
    if (v[0] > w)
        flags |= 1;
    if (v[0] < -w)
        flags |= 2;
    if (v[1] > w)
        flags |= 4;
    if (v[1] < -w)
        flags |= 8;
    if (v[2] > w)
        flags |= 16;
    if (v[2] < -w)
        flags |= 32;
    return flags;
}
