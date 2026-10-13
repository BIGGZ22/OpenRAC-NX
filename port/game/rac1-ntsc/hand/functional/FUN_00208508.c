/* Carried from the PAL decompilation's src/game/menu.c (matched C there) (games/rac1/pal) through the version
 * map (../../README.md): functional C, not matched C. Names and addresses are this version's. */
#include "types.h"
#include "qcopy.h"
#include "qzero.h"
#ifndef MACRO_ADDR
#define MACRO_ADDR
#endif
#ifndef NOT_SDA
#define NOT_SDA
#endif
#ifndef SDATA
#define SDATA(symbol) __asm__(#symbol)
#endif
/* What the PAL file this was written in declares before it. */
extern int D_001A00F0[];
typedef struct {
    float x;
    float y;
    int icon;           /* icon id, or -1 for a plain square */
    unsigned int sub;   /* the icon's frame, or the square's colour */
} MapMarker_c208508;
extern int D_0015ED84 MACRO_ADDR;
extern void func_00200E08(int, int, int, int, long, int);
extern int D_0015FDB0 MACRO_ADDR;
extern int func_001FF960(int, int);
extern char D_0019A4E8_raw[] __asm__("D_0019A3E8");
extern void func_00200080(int, int, int, int, int, int);

void FUN_00208508(int left, int top, int right, int bottom) {
    float nx;
    float ny;
    int n = D_001A00F0[0x2C];
    MapMarker_c208508 *m = (MapMarker_c208508 *)D_001A00F0[7];

    while (n-- != 0) {
        int sx;
        int sy;

        func_00208408(&nx, &ny, D_0015ED84, m->x, m->y);
        sx = left + (int)((float)(right - left) * nx);
        sy = top + (int)((float)(bottom - top) * ny);
        if (sx < -0x199 || sy < -0x199 || sx >= 0x219A || sy >= 0x1B9A) {
            m++;
            continue;
        }
        if (m->icon == -1) {
            func_00200E08(sx - D_0015FDB0, sy - D_0015FDB0, sx + D_0015FDB0,
                          sy + D_0015FDB0, m->sub, 1);
        } else {
            int tex = func_001FF960(m->icon, m->sub);
            int *st = D_001A00F0;
            float *sc = (float *)(st + 0x2D);   /* the zoom scale table, at 0xB4 of the same block (the assembly's D_001A01A4) */
            char *arena = D_0019A4E8_raw;
            unsigned char *e = (unsigned char *)(*(char **)(arena + 0x24)
                + *(short *)(*(char **)(arena + 0x20) + tex * 4 + 2) * 8);
            float scale = (2.0f * sc[st[0x89]] + 5.0f) / 13.0f;
            float w = scale * (float)(1 << (e[6] + 4));

            func_00200080(tex, sx - (int)(scale * (float)(1 << (e[6] + 3))),
                          sy - (int)(scale * (float)(1 << (e[7] + 3))),
                          (int)w, (int)(scale * (float)(1 << (e[7] + 4))), 0x80);
        }
        m++;
    }
}
