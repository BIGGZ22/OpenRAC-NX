/* NON_MATCHING func_L06_002F78A0 -- src/overlays/l06_blarg/vendor_002B5990.c
 * Best so far: SIZE ours 2336 / retail 2396, checked 2026-10-08.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * No longer builds in its file (COMPILE failed, 2026-10-09): match its declarations to the file's first.
 * What the last attempts found:
 *   Moby class 1035 update on level 06 (2396 B). Dispatch on the moby state byte: 0 init, 1 update (two vector pas
 *   State 2 passes the sp+0x50 vector to func_L00_001FF4B0, which the assembly never writes (stale stack); the can
 *   Stopped at run 11 of 16 by choice: the gap is structural across the whole function, not one instruction.
 */
typedef int u128 __attribute__((mode(TI)));

extern void func_L06_002F8200(void *);
extern int func_001F9938(void *);
extern int func_L00_0028EB98(void *, int);
extern int func_0022ED80_r(int, int, int) __asm__("func_0022ED80");
extern float func_001F9D10(void *, void *);
extern float func_00214D28(float *, float, float);
extern float func_001F9F90(float);
extern float func_001F9FA8(float);
extern float func_001FA748(float, float);
extern void func_001F9BD8(void *, void *, void *);
extern void func_001F9BF0(void *, void *, void *);
extern void func_L00_0025A8C0(void *, void *, int, float, void *);
extern int func_L00_001EFFF0(void *, void *, int, void *, void *);
extern int func_001F9850(int);
extern float func_001F9C78(void *, void *);
extern void func_L00_001FF4B0(void *, void *, float);
extern void func_L00_0025AAC0(void *, void *);
extern void func_001F49B0(void *, void *);
extern char *func_L00_0025B478(void *, int, int);
extern int func_L00_00203F20(int, int);
extern void func_L00_0028EBF0(int);
extern void func_001FA218(void *, void *);
extern void func_001FA1C0(float *, float);
extern void func_001FA540(void *, void *, void *);
extern void func_001F9EE8(void *, void *, void *);
extern float func_L00_00258C80(float, float);
extern float func_002140F8(float, float);
extern int func_001FA8A8(int, int, float);
extern int func_001FA898(float);
extern void func_L00_00258DB0(float *, float, float);
extern int func_002140B0(int);
extern unsigned char *func_L00_00273F80(float *, char *, int, unsigned char, unsigned char, int, float);
extern void func_0020D678(void *);

extern int D_L06_00161ED4;
extern int D_L06_00161ED8;
extern int D_L06_00161EDC;
extern int D_L06_00161EE0;
extern int D_L06_00161EE4;
extern short D_L06_00161E98;
extern short D_L06_00161E9C;
extern short D_L06_00161EA8;
extern short D_L06_00161EAC;
extern short D_L06_00161EB0;
extern short D_L06_00161EB4;
extern short D_L06_00161EB8;
extern short D_L06_00161EBC;
extern short D_L06_00161EC0;
extern short D_L06_00161EC4;
extern short D_L06_00161EC8;
extern char D_L06_00167640[];
extern char *D_L06_001746D8;
extern char *D_L06_00160058 MACRO_ADDR;
extern int D_0015EFA4 MACRO_ADDR;
extern int D_0015EE84 MACRO_ADDR;
extern float D_0015EE6C MACRO_ADDR;
extern char D_L06_001DB3B0[];
extern char D_L06_001DB320[];
extern char D_0013E633[];
extern char D_0014171B[];

/* Moby class 1035 update (level 06): 0 init, 1 update, 2 spawn an entry, 3 delete. */
void func_L06_002F78A0(char *m) {
    char *d = *(char **)(m + 0x78);
    unsigned char st = *(unsigned char *)(m + 0x20);
    char *pos = m + 0x10;
    char *P = D_0013E633 + 0xE1D;
    char *B = D_0014171B + 0x34D;
    float h = 0.5f;
    float hp = 1.5707963f;
    float th = 3.0f;
    float pi = 3.1415927f;
    float v0[4], v10[4], v20[4], v30[4], v40[4];
    float s0[4], s70[4], s80[4], s90[4], sA0[4], s50[4];
    float k;
    float v;
    int i;
    float q1, q2, qa, qb, qc, qd, dt;
    float t, x, y1, y2, kf;
    int c, w;
    unsigned char life, bb;

    if (st == 0) {
        D_L06_00161ED4 = 0xB;
        D_L06_00161ED8 = 0x17;
        D_L06_00161EDC = 0x22;
        D_L06_00161ED0 = 0;
        D_L06_00161EE4 = 0x16;
        D_L06_00161EE0 = 0x2D;
        *(int *)(d + 0x10) = -1;
        *(unsigned char *)(m + 0x20) = 1;
        *(unsigned short *)(m + 0x34) = *(unsigned short *)(m + 0x34) | 0x4000;
        return;
    }
    if (st == 1) {
        func_001F9938(d + 0x16);
        if (func_L00_0028EB98(m, *(int *)(d + 0x10)) == 0)
            *(int *)(d + 0x10) = func_0022ED80_r(0, 4, (int)m);
        func_L06_002F8430(m);
        v = func_001F9D10(pos, D_L06_00167640);
        if (v < 48.0f) {
            k = D_0015EE6C;
            func_00214D28((float *)(d + 0x18), 1.0f, k + k);
            v20[0] = func_001F9F90(*(float *)(m + 0x48)) * h;
            v20[1] = func_001F9FA8(*(float *)(m + 0x48)) * h;
            v20[2] = 0.0f;
            q1 = func_001FA748(*(float *)(m + 0x48), hp);
            v30[0] = func_001F9F90(q1) * th;
            q2 = func_001FA748(*(float *)(m + 0x48), hp);
            v30[1] = func_001F9FA8(q2) * th;
            v30[2] = 0.0f;
            *(u128 *)v0 = *(u128 *)pos;
            v0[2] = v0[2] - *(float *)&D_L06_00161EA8;
            *(u128 *)v10 = *(u128 *)v0;
            func_001F9BD8(v0, v0, v20);
            func_001F9BD8(v10, v10, v20);
            func_001F9BD8(v0, v0, v30);
            func_001F9BF0(v10, v10, v30);
            func_L00_0025A8C0(v40, m, 0x10001, 5.0f, v20);
            func_L00_001EFFF0(v0, v10, 0, m, v40);
            if (*(unsigned short *)(B + 0x158) == 0 && *(short *)(d + 0x16) == 0
                && D_L06_001746D8 == *(char **)(P + 0x2080)) {
                short t = *(short *)(d + 0x14) + 1;
                *(short *)(d + 0x14) = t;
                *(short *)(d + 0x16) = func_001F9850(30);
            }
            /* second pass, with pi and pi/2 */
            qa = func_001FA748(*(float *)(m + 0x48), pi);
            v20[0] = func_001F9F90(qa) * h;
            qb = func_001FA748(*(float *)(m + 0x48), pi);
            v20[1] = func_001F9FA8(qb) * h;
            v20[2] = 0.0f;
            qc = func_001FA748(*(float *)(m + 0x48), hp);
            v30[0] = func_001F9F90(qc) * th;
            qd = func_001FA748(*(float *)(m + 0x48), hp);
            v30[1] = func_001F9FA8(qd) * th;
            v30[2] = 0.0f;
            *(u128 *)v0 = *(u128 *)pos;
            v0[2] = v0[2] - *(float *)&D_L06_00161EA8;
            *(u128 *)v10 = *(u128 *)v0;
            func_001F9BD8(v0, v0, v20);
            func_001F9BD8(v10, v10, v20);
            func_001F9BD8(v0, v0, v30);
            func_001F9BF0(v10, v10, v30);
            func_L00_0025A8C0(v40, m, 0x10001, 5.0f, v20);
            func_L00_001EFFF0(v0, v10, 0, m, v40);
            if (*(unsigned short *)(B + 0x158) == 0 && *(short *)(d + 0x16) == 0
                && D_L06_001746D8 == *(char **)(P + 0x2080)) {
                short t = *(short *)(d + 0x14) + 1;
                *(short *)(d + 0x14) = t;
                *(short *)(d + 0x16) = func_001F9850(30);
            }
            if (*(int *)(P + 0x23C) == (int)m) {
                func_001F9BF0(s70, P + 0x80, pos);
                dt = func_001F9C78(s70, m + 0xC0);
                if (0.0f < dt)
                    func_L00_001FF4B0(v40, m + 0xC0, 1.0f);
                else
                    func_L00_001FF4B0(v40, m + 0xC0, -1.0f);
                func_L00_0025AAC0(*(void **)(P + 0x2080), v40);
                if (*(unsigned short *)(B + 0x158) == 0 && *(short *)(d + 0x16) == 0) {
                    short t = *(short *)(d + 0x14) + 1;
                    *(short *)(d + 0x14) = t;
                    *(short *)(d + 0x16) = func_001F9850(30);
                }
            }
        } else {
            func_00214D28((float *)(d + 0x18), 0.0f, (k = D_0015EE6C) + k);
        }
        if (*(float *)(d + 0x18) != 0.0f)
            func_001F49B0(func_L06_002F8200, m);
        /* L7E0C */
        if (*(unsigned short *)(B + 0x158) == 0) {
            if (*(short *)(d + 0x16) == 0) {
                char *r = func_L00_0025B478(m, 0x330000, 0);
                if (r != 0) {
                    short t = *(short *)(d + 0x14) + 1;
                    *(short *)(d + 0x14) = t;
                    *(short *)(d + 0x16) = func_001F9850(30);
                }
            }
            *(unsigned char *)(m + 0xA4) = 0xFF;
            if (*(short *)(d + 0x14) >= 3)
                func_L00_00203F20(0x1774, 0x2B);
        }
        {
            int ok5 = 1;
            int ok16 = 1;
            char *base7 = D_L06_00160058;
            int kk;
            for (kk = 0; kk < 4; kk++) {
                int idx = *(int *)(d + 4 * kk);
                if (idx >= 0) {
                    char *e = base7 + idx * 256;
                    if (*(short *)(e + 0xA6) != 0x516) {
                        if (e[0xBC] == 0)
                            ok5 = 0;
                    } else if (e[0x20] == 2) {
                        ok16 = 0;
                    } else {
                        ok5 = 0;
                    }
                }
            }
            if (!ok5)
                return;
            *(int *)(m + 0x94) = 0;
            *(unsigned char *)(m + 0x20) = 2;
            {
                int w = *(int *)(d + 0x10);
                if (w != -1) {
                    char *c = D_0013E633 + 0x1D + 0x70 * w;
                    if (*(char **)(c + 0x88) == m && (unsigned char)c[0x74] != 0)
                        func_L00_0028EBF0(w);
                }
                *(int *)(d + 0x10) = -1;
            }
            func_0022ED80_r(1, 0, (int)m);
            if (ok16) {
                unsigned short hh = *(unsigned short *)(B + 0x158);
                if (hh <= 0xFFFE)
                    *(short *)(B + 0x158) = hh + 1;
            }
            {
                int q = func_001F9850(D_0015EFA4) / 600;
                if ((int)*(unsigned short *)(B + 0x15A) < q) {
                    int q2 = func_001F9850(D_0015EFA4) / 600;
                    *(short *)(B + 0x15A) = q2;
                }
                *(int *)(B + 0x15C) = *(int *)(B + 0x15C) | (1 << D_0015EE84) | 0x80000000;
            }
        }
        return;
    }
    if (st == 2) {
        char *pos2 = m + 0x10;
        int kk;
        int ib;
        func_001FA218(s0, m + 0x40);
        func_001FA1C0(v40, *(float *)&D_L06_00161E9C);
        func_001FA540(s0, s0, v40);
        i = 0;
        do {
            ib = i;
            kk = 19;
            func_001FA218(v40, D_L06_001DB3B0 + ib * 16);
            i = ib + 1;
            func_001FA540(v40, s0, v40);
            func_001F9EE8(s80, D_L06_001DB320 + ib * 16, s0);
            func_001F9BD8(s70, s80, pos2);
            s70[3] = 1.0f;
            do {
                t = func_L00_00258C80(0.0f, *(float *)&D_L06_00161E98 * 0.5f);
                func_L00_001FF4B0(s90, s50, t);
                func_001F9BD8(s90, s90, s70);
                x = func_002140F8(0.0f, 1.0f);
                c = func_001FA8A8(*(int *)&D_L06_00161EAC, *(int *)&D_L06_00161EB0, x);
                y1 = func_002140F8(*(float *)&D_L06_00161EB4, *(float *)&D_L06_00161EB8);
                y2 = func_002140F8((float)*(int *)&D_L06_00161EBC, (float)*(int *)&D_L06_00161EC0);
                w = func_001FA898(y2);
                kf = D_0015EE6C;
                func_L00_00258DB0(sA0, *(float *)&D_L06_00161EC4 * kf, *(float *)&D_L06_00161EC8 * kf);
                life = (unsigned char)func_001F9850(w);
                bb = (unsigned char)func_002140B0(0xFF);
                func_L00_00273F80(s90, sA0, c, life, bb, 0, y1);
                kk = kk - 1;
            } while (kk >= 0);
        } while (i < 9);
        *(unsigned char *)(m + 0x20) = 3;
        return;
    }
    if (st == 3)
        func_0020D678(m);
    return;
}
