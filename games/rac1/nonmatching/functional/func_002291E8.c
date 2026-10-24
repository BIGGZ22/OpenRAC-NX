/* functional: high. fcheck: unchecked. Shadow record type 1 (a capsule between two spheres; parse_typed_resource_record, 0x30 bytes): builds the shadow-volume outline in the scratchpad as func_00229098 does for one sphere. p+4 = n1, p+8 = n2 (points per end), p+0x10 / p+0x20 = centre xyz and radius w of ends 1 and 2. With d = c2 - c1, L = D_00160470 (light direction) and u = normalised d x L: when |d x L| + r2 - r1 is negative (sign bit) end 2 is hidden in end 1 and only end 1's ring is built exactly as func_00229098 does (n1 points; n1 = 0 leaves the scratchpad as it is and sets the count to 1). Otherwise the half-angle a = 1.57 - FastArcSin(r2 / (|d x L| * r2 / (r1 - r2))) (1.57 when r1 == r2) sets two arcs about -L: end 1 from angle 1.57 + a over 2 * (3.14 - a) in n1 steps, starting at u * r1, end 2 from 1.57 + (a + 2 * (3.14 - a)) - 6.28 over 2a in n2 steps, starting at u * r2 (Rodrigues matrices from FastSin/FastCos, the VU0 micro programs retail calls with vcallms 0xC90/0xC80); an end with 0 steps gives the two points c -/+ u * r (end 1) or c +/- u * r (end 2). Each point goes to 0x70000000 + 0x20*i with point + D_00160490 (extrusion offset) at +0x10; the first pair is repeated after the last and D_001604A0 = the pair count. The w words are VU0 leftovers, never read before func_00228D20 overwrites them: the arc points keep stale vf5.w, written here as 0 (the emulator's reset VU0 state); the fixed end pairs get vf1.w, which is |d x L| * r2 / (r1 - r2) when r1 != r2 (it survives in the upper half of GPR $1 across the FastArcSin call), reproduced here, 0 when r1 == r2 and n1 == 0, and the caller's stale upper half of $1 when r1 == r2 and n1 != 0 (written as 0). Not handled: d parallel to L (|d x L| = 0, retail's rsqrt gives FLT_MAX) and a zero point count in the sphere branch's loop guard as retail. Differential test: 72 cases agree with retail run in an EE emulator (unicorn with an R5900 MMI/VU0 instruction model) to 1e-5 relative (VU0 vs FPU rounding). */
extern float sqrtf(float);
extern float D_00160470[4];
extern float D_00160480[4];
extern float D_00160490[4];
extern int D_001604A0;
extern float func_001F9FA8(float); /* FastSin */
extern float func_001F9F90(float); /* FastCos */
extern float func_001F9FC0(float); /* FastArcSin */

/* Rows of the rotation by angle about axis a, in VU0's op order (s, c from the VU0 sine program). */
static void rot_002291E8(float r[3][3], const float *a, float ang)
{
    float s = func_001F9FA8(ang);
    float c = func_001F9F90(ang);
    float k = 1.0f - c;
    float tx = a[0] * s, ty = a[1] * s, tz = a[2] * s;

    r[0][0] = a[0] * a[0] * k + c;
    r[0][1] = a[1] * a[0] * k - tz;
    r[0][2] = a[2] * a[0] * k + ty;
    r[1][0] = a[0] * a[1] * k + tz;
    r[1][1] = a[1] * a[1] * k + c;
    r[1][2] = a[2] * a[1] * k - tx;
    r[2][0] = a[0] * a[2] * k - ty;
    r[2][1] = a[1] * a[2] * k + tx;
    r[2][2] = a[2] * a[2] * k + c;
}

static void apply_002291E8(float r[3][3], float *v)
{
    float x = v[0], y = v[1], z = v[2];
    int k;
    for (k = 0; k < 3; k++) {
        v[k] = r[0][k] * x + r[1][k] * y + r[2][k] * z;
    }
}

/* Writes the pair (c + v, c + v + offset) at out. */
static void put_002291E8(float *out, const float *c, const float *v, float sign, float w)
{
    int k;
    for (k = 0; k < 3; k++) {
        out[k] = sign < 0.0f ? c[k] - v[k] : c[k] + v[k];
        out[4 + k] = out[k] + D_00160490[k];
    }
    out[3] = w;
    out[7] = w;
}

static int fbits_002291E8(float f)
{
    union {
        float f;
        int i;
    } u;
    u.f = f;
    return u.i;
}

void func_002291E8(char *p)
{
    int n1 = *(int *)(p + 4);
    int n2 = *(int *)(p + 8);
    float *c1 = (float *)(p + 0x10);
    float *c2 = (float *)(p + 0x20);
    float *L = D_00160470;
    float *base = (float *)0x70000000;
    float *out = base;
    int count = 0;
    float axis[3];
    float u[3];
    float v[3];
    float r[3][3];
    float dx, dy, dz, len2, len, a16, a17, a18, a19, q, t;
    float w = 0.0f; /* the w word of the fixed end pairs: retail's stale vf1.w */
    int i, k;

    dx = c2[0] - c1[0];
    dy = c2[1] - c1[1];
    dz = c2[2] - c1[2];
    u[0] = dy * L[2] - L[1] * dz;
    u[1] = dz * L[0] - L[2] * dx;
    u[2] = dx * L[1] - L[0] * dy;
    axis[0] = -L[0];
    axis[1] = -L[1];
    axis[2] = -L[2];
    len2 = u[0] * u[0] + u[1] * u[1] + u[2] * u[2];
    len = sqrtf(len2);

    if (fbits_002291E8(len + c2[3] - c1[3]) < 0) {
        /* End 2 lies inside end 1: end 1's ring alone. */
        if (n1 != 0) {
            rot_002291E8(r, axis, 6.28f / (float)n1);
            v[0] = D_00160480[0] * c1[3];
            v[1] = D_00160480[1] * c1[3];
            v[2] = D_00160480[2] * c1[3];
            i = n1;
            do {
                apply_002291E8(r, v);
                put_002291E8(out, c1, v, 1.0f, 0.0f);
                out += 8;
                count++;
            } while (--i != 0);
            for (k = 0; k < 8; k++) {
                out[k] = base[k];
            }
        }
        D_001604A0 = count + 1;
        return;
    }

    t = c1[3] - c2[3];
    if (t == 0.0f) {
        a16 = 1.57f;
    } else {
        float x = len * c2[3];
        x = x / t;
        w = x;
        a16 = 1.57f - func_001F9FC0(c2[3] / x);
    }
    t = 3.14f - a16;
    a17 = t + t;
    a18 = a16 + a17;
    a19 = a16 + a16;

    q = 1.0f / sqrtf(len2);
    u[0] *= q;
    u[1] *= q;
    u[2] *= q;

    if (n1 == 0) {
        for (k = 0; k < 3; k++) {
            v[k] = u[k] * c1[3];
        }
        put_002291E8(out, c1, v, -1.0f, w);
        put_002291E8(out + 8, c1, v, 1.0f, w);
        out += 16;
        count += 2;
    } else {
        for (k = 0; k < 3; k++) {
            v[k] = u[k] * c1[3];
        }
        rot_002291E8(r, axis, 1.57f + a16);
        apply_002291E8(r, v);
        rot_002291E8(r, axis, a17 / (float)n1);
        i = n1;
        do {
            put_002291E8(out, c1, v, 1.0f, 0.0f);
            apply_002291E8(r, v);
            out += 8;
            count++;
        } while (--i != 0);
    }

    if (n2 == 0) {
        for (k = 0; k < 3; k++) {
            v[k] = u[k] * c2[3];
        }
        put_002291E8(out, c2, v, 1.0f, w);
        put_002291E8(out + 8, c2, v, -1.0f, w);
        out += 16;
        count += 2;
    } else {
        for (k = 0; k < 3; k++) {
            v[k] = u[k] * c2[3];
        }
        rot_002291E8(r, axis, 1.57f + a18 - 6.28f);
        apply_002291E8(r, v);
        rot_002291E8(r, axis, a19 / (float)n2);
        i = n2;
        do {
            put_002291E8(out, c2, v, 1.0f, 0.0f);
            apply_002291E8(r, v);
            out += 8;
            count++;
        } while (--i != 0);
    }

    for (k = 0; k < 8; k++) {
        out[k] = base[k];
    }
    D_001604A0 = count + 1;
}
