/* Carried from the PAL decompilation's nonmatching/functional/func_L01_0030F318.c (games/rac1/pal) through the version
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
/* functional C for FUN_L01_0030df40: MouseCritterUpdate (class 1818 / 0x71A update).
 * Moby state machine for ambient critter on Novalis / shared overlays.
 * Keeps moby active, steps physics / terrain, runs state machine and animations.
 */
extern char *D_L01_00160058_30F318 __asm__("D_L01_0015FFD8");
extern char D_0013E633_30F318[] __asm__("D_0013E533");
extern unsigned char D_0015EEB0_30F318[] __asm__("D_0015EDB0");

extern float func_L00_001FF860_30F318(float, float) __asm__("func_L00_001FF8B0");
extern void func_0020D960_30F318(void *, int, void *) __asm__("func_0020CB10");
extern void func_0020D9D8_30F318(void *, void *) __asm__("func_0020CB88");
extern void func_L01_0030F2A8_30F318(void *) __asm__("func_L01_0030DED0");
extern int func_001F9850_30F318(int) __asm__("func_001F9688");
extern void func_00213DE0_30F318(void *, int, int, int) __asm__("func_00212F90");
extern void func_L00_0025B040_30F318(void *, float) __asm__("func_L00_00259FE8");
extern void func_L01_0030F178_30F318(void *, void *) __asm__("func_L01_0030DDA0");
extern void func_0020D678_30F318(void *) __asm__("func_0020C828");

void FUN_L01_0030df40(char *moby) {
    if (moby == 0) {
        return;
    }
    char *pvars = *(char **)(moby + 0x78);
    if (pvars == 0) {
        return;
    }
    
    int linked_idx = *(int *)(pvars + 0x48);
    if (linked_idx != -1) {
        unsigned char st = (unsigned char)moby[0x20];
        float dx, dy;
        if (st >= 5) {
            char *linked = D_L01_00160058_30F318 + (linked_idx << 8);
            dx = *(float *)(moby + 0x10) - *(float *)(linked + 0x10);
            dy = *(float *)(moby + 0x14) - *(float *)(linked + 0x14);
        } else {
            char *hero = D_0013E633_30F318 + 0xE1D;
            char *linked = *(char **)(hero + 0x5893);
            if (linked != 0) {
                linked = linked + (linked_idx << 8);
                dx = *(float *)(hero + 0x80) - *(float *)(linked + 0x10);
                dy = *(float *)(hero + 0x84) - *(float *)(linked + 0x14);
            } else {
                dx = 0.0f;
                dy = 0.0f;
            }
        }
        float angle = func_L00_001FF860_30F318(dx, dy);
        char *linked_moby = D_L01_00160058_30F318 + (linked_idx << 8);
        char *linked_pvars = *(char **)(linked_moby + 0x78);
        if (linked_pvars != 0) {
            *(float *)linked_pvars = angle;
        }
        *(int *)(linked_moby + 0x94) = 0;
    }
    
    unsigned char state = (unsigned char)moby[0x20];
    if (state >= 3) {
        if (D_0015EEB0_30F318[0] != 0) {
            if ((unsigned char)pvars[0x91] == 0) {
                func_0020D960_30F318(moby, 5, pvars + 0x90);
            }
            *(float *)(pvars + 0xB0) = 2.1f;
            *(float *)(pvars + 0xB4) = 2.1f;
            *(float *)(pvars + 0xB8) = 2.1f;
        } else {
            if ((unsigned char)pvars[0x91] != 0) {
                func_0020D9D8_30F318(moby, pvars + 0x90);
            }
        }
        func_L01_0030F2A8_30F318(moby);
        state = (unsigned char)moby[0x20];
    }
    
    switch (state) {
    case 0:
        memset(pvars, 0, 0x20);
        *(short *)(pvars + 0x50) = 0;
        *(short *)(pvars + 0x52) = 0;
        if ((unsigned char)moby[0x53] != 0) {
            func_00213DE0_30F318(moby, 0, 0, func_001F9850_30F318(10));
        }
        moby[0x30] = (char)0xFF;
        *(short *)(moby + 0x32) = 0xFF;
        moby[0x20] = 1;
        break;
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    default:
        break;
    }
    
    if ((unsigned char)moby[0x20] >= 2) {
        func_L00_0025B040_30F318(moby, 0.3f);
        func_L01_0030F178_30F318(moby, pvars);
    }
    
    int end_linked = *(int *)(pvars + 0x48);
    if (end_linked != -1 && end_linked >= 0) {
        char *target = *(char **)(D_0013E633_30F318 + 0xE1D + 0x5893);
        if (target != 0) {
            target = target + (end_linked << 8);
            char *cls = *(char **)(target + 0x24);
            if (cls != 0) {
                *(int *)(target + 0x94) = *(int *)(cls + 0x10);
            }
        }
    }
}
