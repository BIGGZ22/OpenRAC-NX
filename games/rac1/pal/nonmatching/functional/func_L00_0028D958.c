/* functional: medium. For n records of 0x20 bytes at rec (xyz centre and radius as four floats), flags[i] = 1 when the sphere passes two cone tests against the matrices at D_L00_0016CDC0 (M, s = M[15]) and D_L00_001BDB70 (V); else 0. Computes c = M*V*(s*p), r' = s*r, visible when M[16]*c.z - |c.x| + M[24]*r' >= 0 and M[17]*c.z - |c.y| + M[25]*r' >= 0. Medium: the VU0 rounding is not IEEE, the test uses >= 0 where retail tests the sign bit (differs only for -0.0), and n must be at least 1 (retail loops do-while). */
extern char D_L00_0016CDC0[];
extern char D_L00_001BDB70[];

void func_L00_0028D958(char *rec, int n, char *flags) {
    float *M = (float *)D_L00_0016CDC0;
    float *V = (float *)D_L00_001BDB70;
    float s = M[15];
    float P[3][3];
    int i;
    int j;
    int k;

    for (j = 0; j < 3; j++) {
        float *vj = V + 4 * j;
        P[j][0] = M[0] * vj[0] + M[4] * vj[1] + M[8] * vj[2];
        P[j][1] = M[1] * vj[0] + M[5] * vj[1] + M[9] * vj[2];
        P[j][2] = M[2] * vj[0] + M[6] * vj[1] + M[10] * vj[2];
    }
    for (i = 0; i < n; i++) {
        float *r = (float *)(rec + 0x20 * i);
        float q0 = r[0] * s;
        float q1 = r[1] * s;
        float q2 = r[2] * s;
        float rad = r[3] * s;
        float c[3];
        float ax;
        float ay;
        int vis;

        for (k = 0; k < 3; k++) {
            c[k] = P[0][k] * q0 + P[1][k] * q1 + P[2][k] * q2;
        }
        ax = c[0] < 0.0f ? -c[0] : c[0];
        ay = c[1] < 0.0f ? -c[1] : c[1];
        vis = (M[16] * c[2] - ax + M[24] * rad >= 0.0f) && (M[17] * c[2] - ay + M[25] * rad >= 0.0f);
        flags[i] = vis ? 1 : 0;
    }
}
