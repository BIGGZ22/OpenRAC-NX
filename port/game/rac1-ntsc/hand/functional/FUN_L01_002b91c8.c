/* Carried from the PAL decompilation's src/overlays/shared/vendor_002B90A8.c (matched C there) (games/rac1) through the version
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
extern short D_L01_00161220;
extern f32 D_L01_001CAD00[];
extern void func_001F9A28(void *, void *, void *);
extern void func_001F9D20(void *, void *, void *);
extern u32 func_001F9958(void *);
struct DmaTag_c2b91c8 {
    u32 tag;
    u32 addr;
    u32 vif0;
    u32 vif1;
};
union PacketCursor_c2b91c8 {
    struct DmaTag_c2b91c8 *tag;
    struct GifTag *gif;
    s32 *words;
    u8 *bytes;
    s32 addr;
};
extern union PacketCursor_c2b91c8 D_L01_001611C0 MACRO_ADDR;
extern void func_001F9A68(void *, void *, float);
extern char D_L01_00167240[];
void func_L01_002B8C08(f32 *pos);
void func_L01_002B8CB0(f32 *pos, s32 index, s32 mode);
extern short D_L01_00161228;
extern short D_L01_0016122C;
extern short D_L01_00161230;
extern short D_L01_00161234;
void func_L01_002B8F98(void);
extern void func_L01_00262BC0_BA380(void *, void *) __asm__("func_L01_00261C10");
extern void func_L01_00261DF8(f32 *);
extern void func_L01_002B80D0(f32 *);
extern u64 func_001F44B8(s32);



/* Adapted from Lombyte (MIT) for PAL by OpenRAC's tools/port.py: src/overlays/shared/gameplay/entities/002b8c08.c, FUN_L01_002b91c8. */
void FUN_L01_002b91c8(f32 *pos, s32 count) {
    f32 corners[4][4];
    f32 near_clip[4];
    f32 far_clip[4];
    f32 x_offset, y_offset, x_end, y_end, z;
    u32 near_bits, far_bits;
    s32 mask;
    s32 i, j, mode;
    s32 height_index, index;
    f32 *height;
    u32 *packet;
    u64 *data;

    func_L01_002B8F98();
    i = 0;
    if (count > 0) do {
        f32 *next_pos = pos + 0x464;
        u16 active = *(u16 *)((u8 *)pos + 0x1e);
        if (active == 0) goto next_entity;
        x_offset = pos[0] + D_L01_001CAD00[2];
        x_end = pos[0] - D_L01_001CAD00[2];
        corners[0][0] = x_offset;
        y_offset = pos[1] + D_L01_001CAD00[3];
        y_end = pos[1] - D_L01_001CAD00[3];
        corners[0][1] = y_offset;
        z = pos[2];
        corners[0][2] = z;
        corners[0][3] = 1.0f;
        corners[1][0] = x_end;
        corners[1][1] = y_offset;
        corners[1][2] = z;
        corners[1][3] = 1.0f;
        corners[2][0] = x_offset;
        corners[2][1] = y_end;
        corners[2][2] = z;
        corners[2][3] = 1.0f;
        corners[3][0] = x_end;
        corners[3][1] = y_end;
        corners[3][2] = z;
        corners[3][3] = 1.0f;
        near_bits = 0;
        far_bits = 0;
        for (j = 0; j < 4; j++) {
            func_001F9A28(near_clip, corners[j], D_L01_00167240);
            func_001F9A68(far_clip, near_clip, 1024.0f);
            func_001F9A68(near_clip, near_clip, 1024.0f);
            func_001F9D20(far_clip, far_clip, D_L01_00167240 - 0x80);
            func_001F9D20(near_clip, near_clip, D_L01_00167240 - 0x40);
            far_bits = (far_bits << 8) | func_001F9958(far_clip);
            near_bits = (near_bits << 8) | func_001F9958(near_clip);
        }
        if ((near_bits & 0x01010101) == 0x01010101) goto next_entity;
        if ((near_bits & 0x02020202) == 0x02020202) goto next_entity;
        if ((near_bits & 0x04040404) == 0x04040404) goto next_entity;
        if ((near_bits & 0x08080808) == 0x08080808) goto next_entity;
        if ((near_bits & 0x20202020) == 0x20202020) goto next_entity;

        func_L01_00262BC0_BA380((void *)0x70000000, (u8 *)pos + ((*(s32 *)&D_L01_00161220) * 0x5c0 + 0x50));
        func_L01_00261DF8(pos);
        func_L01_002B8C08(pos);
        func_L01_002B80D0(pos);
        if (*(u32 *)(pos + 0x11) & 1) {
            height = (f32 *)0x70002008;
            for (height_index = 0x10; height_index >= 0; height_index--, height += 3) *height = pos[2];
        }
        if (*(u32 *)(pos + 0x11) & 2) {
            height = (f32 *)0x700020c8;
            for (height_index = 0x10; height_index >= 0; height_index--, height += 0x33) *height = pos[2];
        }
        if (*(u32 *)(pos + 0x11) & 4) {
            height = (f32 *)0x70002cc8;
            for (height_index = 0x10; height_index >= 0; height_index--, height += 3) *height = pos[2];
        }
        if (*(u32 *)(pos + 0x11) & 8) {
            height = (f32 *)0x70002008;
            for (height_index = 0x10; height_index >= 0; height_index--, height += 0x33) *height = pos[2];
        }
        mode = (far_bits & 0x2f2f2f2f) != 0;
        if (!mode) {
            mode = 2;
            if (!(near_bits & 0x2f2f2f2f)) mode = 0;
        }
        mask = *(u16 *)((u8 *)pos + 0x1e);
        if (*(u32 *)(pos + 5) != (*(u32 *)&D_L01_00161228) || *(u32 *)(pos + 6) != (*(u32 *)&D_L01_0016122C) ||
            *(u8 *)(pos + 7) != (*(u32 *)&D_L01_00161230) || *(u8 *)((u8 *)pos + 0x1d) != (*(u32 *)&D_L01_00161234)) {
            D_L01_001611C0.words[0] = 0x10000005;
            D_L01_001611C0.words[1] = 0;
            D_L01_001611C0.words[2] = 0x11000000;
            D_L01_001611C0.words[3] = 0x50000005;
            packet = D_L01_001611C0.words;
            D_L01_001611C0.words += 4;
            data = (u64 *)(packet + 4);
            data[0] = 0x4000000000008001ULL;
            data[1] = 0xeeee;
            data[2] = ((u64)*(u8 *)((u8 *)pos + 0x1d) << 32) | 100;
            data[3] = 0x42;
            data[4] = ((u64)*(u8 *)(pos + 7) << 32) | 100;
            data[5] = 0x43;
            data[6] = func_001F44B8(*(s32 *)(pos + 6));
            data[7] = 6;
            data[8] = func_001F44B8(*(s32 *)(pos + 5));
            data[9] = 7;
            D_L01_001611C0.words = packet + 0x18;
            {
                u32 texture_id = *(u32 *)(pos + 5);
                u32 second_texture_id = *(u32 *)(pos + 6);
                (*(u32 *)&D_L01_00161228) = texture_id;
                (*(u32 *)&D_L01_0016122C) = second_texture_id;
            }
            (*(u32 *)&D_L01_00161230) = *(u8 *)(pos + 7);
            (*(u32 *)&D_L01_00161234) = *(u8 *)((u8 *)pos + 0x1d);
        }
        for (index = 0; index < 16; index++, mask = (s32)mask >> 1) {
            if (mask & 1) func_L01_002B8CB0(pos, index, mode);
        }
next_entity:
        i++;
        pos = next_pos;
    } while (i < count);
    D_L01_001611C0.words[0] = 0x10000000;
    D_L01_001611C0.words[1] = 0;
    D_L01_001611C0.words[2] = 0x13000000;
    D_L01_001611C0.words[3] = 0;
    D_L01_001611C0.words += 4;
}
