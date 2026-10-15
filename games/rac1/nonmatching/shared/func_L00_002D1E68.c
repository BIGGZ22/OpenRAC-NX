/* NON_MATCHING func_L00_002D1E68 -- src/overlays/shared/vendor_002D1168.c
 * Best so far: SIZE ours 2656 / retail 2664, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Run 1-2: candidate with the full control flow (dat/0xA0/0xA4 chain, 0x0026C630 loops, the 12-way switch on m->
 *   Run 3-4: size 2656 then 2648. Branch layout of the first if-else fixed; the n/e0 ladder needed a fifth level (
 *   What differs: the stack frame. Retail is 0x1B0 with locals at 0x00-0xEF: vectors at 0x00/0x10/0x20/0x30, 0x60/
 *   Tried: pos as a pointer local (p5), v30 as a pointer local (p6), both with pt (p3): each drops the size to 265
 *   Unblock: a source form that leaves a 32-byte unreferenced stack gap and the 0xD0-0xEC scalar slots, and recomp
 */
extern int func_001F9B70(int);
extern int func_L00_0028EF68_v(int, int, void *, int) __asm__("func_L00_0028EF68");
extern void func_0022ED80(int, int, int);
extern int func_L00_0025F410(void *);
extern void func_001F9978(void);
extern unsigned char *func_L00_0026C630(void *pos, int spin, int col, float range, float f1, float f2, float f3, float scale);
extern void func_001F9BF0(void *, void *, void *);
extern float func_L00_001FF860(float, float);
extern float func_001FA790(float, float);
extern int func_001FA898_r(float) __asm__("func_001FA898");
extern f32 func_001FA7D8_26410(f32) __asm__("func_001FA7D8");
extern void func_001F9BC0(void *);
extern void func_001FA1F8(void *, void *);
extern void func_001F9EC0(void *, void *, void *);
extern void func_001F9BD8(void *, void *, void *);
extern float func_001FA748(float, float);
extern float func_002140F8(float, float);
extern void func_L00_001FF4B0(void *, void *, float);
extern int func_002140B0(int);
extern unsigned char *func_L00_002B0B98(unsigned char *owner, void *vel, void *pos, void *rot, int type, int n, float lo, float hi);
extern void func_L00_00251E30(void *);
extern void func_L00_00258DB0(float *, float, float);
extern float func_00214158(void);
extern int func_001F9850(int);
extern float func_001F9CB8(void *);
extern int func_L00_00258BC8(int, int);
extern void func_L00_0026B890(void *, void *, int, int, int, int, int, float, int, float);
extern int func_L00_002ADBB0(void *, void *, void *, float, int, int, int, int, int);
extern int D_L00_0015F6B0 MACRO_ADDR;
extern int D_L00_001619A4 MACRO_ADDR;
extern int D_L00_001619A8 MACRO_ADDR;
extern int D_L00_0016007C MACRO_ADDR;
extern char D_0013E633[];
extern char D_L00_00166D80[];
extern char D_L00_001DEA40[];
extern char D_L00_001DEAA0[];
extern float D_0015EE6C MACRO_ADDR;
extern char D_L00_00166EC0[];
extern char D_L00_001EA200[];
extern char D_L00_001EA218[];
extern float D_L00_0015F6B4 MACRO_ADDR;

typedef struct { char b[24]; } B24;

/* CrateBreakFx: crate break flash and debris for the moby m (a1 gates the spawn, a2 is an optional source vector). */
void func_L00_002D1E68(void *mv, int a1, int av) {
    unsigned char *m = (unsigned char *)mv;
    unsigned char *a2 = (unsigned char *)av;
    unsigned char *dat;
    unsigned char *owner;
    int dc = 0;
    int d8;
    int e0;
    int n;
    int k;
    int cnt;
    float v0[4];
    float v10[4];
    float v20[4];
    float v30[4];
    float v60[4];
    float v70[4];
    float v80[4];
    int t90[8];
    int tB0[8];
    float *pv30;
    float f20, f21, f22, f;
    int *q1, *q2;
    int c1, c2, a, b, dd, ee;

    pv30 = v30;
    dat = *(unsigned char **)(m + 0x78);
    if (func_001F9B70(D_L00_0015F6B0 - D_L00_001619A4) >= 2) {
        if (*(short *)(m + 0xA6) == 0x1FA) func_L00_0028EF68_v(0, 0, m, 0x1F4);
        else func_0022ED80(0, 0, (int)m);
        D_L00_001619A4 = D_L00_0015F6B0;
    }
    if (func_001F9B70(D_L00_0015F6B0 - D_L00_001619A8) >= 2) D_L00_001619A8 = D_L00_0015F6B0;
    else dc = 1;

    if (*(unsigned char **)(dat + 0xA0)) {
        unsigned char *ad = *(unsigned char **)(*(unsigned char **)(dat + 0xA0) + 0x78);
        *(unsigned char **)(ad + 0xA4) = *(unsigned char **)(dat + 0xA4);
        *(int *)(*(unsigned char **)(*(unsigned char **)(dat + 0xA0) + 0x78) + 0xAC) |= 4;
    }
    if (*(unsigned char **)(dat + 0xA4)) {
        if (func_L00_0025F410(*(unsigned char **)(*(unsigned char **)(dat + 0xA4) + 0x78) + 0xA0) == 0)
            func_001F9978();
        *(unsigned char **)(*(unsigned char **)(*(unsigned char **)(dat + 0xA4) + 0x78) + 0xA0) = *(unsigned char **)(dat + 0xA0);
        *(int *)(*(unsigned char **)(*(unsigned char **)(dat + 0xA4) + 0x78) + 0xAC) |= 4;
    }

    qcopy(v0, m + 0x10);
    v0[2] = v0[2] + 0.3f;
    func_L00_0026C630(v0, 1, 0x40808080, 0.3f, 1.01f, 1.075f, 0.05f, 5e4f);
    for (k = 4; k >= 0; k--)
        func_L00_0026C630(v0, 1, 0x40808080, 0.3f, 1.01f, 1.07f, 0.05f, 5e4f);

    if (a2 != 0) {
        qcopy(v10, a2 + 0x10);
    } else {
        func_001F9BF0(v10, m + 0x10, *(unsigned char **)(D_0013E633 + 0x2E9D) + 0x10);
    }

    switch ((short)(*(unsigned short *)(m + 0xA6) - 0x1F4)) {
    case 0: d8 = 0x165; break;
    case 1: d8 = 0x15C; break;
    case 2: d8 = 0x162; break;
    case 3: d8 = 0x15F; break;
    case 4: d8 = 0x168; break;
    case 5: d8 = 0x16B; break;
    case 6: d8 = 0x16E; break;
    case 7: d8 = 0x171; break;
    case 8: d8 = 0x171; break;
    case 9: d8 = 0x171; break;
    case 10: d8 = 0x171; break;
    case 11: d8 = 0x165; break;
    default: d8 = 0x95; break;
    }

    if (D_L00_0016007C >= 0xC9) { n = 4; e0 = 4; }
    else if (D_L00_0016007C >= 0x97) { n = 2; e0 = 2; }
    else if (D_L00_0016007C >= 0x65) { n = 2; e0 = 1; }
    else if (D_L00_0016007C >= 0x33) { n = 1; e0 = 0; }
    else { n = 1; e0 = 0; }

    f20 = 1.57f;
    f = func_L00_001FF860(*(float *)(D_L00_00166D80 + 0x140) - *(float *)(m + 0x10),
                          *(float *)(D_L00_00166D80 + 0x144) - *(float *)(m + 0x14));
    f = func_001FA790(f, *(float *)(m + 0x48));
    f21 = f + 3.1415925f;
    f21 = func_001FA7D8_26410((float)func_001FA898_r(f21 / f20) * f20);
    func_001F9BC0(v20);
    v20[2] = f21;
    func_001FA1F8(pv30, v20);

    if (n != 0) {
        unsigned char *p20 = D_L00_001DEAA0;
        unsigned char *p30 = D_L00_001DEA40;
        f20 = 1.0f;
        k = n;
        do {
            func_001F9EC0(v70, p30, pv30);
            func_001F9BD8(v70, v70, m + 0x10);
            qcopy(v80, p20);
            v80[2] = func_001FA748(v80[2], v20[2]);
            func_001F9BF0(v60, v70, m + 0x10);
            v60[2] = 0.0f;
            f = func_002140F8(f20, 5.0f);
            func_L00_001FF4B0(v60, v60, f * D_0015EE6C);
            v60[2] = func_002140F8(4.0f, 9.0f) * D_0015EE6C;
            cnt = func_002140B0(2);
            owner = func_L00_002B0B98(m, v60, v70, v80, d8, cnt, f20, f20);
            if (owner) {
                qcopy(owner + 0x40, p20);
                *(float *)(owner + 0x48) = func_001FA748(*(float *)(owner + 0x48), v20[2]);
                func_L00_00251E30(owner);
            }
            p20 += 0x10;
            p30 += 0x10;
        } while (--k);
    }

    if (e0 != 0) {
        f22 = -0.5f;
        f20 = 0.5f;
        f21 = 0.0f;
        k = e0;
        do {
            v70[0] = func_002140F8(f22, f20);
            v70[1] = func_002140F8(f22, f20);
            v70[2] = func_002140F8(f21, 1.0f);
            func_001F9BD8(v70, v70, m + 0x10);
            f = func_002140F8(6.0f, 12.0f) * D_0015EE6C;
            func_L00_00258DB0(v60, f, f);
            if (v60[2] < f21) v60[2] = -v60[2];
            f = func_00214158();
            cnt = 1;
            v80[0] = f;
            v80[2] = func_00214158();
            if (func_002140B0(6) == 0)
                cnt = func_001FA898_r(func_002140F8(3.0f, 6.0f));
            func_L00_002B0B98(m, v60, v70, v80, d8 + 1, cnt, 0.25f, f20);
        } while (--k);
    }

    *(int *)(m + 0x94) = 0;
    *(unsigned char *)(m + 0x20) = 3;
    *(unsigned short *)(m + 0x34) = *(unsigned short *)(m + 0x34) | 1;
    *(unsigned char *)(m + 0xBC) = func_001F9850(0xF);

    if (a1 != 0) {
        if (*(int *)(D_0013E633 + 0x2EA9) == 0x16)
            *(unsigned char *)(dat + 0xCA) = func_001F9850(5);
        else
            *(unsigned char *)(dat + 0xCA) = 1;

        func_L00_001FF4B0(v60, m + 0xE0, 0.5f);
        func_001F9BD8(v60, v60, m + 0x10);
        func_001F9BF0(v70, D_L00_00166EC0, m + 0x10);
        func_001F9BC0(v80);
        f21 = func_001F9CB8(v70);
        n = 10;
        if (f21 < 8.0f) n = func_001FA898_r(f21) + 2;
        f22 = 0.0f;
        if (f21 < 7.0f) f22 = 7.0f - f21;
        if (dc != 0) {
            if (n >= 7) n = 6;
        }

        if (n > 0) {
            k = n;
            do {
                f = func_002140F8(8.0f, 10.0f);
                *(B24 *)t90 = *(B24 *)D_L00_001EA200;
                f20 = f * D_0015EE6C;
                *(B24 *)tB0 = *(B24 *)D_L00_001EA218;
                f20 = f20 - f22 * D_0015EE6C;
                q1 = t90 + func_002140B0(6);
                q2 = tB0 + func_002140B0(6);
                a = func_001F9850(0xF);
                b = func_001F9850(0x14);
                c1 = func_L00_00258BC8(a, b);
                dd = func_001F9850(0x19);
                ee = func_001F9850(0x1E);
                c2 = func_L00_00258BC8(dd, ee);
                func_L00_0026B890(v60, v80, *q1, *q2, c1, c2, 0, 4e5f, 0, f20);
            } while (--k);
        }

        if (D_L00_0015F6B4 < 0.95f && 9.0f < f21 && dc == 0) {
            f20 = 4.0f;
            a = func_001F9850(0xF);
            func_L00_002ADBB0(m, v60, v80, f20, a, 0x7F, 0x7F, 0x7F, 0x20);
            a = func_001F9850(0x18);
            func_L00_002ADBB0(m, v60, v80, f20, a, 0x7F, 0x20, 0, 0x20);
            a = func_001F9850(0x14);
            func_L00_002ADBB0(m, v60, v80, f20, a, 0x7F, 0x40, 0, 0x30);
        }
        a = func_001F9850(0x1B);
        func_L00_002ADBB0(m, v60, v80, 3.5f, a, 0x60, 0x10, 0, 0x40);
        a = func_001F9850(0x1D);
        func_L00_002ADBB0(m, v60, v80, 3.0f, a, 0x20, 0, 0, 0x20);
    }
}
