/* Carried from the PAL decompilation's src/game/pause.c (matched C there) (games/rac1) through the version
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
struct DisplayDims_284E8_c2271d0 {
    u8 pad_0[0x150];
    s16 display_width; /* 0x150 */
    s16 display_height; /* 0x152 */
};
extern struct DisplayDims_284E8_c2271d0 D_00151880_284E8 __asm__("D_00151780");
extern s32 *D_00161000_284E8 __asm__("D_00160F00") MACRO_ADDR;
extern void func_00234C98_284E8(s32, s64) __asm__("func_00233980");



/* Appends a DMA packet drawing 32-pixel-wide sprite strips over the screen in a fixed grey (0x7F808080). */
void FUN_002271d0(void) {
    s32 *base;
    s64 *commands;
    s64 *vertices;
    struct DisplayDims_284E8_c2271d0 *dimensions;
    s32 strip_count;
    s32 display_height;
    s32 display_width;
    s32 strip_index;
    s32 left_x;
    s32 top_y;
    s32 right_x;
    s32 bottom_y;
    s32 negative_half_width;
    s32 dividend;
    s64 packed_top_y;
    s64 packed_bottom_y;
    s64 first;
    s64 second;

    dimensions = &D_00151880_284E8;
    display_width = dimensions->display_width;
    display_height = dimensions->display_height;
    dividend = (display_width > -1) ? display_width : (display_width + 0x1F);
    strip_count = dividend >> 5;
    func_00234C98_284E8(0x42, 0x64);
    strip_index = 0;
    D_00161000_284E8[0] = (strip_count + 5) | 0x10000000;
    D_00161000_284E8[1] = 0;
    D_00161000_284E8[2] = 0;
    D_00161000_284E8[3] = (strip_count + 5) | 0x50000000;
    base = D_00161000_284E8;
    commands = (s64 *)(base + 4);
    D_00161000_284E8 = (s32 *)commands;
    ((s64 *)base)[2] = 0x1000000000000001;
    commands[1] = 0xE;
    commands[2] = 0x31001;
    commands[3] = 0x47;
    commands[4] = 0x2400000000008001;
    commands[5] = 0x10;
    commands[6] = 0x146;
    commands[7] = 0x7F808080;
    commands[8] = (s64)(strip_count | 0x8000) | 0x2400000000000000;
    commands[9] = 0x44;
    if (strip_index < strip_count) {
        bottom_y = display_height * 8 + 0x7FF0;
        top_y = 0x8000 - display_height * 8;
        negative_half_width = -(display_width * 8);
        packed_top_y = (s64)top_y << 16;
        left_x = negative_half_width + 0x8000;
        right_x = negative_half_width + 0x8200;
        packed_bottom_y = (s64)bottom_y << 16;
        vertices = (s64 *)(base + 0x18);
        do {
            first = (s64)left_x | packed_top_y;
            second = (s64)right_x | packed_bottom_y;
            *vertices = first;
            strip_index++;
            vertices++;
            right_x += 0x200;
            *vertices = second;
            left_x += 0x200;
            vertices++;
        } while (strip_index < strip_count);
    }
    D_00161000_284E8 = (s32 *)((u8 *)D_00161000_284E8 + (strip_count * 0x10 + 0x50));
}
