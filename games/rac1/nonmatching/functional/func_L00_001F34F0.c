/* functional: high. fcheck: unchecked. Sphere against the level collision mesh (retail: VU0 + MMI, scratchpad work buffers). radius in f12, pos in a0 (xyz); everything is done in 1/64 units (pos * 64, radius * 64). D_L00_0015F6F8 (gp-0x7608) is the collision set: a 16-byte header whose first word is the count, then that many 16-byte entries {u16 x, y, z, r (bounding sphere), u8 tri count at +8, u16 vertex count at +0xA, ptr at +0xC to vertex data (u16 x, y, z, w each) followed by the triangles (u8 i0, i1, i2, material)}. Entries whose bounding sphere overlaps are listed at scratchpad 0x70000000; for each, the vertices (in pairs) are converted to float at scratchpad 0x70002000 + 16 i with a 16-bit outcode against the sphere's box in word 3, and each triangle not trivially outside is tested: front side only (d = (P - v0).N >= 0, N = e2 x e1), plane distance within the current best, then the projection inside the triangle or else the closest point on the crossed edge. A hit keeps material + 0x1000, shrinks the best squared distance to 0.9995 of it and keeps the point, N and the vertices. Returns 0 with nothing written when the set is null or nothing was hit; otherwise 1 and writes D_L00_00173F40: +0x18 = 0, +0x1C = material + 0x1000, +0x20 closest point, +0x30 pos pushed out from it to the radius, +0x40 N (unnormalised, 1/64-scale units squared), +0x50/+0x60/+0x70 the triangle's vertices (world units). The w words of those quads are stale VU0 register contents in retail; here 0, except +0x30's, which carries vf2.w as retail leaves it (last vertex's outcode word or w halfword). VU0 vdiv/vrsqrt by zero give +-FLT_MAX as on the VU. Differential test: 400 cases agree with retail run in an EE emulator (unicorn with an R5900 MMI/VU0 instruction model) to 1e-5 relative (VU0 fused multiply-add vs FPU rounding). */
extern int *D_L00_0015F6F8;
extern unsigned char D_L00_00173F40[];

#define FMAX_1F34F0 3.40282347e+38f

static int fbits_1F34F0(float f)
{
    union {
        float f;
        int i;
    } u;
    u.f = f;
    return u.i;
}

static float vdiv_1F34F0(float n, float d)
{
    float q;
    if (d == 0.0f) {
        return (n < 0.0f || (n == 0.0f && fbits_1F34F0(n) < 0)) ? -FMAX_1F34F0 : FMAX_1F34F0;
    }
    q = n / d;
    if (q > FMAX_1F34F0) {
        q = FMAX_1F34F0;
    } else if (q < -FMAX_1F34F0) {
        q = -FMAX_1F34F0;
    }
    return q;
}

static int neg_1F34F0(float f)
{
    return fbits_1F34F0(f) < 0;
}

static int ftoi0_1F34F0(float f)
{
    if (f >= 2147483647.0f) {
        return 0x7FFFFFFF;
    }
    if (f <= -2147483648.0f) {
        return (int)(-2147483647 - 1);
    }
    return (int)f;
}

static float dot_1F34F0(const float *a, const float *b)
{
    float x = a[0] * b[0];
    float y = a[1] * b[1];
    float z = a[2] * b[2];
    return (x + y) + z;
}

/* r = a x b as vopmula/vopmsub compute it */
static void cross_1F34F0(float *r, const float *a, const float *b)
{
    float t0 = a[1] * b[2];
    float t1 = a[2] * b[0];
    float t2 = a[0] * b[1];
    float x = t0 - b[1] * a[2];
    float y = t1 - b[2] * a[0];
    float z = t2 - b[0] * a[1];
    r[0] = x;
    r[1] = y;
    r[2] = z;
}

int func_L00_001F34F0(float radius, void *pos_)
{
    float *pos = (float *)pos_;
    int *set = D_L00_0015F6F8;
    float P[3];
    float r64;
    float best;
    float bmin[3];
    float bmax[3];
    float hit_pt[3];
    float hit_n[3];
    float hit_v0[3];
    float hit_v1[3];
    float hit_v2[3];
    unsigned int mat = 0;
    unsigned int vf2w = 0;
    int count;
    unsigned short *e;
    unsigned int *list;
    unsigned int *lend;
    int i;

    if (set == 0) {
        return 0;
    }
    r64 = radius * 64.0f;
    for (i = 0; i < 3; i++) {
        P[i] = pos[i] * 64.0f;
        bmin[i] = P[i] - r64;
        bmax[i] = P[i] + r64;
    }
    best = r64 * r64;

    /* bounding-sphere pass */
    count = set[0];
    e = (unsigned short *)(set + 4);
    list = (unsigned int *)0x70000000;
    while (count > 0) {
        float c[4];
        float s;
        float d2;
        float dx, dy, dz;
        for (i = 0; i < 4; i++) {
            c[i] = (float)e[i];
        }
        s = r64 + c[3];
        s = s * s;
        dx = c[0] - P[0];
        dy = c[1] - P[1];
        dz = c[2] - P[2];
        dx = dx * dx;
        dy = dy * dy;
        dz = dz * dz;
        d2 = ((dy - s) + dx) + dz;
        count--;
        if (!(fbits_1F34F0(d2) > 0 || (fbits_1F34F0(d2) == 0 && fbits_1F34F0(dx) != 0))) {
            *list++ = (unsigned int)(unsigned long)e;
        }
        e += 8;
    }
    lend = (unsigned int *)(unsigned long)(0x70000000u + (((unsigned int)(unsigned long)list) & 0xFFF));

    for (list = (unsigned int *)0x70000000; list != lend; list++) {
        unsigned char *ent = (unsigned char *)(unsigned long)*list;
        unsigned short *vtx = *(unsigned short **)(ent + 0xC);
        int nv = *(unsigned short *)(ent + 0xA);
        unsigned short mn[3];
        unsigned short mx[3];
        float *out = (float *)0x70002000;
        unsigned char *tri;
        unsigned char *tend;

        for (i = 0; i < 3; i++) {
            mn[i] = (unsigned short)ftoi0_1F34F0(bmin[i]);
            mx[i] = (unsigned short)ftoi0_1F34F0(bmax[i]);
        }
        do {
            int k;
            for (k = 0; k < 2; k++) {
                unsigned int code = 0;
                for (i = 0; i < 3; i++) {
                    unsigned short h = vtx[i];
                    out[i] = (float)h;
                    if (((unsigned short)(h - mn[i])) & 0x8000) {
                        code |= 1u << (8 * i);
                    }
                    if (((unsigned short)(mx[i] - h)) & 0x8000) {
                        code |= 2u << (8 * i);
                    }
                }
                ((unsigned int *)out)[3] = code;
                vf2w = vtx[3];
                vtx += 4;
                out += 4;
            }
            nv -= 2;
        } while (nv > 0);

        tri = (unsigned char *)(*(unsigned short **)(ent + 0xC)) + *(unsigned short *)(ent + 0xA) * 8;
        tend = tri + ent[8] * 4;
        for (; tri != tend; tri += 4) {
            float *a = (float *)(unsigned long)(0x70002000 + tri[0] * 16);
            float *b = (float *)(unsigned long)(0x70002000 + tri[1] * 16);
            float *c = (float *)(unsigned long)(0x70002000 + tri[2] * 16);
            float e1[3], e2[3], p[3], N[3], NQ[3], pp[3], q[3], f[3];
            float c1[3], c2[3], c3[3], t3[3];
            float A[3], B[3];
            float d, n2, Q, pd2, rr, s1, s2, s3, dist2;
            int inside = 0;

            if ((((unsigned int *)a)[3] & ((unsigned int *)b)[3] & ((unsigned int *)c)[3]) != 0) {
                continue;
            }
            vf2w = ((unsigned int *)b)[3];
            for (i = 0; i < 3; i++) {
                e1[i] = b[i] - a[i];
                e2[i] = c[i] - a[i];
                p[i] = P[i] - a[i];
            }
            cross_1F34F0(N, e2, e1);
            d = dot_1F34F0(p, N);
            n2 = dot_1F34F0(N, N);
            Q = vdiv_1F34F0(d, n2);
            if (neg_1F34F0(d)) {
                continue;
            }
            for (i = 0; i < 3; i++) {
                NQ[i] = N[i] * Q;
                pp[i] = p[i] - N[i] * Q;
            }
            for (i = 0; i < 3; i++) {
                NQ[i] = NQ[i] * NQ[i];
            }
            pd2 = (NQ[0] + NQ[1]) + NQ[2];
            for (i = 0; i < 3; i++) {
                f[i] = e2[i] - e1[i];
                q[i] = pp[i] - e1[i];
            }
            cross_1F34F0(c1, pp, e1);
            cross_1F34F0(c2, e2, pp);
            rr = best - pd2;
            cross_1F34F0(c3, q, f);
            if (neg_1F34F0(rr)) {
                continue;
            }
            s1 = dot_1F34F0(c1, N);
            s2 = dot_1F34F0(c2, N);
            s3 = dot_1F34F0(c3, N);
            dist2 = pd2;
            if (neg_1F34F0(s1)) {
                A[0] = A[1] = A[2] = 0.0f;
                for (i = 0; i < 3; i++) {
                    B[i] = e1[i] + 0.0f;
                }
            } else if (neg_1F34F0(s2)) {
                for (i = 0; i < 3; i++) {
                    A[i] = e2[i] + 0.0f;
                }
                B[0] = B[1] = B[2] = 0.0f;
            } else if (!neg_1F34F0(s3)) {
                inside = 1;
            } else {
                for (i = 0; i < 3; i++) {
                    A[i] = e1[i] + 0.0f;
                    B[i] = e2[i] + 0.0f;
                }
            }
            if (!inside) {
                float g[3];
                float t;
                for (i = 0; i < 3; i++) {
                    f[i] = pp[i] - A[i];
                    g[i] = B[i] - A[i];
                }
                t = vdiv_1F34F0(dot_1F34F0(f, g), dot_1F34F0(g, g));
                t = 0.0f + t;
                if (!(t > 0.0f)) {
                    t = 0.0f;
                }
                if (t > 1.0f) {
                    t = 1.0f;
                }
                for (i = 0; i < 3; i++) {
                    pp[i] = A[i] + g[i] * t;
                    t3[i] = pp[i] - p[i];
                    t3[i] = t3[i] * t3[i];
                }
                dist2 = (t3[0] + t3[1]) + t3[2];
                if (neg_1F34F0(best - dist2)) {
                    continue;
                }
            }
            mat = tri[3] + 0x1000;
            best = dist2 * 0.999499977f;
            for (i = 0; i < 3; i++) {
                hit_n[i] = N[i] + 0.0f;
                hit_pt[i] = pp[i] + a[i];
                hit_v0[i] = 0.0f + a[i];
                hit_v1[i] = e1[i] + a[i];
                hit_v2[i] = e2[i] + a[i];
            }
        }
    }

    if (mat == 0) {
        return 0;
    }
    {
        float *o = (float *)D_L00_00173F40;
        float Q;
        float push[3];
        if (best == 0.0f) {
            Q = FMAX_1F34F0;
        } else {
            Q = r64 / __builtin_sqrtf(best < 0.0f ? -best : best);
            if (Q > FMAX_1F34F0) {
                Q = FMAX_1F34F0;
            }
        }
        for (i = 0; i < 3; i++) {
            push[i] = P[i] - hit_pt[i];
            hit_pt[i] = hit_pt[i] * 0.015625f;
            hit_v0[i] = hit_v0[i] * 0.015625f;
            hit_v1[i] = hit_v1[i] * 0.015625f;
            hit_v2[i] = hit_v2[i] * 0.015625f;
        }
        for (i = 0; i < 3; i++) {
            o[8 + i] = hit_pt[i];
            o[16 + i] = hit_n[i];
            o[20 + i] = hit_v0[i];
            o[24 + i] = hit_v1[i];
            o[28 + i] = hit_v2[i];
            o[12 + i] = push[i] * Q * 0.015625f + hit_pt[i];
        }
        o[11] = 0.0f;
        o[19] = 0.0f;
        o[23] = 0.0f;
        o[27] = 0.0f;
        o[31] = 0.0f;
        ((unsigned int *)o)[15] = vf2w;
        ((unsigned int *)o)[6] = 0;
        ((unsigned int *)o)[7] = mat;
    }
    return 1;
}
