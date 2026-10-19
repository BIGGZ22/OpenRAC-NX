/* Carried from the PAL decompilation's nonmatching/functional/func_L05_002D28D0.c (games/rac1) through the version
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
/* functional: medium. Rilgar swimming-creature moby update (state byte at moby+0x20): 0 picks a path and starts it, 1 follows the path points (steering, speed from water level, hero close-by check), 2 waits and, when the hero is in the water, dives at him, 3 rises/falls with splash effects and carries the hero. Written from the asm with the NTSC decomp draft as a guide; the sda float at gp-0x58B0 is addressed as D_L05_001612E8+0xA8 (unchecked: that gp offset is derived from another level's gp). */
extern int func_002140B0_2D28D0(int) __asm__("func_00213260");
extern int func_001F9850_2D28D0(int) __asm__("func_001F9688");
extern void func_0020D678_2D28D0(char *) __asm__("func_0020C828");
extern void func_00213DE0_2D28D0(char *, int, int, int) __asm__("func_00212F90");
extern float func_L00_001FF860_2D28D0(float, float) __asm__("func_L00_001FF8B0");
extern float func_L00_0025CE58_2D28D0(void *, void *, float, float, float, float) __asm__("func_L00_0025BE00");
extern void func_L00_001FF4B0_2D28D0(void *, void *, float) __asm__("func_L00_001FF500");
extern void func_001F9BD8_2D28D0(void *, void *, void *) __asm__("func_001F9A10");
extern void func_001F9BF0_2D28D0(void *, void *, void *) __asm__("func_001F9A28");
extern float func_001F9D10_2D28D0(void *, void *) __asm__("func_001F9B48");
extern int func_001F9908_2D28D0(void *) __asm__("func_001F9740");
extern float func_001F9B88_2D28D0(float) __asm__("func_001F99C0");
extern float func_001F9D48_2D28D0(void *, void *) __asm__("func_001F9B80");
extern float func_00214D88_2D28D0(void *, void *, float, float, float, float) __asm__("func_00213F38");
extern float func_001FA748_2D28D0(float, float) __asm__("func_001FA580");
extern void func_001FA1F8_2D28D0(void *, void *) __asm__("func_001FA030");
extern int func_L00_0025A778_2D28D0(void *, void *, int) __asm__("func_L00_00259740");
extern void func_L00_00217718_2D28D0(void *, void *, int, int) __asm__("func_L00_00216F90");
extern void func_0022ED80_2D28D0(int, int, char *) __asm__("func_0022DA68");
extern void func_L00_00211908_2D28D0(void) __asm__("func_L00_00211250");
extern void func_L00_002EBF50_2D28D0(void *, void *, int, int, int) __asm__("func_L00_002EAAA0");
extern void func_L05_002D3078_2D28D0(char *) __asm__("func_L05_002D1E30");
extern void func_L05_002D3220_2D28D0(char *, void *) __asm__("func_L05_002D1FD8");
extern char *D_L05_001B0CB0_2D28D0[] __asm__("D_L05_001B0930");
extern char D_L05_001613A8_2D28D0[] __asm__("D_L05_001612E8");
extern char D_L05_00167180_2D28D0[] __asm__("D_L05_00167080");
extern char D_0013F450_2D28D0[] __asm__("D_0013F350");
extern float D_0015EE6C_2D28D0 __asm__("D_0015ED6C");
extern float D_0015EE70_2D28D0 __asm__("D_0015ED70");



void FUN_L05_002d1688(char *moby) {
    char *data = *(char **)(moby + 0x78);
    char *hero = D_0013F450_2D28D0;
    int state = *(unsigned char *)(moby + 0x20);
    char *path;
    float v[4] __attribute__((aligned(16)));
    float w[4] __attribute__((aligned(16)));
    float z[4] __attribute__((aligned(16)));
    float speed, height, rise;
    int index, duration;
    /* NTSC-U reads gp-0x5878 = 0x161388 (0x2D179C, ...): PAL's 0x161450 is 0xC8 lower here, not 0xC0 as the base. */
    float lvl = *(float *)(D_L05_001613A8_2D28D0 + 0xA0);

    if (state == 1) {
        path = D_L05_001B0CB0_2D28D0[*(int *)(data + 0x60)];
        qcopy(v, path + *(int *)(data + 0x68) * 16 + 0x10);
        height = func_L00_001FF860_2D28D0(v[0] - *(float *)(moby + 0x10), v[1] - *(float *)(moby + 0x14));
        func_L00_0025CE58_2D28D0(moby + 0x48, data + 0x70, height, D_0015EE70_2D28D0 * 3.1415927f,
                                 D_0015EE70_2D28D0 * 3.1415927f, D_0015EE6C_2D28D0 * 3.1415927f);
        func_L00_001FF4B0_2D28D0(w, moby + 0xC0, D_0015EE6C_2D28D0 * 3.0f);
        func_001F9BD8_2D28D0(moby + 0x10, moby + 0x10, w);
        height = func_001F9D10_2D28D0(moby + 0x10, v);
        if (height < 3.0f || func_001F9908_2D28D0(data + 0x6C) != 0) {
            path = D_L05_001B0CB0_2D28D0[*(int *)(data + 0x60)];
            index = func_002140B0_2D28D0(*(int *)path);
            path = D_L05_001B0CB0_2D28D0[*(int *)(data + 0x60)];
            *(int *)(data + 0x68) = (*(int *)(data + 0x68) + index) % *(int *)path;
            *(int *)(data + 0x6C) = func_001F9850_2D28D0(600);
        }
        if ((*(int *)(hero + 0x208C) == 0x12 || *(int *)(hero + 0x2084) == 0x12) &&
            func_001F9D48_2D28D0(moby + 0x10, hero + 0x80) < 8.0f) {
            speed = (*(float *)D_L05_001613A8_2D28D0 - lvl) - 2.0f;
        } else {
            speed = (*(float *)D_L05_001613A8_2D28D0 - lvl) - 0.5f;
            func_L05_002D3078_2D28D0(moby);
        }
        func_00214D88_2D28D0(moby + 0x18, data + 0x78, speed, D_0015EE70_2D28D0 + D_0015EE70_2D28D0,
                             D_0015EE70_2D28D0 + D_0015EE70_2D28D0, D_0015EE6C_2D28D0 + D_0015EE6C_2D28D0);
        return;
    }
    if (state == 0) {
        if (*(int *)(data + 0x64) != 0) {
            *(unsigned char *)(moby + 0x20) = 2;
            *(unsigned short *)(moby + 0x34) = *(unsigned short *)(moby + 0x34) | 1;
            *(unsigned char *)(moby + 0x30) = 0xFF;
            *(float *)(data + 0x74) = *(float *)(moby + 0x18);
            duration = 30;
            *(int *)(data + 0x7C) = func_001F9850_2D28D0(duration);
            return;
        }
        index = *(int *)(data + 0x60);
        if (index == -1) {
            func_0020D678_2D28D0(moby);
            return;
        }
        path = D_L05_001B0CB0_2D28D0[index];
        *(int *)(data + 0x68) = func_002140B0_2D28D0(*(int *)path);
        *(int *)(data + 0x6C) = func_001F9850_2D28D0(600);
        if (*(unsigned char *)(moby + 0x53) != 1) {
            func_00213DE0_2D28D0(moby, 1, 0, func_001F9850_2D28D0(10));
        }
        path = D_L05_001B0CB0_2D28D0[*(int *)(data + 0x60)];
        height = *(float *)(path + 0x18) - lvl;
        *(float *)(moby + 0x18) = height;
        *(float *)(data + 0x80) = height;
        *(unsigned char *)(moby + 0x20) = 1;
        return;
    }
    if (state == 2) {
        func_001F9908_2D28D0(data + 0x7C);
        height = func_001F9B88_2D28D0(*(float *)(hero + 0x2F0) - *(float *)(data + 0x74));
        if (height < 0.33f && *(int *)(hero + 0x208C) == 0x12) {
            if (*(int *)(data + 0x7C) != 0) return;
            *(float *)(moby + 0x44) = -1.5533431f;
            *(float *)(moby + 0x48) = func_001FA748_2D28D0(*(float *)(D_L05_00167180_2D28D0 + 0x158), 1.5707964f);
            func_001FA1F8_2D28D0(moby + 0xC0, moby + 0x40);
            qcopy(moby + 0x10, hero + 0x80);
            func_L00_001FF4B0_2D28D0(v, moby + 0xE0, lvl);
            func_001F9BF0_2D28D0(moby + 0x10, moby + 0x10, v);
            *(float *)(moby + 0x18) = *(float *)(moby + 0x18) - 2.0f;
            *(float *)(data + 0x78) = D_0015EE6C_2D28D0 * 10.0f;
            *(unsigned short *)(moby + 0x34) = *(unsigned short *)(moby + 0x34) & 0xFFFE;
            if (*(unsigned char *)(moby + 0x53) != 2) {
                func_00213DE0_2D28D0(moby, 2, 0, func_001F9850_2D28D0(5));
            }
            *(unsigned char *)(moby + 0xBC) = 0;
            *(unsigned char *)(moby + 0x20) = 3;
            func_L00_002EBF50_2D28D0(D_L05_00167180_2D28D0 + 0x140, D_L05_00167180_2D28D0 + 0x150, 0, 0, 0);
            return;
        }
        if (*(int *)(hero + 0x2084) == 0x12) return;
        path = D_L05_001B0CB0_2D28D0[*(int *)(data + 0x60)];
        if (func_L00_0025A778_2D28D0(hero + 0x80, path + 0x10, *(int *)path) == 0)
            duration = 180;
        else
            duration = 600;
        *(int *)(data + 0x7C) = func_001F9850_2D28D0(duration);
        return;
    }
    if (state == 3) {
        if (*(float *)(hero + 0x2F0) < *(float *)(moby + 0x18) || *(float *)(data + 0x78) < 0.0f) {
            if (*(int *)(hero + 0x2084) != 0x72) {
                qcopy(v, hero + 0x80);
                v[2] = *(float *)(hero + 0x2F0);
                func_L05_002D3220_2D28D0(moby, v);
                func_L00_00217718_2D28D0(hero + 0x80, hero + 0x90, 0x72, 0);
            } else {
                qcopy(hero + 0x80, moby + 0x10);
            }
            *(unsigned char *)(hero + 0x20A5) = 1;
            *(float *)(hero + 0x88) = *(float *)(hero + 0x88) + 20.0f;
            rise = *(float *)(data + 0x78);
        } else {
            rise = *(float *)(data + 0x78);
        }
        *(float *)(moby + 0x18) = *(float *)(moby + 0x18) + rise;
        speed = *(float *)(data + 0x78) - D_0015EE70_2D28D0 * 10.0f;
        *(float *)(data + 0x78) = speed;
        if (speed < D_0015EE6C_2D28D0 + D_0015EE6C_2D28D0) {
            func_L00_001FF4B0_2D28D0(v, moby + 0xE0, lvl);
            func_L00_0025CE58_2D28D0(moby + 0x44, data + 0x70, 1.5533431f, D_0015EE70_2D28D0 * 12.566371f,
                                     D_0015EE70_2D28D0 * 12.566371f, D_0015EE6C_2D28D0 * 25.132742f);
            func_001FA1F8_2D28D0(moby + 0xC0, moby + 0x40);
            func_L00_001FF4B0_2D28D0(w, moby + 0xE0, lvl);
            func_001F9BF0_2D28D0(z, w, v);
            func_001F9BF0_2D28D0(moby + 0x10, moby + 0x10, z);
        }
        if (*(float *)(data + 0x78) < 0.0f) {
            if (*(float *)(moby + 0x18) < *(float *)(hero + 0x2F0) && *(unsigned char *)(moby + 0xBC) == 0) {
                *(unsigned char *)(moby + 0xBC) = 1;
                func_0022ED80_2D28D0(0, 0, moby);
                func_L00_001FF4B0_2D28D0(v, moby + 0xE0, lvl);
                func_001F9BD8_2D28D0(v, v, moby + 0x10);
                v[2] = *(float *)(hero + 0x2F0);
                func_L05_002D3220_2D28D0(moby, v);
            }
        }
        if (*(float *)(data + 0x78) < 0.0f && *(float *)(moby + 0x18) < *(float *)(data + 0x74) - 8.0f) {
            func_L00_00211908_2D28D0();
        }
        if ((*(unsigned char *)(moby + 0x70) & 2) != 0 && *(unsigned char *)(moby + 0x53) != 1) {
            func_00213DE0_2D28D0(moby, 1, 0, func_001F9850_2D28D0(10));
        }
    }
}
