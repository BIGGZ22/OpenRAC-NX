/* functional: high. fcheck: unchecked (never called in fcheck's capture run of the title, menu and level 0; OpenRAC tools/fcheck_gate.py, 2026-10-18). Corrected from nonmatching/game/func_0020EEE8.c: its three float-to-int conversions (the cell extent and W) were plain C casts, undefined out of range; the VU vftoi0 saturates (0x7FFFFFFF, or 0x80000000 below), so these do too. Checked against the asm: the entry at 0x20EEE8 is func_0020ED48's flag-0x100 path, which loads the rows and jumps to the tail at 0x20ED80; the seq/prev/cache branches, the 0x8000 row-1 negate, the stores, the packed cell test and the call to func_0020EA70 all match. */
extern void func_0020EA70(void *, int);
extern char D_001B2F80[];

static int ftoi_sat(float v)
{
    if (v >= 2147483648.0f) {
        return 0x7FFFFFFF;
    }
    if (v <= -2147483648.0f) {
        return (int)0x80000000u;
    }
    return (int)v;
}

void func_0020EEE8(void *mp)
{
    char *m = (char *)mp;
    signed char st = *(signed char *)(m + 0x20);
    unsigned short flags;
    unsigned char seq, prev, u71;
    char *cls;
    float scale, t;
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

    for (k = 0; k < 3; k++) {
        for (i = 0; i < 4; i++) {
            rows[k][i] = ((float *)(m + 0xC0 + k * 0x10))[i];
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
        int X = ftoi_sat(q[0]);
        int Y = ftoi_sat(q[1]);
        int W = ftoi_sat(P[3]);
        int b0 = ((int)((unsigned)X - (unsigned)W)) >> 14;
        int b1 = ((int)((unsigned)Y - (unsigned)W)) >> 14;
        int b2 = ((int)((unsigned)X + (unsigned)W)) >> 14;
        int b3 = ((int)((unsigned)Y + (unsigned)W)) >> 14;
        unsigned int pk = ((unsigned)b0 & 0xFF) | (((unsigned)b1 & 0xFF) << 8)
                        | (((unsigned)b2 & 0xFF) << 16) | (((unsigned)b3 & 0xFF) << 24);

        /* Retail compares the stored word sign-extended (lw), so the test is on the 64-bit value. */
        if ((unsigned long)(long)*(int *)(m + 0xA0) == (unsigned long)pk) {
            return;
        }
        if (pk & 0xC0C0) {
            return;
        }
        func_0020EA70(mp, (int)pk);
    }
}
