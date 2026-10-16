/* Rotation matrix from three angles in radians: angles[0] about X, angles[1]
   about Y, angles[2] about Z. Writes the 4x4 matrix as four column vectors
   (16 floats, column-major) to out. Starts from the identity and applies Z,
   then Y, then X (M = Rx * Ry * Rz); an angle whose bits are zero is skipped.
   sin(t) is retail's polynomial on t reduced to [-pi/2, pi/2]; cos(t) is
   sin(t + pi/2) by the same code. */
void func_001FA238(float *out, const float *angles) {
    const float PI = 3.14159274f;
    const float HALF_PI = 1.57079637f;
    const float K1 = -0.166666567f;
    const float K2 = 0.0083330255f;
    const float K3 = -0.000198074136f;
    const float K4 = 2.60188699e-06f;
    float m[4][4];   /* m[c][r]: column c, lane r; column 3 is the translation */
    int c;
    int r;
    int idx;

    for (c = 0; c < 4; c++) {
        for (r = 0; r < 4; r++) {
            m[c][r] = (c == r) ? 1.0f : 0.0f;
        }
    }

    for (idx = 0; idx < 3; idx++) {
        int ai = 2 - idx;   /* idx 0 is Z (angles[2]), then Y, then X */
        union {
            float f;
            unsigned int u;
        } bits;
        float th;
        float sv = 0.0f;
        float cv = 0.0f;
        float A[4];
        float B[4];
        float Cc[4];
        int k;

        bits.f = angles[ai];
        if (bits.u == 0) {
            continue;
        }
        th = angles[ai];

        for (k = 0; k < 2; k++) {
            float x = (k == 0) ? th : th + HALF_PI;
            float x2;
            float x3;
            float x5;
            float x7;
            float x9;
            float acc;

            if (x < HALF_PI) {
                if (x < -HALF_PI) {
                    x = -PI - x;
                }
            } else {
                x = PI - x;
            }
            x2 = x * x;
            x3 = x * x2;
            x5 = x3 * x2;
            x7 = x5 * x2;
            x9 = x7 * x2;
            acc = x + x3 * K1;
            acc = acc + x5 * K2;
            acc = acc + x7 * K3;
            acc = acc + x9 * K4;
            if (k == 0) {
                sv = acc;
            } else {
                cv = acc;
            }
        }

        /* Columns of the rotation: A = R e_x, B = R e_y, Cc = R e_z. */
        if (ai == 2) {
            A[0] = cv;   A[1] = sv;   A[2] = 0.0f;  A[3] = 0.0f;
            B[0] = 0.0f - sv; B[1] = cv; B[2] = 0.0f; B[3] = 0.0f;
            Cc[0] = 0.0f; Cc[1] = 0.0f; Cc[2] = 1.0f; Cc[3] = 0.0f;
        } else if (ai == 1) {
            A[0] = cv;   A[1] = 0.0f; A[2] = 0.0f - sv; A[3] = 0.0f;
            B[0] = 0.0f; B[1] = 1.0f; B[2] = 0.0f;      B[3] = 0.0f;
            Cc[0] = sv;  Cc[1] = 0.0f; Cc[2] = cv;      Cc[3] = 0.0f;
        } else {
            A[0] = 1.0f; A[1] = 0.0f; A[2] = 0.0f;      A[3] = 0.0f;
            B[0] = 0.0f; B[1] = cv;   B[2] = sv;        B[3] = 0.0f;
            Cc[0] = 0.0f; Cc[1] = 0.0f - sv; Cc[2] = cv; Cc[3] = 0.0f;
        }

        /* Each of the first three columns becomes R times itself. */
        for (c = 0; c < 3; c++) {
            float v[4];
            float nv[4];
            int lane;

            for (lane = 0; lane < 4; lane++) {
                v[lane] = m[c][lane];
            }
            for (lane = 0; lane < 4; lane++) {
                nv[lane] = (A[lane] * v[0] + B[lane] * v[1]) + Cc[lane] * v[2];
            }
            for (lane = 0; lane < 4; lane++) {
                m[c][lane] = nv[lane];
            }
        }
    }

    for (c = 0; c < 4; c++) {
        for (r = 0; r < 4; r++) {
            out[c * 4 + r] = m[c][r];
        }
    }
}
