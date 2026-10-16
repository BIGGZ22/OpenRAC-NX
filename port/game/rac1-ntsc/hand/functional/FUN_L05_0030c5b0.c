/* Carried from the PAL decompilation's src/overlays/shared/vendor_002CF2C0.c (matched C there) (games/rac1) through the version
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
extern float D_0015ED6C MACRO_ADDR;
extern float func_001FA580(float, float);
extern float func_001F9DE0(float);
extern char D_0013E533[];
extern float func_00213308(void);
typedef struct {
    unsigned char pad0[0x454];
    unsigned char done[1];
} LvlDone_DA08_c30c5b0;
typedef struct {
    unsigned char pad0[0x2FC];
    void *ground_moby;
    unsigned char pad300[0xE];
    short air;
} Hero_DA08_c30c5b0;
struct Moby_DA08_c30c5b0 {
    unsigned char pad0[0x18];
    float z;
    unsigned char pad1C[4];
    unsigned char state;
    unsigned char pad21[0x57];
    int *pvars;
    unsigned char pad7C[0x14];
    int glow;
    unsigned char pad94[0x1C];
    unsigned char mission;
    unsigned char padB1;
    unsigned short save_id;
    unsigned char padB4[8];
    unsigned char command;
};
extern LvlDone_DA08_c30c5b0 D_L05_001BB6B0;
extern LvlDone_DA08_c30c5b0 D_L05_001BAA50;
extern unsigned char D_L05_0015FC88[] MACRO_ADDR;
extern char *D_L05_001B0930[];
extern int D_0015ED84 MACRO_ADDR;
extern int func_001FA6D0(float);
extern int D_0014C190[][64];
extern unsigned char D_0014C050[];
extern int func_0022ED80_DA08() __asm__("func_0022DA68");



/* Floor button update: settles, checks whether it is already pressed, pulses and records the press. */
void FUN_L05_0030c5b0(struct Moby_DA08_c30c5b0 *moby) {
    int *d = moby->pvars;
    short id;
    int i;
    int list;
    char *p;
    float angle;
    int color;
    unsigned int shade;
    unsigned int high;
    int b;
    unsigned char c;

    switch (moby->state) {
    case 0:
        *(float *)(d + 3) = func_00213308();
        moby->z -= 0.35f;
        id = (short)moby->save_id;
        if (D_L05_001BB6B0.done[id] != 0 ||
            (D_0014C190[D_0015ED84][id >> 5] >> (moby->save_id & 31) & 1) != 0 ||
            (d[2] != 0 && D_0014C050[moby->mission + (D_0015ED84 << 4)] == 0xFF)) {
            moby->command = 2;
            moby->glow = 0x80208020;
            moby->state = 2;
            list = d[0];
            if (list != -1) {
                i = 0;
                if (*(int *)D_L05_001B0930[list] > 0) {
                    do {
                        p = D_L05_001B0930[list] + i * 16;
                        if (*(float *)(p + 0x1C) == ((float *)d)[1])
                            *(int *)(p + 0x1C) = 0;
                        list = d[0];
                        i++;
                    } while (i < *(int *)D_L05_001B0930[list]);
                }
            }
        } else {
            moby->state = 1;
        }
        break;
    case 1:
        angle = func_001FA580(*(float *)(d + 3), D_0015ED6C * 6.2831855f);
        *(float *)(d + 3) = angle;
        color = func_001FA6D0((func_001F9DE0(angle) * 4.0f - 3.0f) * 128.0f);
        if (color > 0x80) color = 0x80;
        else if (color < 0x20) color = 0x20;
        shade = 0x80000000 | (color << 8);
        high = color << 16;
        moby->glow = high | shade | color;
        if (((Hero_DA08_c30c5b0 *)(D_0013E533 + 0xE1D))->ground_moby == moby &&
            ((Hero_DA08_c30c5b0 *)(D_0013E533 + 0xE1D))->air == 0) {
            D_L05_001BAA50.done[(short)moby->save_id] = moby->mission + 2;
            if (moby->mission == 0xFF || (D_L05_0015FC88[(unsigned char)moby->mission] != 0xFF && D_0014C050[(unsigned char)moby->mission + (D_0015ED84 << 4)] == 0xFF)) {
                D_L05_001BB6B0.done[(short)moby->save_id] = moby->mission + 2;
            }
            moby->state = 2;
            moby->command = 1;
            moby->glow = 0x80208020;
            func_0022ED80_DA08(0, 0, moby);
            list = d[0];
            if (list != -1) {
                i = 0;
                if (*(int *)D_L05_001B0930[list] > 0) {
                    do {
                        p = D_L05_001B0930[list] + i * 16;
                        if (*(float *)(p + 0x1C) == ((float *)d)[1])
                            *(int *)(p + 0x1C) = 0;
                        list = d[0];
                        i++;
                    } while (i < *(int *)D_L05_001B0930[list]);
                }
            }
        }
        break;
    }
}
