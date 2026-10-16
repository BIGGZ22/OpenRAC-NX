/*
 * Moby transform update (PAL 0x20ED48). Rows: from the Euler angles at moby+0x40 (the VU0
 * program at 0xD18: sin/cos of each angle, then Rx, Ry, Rz applied to the identity), or the
 * stored rows when flag 0x100 is set; flag 0x8000 negates row 1. The rows go to 0xC0/0xD0/0xE0
 * and the transformed extent to 0x0. Grid cells of that extent, packed as bytes, are compared
 * with moby+0xA0; a change inside the cell range calls func_0020EA70 with the packed word.
 * The vector at 0xF0 is the sequence's first vector, blended from the previous sequence by
 * moby+0x54 when the sequence changes (seq 0xFF reads the D_001B2F80 slot at moby+0x50).
 */
extern void func_0020EA70(void *, int);
extern char D_001B2F80[];

void func_0020ED48(void *mp)
{
    char *m = (char *)mp;
    signed char st = *(signed char *)(m + 0x20);
    unsigned short flags;
    unsigned char seq, prev, u71;
    char *cls;
    float scale, t;
    float a[3], arg[2], sc[2];
    float rows[3][4];
    float cur[4], P[4], q[4];
    float *s, *ap;
    int i, j, k;

    if (st < 0) {
        return;
    }
    flags = *(unsigned short *)(m + 0x34);
    cls = *(char **)(m + 0x24);
    seq = *(unsigned char *)(m + 0x52);
    prev = *(unsigned char *)(m + 0x53);
    scale = *(float *)(m + 0x2C);

    if (flags & 0x100) {
        for (k = 0; k < 3; k++) {
            for (i = 0; i < 4; i++) {
                rows[k][i] = ((float *)(m + 0xC0 + k * 0x10))[i];
            }
        }
    } else {
        rows[0][0] = 1.0f; rows[0][1] = 0.0f; rows[0][2] = 0.0f; rows[0][3] = 0.0f;
        rows[1][0] = 0.0f; rows[1][1] = 1.0f; rows[1][2] = 0.0f; rows[1][3] = 0.0f;
        rows[2][0] = 0.0f; rows[2][1] = 0.0f; rows[2][2] = 1.0f; rows[2][3] = 0.0f;
        for (i = 0; i < 3; i++) {
            a[i] = ((float *)(m + 0x40))[i] + 0.0f;
        }
        for (i = 0; i < 3; i++) {
            float y0[4], y1[4], y2[4], n[4];
            float sn, cs;
            if (a[i] == 0.0f) {
                continue;
            }
            arg[0] = a[i];
            arg[1] = a[i] + 1.57079625f;
            for (j = 0; j < 2; j++) {
                float x = arg[j];
                float pi = 3.1415925f;
                float npi = -3.1415925f;
                float fm = (pi - x) < x ? (pi - x) : x;
                float g = npi - x;
                float x2, x3, x5, x7, x9, acc;
                fm = g > fm ? g : fm;
                x2 = fm * fm;
                x3 = fm * x2;
                x5 = x3 * x2;
                x7 = x5 * x2;
                x9 = x7 * x2;
                acc = fm * 1.0f;
                acc = acc + x3 * -0.166666567f;
                acc = acc + x5 * 0.0083330255f;
                acc = acc + x7 * -0.000198074107f;
                sc[j] = acc + x9 * 2.60188699e-6f;
            }
            sn = sc[0];
            cs = sc[1];
            if (i == 0) {
                rows[1][0] = 0.0f; rows[1][1] = cs; rows[1][2] = sn; rows[1][3] = 0.0f;
                rows[2][0] = 0.0f; rows[2][1] = 0.0f - sn; rows[2][2] = cs; rows[2][3] = 0.0f;
                continue;
            }
            if (i == 1) {
                y0[0] = cs; y0[1] = 0.0f; y0[2] = 0.0f - sn; y0[3] = 0.0f;
                y1[0] = 0.0f; y1[1] = 1.0f; y1[2] = 0.0f; y1[3] = 0.0f;
                y2[0] = sn; y2[1] = 0.0f; y2[2] = cs; y2[3] = 0.0f;
            } else {
                y0[0] = cs; y0[1] = sn; y0[2] = 0.0f; y0[3] = 0.0f;
                y1[0] = 0.0f - sn; y1[1] = cs; y1[2] = 0.0f; y1[3] = 0.0f;
                y2[0] = 0.0f; y2[1] = 0.0f; y2[2] = 1.0f; y2[3] = 0.0f;
            }
            for (k = 0; k < 3; k++) {
                for (j = 0; j < 4; j++) {
                    n[j] = ((y0[j] * rows[k][0] + y1[j] * rows[k][1]) + y2[j] * rows[k][2]);
                }
                for (j = 0; j < 4; j++) {
                    rows[k][j] = n[j];
                }
            }
        }
    }

    u71 = *(unsigned char *)(m + 0x71);
    if (seq != prev) {
        if (seq == 0xFF) {
            s = (float *)(D_001B2F80 + *(unsigned char *)(m + 0x50) * 16);
        } else {
            s = *(float **)(cls + 0x48 + seq * 4);
        }
        ap = *(float **)(cls + 0x48 + prev * 4);
        t = *(float *)(m + 0x54);
        for (k = 0; k < 4; k++) {
            cur[k] = (ap[k] * t + s[k]) - s[k] * t;
        }
    } else if (seq != u71) {
        s = *(float **)(cls + 0x48 + seq * 4);
        *(unsigned char *)(m + 0x71) = seq;
        for (k = 0; k < 4; k++) {
            cur[k] = s[k];
            ((float *)(m + 0xF0))[k] = s[k];
        }
    } else {
        for (k = 0; k < 4; k++) {
            cur[k] = ((float *)(m + 0xF0))[k];
        }
    }

    if (flags & 0x8000) {
        for (k = 0; k < 3; k++) {
            rows[1][k] = 0.0f - rows[1][k];
        }
    }
    for (k = 0; k < 3; k++) {
        for (i = 0; i < 4; i++) {
            ((float *)(m + 0xC0 + k * 0x10))[i] = rows[k][i];
        }
    }

    for (k = 0; k < 4; k++) {
        P[k] = cur[k] * scale;
    }
    for (k = 0; k < 3; k++) {
        float tt = rows[0][k] * P[0];
        tt = tt + rows[1][k] * P[1];
        tt = tt + rows[2][k] * P[2];
        q[k] = tt + ((float *)(m + 0x10))[k] * 1024.0f;
    }
    ((float *)m)[0] = q[0];
    ((float *)m)[1] = q[1];
    ((float *)m)[2] = q[2];
    ((float *)m)[3] = P[3];

    *(unsigned short *)(m + 0xA8) = (unsigned short)(*(unsigned short *)(m + 0xA8) + 1);
    if (*(int *)(m + 0x94) == 0) {
        return;
    }

    {
        int X = (int)q[0];
        int Y = (int)q[1];
        int W = (int)P[3];
        int b0 = ((int)((unsigned)X - (unsigned)W)) >> 14;
        int b1 = ((int)((unsigned)Y - (unsigned)W)) >> 14;
        int b2 = ((int)((unsigned)X + (unsigned)W)) >> 14;
        int b3 = ((int)((unsigned)Y + (unsigned)W)) >> 14;
        unsigned int pk = ((unsigned)b0 & 0xFF) | (((unsigned)b1 & 0xFF) << 8)
                        | (((unsigned)b2 & 0xFF) << 16) | (((unsigned)b3 & 0xFF) << 24);

        if ((unsigned long)(long)*(int *)(m + 0xA0) == (unsigned long)pk) {
            return;
        }
        if (pk & 0xC0C0) {
            return;
        }
        func_0020EA70(mp, (int)pk);
    }
}
