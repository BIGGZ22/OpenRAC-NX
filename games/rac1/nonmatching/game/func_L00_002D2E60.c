/* Draw callback of Ratchet's ship on Veldin (class 530, registered by func_L00_002D3330): the
   canopy glass's shine. Pvars M (+0x00, the cockpit joint's matrix) and the cross-fade timer
   (+0x40). While the camera is within 16 of the ship in x and y, or in the moby's first state
   (+0x20 == 1), +0xBC = 1 and the 102 glass points (D_L00_001DEB00) and normals (D_L00_001DFCF0)
   go through M; each point's sphere-map ST (reflection of the view vector about the normal
   scaled to 0.1) goes to D_L00_001E0E50, faded from the frozen ST D_L00_001E1180 while the
   timer runs (ticks(60) after coming back). Farther away only the points are transformed: the
   first such draw freezes the current ST (+0xBC = 0), and the timer restarts. Then the 74
   quads (D_L00_001DF160, 0x18 bytes each, a point index every 6 bytes) are drawn through
   func_L00_001FD1D8 with effect texture D_L00_001619C0, colour D_L00_001619C4 and the alpha
   words D_L00_001619AC..BC.
   Adapted from ReRAC (crates/rc-engine/src/fx_draw.rs ship_glass_prims, ship_glass_st;
   crates/rc-game/src/moby_update/classes/units/veldin_ship.rs; ISC License, Copyright (c)
   2026 ReRAC contributors). */
typedef struct {
    float f[4];
} __attribute__((aligned(16))) SG_V;
typedef struct {
    SG_V v[4];
    int c[4];
    float uv[8];
    long g[4];
} SG_Quad;
extern int D_L00_001619AC_sg[] __asm__("D_L00_001619AC");
extern int D_L00_001619B0_sg[] __asm__("D_L00_001619B0");
extern int D_L00_001619B4_sg[] __asm__("D_L00_001619B4");
extern int D_L00_001619B8_sg[] __asm__("D_L00_001619B8");
extern int D_L00_001619BC_sg[] __asm__("D_L00_001619BC");
extern int D_L00_001619C0_sg[] __asm__("D_L00_001619C0");
extern int D_L00_001619C4_sg[] __asm__("D_L00_001619C4");
extern float D_L00_00166D80_sg[] __asm__("D_L00_00166D80");
extern float D_L00_00166EC0_sg[] __asm__("D_L00_00166EC0");
extern SG_V D_L00_001DEB00_sg[] __asm__("D_L00_001DEB00");
extern SG_V D_L00_001DFCF0_sg[] __asm__("D_L00_001DFCF0");
extern SG_V D_L00_001E07F0_sg[] __asm__("D_L00_001E07F0");
extern float D_L00_001E0E50_sg[] __asm__("D_L00_001E0E50");
extern float D_L00_001E1180_sg[] __asm__("D_L00_001E1180");
extern char D_L00_001DF160_sg[] __asm__("D_L00_001DF160");
extern int func_001F4868_sg(int) __asm__("func_001F4868");
extern float func_001F9B88_sg(float) __asm__("func_001F9B88");
extern int func_001F9908_sg(int *) __asm__("func_001F9908");
extern float func_001FA888_sg(int) __asm__("func_001FA888");
extern int func_001F9850_sg(int) __asm__("func_001F9850");
extern void func_001F9EE8_sg(void *, void *, void *) __asm__("func_001F9EE8");
extern void func_001F9BF0_sg(void *, void *, void *) __asm__("func_001F9BF0");
extern void func_L00_001FF4B0_sg(void *, void *, float) __asm__("func_L00_001FF4B0");
extern float func_001F9C78_sg(void *, void *) __asm__("func_001F9C78");
extern void func_001F9C30_sg(void *, void *, float) __asm__("func_001F9C30");
extern float func_001F9B50_sg(float) __asm__("func_001F9B50");
extern void func_L00_001FD1D8_sg(void *, void *, int) __asm__("func_L00_001FD1D8");

void func_L00_002D2E60(char *m) {
    SG_Quad q;
    SG_V n;
    SG_V r;
    SG_V e;
    int *pv = *(int **)(m + 0x78);
    int near = 0;
    int tex;
    int k;
    int c;
    int idx;
    float fade;
    float mm;
    float s;
    float t;
    float *cur;
    float *prev;

    tex = func_001F4868_sg(D_L00_001619C0_sg[0]);
    q.g[1] = tex;
    q.g[2] = 0xFF9000000260L;
    q.g[3] = (long)D_L00_001619AC_sg[0] | ((long)D_L00_001619B0_sg[0] << 2) |
             ((long)D_L00_001619B4_sg[0] << 4) | ((long)D_L00_001619B8_sg[0] << 6) |
             ((long)D_L00_001619BC_sg[0] << 32);
    q.c[0] = D_L00_001619C4_sg[0];
    q.g[0] = 0;
    q.c[3] = D_L00_001619C4_sg[0];
    q.c[2] = D_L00_001619C4_sg[0];
    q.c[1] = D_L00_001619C4_sg[0];

    if (func_001F9B88_sg(D_L00_00166D80_sg[0x140 / 4] - *(float *)(m + 0x10)) < 16.0f &&
        func_001F9B88_sg(D_L00_00166D80_sg[0x144 / 4] - *(float *)(m + 0x14)) < 16.0f) {
        near = 1;
    }

    if (near != 0 || *(unsigned char *)(m + 0x20) == 1) {
        *(unsigned char *)(m + 0xBC) = 1;
        func_001F9908_sg(pv + 0x10);
        fade = func_001FA888_sg(pv[0x10]);
        fade = fade / func_001FA888_sg(func_001F9850_sg(0x3C));
        cur = D_L00_001E0E50_sg;
        prev = D_L00_001E1180_sg;
        for (k = 0; k < 0x66; k++) {
            func_001F9EE8_sg(&D_L00_001E07F0_sg[k], &D_L00_001DEB00_sg[k], pv);
            func_001F9BF0_sg(&e, &D_L00_001E07F0_sg[k], D_L00_00166EC0_sg);
            func_L00_001FF4B0_sg(&e, &e, 1.0f);
            func_001F9EE8_sg(&n, &D_L00_001DFCF0_sg[k], pv);
            func_L00_001FF4B0_sg(&n, &n, 0.1f);
            s = func_001F9C78_sg(&n, &e);
            func_001F9C30_sg(&r, &n, s + s);
            func_001F9BF0_sg(&r, &r, &e);
            func_L00_001FF4B0_sg(&r, &r, 1.0f);
            r.f[2] = r.f[2] + 1.0f;
            mm = func_001F9B50_sg(r.f[2] + r.f[2]);
            mm = mm + mm;
            if (*(unsigned char *)(m + 0x20) == 1 || pv[0x10] == 0) {
                cur[0] = r.f[0] / mm + 0.5f;
                cur[1] = r.f[1] / mm + 0.5f;
            } else {
                s = r.f[0] / mm + 0.5f;
                cur[0] = s + (prev[0] - s) * fade;
                t = r.f[1] / mm + 0.5f;
                cur[1] = t + (prev[1] - t) * fade;
            }
            prev += 2;
            cur += 2;
        }
        if (*(unsigned char *)(m + 0x20) == 1) {
            *(unsigned char *)(m + 0x20) = 2;
        }
    } else {
        if (*(unsigned char *)(m + 0xBC) == 1) {
            *(unsigned char *)(m + 0xBC) = 0;
            for (k = 0; k < 0x66; k++) {
                D_L00_001E1180_sg[k * 2 + 0] = D_L00_001E0E50_sg[k * 2 + 0];
                D_L00_001E1180_sg[k * 2 + 1] = D_L00_001E0E50_sg[k * 2 + 1];
                func_001F9EE8_sg(&D_L00_001E07F0_sg[k], &D_L00_001DEB00_sg[k], pv);
            }
        } else {
            for (k = 0; k < 0x66; k++) {
                func_001F9EE8_sg(&D_L00_001E07F0_sg[k], &D_L00_001DEB00_sg[k], pv);
            }
        }
        pv[0x10] = func_001F9850_sg(0x3C);
    }

    for (k = 0; k < 0x4A; k++) {
        for (c = 0; c < 4; c++) {
            idx = *(short *)(D_L00_001DF160_sg + k * 0x18 + c * 6);
            q.v[c] = D_L00_001E07F0_sg[idx];
            q.uv[c * 2 + 0] = D_L00_001E0E50_sg[idx * 2 + 0];
            q.uv[c * 2 + 1] = D_L00_001E0E50_sg[idx * 2 + 1];
        }
        func_L00_001FD1D8_sg(&q, 0, 0);
    }
}
