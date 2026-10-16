/* Carried from the PAL decompilation's src/overlays/l05_rilgar/vendor_002D28D0.c (matched C there) (games/rac1) through the version
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
extern void func_001F9A10(void *, void *, void *);
extern int func_001FA6E0(int, int, float);
extern int D_L05_0015F5CC MACRO_ADDR;
typedef struct {
    char vertices[0x40];
    int color[4];
    float uv[4][2];
    u64 zero;
    u64 effect_texture;
    u64 texture;
    u64 flags;
} L05GlowPacket_c2d7920;
extern u64 func_001F44B8(int);
extern float func_001FA6C0(int);
extern void func_001153FC(void *, int, int);
extern int func_L00_002002E0(void *, float);
extern void func_L00_001FD228(void *, int, int);
extern unsigned char D_0015EDB0[] MACRO_ADDR;
extern short D_L05_001613E0;
extern short D_L05_001613F0;
extern short D_L05_001613F4;
extern short D_L05_001613F8;
extern short D_L05_001613FC;
extern short D_L05_00161400;
extern short D_L05_00161404;
extern short D_L05_00161408;
extern short D_L05_0016140C;
extern short *D_L05_001ABCC0[];
extern int D_L05_0015FFD8 MACRO_ADDR;
extern float D_L05_001CDD70[][2];



void FUN_L05_002d7920(char *moby) {
    L05GlowPacket_c2d7920 packet[16];
    float matrix[8];
    float center[4];
    int i;
    int j;
    short *list;
    char *member;
    char *data;
    float phase;
    float ratio;
    u64 texture;
    L05GlowPacket_c2d7920 *p;

    if (*(int *)&D_L05_001613F0 != 0)
        return;
    if (D_0015EDB0[2] != 0)
        *(float *)&D_L05_001613E0 = 16.0f;
    else
        *(float *)&D_L05_001613E0 = 5.0f;
    for (i = 0; i < 16; i++) {
        texture = func_001F44B8(0x13);
        p = &packet[i];
        p->effect_texture = texture;
        p->texture = 0xff9000000260ULL;
        p->zero = 0;
        p->flags = (u64)*(int *)&D_L05_001613F8 | ((u64)*(int *)&D_L05_001613FC << 2) |
                   ((u64)*(int *)&D_L05_00161400 << 4) | ((u64)*(int *)&D_L05_00161404 << 6) |
                   0x8000000000ULL;
        phase = func_001FA6C0((D_L05_0015F5CC + 3) & 3) / (*(float *)&D_L05_001613E0 * 4.0f);
        for (j = 0; j < 4; j++) {
            packet[i].uv[j][0] = D_L05_001CDD70[j][0];
            packet[i].uv[j][1] = D_L05_001CDD70[j][1];
            ratio = func_001FA6C0(i + 1 - (j >> 1)) / (*(float *)&D_L05_001613E0 + phase);
            if (ratio > 1.0f)
                ratio = 1.0f;
            else if (ratio < 0.0f)
                ratio = 0.0f;
            packet[i].color[j] = func_001FA6E0(*(int *)&D_L05_00161408, *(int *)&D_L05_0016140C, ratio);
        }
    }
    func_001153FC(matrix, 0, 0x20);
    matrix[2] = 0.15f;
    matrix[6] = -0.15f;
    list = D_L05_001ABCC0[(unsigned char)moby[0x21]];
    do {
        member = (char *)(((*list & 0x7fff) << 8) + *(int *)&D_L05_0015FFD8);
        if (*(short *)(member + 0xA6) == 0x4f && (unsigned char)member[0x20] == 1) {
            int frame;
            data = *(char **)(member + 0x78);
            qcopy_nc(center, member + 0x10);
            center[3] = 20.0f;
            if (func_L00_002002E0(center, 512.0f) != -1) {
                for (frame = 0; frame < *(short *)(data + 0x320); frame++) {
                    int first = (*(short *)(data + 0x322) - frame + 15) & 15;
                    int ribbon;
                    for (ribbon = 0; ribbon < 3; ribbon++) {
                        if (*(int *)&D_L05_001613F4 == 0) {
                            char *points = data + (ribbon * 0x100 + 0x20);
                            int k;
                            for (k = 0; k < 4; k++) {
                                int index = (first + (k >> 1)) & 15;
                                func_001F9A10(packet[frame].vertices + k * 0x10,
                                              points + (index << 4),
                                              (char *)matrix + (k & 1) * 0x10);
                            }
                            func_L00_001FD228(&packet[frame], 0, 0);
                        }
                    }
                }
            }
        }
    } while (*list++ >= 0);
}
