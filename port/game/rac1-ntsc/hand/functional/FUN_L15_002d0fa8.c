/* Carried from the PAL decompilation's src/overlays/l15_quartu/vendor_0029C1D0.c (matched C there) (games/rac1) through the version
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
typedef u32 VecQuad_c2d0fa8 __attribute__((mode(TI), aligned(16)));
/* Four floats aligned to 16 bytes, the layout moved by one lq/sq. */
typedef struct {
    f32 x;
    f32 y;
    f32 z;
    f32 w;
} __attribute__((aligned(16))) Vec4A_c2d0fa8;
typedef union {
    VecQuad_c2d0fa8 q;
    f32 f[4];
    s32 i[4];
} Vec4_c2d0fa8;
extern char D_L15_00178700[];
extern void func_L00_00257470(void *, int, int);
extern void func_L00_0025E450(void *, void *, void *, float, float, int, int, int, float, float, float, float, int, float, int, int, int, unsigned char);
extern int func_001FA6D0(float);
extern float func_L00_001FF8B0(float, float);
extern float D_0015ED6C MACRO_ADDR;
extern int func_L00_0025A478(void *, void *, void *, int, void *, void *, int, int);
extern void func_L00_0025D458(void *m, short *p);
extern void func_L00_0025D538(void *, void *);
extern void func_L00_00260D30_t(void *, float *, float) __asm__("func_L00_0025FCB8");
typedef int u128_c2d0fa8 __attribute__((mode(TI)));
typedef struct { int n; int pad[3]; Vec4_c2d0fa8 pt[1]; } UPath_c2d0fa8;
extern float func_001F9B48(void *, void *);
extern void func_L00_0024F7C8(void *, int, void *);
extern unsigned char D_0013E533[];
extern UPath_c2d0fa8 *D_L15_001B0AB0[];
extern float D_0015ED70 MACRO_ADDR;
extern void func_L03_00250A78(float *, float, float);
extern void func_L00_0025AB48(void *, float *, void *, void *);
extern void func_L00_0025C558(float, void *, void *, int, int, int);
extern int func_L00_0028EF68_2E9358(int, int, void *, int) __asm__("func_L00_0028DC90");
extern int func_L00_0025FF38(float, void *, void *, int, int, void *, int);
extern short D_L15_00161B2C;
extern short D_L15_00161B00;
extern short D_L15_00161B08;
extern short D_L15_00161B04;
extern short D_L15_00161B30;



/* Level 15 moby update: finds the table entry that owns this moby, runs its state switch and refreshes its range and target. */
void FUN_L15_002d0fa8(void *mobyp) {
    char *moby = (char *)mobyp;
    char *data = *(char **)(moby + 0x78);
    char *other = 0;
    int i;
    Vec4A_c2d0fa8 vA;
    Vec4A_c2d0fa8 vB;
    int a40;
    int a44;
    float f48;
    int sel;
    int r2;

    *(float *)(moby + 0x2C) = *(float *)&D_L15_00161B2C * *(float *)(*(char **)(moby + 0x24) + 0x24);
    a44 = 0;
    *(int *)(data + 0x1C8) = 0;

    for (i = 0; i < 0x40; i++) {
        char *e = D_L15_00178700 + (i << 6);
        int fl;
        if (*(int *)(e + 0x34) != (int)moby)
            continue;
        fl = *(int *)(e + 0x24);
        if ((fl & 0xA0000) == 0)
            continue;
        if (*(int *)(e + 0x20) == *(int *)(D_0013E533 + 0x2E9D) && (fl & 0x80000) == 0) {
            *(int *)(e + 0x34) = 0;
            continue;
        }
        other = e;
        break;
    }

    sel = func_L00_0025A478(moby, other, data + 0x20, 0, &a40, &a44, 0, 4);
    if (other != 0 && a40 != 1 && (unsigned char)moby[0x20] != 10 && (unsigned char)moby[0x20] != 0x40
        && *(short *)(*(char **)(other + 0x20) + 0xA6) != 0x350) {
        float d;
        d = *(float *)(data + 0x20) - *(float *)&a44;
        *(float *)(data + 0x20) = d;
        if (*(unsigned short *)(other + 0x2A) == 0x100)
            sel = 9;
        if ((unsigned char)moby[0x20] == 2)
            sel = 9;
        if (d <= 0.0f)
            sel = 1;

        r2 = func_001FA6D0(2560.0f);
        *(float *)(data + 0x80) = *(float *)&D_L15_00161B00 * D_0015ED70;
        *(int *)(data + 0x90) = r2;
        *(float *)(data + 0x98) = 2.5f;
        *(float *)(data + 0x84) = 0.0005f;
        *(int *)(data + 0x94) = 9;
        ((unsigned char *)data)[0xAD] = 0;

        switch (sel) {
        case 0:
            break;
        case 9:
        case 10:
            *(int *)(data + 0x1C8) = 1;
            ((unsigned char *)data)[0x67] = 0xFA;
            break;
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8: {
            char *p20;
            float k = D_0015ED6C;
            float a = *(float *)&D_L15_00161B08 * k;
            float b = *(float *)&D_L15_00161B04 * k;
            float r;
            *(int *)(data + 0x1C8) = 1;
            *(float *)(data + 0x88) = a;
            *(float *)(data + 0x8C) = b;
            func_L03_00250A78((float *)(data + 0x70), 4.0f, 1.0f);
            *(float *)(data + 0xC0) = 12.0f;
            *(float *)(data + 0xC4) = 16.0f;
            p20 = *(char **)(other + 0x20);
            r = func_L00_001FF8B0(*(float *)(moby + 0x10) - *(float *)(p20 + 0x10),
                                  *(float *)(moby + 0x14) - *(float *)(p20 + 0x14));
            *(u128_c2d0fa8 *)&vA = *(u128_c2d0fa8 *)(other + 0x10);
            f48 = r;
            func_L00_0025AB48(&vA, &f48, data + 0x88, data + 0x8C);
            func_L00_0025C558(f48, moby, data + 0x70, 8, 1, 0);
            func_L00_0028EF68_2E9358(0, 0, moby, 0x79);
            moby[0x20] = 9;
            ((unsigned char *)data)[0x67] = 0x78;
            break;
        }
        case 1:
        case 2:
            *(u128_c2d0fa8 *)&vA = 0;
            *(unsigned short *)(moby + 0x34) &= 0xEFFF;
            func_L00_0024F7C8(moby, 7, &vB);
            func_L00_0025E450(moby, &vA, &vB, 0.0f, 0.0f, 20, 6, 0x20, 16.0f, 8.0f, 9.0f, 4.0f, -1, 32.0f, 1, 0x10, -1, 0);
            func_L00_0028EF68_2E9358(0, 0, moby, 0x79);
            moby[0x20] = 10;
            ((unsigned char *)data)[0x67] = 0xF0;
            func_L00_00257470(moby, 0x200, -1);
            break;
        case 11:
            break;
        }
        func_L00_0025D458(moby, (short *)(data + 0x60));
    }
    if (other != 0)
        *(int *)(other + 0x34) = 0;

    ((unsigned char *)moby)[0xA4] = 0xFF;
    func_L00_0025D538(moby, data + 0x60);
    *(float *)(moby + 0x2C) = *(float *)(*(char **)(moby + 0x24) + 0x24) * *(float *)&D_L15_00161B30;
    *(int *)(data + 0x164) = 0;

    if ((unsigned char)moby[0x20] == 1 || *(int *)(data + 0x1D0) == -1) {
        if ((unsigned char)moby[0x20] == 1)
            *(float *)(data + 0x1B8) = *(float *)(data + 0x174);
        else
            *(float *)(data + 0x1B8) = *(float *)(data + 0x178);
        func_L00_00260D30_t(moby, (float *)(data + 0x120), *(float *)(data + 0x1B8));
    } else {
        UPath_c2d0fa8 *p;
        *(float *)(data + 0x1B8) = *(float *)(data + 0x178);
        p = D_L15_001B0AB0[*(int *)(data + 0x1D0)];
        func_L00_0025FF38(*(float *)(data + 0x1B8), moby, data + 0x120, 0, 0, p->pt, p->n);
    }

    if (*(float *)(data + 0x1B8) < func_001F9B48(moby + 0x10, data + 0x120))
        *(int *)(data + 0x164) = 2;
    if (*(int *)(data + 0x160) == 0)
        *(int *)(data + 0x160) = *(int *)(D_0013E533 + 0x2E9D);
}
