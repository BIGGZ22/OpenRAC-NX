/* Carried from the PAL decompilation's src/overlays/l12_hoven/vendor_002EDAA0.c (matched C there) (games/rac1) through the version
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
extern void func_0020C828(void *);
extern float func_001FA580(float, float);
extern float func_L00_0025BC98(void *, float, void *, int, float, float, float);
extern void func_L00_00250DF8(void *);
extern int D_L12_001FB688[];
extern int D_L12_00160058_m __asm__("D_L12_0015FFD8") MACRO_ADDR;
extern float D_L12_00161F40 MACRO_ADDR;
extern float D_L12_00161F50 MACRO_ADDR;
extern float D_L12_00161F60 MACRO_ADDR;
extern float D_L12_00161F70 MACRO_ADDR;
extern float D_0015ED6C MACRO_ADDR;



// Level 12 vendor: updates each of the 18 slot objects for one frame.
void FUN_L12_003064e8(char *moby) {
    int i;
    int off;
    char *data = *(char **)(moby + 0x78);
    int last = 17;
    for (i = 0, off = 0; i < 18; i++, off += 4) {
        int s = D_L12_001FB688[i];
        char *obj;
        float f1;
        if (s == 0) {
            continue;
        }
        if (i == last) {
            obj = moby;
        } else {
            if (s == -1) {
                continue;
            }
            obj = (char *)(D_L12_00160058_m + (s << 8));
        }
        if (i == last) {
            f1 = D_L12_00161F40;
            if (f1 == -1.0f) {
                qcopy((char *)&D_L12_00161F40, obj + 0x10);
                qcopy((char *)&D_L12_00161F50, obj + 0x40);
            }
            qcopy((char *)&D_L12_00161F60, obj + 0x10);
            qcopy((char *)&D_L12_00161F70, obj + 0x40);
            qcopy(obj + 0x10, (char *)&D_L12_00161F40);
            qcopy(obj + 0x40, (char *)&D_L12_00161F50);
            f1 = *(float *)(obj + 0x18);
        } else {
            f1 = *(float *)(obj + 0x18);
        }
        if (f1 < 20.0f) {
            if (i == last) {
                return;
            }
            func_0020C828(obj);
            D_L12_001FB688[i] = -1;
        }
        f1 = *(float *)(data + 0x338 + i * 16) - *(float *)(data + off + 0x570);
        *(float *)(data + 0x338 + i * 16) = f1;
        *(float *)(obj + 0x18) = *(float *)(obj + 0x18) + f1;
        if (*(short *)(obj + 0xA6) == 0x500) {
            *(float *)(obj + 0x40) = func_001FA580(*(float *)(obj + 0x40), D_0015ED6C * 0.34906584f);
            func_L00_0025BC98(obj + 0x44, 1.5707964f, data + 0x454 + i * 16, 0, 0.00200000009f, 0.3f, 0.0f);
        } else if (*(short *)(obj + 0xA6) == 0x4FD) {
            *(float *)(obj + 0x40) = func_001FA580(*(float *)(obj + 0x40), *(float *)(data + 0x450 + i * 16));
            func_L00_0025BC98(obj + 0x44, -1.5707964f, (float *)(data + 0x450 + i * 16) + 1, 0, *(float *)(data + 0x45C + i * 16), 0.3f, 0.0f);
        } else if (*(short *)(obj + 0xA6) == 0x4FA) {
            float t;
            *(float *)(obj + 0x40) = func_001FA580(*(float *)(obj + 0x40), D_0015ED6C * -0.87266463f);
            t = func_001FA580(*(float *)(obj + 0x44), *(float *)(data + 0x454 + i * 16));
            *(float *)(obj + 0x14) = *(float *)(obj + 0x14) + D_0015ED6C * -4.0f;
            *(float *)(obj + 0x44) = t;
        } else {
            *(float *)(obj + 0x14) = *(float *)(obj + 0x14) + D_0015ED6C * -4.0f;
            *(float *)(obj + 0x40) = func_001FA580(*(float *)(obj + 0x40), *(float *)(data + 0x450 + i * 16));
            *(float *)(obj + 0x44) = func_001FA580(*(float *)(obj + 0x44), *(float *)(data + 0x454 + i * 16));
        }
        if (i == last) {
            qcopy((char *)&D_L12_00161F40, obj + 0x10);
            qcopy((char *)&D_L12_00161F50, obj + 0x40);
        }
        func_L00_00250DF8(obj);
    }
}
