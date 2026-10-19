/* functional: medium. fcheck: unchecked (never called in fcheck's capture run of the title, menu and level 0; OpenRAC tools/fcheck_gate.py, 2026-10-18). Corrected in place: the w column of the first two rows is 0 (the VU starts every row from 0 and only sets x or y to 1), the file had 1. Builds the 3x3 rotation matrix of the quaternion (x,y,z,w) at src (2xy, 2wz and so on, in the VU's evaluation order) as three rows of xyzw, with the fourth row (0,0,0,1); stored as four quadwords at dst. Rounding order inside the VU may differ in the last bit. */
void func_001FA6C0(float *src, float *dst)
{
    float x = src[0], y = src[1], z = src[2], w = src[3];
    float xx = 2.0f * x * x, yy = 2.0f * y * y, zz = 2.0f * z * z;
    float xy = 2.0f * x * y, xz = 2.0f * x * z, yz = 2.0f * y * z;
    float wx = 2.0f * w * x, wy = 2.0f * w * y, wz = 2.0f * w * z;

    dst[0] = 1.0f - yy - zz;
    dst[1] = xy - wz;
    dst[2] = xz + wy;
    dst[3] = 0.0f;

    dst[4] = xy + wz;
    dst[5] = 1.0f - xx - zz;
    dst[6] = yz - wx;
    dst[7] = 0.0f;

    dst[8] = xz - wy;
    dst[9] = yz + wx;
    dst[10] = 1.0f - xx - yy;
    dst[11] = 0.0f;

    dst[12] = 0.0f;
    dst[13] = 0.0f;
    dst[14] = 0.0f;
    dst[15] = 1.0f;
}
