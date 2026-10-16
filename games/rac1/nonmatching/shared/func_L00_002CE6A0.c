/* NON_MATCHING func_L00_002CE6A0 -- src/overlays/shared/vendor_002C96D0.c
 * Best so far: SIZE ours 2796 / retail 2764, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Rocket moby update (class 457), 2764 bytes: steers the rocket toward its target, then runs lock-on, the 0x1C9/
 *   Left: the loop's 0.7 constant is hoisted into a saved $f23 (retail rebuilds it per iteration, 3 float saves no
 *   Unblock: a source form that keeps the two identical tails apart (the register each path loads its g+0x20 point
 */
typedef struct { float a; float b; char *p; int c; unsigned char b10; unsigned char b11; unsigned short s; float e; int i; } Ev;

extern char D_0013E633[];
extern float D_0015EE60 MACRO_ADDR;
extern float D_0015EE64 MACRO_ADDR;
extern float D_0015EE6C MACRO_ADDR;
extern float D_0015EE70 MACRO_ADDR;
extern int func_001F9850(int);
extern float func_001F9FA8(float);
extern float func_001FA748(float, float);
extern float func_001F9F90(float);
extern void func_001F9BF0(void *dst, void *a, void *b);
extern float func_L00_001FF3E0(void *);
extern float func_001F9B50(float);
extern void func_001F9C30(void *, void *, float);
extern void func_001F9BD8(void *, void *, void *);
extern float func_L00_001FF860(float, float);
extern float func_L00_00259148(float *vel, float cur, float target, float k, float d, float max);
extern float func_001F9CE8(void *);
extern float func_001F9CB8(void *a);
extern void func_00215C00(void *, float, float, float);
extern float func_002140F8(float, float);
extern void func_L00_001FF4B0(void *, void *, float);
extern int func_L00_00258BC8(int, int);
extern void func_L00_0026A7F8(void *, void *, int, int, int, int, int, int);
extern void func_001FA218(void *, void *);
extern void func_001F9EE8(void *, void *, void *);
extern int func_L00_001EFFF0(void *, void *, int, int, int);
extern float func_001F9D48(void *, void *);
extern void func_L00_001FF500(void *, void *, float);
extern void func_L00_001FF610(void *, void *, void *);
extern void func_L00_0025F4A8_alt(void *, void *, void *, float, float, int, int, int, float, float, float, float, int, float, int, int, int, int) __asm__("func_L00_0025F4A8");
extern int func_001F9908(int *);
extern void func_0020D678(void *);
extern int func_001160D8(void);

/* Rocket moby update: steers the rocket and its spring toward the target, then runs the lock-on and detonation checks. */
void func_L00_002CE6A0(char *m) {
    char *d;
    char *q;
    V4 z;
    V4 dir;
    V4 v20;
    V4 v50;
    V4 v60;
    V4 v70;
    V4 v80;
    V4 v90;
    V4 va0;
    Ev ev;
    int r;
    int r2;
    int flag;
    float t;
    float s;
    float x;
    float y;
    float w;
    float g;

    if (m == 0) return;
    d = *(char **)(m + 0x78);
    if (d == 0) return;

    *(u128 *)&z = 0;
    z.f[2] = D_0015EE6C * 8.0f;
    *(u128 *)&dir = *(u128 *)&z;
    {
        float a = *(float *)(m + 0x2C);
        float p = *(float *)(*(char **)(m + 0x24) + 0x24);
        *(float *)(m + 0x2C) = a + (p - a) * (D_0015EE60 * 0.05f);
    }
    {
        float e = *(float *)(d + 0x30);
        *(float *)(d + 0x30) = e + (D_0015EE6C * 20.0f - e) * (D_0015EE60 * 0.1f) * D_0015EE64;
    }
    r = func_001F9850(250);
    r -= func_001F9850(5);
    if (*(int *)(d + 0x24) < r) {
        q = *(char **)(d + 0x28);
        if (q == 0 || *(short *)(q + 0xA6) != *(int *)(d + 0x58) || *(unsigned char *)(q + 0x20) == 0xFE || *(unsigned char *)(q + 0x20) == 0xFD) {
            /* no lock: drift toward the aim point */
            *(float *)(d + 0x44) = *(float *)(d + 0x44) + D_0015EE60 * 0.1f;
            *(int *)(d + 0x28) = 0;
            r2 = func_001F9850(250);
            r2 -= func_001F9850(100);
            if (*(int *)(d + 0x24) < r2) {
                float a = func_001F9FA8(*(float *)(d + 0x48));
                *(float *)(m + 0x44) = func_001FA748(*(float *)(m + 0x44), a * (D_0015EE60 * 0.04f));
            }
            *(float *)(m + 0x48) = func_001FA748(*(float *)(m + 0x48), func_001F9F90(*(float *)(d + 0x48)) * (D_0015EE60 * 0.05f));
        } else {
            /* lock-on branch: steer toward the target position */
            *(u128 *)&v80 = *(u128 *)(q + 0x10);
            v80.f[2] = v80.f[2] + *(float *)(d + 0x2C);
            func_001F9BF0(&v70, &v80, d);
            func_001F9BF0(&v60, &v80, d + 0x10);
            *(u128 *)d = *(u128 *)&v80;
            {
                float r1 = func_L00_001FF3E0(&v70);
                float fA = *(float *)(d + 0x30) * *(float *)(d + 0x30) - r1;
                float fB = (v70.f[0] * v60.f[0] + v70.f[1] * v60.f[1] + v70.f[2] * v60.f[2]) * -2.0f;
                float r2f = func_L00_001FF3E0(&v60);
                float disc = fB * fB - (4.0f * fA) * (-r2f);
                float sq = func_001F9B50(disc);
                float t1;
                float t2;
                fB = -fB;
                fA = fA + fA;
                t1 = (fB + sq) / fA;
                t2 = (fB - sq) / fA;
                if (0.0f < t1 && 0.0f < t2) {
                    t = (t2 < t1) ? t1 : t2;
                } else if (0.0f < t1) {
                    t = t1;
                } else if (0.0f < t2) {
                    t = t2;
                } else {
                    t = -1.0f;
                }
            }
            if (0.0f < t) {
                func_001F9C30(&v90, &v70, t);
                func_001F9BD8(&v90, &v90, &v60);
                x = func_L00_001FF860(v90.f[0], v90.f[1]);
                *(float *)(m + 0x48) = func_L00_00259148((float *)(d + 0x34), *(float *)(m + 0x48), x, D_0015EE70 * 188.49556f, 0.0f, 0.0f);
                y = func_001F9CE8(&v90);
                w = func_L00_001FF860(y, v90.f[2]);
                *(float *)(m + 0x44) = func_L00_00259148((float *)(d + 0x38), *(float *)(m + 0x44), -w, D_0015EE70 * 188.49556f, 0.0f, 0.0f);
            }
            s = func_001F9CB8(&v60);
            if (s < 6.0f) {
                *(float *)(d + 0x44) = *(float *)(d + 0x44) * (s / 6.0f);
            } else {
                *(float *)(d + 0x44) = *(float *)(d + 0x44) + D_0015EE60 * 0.1f;
            }
        }
    }

    /* common tail */
    func_00215C00(&v50, *(float *)(d + 0x30), *(float *)(m + 0x48), -*(float *)(m + 0x44));
    *(u128 *)&v70 = 0;
    v70.f[0] = func_002140F8(-1.0f, 1.0f);
    v70.f[1] = func_002140F8(-1.0f, 1.0f);
    v70.f[2] = func_002140F8(-1.0f, 1.0f);
    *(u128 *)&v60 = *(u128 *)&v70;
    func_L00_001FF4B0(&v60, &v60, D_0015EE6C * func_002140F8(0.1f, 0.2f));
    g = func_002140F8(D_0015EE6C * 0.1f, D_0015EE6C);
    func_L00_001FF4B0(&z, &v50, -g);
    func_001F9BD8(&v60, &v60, &z);
    g = func_002140F8(0.0f, 1.0f);
    func_L00_001FF4B0(&z, &v50, *(float *)(d + 0x30) * g);
    func_001F9BD8(&z, &z, m + 0x10);
    {
        int c1 = func_L00_00258BC8(0x1C, 0x28);
        int r17 = func_001F9850(c1);
        int c2 = func_L00_00258BC8(0x28, 0x46);
        func_L00_0026A7F8(&z, &v60, 0x6F00AFFF, 0x100000FF, r17, 0x28, c2, 1);
    }
    *(u128 *)&v20 = *(u128 *)(m + 0x10);
    func_001F9BD8(d + 0x10, d + 0x10, &v50);
    *(float *)(d + 0x48) = func_001FA748(*(float *)(d + 0x48), *(float *)(d + 0x50));
    {
        float r2f = func_001FA748(*(float *)(d + 0x4C), *(float *)(d + 0x54));
        float r3;
        *(float *)(d + 0x4C) = r2f;
        r3 = func_001F9FA8(r2f);
        *(float *)(d + 0x48) = func_001FA748(*(float *)(d + 0x48), r3 * 0.15f);
    }
    if (0.9f < *(float *)(d + 0x44)) *(float *)(d + 0x44) = 0.9f;

    flag = 0;
    do {
        float a1 = func_001F9FA8(*(float *)(d + 0x48));
        float a2 = func_001F9F90(*(float *)(d + 0x4C));
        float vy = *(float *)(d + 0x44) * a1 * a2;
        float a4 = func_001F9F90(*(float *)(d + 0x48));
        float a5 = func_001F9F90(*(float *)(d + 0x4C));
        float vz = *(float *)(d + 0x44) * a4 * a5;
        v60.f[0] = 0.0f;
        v60.f[1] = vy;
        v60.f[2] = vz;
        v60.f[3] = 0.0f;
        func_001FA218(&va0, m + 0x40);
        func_001F9EE8(&v60, &v60, &va0);
        func_001F9BD8(m + 0x10, d + 0x10, &v60);
        r = func_L00_001EFFF0(&v20, m + 0x10, 6, (int)m, 0);
        if (r == 0) break;
        flag = 1;
        *(float *)(d + 0x44) = *(float *)(d + 0x44) * 0.7f;
    } while (0.1f < *(float *)(d + 0x44));

    if (flag) {
        x = func_L00_001FF860(*(float *)(m + 0x10) - v20.f[0], *(float *)(m + 0x14) - v20.f[1]);
        *(float *)(m + 0x48) = x;
        y = func_001F9D48(&v20, m + 0x10);
        w = func_L00_001FF860(y, *(float *)(m + 0x18) - v20.f[2]);
        *(float *)(m + 0x44) = -w;
        *(int *)(d + 0x34) = 0;
        *(int *)(d + 0x38) = 0;
    }

    if (*(float *)(m + 0x10) < 0.0f || *(float *)(m + 0x14) < 0.0f || *(float *)(m + 0x18) < 0.0f) {
        func_0020D678(m);
        return;
    }
    ev.a = 1.0f;
    ev.b = 5627.925f;
    ev.p = m;
    ev.c = 0x830000;
    ev.b10 = 3;
    ev.b11 = 3;
    ev.s = *(unsigned short *)(m + 0xA6);
    ev.e = 3.0f;
    ev.i = 1;
    *(u128 *)&v60 = *(u128 *)&v50;
    func_L00_001FF500(&v60, &v60, 1.0f);

    r2 = func_L00_001EFFF0(&v20, m + 0x10, 0, (int)m, (int)&v60);
    if (r2) {
        char *gg = D_L00_00173F40;
        char *p = *(char **)(gg + 0x18);
        if (p != 0 && !(p == *(char **)(D_0013E633 + 0x2E9D) || (int)p == *(int *)(d + 0x20) || *(short *)(p + 0xA6) == 0x1C9 || *(short *)(p + 0xA6) == 0)) {
            m[0xBC] = 2;
            *(u128 *)(m + 0x10) = *(u128 *)(gg + 0x20);
            func_L00_001FF610(&dir, &v50, gg + 0x40);
            func_L00_001FF4B0(&dir, &dir, D_0015EE6C + D_0015EE6C);
            func_L00_0025F4A8_alt(m, &dir, 0, 1.0f, 1.0f, 10, 3, 9, 4.0f, 2.0f, 9.0f, 1.0f, 0, 15.0f, 1, 1, -1, 0);
            func_0020D678(m);
            return;
        }
        if (p == 0 && *(int *)(gg + 0x1C) > 0) {
            m[0xBC] = 1;
            *(u128 *)(m + 0x10) = *(u128 *)(gg + 0x20);
            func_L00_001FF610(&dir, &v50, gg + 0x40);
            func_L00_001FF4B0(&dir, &dir, D_0015EE6C + D_0015EE6C);
            func_L00_0025F4A8_alt(m, &dir, 0, 1.0f, 1.0f, 10, 3, 9, 4.0f, 2.0f, 9.0f, 1.0f, 0, 15.0f, 1, 1, -1, 0);
            func_0020D678(m);
            return;
        }
        goto F090;
    }
    if (func_001F9908((int *)(d + 0x24))) {
        func_0020D678(m);
        return;
    }
    if (200.0f < func_001F9D48(m + 0x10, *(char **)(D_0013E633 + 0x2E9D) + 0x10)) {
        func_0020D678(m);
        return;
    }
F090:
    r = func_001F9850(0x20);
    if (*(int *)(d + 0x24) % r != 0) return;
    if (func_001160D8() & 1) {
        *(float *)(d + 0x50) = *(float *)(d + 0x50) * func_002140F8(-0.7f, -1.5f);
    }
    if (func_001160D8() & 1) {
        *(float *)(d + 0x54) = *(float *)(d + 0x54) * func_002140F8(-0.6f, -1.6f);
    }
}
