/* Veldin level code: bounding-sphere visibility test against the view block at D_L00_0016CDC0
 * (retail func_L00_00200290, 0xE8 bytes, VU0). sphere = x, y, z, radius (floats); far = far distance.
 * Returns 1 when visible, -1 when the depth or the first side test culls it (retail func_L00_001F3980),
 * 0 when the second side test rejects it (retail func_L00_001F39B0). The view block holds the camera
 * position in row 3 (scaled by k = row3.w), the view axes in rows 0..2 and the tangent/sec terms in rows 4..7. */
extern f32 D_L00_0016CDC0[];

s32 func_L00_00200290(void *sphere, f32 far) {
    f32 *v = (f32 *)sphere;
    f32 *m = D_L00_0016CDC0;
    f32 k, px, py, pz, pr, cx, cy, cz, X, Y, Z, Rk, ax, ay;
    f32 t8x, t8y, t7x, t7y, vf1x, vf1y, vf3x, vf3y, vf5x, vf5y, vf9x, vf9y;
    f32 vf6x, vf6y, vf8x, vf8y, vf4x, vf4y, vf7x, vf7y;

    k = m[15];
    px = v[0] * k;
    py = v[1] * k;
    pz = v[2] * k;
    pr = v[3] * k;
    cx = px - m[12];
    cy = py - m[13];
    cz = pz - m[14];
    X = (m[0] * cx + m[4] * cy) + m[8] * cz;
    Y = (m[1] * cx + m[5] * cy) + m[9] * cz;
    Z = (m[2] * cx + m[6] * cy) + m[10] * cz;
    Rk = far * k;
    *(s32 *)&ax = *(s32 *)&X & 0x7FFFFFFF;
    *(s32 *)&ay = *(s32 *)&Y & 0x7FFFFFFF;
    t8x = m[24] * pr;
    t8y = m[25] * pr;
    t7x = t8x * m[26];
    t7y = t8y * m[26];
    vf1x = (0.0f + pr) + Z;
    vf1y = (0.0f - pr) + Z;
    vf3x = 0.0f - vf1x;
    vf3y = Rk - vf1y;
    vf5x = m[16] * Z;
    vf5y = m[17] * Z;
    vf9x = vf5x * m[20];
    vf9y = vf5y * m[21];
    vf6x = ax - t8x;
    vf6y = ay - t8y;
    vf8x = ax + t7x;
    vf8y = ay + t7y;
    vf4x = vf1x - m[28];
    vf4y = vf1y - m[29];
    vf7x = vf5x - vf6x;
    vf7y = vf5y - vf6y;
    vf8x = vf9x - vf8x;
    vf8y = vf9y - vf8y;

    if (*(s32 *)&vf3y < 0 || *(s32 *)&vf3x >= 0 || *(s32 *)&vf7y < 0 || *(s32 *)&vf7x < 0)
        return -1;
    if (*(s32 *)&vf8y < 0 || *(s32 *)&vf4y < 0 || *(s32 *)&vf8x < 0 || *(s32 *)&vf4x >= 0)
        return 0;
    return 1;
}
