/* functional: high. fcheck: unchecked. LightTies(list): relights the 64 vertex colours of every tie instance named by a 0xFFFF-terminated u16 index list. Instance records are 0x20 bytes at *D_00161050 (+0x00 position, +0x1A class index into the class pointer table D_001E1A00 whose +0xC is the s16 x4 normal list, +0x1B dirty byte (cleared), +0x1C light select, +0x1E point-light nibbles); the per-instance block is 0x1C0 bytes at *D_00161058 (+0x00 three matrix rows scaled by their w, +0x40 the 64 output colours, +0x140 the 64 1555 input colours). The light set (or a blend of two by weight byte / 256, directions renormalised) from D_0019BEC0 gives two directional lights; the point lights (D_0019C2C0, up to four, 0xF ends) within range of the instance position are folded into a third: direction = sum of the unit vectors from the light to the instance (renormalised when two or more are in range), colour = sum of colour * (1 - d/r), back factor = the summed w. The three directions are taken into model space as -M L; per vertex the colour is 65536 + c/128 + sum colour_k * max(s_k, s_k * back_k) with s_k = L'_k . n (n = s16 normal / 32768), clamped to 65536 + 243/128 (the VU0 I register of the micro program) in x, y, z and packed from the low byte of each float. Retail streams four vertices at a time through VU0 micro programs 0x2C0 / 0x420 of D_00100AE0 (loaded by drawNormalFrame first); this C does that arithmetic directly, as float products then float adds. Retail's register spill slots at scratchpad 0x70003FD8..0x70004000 are not written. Differential test: 40 cases agree with retail run in an EE emulator (unicorn with an R5900 MMI/VU0 instruction model, running the D_00100AE0 micro programs; every instruction of the retail routine executed; spill slots excluded). */
extern char D_00161050[];
extern char D_00161058[];
extern char D_001E1A00[];
extern char D_0015FB80[];
extern char D_0019BEC0[];
extern char D_0019C2C0[];
extern float sqrtf(float);

static float u2f_238688(unsigned int u)
{
    union { float f; unsigned int u; } c;
    c.u = u;
    return c.f;
}

static unsigned int f2u_238688(float f)
{
    union { float f; unsigned int u; } c;
    c.f = f;
    return c.u;
}

static float vmax_238688(float a, float b)
{
    return a > b ? a : b;
}

static float vmin_238688(float a, float b)
{
    return a < b ? a : b;
}

/* VU0 vdiv/vrsqrt with a zero divisor give the largest float */
static float vdiv_238688(float a, float b)
{
    if (b == 0.0f) {
        return a < 0.0f ? -3.4028235e38f : 3.4028235e38f;
    }
    return a / b;
}

/* (x*y) + k*z as retail sums a dot product */
static float dot_238688(float *a, float *b, float k)
{
    float x = a[0] * b[0];
    float y = a[1] * b[1];
    float z = a[2] * b[2];

    return (x + y) + k * z;
}

static void normalize_238688(float *v, float k21)
{
    float q = dot_238688(v, v, k21);

    q = q < 0.0f ? -q : q;
    q = vdiv_238688(1.0f, sqrtf(q));
    v[0] *= q;
    v[1] *= q;
    v[2] *= q;
}

void func_00238688(void *arg)
{
    unsigned short *list = (unsigned short *)arg;
    char *insts = *(char **)D_00161050;
    char *blocks = *(char **)D_00161058;
    float k21 = ((float *)D_0015FB80)[0];
    float lim = u2f_238688(0x478000F3);

    for (;;) {
        unsigned int idx = *list++;
        char *r;
        float *pos;
        float *m;
        short *nrm;
        unsigned short *cin;
        unsigned int *cout;
        float c0[4], d0[4], c1[4], d1[4], c2[4], d2[3];
        float k0, k1, k2;
        float row[3][3];
        float T[3][3];
        unsigned int h, sel;
        int cnt, i, k, v;

        if (idx == 0xFFFF) {
            break;
        }
        r = insts + (idx << 5);
        h = *(unsigned short *)(r + 0x1C);

        if ((h & 0xFF00) != 0) {
            float f = (float)((h >> 8) & 0xFF) * (1.0f / 256.0f);
            float g = 1.0f - f;
            float *A = (float *)(D_0019BEC0 + ((h & 0xF) << 6));
            float *B = (float *)(D_0019BEC0 + (((h >> 4) & 0xF) << 6));

            for (i = 0; i < 4; i++) {
                c0[i] = A[i] * g + B[i] * f;
                c1[i] = A[8 + i] * g + B[8 + i] * f;
            }
            for (i = 0; i < 3; i++) {
                d0[i] = A[4 + i] * g + B[4 + i] * f;
                d1[i] = A[12 + i] * g + B[12 + i] * f;
            }
            normalize_238688(d0, k21);
            normalize_238688(d1, k21);
        } else {
            float *A = (float *)(D_0019BEC0 + ((h & 0xF) << 6));

            for (i = 0; i < 4; i++) {
                c0[i] = A[i];
                c1[i] = A[8 + i];
            }
            for (i = 0; i < 3; i++) {
                d0[i] = A[4 + i];
                d1[i] = A[12 + i];
            }
        }
        if (*(unsigned char *)(r + 0x1B) != 0) {
            *(unsigned char *)(r + 0x1B) = 0;
        }
        k0 = c0[3];
        k1 = c1[3];
        c0[3] = 0.0f;
        c1[3] = 0.0f;

        /* point lights in range fold into a third light */
        pos = (float *)r;
        sel = *(unsigned short *)(r + 0x1E) | 0xF0000;
        cnt = -2;
        for (i = 0; i < 4; i++) {
            c2[i] = 0.0f;
        }
        d2[0] = 0.0f;
        d2[1] = 0.0f;
        d2[2] = 0.0f;
        while ((sel & 0xF) != 0xF) {
            float *P = (float *)(D_0019C2C0 + ((sel & 0xF) << 5));
            float rinv = vdiv_238688(1.0f, P[7]);
            float vv[3];
            float dd2, d, w, q;

            sel = (int)sel >> 4;
            vv[0] = pos[0] - P[4];
            vv[1] = pos[1] - P[5];
            vv[2] = pos[2] - P[6];
            dd2 = dot_238688(vv, vv, k21);
            if (P[7] * P[7] - dd2 < 0.0f) {
                continue;
            }
            d = sqrtf(dd2 < 0.0f ? -dd2 : dd2);
            w = 1.0f - rinv * d;
            q = vdiv_238688(1.0f, d);
            for (i = 0; i < 4; i++) {
                c2[i] = c2[i] + P[i] * w;
            }
            for (i = 0; i < 3; i++) {
                d2[i] = vv[i] * q + d2[i];
            }
            cnt++;
        }
        if (cnt >= 0) {
            normalize_238688(d2, k21);
        }
        k2 = c2[3];
        c2[3] = 0.0f;

        /* the light directions in model space: L'_k = -M L_k */
        m = (float *)(blocks + idx * 0x1C0);
        for (i = 0; i < 3; i++) {
            for (k = 0; k < 3; k++) {
                row[i][k] = 0.0f - m[4 * i + k] * m[4 * i + 3];
            }
        }
        for (i = 0; i < 3; i++) {
            float acc;

            acc = row[i][0] * d0[0];
            acc = acc + row[i][1] * d0[1];
            T[0][i] = acc + row[i][2] * d0[2];
            acc = row[i][0] * d1[0];
            acc = acc + row[i][1] * d1[1];
            T[1][i] = acc + row[i][2] * d1[2];
            acc = row[i][0] * d2[0];
            acc = acc + row[i][1] * d2[1];
            T[2][i] = acc + row[i][2] * d2[2];
        }

        nrm = *(short **)(*(char **)(D_001E1A00 + *(unsigned char *)(r + 0x1A) * 4) + 0xC);
        cin = (unsigned short *)(blocks + idx * 0x1C0 + 0x140);
        cout = (unsigned int *)(blocks + idx * 0x1C0 + 0x40);
        for (v = 0; v < 64; v++) {
            float nx = (float)nrm[4 * v] * (1.0f / 32768.0f);
            float ny = (float)nrm[4 * v + 1] * (1.0f / 32768.0f);
            float nz = (float)nrm[4 * v + 2] * (1.0f / 32768.0f);
            int hc = *(short *)&cin[v];
            unsigned int cw = ((hc & 0x1F) << 3) | (((hc >> 5) & 0x1F) << 11) | (((hc >> 10) & 0x1F) << 19) | (((hc >> 15) & 1) << 31);
            float s[3], kk[3], col[4];
            unsigned int out;

            kk[0] = k0;
            kk[1] = k1;
            kk[2] = k2;
            for (k = 0; k < 3; k++) {
                float acc = T[k][0] * nx;
                acc = acc + T[k][1] * ny;
                s[k] = acc + T[k][2] * nz;
                s[k] = vmax_238688(s[k], s[k] * kk[k]);
            }
            for (i = 0; i < 4; i++) {
                float acc = u2f_238688(0x47800000 + ((cw >> (8 * i)) & 0xFF));
                acc = acc + c0[i] * s[0];
                acc = acc + c1[i] * s[1];
                col[i] = acc + c2[i] * s[2];
            }
            out = f2u_238688(vmin_238688(col[0], lim)) & 0xFF;
            out |= (f2u_238688(vmin_238688(col[1], lim)) & 0xFF) << 8;
            out |= (f2u_238688(vmin_238688(col[2], lim)) & 0xFF) << 16;
            out |= (f2u_238688(col[3]) & 0xFF) << 24;
            cout[v] = out;
        }
    }
}
