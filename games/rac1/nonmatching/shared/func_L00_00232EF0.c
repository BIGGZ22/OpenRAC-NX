/* NON_MATCHING func_L00_00232EF0 -- src/overlays/shared/help_00232560.c
 * Best so far: SIZE ours 1288 / retail 1312, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   func_L00_00232EF0 (RatchetAnimAdvance): advances the player animation by the frame step, handles sequence tran
 *   Understood and reproduced: the five separate D_0013E633+0xE1D pointers (top, loop prelude, third loop branch, 
 *   Not reproduced: retail hoists `mtc1 $0,$f22` and stores case 1's r[0x54]=0 with swc1 (ours sw $zero), delta la
 *   Unblock: find the source form that makes gcc keep a float zero in a register across the loop (and another save
 */
extern void func_L00_00232B78(void);
extern void func_L00_00232980(void);
extern void func_L00_00232B20(void);
extern void func_L00_0028EBF0(int);
extern int func_0022ED80(int, int, void *);
extern float func_0020D830(void *);
extern float D_L00_0017BEF0[];
extern float D_L00_0015F7E8;
extern short D_L00_0015F6A8_s __asm__("D_L00_0015F6A8");

// Advances the player model's animation by the frame step, running sequence changes and sounds.
void func_L00_00232EF0(void) {
    char *g = D_0013E633 + 0xE1D;
    char *gB, *gL, *gC, *gD;
    float cur2, x, z;
    unsigned char *r = *(unsigned char **)(g + 0x2080);
    unsigned char a, b, prev;
    int same, i, n, lo, hi, v, t;
    float old, delta, cur, k, tmp;
    char *seq, *p, *e;
    func_L00_00211F68();
    *(int *)(g + 0xA98) &= -4;
    a = r[0x52];
    b = r[0x53];
    prev = r[0x50];
    old = *(float *)(r + 0x54);
    same = a == b;
    if (a == b) {
        *(float *)(r + 0x54) = old + *(float *)(g + 0xA90) * *(float *)(g + 0xA94);
    } else if (*(int *)(g + 0xAA0) >= 0) {
        i = *(int *)(g + 0xAA4);
        *(float *)(r + 0x54) = *(float *)((char *)D_L00_0017BEF0 + *(int *)(g + 0xAA0) * 100 + i * 4);
        *(int *)(g + 0xAA4) = i + 1;
    } else {
        *(float *)(r + 0x54) = old + *(float *)(g + 0xA94);
    }
    if (*(float *)(r + 0x54) > 0.99f && *(float *)(r + 0x54) < 1.01f) {
        *(float *)(r + 0x54) = 1.0f;
    }
    if (*(float *)(r + 0x54) > -0.01f && *(float *)(r + 0x54) < 0.01f) {
        *(float *)(r + 0x54) = 0.0f;
    }
    x = *(float *)(r + 0x54);
    delta = x - old;
    if (1.0f <= x) {
        k = D_L00_0015F7E8;
        z = 0.0f;
        while (1.0f <= x) {
            if (r[0x52] != r[0x53]) {
                r[0x52] = r[0x53];
                r[0x7E] = *(unsigned char *)(*(char **)(*(char **)(r + 0x24) + r[0x53] * 4 + 0x48) + 0x12);
            }
            gL = D_0013E633 + 0xE1D;
            *(int *)(gL + 0xAA0) = -1;
            *(int *)(gL + 0xA98) |= 1;
            r[0x50] = r[0x51];
            *(int *)(r + 0x68) = *(int *)(r + 0x6C);
            r[0x51] = r[0x51] + 1;
            if (*(int *)(gL + 0xAB8) != 0) {
                *(int *)(gL + 0xAB8) = 0;
                *(float *)(gL + 0xA94) = k;
                *(float *)(r + 0x54) = z;
                r[0x51] = *(unsigned char *)(gL + 0xAB4);
                *(int *)(r + 0x6C) = *(int *)(*(char **)(*(char **)(r + 0x24) + r[0x53] * 4 + 0x48) + (unsigned char)r[0x51] * 4 + 0x1C);
                func_L00_00232EA8();
                func_L00_00232B78();
            } else if (*(int *)(gL + 0xAB0) != -1 && *(int *)(gL + 0xAB4) < (int)(unsigned char)r[0x51]) {
                *(float *)(gL + 0xA94) = 0.33333334f;
                *(int *)(r + 0x54) = 0;
                r[0x51] = *(unsigned char *)(gL + 0xAB0);
                *(int *)(r + 0x6C) = *(int *)(*(char **)(*(char **)(r + 0x24) + r[0x53] * 4 + 0x48) + (unsigned char)r[0x51] * 4 + 0x1C);
                func_L00_00232B78();
            } else {
                gB = D_0013E633 + 0xE1D;
                seq = *(char **)(*(char **)(r + 0x24) + (unsigned char)r[0x53] * 4 + 0x48);
                if (!((unsigned char)r[0x51] < (unsigned char)seq[0x10])) {
                    r[0x51] = 0;
                    *(int *)(gB + 0xA98) |= 2;
                }
                *(float *)(r + 0x54) -= 1.0f;
                *(int *)(r + 0x6C) = *(int *)(*(char **)(*(char **)(r + 0x24) + (unsigned char)r[0x53] * 4 + 0x48) + (unsigned char)r[0x51] * 4 + 0x1C);
                *(float *)(r + 0x54) /= *(float *)(gB + 0xA94);
                *(float *)(gB + 0xA94) = **(float **)(r + 0x68);
                *(float *)(r + 0x54) *= *(float *)(gB + 0xA94);
            }
            x = *(float *)(r + 0x54);
        }
    }
    if (same != 0 && r[0x7E] != 0) {
        hi = ((unsigned char)r[0x50] << 4) + func_001FA898(*(float *)(r + 0x54) * 16.0f);
        lo = (prev << 4) + func_001FA898(old * 16.0f);
        seq = *(char **)(*(char **)(r + 0x24) + (unsigned char)r[0x52] * 4 + 0x48);
        p = seq + 0x1C + (unsigned char)seq[0x10] * 4;
        for (n = 0; n < (unsigned char)seq[0x12]; n++, p += 4) {
            v = *(int *)p;
            if (lo < (v >> 16) && !(hi < (v >> 16))) {
                func_0022ED80(v & 0xFFFF, 0, r);
                break;
            }
        }
    }
    t = (unsigned char)r[0x7D];
    if (t != 0xFF) {
        e = D_0013E633 + 0x1D + t * 0x70;
        if (*(unsigned char **)(e + 0x88) == r) {
            if (*(short *)(e + 0x7E) != (unsigned char)r[0x7C]) {
                func_L00_0028EBF0(t);
                r[0x7D] = 0xFF;
            }
        } else {
            r[0x7D] = 0xFF;
        }
    } else if ((unsigned char)r[0x7C] != t) {
        if (*(int *)&D_L00_0015F6A8_s != 2 && *(int *)&D_L00_0015F6A8_s != 6) {
            r[0x7D] = func_0022ED80((unsigned char)r[0x7C], 4, r);
        }
    }
    func_L00_00232980();
    func_L00_00232B20();
    gC = D_0013E633 + 0xE1D;
    tmp = *(float *)(gC + 0xAA8);
    cur2 = func_0020D830(r);
    *(float *)(gC + 0xAA8) = cur2;
    if (tmp <= cur2) {
        *(float *)(gC + 0xAAC) = cur2 - tmp;
    } else {
        *(float *)(gC + 0xAAC) = delta;
    }
    gD = D_0013E633 + 0xE1D;
    *(int *)(gD + 0xA9C) = r[0x52] != r[0x53];
}
