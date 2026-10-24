/* Carried from the PAL decompilation's nonmatching/functional/func_00226978.c (games/rac1) through the version
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
extern void * D_001B3200[224];
extern u8 D_001B3AC0[0x800];
struct DiscFile_26978_c2256e8 {
    s32 sector;
    s32 size;
};
struct DiscTable_26978_c2256e8 {
    u8 pad_0[0x8];
    struct DiscFile_26978_c2256e8 debug_font; /* 0x8 */
    struct DiscFile_26978_c2256e8 memcard_data; /* 0x10, read by memcard_update_state and memcard_restore_game */
    struct DiscFile_26978_c2256e8 animation_streams[49]; /* 0x18, streamed animations by archive index */
    struct DiscFile_26978_c2256e8 music_10000[37]; /* 0x1A0, music tracks 10000.. */
    struct DiscFile_26978_c2256e8 unk2C8[6]; /* 0x2C8, one per language */
    struct DiscFile_26978_c2256e8 unk2F8[6]; /* 0x2F8, one per language */
    u8 pad_328[0x1D0];
    struct DiscFile_26978_c2256e8 level_chunk; /* 0x4F8, read by load_level_chunk_from_disc */
    u8 pad_500[0x3B8];
    /* 0x8B8: render archive sector/length records; extent reaches the next
       known field, not a proven count of valid archives. */
    struct DiscFile_26978_c2256e8 render_archives[201];
    s32 music_50000[40][6]; /* 0xF00, tracks 50000.., one location per language */
    struct DiscFile_26978_c2256e8 unk12C0; /* 0x12C0 */
    struct DiscFile_26978_c2256e8 level_archives[24]; /* 0x12C8, by level index */
    struct DiscFile_26978_c2256e8 loading_slides[6]; /* 0x1388, one per language */
    struct DiscFile_26978_c2256e8 unk13B8; /* 0x13B8 */
    struct DiscFile_26978_c2256e8 music_40000[36]; /* 0x13C0, music tracks 40000.. */
    s32 sound_bank; /* 0x14E0, passed to load_audio_bank_by_location */
    s32 pad_14E4;
    struct DiscFile_26978_c2256e8 unk14E8; /* 0x14E8 */
    u8 pad_14F0[0x8];
    struct DiscFile_26978_c2256e8 wad_chunks[6]; /* 0x14F8, level wad chunks */
    struct DiscFile_26978_c2256e8 help_text; /* 0x1528, per-language archive read by update_menu_help_text_load */
    struct DiscFile_26978_c2256e8 unk1530[14]; /* 0x1530, overlays: x of an x/y pair picked by D_0015EC80 */
    struct DiscFile_26978_c2256e8 unk15A0[14]; /* 0x15A0, the y half of the 0x1530 pair */
    struct DiscFile_26978_c2256e8 animation_table; /* 0x1610, read with animation_asset_read_active set */
    struct DiscFile_26978_c2256e8 music_60000[62]; /* 0x1618, music tracks 60000.. */
    struct DiscFile_26978_c2256e8 unk1808[19]; /* 0x1808, overlays: x of an x/y pair picked by D_0015EC80 */
    struct DiscFile_26978_c2256e8 unk18A0[19]; /* 0x18A0, the y half of the 0x1808 pair */
    struct DiscFile_26978_c2256e8 movies[12]; /* 0x1938, level transition movies */
    struct DiscFile_26978_c2256e8 movies_alt[12]; /* 0x1998, picked by the video mode flag */
    struct DiscFile_26978_c2256e8 unk19F8[3]; /* 0x19F8, overlays: x/y pair picked by D_0015EC80 */
    struct DiscFile_26978_c2256e8 unk1A10[3]; /* 0x1A10, the y half of the 0x19F8 pair */
    struct DiscFile_26978_c2256e8 unk1A28[5]; /* 0x1A28, overlays: x/y pair picked by D_0015EC80 */
    struct DiscFile_26978_c2256e8 unk1A50[5]; /* 0x1A50, the y half of the 0x1A28 pair */
    struct DiscFile_26978_c2256e8 start_movies[4]; /* 0x1A78, played by start_level */
    struct DiscFile_26978_c2256e8 start_movies_alt[4]; /* 0x1A98, while D_0015EC80 is set */
    u8 pad_1AB8[0xEB0];
    struct DiscFile_26978_c2256e8 shared_archive; /* 0x2968 */
    struct DiscFile_26978_c2256e8 sound_archive; /* 0x2970 */
    struct DiscFile_26978_c2256e8 sound_archive_alt; /* 0x2978, while D_0015EC80 is set */
    u8 pad_2980[0x8];
    struct DiscFile_26978_c2256e8 music_20000[36]; /* 0x2988, music tracks 20000.. */
    s32 music_tracks[1]; /* 0x2AA8, by track number */
};
extern struct DiscTable_26978_c2256e8 D_00137B80;
typedef struct {
    u8 pad_0[0x20];
    u8 moby_state;
    u8 pad_21[0x52 - 0x21];
    u8 primary_animation;
    u8 secondary_animation;
    u8 pad_54[0x70 - 0x54];
    u8 transition_flags;
    u8 pad_71[0xBC - 0x71];
    u8 animation_frame_counter;
} StreamedAnimationMoby_26978_c2256e8;
typedef struct {
    u8 pad_0[0x48];
    u8 *animation_tables[4];
} StreamedClassResource_26978_c2256e8;
typedef struct {
    s32 offset;
    s32 reserved4;
} AnimationTableHeader_26978_c2256e8;
typedef struct {
    u8 pad_0[0x34];
    s32 state;
    s32 pad_38;
    u8 *buffer;
    s32 read_offset;
    StreamedAnimationMoby_26978_c2256e8 *moby;
} MobyAnimationStream_26978_c2256e8;
extern s16 D_001516D8[];
extern s32 D_001516EC[];
extern s16 D_0015172A[];
extern s32 D_0015ED88 MACRO_ADDR;
extern s32 D_0015EE20 MACRO_ADDR;
extern u8 D_001D5CBB[];
extern s32 func_001F96F8(s32);
extern void func_002032E0(StreamedClassResource_26978_c2256e8 *, s32);
extern void func_0020B618(u8 *, u8 *);
extern void func_00212F90(void *, s32, s32, s32);
extern s32 func_00215B10(void);
extern s32 func_00216788(u8 *, s32, s32);
extern s32 func_00225DD8(u8 *);



/* Adapted from Lombyte (GPL-3.0) for PAL by OpenRAC's tools/port.py: src/assembly/textbin/fun_002256e8.c, update_streamed_moby_animation. */
s32 FUN_002256e8(MobyAnimationStream_26978_c2256e8 *stream) {
    StreamedAnimationMoby_26978_c2256e8 *moby;
    AnimationTableHeader_26978_c2256e8 *header;
    StreamedClassResource_26978_c2256e8 **class_resource;
    s32 table_index;
    s32 dialogue_column;
    u8 *animation_table;
    s32 animation_index;
    u16 class_slot;

    switch (stream->state) {
    case 0:
        if (D_001516D8[0] != 0) {
            break;
        }
        table_index = 0x4F000 - (D_00137B80.animation_table.size << 11);
        if (func_00216788(stream->buffer + table_index, D_00137B80.animation_table.sector,
                                    D_00137B80.animation_table.size) != 0) {
            stream->read_offset = table_index;
            stream->state = 1;
            D_001D5CBB[0] = 1;
            func_00225DD8(stream->buffer);
            return 0;
        }
        stream->state = 3;
        break;
    case 1:
        /* Decompress the completed read and relocate three animation tables. */
        if (D_001516D8[0] != 0) {
            break;
        }
        D_001D5CBB[0] = 0;
        func_0020B618(stream->buffer + stream->read_offset, stream->buffer);
        class_slot = D_001B3AC0[0x7A5];
        class_resource = (StreamedClassResource_26978_c2256e8 **)&D_001B3200[class_slot];
        header = (AnimationTableHeader_26978_c2256e8 *)stream->buffer;
        table_index = class_slot >> 8;
    next:
        animation_table = stream->buffer + ((AnimationTableHeader_26978_c2256e8 *)((u8 *)header + (table_index << 3)))->offset;
        table_index++;
        (*class_resource)->animation_tables[table_index] = animation_table;
        func_002032E0(*class_resource, table_index);
        if (table_index < 3) {
            goto next;
        }
        stream->state = 2;
        if (D_0015EE20 == 0) {
            stream->moby->moby_state = 0;
        } else {
            stream->moby->moby_state = 4;
        }
        stream->moby->animation_frame_counter = 0;
        break;
    case 2:
        /* Even phases queue dialogue; odd phases wait for animation completion. */
        moby = stream->moby;
        switch (moby->moby_state) {
        case 0:
        case 2:
        case 4:
            dialogue_column = D_0015ED88 - 1;
            if (dialogue_column < 0) {
                dialogue_column = 0;
            }
            animation_index = moby->moby_state >> 1;
            if (moby->animation_frame_counter == 0) {
                D_001516EC[0] = animation_index * 6 + dialogue_column + 60000;
            }
            moby->animation_frame_counter++;
            if (moby->animation_frame_counter > func_001F96F8(0x78)) {
                moby->animation_frame_counter = func_001F96F8(0x78);
            }
            if (moby->animation_frame_counter < func_001F96F8(0x78)) {
                return 0;
            }
            if (D_0015172A[0] != 3) {
                return 0;
            }
            moby->animation_frame_counter = 0;
            moby->moby_state++;
            func_00212F90(moby, animation_index + 1, 0, func_001F96F8(0x18));
            break;
        case 1:
        case 3:
        case 5:
            if (moby->animation_frame_counter == 0 &&
                moby->primary_animation == moby->secondary_animation) {
                func_00215B10();
                moby->animation_frame_counter = 1;
            }
            if (moby->transition_flags & 2) {
                if (moby->moby_state == 1) {
                    moby->moby_state = moby->moby_state + 1;
                } else {
                    moby->moby_state = 6;
                }
                moby->animation_frame_counter = 0;
                func_00212F90(moby, 0, 0, func_001F96F8(0x18));
                break;
            }
            break;
        case 6:
            moby->animation_frame_counter++;
            if (moby->animation_frame_counter > func_001F96F8(0xF0)) {
                moby->animation_frame_counter = func_001F96F8(0xF0);
            }
            if (moby->animation_frame_counter < func_001F96F8(0xF0)) {
                return 0;
            }
            if (D_0015EE20 == 0) {
                moby->moby_state = 0;
            } else {
                moby->moby_state = 4;
            }
            moby->animation_frame_counter = 0;
            break;
        }
        break;
    }
    return 0;
}
