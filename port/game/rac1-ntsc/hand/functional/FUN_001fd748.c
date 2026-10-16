/* Carried from the PAL decompilation's src/game/help.c (matched C there) (games/rac1) through the version
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
typedef struct {
    s32 x;
    s32 y;
    s32 label_offset_x;
    s32 label_offset_y;
} LevelMapMarker_FDF78_c1fd748;
typedef struct {
    s32 label_text;
    s32 pad4[2];
} LevelMapMarkerText_FDF78_c1fd748;
extern LevelMapMarkerText_FDF78_c1fd748 D_001DDD44[19];
extern LevelMapMarker_FDF78_c1fd748 D_001DDE28[20];
struct MapState_FDF78_c1fd748 {
    u8 pad0[0x8];
    s32 unk8; /* 0x008: draw_map_overlay draft tests it with unk2C */
    u8 *mask; /* 0x00C: 512x512 1-bit explored mask of the current level */
    u8 pad10[0x4];
    s32 unk14; /* 0x014: passed with mask to func_001FA860__NOT_CARRIED */
    s32 z; /* 0x018: GS Z of the map quads (draw_map_overlay packets) */
    struct MapMarker *markers; /* 0x01C: drawn by draw_map_markers, may be null */
    struct MapIcon *icons; /* 0x020: icon list of the shown level */
    s32 unk24; /* 0x024: zero: update_map_zoom_and_pan and draw_map_overlay return early */
    s32 unk28; /* 0x028: zero: the save page is cleared, not packed */
    s32 unk2C; /* 0x02C: see unk8 */
    struct {
        s32 unk0; /* draw_map_overlay draft: <0 skipped */
        u8 pad4[0xC];
    } unk30[8]; /* 0x030 */
    s32 marker_count; /* 0x0B0: entries in markers */
    f32 zoom[20]; /* 0x0B4: per-map zoom, 0.65..4 */
    s32 pan_x[20]; /* 0x104 */
    s32 pan_y[20]; /* 0x154 */
    u8 pad1A4[0x80];
    s32 level; /* 0x224: selected level 0..19 */
    s32 loaded; /* 0x228: index of the loaded map, -1 none */
    s32 unk22C; /* 0x22C: stash_receive_data argument */
    s32 unk230; /* 0x230: compared with -current_level_index (draw_map_screen) */
    s32 unk234; /* 0x234: stash_receive_data byte count */
    u8 pad238[0x4];
    struct MapHdr *hdr; /* 0x23C */
    s32 map_image_vram; /* 0x240: TBP0 of the 512x512 composed map */
    s32 tex_clut_vram; /* 0x244: CBP shared by tex0..tex2 */
    s32 tex0_vram; /* 0x248: TBP0 of tex0 */
    s32 tex1_vram; /* 0x24C: TBP0 of tex1 */
    s32 tex2_vram; /* 0x250: TBP0 of tex2 */
    u8 pad254[0x4];
    s64 tex0; /* 0x258: TEX0 of the image at hdr +0x10 */
    s64 tex1; /* 0x260: hdr +0x14 */
    s64 tex2; /* 0x268: hdr +0x18 */
    u8 pad270[0x8];
    s32 slot[5]; /* 0x278: decoded map buffer per slot, 0 if free */
    s32 slot_id[5]; /* 0x28C: map id held by each slot, -1 if none */
    s32 sel; /* 0x2A0: slot being filled, -1 none */
    s32 slot_size[5]; /* 0x2A4 */
};
extern struct MapState_FDF78_c1fd748 D_001A00F0;
extern u8 D_0013DD40[20];
extern u8 D_0013DD58[20];
extern s32 D_0015ED80 MACRO_ADDR;
extern s32 D_0015F438 MACRO_ADDR;
extern short D_0015F690;
extern void func_001F4280(s32);
extern void func_001F4398(void);
extern s32 func_001F6250(u8 *, s32);
extern void func_001F6530(s32, s32, u64, u8 *, s32);
extern s32 func_001F96F8(s32);
extern f32 func_001F9B20(f32 *);
extern void *func_001FDD10(s32);
extern s32 func_001FF960(s32, s32);
extern void func_001FFC30(s32, s32, s32, s32, s32, s32);
extern void func_00200258(s32, s32, s32, s32, s32, s32, s32, s32);
extern void func_00200C80(s32, s32, s32, s32, u64, s32);
extern void func_00200E08(s32, s32, s32, s32, u64, s32);
extern void func_00233980(s32, u64);
static inline s32 map_availability_FDF78_c1fd748(s32 level_index) {
    if (D_0013DD58[level_index] != 0) {
        return 3;
    }
    if (D_0013DD40[level_index] != 0) {
        return 2;
    }
    return 0;
}



/* Adapted from Lombyte (MIT) for PAL by OpenRAC's tools/port.py: src/assembly/textbin/fun_001fd748.c, draw_level_selection_map. */
void FUN_001fd748(s32 left, s32 right, s32 top, s32 bottom) {
    f32 label_direction[4];
    s32 level_index;
    s32 availability;
    s32 screen_width_subpixels;
    s32 screen_height_subpixels;
    s32 marker_x;
    s32 marker_y;
    s32 label_x;
    s32 label_y;
    s32 line_start_x;
    s32 line_start_y;
    s32 direction_length;
    s32 line_fraction;
    s32 text_width;
    s32 line_end_x;
    s32 text_x;
    u8 *label_text;
    s32 texture_index;

    func_001F4280(0);
    func_00233980(0x42, 0x8000000044);
    func_00233980(0x47, 0x4B);
    func_00200E08(0, 0, 0x200, 0x1C0, 0x80000000, 0);
    texture_index = func_001FF960(0xE99A, 0xE);
    screen_width_subpixels = (right - left) * 16;
    screen_height_subpixels = (bottom - top) * 16;
    func_00200258(texture_index, 0, 0, screen_width_subpixels, screen_height_subpixels, 0, 0,
                       0x80);
    func_00233980(8, 0);
    func_00200258(func_001FF960(0xE99A, 0xF), 0, 0, screen_width_subpixels,
                       screen_height_subpixels, D_0015F438 & 0xFFF, 0, 0x80);
    func_00233980(8, 5);
    for (level_index = 1; level_index < 20; level_index++) {
        if (D_001DDE28[level_index].x == 0) {
            continue;
        }
        availability = map_availability_FDF78_c1fd748(level_index);
        if (availability == 0) {
            continue;
        }
        marker_x = D_001DDE28[level_index].x;
        marker_y = D_001DDE28[level_index].y;
        if (D_0015ED80 != 0) {
            marker_y = marker_y * 0x1C0 / 0x1A0;
        }
        if (availability == 3 ||
            (availability == 2 &&
             D_0015F438 % (func_001F96F8(0x16) + func_001F96F8(8)) <
                 func_001F96F8(0x16))) {
            func_001FFC30(func_001FF960(0xE99A, 0xC), marker_x - 5,
                            marker_y - 5, 10, 10, 0x80);
        }
        if (level_index == D_001A00F0.level) {
            label_x = marker_x + D_001DDE28[level_index].label_offset_x;
            label_y = marker_y + D_001DDE28[level_index].label_offset_y;
            label_direction[1] = D_001DDE28[level_index].label_offset_y;
            label_direction[0] = D_001DDE28[level_index].label_offset_x;
            direction_length = func_001F9B20(label_direction) * 1000.0f;
            line_fraction = direction_length - 8000;
            line_start_x = label_x + (marker_x - label_x) * line_fraction / direction_length;
            line_start_y = label_y + (marker_y - label_y) * line_fraction / direction_length;
            label_text = func_001FDD10(D_001DDD44[level_index].label_text);
            text_width = func_001F6250(label_text, -1);
            if (D_001DDE28[level_index].label_offset_x <= -1) {
                line_end_x = label_x - text_width;
            } else {
                line_end_x = label_x + text_width;
            }
            func_00200C80(line_start_x + 1, line_start_y + 1, label_x + 1, label_y + 1, 0x80000000,
                          0);
            func_00200C80(label_x + 1, label_y + 1, line_end_x + 1, label_y + 1, 0x80000000, 0);
            text_x = (line_end_x < label_x) ? line_end_x : label_x;
            func_001F6530(text_x + 1, label_y - (*(s32 *)&D_0015F690) + 1, 0x80000000, label_text,
                             -1);
            func_00200C80(line_start_x, line_start_y, label_x, label_y, 0x80F0F0F0, 0);
            func_00200C80(label_x, label_y, line_end_x, label_y, 0x80F0F0F0, 0);
            func_001F6530(text_x, label_y - (*(s32 *)&D_0015F690), 0x80F0F0F0, label_text, -1);
            func_001FFC30(func_001FF960(0xE99A, 0xD), marker_x - 10,
                            marker_y - 10, 20, 20, 0x80);
        }
    }
    func_001F4398();
}
