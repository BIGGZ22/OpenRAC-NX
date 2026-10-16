/* euler_to_matrix: rotation rows from Euler angles in[0..2] (x, y, z), out = 4 rows of 4
   floats. Retail loads in into vf1 and runs VU0 microprogram 28259 at 0xD18 (vcallms 0x1A3),
   then stores vf20..vf23. That program (ReRAC rc-formats moby_light.rs rotation_rows and
   vu0_sin_cos) starts from the identity and applies Rx, then Ry, then Rz, each step skipped
   when its angle is zero (exponent bits zero), so row i = R.e_i with R = Rz.Ry.Rx. Sine and
   cosine fold the angle into [-pi/2, pi/2] with min/max and use a 9th-order odd polynomial;
   cosine is the sine of angle + pi/2. Row 3 (vf23) is (0, 0, 0, 1). */
void func_001FA218(float *out, float *in) {
    union { unsigned int u; float f; } cv;
    float k[7];
    float r[3][4];
    float m[3][4];
    float nr[4];
    int step, i, j, n;

    /* pi/2, pi, -pi as the microprogram holds them, then the x^3, x^5, x^7, x^9 coefficients */
    cv.u = 0x3FC90FDA; k[0] = cv.f;
    cv.u = 0x40490FDA; k[1] = cv.f;
    cv.u = 0xC0490FDA; k[2] = cv.f;
    cv.u = 0xBE2AAAA4; k[3] = cv.f;
    cv.u = 0x3C08873E; k[4] = cv.f;
    cv.u = 0xB94FB21D; k[5] = cv.f;
    cv.u = 0x362E9C14; k[6] = cv.f;
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 4; j++) {
            r[i][j] = (i == j) ? 1.0f : 0.0f;
        }
    }

    for (step = 0; step < 3; step++) {
        float a = in[step];
        float sc[2];

        cv.f = a;
        if ((cv.u & 0x7F800000) == 0) {
            continue;
        }
        for (n = 0; n < 2; n++) {
            float x = (n == 0) ? a : a + k[0];
            float t = k[1] - x;
            float x2, x3, x5, x7, x9, acc;
            if (!(t < x)) {
                t = x;
            }
            x = k[2] - x;
            if (t > x) {
                x = t;
            }
            x2 = x * x;
            x3 = x * x2;
            x5 = x3 * x2;
            x7 = x5 * x2;
            x9 = x7 * x2;
            acc = x * 1.0f;
            acc = acc + x3 * k[3];
            acc = acc + x5 * k[4];
            acc = acc + x7 * k[5];
            acc = acc + x9 * k[6];
            sc[n] = acc;
        }
        /* sc[0] = sine, sc[1] = cosine */
        if (step == 0) {
            r[1][1] = 0.0f + sc[1];
            r[1][2] = 0.0f + sc[0];
            r[2][2] = 0.0f + sc[1];
            r[2][1] = 0.0f - sc[0];
            continue;
        }
        for (i = 0; i < 3; i++) {
            for (j = 0; j < 4; j++) {
                m[i][j] = 0.0f;
            }
        }
        if (step == 1) {
            m[0][0] = 0.0f + sc[1];
            m[0][2] = 0.0f - sc[0];
            m[1][1] = 1.0f;
            m[2][0] = 0.0f + sc[0];
            m[2][2] = 0.0f + sc[1];
        } else {
            m[0][0] = 0.0f + sc[1];
            m[0][1] = 0.0f + sc[0];
            m[1][0] = 0.0f - sc[0];
            m[1][1] = 0.0f + sc[1];
            m[2][2] = 1.0f;
        }
        for (i = 0; i < 3; i++) {
            for (j = 0; j < 4; j++) {
                nr[j] = (m[0][j] * r[i][0] + m[1][j] * r[i][1]) + m[2][j] * r[i][2];
            }
            for (j = 0; j < 4; j++) {
                r[i][j] = nr[j];
            }
        }
    }

    for (i = 0; i < 3; i++) {
        for (j = 0; j < 4; j++) {
            out[i * 4 + j] = r[i][j];
        }
    }
    out[12] = 0.0f;
    out[13] = 0.0f;
    out[14] = 0.0f;
    out[15] = 1.0f;
}
