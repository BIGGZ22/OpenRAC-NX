/* Carried from the PAL decompilation's src/overlays/shared/mobyutil_00261B00.c (matched C there) (games/rac1) through the version
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
extern f32 func_001F99C0(f32);
typedef int OvlQuad_630A8_c262030 __attribute__((mode(TI)));
typedef union {
    OvlQuad_630A8_c262030 q;
    f32 f[4];
    s32 i[4];
} OvlVec4_630A8_c262030;
typedef struct {
    s32 count;
    u8 pad[12];
    OvlVec4_630A8_c262030 points[1];
} SegmentPath_262030_c262030;
extern SegmentPath_262030_c262030 * D_L00_001B0830_630A8[] __asm__("D_L00_001B04B0");
extern OvlVec4_630A8_c262030 D_0013F5E0;
extern void func_001F9BD8_630A8(OvlVec4_630A8_c262030 *, OvlVec4_630A8_c262030 *, OvlVec4_630A8_c262030 *) __asm__("func_001F9A10");
extern void func_001F9BF0_630A8(OvlVec4_630A8_c262030 *, OvlVec4_630A8_c262030 *, OvlVec4_630A8_c262030 *) __asm__("func_001F9A28");
extern f32 func_001F9C78_630A8(OvlVec4_630A8_c262030 *, OvlVec4_630A8_c262030 *) __asm__("func_001F9AB0");
extern void func_001F9CA0_630A8(OvlVec4_630A8_c262030 *, OvlVec4_630A8_c262030 *, OvlVec4_630A8_c262030 *) __asm__("func_001F9AD8");
extern f32 func_001F9CB8_630A8(OvlVec4_630A8_c262030 *) __asm__("func_001F9AF0");
extern void func_L00_001FF4B0_630A8(f32, OvlVec4_630A8_c262030 *, OvlVec4_630A8_c262030 *) __asm__("func_L00_001FF500");



/* Adapted from Lombyte (MIT) for PAL by OpenRAC's tools/port.py: src/overlays/shared/gameplay/state/00261968.c, FUN_L00_00262030. */
s32 FUN_L00_00262030(f32 distance, s32 path_index, OvlVec4_630A8_c262030 *point, OvlVec4_630A8_c262030 *out) {
    OvlVec4_630A8_c262030 difference;
    OvlVec4_630A8_c262030 scaled_difference;
    OvlVec4_630A8_c262030 working;
    OvlVec4_630A8_c262030 segment;
    OvlVec4_630A8_c262030 scaled_segment;
    OvlVec4_630A8_c262030 cross;
    OvlVec4_630A8_c262030 scaled_cross;
    SegmentPath_262030_c262030 *entry;
    OvlVec4_630A8_c262030 *height_axis;
    OvlVec4_630A8_c262030 *current;
    s32 next;
    s32 i;
    s32 offset;
    OvlVec4_630A8_c262030 *saved_out;
    s32 found;
    f32 original_height;
    f32 segment_length;
    f32 projection;

    found = 0;
    saved_out = out;
    qcopy(&working, point);
    entry = D_L00_001B0830_630A8[path_index];
    offset = (s32)&D_0013F5E0;
    original_height = func_001F9C78_630A8(&working, (OvlVec4_630A8_c262030 *)offset);
    i = 0;
    if (entry->count - 1 > found) {
        height_axis = (OvlVec4_630A8_c262030 *)offset;
        do {
            offset = i << 4;
            next = i + 1;
            if (((OvlVec4_630A8_c262030 *)((u8 *)entry + offset + 0x10))->f[3] != 0.0f || entry->points[next].f[3] != 0.0f) {
                current = (OvlVec4_630A8_c262030 *)((u8 *)entry + (offset + 0x10));
                func_001F9BF0_630A8(&difference, &working, current);
                func_001F9BF0_630A8(&segment, (OvlVec4_630A8_c262030 *)((u8 *)entry + (offset + 0x20)), current);
                projection = func_001F9C78_630A8(&difference, height_axis);
                func_L00_001FF4B0_630A8(projection, &scaled_cross, height_axis);
                func_001F9BF0_630A8(&difference, &difference, &scaled_cross);
                projection = func_001F9C78_630A8(&segment, height_axis);
                func_L00_001FF4B0_630A8(projection, &scaled_cross, height_axis);
                func_001F9BF0_630A8(&segment, &segment, &scaled_cross);
                func_L00_001FF4B0_630A8(1.0f, &scaled_segment, &segment);
                func_001F9CA0_630A8(&cross, &difference, &scaled_segment);
                segment_length = func_001F99C0(func_001F9C78_630A8(&cross, height_axis));
                if (distance < segment_length)
                    goto next_segment_262030;
                segment_length = func_001F9CB8_630A8(&segment);
                projection = func_001F9C78_630A8(&difference, &scaled_segment);
                if (segment_length < projection || projection < 0.0f) {
                    segment_length = func_001F9CB8_630A8(&difference);
                    if (!(segment_length < distance))
                        goto next_segment_262030;
                    func_L00_001FF4B0_630A8(distance, &scaled_difference, &difference);
                    func_001F9BD8_630A8(&working, &scaled_difference, current);
                    found = 1;
                } else {
                    found = 1;
                    func_L00_001FF4B0_630A8(projection, &scaled_cross, &scaled_segment);
                    func_001F9BF0_630A8(&scaled_difference, &difference, &scaled_cross);
                    func_L00_001FF4B0_630A8(distance, &scaled_difference, &scaled_difference);
                    func_001F9BD8_630A8(&scaled_difference, &scaled_difference, &scaled_cross);
                    func_001F9BD8_630A8(&working, &scaled_difference, current);
                }
            }
next_segment_262030:
            i = next;
        } while (i < entry->count - 1);
    }
    if ((i = found)) {
        projection = func_001F9C78_630A8(&working, &D_0013F5E0);
        func_L00_001FF4B0_630A8(original_height - projection, &segment, &D_0013F5E0);
        func_001F9BD8_630A8(saved_out, &working, &segment);
    }
    return found;
}
