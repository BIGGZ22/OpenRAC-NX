/* Carried from the PAL decompilation's src/overlays/l06_blarg/vendor_002FE5D0.c (matched C there) (games/rac1) through the version
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
extern void func_001FA378(void *, void *, void *);
extern void func_001F9A68(void *, void *, float);
extern int func_001F9688(int);
extern void func_001F4600(int, char *);
extern float func_002132A8(float, float);
extern void func_L00_001FF500(void *, void *, float);
extern void func_001F9A10(void *, void *, void *);
extern char D_0013E533[] NOT_SDA;
extern float func_001F9B48(void *, void *);
extern char D_L06_00167540[];
extern float D_0015ED6C  MACRO_ADDR;
extern void func_L00_00257D78(void *, float, float);
extern int func_001FA6E0(int, int, float);
extern int func_001FA6D0(float);
extern char *D_L06_0015FFD8 MACRO_ADDR;
extern void func_0022DA68(int, int, void *);
extern int func_00213260(int);
extern void func_L00_0028D918(int);
extern int func_L00_0028D8C0(void *, int);
extern int func_0022ED80_305E38(int, int, void *) __asm__("func_0022DA68");
extern void func_L06_00217300(void *);
extern void func_001FA050(float *, float *);
extern void func_001F9FF8(float *, float);
extern float func_L00_00257C48(float lo, float hi);
extern void func_L00_00273F80_r(void *, void *, int, float, int, int, int) __asm__("func_L00_002730E0");
extern int D_L06_00162238 MACRO_ADDR;
extern int D_L06_00162234 MACRO_ADDR;
extern short D_L06_001621F8;
extern short D_L06_00162208;
extern short D_L06_001621FC;
extern float D_L06_00162200 SDATA(D_L06_00162200);
extern short D_L06_00162204;
extern short D_L06_00162210;
extern short D_L06_00162214;
extern short D_L06_00162218;
extern short D_L06_0016221C;
extern short D_L06_00162220;
extern short D_L06_00162224;
extern short D_L06_00162228;
extern short D_L06_0016222C;
extern short D_L06_00162230;
void func_L06_00305170(char *m);
void func_L06_00305000(char *m);
extern float D_L06_001FE890[];
typedef struct {
    float a[4][30][4];
    float b[4][30][4];
    int timer[4];
    int f10;
    int f14;
    int f18;
} Data_305E38_c3049f8;



/* Level 06 update for moby class 1123: waits for its partner, animates four ribbons, then bursts into particles. */
void FUN_L06_003049f8(char *moby)
{
    Data_305E38_c3049f8 *d = *(Data_305E38_c3049f8 **)(moby + 0x78);
    union {
        float tmp[30][4];
        struct {
            float a[16];
            float b[16];
            float pos[4];
            float vel[4];
        } m;
    } u;

    switch ((unsigned char)moby[0x20]) {
    case 0:
        *(float *)(moby + 0x2C) = *(float *)(*(char **)(moby + 0x24) + 0x24) * *(float *)&D_L06_00162210;
        D_L06_00162238 = 0x16;
        D_L06_00162234 = 0x2D;
        d->timer[1] = 0xB;
        d->timer[2] = 0x17;
        d->timer[3] = 0x22;
        d->timer[0] = 0;
        d->f18 = -1;
        if (d->f10 != -1) {
            moby[0x20] = 1;
            *(int *)(moby + 0x94) = 0;
        } else {
            moby[0x20] = 2;
        }
        break;
    case 1: {
        char *e = D_L06_0015FFD8 + (d->f10 << 8);
        if (*(short *)(e + 0xA6) == 0x463 && (unsigned char)e[0x20] < 3) {
            return;
        }
        moby[0x20] = 2;
        *(int *)(moby + 0x94) = *(int *)(*(char **)(moby + 0x24) + 0x10);
        break;
    }
    case 2: {
        float f20;
        float f21;
        float v;
        int i;
        int j;
        int k;
        int l;

        if (func_L00_0028D8C0(moby, d->f18) == 0) {
            d->f18 = func_0022ED80_305E38(0, 4, moby);
        }
        func_L06_00305000(moby);
        v = *(float *)&D_L06_001621F8 + *(float *)&D_L06_001621FC * D_0015ED6C;
        *(float *)&D_L06_001621F8 = v;
        if (1.0f < v) {
            *(float *)&D_L06_001621F8 = v - 1.0f;
        }
        f21 = *(float *)&D_L06_00162204 / 30.0f;
        f20 = -(*(float *)&D_L06_00162204 * 0.5f);
        for (i = 0; i < 4; i++) {
            if (d->timer[i] == 0) {
                for (j = 0; j < 30; j++) {
                    func_L06_00217300(d->a[i][j]);
                    d->a[i][j][0] = f20;
                    func_L00_00257D78(u.tmp[j], 0.0f, D_L06_00162200 * D_0015ED6C);
                    f20 += f21;
                }
                for (k = 1; k < 29; k++) {
                    func_001F9A10(d->b[i][k], u.tmp[k - 1], u.tmp[k]);
                    func_001F9A10(d->b[i][k], d->b[i][k], u.tmp[k + 1]);
                    func_001F9A68(d->b[i][k], d->b[i][k], 0.333f);
                }
                d->timer[i] = D_L06_00162234;
            } else {
                for (l = 1; l < 29; l++) {
                    func_001F9A10(d->a[i][l], d->a[i][l], d->b[i][l]);
                }
                d->timer[i]--;
            }
        }
        if (func_001F9B48(moby + 0x10, D_L06_00167540) < 48.0f) {
            func_001F4600((int)func_L06_00305170, moby);
        }
        if (d->f14 >= 0 && *(unsigned char *)(D_L06_0015FFD8 + 0xBC + (d->f14 << 8)) != 0) {
            int h;
            *(int *)(moby + 0x94) = 0;
            moby[0x20] = 3;
            h = d->f18;
            if (h != -1) {
                char *e = D_0013E533 + 0x1D + h * 0x70;
                if (*(char **)(e + 0x88) == moby && *(unsigned char *)(e + 0x74) != 0) {
                    func_L00_0028D918(h);
                }
            }
            d->f18 = -1;
            func_0022DA68(1, 0, moby);
        }
        break;
    }
    case 3: {
        int i;
        int j;

        func_001FA050(u.m.a, (float *)(moby + 0x40));
        func_001F9FF8(u.m.b, *(float *)&D_L06_00162208);
        func_001FA378(u.m.a, u.m.a, u.m.b);
        for (i = 0; i < 4; i++) {
            func_001FA050(u.m.b, &D_L06_001FE890[i * 4]);
            func_001FA378(u.m.b, u.m.a, u.m.b);
            func_001F9A10(u.m.b + 12, u.m.b + 12, moby + 0x10);
            u.m.b[14] += (float)i * 1.7f + 0.6f;
            u.m.b[15] = 1.0f;
            for (j = 0; j < 50; j++) {
                int color;
                float scale;
                int s;
                unsigned char life;
                func_L00_001FF500(u.m.pos, u.m.b, func_L00_00257C48(0.0f, *(float *)&D_L06_00162204 * 0.75f));
                func_001F9A10(u.m.pos, u.m.pos, u.m.b + 12);
                color = func_001FA6E0(*(int *)&D_L06_00162214, *(int *)&D_L06_00162218, func_002132A8(0.0f, 1.0f));
                scale = func_002132A8(*(float *)&D_L06_0016221C, *(float *)&D_L06_00162220);
                s = func_001FA6D0(func_002132A8((float)*(int *)&D_L06_00162224, (float)*(int *)&D_L06_00162228));
                func_L00_00257D78(u.m.vel, *(float *)&D_L06_0016222C * D_0015ED6C, *(float *)&D_L06_00162230 * D_0015ED6C);
                life = func_001F9688(s);
                func_L00_00273F80_r(u.m.pos, u.m.vel, color, scale, life, (unsigned char)func_00213260(0xFF), 0);
            }
        }
        moby[0x20] = 4;
        break;
    }
    }
}
