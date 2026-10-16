/* Carried from the PAL decompilation's src/overlays/shared/help_00237B00.c (matched C there) (games/rac1) through the version
 * map (../../README.md): functional C, not matched C. Names and addresses are this version's.
 * The two versions' code differs here: func_L05_002559F0 (near) was paired by place; read it against NTSC-U before trusting it. */
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
extern unsigned char D_0013E533[] NOT_SDA;
extern int func_001F9688(int);
typedef struct {
    u8 pad0[0x18];
    s32 moby;
    s32 unk1C;
    u8 pad20[0x20];
    f32 normal_x;
    f32 normal_y;
    f32 normal_z;
} CollisionHit_4C558_c255208;
extern CollisionHit_4C558_c255208 D_L05_00174340_4C558 __asm__("D_L05_00174240") NOT_SDA;
extern float func_001F9CE8_4C558(void *) __asm__("func_001F9B20");
extern void func_001F9BD8_4C558(void *, void *, void *) __asm__("func_001F9A10");
extern float func_001F99C0(float);
extern int func_L00_001EFC70(void *, void *, int, int, int);
extern void func_L05_0024CEE8(int, int);
extern float func_L00_001FF8B0(float, float);
extern float func_001FA688(float, float);
extern float func_001F9B80(void *, void *);
extern int D_0015EE84_2523C8 __asm__("D_0015ED84") MACRO_ADDR;
extern void func_L00_002323B8(int, int, float);
extern int func_L05_002551B8(void);
extern unsigned char D_0013D407[] NOT_SDA;
extern unsigned char D_0013D355_254DE8[] __asm__("D_0013D255") NOT_SDA;
extern int func_0022EE28_254DE8(int, int, int) __asm__("func_0022DB10");
extern void func_L00_00264DB8_254DE8(int, int) __asm__("func_L00_00263D40");
extern short D_L05_0015F698;
extern float D_0015ED6C MACRO_ADDR;
extern void func_L00_00233660(float *, float, float, float);
extern void func_L00_001FF500(void *, void *, float);
extern float func_001FA580(float, float);
extern int func_L01_0028B510(void *, void *, float);
extern int func_L00_0025D7A0(void *, int, int, int);
typedef struct {
    char pad0[0x80];
    float f80[4];
    float f90;
    float f94;
    float f98;
    char pad9C[0xE0 - 0x9C];
    float fE0[4];
    char padF0[0x100 - 0xF0];
    float f100[4];
    char pad110[0x164 - 0x110];
    float f164;
    char pad168[0x218 - 0x168];
    float f218;
    char pad21C[0x257 - 0x21C];
    unsigned char b257;
    char pad258[0x2D8 - 0x258];
    float f2D8;
    float f2DC;
    char pad2E0[0x30C - 0x2E0];
    short s30C;
    char pad30E[0x6F0 - 0x30E];
    int i6F0;
    char pad6F4[0x70C - 0x6F4];
    int i70C;
    char pad710[0x714 - 0x710];
    int i714;
    char pad718[0x874 - 0x718];
    float f874;
    char pad878[0x88F - 0x878];
    unsigned char b88F;
    char pad890[0x89C - 0x890];
    short s89C;
    char pad89E[0x8A0 - 0x89E];
    float f8A0;
    float f8A4;
    char pad8A8[0x8B4 - 0x8A8];
    unsigned char *p8B4;
    char pad8B8[0x8BC - 0x8B8];
    short s8BC;
    char pad8BE[0x8C8 - 0x8BE];
    short s8C8;
    char pad8CA[0xAA8 - 0x8CA];
    float fAA8;
    char padAAC[0x12EA - 0xAAC];
    unsigned char b12EA;
    char pad12EB[0x2080 - 0x12EB];
    unsigned char *p2080;
    int i2084;
} H_9F0_c255208;
typedef struct { char pad0[7]; unsigned char f7; } Flags_9F0_c255208;




/* Help picker: tests the hero's state, surroundings and path, then sets a help mode or shows a hint. */
int FUN_L05_00255208(void)
{
    H_9F0_c255208 *g;
    char *h;
    unsigned char *pp;
    unsigned char *m;
    float *vp;
    float (*pts)[4];
    float v1[4];
    float v2[4];
    float v3[4];
    float f0;
    float f1;
    float f2;
    float f20;
    int r16;
    int r4;
    int ok;
    int ee;
    unsigned char c;

    g = ((H_9F0_c255208 *)(D_0013E533 + 0xE1D));
    if (!(D_0015ED6C * 12.0f < g->f164)) goto L_BAC;
    f20 = 0.0f;
    func_L00_00233660(v1, f20, f20, 0.75f);
    vp = v2;
    qcopy(vp, v1);
    qcopy(v3, g->fE0);
    func_L00_001FF500(v3, v3, 0.55f);
    func_001F9BD8_4C558(v3, v3, g->fE0);
    v3[2] = f20;
    func_001F9BD8_4C558(vp, v1, v3);
    if (!func_L00_001EFC70(v1, vp, 2, 0, 0)) goto L_BAC;
    h = (char *)&D_L05_00174340_4C558;
    if (*(int *)(h + 0x18) != 0) goto L_BAC;
    if (*(int *)(h + 0x1C) == 0) goto L_BAC;
    if (!(1.2217305f < func_L00_001FF8B0(*(float *)(h + 0x48), func_001F9CE8_4C558(h + 0x40)))) goto L_BAC;
    g->f8A4 = func_001FA580(func_L00_001FF8B0(*(float *)(h + 0x40), *(float *)(h + 0x44)), 3.1415927f);
    if (!(func_001FA688(func_L00_001FF8B0(g->fE0[0], g->fE0[1]), g->f8A4) < 0.7853982f)) goto L_BAC;
    if (g->f2DC < 1.0f) {
        func_L05_0024CEE8(0x6F, 1);
        return 1;
    }
    func_L05_0024CEE8(0x6D, 1);
    return 1;

L_BAC:
    if (((H_9F0_c255208 *)(D_0013E533 + 0xE1D))->b257 != 0) {
        f20 = func_001F9CE8_4C558(((H_9F0_c255208 *)(D_0013E533 + 0xE1D))->f100);
        if (f20 < func_001F9CE8_4C558(((H_9F0_c255208 *)(D_0013E533 + 0xE1D))->fE0) * 0.25f) {
            func_L05_0024CEE8(0x6D, 1);
            return 1;
        }
    }
    ee = D_0015EE84_2523C8;
    if (ee == 5) {
        if (((H_9F0_c255208 *)(D_0013E533 + 0xE1D))->f80[2] < 61.4f) {
            ((H_9F0_c255208 *)(D_0013E533 + 0xE1D))->f8A0 = 61.4f;
            func_L05_0024CEE8(0x6E, 1);
            return 1;
        }
    }
    if (ee == 0x10) {
        if (((H_9F0_c255208 *)(D_0013E533 + 0xE1D))->f80[2] < 76.0f) {
            ((H_9F0_c255208 *)(D_0013E533 + 0xE1D))->f8A0 = 76.0f;
            func_L05_0024CEE8(0x6E, 1);
            return 1;
        }
        if (((H_9F0_c255208 *)(D_0013E533 + 0xE1D))->b12EA != 0) {
            if (((H_9F0_c255208 *)(D_0013E533 + 0xE1D))->f2DC < 0.1f) {
                func_L05_0024CEE8(0x6D, 1);
                return 1;
            }
        }
    }
    if (((H_9F0_c255208 *)(D_0013E533 + 0xE1D))->s30C != 0) goto L_DEC;
    pp = ((H_9F0_c255208 *)(D_0013E533 + 0xE1D))->p8B4;
    if (pp == 0) goto L_D68;
    vp = ((H_9F0_c255208 *)(D_0013E533 + 0xE1D))->f80;
    r16 = func_L01_0028B510(vp, pp, 0.0f);
    r4 = func_L00_0025D7A0(((H_9F0_c255208 *)(D_0013E533 + 0xE1D))->p8B4, r16, -1, 1);
    ((H_9F0_c255208 *)(D_0013E533 + 0xE1D))->s8C8 = r16;
    if (D_0015EE84_2523C8 != 5) goto L_D68;
    pts = (float (*)[4])((H_9F0_c255208 *)(D_0013E533 + 0xE1D))->p8B4;
    f1 = *(float *)((char *)pts + (r16 << 4) + 0x18);
    f0 = *(float *)((char *)pts + (r4 << 4) + 0x18);
    if (f1 < f0) f20 = f1;
    else f20 = f0;
    f2 = func_001F9B80(vp, pts[r16 + 1]);
    if (0.5f < f20 - ((H_9F0_c255208 *)(D_0013E533 + 0xE1D))->f80[2]) {
        if (4.0f < f2) {
            func_L05_0024CEE8(0x6D, 1);
            return 1;
        }
    }

L_D68:
    if (((H_9F0_c255208 *)(D_0013E533 + 0xE1D))->s30C != 0) goto L_DEC;
    if (((H_9F0_c255208 *)(D_0013E533 + 0xE1D))->s8BC == 0) goto L_DEC;
    if (((H_9F0_c255208 *)(D_0013E533 + 0xE1D))->s89C != 0) goto L_DEC;
    m = ((H_9F0_c255208 *)(D_0013E533 + 0xE1D))->p2080;
    if (m[0x53] != 0x55 && m[0x53] != 0x7F) goto L_DEC;
    if (m[0x51] < 0xD) {
        int r = func_L05_002551B8();
        int t = func_001F9688(7);
        func_L00_002323B8(r, 14, (float)t);
    }

L_DEC:
    ok = (((H_9F0_c255208 *)(D_0013E533 + 0xE1D))->s30C == 0);
    if (((H_9F0_c255208 *)(D_0013E533 + 0xE1D))->b257 != 0) {
        if (func_001F99C0(((H_9F0_c255208 *)(D_0013E533 + 0xE1D))->f218 - ((H_9F0_c255208 *)(D_0013E533 + 0xE1D))->f2D8) < 0.05f) ok = 1;
    }
    if (ok == 0) return 0;
    if (((H_9F0_c255208 *)(D_0013E533 + 0xE1D))->s89C == 0) return 0;
    if (!(((H_9F0_c255208 *)(D_0013E533 + 0xE1D))->fE0[2] < 0.0f)) return 0;
    f20 = 1.0471976f;
    if (f20 < func_001F99C0(((H_9F0_c255208 *)(D_0013E533 + 0xE1D))->f94)) {
        func_L05_0024CEE8(0x6D, 1);
        return 1;
    }
    if (f20 < func_001F99C0(((H_9F0_c255208 *)(D_0013E533 + 0xE1D))->f90)) {
        func_L05_0024CEE8(0x6D, 1);
        return 1;
    }
    if (1.3962634f < func_001FA688(((H_9F0_c255208 *)(D_0013E533 + 0xE1D))->f98, ((H_9F0_c255208 *)(D_0013E533 + 0xE1D))->f874)) {
        func_L05_0024CEE8(0x6D, 1);
        return 1;
    }
    if (((H_9F0_c255208 *)(D_0013E533 + 0xE1D))->p2080[0x53] >= 0x69 && ((H_9F0_c255208 *)(D_0013E533 + 0xE1D))->p2080[0x53] <= 0x6C) {
        if (((H_9F0_c255208 *)(D_0013E533 + 0xE1D))->fAA8 < (float)((int *)&D_L05_0015F698)[((H_9F0_c255208 *)(D_0013E533 + 0xE1D))->b88F]) {
            func_L05_0024CEE8(0x6D, 1);
            return 1;
        }
    }

    if (D_0013D355_254DE8[0x13B] != 0) {
        if (D_0015EE84_2523C8 == 5 && ((H_9F0_c255208 *)(D_0013E533 + 0xE1D))->i6F0 == 4 && ((H_9F0_c255208 *)(D_0013E533 + 0xE1D))->i70C >= 4) {
            Flags_9F0_c255208 *fl = (Flags_9F0_c255208 *)(D_0013D407 + 1);
            if (fl->f7 == 0) {
                fl->f7 = 1;
                func_0022EE28_254DE8(1, 0, 0);
                func_L00_00264DB8_254DE8(0x53DB, -1);
            }
        }
        ((H_9F0_c255208 *)(D_0013E533 + 0xE1D))->i714 = 0;
    }

    if (((H_9F0_c255208 *)(D_0013E533 + 0xE1D))->i2084 == 0x6C) {
        func_L05_0024CEE8(0x6B, 0);
        ee = (int)((H_9F0_c255208 *)(D_0013E533 + 0xE1D))->p2080;
        if (((unsigned char *)ee)[0x53] != 0x55 && ((unsigned char *)ee)[0x53] != 0x7F) {
            int r = func_L05_002551B8();
            int t = func_001F9688(7);
            func_L00_002323B8(r, 14, (float)t);
        }
        return 1;
    }
    c = ((H_9F0_c255208 *)(D_0013E533 + 0xE1D))->p2080[0x53];
    if (c != 0x55 && c != 0x7F) {
        int r = func_L05_002551B8();
        int t = func_001F9688(7);
        func_L00_002323B8(r, 14, (float)t);
    }
    return 0;
}
