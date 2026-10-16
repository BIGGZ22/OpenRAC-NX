/* NON_MATCHING func_L18_002EAB58 -- src/overlays/l18_veldin2/vendor_002A8400.c
 * Best so far: BYTES 339/1764 (80.8% of the bytes match), checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Size reached 1764 = retail in p13/p14 (p12 was 1768).
 *   Remaining differences (p14, BYTES 351/1764): frame is 0x160 vs 0x150 (loop-2 locals: retail
 *   puts the zeroed QVec at sp+0x70 and the else-branch tmp at sp+0, ours places the QVec elsewhere and
 *   hoists the zero store out of the loop); the else-branch tmp address is hoisted into $s0 instead of
 *   `daddu $a0,$sp,$zero`; the `addiu $v0,$0,1` in state 0 is scheduled differently. p12.c (1768,
 *   before the tmp compound-literal change) has the smaller frame diffs; p11/p10 are the earlier steps.
 *   Untried: -mno-split-addresses, tmp declared as a function-level object, a separate `static const
 *   QVec zero` copy.
 */
#include "common.h"

extern char *func_L00_0025B478(void *, int, int);
extern void func_L18_002EB240(char *);
extern int func_L00_0028EF68(int i, int a1, int v, int k);
extern float func_002140F8(float, float);
extern int func_002140B0(int);
extern int func_001F9850(int);
extern int func_L00_00258BC8(int, int);
extern void func_L00_0026B890(void *, void *, int, int, int, int, int, int, float, float);
extern void func_L00_001FF4B0(void *, void *, float);
extern void func_L00_0026B368(void *pos, void *vel, int color, int tag, int life, float size);
extern void func_0020D678(void *);
extern float func_L00_00258C80(float lo, float hi);
extern void func_001F9BF0(void *, void *, void *);
extern void func_L00_00258DB0(float *, float, float);
extern void func_001F9BD8(void *, void *, void *);
extern float func_001F9B88(float);
extern float func_001FA748(float, float);
extern int func_L00_001F10E0(float, void *, int, void *);
extern void func_L00_00260108(void *, void *, int, float, float);
extern char D_0013E633[];
extern unsigned char D_0014C150[];
extern int D_0015EE84 MACRO_ADDR;
extern float D_0015EE6C MACRO_ADDR;
extern float D_0015EE70 MACRO_ADDR;
extern int D_L18_0015F660 MACRO_ADDR;
typedef struct { int v[6]; } Tab6;
extern Tab6 D_L18_001F2C90;
extern Tab6 D_L18_001F2CA8;

void func_L18_002EAB58(char *moby) {
    char *d = *(char **)(moby + 0x78);

    switch ((unsigned char)moby[0x20]) {
    case 0: {
        unsigned char s = moby[0xB0];
        if (s != 0xFF && D_0014C150[s + D_0015EE84 * 16] == 0xFF) {
            goto del;
        }
        moby[0x20] = 1;
        {
            int *p = *(int **)d;
            if (p != 0) {
                *p = 3;
            }
        }
        break;
    }
    case 1:
        if (func_L00_0025B478(moby, 0x80000, 0) != 0) {
            func_L18_002EB240((char *)(int)(unsigned char)moby[0x21]);
            func_L00_0028EF68(0, 0, (int)moby, 0x375);
        }
        moby[0xA4] = 0xFF;
        break;
    case 2:
        if (*(short *)(moby + 0xA6) == 0x37C) {
            int i = 0;
            int j;
            while ((float)i < 10.0f) {
                float sc = func_002140F8(8.0f, 10.0f) * D_0015EE6C;
                Tab6 ta = D_L18_001F2C90;
                Tab6 tb = D_L18_001F2CA8;
                float pos[4];
                qcopy(pos, moby + 0x10);
                i++;
                pos[2] += func_002140F8(0.0f, 6.0f);
                func_L00_0026B890(pos, &D_L18_0015F660, ta.v[func_002140B0(6)], tb.v[func_002140B0(6)],
                                  func_L00_00258BC8(func_001F9850(15), func_001F9850(20)),
                                  func_L00_00258BC8(func_001F9850(30), func_001F9850(45)), 0, 0, 1500000.0f, sc);
                func_L00_0026B890(pos, &D_L18_0015F660, 0x7FFFFFFF, 0xFFFFFF,
                                  func_L00_00258BC8(func_001F9850(5), func_001F9850(10)),
                                  func_L00_00258BC8(func_001F9850(15), func_001F9850(20)), 0, 0, 1500000.0f, sc * 0.5f);
            }
            j = 0;
            while ((float)j < 10.0f) {
                float pos[4];
                Tab6 ta;
                Tab6 tb;
                float vel[4] __attribute__((aligned(16)));
                QVec tmp;
                qcopy(pos, moby + 0x10);
                j++;
                pos[2] += func_002140F8(0.0f, 6.0f);
                ta = D_L18_001F2C90;
                tb = D_L18_001F2CA8;
                tmp = (QVec){{0.0f, 0.0f, 0.0f, 0.0f}};
                tmp.v[0] = func_002140F8(-1.0f, 1.0f);
                tmp.v[1] = func_002140F8(-1.0f, 1.0f);
                tmp.v[2] = func_002140F8(-1.0f, 1.0f);
                *(QVec *)vel = tmp;
                func_L00_001FF4B0(vel, vel, func_002140F8(0.0f, 3.0f) * D_0015EE6C);
                func_L00_0026B368(pos, vel, ta.v[func_002140B0(6)], tb.v[func_002140B0(6)],
                                  func_L00_00258BC8(func_001F9850(30), func_001F9850(45)), 500000.0f);
            }
          del:
            func_0020D678(moby);
        } else {
            float a, f;
            QVec tmp;
            float *v = (float *)(d + 0x10);
            a = func_002140F8(10.0f, 15.0f) * D_0015EE6C;
            *(int *)(moby + 0x94) = 0;
            moby[0x20] = 3;
            *(float *)(d + 8) = func_L00_00258C80(0.0f, 90.0f) * 0.017453292f * D_0015EE6C;
            *(float *)(d + 0xC) = func_L00_00258C80(0.0f, 90.0f) * 0.017453292f * D_0015EE6C;
            func_001F9BF0(v, moby + 0x10, D_0013E633 + 0xE9D);
            *(float *)(d + 0x18) = 0.0f;
            func_L00_001FF4B0(v, v, a);
            func_L00_00258DB0(&tmp, a, a);
            func_001F9BD8(v, v, &tmp);
            *(float *)(d + 0x18) = func_001F9B88(*(float *)(d + 0x18));
            *(float *)(d + 0x1C) = *(float *)(moby + 0x18);
            return;
        }
        break;
    case 3: {
        float k = D_0015EE70;
        *(float *)(moby + 0x40) = func_001FA748(*(float *)(moby + 0x40), *(float *)(d + 8));
        *(float *)(moby + 0x44) = func_001FA748(*(float *)(moby + 0x44), *(float *)(d + 8));
        *(float *)(d + 0x18) = *(float *)(d + 0x18) - k * 20.0f;
        func_001F9BD8(moby + 0x10, moby + 0x10, d + 0x10);
        if (*(float *)(moby + 0x18) < *(float *)(d + 0x1C) - 16.0f ||
            func_L00_001F10E0(1.0f, moby + 0x10, 2, moby) != 0) {
            func_L00_00260108(moby, moby + 0x10, -1, 1.5f, 0.0f);
            func_0020D678(moby);
        }
        break;
    }
    }
}
