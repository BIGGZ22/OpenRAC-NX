/* NON_MATCHING func_L18_002EBBF0 -- src/overlays/l18_veldin2/vendor_002A8400.c
 * Best so far: SIZE ours 1200 / retail 1236, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   func_L18_002EBBF0 (level 18, 1236 bytes, vendor_002A8400.c): builds a vector A (copied from m+0x10) and vector
 *   Stopped after 4 runs. The prototype in the file is `void func_L18_002EBBF0(void *)`, so the candidate takes vo
 */
extern short D_L18_00161FF8;
extern short D_L18_00161FFC;
extern short D_L18_00162000;
extern short D_L18_00162004;
extern short D_L18_00162008;
extern short D_L18_0016200C;
extern short D_L18_00162010;
extern short D_L18_00162014;
extern short D_L18_00162018;
extern short D_L18_0016201C;
extern short D_L18_00162020;
extern short D_L18_00162024;
extern short D_L18_00162028;
extern short D_L18_00162030;
extern short D_L18_00162034;
extern short D_L18_00162038;
extern short D_L18_0016203C;
extern short D_L18_00162040;
extern short D_L18_00162044;
extern short D_L18_00162048;
extern short D_L18_0016204C;
extern short D_L18_00162050;
extern short D_L18_00162054;
extern char D_L18_00162058;
extern short D_L18_0016205C;
extern short D_L18_00162060;
extern char D_L18_00162064;
extern short D_L18_00162068;
extern short D_L18_0016206C;
extern char D_L18_00162070;
extern short D_L18_00162074;
extern short D_L18_00162078;
extern unsigned char D_L18_00162059;
extern unsigned char D_L18_0016205A;
extern unsigned char D_L18_0016205B;
extern unsigned char D_L18_00162065;
extern unsigned char D_L18_00162066;
extern unsigned char D_L18_00162067;
extern unsigned char D_L18_00162071;
extern unsigned char D_L18_00162072;
extern unsigned char D_L18_00162073;
extern int D_L18_001D9F40[];
extern int D_L18_001D9F58[];
extern float D_0015EE6C MACRO_ADDR;
extern int D_L18_0015F660 MACRO_ADDR;
extern char D_L18_00167700[];
extern int func_001F9850(int);
extern void func_L06_0030D338(char *src, int a1, int a2, int a3);
extern int func_L00_0028EF68(int i, int a1, int v, int k);
extern float func_002140F8(float, float);
extern float func_00214158(void);
extern void func_00215C00(void *, float, float, float);
extern void func_001F9C30(void *, void *, float);
extern int func_001FA898_r(float) __asm__("func_001FA898");
extern char *func_00219780(void *, void *, void *, int, int, int, int, int, int);
extern float func_001F9F90(float);
extern float func_001F9FA8(float);
extern void func_001F9BD8(void *, void *, void *);
extern int func_002140B0(int);
extern int func_L00_00258BC8(int, int);
extern void func_L00_0026B890(void *, void *, int, int, float, int, int, int, int, float);
extern int func_L00_002ADBB0(void *, void *, void *, float, int, int, int, int, int);

/* Builds two vector sets from the object at m and the level tables, then runs the per-entry updates. */
void func_L18_002EBBF0(void *mv) {
    char *m = mv;
    float A[4];
    float B[4];
    int i;
    int k;
    int r;
    int xa, xb, xc;
    int s1, s2, t18, t17, q1, q2, r16, r2;
    float f20, f21, f22, f23, t, v, g, h, kk;

    qcopy(A, m + 0x10);
    r = func_001F9850(0x1E);
    func_L06_0030D338((char *)A, r, 0x80806060, 0x801010);
    func_L00_0028EF68(0x10, 0, (int)m, 0x58E);
    f23 = 0.8f;
    f22 = 1.2f;
    for (i = 0; i < *(int *)&D_L18_00161FFC; i++) {
        f21 = func_002140F8(-0.34906584f, 0.34906584f);
        f20 = func_00214158();
        t = func_002140F8(*(float *)&D_L18_00162000, *(float *)&D_L18_00162004);
        func_00215C00(A, t * D_0015EE6C, f20, f21);
        func_001F9C30(B, A, *(float *)&D_L18_00162024);
        t = func_002140F8(f23, f22);
        B[2] = B[2] + *(float *)&D_L18_00162020 * t * D_0015EE6C;
        B[3] = *(float *)&D_L18_0016201C;
        A[3] = *(float *)&D_L18_0016201C;
        f20 = func_002140F8(f23, f22);
        r = func_001F9850(*(int *)&D_L18_00162010);
        xa = func_001FA898_r((float)r * f20);
        r = func_001F9850(*(int *)&D_L18_00162014);
        xb = func_001FA898_r((float)r * f20);
        r = func_001F9850(*(int *)&D_L18_00162018);
        xc = func_001FA898_r((float)r * f20);
        func_00219780(m + 0x10, A, B, *(int *)&D_L18_00162008, *(int *)&D_L18_0016200C, xa, xb, xc, *(int *)&D_L18_00161FF8);
    }

    f23 = 0.0f;
    for (k = 0; k < *(int *)&D_L18_00162028; k++) {
        t = func_002140F8(*(float *)&D_L18_00162030, *(float *)&D_L18_00162034);
        f22 = t * D_0015EE6C;
        f20 = func_002140F8(1.04719758f, 1.39626336f);
        t = func_00214158();
        func_00215C00(A, f22, t, f20);
        f21 = func_00214158();
        g = func_001F9F90(f21);
        f20 = g;
        h = func_002140F8(f23, *(float *)&D_L18_0016204C);
        f20 = f20 * h;
        B[0] = f20;
        kk = func_001F9FA8(f21);
        f20 = kk;
        h = func_002140F8(f23, *(float *)&D_L18_0016204C);
        f20 = f20 * h;
        B[2] = f23;
        B[1] = f20;
        func_001F9BD8(B, B, m + 0x10);
        s1 = func_002140B0(6);
        t18 = D_L18_001D9F40[s1];
        s2 = func_002140B0(6);
        t17 = D_L18_001D9F58[s2];
        f20 = *(float *)&D_L18_00162038 * 400000.0f;
        q1 = func_L00_00258BC8(*(int *)&D_L18_0016203C, *(int *)&D_L18_00162040);
        r16 = func_001F9850(q1);
        q2 = func_L00_00258BC8(*(int *)&D_L18_00162044, *(int *)&D_L18_00162048);
        r2 = func_001F9850(q2);
        func_L00_0026B890(B, A, t18, t17, f20, r16, r2, 0, 0, f22);
    }

    f20 = 0.0f;
    v = *(float *)&D_L18_0016205C;
    if (f20 < *(float *)&D_L18_00162050) {
        r = func_001F9850(*(int *)&D_L18_00162054);
        func_L00_002ADBB0(m, m + 0x10, &D_L18_0015F660, *(float *)&D_L18_00162050, r, *(unsigned char *)&D_L18_00162058, D_L18_00162059, D_L18_0016205A, D_L18_0016205B);
        v = *(float *)&D_L18_0016205C;
    }
    if (f20 < v) {
        r = func_001F9850(*(int *)&D_L18_00162060);
        func_L00_002ADBB0(m, m + 0x10, &D_L18_0015F660, *(float *)&D_L18_0016205C, r, *(unsigned char *)&D_L18_00162064, D_L18_00162065, D_L18_00162066, D_L18_00162067);
        v = *(float *)&D_L18_00162068;
    }
    if (f20 < v) {
        r = func_001F9850(*(int *)&D_L18_0016206C);
        func_L00_002ADBB0(m, m + 0x10, &D_L18_0015F660, *(float *)&D_L18_00162068, r, *(unsigned char *)&D_L18_00162070, D_L18_00162071, D_L18_00162072, D_L18_00162073);
    }
    if (f20 < *(float *)&D_L18_00162074) {
        *(float *)(D_L18_00167700 + 0x160) = *(float *)&D_L18_00162074;
        *(int *)(D_L18_00167700 + 0x168) = func_001F9850(*(int *)&D_L18_00162078);
    }
}
