/* functional: high. checked against the US assembly: same address, constant and store-order check as FUN_00237a78 (float outputs) */
/* Carried from the PAL decompilation's nonmatching/functional/func_00238F98.c (games/rac1) through the version
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
/* functional: medium. The NTSC decomp's pending C for the same instructions (a draft it has not matched
   yet), carried to PAL's names by OpenRAC's tools/port.py. Never compared with retail's bytes here. */
typedef u32 u128_38F98_c237c80 __attribute__((mode(TI), aligned(16)));
extern u8 D_00187080[];
extern volatile f32 D_0018CD00[];
struct Screen_38F98_c237c80 {
    s32 width; /* 0x0: display width (D_00151680 +0x150) */
    s32 height; /* 0x4: display height (D_00151680 +0x152) */
    s32 half_width; /* 0x8: width >> 1 */
    s32 half_height; /* 0xC: height >> 1 */
    s32 left; /* 0x10: (0x800 - half_width) << 4 */
    s32 top; /* 0x14: (0x800 - half_height) << 4 */
    s32 right; /* 0x18: (0x800 + half_width) << 4 */
    s32 bottom; /* 0x1C: (0x800 + half_height) << 4 */
};
extern struct Screen_38F98_c237c80 D_0013E500;
extern void func_001F9A28(void *, void *, void *);
extern void func_001F9A68(void *, void *, f32);
extern void func_001F9D20(void *, void *, void *);



/* Project opposing world bounds into a floating-point screen rectangle. The first point supplies x/y; subtracting it from the opposite point supplies width/height. Keep the four output stores in retail order because callers may alias outputs.
   Adapted from Lombyte (GPL-3.0) for PAL by OpenRAC's tools/port.py: src/assembly/textbin/fun_00237c80.c, project_graphics_bounds_float. */
void FUN_00237c80(u128_38F98_c237c80 *first, u128_38F98_c237c80 *opposite, f32 *width, f32 *height, f32 *x,
                                   f32 *y) {
    f32 first_projected[4] __attribute__((aligned(16)));
    f32 opposite_projected[4] __attribute__((aligned(16)));
    f32 *opposite_pointer;
    f32 scale_x;
    f32 scale_y;
    f32 projected_coordinate;

    opposite_pointer = opposite_projected;
    *(u128_38F98_c237c80 *)first_projected = *first;
    *(u128_38F98_c237c80 *)opposite_pointer = *opposite;
    func_001F9A28(first_projected, first_projected, D_00187080);
    func_001F9A28(opposite_pointer, opposite_pointer, D_00187080);
    func_001F9A68(first_projected, first_projected, 1024.0f);
    func_001F9A68(opposite_pointer, opposite_pointer, 1024.0f);
    opposite_pointer[3] = 1.0f;
    first_projected[3] = 1.0f;
    func_001F9D20(first_projected, first_projected, D_00187080 - 0x40);
    func_001F9D20(opposite_pointer, opposite_pointer, D_00187080 - 0x40);
    first_projected[0] *= 1.0f / first_projected[3];
    first_projected[1] *= 1.0f / first_projected[3];
    opposite_pointer[0] *= 1.0f / opposite_pointer[3];
    *(volatile f32 *)&opposite_pointer[1] *= 1.0f / opposite_pointer[3];
    scale_y = D_0018CD00[101];
    scale_x = D_0018CD00[100];
    first_projected[0] *= scale_x;
    first_projected[1] *= scale_y;
    opposite_pointer[0] *= scale_x;
    projected_coordinate = opposite_pointer[1] * scale_y;
    opposite_pointer[1] = projected_coordinate;
    projected_coordinate = (f32)D_0013E500.half_width;
    *x = first_projected[0] * 0.25f + projected_coordinate;
    *y = first_projected[1] * 0.25f + (f32)D_0013E500.half_height;
    *(volatile f32 *)width = (opposite_pointer[0] - first_projected[0]) * 0.25f;
    *height = (*(volatile f32 *)&opposite_pointer[1] - first_projected[1]) * 0.25f;
}
