/* Carried from the PAL decompilation's nonmatching/functional/func_L07_0030E370.c (games/rac1/pal) through the version
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
/* functional: high. The near miss with its look-alike global fixed (state 2's step rate is the float D_0015ED60, read through $gp, not D_0013ED60) and host-safe aliased declarations; the rest differs from retail only in compiler choices (tail merging of the two identical step-and-move tails, a range test for states 0xFD/0xFE, folded offsets, registers). */
/* Level 07 (Umbris) update for a moby that shuttles between a home position and a point tied to
 * another moby's state. Its data (moby+0x78) holds the home position (+0x0..+0x8), a rider slot
 * (+0xC), two sound ids (+0xE, +0xF), the index of a watched moby (+0x10), a flag slot in
 * D_L07_00184528 (+0x14), two states of the watched moby (+0x16, +0x18) and a travel distance
 * (+0x1C). A level table at D_0014161B+0xAA35 clears the watched index when its entry is 0xFF.
 * State 0 records the home position; 1 waits for the watched moby to leave state +0x16, plays
 * sound +0xE; 2 steps along its local y (rotated by the matrix at +0xC0; direction flipped for
 * classes 0x3F5 and 0x429) by D_0015ED60*0.01 per frame until it is farther than +0x1C from home
 * (sound +0xF, flag = 1, state 3), or plays +0xE and goes to 4 when the watched moby reaches
 * state +0x18; 3 waits for that state; 4 steps back and, once home, snaps there, releases the
 * rider (func_L00_0028D918) if it rides this moby, plays sound +0xF, clears the flag, state 1.
 * The step vectors are 128-bit values as in retail (por/sq/lq); only their y float is set. */
extern int func_0022ED80_30E370(int, int, int) __asm__("func_0022DA68");
extern float func_001F9D48_30E370(void *, void *) __asm__("func_001F9B80");
extern void func_001F9EC0_30E370(void *, void *, void *) __asm__("func_001F9CF8");
extern void func_001F9BD8_30E370(void *, void *, void *) __asm__("func_001F9A10");
extern void func_L00_0028EBF0_30E370(int) __asm__("func_L00_0028D918");
extern int D_0015EE84_30E370 __asm__("D_0015ED84") MACRO_ADDR;
extern char D_0014171B_30E370[] __asm__("D_0014161B");
extern int D_L07_001845A8_30E370[] __asm__("D_L07_00184528");
extern char *D_L07_00160058_30E370 __asm__("D_L07_0015FFD8") MACRO_ADDR;
extern float D_0015EE60_30E370 __asm__("D_0015ED60") MACRO_ADDR;
extern char D_0013E633_30E370[] __asm__("D_0013E533");

typedef int u128_30E370_c30cf90 __attribute__((mode(TI), aligned(16)));

void FUN_L07_0030cf90(char *moby) {
    char *p = *(char **)(moby + 0x78);
    float v[4];
    u128_30E370_c30cf90 t10;
    u128_30E370_c30cf90 t20;
    int id;
    unsigned char s;
    float d;

    v[0] = *(float *)p;
    v[1] = *(float *)(p + 4);
    v[2] = *(float *)(p + 8);
    v[3] = 0;

    if ((unsigned char)moby[0xB0] != 0xFF) {
        if ((unsigned char)D_0014171B_30E370[(unsigned char)moby[0xB0] + (D_0015EE84_30E370 << 4) + 0xAA35] == 0xFF) {
            *(int *)(p + 0x10) = -1;
        }
    }

    switch ((unsigned char)moby[0x20]) {
    case 0:
        *(float *)p = *(float *)(moby + 0x10);
        *(float *)(p + 4) = *(float *)(moby + 0x14);
        *(short *)(p + 0xC) = -1;
        *(float *)(p + 8) = *(float *)(moby + 0x18);
        id = *(short *)(p + 0x14);
        if (id != -1) {
            D_L07_001845A8_30E370[id] = 0;
        }
        moby[0x20] = 1;
        break;

    case 1:
        id = *(int *)(p + 0x10);
        if (id != -1) {
            if (D_L07_00160058_30E370 + (id << 8) != 0) {
                s = (unsigned char)(D_L07_00160058_30E370 + (id << 8))[0x20];
                if (s != 0xFE && s != 0xFD && (int)s != *(short *)(p + 0x16)) {
                    return;
                }
            }
        }
        if ((signed char)p[0xE] != -1) {
            func_0022ED80_30E370((signed char)p[0xE], 0, (int)moby);
        }
        moby[0x20] = 2;
        break;

    case 2:
        id = *(int *)(p + 0x10);
        if (id != -1 && D_L07_00160058_30E370 + (id << 8) != 0) {
            s = (unsigned char)(D_L07_00160058_30E370 + (id << 8))[0x20];
            if (s != 0xFE && s != 0xFD && (int)s == *(int *)(p + 0x18)) {
                func_0022ED80_30E370((signed char)p[0xE], 0, (int)moby);
                moby[0x20] = 4;
                break;
            }
        }
        d = func_001F9D48_30E370(moby + 0x10, v);
        if (*(float *)(p + 0x1C) < d) {
            if ((signed char)p[0xF] != -1) {
                func_0022ED80_30E370((signed char)p[0xF], 0, (int)moby);
            }
            id = *(short *)(p + 0x14);
            if (id != -1) {
                D_L07_001845A8_30E370[id] = 1;
            }
            moby[0x20] = 3;
        } else {
            /* retail: lwc1 -0x7EA0($gp) = D_0015ED60, the rate state 4 also reads */
            float k = D_0015EE60_30E370 * 0.01f;
            t20 = 0;
            ((float *)&t20)[1] = -k;
            t10 = t20;
            if (*(short *)(moby + 0xA6) == 0x3F5 || *(short *)(moby + 0xA6) == 0x429) {
                ((float *)&t10)[1] = -((float *)&t10)[1];
            }
            func_001F9EC0_30E370(&t20, &t10, moby + 0xC0);
            func_001F9BD8_30E370(moby + 0x10, moby + 0x10, &t20);
        }
        break;

    case 3:
        id = *(int *)(p + 0x10);
        if (id != -1 && D_L07_00160058_30E370 + (id << 8) != 0) {
            s = (unsigned char)(D_L07_00160058_30E370 + (id << 8))[0x20];
            if (s != 0xFE && s != 0xFD) {
                if ((int)s != *(int *)(p + 0x18)) {
                    return;
                }
                if ((signed char)p[0xE] != -1) {
                    func_0022ED80_30E370((signed char)p[0xE], 0, (int)moby);
                }
                moby[0x20] = 4;
            }
        }
        break;

    case 4:
        id = *(int *)(p + 0x10);
        if (id != -1 && D_L07_00160058_30E370 + (id << 8) != 0) {
            s = (unsigned char)(D_L07_00160058_30E370 + (id << 8))[0x20];
            if (s != 0xFE && s != 0xFD && (int)s != *(short *)(p + 0x16)) {
                d = func_001F9D48_30E370(moby + 0x10, v);
                if (d <= D_0015EE60_30E370 * 0.01f) {
                    *(float *)(moby + 0x10) = *(float *)p;
                    *(float *)(moby + 0x14) = *(float *)(p + 4);
                    *(int *)(moby + 0x1C) = 0;
                    *(float *)(moby + 0x18) = *(float *)(p + 8);
                    id = *(short *)(p + 0xC);
                    if (id != -1) {
                        char *t = D_0013E633_30E370 + 0x1D + id * 0x70;
                        if (*(int *)(t + 0x88) == (int)moby && *(unsigned char *)(t + 0x74) != 0) {
                            func_L00_0028EBF0_30E370(id);
                        }
                    }
                    *(short *)(p + 0xC) = -1;
                    if ((signed char)p[0xF] != -1) {
                        func_0022ED80_30E370((signed char)p[0xF], 0, (int)moby);
                    }
                    id = *(short *)(p + 0x14);
                    if (id != -1) {
                        D_L07_001845A8_30E370[id] = 0;
                    }
                    moby[0x20] = 1;
                } else {
                    t20 = 0;
                    ((float *)&t20)[1] = D_0015EE60_30E370 * 0.01f;
                    t10 = t20;
                    if (*(short *)(moby + 0xA6) == 0x3F5 || *(short *)(moby + 0xA6) == 0x429) {
                        ((float *)&t10)[1] = -((float *)&t10)[1];
                    }
                    func_001F9EC0_30E370(&t20, &t10, moby + 0xC0);
                    func_001F9BD8_30E370(moby + 0x10, moby + 0x10, &t20);
                }
                return;
            }
            func_0022ED80_30E370((signed char)p[0xE], 0, (int)moby);
            moby[0x20] = 2;
        } else {
            func_0022ED80_30E370((signed char)p[0xE], 0, (int)moby);
            moby[0x20] = 2;
        }
        break;

    default:
        break;
    }
}
