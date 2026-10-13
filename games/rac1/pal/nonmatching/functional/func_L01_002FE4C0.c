/* functional: high. Same reads, writes, calls and arguments in the same order as retail (no VU0 code; the lq/sq pair is a plain 16-byte copy); differs only in compiler choices (registers, stack slots, constant placement, folded offsets, loop layout), and the gp-relative globals are named by their addresses (_gp = 0x166D00). */
/* Update of the level 1 (Novalis) water surface effect moby. m = the moby, m[0x78] = its table of 7 trigger ids.
 *  State 0 (init): m.z += 0.5, state 1, m[0x30] = 0xFF; sets up the 21 water grids at D_L01_001E3780 (stride 0x1190)
 *    with func_L01_002B8C00 and a few per-grid constants, func_L01_002B90A8(1.0), the light block D_L01_001CAF80
 *    (direction from func_L00_001FF860 of D_L01_001803C0[4..5]), the globals 0x161288..0x161358, func_L01_002B8E20
 *    for each grid, then two random ripples per grid (func_L01_002B9440).
 *  State 1: for each trigger i whose id is set and func_00215570 reports it, flag = i, every grid of the group gets its
 *    id from D_L01_001FA850 and, when func_002140B0(D_L01_001FA8E8[i]) is 0, a random splash (func_L00_002A5158);
 *    other groups have their halfword at +0x1E cleared. The last triggered group picks the colour bytes
 *    0x161280..0x161286, then func_L01_002B9198 and func_001F49B0(func_L01_002FE498, m). Groups 0 and 6 count down
 *    0x161D14 and spawn func_L01_003010A8 at a random point of D_L01_001FA910; group 5 spawns the steam particles
 *    (func_L01_00288A48 once, func_L01_002888C8 at 20 heights). */
typedef int Q_FE4C0 __attribute__((mode(TI)));
typedef union { Q_FE4C0 q; float f[4]; } V_FE4C0;

extern char D_L01_001E3780_FE4C0[] __asm__("D_L01_001E3780");
extern char D_L01_001E379E_FE4C0[] __asm__("D_L01_001E379E");
extern char D_L01_001FA980_FE4C0[] __asm__("D_L01_001FA980");
extern float D_L01_001803C0_FE4C0[] __asm__("D_L01_001803C0");
extern float D_L01_001CAF80_FE4C0[] __asm__("D_L01_001CAF80");
extern int D_L01_00161358_FE4C0 __asm__("D_L01_00161358") MACRO_ADDR;
extern int D_L01_00161350_FE4C0 __asm__("D_L01_00161350") MACRO_ADDR;
extern int D_L01_00161354_FE4C0 __asm__("D_L01_00161354") MACRO_ADDR;
extern float D_L01_00161288_FE4C0 __asm__("D_L01_00161288") MACRO_ADDR;
extern float D_L01_0016128C_FE4C0 __asm__("D_L01_0016128C") MACRO_ADDR;
extern float D_L01_00161290_FE4C0 __asm__("D_L01_00161290") MACRO_ADDR;
extern float D_L01_00161294_FE4C0 __asm__("D_L01_00161294") MACRO_ADDR;
extern unsigned char D_L01_00161280_FE4C0 __asm__("D_L01_00161280") MACRO_ADDR;
extern unsigned char D_L01_00161281_FE4C0 __asm__("D_L01_00161281") MACRO_ADDR;
extern unsigned char D_L01_00161282_FE4C0 __asm__("D_L01_00161282") MACRO_ADDR;
extern unsigned char D_L01_00161283_FE4C0 __asm__("D_L01_00161283") MACRO_ADDR;
extern unsigned char D_L01_00161284_FE4C0 __asm__("D_L01_00161284") MACRO_ADDR;
extern unsigned char D_L01_00161285_FE4C0 __asm__("D_L01_00161285") MACRO_ADDR;
extern unsigned char D_L01_00161286_FE4C0 __asm__("D_L01_00161286") MACRO_ADDR;
extern int D_L01_00161D14_FE4C0 __asm__("D_L01_00161D14") MACRO_ADDR;
extern int D_L01_0015F6B0_FE4C0 __asm__("D_L01_0015F6B0") MACRO_ADDR;
extern char D_L01_001672C0_FE4C0[] __asm__("D_L01_001672C0");
extern int D_L01_001FA8A8_FE4C0[] __asm__("D_L01_001FA8A8");
extern int D_L01_001FA8C8_FE4C0[] __asm__("D_L01_001FA8C8");
extern int D_L01_001FA8E8_FE4C0[] __asm__("D_L01_001FA8E8");
extern unsigned short D_L01_001FA850_FE4C0[] __asm__("D_L01_001FA850");
extern char D_L01_001FA910_FE4C0[] __asm__("D_L01_001FA910");
extern unsigned char D_L01_001FA960_FE4C0[] __asm__("D_L01_001FA960");

extern void func_L01_002B8C00_FE4C0(void *, int) __asm__("func_L01_002B8C00");
extern void func_L01_002B90A8_FE4C0(float) __asm__("func_L01_002B90A8");
extern float func_L00_001FF860_FE4C0(float, float) __asm__("func_L00_001FF860");
extern float func_001F9F90_FE4C0(float) __asm__("func_001F9F90");
extern float func_001F9FA8_FE4C0(float) __asm__("func_001F9FA8");
extern void func_L01_002B8E20_FE4C0(void *, void *) __asm__("func_L01_002B8E20");
extern float func_002140F8_FE4C0(float, float) __asm__("func_002140F8");
extern void func_L01_002B9440_FE4C0(void *, int, float, float, float, float) __asm__("func_L01_002B9440");
extern int func_00215570_FE4C0(void *, int) __asm__("func_00215570");
extern int func_002140B0_FE4C0(int) __asm__("func_002140B0");
extern void func_L00_002A5158_FE4C0(void *, int, int, float, float, float, float) __asm__("func_L00_002A5158");
extern void func_L01_002B9198_FE4C0(void *, int) __asm__("func_L01_002B9198");
extern void func_001F49B0_FE4C0(void *, void *) __asm__("func_001F49B0");
extern void func_L01_002FE498_FE4C0(void) __asm__("func_L01_002FE498");
extern void func_L01_003010A8_FE4C0(void *, float) __asm__("func_L01_003010A8");
extern int func_L00_00258BC8_FE4C0(int, int) __asm__("func_L00_00258BC8");
extern void func_L01_00288A48_FE4C0(void *, void *, float, float) __asm__("func_L01_00288A48");
extern void func_L01_002888C8_FE4C0(void *, void *, float) __asm__("func_L01_002888C8");

void func_L01_002FE4C0(char *m) {
    int *ids = *(int **)(m + 0x78);
    char *grid = D_L01_001E3780_FE4C0;
    int flag, i, j, k, n;

    switch (((unsigned char *)m)[0x20]) {
    case 0: {
        float *L = D_L01_001CAF80_FE4C0;
        float kk = -0.0016f;
        float a;
        char *rec;
        char *out;

        *(float *)(m + 0x18) = *(float *)(m + 0x18) + 0.5f;
        m[0x20] = 1;
        ((unsigned char *)m)[0x30] = 0xFF;
        func_L01_002B8C00_FE4C0(grid, 0x15);
        *(int *)(grid + 0x107B4) = 8;
        *(float *)(grid + 0xE478) = kk;
        *(float *)(grid + 0x1079C) = 0.0f;
        *(float *)(grid + 0xE47C) = 0.0f;
        *(float *)(grid + 0x11928) = kk;
        *(float *)(grid + 0x10798) = kk;
        *(float *)(grid + 0xF608) = kk;
        *(float *)(grid + 0x1192C) = 0.0f;
        *(float *)(grid + 0xF60C) = 0.0f;
        func_L01_002B90A8_FE4C0(1.0f);
        L[0] = 16.0f;
        L[3] = -8.0f;
        L[6] = 0.9f;
        L[8] = 0.1f;
        L[2] = -8.0f;
        L[5] = 1.0f;
        L[4] = 1.0f;
        L[1] = 16.0f;
        a = func_L00_001FF860_FE4C0(D_L01_001803C0_FE4C0[4], D_L01_001803C0_FE4C0[5]);
        L[12] = func_001F9F90_FE4C0(a) * 0.57735002f;
        L[13] = func_001F9FA8_FE4C0(a) * 0.57735002f;
        ((unsigned char *)L)[0x3D] = 0x40;
        L[14] = -0.57735002f;
        ((int *)L)[10] = 41;
        ((int *)L)[11] = 42;
        ((unsigned char *)L)[0x3C] = 28;
        D_L01_00161358_FE4C0 = 21;
        D_L01_00161288_FE4C0 = 0.0f;
        D_L01_0016128C_FE4C0 = 32768.0f;
        D_L01_00161290_FE4C0 = 255.0f;
        D_L01_00161294_FE4C0 = 48.0f;
        D_L01_00161350_FE4C0 = (int)grid;
        D_L01_00161354_FE4C0 = (int)D_L01_001FA980_FE4C0;

        rec = grid;
        out = D_L01_001FA980_FE4C0;
        for (i = 20; i >= 0; i--) {
            func_L01_002B8E20_FE4C0(rec, out);
            out += 0x10;
            rec += 0x1190;
        }

        for (i = 0; i < 21; i++) {
            float *fv = (float *)(grid + i * 0x1190);
            for (j = 0; j < 2; j++) {
                float x, y;
                x = fv[0] + func_002140F8_FE4C0(-6.0f, 6.0f);
                y = func_002140F8_FE4C0(-6.0f, 6.0f);
                y = fv[1] + y;
                func_L01_002B9440_FE4C0(grid, 0x15, x, y, 2.0f, 0.2f);
            }
        }
        break;
    }
    case 1:
        flag = -1;
        for (i = 0; i < 7; i++) {
            if (ids[i] != -1 && func_00215570_FE4C0(D_L01_001672C0_FE4C0, ids[i]) != 0) {
                flag = i;
                for (k = 0; k < D_L01_001FA8C8_FE4C0[i]; k++) {
                    int idx = D_L01_001FA8A8_FE4C0[i] + k;
                    *(unsigned short *)(grid + idx * 0x1190 + 0x1E) = D_L01_001FA850_FE4C0[idx];
                    if (func_002140B0_FE4C0(D_L01_001FA8E8_FE4C0[i]) == 0) {
                        float x, y;
                        x = func_002140F8_FE4C0(-4.0f, 4.0f);
                        x = *(float *)(grid + (D_L01_001FA8A8_FE4C0[i] + k) * 0x1190) + x;
                        y = func_002140F8_FE4C0(-4.0f, 4.0f);
                        y = *(float *)(grid + (D_L01_001FA8A8_FE4C0[i] + k) * 0x1190 + 4) + y;
                        func_L00_002A5158_FE4C0((char *)D_L01_00161350_FE4C0 + D_L01_001FA8A8_FE4C0[i] * 0x1190 + k * 0x1190,
                                                1, 1, x, y, 1.0f, -0.05f);
                    }
                }
            } else {
                int cnt = D_L01_001FA8C8_FE4C0[i];
                if (cnt > 0) {
                    char *p = D_L01_001E379E_FE4C0 + D_L01_001FA8A8_FE4C0[i] * 0x1190;
                    do {
                        *(short *)p = 0;
                        p += 0x1190;
                    } while (--cnt != 0);
                }
            }
        }

        if (flag >= 0) {
            if ((unsigned)(flag - 1) < 3) {
                D_L01_00161285_FE4C0 = 0x28;
                D_L01_00161283_FE4C0 = 0x40;
                D_L01_00161286_FE4C0 = 0x30;
                D_L01_00161280_FE4C0 = 0;
                D_L01_00161281_FE4C0 = 0x28;
                D_L01_00161282_FE4C0 = 0x30;
                D_L01_00161284_FE4C0 = 0;
            } else {
                D_L01_00161284_FE4C0 = 0x18;
                D_L01_00161285_FE4C0 = 0x30;
                D_L01_00161286_FE4C0 = 0x90;
                D_L01_00161280_FE4C0 = 0x18;
                D_L01_00161281_FE4C0 = 0x30;
                D_L01_00161282_FE4C0 = 0x90;
                D_L01_00161283_FE4C0 = 0x30;
            }
        }

        func_L01_002B9198_FE4C0(grid, 0x15);
        func_001F49B0_FE4C0((void *)func_L01_002FE498_FE4C0, m);

        if (flag == 0 || flag == 6) {
            n = D_L01_00161D14_FE4C0 - 1;
            D_L01_00161D14_FE4C0 = n;
            if (n <= 0) {
                V_FE4C0 v;
                float r;
                int idx = func_002140B0_FE4C0(5);
                v.q = *(Q_FE4C0 *)(D_L01_001FA910_FE4C0 + idx * 16);
                r = func_002140F8_FE4C0(-0.15f, 0.15f);
                v.f[0] = v.f[0] + r;
                r = func_002140F8_FE4C0(-0.15f, 0.15f);
                v.f[1] = v.f[1] + r;
                func_L01_003010A8_FE4C0(&v, 0.2f);
                D_L01_00161D14_FE4C0 = func_L00_00258BC8_FE4C0(300, 1200);
            }
        }

        if (flag == 5) {
            V_FE4C0 vel;
            V_FE4C0 pos;
            float t, r;
            int rem = D_L01_0015F6B0_FE4C0 % 20;
            vel.f[0] = 0.02f;
            *(int *)&vel.f[1] = 0;
            *(int *)&vel.f[2] = 0;
            t = (float)D_L01_001FA960_FE4C0[rem] / 20.0f;
            r = func_002140F8_FE4C0(-0.6f, 0.6f);
            pos.f[0] = r + 177.0f;
            r = func_002140F8_FE4C0(-0.1f, 0.1f);
            pos.f[2] = 39.0f;
            pos.f[1] = t * -11.0f + 176.0f + r;
            r = func_002140F8_FE4C0(0.4f, 0.5f);
            func_L01_00288A48_FE4C0(&pos, &vel, 1.0f, r);
            *(int *)&vel.f[0] = 0;
            *(int *)&vel.f[1] = 0;
            *(int *)&vel.f[2] = 0;
            t = vel.f[0];
            do {
                if (func_002140B0_FE4C0(0x20) == 0) {
                    r = func_002140F8_FE4C0(-0.6f, 0.6f);
                    pos.f[0] = r + 178.0f;
                    r = func_002140F8_FE4C0(-0.1f, 0.1f);
                    pos.f[1] = t * -11.0f + 176.0f + r;
                    r = func_002140F8_FE4C0(-0.2f, 0.0f);
                    pos.f[2] = r + 39.0f;
                    r = func_002140F8_FE4C0(4.0f, 10.0f);
                    func_L01_002888C8_FE4C0(&pos, &vel, r);
                }
                t = t + 0.05f;
            } while (t < 1.0f);
        }
        break;
    default:
        break;
    }
}
