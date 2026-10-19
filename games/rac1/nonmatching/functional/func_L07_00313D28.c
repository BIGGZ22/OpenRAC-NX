/* functional: high. fcheck: unchecked (level/overlay function; fcheck runs boot-executable functions only; OpenRAC tools/fcheck_gate.py, 2026-10-18). Corrected from nonmatching/l07_umbris/func_L07_00313D28.c: outer step was pi/2 but retail adds 0x3EC90FDB (pi/8); the func_001FA748 second argument was 0.7853982 but retail passes 0x3EC90FDB (pi/8); start angle was -2.7443 but retail is 0xC02FEDE0 (-2.74889); loop bound/pi/2 constants now exact bits (pi 0x40490FDB); locals are now one frame struct at retail offsets because the callees read the stack buffers. */
#include "common.h"
extern void func_00234C98(int, long);
extern int func_001F4868(int);
extern float func_001FA888(int);
extern int func_001FA898(float);
extern void func_L00_00250800(void *, int, void *);
extern float func_L00_002001D8(void *, float);
extern float func_L00_0025F368(float);
extern void func_00215C00(void *, float, float, float);
extern float func_001FA748(float, float);
extern void func_001F9BD8(void *, void *, void *);
extern void func_L00_001FD1D8(void *, void *, int);
extern int D_0015EF88_m __asm__("D_0015EF88") MACRO_ADDR;
extern int D_L07_0015F6B0 MACRO_ADDR;

static inline float f_bits(unsigned u) { union { unsigned u; float f; } x; x.u = u; return x.f; }

typedef struct {
    float v0[4], v10[4], v20[4], v30[4];
    int ints[4];
    float s50[8];
    long q[4];
    float v90[4], vA0[4], vB0[4];
} L07Frame __attribute__((aligned(16)));

// Umbris effect: builds GS packets for a flare strip from the moby's data and draws its segments
void func_L07_00313D28(char *moby, int arg2, float fa, float fb, float fc, float fd, float fe) {
    char *data = *(char **)(moby + 0x78);
    int c = *(unsigned char *)(data + 0x1C6);
    int col = (c << 24) | (c << 16) | (c << 8) | c;
    long w1 = (long)(D_0015EF88_m >> 13) | 0x1000000L | (0x8000L << 17);
    int c17, n, a4;
    L07Frame L;
    float r1, r2, f0, f1, f12, f13, f20, f21, f22, f23, f24, f25, f27, f28, f29, f30, f31;
    const float PI_8 = f_bits(0x3EC90FDB);
    const float PI_2 = f_bits(0x3FC90FDB);
    const float PI = f_bits(0x40490FDB);

    func_00234C98(0x4E, w1);
    L.q[1] = func_001F4868(0x3C);
    L.q[2] = 0xFF9000000260L;
    L.q[3] = 0xA000000029L;
    n = func_001FA898(func_001FA888(0) * fd);
    c17 = col | 0x404040;
    a4 = D_L07_0015F6B0 + arg2;
    n = (n << 24) | c17;
    L.ints[0] = n;
    L.ints[3] = n;
    L.ints[2] = n;
    L.ints[1] = n;
    L.q[0] = 0;
    f30 = func_001FA888(a4) * fa;
    func_L00_00250800(moby, 9, L.v90);
    qcopy(L.vA0, L.v90);
    f1 = fb - 0.8f;
    f0 = L.vA0[2] - f1;
    L.vA0[2] = f0;
    *(int *)&L.vB0[3] = c17;
    f12 = f30 * -0.01f;
    r1 = func_L00_002001D8(L.vB0, f12);
    f12 = f30 * -0.02f;
    f1 = r1 + 1.0f;
    L.s50[0] = r1;
    L.s50[4] = r1;
    L.s50[6] = f1;
    L.s50[2] = f1;
    r2 = func_L00_002001D8(L.vB0, f12);
    f20 = r2 + 1.0f;
    L.s50[1] = r2;
    L.s50[3] = r2;
    L.s50[5] = f20;
    L.s50[7] = f20;
    f21 = fe - fd;
    f31 = f21 * 0.0625f;
    f28 = fc * 0.0625f;
    f27 = f_bits(0xC02FEDE0);

    do {
        qcopy(L.v20, L.vA0);
        qcopy(L.v30, L.vA0);
        f29 = f27 + PI_8;
        f22 = fd;
        n = func_001FA898(func_001FA888(0) * f22);
        f1 = fc - PI_2;
        n = (n << 24) | c17;
        L.ints[2] = n;
        L.ints[3] = n;
        f21 = f28 + -PI_2;
        if (f21 <= f1) {
            f24 = 0.6f;
            f23 = 0.375f;
            f25 = f1;
            do {
                qcopy(L.v0, L.v20);
                qcopy(L.v10, L.v30);
                f22 = f22 + f31;
                L.ints[0] = L.ints[2];
                L.ints[1] = L.ints[3];
                n = func_001FA898(func_001FA888(0) * f22);
                f12 = 0.02f;
                n = (n << 24) | c17;
                f12 = f30 * f12;
                L.ints[2] = n;
                L.ints[3] = n;
                f20 = func_L00_0025F368(f27 + f12);
                func_00215C00(L.v20, fb, f20, f21);
                f13 = func_001FA748(f20, PI_8);
                func_00215C00(L.v30, fb, f13, f21);
                f0 = L.v20[2];
                f1 = L.v30[2];
                if (0.0f < f21) {
                    f0 = f0 + f24;
                    f1 = f1 + f24;
                } else {
                    f0 = f0 - f23;
                    f1 = f1 - f23;
                }
                L.v20[2] = f0;
                L.v30[2] = f1;
                func_001F9BD8(L.v20, L.v20, L.v90);
                f21 = f21 + f28;
                func_001F9BD8(L.v30, L.v30, L.v90);
                func_L00_001FD1D8(L.v0, 0, 1);
            } while (f21 <= f25);
        }
        f27 = f29;
    } while (f27 < PI);

    func_00234C98(0x4E, (long)(D_0015EF88_m >> 13) | 0x1000000L);
}
