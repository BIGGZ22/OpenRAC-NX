/* functional: high. fcheck: unchecked (never called in fcheck's capture run of the title, menu and level 0; OpenRAC tools/fcheck_gate.py, 2026-10-18). Rotation matrix of the quaternion q = (x, y, z, w) at arg0, not normalised (retail does no normalising): rows 0..2 of four floats at arg1 (+0, +0x10, +0x20), each with w = 0; the fourth row at +0x30 is not written. Matches the port's hand C in port/game/rac1-pal/hand/game_fastfunc.c entry by entry. */
void func_001FA648(const float *q, float *m) {
    float x = q[0], y = q[1], z = q[2], w = q[3];
    float x2 = x + x, y2 = y + y, z2 = z + z;
    float wx = x2 * w, wy = y2 * w, wz = z2 * w;
    float xx = x2 * x, yx = y2 * x, zx = z2 * x;
    float yy = y2 * y, zy = z2 * y;
    float zz = z2 * z;

    m[0] = 1.0f - yy - zz;
    m[1] = yx - wz;
    m[2] = zx + wy;
    m[3] = 0.0f;
    m[4] = wz + yx;
    m[5] = 1.0f - xx - zz;
    m[6] = zy - wx;
    m[7] = 0.0f;
    m[8] = zx - wy;
    m[9] = wx + zy;
    m[10] = 1.0f - xx - yy;
    m[11] = 0.0f;
}
