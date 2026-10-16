/*
 * MobyBuildMatrix(moby): the moby's rotation rows and its bounding sphere in the world, then its cells
 * in the moby grid. As ReRAC's rebuild_matrix (crates/rc-game/src/moby_update/scheduler.rs; ISC
 * License, Copyright (c) 2026 ReRAC contributors), read against the hand-written routine:
 * - nothing for a deleted moby (state byte +0x20 negative);
 * - the rows: from the Euler angles at +0x40 (func_001FA1F8, the VU0 program's R = Rz.Ry.Rx), or the
 *   stored ones with mode 0x100; row 1 negated with mode 0x8000 (kept rows too); stored at +0xC0;
 * - the sphere: key A's sequence's (cached at +0xF0, its sequence at +0x71) when A = B, else lerped
 *   from A's (the snapshot table at frame slot +0x50 when A is 0xFF) to B's by t at +0x54; scaled by
 *   +0x2C, rotated, and placed at position * 1024 into +0x00 (radius in w);
 * - the change counter +0xA8 counted;
 * - with a collision blob (+0x94), the grid rectangle from the sphere ((c -/+ r) >> 14 per axis,
 *   bytes x0, y0, x1, y1): unless equal to +0xA0 or off the grid (bits 6/7 of x0 or y0),
 *   UpdateMobyGrids (the executable's func_0020EA70, the level's func_L00_00251B58).
 */
extern char D_L00_00197080[];
extern void func_001FA1F8(float *out, float *in);
extern void func_0020EA70(unsigned char *m, unsigned int rect);

void func_L00_00251E30(unsigned char *m) {
    float rows[3][4];
    float sphere[4];
    float s5[4];
    float c[3];
    float scale;
    unsigned short flags;
    unsigned int a, b, rect, old;
    char *cls;
    int i, k, ix, iy, ir;

    if ((signed char)m[0x20] < 0) {
        return;
    }
    flags = *(unsigned short *)(m + 0x34);
    cls = *(char **)(m + 0x24);
    scale = *(float *)(m + 0x2C);
    if (flags & 0x100) {
        for (i = 0; i < 3; i++) {
            for (k = 0; k < 4; k++) {
                rows[i][k] = ((float *)(m + 0xC0))[i * 4 + k];
            }
        }
    } else {
        func_001FA1F8(&rows[0][0], (float *)(m + 0x40));
    }

    a = m[0x52];
    b = m[0x53];
    if (a != b) {
        float t = *(float *)(m + 0x54);
        float *pb = *(float **)(cls + 0x48 + b * 4);
        float *pa = a == 0xFF ? (float *)(D_L00_00197080 + m[0x50] * 16) : *(float **)(cls + 0x48 + a * 4);

        for (k = 0; k < 4; k++) {
            sphere[k] = (pb[k] * t + pa[k] * 1.0f) - pa[k] * t;
        }
    } else if (a != m[0x71]) {
        float *pa = *(float **)(cls + 0x48 + a * 4);

        m[0x71] = (unsigned char)a;
        for (k = 0; k < 4; k++) {
            sphere[k] = pa[k];
            ((float *)(m + 0xF0))[k] = pa[k];
        }
    } else {
        for (k = 0; k < 4; k++) {
            sphere[k] = ((float *)(m + 0xF0))[k];
        }
    }

    if (flags & 0x8000) {
        for (k = 0; k < 3; k++) {
            rows[1][k] = 0.0f - rows[1][k];
        }
    }
    for (i = 0; i < 3; i++) {
        for (k = 0; k < 4; k++) {
            ((float *)(m + 0xC0))[i * 4 + k] = rows[i][k];
        }
    }

    for (k = 0; k < 4; k++) {
        s5[k] = sphere[k] * scale;
    }
    for (k = 0; k < 3; k++) {
        float p = ((float *)(m + 0x10))[k] * 1024.0f;

        c[k] = ((rows[0][k] * s5[0] + rows[1][k] * s5[1]) + rows[2][k] * s5[2]) + p * 1.0f;
    }
    ((float *)m)[0] = c[0];
    ((float *)m)[1] = c[1];
    ((float *)m)[2] = c[2];
    ((float *)m)[3] = s5[3];
    *(unsigned short *)(m + 0xA8) = (unsigned short)(*(unsigned short *)(m + 0xA8) + 1);

    if (*(int *)(m + 0x94) == 0) {
        return;
    }
    /* vftoi0: truncation toward zero, saturating. */
    ix = c[0] >= 2147483520.0f ? 0x7FFFFFFF : c[0] <= -2147483648.0f ? (int)0x80000000 : (int)c[0];
    iy = c[1] >= 2147483520.0f ? 0x7FFFFFFF : c[1] <= -2147483648.0f ? (int)0x80000000 : (int)c[1];
    ir = s5[3] >= 2147483520.0f ? 0x7FFFFFFF : s5[3] <= -2147483648.0f ? (int)0x80000000 : (int)s5[3];
    rect = ((unsigned int)((ix - ir) >> 14) & 0xFF) | (((unsigned int)((iy - ir) >> 14) & 0xFF) << 8)
           | (((unsigned int)((ix + ir) >> 14) & 0xFF) << 16) | (((unsigned int)((iy + ir) >> 14) & 0xFF) << 24);
    old = *(unsigned int *)(m + 0xA0);
    /* Retail compares the packed rectangle (upper half zero) with the sign-extended word. */
    if ((long)(unsigned long)rect == (long)(int)old) {
        return;
    }
    if (rect & 0xC0C0) {
        return;
    }
    func_0020EA70(m, rect);
}
