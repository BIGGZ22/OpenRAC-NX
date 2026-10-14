/* NON_MATCHING func_L05_00305778 -- src/overlays/l05_rilgar/vendor_002D28D0.c
 * Best so far: BYTES 12/3672 (99.7% of the bytes match), checked 2026-10-14.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   00250800 call; assigning them inside loop 1 (or not at all) gives the exact placement but
 *   the other slot order (GCSE seems to rename the hoisted values). Shifting d's pseudo number
 *   (p17) changed nothing.
 *   Lessons: case-level `g` shared by three blocks went to global-alloc (a1); block-local g2
 *   fixed the spawn blocks. Float stores before the int store at 0x70 fixed the f0-f3 choice
 *   (int/float stores do not alias). `extern const float` for D_L05_00161C38 let its load
 *   schedule before the m+0x48 load/0x194 store.
 *   - run23 p22: no a1xx locals at all (tail uses d+0x...) -> identical BYTES 12; simpler source, new best
 */
void func_L05_003065D0(char *moby);
void func_L05_00306CF0(char *m);
extern void func_L00_0025B178(void *);
extern void func_0020D960(char *, int, unsigned char *);
extern void func_001F9BC0(void *);
extern int func_L00_0025A060(char *m, char *p, float *pos, char *v);
extern int func_L00_0025A778(void *, void *, int);
extern float func_L00_0025CC58(float *, int, float, float);
extern void func_L00_001FFED8(void *, int, float);
extern char D_L05_001672C0[];
extern short D_L05_00161BB8;
extern const float D_L05_00161C38_c SDATA(D_L05_00161C38);
extern short D_L05_00161C3C;
extern short D_L05_00161BF0;
extern short D_L05_00161BF4;
extern short D_L05_00161BE0;
extern short D_L05_00161BE4;
extern short D_L05_00161BF8;
extern short D_L05_00161BE8;
extern short D_L05_00161BEC;
extern short D_L05_00161BD4;
extern short D_L05_00161BD8;
extern short D_L05_00161BDC;
extern short D_L05_00161BCC;
extern short D_L05_00161BD0;
extern short D_L05_00161C20;
extern short D_L05_00161C24;
extern short D_L05_00161C2C;
extern short D_L05_00161C10;
extern short D_L05_00161C14;
extern short D_L05_00161C28;
extern short D_L05_00161C18;
extern short D_L05_00161C1C;
extern short D_L05_00161C04;
extern short D_L05_00161C08;
extern short D_L05_00161C0C;
extern short D_L05_00161BFC;
extern short D_L05_00161C00;
typedef int u128_305778 __attribute__((mode(TI)));

/* Extermitank update: spawns its two parts, follows its path, turns its turret toward the hero and sprays particles while firing. */
void func_L05_00305778(unsigned char *m) {
    char *d = *(char **)(m + 0x78);
    float v10[4];
    float v20[4];
    float v30[4];
    float v40[4];

    func_L05_003065D0((char *)m);
    func_L05_00306CF0((char *)m);
    if (*(unsigned char *)(d + 0x2E) == 2) {
        if (*(char **)(d + 0x184) != 0) func_0020D678(*(char **)(d + 0x184));
        if (*(char **)(d + 0x188) != 0) func_0020D678(*(char **)(d + 0x188));
        func_0020D678(m);
        return;
    }
    if (m[0x31] != 0) {
        if (func_001F9D10(m + 0x10, D_L05_001672C0) < 27.0f) {
            func_L00_0025B178(m);
            m[0x7F] = 0x15;
        }
    }
    switch (m[0x20]) {
    case 0: {
        char *c;
        char *g;
        if (*(char **)(d + 0x184) == 0) {
            *(char **)(d + 0x184) = func_0020D348(0x368);
            (*(char **)(d + 0x184))[0x30] = 0x40;
            *(short *)(*(char **)(d + 0x184) + 0x32) = 0x40;
            (*(char **)(d + 0x184))[0x31] = 1;
            {
                char *g2 = D_0013E633 + 0xE1D;
                *(long *)(*(char **)(d + 0x184) + 0x38) = *(long *)(*(char **)(g2 + 0x2080) + 0x38);
            }
            *(short *)(*(char **)(d + 0x184) + 0x34) = 0x4000;
            c = *(char **)(d + 0x184);
            qcopy(c + 0x10, m + 0x10);
            *(int *)(c + 0x94) = 0;
            func_L00_00251E30(*(char **)(d + 0x184));
        }
        if (*(char **)(d + 0x188) == 0) {
            *(char **)(d + 0x188) = func_0020D348(0x369);
            (*(char **)(d + 0x188))[0x30] = 0x40;
            *(short *)(*(char **)(d + 0x188) + 0x32) = 0x40;
            (*(char **)(d + 0x188))[0x31] = 1;
            {
                char *g2 = D_0013E633 + 0xE1D;
                *(long *)(*(char **)(d + 0x188) + 0x38) = *(long *)(*(char **)(g2 + 0x2080) + 0x38);
            }
            *(short *)(*(char **)(d + 0x188) + 0x34) = 0x4000;
            c = *(char **)(d + 0x188);
            qcopy(c + 0x10, m + 0x10);
            *(int *)(c + 0x94) = 0;
            func_L00_00251E30(*(char **)(d + 0x188));
        }
        func_0020D960((char *)m, 0, (unsigned char *)(d + 0xF0));
        func_0020D960((char *)m, 1, (unsigned char *)(d + 0x130));
        *(unsigned short *)(m + 0x34) |= 0x1000;
        g = D_0013E633 + 0xE1D;
        *(long *)(m + 0x38) = *(long *)(*(char **)(g + 0x2080) + 0x38);
        func_L01_0026E8E0(d + 0x70);
        *(float *)(d + 0x7C) = 2.0f;
        *(float *)(d + 0x78) = 4.0f;
        *(int *)(d + 0x70) = 0x38D;
        *(float *)(d + 0xAC) = 0.01f;
        *(float *)(d + 0xB0) = 0.3f;
        *(float *)(d + 0xB4) = 0.1f;
        *(char **)(d + 0xD0) = D_L05_001B0CB0_2D8388[*(int *)(d + 0xD4)];
        *(int *)(d + 0xC0) = 1;
        qcopy(m + 0x10, *(char **)(d + 0xD0) + 0x10);
        m[0x20] = 1;
        if (m[0x53] != 0) func_00213DE0(m, 0, 0, 0);
        *(float *)(d + 0x94) = *(float *)&D_L05_00161BB8 * D_0015EE6C;
        func_001F9BC0(d + 0x40);
        *(unsigned short *)(d + 0x3E) |= 2;
        d[0x2E] = 1;
        d[0x29] = 0;
        *(short *)(d + 0x24) = 6;
        d[0x28] = 3;
        *(float *)(d + 0x20) = 6.0f;
        *(int *)(d + 0x180) = func_001F9850(0x3C);
        *(int *)(d + 0x198) = 0;
        *(float *)(d + 0x194) = *(float *)(m + 0x48);
        d[0x58] = func_001FA898(D_L05_00161C38_c);
        d[0x5A] = func_001FA898(*(float *)&D_L05_00161C3C * 8.0f);
        break;
    }
    case 1: {
        char *pp = d + 0xC0;
        char *path = *(char **)(pp + 0x10);
        int n = *(int *)path;
        signed char dir = *(signed char *)(pp + 4);
        int last = 0;
        int idx;
        char *h;
        if (dir >= 0) last = n - 1;
        idx = (*(int *)(d + 0xC0) + n + dir) % n;
        func_L00_0025A060((char *)m, d + 0x70, (float *)(path + (idx * 16 + 0x10)), d + 0x40);
        if (func_001F9D48(m + 0x10, *(char **)(pp + 0x10) + (idx * 16 + 0x10)) < 1.0f) {
            if (idx == last) {
                char *q = *(char **)(pp + 0x10);
                if (5.0f < func_001F9D10(q + *(int *)q * 16, q + 0x10)) {
                    int t = -*(signed char *)(pp + 4);
                    *(signed char *)(pp + 4) = t;
                }
            }
            *(int *)pp = idx;
        }
        h = D_0013E633 + 0xE9D;
        if (func_001F9D48(m + 0x10, h) < *(float *)(d + 0x174)) {
            if (func_001F9B88_3047E8(*(float *)(m + 0x18) - *(float *)(h + 8)) < 4.0f) {
                if (*(int *)(d + 0x190) == -1
                    || func_L00_0025A778(h, D_L05_001B0CB0_2D8388[*(int *)(d + 0x190)] + 0x10,
                                         *(int *)D_L05_001B0CB0_2D8388[*(int *)(d + 0x190)]) != 0) {
                    qcopy(v10, m + 0x10);
                    v10[2] = v10[2] + 3.0f;
                    if (func_L00_001EFFF0_3047E8(v10, h + 0x50, 2, m, 0) == 0) {
                        if (m[0x53] != 0) func_00213DE0(m, 0, 0, 3);
                        m[0x20] = 2;
                    }
                }
            }
        }
        if (func_001F9D48(m + 0x10, D_0013E633 + 0xE9D) < 16.0f) {
            char *g = D_0013E633 + 0xE1D;
            float a = func_L00_001FF860(*(float *)(g + 0x80) - *(float *)(m + 0x10), *(float *)(g + 0x84) - *(float *)(m + 0x14));
            func_L00_0025CC58((float *)(d + 0x194), 0, a, D_0015EE6C * 1.5707964f);
            func_L00_0025CC58((float *)(d + 0x198), 0, 0.08726646f, D_0015EE6C * 1.5707964f);
        } else {
            func_L00_0025CC58((float *)(d + 0x194), 0, *(float *)(m + 0x48), D_0015EE6C * 1.5707964f);
            func_L00_0025CC58((float *)(d + 0x198), 0, 0.0f, D_0015EE6C * 1.5707964f);
        }
        func_L00_001FFED8(d + 0x140, 1, *(float *)(d + 0x198));
        func_L00_001FFED8(d + 0x100, 2, func_001FA790_3047E8(*(float *)(d + 0x194), *(float *)(m + 0x48)));
        break;
    }
    case 2: {
        char *pp = d + 0xC0;
        char *path = *(char **)(pp + 0x10);
        int n = *(int *)path;
        signed char dir = *(signed char *)(pp + 4);
        int last = 0;
        int idx;
        int i;
        int k;
        char *h;
        if (dir >= 0) last = n - 1;
        idx = (*(int *)(d + 0xC0) + n + dir) % n;
        func_L00_0025A060((char *)m, d + 0x70, (float *)(path + (idx * 16 + 0x10)), d + 0x40);
        if (func_001F9D48(m + 0x10, *(char **)(pp + 0x10) + (idx * 16 + 0x10)) < 1.0f) {
            if (idx == last) {
                char *q = *(char **)(pp + 0x10);
                if (5.0f < func_001F9D10(q + *(int *)q * 16, q + 0x10)) {
                    int t = -*(signed char *)(pp + 4);
                    *(signed char *)(pp + 4) = t;
                }
            }
            *(int *)pp = idx;
        }
        v20[3] = *(float *)&D_L05_00161BF0;
        v30[3] = *(float *)&D_L05_00161BF4;
        func_L00_00250800(m, 2, v10);
        for (i = 0; i < 2; i++) {
            int a, b, e;
            float ang;
            char *r;
            qcopy(v40, v10);
            v40[0] += random_float_between(-0.065f, 0.065f);
            v40[1] += random_float_between(-0.065f, 0.065f);
            v40[2] += random_float_between(-0.065f, 0.065f);
            v20[0] = FastCos(*(float *)(d + 0x194)) * (*(float *)&D_L05_00161BE0 * D_0015EE6C);
            v20[1] = FastSin(*(float *)(d + 0x194)) * (*(float *)&D_L05_00161BE0 * D_0015EE6C);
            v20[2] = 0.0f;
            v20[2] = random_float_between(*(float *)&D_L05_00161BE4 * D_0015EE6C * 0.9f, *(float *)&D_L05_00161BE4 * D_0015EE6C * 1.1f);
            ang = random_float_between(-*(float *)&D_L05_00161BF8, *(float *)&D_L05_00161BF8) * 0.017453292f;
            v30[0] = FastCos(FastAddRots(*(float *)(d + 0x194), ang)) * (*(float *)&D_L05_00161BE8 * D_0015EE6C);
            v30[1] = FastSin(FastAddRots(*(float *)(d + 0x194), ang)) * (*(float *)&D_L05_00161BE8 * D_0015EE6C);
            v30[2] = 0.0f;
            v30[2] = random_float_between(*(float *)&D_L05_00161BEC * D_0015EE6C * 0.9f, *(float *)&D_L05_00161BEC * D_0015EE6C * 1.1f);
            a = func_001FA898(func_001F9878(random_float_between(*(float *)&D_L05_00161BD4, *(float *)&D_L05_00161BD4 + *(float *)&D_L05_00161BD4)));
            b = func_001FA898(func_001F9878(random_float_between(*(float *)&D_L05_00161BD8, *(float *)&D_L05_00161BD8 * 1.2f)));
            e = func_001FA898(func_001F9878(random_float_between(*(float *)&D_L05_00161BDC, *(float *)&D_L05_00161BDC * 1.2f)));
            r = func_00219780(v40, v20, v30, *(int *)&D_L05_00161BCC, *(int *)&D_L05_00161BD0, a, b, e, -1);
            if ((D_L05_0015F6B0 & 7) == 0 && r != 0 && i == 0) {
                int j;
                for (j = 0; j < 8; j++) {
                    int o = j * 4;
                    char *s = d + 0x1A0;
                    if (*(char **)(s + o) == 0) {
                        *(char **)(s + o) = r;
                        break;
                    }
                }
            }
        }
        v20[3] = *(float *)&D_L05_00161C20;
        v30[3] = *(float *)&D_L05_00161C24;
        for (k = 1; k >= 0; k--) {
            int a, b, e;
            float ang;
            qcopy(v40, v10);
            v40[0] += random_float_between(-0.2f, 0.2f);
            v40[1] += random_float_between(-0.2f, 0.2f);
            v40[2] += random_float_between(-0.2f, 0.2f);
            ang = random_float_between(-*(float *)&D_L05_00161C2C, *(float *)&D_L05_00161C2C) * 0.017453292f;
            v20[0] = FastCos(FastAddRots(*(float *)(d + 0x194), ang)) * (*(float *)&D_L05_00161C10 * D_0015EE6C);
            v20[1] = FastSin(FastAddRots(*(float *)(d + 0x194), ang)) * (*(float *)&D_L05_00161C10 * D_0015EE6C);
            v20[2] = 0.0f;
            v20[2] = random_float_between(*(float *)&D_L05_00161C14 * D_0015EE6C * 0.7f, *(float *)&D_L05_00161C14 * D_0015EE6C * 1.5f);
            ang = random_float_between(-*(float *)&D_L05_00161C28, *(float *)&D_L05_00161C28) * 0.017453292f;
            v30[0] = FastCos(FastAddRots(*(float *)(d + 0x194), ang)) * (*(float *)&D_L05_00161C18 * D_0015EE6C);
            v30[1] = FastSin(FastAddRots(*(float *)(d + 0x194), ang)) * (*(float *)&D_L05_00161C18 * D_0015EE6C);
            v30[2] = 0.0f;
            v30[2] = random_float_between(*(float *)&D_L05_00161C1C * D_0015EE6C * 0.7f, *(float *)&D_L05_00161C1C * D_0015EE6C * 1.5f);
            a = func_001FA898(func_001F9878(random_float_between(*(float *)&D_L05_00161C04, *(float *)&D_L05_00161C04 + *(float *)&D_L05_00161C04)));
            b = func_001FA898(func_001F9878(random_float_between(*(float *)&D_L05_00161C08, *(float *)&D_L05_00161C08 * 1.2f)));
            e = func_001FA898(func_001F9878(random_float_between(*(float *)&D_L05_00161C0C, *(float *)&D_L05_00161C0C * 1.2f)));
            func_00219780(v40, v20, v30, *(int *)&D_L05_00161BFC, *(int *)&D_L05_00161C00, a, b, e, -1);
        }
        qcopy(v40, m + 0x10);
        v40[2] = v40[2] + 3.0f;
        h = D_0013E633 + 0xE9D;
        if (*(float *)(d + 0x174) < func_001F9D48(m + 0x10, h)
            || 4.0f <= func_001F9B88_3047E8(*(float *)(m + 0x18) - *(float *)(h + 8))
            || (*(int *)(d + 0x190) != -1
                && func_L00_0025A778(h, D_L05_001B0CB0_2D8388[*(int *)(d + 0x190)] + 0x10,
                                     *(int *)D_L05_001B0CB0_2D8388[*(int *)(d + 0x190)]) == 0)
            || func_L00_001EFFF0_3047E8(v40, D_0013E633_3047E8 + 0xEED, 2, m, 0) != 0) {
            if (m[0x53] != 0) func_00213DE0(m, 0, 0, 3);
            m[0x20] = 1;
        }
        {
            char *g = D_0013E633 + 0xE1D;
            float a = func_L00_001FF860(*(float *)(g + 0x80) - *(float *)(m + 0x10), *(float *)(g + 0x84) - *(float *)(m + 0x14));
            func_L00_0025CC58((float *)(d + 0x194), 0, a, D_0015EE6C * 1.5707964f);
            func_L00_0025CC58((float *)(d + 0x198), 0, 0.08726646f, D_0015EE6C * 1.5707964f);
        }
        func_L00_001FFED8(d + 0x140, 1, *(float *)(d + 0x198));
        func_L00_001FFED8(d + 0x100, 2, func_001FA790_3047E8(*(float *)(d + 0x194), *(float *)(m + 0x48)));
        break;
    }
    case 3:
        if (*(char **)(d + 0x184) != 0) func_0020D678(*(char **)(d + 0x184));
        if (*(char **)(d + 0x188) != 0) func_0020D678(*(char **)(d + 0x188));
        func_0020D678(m);
        break;
    }
}
