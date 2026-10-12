/* NON_MATCHING func_L01_00303810 -- src/overlays/shared/vendor_002F7700.c
 * Best so far: SIZE ours 464 / retail 524, checked 2026-10-12.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   func_L01_00303810: draw callback, tiles the screen (32 px) with textured quads: tile count from the screen siz
 */
typedef struct { int w; int h; int cx; int cy; } Scr_303810;
extern Scr_303810 D_0013E600_303810 __asm__("D_0013E600");
extern long func_001F4868_a(int) __asm__("func_001F4868");
extern void func_001F5800_a(int, int, int, int, int, int, int, int, long, long) __asm__("func_001F5800");

void func_L01_00303810(char *arg) {
    char *pv;
    float ctr[2];
    long tex;
    int nx;
    int ny;
    int i;
    int j;
    int half;
    float fx;
    float fy;
    float a;
    int col;
    int r1;
    int r2;
    int px;
    int py;

    pv = *(char **)(arg + 0x78);
    ctr[0] = func_001FA888(D_0013E600_303810.cx);
    ctr[1] = func_001FA888(D_0013E600_303810.cy);
    tex = func_001F4868_a(0x1E);
    nx = (D_0013E600_303810.w >> 5) + 1;
    ny = (D_0013E600_303810.h >> 5) + 1;
    for (i = 0; i < nx; i++) {
        for (j = 0; j < ny; j++) {
            fx = (float)(i << 5);
            fy = (float)(j << 5);
            half = 0x20 >> 1;
            a = func_001FA888(*(int *)(pv + 0x140));
            a = a * 127.5f / func_001FA888(func_001F9850(0x5A));
            col = (func_001FA898_r(a) << 24) + 0xFF7F7F;
            r1 = func_002140B0(0x20);
            r2 = func_002140B0(0x20);
            px = func_001FA898_r(fx - (float)half);
            py = func_001FA898_r(fy - (float)half);
            func_001F5800_a(px, py, 0x20, 0x20, r1, r2, 0x20, 0x20, col, tex);
        }
    }
}
