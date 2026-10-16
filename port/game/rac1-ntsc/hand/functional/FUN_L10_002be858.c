/* Carried from the PAL decompilation's src/overlays/l10_orxon/vendor_00296BD8.c (matched C there) (games/rac1) through the version
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
extern int func_001F9740(int *);
extern int func_0022DA68(int, int, int);
extern int func_001F9688(int);
extern float D_0015ED70 MACRO_ADDR;
extern float func_00213308(void);
extern int func_L00_00257B90(int, int);
extern char *D_L10_00160058_00299818 __asm__("D_L10_0015FFD8") MACRO_ADDR;
extern char D_0013E533[];
extern void func_L00_00216F90(void *, void *, int, int);
typedef unsigned int Q_2BFBB8_c2be858 __attribute__((mode(TI), aligned(16)));
#define D_0013E633_2BFBB8 ((char *)D_0013E533)
extern char D_L10_00167100[] ;
#define D_L10_00167180_2BFBB8 D_L10_00167100
extern int D_L10_0015F5CC MACRO_ADDR;
#define D_L10_0015F6B0_2BFBB8 (*(int *)&D_L10_0015F5CC)
extern short D_L10_001618A0;
#define D_L10_00161920_2BFBB8 (*(int *)&D_L10_001618A0)
extern short D_L10_001618A4;
#define D_L10_00161924_2BFBB8 (*(int *)&D_L10_001618A4)
extern int D_L10_001618A8 MACRO_ADDR;
#define D_L10_00161928_2BFBB8 D_L10_001618A8
extern float D_0015ED6C MACRO_ADDR;
#define D_0015EE6C_2BFBB8 (*(float *)&D_0015ED6C)
#define D_0015EE70_2BFBB8 (*(float *)&D_0015ED70)
#define func_001F9850_2BFBB8 ((int (*)(int))func_001F9688)
extern float func_001F9B80(void *, void *);
#define func_001F9D48_2BFBB8 ((float (*)(void *, void *))func_001F9B80)
extern float func_002132A8(float, float);
#define func_002140F8_2BFBB8 ((float (*)(float, float))func_002132A8)
#define func_00214158_2BFBB8 ((float (*)(void))func_00213308)
extern int func_00213260(int);
#define func_002140B0_2BFBB8 ((int (*)(int))func_00213260)
#define func_L00_00258BC8_2BFBB8 ((int (*)(int, int))func_L00_00257B90)
extern float func_001F9DC8(float);
#define func_001F9F90_2BFBB8 ((float (*)(float))func_001F9DC8)
extern float func_001F9DE0(float);
#define func_001F9FA8_2BFBB8 ((float (*)(float))func_001F9DE0)
extern void func_L00_001FF290(void *, void *, void *);
#define func_L00_001FF240_2BFBB8 ((void (*)(void *, void *, void *))func_L00_001FF290)
extern void func_L00_001FF500(void *, void *, float);
#define func_L00_001FF4B0_2BFBB8 ((void (*)(void *, void *, float))func_L00_001FF500)
extern void * func_L00_00272060(void *, int, float, float, int, int, int, void *, float);
#define func_L00_00272F00_2BFBB8 func_L00_00272060
extern void * func_L00_002715E8(void *, int, int, float, float);
#define func_L00_00272488_2BFBB8 func_L00_002715E8
#define func_0022ED80_2BFBB8 ((int (*)(int, int, int))func_0022DA68)
#define func_L00_00217718_2BFBB8 func_L00_00216F90
extern float func_001FA6C0(int);
#define func_001FA888_2BFBB8 ((float (*)(int))func_001FA6C0)
extern int func_001FA6D0(float);
#define func_001FA898_2BFBB8 ((int (*)(float))func_001FA6D0)
extern unsigned int func_001FA6E0(unsigned int, unsigned int, float);
#define func_001FA8A8_2BFBB8 func_001FA6E0
#define func_001F9908_2BFBB8 ((int (*)(int *))func_001F9740)
extern void func_L00_002E8590(float, float);
#define func_L00_002E9A40_2BFBB8 func_L00_002E8590
extern void func_L00_002E8680(void);
#define func_L00_002E9B30_2BFBB8 func_L00_002E8680
extern void func_L00_002E8648(void);
#define func_L00_002E9AF8_2BFBB8 func_L00_002E8648
extern void func_L00_002E8568(int);
#define func_L00_002E9A18_2BFBB8 func_L00_002E8568



void FUN_L10_002be858(char *m)
{
    int *data = *(int **)(m + 0x78);
    float position[4];
    float velocity[4];
    float offset[4];
    float angle;
    float *op;
    int i;
    char *g;
    int life;
    if (*(unsigned char *)(m + 0x20) == 0) {
        D_L10_00161928_2BFBB8 = 0;
        D_L10_00161924_2BFBB8 = func_001F9850_2BFBB8(D_L10_00161924_2BFBB8);
        D_L10_00161920_2BFBB8 = func_001F9850_2BFBB8(D_L10_00161920_2BFBB8);
        *(unsigned char *)(m + 0x20) = 1;
    }
    g = D_0013E633_2BFBB8 + 0xE1D;
    if ((unsigned char)g[0x20A4] == 1 && *data != -1 &&
        func_001F9D48_2BFBB8(m + 0x10, g + 0x80) < 0.5f) {
        op = offset;
        i = 9;
        do {
            float size, size_scaled;
            int red, green, blue, color, spin, gk;
            i--;
            size = func_002140F8_2BFBB8(0.1f, 1.0f);
            red = func_L00_00258BC8_2BFBB8(0x40, 0xff);
            green = func_L00_00258BC8_2BFBB8(0x40, 0xff);
            blue = func_L00_00258BC8_2BFBB8(0x40, 0xff);
            velocity[0] = func_001F9F90_2BFBB8(func_00214158_2BFBB8()) * func_002140F8_2BFBB8(0.0f, 0.5f);
            velocity[1] = func_001F9FA8_2BFBB8(func_00214158_2BFBB8()) * func_002140F8_2BFBB8(0.0f, 0.5f);
            velocity[2] = 0.0f;
            velocity[2] += func_002140F8_2BFBB8(0.0f, 0.5f);
            func_L00_001FF240_2BFBB8(op, velocity, D_0013E633_2BFBB8 + 0xE9D);
            func_L00_001FF4B0_2BFBB8(position, position, func_002140F8_2BFBB8(0.0f, D_0015EE6C_2BFBB8));
            position[2] = func_002140F8_2BFBB8(D_0015EE6C_2BFBB8 * 2.5f, D_0015EE6C_2BFBB8 * 5.0f);
            life = func_001F9850_2BFBB8(func_L00_00258BC8_2BFBB8(15, 30));
            gk = (green << 8) | 0x7f000000;
            color = (blue << 16) | gk | red;
            size_scaled = size * 0.07f;
            size *= 0.7f;
            spin = func_002140B0_2BFBB8(2) ? 1 : -1;
            func_L00_00272F00_2BFBB8(velocity, life, size_scaled, size, color, 0, spin, position, D_0015EE70_2BFBB8 * 15.0f);
        } while (i >= 0);
        func_0022ED80_2BFBB8(0, 0, (int)m);
        {
            int di = *data << 8;
            char *effect = D_L10_00160058_00299818 + di;
            func_L00_00217718_2BFBB8(effect + 0x10, effect + 0x40, 0x43, 0);
        }
        {
            char *b180 = D_L10_00167180_2BFBB8;
            char *effect = *(char **)(b180 + 0x180);
            D_L10_00161928_2BFBB8 = D_L10_00161920_2BFBB8;
            if (*(short *)(effect + 0x86) == 0) {
                char *e = *(char **)(b180 + 0x180);
                short *timer = (short *)(*(char **)(e + 0x70) + 0x20);
                *timer = func_001F9850_2BFBB8(0x5a);
                qcopy(e + 0x40, e);
            }
        }
        i = 9;
        do {
            float size, size_scaled;
            int red, green, blue, color, spin, gk;
            *(Q_2BFBB8_c2be858 *)velocity = 0;
            velocity[0] = func_002140F8_2BFBB8(-1.0f, 1.0f);
            i--;
            velocity[1] = func_002140F8_2BFBB8(-1.0f, 1.0f);
            *(Q_2BFBB8_c2be858 *)position = *(Q_2BFBB8_c2be858 *)velocity;
            size = func_002140F8_2BFBB8(0.1f, 1.0f);
            red = func_L00_00258BC8_2BFBB8(0x40, 0xff);
            green = func_L00_00258BC8_2BFBB8(0x40, 0xff);
            blue = func_L00_00258BC8_2BFBB8(0x40, 0xff);
            velocity[0] = func_001F9F90_2BFBB8(func_00214158_2BFBB8()) * func_002140F8_2BFBB8(0.0f, 0.5f);
            velocity[1] = func_001F9FA8_2BFBB8(func_00214158_2BFBB8()) * func_002140F8_2BFBB8(0.0f, 0.5f);
            velocity[2] = 0.0f;
            velocity[2] += func_002140F8_2BFBB8(0.0f, 0.5f);
            func_L00_001FF240_2BFBB8(op, velocity, D_L10_00160058_00299818 + (*data << 8) + 0x10);
            func_L00_001FF4B0_2BFBB8(position, position, func_002140F8_2BFBB8(0.0f, D_0015EE6C_2BFBB8));
            position[2] = func_002140F8_2BFBB8(D_0015EE6C_2BFBB8 * 2.5f, D_0015EE6C_2BFBB8 * 5.0f);
            life = func_001F9850_2BFBB8(func_L00_00258BC8_2BFBB8(15, 30));
            gk = (green << 8) | 0x7f000000;
            color = (blue << 16) | gk | red;
            size_scaled = size * 0.07f;
            size *= 0.7f;
            spin = func_002140B0_2BFBB8(2) ? 1 : -1;
            func_L00_00272F00_2BFBB8(velocity, life, size_scaled, size, color, 0, spin, position, D_0015EE70_2BFBB8 * 15.0f);
        } while (i >= 0);
    }
    if (D_L10_0015F6B0_2BFBB8 % func_001F9850_2BFBB8(0x3c) == 0 && *data != -1) {
        for (i = 0; i < func_001FA898_2BFBB8(16.0f); i++) {
            char *effect;
            *(Q_2BFBB8_c2be858 *)position = *(Q_2BFBB8_c2be858 *)(m + 0x10);
            position[2] += func_001FA888_2BFBB8(i) * 0.1f * 0.0625f + 1.8f;
            effect = (char *)func_L00_00272488_2BFBB8(position, 0, 0x1eff7f40, 0.0f, D_0015EE6C_2BFBB8 * 315000.0f);
            if (effect != 0) {
                float *vel = (float *)(effect + 0x20);
                float z = D_0015EE6C_2BFBB8 * -1.5f;
                vel[0] = 0.0f;
                vel[1] = 0.0f;
                vel[2] = z;
            }
        }
    }
    {
        float phase = (float)(D_L10_0015F6B0_2BFBB8 % 150) / 150.0f;
        phase += phase;
        angle = phase * 3.1415927f;
    }
    if (3.1415927f < angle) angle -= 6.2831855f;
    *(unsigned int *)(m + 0x90) = func_001FA8A8_2BFBB8(0x80464646, 0x80828282, (func_001F9FA8_2BFBB8(angle) + 1.0f) * 0.5f);
    func_001F9908_2BFBB8(&D_L10_00161928_2BFBB8);
    if (D_L10_00161928_2BFBB8 != 0) {
        if (D_L10_00161924_2BFBB8 < D_L10_00161928_2BFBB8) angle = 0.002f;
        else angle = func_001FA888_2BFBB8(D_L10_00161928_2BFBB8) / func_001FA888_2BFBB8(D_L10_00161924_2BFBB8) * 0.007999999f + 0.002f;
        func_L00_002E9A40_2BFBB8(angle, 0.2f);
        func_L00_002E9B30_2BFBB8();
        func_L00_002E9AF8_2BFBB8();
        func_L00_002E9A18_2BFBB8(0);
    }
}
