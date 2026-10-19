/* functional: high. fcheck: unchecked. LightTfrags(list): vertex lighting of the tfrags named by a 0xFFFF-terminated u16 index list (tfrag records of 0x40 bytes at *D_00160F8C). The normal table (0x80 quadwords at D_00165600) is copied to scratchpad 0x70003800 first (retail: DMA channel 9; here a direct copy, leaving the channel registers as a finished transfer does). Per tfrag (data base at +0x10, vertex list at base + (s16)+0x30 + 0x10, 8 bytes per vertex: u16 position offset, two normal-table bytes, 1555 colour, light select): with a light-set byte (+0x34) >= 0, 4 * (+0x29) vertices are lit by that set's two directional lights (D_0019BEC0 + 64 * set: colour 0, direction 0, colour 1, direction 1, back factors in the colour w lanes); otherwise (+0x3C) vertices each pick a set (or blend two, weight byte / 256, renormalising the directions) from their light select word. Normals (-(a0 a1, b0 a1, b1) from the table, w = b1) go to scratchpad 0x70000000 + 16 * i; colours are made as 65536 + c/128 floats, summed with the clamped dot products, clamped to 65536 + 255/128 and packed from the low byte of each float, into the scratchpad buffer at 0x70001000 / 0x70002000 (alternating). Then the point lights named by the nibbles of +0x36 (up to 4, 0xF ends; D_0019C2C0 + 32 * n: colour + back factor, position + radius) add colour * max(dot, dot * back) with the light vector scaled by (1 - d/r)/d to every vertex within the radius, positions being s16 / 1024 plus the int origin at base + (s16)+0x30 (in 1/1024 units); +0x35 is cleared. Each buffer goes to +0x10 + (s16)+0x1E in main memory ((s8)+0x29 quadwords; retail: DMA channel 8, here a direct copy). Retail's register spill slots at scratchpad 0x70000800..0x70000840 are not written. VU0 multiply-adds are written as a float product then a float add. Differential test: 40 cases agree with retail run in an EE emulator (unicorn with an R5900 MMI/VU0 instruction model; every instruction of the retail routine executed; with a test-only scratchpad DMA model that completes a transfer when retail polls the channel; spill slots excluded). */
extern char D_00160F8C[];
extern char D_00165600[];
extern char D_0015FB80[];
extern char D_0019BEC0[];
extern char D_0019C2C0[];
extern float sqrtf(float);

#define SPR(o) ((unsigned char *)(0x70000000 + (o)))

static float u2f_2362B0(unsigned int u)
{
    union { float f; unsigned int u; } c;
    c.u = u;
    return c.f;
}

static unsigned int f2u_2362B0(float f)
{
    union { float f; unsigned int u; } c;
    c.f = f;
    return c.u;
}

/* acc + a * b */
static float madd_2362B0(float acc, float a, float b)
{
    return acc + a * b;
}

static float vmax_2362B0(float a, float b)
{
    return a > b ? a : b;
}

static float vmin_2362B0(float a, float b)
{
    return a < b ? a : b;
}

/* VU0 vdiv/vrsqrt with a zero divisor give the largest float */
static float vdiv_2362B0(float a, float b)
{
    if (b == 0.0f) {
        return a < 0.0f ? -3.4028235e38f : 3.4028235e38f;
    }
    return a / b;
}

static void copy_qw_2362B0(unsigned char *dst, unsigned char *src, int qwc)
{
    unsigned int *d = (unsigned int *)dst;
    unsigned int *s = (unsigned int *)src;
    int i;

    for (i = 0; i < qwc * 4; i++) {
        d[i] = s[i];
    }
}

/* the normal from two (cos, sin) table entries, stored in the scratchpad normal list */
static void normal_2362B0(int h, float *out)
{
    float *t0 = (float *)SPR(0x3800 + (h & 0xFF) * 8);
    float *t1 = (float *)SPR(0x3800 + ((h >> 8) & 0xFF) * 8);
    float a1 = t1[0];
    float b1 = t1[1];

    out[0] = 0.0f - t0[0] * a1;
    out[1] = 0.0f - t0[1] * a1;
    out[2] = 0.0f - b1;
    out[3] = b1;
}

/* the four bytes as 65536 + c/128 */
static void unpack_2362B0(unsigned int w, float *c)
{
    int i;

    for (i = 0; i < 4; i++) {
        c[i] = u2f_2362B0(0x47800000 + ((w >> (8 * i)) & 0xFF));
    }
}

static unsigned int col1555_2362B0(int h)
{
    return ((h & 0x1F) << 3) | (((h >> 5) & 0x1F) << 11) | (((h >> 10) & 0x1F) << 19) | (((h >> 15) & 1) << 31);
}

static unsigned int pack_2362B0(float *v)
{
    float lim = u2f_2362B0(0x478000FF);
    unsigned int r = f2u_2362B0(vmin_2362B0(v[0], lim)) & 0xFF;

    r |= (f2u_2362B0(vmin_2362B0(v[1], lim)) & 0xFF) << 8;
    r |= (f2u_2362B0(vmin_2362B0(v[2], lim)) & 0xFF) << 16;
    r |= (f2u_2362B0(v[3]) & 0xFF) << 24;
    return r;
}

/* (x*y) + k*z as retail sums a dot product */
static float dot_2362B0(float *a, float *b, float k)
{
    float x = a[0] * b[0];
    float y = a[1] * b[1];
    float z = a[2] * b[2];

    return madd_2362B0(x + y, k, z);
}

/* one vertex under two directional lights: c0/d0, c1/d1, back factors k0/k1 */
static unsigned int light2_2362B0(float *n, unsigned int col, float *c0, float *d0, float *c1, float *d1,
                                  float k0, float k1, float k21)
{
    float base[4], out[4];
    float s0, s1;
    int i;

    unpack_2362B0(col, base);
    s0 = dot_2362B0(n, d0, k21);
    s1 = dot_2362B0(n, d1, k21);
    s0 = vmax_2362B0(s0, s0 * k0);
    s1 = vmax_2362B0(s1, s1 * k1);
    for (i = 0; i < 4; i++) {
        float acc = madd_2362B0(base[i], c0[i], s0);
        out[i] = madd_2362B0(acc, c1[i], s1);
    }
    return pack_2362B0(out);
}

void func_002362B0(void *arg)
{
    unsigned short *list = (unsigned short *)arg;
    char *tfrags = *(char **)D_00160F8C;
    float k21 = ((float *)D_0015FB80)[0];
    unsigned int buf = 0x1000;
    volatile unsigned int *ch9 = (volatile unsigned int *)0x1000D400;
    volatile unsigned int *ch8 = (volatile unsigned int *)0x1000D000;

    ch9[0x80 / 4] = 0x3800;
    ch9[0x10 / 4] = (unsigned int)D_00165600;
    ch9[0x20 / 4] = 0x80;
    copy_qw_2362B0(SPR(0x3800), (unsigned char *)D_00165600, 0x80);
    ch9[0] = 0;

    for (;;) {
        unsigned int idx = *list++;
        char *t;
        char *base;
        unsigned char *v;
        unsigned int *out;
        float *nrm;
        float c0[4], d0[4], c1[4], d1[4];
        float k0, k1;
        int set;
        int n;
        unsigned int sel;

        if (idx == 0xFFFF) {
            break;
        }
        t = tfrags + (idx << 6);
        base = *(char **)(t + 0x10);
        v = (unsigned char *)(base + *(short *)(t + 0x30) + 0x10);
        out = (unsigned int *)SPR(buf);
        nrm = (float *)SPR(0);
        set = *(signed char *)(t + 0x34);

        if (set >= 0) {
            float *L = (float *)(D_0019BEC0 + (set << 6));
            int i;

            for (i = 0; i < 4; i++) {
                c0[i] = L[i];
                d0[i] = L[4 + i];
                c1[i] = L[8 + i];
                d1[i] = L[12 + i];
            }
            k0 = c0[3];
            k1 = c1[3];
            c0[3] = 0.0f;
            c1[3] = 0.0f;
            n = *(signed char *)(t + 0x29) * 2;
            do {
                normal_2362B0(*(short *)(v + 2), nrm);
                normal_2362B0(*(short *)(v + 0xA), nrm + 4);
                out[0] = light2_2362B0(nrm, col1555_2362B0(*(short *)(v + 4)), c0, d0, c1, d1, k0, k1, k21);
                out[1] = light2_2362B0(nrm + 4, col1555_2362B0(*(short *)(v + 0xC)), c0, d0, c1, d1, k0, k1, k21);
                nrm += 8;
                out += 2;
                v += 0x10;
            } while (--n != 0);
        } else {
            n = *(signed char *)(t + 0x3C);
            do {
                int h = *(short *)(v + 6);
                int i;

                if ((h & 0xFF00) != 0) {
                    float f = (float)(((unsigned int)h >> 8) & 0xFF) * (1.0f / 256.0f);
                    float g = 1.0f - f;
                    float *A = (float *)(D_0019BEC0 + ((h & 0xF) << 6));
                    float *B = (float *)(D_0019BEC0 + (((h >> 4) & 0xF) << 6));
                    float q;

                    for (i = 0; i < 4; i++) {
                        c0[i] = A[i] * g + B[i] * f;
                        c1[i] = A[8 + i] * g + B[8 + i] * f;
                    }
                    for (i = 0; i < 3; i++) {
                        d0[i] = A[4 + i] * g + B[4 + i] * f;
                        d1[i] = A[12 + i] * g + B[12 + i] * f;
                    }
                    q = dot_2362B0(d0, d0, k21);
                    q = q < 0.0f ? -q : q;
                    q = vdiv_2362B0(1.0f, sqrtf(q));
                    d0[0] *= q;
                    d0[1] *= q;
                    d0[2] *= q;
                    q = dot_2362B0(d1, d1, k21);
                    q = q < 0.0f ? -q : q;
                    q = vdiv_2362B0(1.0f, sqrtf(q));
                    d1[0] *= q;
                    d1[1] *= q;
                    d1[2] *= q;
                } else {
                    float *A = (float *)(D_0019BEC0 + ((h & 0xF) << 6));

                    for (i = 0; i < 4; i++) {
                        c0[i] = A[i];
                        d0[i] = A[4 + i];
                        c1[i] = A[8 + i];
                        d1[i] = A[12 + i];
                    }
                }
                k0 = c0[3];
                k1 = c1[3];
                c0[3] = 0.0f;
                c1[3] = 0.0f;
                normal_2362B0(*(short *)(v + 2), nrm);
                out[0] = light2_2362B0(nrm, col1555_2362B0(*(short *)(v + 4)), c0, d0, c1, d1, k0, k1, k21);
                nrm += 4;
                out += 1;
                v += 8;
            } while (--n != 0);
        }

        /* point lights */
        if (*(signed char *)(t + 0x35) != 0) {
            *(signed char *)(t + 0x35) = 0;
        }
        sel = *(unsigned short *)(t + 0x36) | 0xF0000;
        while ((sel & 0xF) != 0xF) {
            float *P = (float *)(D_0019C2C0 + ((sel & 0xF) << 5));
            int *org = (int *)(base + *(short *)(t + 0x30));
            float ox = (float)(org[0] << 2) * (1.0f / 4096.0f);
            float oy = (float)(org[1] << 2) * (1.0f / 4096.0f);
            float oz = (float)(org[2] << 2) * (1.0f / 4096.0f);
            float rinv = vdiv_2362B0(1.0f, P[7]);
            float r2 = P[7] * P[7];

            sel = (int)sel >> 4;
            v = (unsigned char *)(base + *(short *)(t + 0x30) + 0x10);
            out = (unsigned int *)SPR(buf);
            nrm = (float *)SPR(0);
            n = *(signed char *)(t + 0x3C);
            do {
                short *p = (short *)(base + *(unsigned short *)v);
                float dv[3];
                float d2, d, s, w;

                dv[0] = ((float)(p[0] << 2) * (1.0f / 4096.0f) + ox) - P[4];
                dv[1] = ((float)(p[1] << 2) * (1.0f / 4096.0f) + oy) - P[5];
                dv[2] = ((float)(p[2] << 2) * (1.0f / 4096.0f) + oz) - P[6];
                d2 = dot_2362B0(dv, dv, k21);
                if (!(r2 - d2 < 0.0f)) {
                    float base4[4], res[4];
                    int i;

                    d = sqrtf(d2 < 0.0f ? -d2 : d2);
                    w = 1.0f - rinv * d;
                    w = w * vdiv_2362B0(1.0f, d);
                    dv[0] *= w;
                    dv[1] *= w;
                    dv[2] *= w;
                    s = dot_2362B0(nrm, dv, k21);
                    s = vmax_2362B0(s, s * P[3]);
                    unpack_2362B0(*out, base4);
                    for (i = 0; i < 4; i++) {
                        res[i] = madd_2362B0(base4[i], P[i], s);
                    }
                    *out = pack_2362B0(res);
                }
                nrm += 4;
                out += 1;
                v += 8;
            } while (--n != 0);
        }

        ch8[0x80 / 4] = buf;
        ch8[0x10 / 4] = (unsigned int)(*(char **)(t + 0x10) + *(short *)(t + 0x1E));
        ch8[0x20 / 4] = *(signed char *)(t + 0x29);
        copy_qw_2362B0((unsigned char *)(*(char **)(t + 0x10) + *(short *)(t + 0x1E)), SPR(buf),
                       *(signed char *)(t + 0x29));
        ch8[0] = 0;
        buf = 0x3000 - buf;
    }
}
