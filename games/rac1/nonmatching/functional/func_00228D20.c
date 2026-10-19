/* functional: high. fcheck: unchecked. Projects the shadow-volume ring in the scratchpad (buf = 0x70000000, count = D_001604A0 pairs of xyzw points) to screen space and classifies it, for func_002298B0. Pass 1: each point goes through gCamera's matrix (D_00187040 +0xC0..0xF0) scaled by 1024 with the translation -1024 * camera position (+0x140) folded in, is divided by its w and scaled/offset by D_0018CE00 +0x190 / +0x1A0; w becomes 0. With clip != 0 the sum runs in x,y,z,w order and when either point of a pair lies outside -|w|..|w| in x, y or z (the VU0 clip test), the low byte of both w words is set to 1; with clip == 0 it runs w,z,y,x and nothing is flagged. Pass 2: D_001604A8[0] / [1] = the 2D cross products (winding) of points 0, 2, 4 and 1, 3, 5 (element 0 / 1 of pairs 0..2); then for each pair i the two flag bytes go to 0x70003800 + 2i, the side byte 0x70003000 + i is 0 when the first point of pair i or of pair i+1 is flagged, else 1 / 2 for a positive / negative (sign bit) cross of (p[i].0, p[i].1, p[i+1].0), and each point is rewritten as x, y = (coord - D_0018CE00[0x1A0] + D_0018CE00[0x200]) / 512 (floats), then x, y = (int)((coord - 0.5) * 16) packed as two 16-bit halves and z = (int)z (VU0 vftoi0: truncation). Pass 2 reads the pair after the last one (its first point and flag byte) and the cross for D_001604A8 needs count >= 3, as retail. The VU0 registers retail leaves behind (vf18..vf31 among them) are not reproduced. Differential test: 72 cases agree with retail run in an EE emulator (unicorn with an R5900 MMI/VU0 instruction model) to 1e-5 relative (VU0 vs FPU rounding). */
extern float D_00187040[];
extern float D_0018CE00[];
extern float D_001604A8[2];

static float cross_00228D20(const float *a, const float *b, const float *c)
{
    float v1x = c[0] - a[0];
    float v1y = c[1] - a[1];
    float v2x = b[0] - a[0];
    float v2y = b[1] - a[1];
    return v1x * v2y - v2x * v1y;
}

static int fbits_00228D20(float f)
{
    union {
        float f;
        int i;
    } u;
    u.f = f;
    return u.i;
}

/* Clip judgement of o.xyz against |o.w|: nonzero when any lane is outside. */
static int outside_00228D20(const float *o)
{
    float w = o[3] < 0.0f ? -o[3] : o[3];
    int k;
    for (k = 0; k < 3; k++) {
        if (o[k] > w || o[k] < -w) {
            return 1;
        }
    }
    return 0;
}

static void project_00228D20(float *p, const float *o)
{
    const float *scale = D_0018CE00 + 0x190 / 4;
    const float *offs = D_0018CE00 + 0x1A0 / 4;
    float q = 1.0f / o[3];
    int k;
    for (k = 0; k < 3; k++) {
        p[k] = o[k] * q * scale[k] + offs[k];
    }
    p[3] = 0.0f;
}

void func_00228D20(unsigned int buf, int count, int clip)
{
    const float *cam = D_00187040 + 0xC0 / 4; /* four rows */
    const float *eye = D_00187040 + 0x140 / 4;
    const float *offs = D_0018CE00 + 0x1A0 / 4;
    const float *bias = D_0018CE00 + 0x200 / 4;
    float m[4][4];
    float px = eye[0] * -1024.0f;
    float py = eye[1] * -1024.0f;
    float pz = eye[2] * -1024.0f;
    float *p;
    float o0[4], o1[4];
    unsigned char *side = (unsigned char *)0x70003000;
    unsigned char *flag = (unsigned char *)0x70003800;
    int n, k;

    for (k = 0; k < 4; k++) {
        m[0][k] = cam[k] * 1024.0f;
        m[1][k] = cam[4 + k] * 1024.0f;
        m[2][k] = cam[8 + k] * 1024.0f;
        m[3][k] = cam[k] * px + cam[4 + k] * py + cam[8 + k] * pz + cam[12 + k];
    }

    p = (float *)buf;
    n = count;
    if (clip == 0) {
        do {
            for (k = 0; k < 4; k++) {
                o0[k] = m[3][k] + m[2][k] * p[2] + m[1][k] * p[1] + m[0][k] * p[0];
                o1[k] = m[3][k] + m[2][k] * p[6] + m[1][k] * p[5] + m[0][k] * p[4];
            }
            project_00228D20(p, o0);
            project_00228D20(p + 4, o1);
            p += 8;
        } while (--n != 0);
    } else {
        do {
            int out;
            for (k = 0; k < 4; k++) {
                o0[k] = m[0][k] * p[0] + m[1][k] * p[1] + m[2][k] * p[2] + m[3][k];
                o1[k] = m[0][k] * p[4] + m[1][k] * p[5] + m[2][k] * p[6] + m[3][k];
            }
            out = outside_00228D20(o0) | outside_00228D20(o1);
            project_00228D20(p, o0);
            project_00228D20(p + 4, o1);
            if (out) {
                *((unsigned char *)p + 0xC) = 1;
                *((unsigned char *)p + 0x1C) = 1;
            }
            p += 8;
        } while (--n != 0);
    }

    p = (float *)buf;
    D_001604A8[0] = cross_00228D20(p, p + 8, p + 16);
    D_001604A8[1] = cross_00228D20(p + 4, p + 12, p + 20);
    n = count;
    do {
        signed char f0 = *((signed char *)p + 0xC);
        signed char f1 = *((signed char *)p + 0x1C);
        signed char fn = *((signed char *)p + 0x2C);
        int k2;

        flag[0] = f0;
        flag[1] = f1;
        flag += 2;
        if ((f0 | fn) != 0) {
            *side = 0;
        } else {
            *side = fbits_00228D20(cross_00228D20(p, p + 4, p + 8)) < 0 ? 2 : 1;
        }
        side++;
        for (k2 = 0; k2 < 8; k2 += 4) {
            float *v = p + k2;
            float sx = (v[0] - offs[0] + bias[0]) * 0.001953125f;
            float sy = (v[1] - offs[1] + bias[1]) * 0.001953125f;
            int ix = (int)((v[0] - 0.5f) * 16.0f);
            int iy = (int)((v[1] - 0.5f) * 16.0f);
            int iz = (int)v[2];
            v[0] = sx;
            v[1] = sy;
            ((unsigned int *)v)[2] = ((unsigned int)ix & 0xFFFF) | ((unsigned int)iy << 16);
            ((int *)v)[3] = iz;
        }
        p += 8;
    } while (--n != 0);
}
