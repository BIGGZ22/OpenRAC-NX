/* functional: medium. fcheck: unchecked (never called in fcheck's capture run of the title, menu and level 0; OpenRAC tools/fcheck_gate.py, 2026-10-18). The NTSC decomp's pending C for the same instructions (a draft it has not matched
   yet), carried to PAL's names by OpenRAC's tools/port.py. Never compared with retail's bytes here. */
typedef u32 u128_38F98 __attribute__((mode(TI), aligned(16)));
extern u8 D_00187180[];
extern volatile f32 D_0018CE00[];
struct Screen_38F98 {
    s32 width; /* 0x0: display width (D_00151780 +0x150) */
    s32 height; /* 0x4: display height (D_00151780 +0x152) */
    s32 half_width; /* 0x8: width >> 1 */
    s32 half_height; /* 0xC: height >> 1 */
    s32 left; /* 0x10: (0x800 - half_width) << 4 */
    s32 top; /* 0x14: (0x800 - half_height) << 4 */
    s32 right; /* 0x18: (0x800 + half_width) << 4 */
    s32 bottom; /* 0x1C: (0x800 + half_height) << 4 */
};
extern struct Screen_38F98 D_0013E600;
extern void func_001F9BF0(void *, void *, void *);
extern void func_001F9C30(void *, void *, f32);
extern void func_001F9EE8(void *, void *, void *);

/* Project opposing world bounds into a floating-point screen rectangle. The first point supplies x/y; subtracting it from the opposite point supplies width/height. Keep the four output stores in retail order because callers may alias outputs.
   Adapted from Lombyte (MIT) for PAL by OpenRAC's tools/port.py: src/assembly/textbin/fun_00237c80.c, project_graphics_bounds_float. */
void func_00238F98(u128_38F98 *first, u128_38F98 *opposite, f32 *width, f32 *height, f32 *x,
                                   f32 *y) {
    f32 first_projected[4] __attribute__((aligned(16)));
    f32 opposite_projected[4] __attribute__((aligned(16)));
    f32 *opposite_pointer;
    f32 scale_x;
    f32 scale_y;
    f32 projected_coordinate;

    opposite_pointer = opposite_projected;
    *(u128_38F98 *)first_projected = *first;
    *(u128_38F98 *)opposite_pointer = *opposite;
    func_001F9BF0(first_projected, first_projected, D_00187180);
    func_001F9BF0(opposite_pointer, opposite_pointer, D_00187180);
    func_001F9C30(first_projected, first_projected, 1024.0f);
    func_001F9C30(opposite_pointer, opposite_pointer, 1024.0f);
    opposite_pointer[3] = 1.0f;
    first_projected[3] = 1.0f;
    func_001F9EE8(first_projected, first_projected, D_00187180 - 0x40);
    func_001F9EE8(opposite_pointer, opposite_pointer, D_00187180 - 0x40);
    first_projected[0] *= 1.0f / first_projected[3];
    first_projected[1] *= 1.0f / first_projected[3];
    opposite_pointer[0] *= 1.0f / opposite_pointer[3];
    *(volatile f32 *)&opposite_pointer[1] *= 1.0f / opposite_pointer[3];
    scale_y = D_0018CE00[101];
    scale_x = D_0018CE00[100];
    first_projected[0] *= scale_x;
    first_projected[1] *= scale_y;
    opposite_pointer[0] *= scale_x;
    projected_coordinate = opposite_pointer[1] * scale_y;
    opposite_pointer[1] = projected_coordinate;
    projected_coordinate = (f32)D_0013E600.half_width;
    *x = first_projected[0] * 0.25f + projected_coordinate;
    *y = first_projected[1] * 0.25f + (f32)D_0013E600.half_height;
    *(volatile f32 *)width = (opposite_pointer[0] - first_projected[0]) * 0.25f;
    *height = (*(volatile f32 *)&opposite_pointer[1] - first_projected[1]) * 0.25f;
}
