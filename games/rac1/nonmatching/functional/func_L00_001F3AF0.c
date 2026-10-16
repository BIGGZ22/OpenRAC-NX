/* functional: medium. The NTSC decomp's pending C for the same instructions (a draft it has not matched
   yet), carried to PAL's names by OpenRAC's tools/port.py. Never compared with retail's bytes here. */
extern u64 * volatile D_L00_00161280 MACRO_ADDR;
extern volatile u64 D_0015EFC8 MACRO_ADDR;
typedef struct {
    s32 width;
    s32 height;
    s32 half_width;
    s32 half_height;
    s32 left;
    s32 top;
} DebugViewport_F3AF0;
extern DebugViewport_F3AF0 D_0013E600;
extern s32 D_L00_00169A80[];
extern s32 D_L00_00169C00[];
extern void func_00234C98(s32, u64);

/* Adapted from Lombyte (MIT) for PAL by OpenRAC's tools/port.py: src/overlays/shared/rendering/debug/001f36f8.c, FUN_L00_001f3770. */
void func_L00_001F3AF0(s32 x, s32 y, s32 color, s32 text_address) {
    const u8 *text = (const u8 *)text_address;
    s32 glyph_count = 0;
    u32 quadwords;
    u32 glyph;
    s32 texture_coordinates;
    s32 left;
    s32 top;
    u64 *packet;
    volatile u64 *tags;
    u64 *glyph_tag;
    u64 *cursor;
    u64 texture_state;

    func_00234C98(0x42, 0x8000000044ULL);
    func_00234C98(0x47, 0x5360B);
    packet = D_L00_00161280;
    D_L00_00161280 = packet + 2;
    packet[2] = 0x10AB400000000001ULL;
    tags = packet + 2;
    tags[1] = 0xE;
    tags[2] = 1;
    tags[3] = 0x14;
    tags[4] = 0x2400000000000001ULL;
    tags[5] = 0x61;
    ((u64 *)tags)[6] = color & 0xFFFFFFFFULL;
    texture_state = D_0015EFC8;
    tags[9] = 0x5353;
    tags[7] = texture_state;
    tags[8] = 0x4400000000008003ULL;
    glyph_tag = packet + 10;
    D_L00_00161280 = D_L00_00161280 + 10;
    quadwords = 5;
    while (*text != 0) {
        cursor = D_L00_00161280;
        glyph = (u8)(*text - 0x20);
        if (glyph >= 0x60)
            glyph = 0x20;
        glyph *= 4;
        texture_coordinates = *(s32 *)((u8 *)D_L00_00169A80 + glyph);
        text++;
        if (texture_coordinates != -1) {
            cursor[0] = (s64)texture_coordinates;
            top = D_0013E600.top;
            left = D_0013E600.left;
            glyph_count++;
            cursor[2] = (s64)(texture_coordinates + 0xC000A0);
            cursor[1] = (s64)((x * 16) + left - 8) |
                        ((u64)(s64)((y * 16) + top - 8) << 16) | 0xFFFFF300000000ULL;
            quadwords += 2;
            ((volatile u64 *)cursor)[3] = (s64)(((x + 10) * 16) + D_0013E600.left - 8) |
                        ((u64)(s64)(((y + 12) * 16) + D_0013E600.top - 8) << 16) |
                        0xFFFFF300000000ULL;
            D_L00_00161280 = D_L00_00161280 + 4;
        }
        x += *(s32 *)((u8 *)D_L00_00169C00 + glyph);
    }
    ((u32 *)packet)[3] = quadwords | 0x50000000;
    ((u32 *)packet)[0] = quadwords | 0x10000000;
    ((u32 *)packet)[1] = 0;
    ((u32 *)packet)[2] = 0;
    *glyph_tag = (s64)(glyph_count | 0x8000) | 0x4400000000000000ULL;
}
