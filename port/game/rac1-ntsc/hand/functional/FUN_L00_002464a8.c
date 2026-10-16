/* Carried from the PAL decompilation's src/overlays/shared/map_002465F8.c (matched C there) (games/rac1) through the version
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
extern int D_0015EE84_2465F8 __asm__("D_0015ED84") MACRO_ADDR;
int func_00232E40(int arg0, int arg1, int arg2, int arg3);
extern void func_001F98D0(void *, void *, int);
extern void func_00207BB0(void *arg0, unsigned char *arg1, int arg2);
extern void func_00206710(int *dst, unsigned char *src, short *offs);
extern int D_L00_00183EF0[];
extern float D_L00_00182810[];
extern float D_L00_00182A70[];
extern char D_L00_0015FE00[];
extern int *D_L00_00173E08;
extern char D_L00_001E8B00[];
extern unsigned char D_L00_00184394[];
extern unsigned char D_0013D407[] NOT_SDA;
extern unsigned char D_0014161B[] NOT_SDA;



/* Sets up the map's tables, loads its data from disc and builds the circular mask. */
void FUN_L00_002464a8(void) {
    int i, j;
    int *gl;
    int *q;
    int v;
    int *e;
    int *data;
    int n, len;
    int heap_tag, first_size, second_size, scratch;
    int *archive, *archive_base;
    unsigned char *mask, *level_data;
    int x, y, w;
    float lim;
    unsigned char *pp;
    extern int D_L00_001841F0[];
    extern char D_00137B80[];

    D_L00_001841F0[4] = 1;
    { int i;
    for (i = 0; i < 20; i++) {
        D_L00_001841F0[0x41 + i] = D_L00_00183EF0[i * 2];
        ((float *)D_L00_001841F0)[0x2D + i] = 0.65f;
        D_L00_001841F0[0x55 + i] = D_L00_00183EF0[i * 2 + 1];
    } }
    v = D_0015EE84_2465F8;
    D_L00_001841F0[0x8B] = -1;
    D_L00_001841F0[0x89] = v > 18 ? 0 : v;
    { int i;
    for (i = 0; i < 19; i++) {
        float a = D_L00_00182810[i * 8], b = D_L00_00182810[i * 8 + 1], c = D_L00_00182810[i * 8 + 2], dd = D_L00_00182810[i * 8 + 3];
        float ee = D_L00_00182810[i * 8 + 4], f = D_L00_00182810[i * 8 + 5], gg = D_L00_00182810[i * 8 + 6], h = D_L00_00182810[i * 8 + 7];
        float p = (ee - gg) / (a - c);
        float r = (f - h) / (b - dd);
        D_L00_00182A70[i * 4 + 1] = p;
        D_L00_00182A70[i * 4 + 3] = r;
        D_L00_00182A70[i * 4] = ee - p * a;
        D_L00_00182A70[i * 4 + 2] = f - r * b;
    } }
    pp = (unsigned char *)D_0013D407 + 0xB9;
    if (pp[0x21] != 0) {
        D_L00_001841F0[0x8C] = 1;
        e = (int *)(D_00137B80 + 0x8B8) + D_L00_001841F0[0x89] * 2;
    } else {
        D_L00_001841F0[0x8C] = 0;
        e = (int *)(D_00137B80 + 0x820) + D_L00_001841F0[0x89] * 2;
    }
    if (e[1] != 0) {
        gl = D_L00_001841F0;
        gl[10] = 1;
        heap_tag = (int)D_L00_0015FE00;
        archive_base = D_L00_00173E08;
        archive = (int *)((int)archive_base + 0x9A800);
        scratch = e[1] << 7;
        func_00216828(archive, e[0], e[1]);
        gl[0x8B] = func_00232E40((int)archive, scratch, 0x4800, (int)D_L00_001E8B00);
        gl[0x8D] = scratch;
        func_0020B618((int)archive, (int)archive_base);
        archive = archive_base;
        first_size = archive[1] - archive[0];
        gl[0] = func_001FF288(first_size, 0, (char *)heap_tag, 0x2DB);
        func_001F98D0((void *)gl[0], (char *)archive + archive[0], first_size);
        gl[0] += 8;
        gl[1] = gl[0];
        second_size = archive[2] - archive[1];
        gl[5] = func_001FF288(second_size, 0, (char *)heap_tag, 0x2EC);
        func_001F98D0((void *)gl[5], (char *)archive + archive[1], second_size);
        gl[2] = func_001FF288(0x1000, 0, (char *)heap_tag, 0x2F3);
        for (i = 0; i < 8; i++) {
            D_L00_001841F0[12 + i * 4] = -1;
            D_L00_001841F0[13 + i * 4] = -1;
            D_L00_001841F0[14 + i * 4] = D_L00_001841F0[2] + i * 0x200;
        }
        w = func_001FF288(0x8000, 0, D_L00_0015FE00, 0x2FC);
        level_data = D_0014161B + 0x8A5 + D_L00_001841F0[0x89] * 0x800;
        D_L00_001841F0[3] = w;
        if (level_data[0] != 0) {
            func_00207BB0((void *)D_L00_001841F0[3], level_data, D_L00_001841F0[5]);
        } else {
            func_00206710((int *)D_L00_001841F0[3], (unsigned char *)D_L00_001841F0[0], (short *)D_L00_001841F0[1]);
        }
        lim = 256.0f;
        for (y = 0; y < 32; y++) {
            for (x = 0; x < 32; x++) {
                if ((15.5f - (float)x) * (15.5f - (float)x) +
                    (15.5f - (float)y) * (15.5f - (float)y) > lim) {
                    D_L00_00184394[x / 8 + y * 4] |= 1 << (x - (x / 8) * 8);
                }
            }
        }
    } else {
        D_L00_001841F0[0] = 0;
        D_L00_001841F0[2] = 0;
        D_L00_001841F0[3] = 0;
        D_L00_001841F0[1] = 0;
        D_L00_001841F0[10] = 0;
    }
}
