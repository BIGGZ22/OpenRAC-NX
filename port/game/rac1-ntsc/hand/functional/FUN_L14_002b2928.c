/* Carried from the PAL decompilation's nonmatching/shared/func_L14_002B3B78.c (games/rac1) through the version
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
u64 func_001F44B8(s32);

/* NON_MATCHING FUN_L14_002b2928 -- src/overlays/shared/vendor_002B2A28.c
 * Best so far: SIZE ours 1412 / retail 1452, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Five-block GIF packet builder: each block copies a 16-byte matrix row from data+0x1E0/0x200/0x1F0/0x210/0x220 
 *   Would unblock it: a way to write the data-relative row copies so the compiler emits retail's addiu-then-lq pai
 */
typedef int u128_2B3B78_c2b2928 __attribute__((mode(TI)));
extern void func_001F9A28(void *, void *, void *);
extern void func_L00_001FF500(void *, void *, float);
extern void func_001F9AD8(void *, void *, void *);
extern void func_001F9A68(void *, void *, float);
extern void func_001F9D20(void *, void *, void *);
extern void func_L00_001FD228(void *, void *, int);
extern void func_001F9A10(void *, void *, void *);
extern char D_L14_001674C0[];
extern char D_L14_001D8850[];
extern char D_0013F5E0[];
extern short D_L14_001614AC;
extern short D_L14_001614BC;
extern short D_L14_001614C0;

// Builds one GIF packet from the moby's matrix block at data+0x1E0..0x220 and runs the four row loops.
void FUN_L14_002b2928(char *moby) {
    char *data = *(char **)(moby + 0x78);
    struct {
        float o[16];
        float g[12];
        u64 t[4];
    } pk;
    float m[16];
    float w[4];
    char *pp;
    char *q;
    char *r;
    char *src;
    int k;
    float *op, *mp;

    src = data + 0x1E0;
    *(u128_2B3B78_c2b2928 *)(m + 12) = *(u128_2B3B78_c2b2928 *)src;
    m[15] = 1.0f;
    func_001F9A28(m, D_L14_001674C0, m + 12);
    func_L00_001FF500(m, m, 1.0f);
    func_001F9AD8(m + 4, m, D_0013F5E0);
    func_L00_001FF500(m + 4, m + 4, -1.0f);
    func_001F9AD8(m + 8, m + 4, m);
    pk.t[1] = func_001F44B8(11);
    pk.t[3] = 0x8000000048ULL;
    pk.t[2] = 0xFF9000000260ULL;
    pk.t[0] = 5;
    *(unsigned int *)&pk.g[0] = 0x80FFFFFF;
    *(unsigned int *)&pk.g[1] = 0x80FFFFFF;
    *(unsigned int *)&pk.g[2] = 0x80FFFFFF;
    *(unsigned int *)&pk.g[3] = 0x80FFFFFF;
    pk.g[4] = 0.0f;
    pk.g[5] = 0.0f;
    pk.g[6] = 0.0f;
    pk.g[7] = 1.0f;
    pk.g[8] = 1.0f;
    pk.g[9] = 0.0f;
    pk.g[10] = 1.0f;
    pk.g[11] = 1.0f;
    op = pk.o;
    mp = m;
    k = 3;
    do {
        func_001F9A68(op, mp, 0.4f);
        mp += 4;
        k--;
        func_001F9D20(op, op, m);
        op += 4;
    } while (k >= 0);
    func_L00_001FD228(pk.o, 0, 0);

    src = data + 0x200;
    *(u128_2B3B78_c2b2928 *)(m + 12) = *(u128_2B3B78_c2b2928 *)src;
    pp = data + 0x220;
    func_001F9A28(m, D_L14_001674C0, m + 12);
    q = data + 0x1F0;
    r = data + 0x210;
    func_L00_001FF500(m, m, 1.0f);
    k = 3;
    func_001F9AD8(m + 4, m, D_0013F5E0);
    func_L00_001FF500(m + 4, m + 4, -1.0f);
    func_001F9AD8(m + 8, m + 4, m);
    *(unsigned int *)&pk.g[0] = 0x202020FF;
    *(unsigned int *)&pk.g[1] = 0x202020FF;
    *(unsigned int *)&pk.g[2] = 0x202020FF;
    *(unsigned int *)&pk.g[3] = 0x202020FF;
    op = pk.o;
    mp = (float *)D_L14_001D8850;
    do {
        func_001F9A68(op, mp, 0.2f);
        mp += 4;
        k--;
        func_001F9D20(op, op, m);
        op += 4;
    } while (k >= 0);
    func_L00_001FD228(pk.o, 0, 0);

    *(u128_2B3B78_c2b2928 *)(m + 12) = *(u128_2B3B78_c2b2928 *)q;
    func_001F9A28(m, D_L14_001674C0, m + 12);
    func_L00_001FF500(m, m, 1.0f);
    k = 3;
    func_001F9AD8(m + 4, m, D_0013F5E0);
    func_L00_001FF500(m + 4, m + 4, -1.0f);
    func_001F9AD8(m + 8, m + 4, m);
    func_001F9A68(w, m, *(float *)&D_L14_001614AC);
    func_001F9A10(m + 12, m + 12, w);
    *(unsigned int *)&pk.g[0] = 0x4020FF20;
    *(unsigned int *)&pk.g[1] = 0x4020FF20;
    *(unsigned int *)&pk.g[2] = 0x4020FF20;
    *(unsigned int *)&pk.g[3] = 0x4020FF20;
    op = pk.o;
    mp = (float *)D_L14_001D8850;
    do {
        func_001F9A68(op, mp, 0.2f);
        mp += 4;
        k--;
        func_001F9D20(op, op, m);
        op += 4;
    } while (k >= 0);
    func_L00_001FD228(pk.o, 0, 0);

    *(u128_2B3B78_c2b2928 *)(m + 12) = *(u128_2B3B78_c2b2928 *)r;
    func_001F9A28(m, D_L14_001674C0, m + 12);
    func_L00_001FF500(m, m, 1.0f);
    k = 3;
    func_001F9AD8(m + 4, m, D_0013F5E0);
    func_L00_001FF500(m + 4, m + 4, -1.0f);
    func_001F9AD8(m + 8, m + 4, m);
    func_001F9A68(w, m, *(float *)&D_L14_001614BC);
    func_001F9A10(m + 12, m + 12, w);
    *(unsigned int *)&pk.g[0] = 0x40FF2020;
    *(unsigned int *)&pk.g[1] = 0x40FF2020;
    *(unsigned int *)&pk.g[2] = 0x40FF2020;
    *(unsigned int *)&pk.g[3] = 0x40FF2020;
    op = pk.o;
    mp = (float *)D_L14_001D8850;
    do {
        func_001F9A68(op, mp, *(float *)&D_L14_001614C0);
        mp += 4;
        k--;
        func_001F9D20(op, op, m);
        op += 4;
    } while (k >= 0);
    func_L00_001FD228(pk.o, 0, 0);

    *(u128_2B3B78_c2b2928 *)(m + 12) = *(u128_2B3B78_c2b2928 *)pp;
    func_001F9A28(m, D_L14_001674C0, m + 12);
    func_L00_001FF500(m, m, 1.0f);
    k = 3;
    func_001F9AD8(m + 4, m, D_0013F5E0);
    func_L00_001FF500(m + 4, m + 4, -1.0f);
    func_001F9AD8(m + 8, m + 4, m);
    *(unsigned int *)&pk.g[0] = 0x40FF2020;
    *(unsigned int *)&pk.g[1] = 0x40FF2020;
    *(unsigned int *)&pk.g[2] = 0x40FF2020;
    *(unsigned int *)&pk.g[3] = 0x40FF2020;
    op = pk.o;
    mp = (float *)D_L14_001D8850;
    do {
        func_001F9A68(op, mp, *(float *)&D_L14_001614C0);
        mp += 4;
        k--;
        func_001F9D20(op, op, m);
        op += 4;
    } while (k >= 0);
    func_L00_001FD228(pk.o, 0, 0);
}
