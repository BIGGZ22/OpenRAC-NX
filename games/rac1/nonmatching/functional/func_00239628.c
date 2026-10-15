/* functional: medium. The NTSC decomp's pending C for the same instructions (a draft it has not matched
   yet), carried to PAL's names by OpenRAC's tools/port.py. Never compared with retail's bytes here. */
extern volatile s32 D_001E6318[64];
extern s32 D_001E6418[64];
extern s32 D_0015F538 MACRO_ADDR;
extern void func_001F5800(s32, s32, s32, s32, s32, s32, s32, s32, s64, s64);
extern s32 func_001F98C0(s32);
extern f32 func_001FA888(s32);
extern s32 func_001FA898(f32);
extern s32 func_00200198(s32, s32);
extern u64 func_00200248(s32);

/* Adapted from Lombyte (MIT) for PAL by OpenRAC's tools/port.py: src/assembly/textbin/fun_00238310.c, render_capture_scrolling_text. */
void func_00239628(char *text, s32 start_x, s32 start_y, f32 scale) {
    s64 texture;
    s32 cursor_x;
    s32 cursor_y;
    s32 glyph_index;
    s32 blink_next_glyph;
    s32 skip_draw;
    s32 long_blink_period;
    s32 short_blink_period;
    s32 blink_period;
    s32 blink_remainder;
    char *cursor_text;
    s32 packed_coordinates;
    s32 texture_v;
    s32 texture_u;
    s32 glyph_width;
    s32 glyph_height;
    u8 character;

    blink_next_glyph = 0;
    cursor_text = text;
    texture = func_00200248(func_00200198(0xE935, 0));
    cursor_x = start_x;
    cursor_y = start_y;

    for (;;) {
        skip_draw = 0;
        character = (u8)*cursor_text;
        if (character == 0) {
            return;
        }
        cursor_text++;
        glyph_index = (s32)character - 0x20;

        if (glyph_index == 0x42) {
            blink_next_glyph = 1;
        }
        /* Retail retains this branch even though an unsigned byte minus 0x20 cannot reach 0xEA. */
        if (glyph_index == 0xEA) {
            cursor_x = start_x;
            cursor_y += 9;
        }

        if (glyph_index < 0x3B) {
            {
                volatile s32 *glyph_entry;
                glyph_entry = D_001E6318 + glyph_index;
                if (*glyph_entry != -1 || glyph_index == 0) {
                    if (blink_next_glyph != 0) {
                        skip_draw = 0;
                        blink_next_glyph = 0;
                        long_blink_period = func_001F98C0(0x1E);
                        short_blink_period = func_001F98C0(0xA);
                        blink_period = long_blink_period + short_blink_period;
                        blink_remainder = D_0015F538 % blink_period;
                        long_blink_period = func_001F98C0(0xA);
                        if (blink_remainder < long_blink_period) {
                            skip_draw = 1;
                        }
                    }

                    if (glyph_index != 0 && skip_draw == 0) {
                        packed_coordinates = *glyph_entry;
                        texture_v = (u32)packed_coordinates >> 0x14;
                        texture_u = (packed_coordinates & 0xFFFF) >> 4;

                        if (cursor_x + 9 > 0) {
                            if (cursor_x < 0x100) {
                                glyph_width =
                                    func_001FA898(func_001FA888(9) * scale);
                                glyph_height =
                                    func_001FA898(func_001FA888(9) * scale);
                                func_001F5800(cursor_x, cursor_y, glyph_width, glyph_height,
                                                   texture_u, texture_v, 9, 9, (s64)0x80404040,
                                                   texture);
                            }
                        }
                    }

                    cursor_x += func_001FA898(
                        func_001FA888(D_001E6418[glyph_index]) * scale);
                }
            }
        }
    }
}
