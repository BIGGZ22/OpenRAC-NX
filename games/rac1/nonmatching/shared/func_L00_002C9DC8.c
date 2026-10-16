/* NON_MATCHING func_L00_002C9DC8 -- src/overlays/shared/vendor_002C96D0.c
 * Best so far: SIZE ours 2248 / retail 2288, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   # func_L00_002C9DC8 (hq10 s08, bomb update, 2288 bytes)
 *   Outline: null/state checks, position bounds check (bad path calls func_0020D678), flag update at moby+0x34, th
 *   Run 1 (p0): compiles, 2224 bytes vs 2288. Run 2 (p1): base inlined instead of a local, worse. Run 3 (p2): flag
 *   Left: 40 bytes short. The prologue saves $fp and $s7 in retail (hi part of D_0013E633+0xE1D kept in $fp, G reb
 */
/* Bomb (class 230) update: steers the bomb's position toward its target and runs its state machine (idle, arm, flight, detonate). Written from the assembly. */
extern char D_0013E633[];
extern char D_L00_00173F60[];
extern char D_L00_00173F70[];
extern int D_L00_00173F40[];
extern void *D_L00_00173F58;
extern int D_L00_001600A4 MACRO_ADDR;
extern float D_0015EE70 MACRO_ADDR;
extern float D_0015EE60 MACRO_ADDR;
extern float func_001F9CB8(void *a);
extern int func_L00_0025D390(char *);
extern int func_001F9850(int);
extern int func_001F9908(int *);
extern void *func_L00_0026FBC8(void *);
extern float func_001FA888(int);
extern void func_001F9BD8(void *, void *, void *);
extern void func_L00_001FF240(void *, void *, void *);
extern int func_L00_001F10E0(float, void *, int, void *);
extern int func_L00_001EFFF0(void *, void *, int, int, int);
extern void func_L00_001FF610(void *, void *, void *);
extern void func_001F9C30(void *, void *, float);
extern void func_L00_002C93D8(void *);
extern float func_00214358(void *, int, float);
extern int func_L00_00261478(int, char *, char *, void *, char *, void *);
extern int func_001F9938(void *);
extern float func_001F9F90(float);
extern float func_001F9FA8(float);
extern char *func_L00_002C1110(int, void *, void *);
extern void func_0020D678(void *);

void func_L00_002C9DC8(char *m) {
    char *s;
    char *u;
    float f0, f1, f2, f3, f20, f21, f22, f23;
    Q4 v0, v1, v2, v3;
    int r;

    if (m == 0) return;
    s = *(char **)(m + 0x78);
    if (s == 0) return;
    if (*(float *)(m + 0x10) < 2.0f || 1023.0f < *(float *)(m + 0x10) ||
        *(float *)(m + 0x14) < 2.0f || 1023.0f < *(float *)(m + 0x14) ||
        *(float *)(m + 0x18) < 2.0f || 1023.0f < *(float *)(m + 0x18)) {
        func_0020D678(m);
        return;
    }
    {
        char *P = D_0013E633 + 0xE1D;
        if (*(unsigned char *)(P + 0x20A5) != 0 && *(unsigned char *)(m + 0x20) < 2) {
            *(unsigned short *)(m + 0x34) = *(unsigned short *)(m + 0x34) | 0x41;
        } else {
            *(unsigned short *)(m + 0x34) = *(unsigned short *)(m + 0x34) & 0xFFBE;
        }
    }
    {
        char *P = D_0013E633 + 0xE1D;
        if (*(short *)(*(char **)(s + 0x30) + 0xA6) == 0xE5 && *(short *)(s + 0x3A) == 0 &&
            func_001F9CB8(s) > 0.0f) {
            if (*(int *)(P + 0x2FC) == 0 || func_L00_0025D390(*(char **)(P + 0x2FC)) == 0) {
                func_L00_002C9820(s, m, 1);
            } else {
                if (func_L00_002C9820(s, m, 0) == 0) {
                    func_L00_002C9820(s, m, 2);
                }
            }
        }
    }
    switch (*(unsigned char *)(m + 0x20)) {
    case 0:
        m[0x20] = 1;
        *(int *)(s + 0x34) = func_001F9850(4);
        *(short *)(s + 0x38) = 0;
        *(int *)(s + 0x3C) = 0;
        break;
    case 1: {
        char *P = D_0013E633 + 0xE1D;
        *(short *)(s + 0x38) = 0;
        {
            char *p = *(char **)(m + 0x24);
            if (*(float *)(m + 0x2C) < *(float *)(p + 0x24) * 0.25f) {
                *(float *)(m + 0x2C) = *(float *)(m + 0x2C) + *(float *)(p + 0x24) * (D_0015EE60 * 0.05f);
            }
        }
        if (func_001F9908((int *)(s + 0x34)) != 0) {
            *(int *)(s + 0x34) = func_001F9850(4);
            if (P[0x20A5] == 0) {
                func_L00_0026FBC8(m);
            }
        }
        if (P[0x20A4] == 0) {
            char *p2 = *(char **)(s + 0x30);
            if (p2 != 0 && (((unsigned char *)p2)[0x20] & 0xF0) != 0xF0) {
                char *q = *(char **)(P + 0x1090);
                if (q != 0 && *(short *)(q + 0xA6) == 0xE5) {
                    return;
                }
            }
        }
        func_0020D678(m);
        break;
    }
    case 2: {
        char *P = D_0013E633 + 0xE1D;
        f22 = 0.5f;
        f21 = func_001FA888(((unsigned char *)D_0013E633)[1]) + 1.0f;
        {
            char *p = *(char **)(m + 0x24);
            if (*(float *)(m + 0x2C) < *(float *)(p + 0x24) * f22 * f21) {
                *(float *)(m + 0x2C) = *(float *)(m + 0x2C) +
                    *(float *)(p + 0x24) * (D_0015EE60 * 0.05f) * f21;
            }
        }
        if (func_001F9908((int *)(s + 0x34)) != 0) {
            *(int *)(s + 0x34) = func_001F9850(4);
            func_L00_0026FBC8(m);
        }
        u = m + 0x10;
        *(float *)(s + 0x8) = *(float *)(s + 0x8) - D_0015EE70 * 9.0f;
        func_001F9BD8(&v0, u, s);
        func_L00_001FF240(&v3, u, D_0013E633 + 0xF5D);
        qcopy(&v1, u);
        f23 = 0.15f;
        {
            char *p = *(char **)(m + 0x24);
            f1 = D_0015EE60 * f23 * *(float *)(m + 0x2C);
            f2 = *(float *)(p + 0x24);
            v1.v[2] = v1.v[2] - f1 / *(float *)(p + 0x24);
            qcopy(&v2, &v0);
            v2.v[2] = v2.v[2] - f1 / f2;
        }
        qcopy(&v3, &v0);
        f20 = f21 * 0.2f;
        v3.v[2] = v3.v[2] + f20;
        r = func_L00_001F10E0(f21 * 0.3f, &v3, 4, *(void **)(P + 0x2080));
        if (r != 0) {
            qcopy(u, D_L00_00173F70);
            *(float *)(m + 0x18) = *(float *)(m + 0x18) - f20;
            f21 = 0.01f;
            func_L00_001FF610(s, s, D_L00_00173F70 + 0x10);
        } else {
            r = func_L00_001EFFF0(&v1, &v2, 0, *(int *)(P + 0x2080), 0);
            if (r == 0) {
                qcopy(u, &v0);
                return;
            }
            qcopy(u, D_L00_00173F60);
            {
                char *p = *(char **)(m + 0x24);
                f0 = D_0015EE60 * f23 * *(float *)(m + 0x2C) / *(float *)(p + 0x24);
                *(float *)(m + 0x18) = *(float *)(m + 0x18) + f0;
            }
            f21 = 0.01f;
            func_L00_001FF610(s, s, D_L00_00173F60 + 0x20);
        }
        func_001F9C30(s, s, f22);
        if (func_001F9CB8(s) < D_0015EE60 * f21) {
            goto L344;
        }
        if (*(int *)(P + 0x2FC) != 0 && func_L00_0025D390(*(char **)(P + 0x2FC)) != 0) {
            float a20 = func_001F9CB8(s);
            if (a20 < D_0015EE60 * f21 + func_001F9CB8(D_0013E633 + 0xF1D)) {
                goto L344;
            }
        }
        goto L394;
    L344:
        {
            int n = *(unsigned short *)(s + 0x38) + 1;
            *(short *)(s + 0x38) = n;
            if (D_L00_00173F40[6] == 0 || D_L00_00173F40[7] > 0 ||
                func_001F9850(0xF0) < (short)n) {
                m[0x20] = 3;
                *(short *)(s + 0x38) = 0;
            }
        }
    L394:
        if (*(int *)(P + 0x2FC) != 0 && func_L00_0025D390(*(char **)(P + 0x2FC)) != 0) {
            func_001F9BD8(s, s, P + 0x100);
        }
        func_001F9BD8(u, u, s);
        func_L00_002C93D8(m);
        return;
    }
    case 3: {
        char *P = D_0013E633 + 0xE1D;
        u = m + 0x10;
        f0 = func_00214358(u, 0, 0.5f);
        if (f0 != 0.0f && D_L00_00173F58 != 0) {
            func_L00_00261478((int)m, D_L00_00173F58, u, m + 0x40, u, m + 0x40);
        }
        if (func_001F9938(s + 0x38) != 0) {
            char *p = (char *)D_L00_001600A4;
            int cnt = 0;
            while (p != 0) {
                if (*(short *)(p + 0xA6) == 0xBA) {
                    if ((((unsigned char *)p)[0x20] & 0xF0) != 0xF0) cnt++;
                }
                p = *(char **)(p + 0x28);
            }
            if (cnt >= 8) {
                *(short *)(s + 0x38) = func_001F9850(0xF);
            } else {
                f20 = 0.1f;
                f21 = 6.2831855f;
                f22 = 0.25f;
                f0 = func_001F9F90(((float)*(int *)(s + 0x3C)) * f21 * f22);
                f1 = D_0015EE60;
                v1.v[0] = f1 * f20 * f0;
                f0 = func_001F9FA8(((float)*(int *)(s + 0x3C)) * f21 * f22);
                v1.v[1] = (f1 * f20) * f0;
                v1.v[2] = f1 * 0.02f;
                if (*(int *)(P + 0x2FC) != 0 && func_L00_0025D390(*(char **)(P + 0x2FC)) != 0) {
                    func_001F9BD8(&v1, &v1, P + 0x100);
                }
                func_001F9BD8(&v0, &v1, u);
                func_L00_002C1110(*(int *)(s + 0x30), &v0, &v1);
                *(short *)(s + 0x38) = func_001F9850(8);
                *(int *)(s + 0x3C) = *(int *)(s + 0x3C) + 1;
                if (*(int *)(s + 0x3C) >= 4) {
                    m[0x20] = 4;
                }
            }
        }
        if (*(int *)(s + 0x3C) < 2) {
            if (func_001F9908((int *)(s + 0x34)) != 0) {
                *(int *)(s + 0x34) = func_001F9850(4);
                func_L00_0026FBC8(m);
            }
        }
        break;
    }
    case 4:
        f0 = *(float *)(m + 0x2C) * 0.92f;
        m[0x23] = m[0x23] - 4;
        *(float *)(m + 0x2C) = f0;
        f0 = func_00214358(m + 0x10, 0, 0.5f);
        if (f0 != 0.0f && D_L00_00173F58 != 0) {
            func_L00_00261478((int)m, D_L00_00173F58, m + 0x10, m + 0x40, m + 0x10, m + 0x40);
        }
        if ((unsigned char)m[0x23] == 0) {
            func_0020D678(m);
        }
        break;
    case 5:
    default:
        m[0x20] = 0;
        break;
    }
}
