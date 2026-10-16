/* Carried from the PAL decompilation's src/overlays/shared/vendor_002B2A28.c (matched C there) (games/rac1) through the version
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
/* The quadword. Not called u128: source files declare their own u128,
   and a second typedef of one name is an error. */
typedef u32 VecQuad_c2b2928 __attribute__((mode(TI), aligned(16)));
typedef union {
    VecQuad_c2b2928 q;
    f32 f[4];
    s32 i[4];
} Vec4_c2b2928;
extern char D_0013E533[];
extern void func_L00_001FF500(void *, void *, float);
extern void func_001F9A10(void *, void *, void *);
extern void func_001F9A28(void *, void *, void *);
extern void func_001F9AD8(void *, void *, void *);
extern void func_001F9A68(void *, void *, float);
extern void func_001F9D20(void *, void *, void *);
extern void func_L00_001FD228(void *, void *, s32);
u64 func_001F44B8(s32);
extern char D_L14_001674C0[];
extern Vec4_c2b2928 D_L14_001D8850[];
extern f32 D_L14_00161574_3B78 SDATA(D_L14_001614AC);
extern f32 D_L14_00161584_3B78 SDATA(D_L14_001614BC);
extern f32 D_L14_00161588_3B78 SDATA(D_L14_001614C0);



// Draws five GIF line packets from the moby's beam points at data+0x1E0..0x220.
void FUN_L14_002b2928(char *moby) {
    char *data = *(char **)(moby + 0x78);
    struct {
        Vec4_c2b2928 o[4];
        unsigned int c[4];
        float g[8];
        u64 t[4];
    } pk;
    Vec4_c2b2928 m[4];
    Vec4_c2b2928 w;
    char *pp;
    char *q;
    char *r;
    int i;
    int j;
    int a;

    qcopy(&m[3], data + 0x1E0);
    m[3].f[3] = 1.0f;
    func_001F9A28(&m[0], D_L14_001674C0, &m[3]);
    func_L00_001FF500(&m[0], &m[0], 1.0f);
    func_001F9AD8(&m[1], &m[0], D_0013E533 + 0x10AD);
    func_L00_001FF500(&m[1], &m[1], -1.0f);
    func_001F9AD8(&m[2], &m[1], &m[0]);
    pk.t[1] = func_001F44B8(11);
    pk.t[3] = 0x8000000048ULL;
    pk.t[2] = 0xFF9000000260ULL;
    pk.t[0] = 5;
    pk.c[3] = 0x80FFFFFF;
    pk.c[2] = 0x80FFFFFF;
    pk.c[1] = 0x80FFFFFF;
    pk.c[0] = 0x80FFFFFF;
    pk.g[0] = 0.0f;
    pk.g[1] = 0.0f;
    pk.g[2] = 0.0f;
    pk.g[3] = 1.0f;
    pk.g[4] = 1.0f;
    pk.g[5] = 0.0f;
    pk.g[6] = 1.0f;
    pk.g[7] = 1.0f;
    for (i = 0; i < 4; i++) {
        func_001F9A68(&pk.o[i], &D_L14_001D8850[i], 0.4f);
        func_001F9D20(&pk.o[i], &pk.o[i], &m[0]);
    }
    func_L00_001FD228(&pk, 0, 0);

    qcopy(&m[3], data + 0x200);
    pp = data + 0x220;
    func_001F9A28(&m[0], D_L14_001674C0, &m[3]);
    r = data + 0x210;
    q = data + 0x1F0;
    func_L00_001FF500(&m[0], &m[0], 1.0f);
    func_001F9AD8(&m[1], &m[0], D_0013E533 + 0x10AD);
    func_L00_001FF500(&m[1], &m[1], -1.0f);
    func_001F9AD8(&m[2], &m[1], &m[0]);
    pk.c[3] = 0x202020FF;
    pk.c[2] = 0x202020FF;
    pk.c[1] = 0x202020FF;
    pk.c[0] = 0x202020FF;
    for (j = 0; j < 4; j++) {
        func_001F9A68(&pk.o[j], &D_L14_001D8850[j], 0.2f);
        func_001F9D20(&pk.o[j], &pk.o[j], &m[0]);
    }
    func_L00_001FD228(&pk, 0, 0);

    qcopy(&m[3], q);
    func_001F9A28(&m[0], D_L14_001674C0, &m[3]);
    func_L00_001FF500(&m[0], &m[0], 1.0f);
    func_001F9AD8(&m[1], &m[0], D_0013E533 + 0x10AD);
    func_L00_001FF500(&m[1], &m[1], -1.0f);
    func_001F9AD8(&m[2], &m[1], &m[0]);
    func_001F9A68(&w, &m[0], D_L14_00161574_3B78);
    func_001F9A10(&m[3], &m[3], &w);
    pk.c[3] = 0x4020FF20;
    pk.c[2] = 0x4020FF20;
    pk.c[1] = 0x4020FF20;
    pk.c[0] = 0x4020FF20;
    a = 0x40;
    for (j = 0; j < 4; j++) {
        func_001F9A68(&pk.o[j], &D_L14_001D8850[j], 0.2f);
        func_001F9D20(&pk.o[j], &pk.o[j], &m[0]);
    }
    func_L00_001FD228(&pk, 0, 0);

    qcopy(&m[3], r);
    func_001F9A28(&m[0], D_L14_001674C0, &m[3]);
    func_L00_001FF500(&m[0], &m[0], 1.0f);
    func_001F9AD8(&m[1], &m[0], D_0013E533 + 0x10AD);
    func_L00_001FF500(&m[1], &m[1], -1.0f);
    func_001F9AD8(&m[2], &m[1], &m[0]);
    func_001F9A68(&w, &m[0], D_L14_00161584_3B78);
    func_001F9A10(&m[3], &m[3], &w);
    pk.c[3] = (a << 24) | 0xFF2020;
    pk.c[2] = (a << 24) | 0xFF2020;
    pk.c[1] = (a << 24) | 0xFF2020;
    pk.c[0] = (a << 24) | 0xFF2020;
    for (j = 0; j < 4; j++) {
        func_001F9A68(&pk.o[j], &D_L14_001D8850[j], D_L14_00161588_3B78);
        func_001F9D20(&pk.o[j], &pk.o[j], &m[0]);
    }
    func_L00_001FD228(&pk, 0, 0);

    qcopy(&m[3], pp);
    func_001F9A28(&m[0], D_L14_001674C0, &m[3]);
    func_L00_001FF500(&m[0], &m[0], 1.0f);
    func_001F9AD8(&m[1], &m[0], D_0013E533 + 0x10AD);
    func_L00_001FF500(&m[1], &m[1], -1.0f);
    func_001F9AD8(&m[2], &m[1], &m[0]);
    func_001F9A68(&w, &m[0], D_L14_00161584_3B78);
    func_001F9A10(&m[3], &m[3], &w);
    pk.c[3] = (a << 24) | 0xFF2020;
    pk.c[2] = (a << 24) | 0xFF2020;
    pk.c[1] = (a << 24) | 0xFF2020;
    pk.c[0] = (a << 24) | 0xFF2020;
    for (j = 0; j < 4; j++) {
        func_001F9A68(&pk.o[j], &D_L14_001D8850[j], D_L14_00161588_3B78);
        func_001F9D20(&pk.o[j], &pk.o[j], &m[0]);
    }
    func_L00_001FD228(&pk, 0, 0);
}
