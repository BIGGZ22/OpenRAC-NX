/* Carried from the PAL decompilation's nonmatching/l15_quartu/func_L15_002D2398.c (games/rac1/pal) through the version
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
typedef u32 VecQuad_c2d0fa8 __attribute__((mode(TI), aligned(16)));
typedef union {
    VecQuad_c2d0fa8 q;
    f32 f[4];
    s32 i[4];
} Vec4_c2d0fa8;
typedef struct { int n; int pad[3]; Vec4_c2d0fa8 pt[1]; } UPath_c2d0fa8;
typedef int u128_c2d0fa8 __attribute__((mode(TI)));
extern float D_0015ED70 MACRO_ADDR;
extern float D_0015ED6C MACRO_ADDR;
extern UPath_c2d0fa8 *D_L15_001B0AB0[];

/* NON_MATCHING FUN_L15_002d0fa8 -- src/overlays/l15_quartu/vendor_0029C1D0.c
 * Best so far: SIZE ours 1196 / retail 1172, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Level 15 moby update (1172 bytes): finds the owning entry in the 64-entry table, runs a state switch (cases 0.
 *   Remaining differences: frame and saved-register layout (ours saves an extra register and the table base is hoi
 */
extern int func_L00_0025A478(void *, void *, void *, int, void *, void *, int, int);
extern int func_001FA6D0(float);
extern void func_L03_00250A78(float *, float, float);
extern float func_L00_001FF8B0(float, float);
extern void func_L00_0025AB48(void *, float *, void *, void *);
extern void func_L00_0025C558(float, void *, void *, int, int, int);
extern int func_L00_0028DC90(int i, int a1, int v, int k);
extern void func_L00_0024F7C8(void *, int, void *);
extern void func_L00_0025E450(void *, void *, void *, float, float, int, int, int, float, float, float, float, int, float, int, int, int, int);
extern void func_L00_00257470(void *, int, int);
extern void func_L00_0025D458(void *m, short *p);
extern void func_L00_0025D538(void *, void *);
extern void func_L00_0025FCB8(float, void *, void *);
extern int func_L00_0025FF38(float, char *, void *, int, int, void *, int);
extern float func_001F9B48(void *, void *);
extern char D_L15_00178700[];
extern short D_L15_00161B2C;
extern short D_L15_00161B00;
extern short D_L15_00161B08;
extern short D_L15_00161B04;
extern short D_L15_00161B30;
extern int D_001413D0 MACRO_ADDR;

/* Level 15 moby update: finds the table entry that owns this moby, runs its state switch and refreshes its timer and target. */
void FUN_L15_002d0fa8(void *mobyp) {
    char *moby = (char *)mobyp;
    char *data = *(char **)(moby + 0x78);
    char *vec = data + 0x60;
    char *vec2 = data + 0x120;
    char *other = 0;
    int i = 0;
    int a40;
    int a44 = 0;
    u128_c2d0fa8 vA;
    u128_c2d0fa8 vB;
    float f48;
    int ret;
    int sel = 0;
    int r2;
    float f;

    *(float *)(moby + 0x2C) = *(float *)&D_L15_00161B2C * *(float *)(*(char **)(moby + 0x24) + 0x24);
    *(int *)(data + 0x1C8) = 0;

    while (i < 0x40) {
        char *e = D_L15_00178700 + (i << 6);
        if (*(int *)(e + 0x34) != (int)moby) {
            i++;
            continue;
        }
        if ((*(int *)(e + 0x24) & 0xA0000) == 0) {
            i++;
            continue;
        }
        if (*(int *)(e + 0x20) != D_001413D0) {
            other = e;
            break;
        }
        if ((*(int *)(e + 0x24) & 0x80000) != 0) {
            other = e;
            break;
        }
        *(int *)(e + 0x34) = 0;
        i++;
    }

    if (other != 0) {
        ret = func_L00_0025A478(moby, other, data + 0x20, 0, &a40, &a44, 0, 4);
        if (a40 != 1 && moby[0x20] != 10 && moby[0x20] != 0x40
            && *(short *)(*(char **)(other + 0x20) + 0xA6) != 0x350) {
            float d;
            sel = 9;
            d = *(float *)(data + 0x20) - *(float *)&a44;
            *(float *)(data + 0x20) = d;
            if (*(unsigned short *)(other + 0x2A) != 0x100)
                sel = ret;
            if (moby[0x20] == 2)
                sel = 9;
            if (d <= 0.0f)
                sel = 1;

            r2 = func_001FA6D0(2560.0f);
            *(int *)(data + 0x90) = r2;
            *(float *)(data + 0x98) = 2.5f;
            *(float *)(data + 0x80) = *(float *)&D_L15_00161B00 * D_0015ED70;
            *(float *)(data + 0x84) = 0.000500000024f;
            *(int *)(data + 0x94) = 9;
            ((unsigned char *)data)[0xAD] = 0;

            switch (sel) {
            case 0:
                break;
            case 1:
            case 2: {
                u128_c2d0fa8 z = 0;
                *(u128_c2d0fa8 *)&vA = z;
                *(unsigned short *)(moby + 0x34) &= 0xEFFF;
                func_L00_0024F7C8(moby, 7, &vB);
                func_L00_0025E450(moby, &vA, &vB, 0.0f, 0.0f, 20, 6, 0x20, 16.0f, 8.0f, 9.0f, 4.0f, -1, 32.0f, 1, 0x10, -1, 0);
                func_L00_0028DC90(0, 0, (int)moby, 0x79);
                moby[0x20] = 10;
                ((unsigned char *)data)[0x67] = 0xF0;
                func_L00_00257470(moby, 0x200, -1);
                break;
            }
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
            case 8: {
                *(float *)(data + 0x88) = *(float *)&D_L15_00161B08 * D_0015ED6C;
                *(float *)(data + 0x8C) = *(float *)&D_L15_00161B04 * D_0015ED6C;
                *(int *)(data + 0x1C8) = 1;
                func_L03_00250A78((float *)vec, 4.0f, 1.0f);
                *(float *)(data + 0xC0) = 12.0f;
                *(float *)(data + 0xC4) = 16.0f;
                {
                    char *p20 = *(char **)(other + 0x20);
                    float fx = *(float *)(moby + 0x10) - *(float *)(p20 + 0x10);
                    float fy = *(float *)(moby + 0x14) - *(float *)(p20 + 0x14);
                    f48 = func_L00_001FF8B0(fx, fy);
                }
                *(u128_c2d0fa8 *)&vA = *(u128_c2d0fa8 *)(other + 0x10);
                func_L00_0025AB48(&vA, &f48, data + 0x88, data + 0x8C);
                func_L00_0025C558(f48, moby, vec, 8, 1, 0);
                func_L00_0028DC90(0, 0, (int)moby, 0x79);
                moby[0x20] = 9;
                ((unsigned char *)data)[0x67] = 0x78;
                break;
            }
            case 9:
            case 10:
                *(int *)(data + 0x1C8) = 1;
                ((unsigned char *)data)[0x67] = 0xFA;
                break;
            case 11:
                break;
            }

            func_L00_0025D458(moby, (short *)vec);
        }
        *(int *)(other + 0x34) = 0;
    }

    ((unsigned char *)moby)[0xA4] = 0xFF;
    func_L00_0025D538(moby, vec);
    *(float *)(moby + 0x2C) = *(float *)(*(char **)(moby + 0x24) + 0x24) * *(float *)&D_L15_00161B30;
    *(int *)(data + 0x164) = 0;

    if (moby[0x20] == 1) {
        f = *(float *)(data + 0x174);
        *(float *)(data + 0x1B8) = f;
        func_L00_0025FCB8(f, moby + 0, vec2);
    } else {
        f = *(float *)(data + 0x178);
        *(float *)(data + 0x1B8) = f;
        if (*(int *)(data + 0x1D0) == -1) {
            func_L00_0025FCB8(f, moby, vec2);
        } else {
            char *p = (char *)D_L15_001B0AB0[*(int *)(data + 0x1D0)];
            int n = *(int *)p;
            func_L00_0025FF38(f, moby, vec2, 0, 0, p + 0x10, n);
        }
    }

    f = func_001F9B48(moby + 0x10, vec2);
    if (*(float *)(data + 0x1B8) < f)
        *(int *)(data + 0x164) = 2;
    if (*(int *)(data + 0x160) == 0)
        *(int *)(data + 0x160) = D_001413D0;
}
