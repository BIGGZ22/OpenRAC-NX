/* functional: high. fcheck: unchecked (level/overlay function; fcheck runs boot-executable functions only; OpenRAC tools/fcheck_gate.py, 2026-10-18). Builds 4 GIF/VIF packets (a ribbon of S[5] quads per strip, strip index 3..0) at the packet pointer D_00161240 behind the templates D_L06_001C4AA0/00160F40/00160F60/00160F80; VU0 maths re-traced against the assembly (row-vector matrix product, ST pairs, colour lerp, pack to bytes) and agrees. Corrected from itself: VU0 never gives inf or NaN, so the two 1/x (vdiv by 0 gives FLT_MAX), the products and the float-to-int (saturating) are now clamped; the C divided by zero when S[0]==S[1] or S[1]==0. Not reproduced: the assembly leaves the packet cursors in $24/$25 (not an output). Hand-traced, cannot be run. */
extern unsigned int *D_00161240 MACRO_ADDR;
extern unsigned int D_L06_001C4AA0_16B38[] __asm__("D_L06_001C4AA0");
extern unsigned int D_L06_00160F40_16B38[] __asm__("D_L06_00160F40");
extern unsigned int D_L06_00160F60_16B38[] __asm__("D_L06_00160F60");
extern unsigned int D_L06_00160F80_16B38[] __asm__("D_L06_00160F80");

static float vu_div1_16B38(float den) { return den == 0.0f ? 3.4028235e38f : 1.0f / den; }
static float vu_clamp_16B38(float v) {
    if (v > 3.4028235e38f) return 3.4028235e38f;
    if (v < -3.4028235e38f) return -3.4028235e38f;
    return v;
}
static int vu_ftoi_16B38(float v) {
    if (v >= 2147483648.0f) return 0x7FFFFFFF;
    if (v <= -2147483648.0f) return (int)0x80000000;
    return (int)v;
}

void func_L06_00216B38(int *S, float *M) {
    int idx = 4;
    float step_v = 0.05f;

    do {
        float *buf1 = (float *)0x70001000;
        float *buf2 = (float *)0x70002000;
        float *vp;
        float u4, u5, step;
        int n, n2, k, val, d, c;
        unsigned int sh, col;
        float inv_rng, inv_s4, x, w, fa[4], fb[4], r;
        unsigned int *p, *start, *src, *s2;
        unsigned int cw;
        int ci;

        idx--;
        u4 = *(float *)&S[8];
        u5 = *(float *)&S[8];
        step = *(float *)&S[7];
        n = S[5];
        vp = (float *)((char *)S[6] + idx * n * 16);
        k = n;
        do {
            float vx, vy, vz, lx, ly, lz;
            u4 = u4 + step;
            u5 = u5 + step;
            vx = vp[0]; vy = vp[1]; vz = vp[2];
            lz = vz + -step_v;          /* z - 0.05 */
            vz = vz + step_v;           /* z + 0.05 */
            lx = vx; ly = vy;
            /* upper vertex (z + 0.05) goes second, lower (z - 0.05) first */
            for (c = 0; c < 4; c++) {
                buf1[c] = ((M[12 + c] * 1.0f + M[8 + c] * lz) + M[4 + c] * ly) + M[c] * lx;
                buf1[4 + c] = ((M[12 + c] * 1.0f + M[8 + c] * vz) + M[4 + c] * vy) + M[c] * vx;
            }
            buf2[0] = u4;
            buf2[1] = 0.0f;
            buf2[2] = u5;
            buf2[3] = 1.0f;
            buf1 += 8;
            buf2 += 4;
            vp += 4;
            k--;
        } while (k != 0);

        buf1 = (float *)0x70001000;
        buf2 = (float *)0x70002000;

        inv_rng = vu_div1_16B38((float)(S[0] - S[1]));
        inv_s4 = vu_div1_16B38((float)S[1]);
        val = ((int *)S[2])[idx];
        d = val - S[1];
        if (d > 0) {
            x = vu_clamp_16B38((float)d * inv_rng);
        } else {
            x = vu_clamp_16B38((float)val * inv_s4);
            w = 1.0f - x;
            x = w;
        }
        w = 1.0f - x;
        for (c = 0; c < 4; c++) {
            fa[c] = (float)((unsigned char)((unsigned int)S[3] >> (8 * c)));
            fb[c] = (float)((unsigned char)((unsigned int)S[4] >> (8 * c)));
        }
        col = 0;
        for (c = 0; c < 4; c++) {
            r = vu_clamp_16B38(vu_clamp_16B38(fa[c] * w) + vu_clamp_16B38(fb[c] * x));
            ci = vu_ftoi_16B38(r);
            col |= ((unsigned int)ci & 0xFF) << (8 * c);
        }

        n2 = S[5] * 2;
        sh = (unsigned int)n2 << 16;
        p = D_00161240;
        start = p;
        src = D_L06_001C4AA0_16B38;
        for (k = 0; k < 5; k++) {
            p[0] = src[0]; p[1] = src[1]; p[2] = src[2]; p[3] = src[3];
            src += 4;
            p += 4;
        }
        p[-16] |= (unsigned int)n2;
        src = D_L06_00160F40_16B38;
        p[0] = src[0]; p[1] = src[1]; p[2] = src[2]; p[3] = src[3];
        p += 4;
        p[-1] |= sh;
        s2 = (unsigned int *)buf2;
        for (k = 0; k < n2; k++) {
            p[0] = s2[0];
            p[1] = s2[1];
            s2 += 2;
            p += 2;
        }
        while (((unsigned int)p & 0xF) != 0) {
            *p = 0;
            p++;
        }
        src = D_L06_00160F60_16B38;
        p[0] = src[0]; p[1] = src[1]; p[2] = src[2]; p[3] = src[3];
        p += 4;
        p[-1] |= sh;
        for (k = 0; k < n2; k++) {
            *p = col;
            p++;
        }
        while (((unsigned int)p & 0xF) != 0) {
            *p = 0;
            p++;
        }
        src = D_L06_00160F80_16B38;
        p[0] = src[0]; p[1] = src[1]; p[2] = src[2]; p[3] = src[3];
        p += 4;
        p[-1] |= sh;
        s2 = (unsigned int *)buf1;
        for (k = 0; k < n2; k++) {
            p[0] = s2[0];
            p[1] = s2[1];
            p[2] = s2[2];
            s2 += 4;
            p += 3;
        }
        while (((unsigned int)p & 0xF) != 0) {
            *p = 0;
            p++;
        }
        p[0] = 0x15000000u | 0xE;
        p[1] = 0x11000000u;
        p[2] = 0;
        p[3] = 0;
        p += 4;
        cw = (((unsigned int)p - (unsigned int)start) >> 4) - 1;
        start[0] |= cw;
        D_00161240 = p;
    } while (idx != 0);
}
