/* Carried from the PAL decompilation's nonmatching/shared/func_L01_002649D8.c (games/rac1) through the version
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
/* NON_MATCHING FUN_L01_00263a28 -- src/overlays/shared/mobyfunc_002649C8.c
 * Best so far: SIZE ours 648 / retail 652, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   unitclose05: fresh retail-assembly reconstruction resolves collision shape type0/1/3 inline vector, type2 scra
 */
extern void func_001FA050(void *, void *);
extern void func_001F9A68(void *, void *, float);
extern void func_001F9A80(void *, void *, float);
extern void func_001F9D20(void *, void *, void *);
extern void func_001F9A10(void *, void *, void *);
extern void func_001F9A28(void *, void *, void *);
extern float func_001F9AF0(void *);
extern void func_L00_002531E0(void *, int, int);

typedef union { int index; short pair[2]; float value; } ShapeIndices649D8_c263a28;

/* Resolves a collision shape from a moby's shape table and transformed vertices. */
int FUN_L01_00263a28(char *obj, int index, unsigned int *type, float *out, float *radius, float *length) {
    float matrix[16];
    float first[4];
    float second[4];
    char *shape = *(char **)(obj + 0x94);
    float scale;
    if (!shape) return 0;
    shape += index * 32 + 0x10;
    *type = *(unsigned char *)shape;
    scale = *(float *)(obj + 0x2C) * 0.0009765625f;
    func_001FA050(matrix, obj + 0x40);
    if (*type < 2 || *type == 3) {
        qcopy(out, shape + 0x10);
        func_001F9A68(out, out, scale);
        func_001F9D20(out, out, matrix);
        func_001F9A10(out, out, obj + 0x10);
        *radius = out[3] * scale;
        if (*type < 2) *length = 0.0f;
        else *length = *(float *)(shape + 4) * scale;
        return 1;
    } else if (*type == 2) {
        short bone = *(short *)(*(char **)(obj + 0x94) + 2);
        if (bone) func_L00_002531E0(obj, bone, 0);
        qcopy(out, (char *)0x70000000 + *(int *)(shape + 4) * 16);
        func_001F9A68(out, out, scale);
        func_001F9D20(out, out, matrix);
        func_001F9A10(out, out, obj + 0x10);
        *radius = *(float *)(shape + 0xC) * scale;
        *length = 0.0f;
        return 1;
    } else if (*type == 4) {
        short bone = *(short *)(*(char **)(obj + 0x94) + 2);
        if (bone) func_L00_002531E0(obj, bone, 0);
        func_001F9A80(first, (char *)0x70000000 + *(short *)(shape + 4) * 16, scale);
        {
            ShapeIndices649D8_c263a28 *indices = (ShapeIndices649D8_c263a28 *)(shape + 4);
            func_001F9A80(second, (char *)0x70000000 + indices->pair[1] * 16, scale);
        }
        if (first[2] < second[2]) qcopy(out, first);
        else qcopy(out, second);
        func_001F9D20(out, out, matrix);
        func_001F9A10(out, out, obj + 0x10);
        *radius = *(float *)(shape + 0xC) * scale;
        func_001F9A28(second, second, first);
        *length = func_001F9AF0(second);
        return 1;
    }
    return 0;
}
