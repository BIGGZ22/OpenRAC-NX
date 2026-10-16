/* NON_MATCHING func_L07_0030C3E0 -- src/overlays/l07_umbris/vendor_002CE470.c
 * Best so far: SIZE ours 1344 / retail 1336, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   func_L07_0030C3E0 (level 07 moby update, class 871): stopped at 8 runs of 10, best candidate p4.c (p5.c is the
 *   Remaining differences: retail computes f = p+0x20 before the lui/jal of func_L00_0025B478 (ours fills that jal
 *   Unblock: the scheduler tie around the B478 call and block B's st40 addressing; a retail-shaped source for bloc
 *   Runs 9 and 10 (budget spent): p6 flips the block A nonzero branch (no gain); p7 reads the count once for the b
 */
extern char *func_L00_0025B478(void *, int, int);
extern int func_L00_0025B4D0(void *, void *, void *, int, int *, float *, int, int);
extern void func_L01_0026F090(int list, int state);
extern void func_001F9BC0(float *);
extern void func_L00_0025F4A8_alt(void *, void *, void *, float, float, int, int, int, float, float, float, float, int, float, int, int, int, int) __asm__("func_L00_0025F4A8");
extern void func_0020D678(void *);
extern int func_001F9850(int);
extern void func_L00_0025E4B0(void *, short *);
extern void func_L00_0025E590(void *, void *);
extern int func_L00_0025A208(int *, int, int, int);
extern int func_001E9730();
extern int func_L01_0028C2D8(void *, void *, float);
extern int func_L00_0025A2F0(int *, int, int, int);
extern int func_L00_0025E860(void *, void *, int *, float *, float, int);
extern void func_L00_0025B040(unsigned char *, float);
extern void func_001F9BF0(void *, void *, void *);
extern int func_001F9908(int *arg0);
extern float func_001F9D10(void *, void *);
extern char *D_L07_001B0830[];
extern float D_0015EE6C MACRO_ADDR;
extern char D_L07_00211840[];
extern char D_L07_00211870[];
extern char D_L07_002118C0[];

// Level 07 update for moby class 871: samples points along its path and steers it.
void func_L07_0030C3E0(char *m) {
    char *p;
    char *t;
    char *r;
    float *f;
    float vec[4];
    int st30;
    int st34;
    int st38;
    int st3C;
    int st40;
    float st44;
    int n;
    int i;
    int k;
    int cnt;
    int state;
    char *e;
    float fv;
    int v;

    if (m == 0) return;
    p = *(char **)(m + 0x78);
    if (p == 0) return;
    if (*(unsigned char *)(p + 0x94) != 0) {
        f = (float *)(p + 0x20);
        r = func_L00_0025B478(m, 0x230000, 0);
        n = func_L00_0025B4D0(m, r, f, 0, &st30, 0, 0, 4);
        switch (n) {
        case 0:
            break;
        case 1:
        case 2:
            *f = 0;
            break;
        case 3:
            break;
        case 4:
            break;
        case 5:
            break;
        case 6:
            break;
        case 7:
            break;
        case 8:
            break;
        case 9:
            break;
        case 10:
            break;
        case 11:
            break;
        }
        if (st30 >= 2) {
            if (*f <= *(float *)(r + 0x2C)) {
                *f = 0;
                func_L01_0026F090(*(int *)(p + 0x74), 3);
                func_001F9BC0(vec);
                func_L00_0025F4A8_alt(m, vec, (void *)0, 0.0f, 0.0f, 10, 3, 16, 4.0f, 2.0f, 9.0f, 1.0f, -1, 15.0f, 1, 1, -1, 0);
                func_0020D678(m);
                return;
            }
            *f = *f - *(float *)(r + 0x2C);
            *(unsigned char *)(p + 0x67) = 0xFA;
            *(short *)(p + 0x26) = func_001F9850(0x3C);
            func_L00_0025E4B0(m, (short *)(p + 0x60));
        }
        *(unsigned char *)(m + 0xA4) = 0xFF;
    }

    func_L00_0025E590(m, p + 0x60);
    n = *(int *)(p + 0x70);
    if (n == -1) return;
    t = D_L07_001B0830[n];
    state = *(unsigned char *)(m + 0x20);

    switch (state) {
    case 0: {
        st34 = 0;
        st3C = 0;
        qcopy(vec, t + 0x10);
        cnt = *(int *)t;
        i = 0;
        if (cnt > 0) {
            do {
                *(float *)(t + i * 16 + 0x1C) = func_001F9D10(t + i * 16 + 0x10, t + ((i + 1) % cnt) * 16 + 0x10);
                i = i + 1;
            } while (i < cnt);
        }
        if (func_L00_0025A208(&st34, *(int *)(p + 0x74), 1, 0)) {
            func_001E9730(D_L07_00211840, *(int *)(p + 0x74));
            return;
        }
        n = func_L01_0028C2D8(m + 0x10, t, 0.0f);
        *(int *)(p + 0x90) = n;
        st38 = n;
        k = 0;
        if (*(int *)(p + 0x78) > 0) {
            do {
                e = *(char **)(st34 + 0x78);
                *(int *)(e + 0xA0) = (int)m;
                qcopy((char *)st34 + 0x10, vec);
                qcopy(e + 0x80, vec);
                qcopy(e + 0x70, vec);
                *(int *)(e + 0x90) = st38;
                *(int *)(e + 0x94) = st38 + 1;
                if (func_L00_0025A2F0(&st34, st34, 1, 0) != 0) {
                    cnt = *(int *)(p + 0x78);
                    if (k < cnt - 1) {
                        func_001E9730(D_L07_00211870, k + 1, *(int *)(p + 0x74));
                    } else if (k == cnt - 1) {
                        func_001E9730(D_L07_002118C0, *(int *)(p + 0x74), cnt);
                    }
                    *(int *)(p + 0x78) = k + 1;
                    break;
                } else {
                    func_L00_0025E860(t, vec, &st38, (float *)&st3C, *(float *)(p + 0x80), 1);
                }
                k = k + 1;
            } while (k < *(int *)(p + 0x78));
        }
        if (*(unsigned char *)(p + 0x94) == 0) {
            *(int *)(m + 0x94) = 0;
            *(unsigned short *)(m + 0x34) = *(unsigned short *)(m + 0x34) & 0xEFFF;
        }
        v = *(unsigned char *)(p + 0x95);
        if (v == 0) {
            *(unsigned short *)(m + 0x34) = *(unsigned short *)(m + 0x34) | 1;
            v = 1;
        }
        *(unsigned char *)(m + 0x20) = v;
        break;
    }
    case 1: {
        st40 = 0;
        func_L00_0025A208(&st40, *(int *)(p + 0x74), 1, 0);
        if (*(int *)(p + 0x78) > 0) {
            k = 0;
            do {
                e = *(char **)(st40 + 0x78);
                fv = func_001F9D10(e + 0x80, t + *(int *)(e + 0x90) * 16 + 0x10);
                st44 = fv;
                func_L00_0025E860(t, e + 0x80, (int *)(e + 0x90), &st44, *(float *)(p + 0x7C) * D_0015EE6C, 1);
                *(int *)(e + 0x94) = *(int *)(e + 0x90) + 1;
                if (st40 == 0 || *(unsigned char *)(st40 + 0x20) == 0xFE || *(unsigned char *)(st40 + 0x20) == 0xFD) {
                    if (*(int *)(p + 0x84) != -1) {
                        func_001F9BF0(e + 0x60, e + 0x80, (char *)st40 + 0x10);
                        qcopy((char *)st40 + 0x10, e + 0x80);
                        if (func_001F9908((int *)(e + 0x98))) {
                            if (*(int *)(e + 0x90) == *(int *)(p + 0x90)) {
                                func_L07_0030C320((void *)st40);
                            }
                        }
                    }
                }
                func_L00_0025A2F0(&st40, st40, 1, 0);
                k = k + 1;
            } while (k < *(int *)(p + 0x78));
        }
        break;
    }
    }

    func_L00_0025B040(m, 1.5f);
}
