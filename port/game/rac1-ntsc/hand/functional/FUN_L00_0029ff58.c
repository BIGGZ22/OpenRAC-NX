/* Carried from the PAL decompilation's src/overlays/shared/vendor_0029FD68.c (matched C there) (games/rac1) through the version
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
/* What the PAL file this was written in declares before it (its unit, and its own file). */
extern int func_001F44B8(int);
extern void func_L00_001FD228(void *, void *, int);
extern void func_001F9CF8(void *, void *, void *);
struct Quad_c29ff58 { int a, b, c, d; };
extern struct Quad_c29ff58 D_L00_001CA480[];
extern float D_L00_001CA380[][4];
extern float D_L00_001CA410[][2];
extern int D_L00_001CA458[];
extern int D_L00_001CA4C0[];
extern short D_L00_00161184;
struct HoloPacket_c29ff58 {
    float pos[4][4];
    int col[4];
    float uv[4][2];
    long hdr[4];
};



/* Draws the four quads of the hologram cone, scrolling the texture coordinates. */
void FUN_L00_0029ff58(void) {
    struct HoloPacket_c29ff58 s;
    float v[4][4];
    int i;
    *(float *)&D_L00_00161184 = *(float *)&D_L00_00161184 + 0.01f;
    if (*(float *)&D_L00_00161184 > 1.0f) *(float *)&D_L00_00161184 = *(float *)&D_L00_00161184 - 1.0f;
    s.hdr[2] = 1;
    s.hdr[3] = 0x4000000044L;
    s.hdr[0] = 0;
    s.hdr[1] = func_001F44B8(0x18);
    for (i = 0; i < 4; i++) {
        func_001F9CF8(v[0], D_L00_001CA380[D_L00_001CA480[i].a], (char *)D_L00_001CA4C0[7] + 0xC0);
        func_001F9CF8(v[1], D_L00_001CA380[*(int *)((char *)D_L00_001CA480 + i * 16 + 4)], (char *)D_L00_001CA4C0[7] + 0xC0);
        func_001F9CF8(v[2], D_L00_001CA380[*(int *)((char *)D_L00_001CA480 + i * 16 + 8)], (char *)D_L00_001CA4C0[7] + 0xC0);
        func_001F9CF8(v[3], D_L00_001CA380[D_L00_001CA480[i].d], (char *)D_L00_001CA4C0[7] + 0xC0);
        s.pos[0][0] = *(float *)((char *)D_L00_001CA4C0[7] + 0x10) + v[0][0];
        s.pos[0][1] = *(float *)((char *)D_L00_001CA4C0[7] + 0x14) + v[0][1];
        s.pos[0][2] = *(float *)((char *)D_L00_001CA4C0[7] + 0x18) + v[0][2];
        s.pos[1][0] = *(float *)((char *)D_L00_001CA4C0[7] + 0x10) + v[1][0];
        s.pos[1][1] = *(float *)((char *)D_L00_001CA4C0[7] + 0x14) + v[1][1];
        s.pos[1][2] = *(float *)((char *)D_L00_001CA4C0[7] + 0x18) + v[1][2];
        s.pos[2][0] = *(float *)((char *)D_L00_001CA4C0[7] + 0x10) + v[2][0];
        s.pos[2][1] = *(float *)((char *)D_L00_001CA4C0[7] + 0x14) + v[2][1];
        s.pos[2][2] = *(float *)((char *)D_L00_001CA4C0[7] + 0x18) + v[2][2];
        s.pos[3][0] = *(float *)((char *)D_L00_001CA4C0[7] + 0x10) + v[3][0];
        s.pos[3][1] = *(float *)((char *)D_L00_001CA4C0[7] + 0x14) + v[3][1];
        s.pos[3][2] = *(float *)((char *)D_L00_001CA4C0[7] + 0x18) + v[3][2];
        s.col[0] = D_L00_001CA458[D_L00_001CA480[i].a];
        s.col[1] = D_L00_001CA458[*(int *)((char *)D_L00_001CA480 + i * 16 + 4)];
        s.col[2] = D_L00_001CA458[*(int *)((char *)D_L00_001CA480 + i * 16 + 8)];
        s.col[3] = D_L00_001CA458[D_L00_001CA480[i].d];
        s.uv[0][0] = D_L00_001CA410[D_L00_001CA480[i].a][0];
        s.uv[0][1] = D_L00_001CA410[D_L00_001CA480[i].a][1] + *(float *)&D_L00_00161184;
        s.uv[1][0] = D_L00_001CA410[*(int *)((char *)D_L00_001CA480 + i * 16 + 4)][0];
        s.uv[1][1] = D_L00_001CA410[*(int *)((char *)D_L00_001CA480 + i * 16 + 4)][1] + *(float *)&D_L00_00161184;
        s.uv[2][0] = D_L00_001CA410[*(int *)((char *)D_L00_001CA480 + i * 16 + 8)][0];
        s.uv[2][1] = D_L00_001CA410[*(int *)((char *)D_L00_001CA480 + i * 16 + 8)][1] + *(float *)&D_L00_00161184;
        s.uv[3][0] = D_L00_001CA410[D_L00_001CA480[i].d][0];
        s.uv[3][1] = D_L00_001CA410[D_L00_001CA480[i].d][1] + *(float *)&D_L00_00161184;
        func_L00_001FD228(&s, 0, 1);
    }
}
