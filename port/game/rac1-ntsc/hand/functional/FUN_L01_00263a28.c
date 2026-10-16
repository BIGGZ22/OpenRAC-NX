/* Carried from the PAL decompilation's src/overlays/shared/mobyfunc_002649C8.c (matched C there) (games/rac1) through the version
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
typedef u32 u128_c263a28 __attribute__((mode(TI), aligned(16)));
typedef f32 sceVu0FVECTOR_c263a28[4] __attribute__((aligned(16)));
void func_001F9A10(void *out, void *a, void *b);
void func_001F9A28(void *out, void *a, void *b);
void func_001F9A68(void *out, void *a, f32 scale);
f32 func_001F9AF0(void *a);
extern void func_001FA050(void *matrix, void *rotation);
extern void func_001F9D20(void *out, void *in, void *matrix);
extern void func_001F9A80(float scale, void *out, void *joint);
extern void func_L00_002531E0(void *moby, short joint, int flags);



/* Joint positions in scratchpad, 16 bytes each.
   Adapted from Lombyte (MIT) for PAL by OpenRAC's tools/port.py: src/overlays/shared/gameplay/state/00263a28.c, FUN_L01_00263a28. */
int FUN_L01_00263a28(char *moby, int index, unsigned int *type, float *result,
                      float *radius, float *distance)
{
    char *entry;
    float scale;
    u128_c263a28 matrix[4];
    sceVu0FVECTOR_c263a28 first;
    sceVu0FVECTOR_c263a28 second;
    short *joints;

    entry = *(char **)(moby + 0x94);
    if (entry == 0) {
        return 0;
    }
    entry += index * 0x20 + 0x10;
    *type = *(unsigned char *)entry;
    scale = *(float *)(moby + 0x2c) * 0.0009765625f;
    func_001FA050(matrix, moby + 0x40);

    if (*type < 2 || *type == 3) {
        qcopy(result, entry + 0x10);
        func_001F9A68(result, result, scale);
        func_001F9D20(result, result, matrix);
        func_001F9A10(result, result, moby + 0x10);
        *radius = result[3] * scale;
        if (*type < 2) {
            *distance = 0.0f;
        } else {
            *distance = *(float *)(entry + 4) * scale;
        }
        return 1;
    } else if (*type == 2) {
        short joint = *(short *)(*(char **)(moby + 0x94) + 2);
        if (joint != 0) {
            func_L00_002531E0(moby, joint, 0);
        }
        qcopy(result, (void *)(0x70000000 + *(int *)(entry + 4) * 0x10));
        func_001F9A68(result, result, scale);
        func_001F9D20(result, result, matrix);
        func_001F9A10(result, result, moby + 0x10);
        *radius = *(float *)(entry + 0xc) * scale;
        *distance = 0.0f;
        return 1;
    } else if (*type == 4) {
        short joint = *(short *)(*(char **)(moby + 0x94) + 2);
        if (joint != 0) {
            func_L00_002531E0(moby, joint, 0);
        }
        joints = (short *)(entry + 4);
        func_001F9A80(scale, first, (void *)(0x70000000 + joints[0] * 0x10));
        func_001F9A80(scale, second, (void *)(0x70000000 + joints[1] * 0x10));
        if (first[2] < second[2]) {
            qcopy(result, first);
        } else {
            qcopy(result, second);
        }
        func_001F9D20(result, result, matrix);
        func_001F9A10(result, result, moby + 0x10);
        *radius = *(float *)(entry + 0xc) * scale;
        func_001F9A28(second, second, first);
        *distance = func_001F9AF0(second);
        return 1;
    } else {
        return 0;
    }
}
