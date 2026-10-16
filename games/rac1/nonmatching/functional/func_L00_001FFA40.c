/* functional: medium. Builds a 4x4 rotation matrix (4 rows of 4 floats at out) from three angle floats at ang[2], ang[1], ang[0] (applied in that order: first a rotation about z, then y, then x; an angle whose bits are 0 is skipped). Starts from identity. sin/cos come from the joined polynomial func_L00_001FFB78 (range-reduce to +-pi/2 by one reflection, both tests on the unreduced angle as retail's delay slot does, then x + c5 x^3 + c6 x^5 + c7 x^7 + c8 x^9; cos(a) = sin(a + pi/2)); written inline. VU0 rounding and the FPU's no-NaN behaviour not checked against hardware. */
void func_L00_001FFA40(float *out, int *ang) {
    float m[4][4];
    float pi = 3.1415927410125732f;
    float hp = 1.5707963705062866f;
    float mpi = -pi;
    float mhp = -hp;
    int k, i, j;
    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++) m[i][j] = (i == j) ? 1.0f : 0.0f;
    for (k = 0; k < 3; k++) {
        int bits = ang[2 - k];
        float a, b, s, c;
        float x, x2, p3, p5, p7, p9;
        float r5[4], r6[4], r7[4];
        if (bits == 0) continue;
        a = *(float *)&ang[2 - k];
        b = a + hp;
        /* sin(a) */
        x = a;
        if (!(a < hp)) x = pi - a;
        if (a < mhp) x = mpi - a;
        x2 = x * x;
        p3 = x * x2;
        p5 = p3 * x2;
        p7 = p5 * x2;
        p9 = p7 * x2;
        s = x + 0.0f;
        s = s + p3 * -0.16666656732559204f;
        s = s + p5 * 0.008333025500178337f;
        s = s + p7 * -0.00019807413627859205f;
        s = s + p9 * 2.601886990305502e-06f;
        /* sin(a + pi/2) = cos(a) */
        x = b;
        if (!(b < hp)) x = pi - b;
        if (b < mhp) x = mpi - b;
        x2 = x * x;
        p3 = x * x2;
        p5 = p3 * x2;
        p7 = p5 * x2;
        p9 = p7 * x2;
        c = x + 0.0f;
        c = c + p3 * -0.16666656732559204f;
        c = c + p5 * 0.008333025500178337f;
        c = c + p7 * -0.00019807413627859205f;
        c = c + p9 * 2.601886990305502e-06f;
        if (k == 0) {
            m[0][0] = 0.0f + c;
            m[0][1] = 0.0f + s;
            m[1][1] = 0.0f + c;
            m[1][0] = 0.0f - s;
        } else {
            for (j = 0; j < 4; j++) r5[j] = r6[j] = r7[j] = 0.0f;
            if (k == 1) {
                r5[0] = 0.0f + c;
                r5[2] = 0.0f - s;
                r6[1] = 0.0f + 1.0f;
                r7[2] = 0.0f + c;
                r7[0] = 0.0f + s;
            } else {
                r5[0] = 0.0f + 1.0f;
                r6[1] = 0.0f + c;
                r6[2] = 0.0f + s;
                r7[2] = 0.0f + c;
                r7[1] = 0.0f - s;
            }
            for (i = 0; i < 3; i++) {
                float rx = m[i][0], ry = m[i][1], rz = m[i][2];
                for (j = 0; j < 4; j++) {
                    float acc = r5[j] * rx;
                    acc = acc + r6[j] * ry;
                    m[i][j] = acc + r7[j] * rz;
                }
            }
        }
    }
    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++) out[i * 4 + j] = m[i][j];
}
