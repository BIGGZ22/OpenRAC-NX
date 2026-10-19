/* functional: medium. fcheck: unchecked (the harness could not compile it in place; OpenRAC tools/fcheck_gate.py, 2026-10-18). The NTSC decomp's pending C for the same instructions (a draft it has not matched
   yet), carried to PAL's names by OpenRAC's tools/port.py. Never compared with retail's bytes here. */
typedef u32 u128_04340 __attribute__((mode(TI), aligned(16)));
typedef struct ResidentRenderTextureDefinition_04340 {
    s32 unk0; /* unused here; a word, so the record is 4-byte aligned */
    s16 width;
    s16 height;
    s16 draw_control_count;
    s16 clut_block_offset; /* 0x0A: TEX0.CBP (bit 37) after adding the base */
    s16 mip_block_offset_0;
    s16 mip_block_offset_1;
} ResidentRenderTextureDefinition_04340;
typedef union MaterialMap_04340 {
    u128_04340 q;
    u8 b[16];
} MaterialMap_04340;
typedef struct {
    s16 width;
    s16 height;
    s16 draw_control_count;
    s16 texture_block_offset;
    s16 palette_block_offset;
    s16 mip_block_offset_0;
    s16 mip_block_offset_1;
    s16 mip_block_offset_2;
} ShrubMipTextureDefinition_04340;
typedef struct {
    s32 draw_high;
    s32 draw_shift;
    u8 pad8[8];
    s32 material_base;
    s32 material_shift;
    u8 pad18[8];
    s32 material_index;
    u8 pad24[0x1C];
} ShrubRenderPacket_04340;
typedef struct {
    u8 pad0[0x10];
    union {
        u64 d;
        s32 w[2];
    } tex1;
    u64 payload18;
    union {
        u64 d;
        s32 w[2];
    } tex0;
    u64 payload28;
    u64 miptbp1;
    u64 payload38;
} ShrubMipPacket_04340;
typedef struct {
    s32 count;
    s32 prefix_record_count;
} ShrubRenderGroupHeader_04340;
typedef struct {
    u8 pad0[0x10];
    ShrubRenderGroupHeader_04340 group_header;
} ShrubRenderGroup_04340;
typedef struct {
    ShrubRenderGroup_04340 *group;
    s32 pad4;
} ShrubRenderGroupReference_04340;
typedef struct {
    u8 pad0[0x10];
    f32 scale;
    u8 pad14[2];
    s16 runtime_count;
    s32 runtime_data;
    ShrubMipPacket_04340 *mip_packet;
    u8 pad20[6];
    u16 class_slot;
    s16 group_count;
    u8 pad2A[2];
    s32 packed_geometry;
    u8 pad30[0x10];
    ShrubRenderGroupReference_04340 groups[1];
} ShrubRenderClass_04340;
extern ShrubRenderClass_04340 * D_001D82C0[64];
extern s16 D_001D83C0[64];
extern u8 D_001D8440[0x400];
extern s32 D_001D9040[64];
extern MaterialMap_04340 D_001D9640[64];
extern s32 D_001604CC MACRO_ADDR;
extern s32 D_0015EF8C MACRO_ADDR;
extern u64 D_0019E640[0x30];
extern s32 func_001FA898(f32);
extern s32 func_001F9968(s32);

/* Adapted from Lombyte (MIT) for PAL by OpenRAC's tools/port.py: src/assembly/textbin/fun_00203b08.c, register_shrub_render_class. */
void func_00204340(ShrubRenderClass_04340 *render_class,
                                 ResidentRenderTextureDefinition_04340 *textures, u128_04340 *material_map,
                                 ShrubMipTextureDefinition_04340 *mip_texture, s32 class_id) {
    s32 group_index;
    s32 index;
    s32 template_index;
    s32 material_index;
    s32 packet_material_base;
    s32 fixed_threshold;
    s32 mip_width_units[4];
    s32 draw_high;
    s32 draw_shift;
    s32 material_base;
    s32 material_shift;
    s32 width_units_64;
    s32 width_units_128;
    s32 width_log2;
    s32 height_log2;
    s32 gs_block_base;
    MaterialMap_04340 *slot_materials;
    ShrubMipPacket_04340 *mip_packet;
    ShrubRenderGroupHeader_04340 *group_header;
    ShrubRenderPacket_04340 *packet;
    ResidentRenderTextureDefinition_04340 *texture;
    u64 draw_word;
    u64 material_word;
    u64 mip_word;
    u64 texture_word;
    D_001D8440[class_id] = D_001604CC;
    D_001D83C0[D_001604CC] = class_id;
    D_001D82C0[D_001604CC] = render_class;
    render_class->class_slot = D_001604CC;
    fixed_threshold = func_001FA898(render_class->scale * 1024.0f);
    D_001D9040[D_001604CC] = fixed_threshold;
    D_001604CC++;
    render_class->runtime_data = 0;
    render_class->runtime_count = 0;
    if (render_class->packed_geometry != 0) {
        render_class->packed_geometry += (s32)render_class;
    }
    for (group_index = 0; group_index < render_class->group_count; group_index++) {
        render_class->groups[group_index].group =
            (ShrubRenderGroup_04340 *)(((s32)render_class->groups[group_index].group) +
                                 ((s32)render_class));
    }

    if (render_class->mip_packet != 0) {
        mip_packet = (ShrubMipPacket_04340 *)(((s32)render_class->mip_packet) + ((s32)render_class));
        render_class->mip_packet = mip_packet;
        material_base = mip_packet->tex1.w[0];
        material_shift = mip_packet->tex1.w[1];
        if (mip_texture != 0) {
            for (index = 0; index < 4; index++) {
                mip_width_units[index] = mip_texture->width >> (index + 6);
                if (mip_width_units[index] <= 0) {
                    mip_width_units[index] = 1;
                }
            }

            width_log2 = func_001F9968(mip_texture->width);
            height_log2 = func_001F9968(mip_texture->height);
            draw_word = (((u64)material_base) << 32) | (((u64)material_shift) << 6) | 0x20;
            draw_word |= ((u64)(mip_texture->draw_control_count - 1)) << 2;
            mip_packet->tex1.d = draw_word;
            gs_block_base = D_0015EF8C >> 8;
            mip_packet->tex0.d = ((u64)(mip_texture->palette_block_offset + gs_block_base)) |
                                 ((u64)mip_width_units[0] << 14) |
                                 ((u64)(mip_texture->texture_block_offset + gs_block_base) << 37) |
                                 0x1300000ULL | ((u64)width_log2 << 26) | ((u64)height_log2 << 30) |
                                 (1ULL << 34) | (1ULL << 63);
            mip_packet->miptbp1 = (((u64)(mip_texture->mip_block_offset_2 + gs_block_base)) << 40) |
                                  (((u64)mip_width_units[1]) << 14) |
                                  (((u64)(mip_texture->mip_block_offset_1 + gs_block_base)) << 20) |
                                  ((u64)(mip_texture->mip_block_offset_0 + gs_block_base)) |
                                  (((u64)mip_width_units[3]) << 54) |
                                  (((u64)mip_width_units[2]) << 34);
        } else {
            for (template_index = 0; D_0019E640[(template_index + 1) * 3] != 0;
                 template_index++) {
            }

            material_word = ((((u64)material_base) << 32) | (((u64)material_shift) << 6)) |
                            (D_0019E640[(template_index * 3) + 1] & 0x1C) | 0x20;
            mip_packet->tex0.d = D_0019E640[template_index * 3];
            mip_packet->miptbp1 = D_0019E640[(template_index * 3) + 2];
            mip_packet->tex1.d = material_word;
        }
    }
    /* Copy all sixteen selectors, as one quadword, before rebuilding packets. */
    slot_materials = &D_001D9640[D_001D8440[class_id]];
    qcopy(slot_materials, material_map);
    for (group_index = 0; group_index < render_class->group_count; group_index++) {
        group_header = &render_class->groups[group_index].group->group_header;
        packet = (ShrubRenderPacket_04340 *)((((u8 *)group_header) +
                                        (group_header->prefix_record_count * 16)) +
                                       0x10);
        for (index = 0; index < group_header->count; index++) {
            material_index = slot_materials->b[packet->material_index];
            material_base = packet->material_base;
            /* The original material control survives the width scratch below. */
            packet_material_base = material_base;
            draw_high = packet->draw_high;
            draw_shift = packet->draw_shift;
            material_shift = packet->material_shift;
            if (textures != 0) {
                texture = &textures[material_index];
                width_units_64 = (s32)((u32)(u16)texture->width << 16) >> 22;
                width_units_128 = (s32)((u32)(u16)texture->width << 16) >> 23;
                if (width_units_64 <= 0) {
                    width_units_64 = 1;
                }
                if (width_units_128 <= 0) {
                    width_units_128 = 1;
                }
                width_log2 = func_001F9968((s16)texture->width);
                height_log2 = func_001F9968(texture->height);
                gs_block_base = D_0015EF8C >> 8;
                draw_word = (((u64)draw_high) << 32) |
                            (((u64)(texture->draw_control_count - 1)) << 2) |
                            (((u64)draw_shift) << 6) | 0x20;
                material_word = (packet_material_base | (((u64)material_shift) << 2)) |
                                (((u64)material_index) << 24);
                mip_word = (((u64)(texture->mip_block_offset_1 + gs_block_base) << 40) |
                            (((u64)1) << 34)) |
                           (((u64)width_units_128 << 14) |
                            (((u64)(texture->mip_block_offset_0 + gs_block_base)) << 20));
                mip_word |= ((u64)1) << 54;
                texture_word =
                    ((((u64)width_units_64 << 14) | (((u64)width_log2 << 26) | 0x1300000)) |
                     ((u64)height_log2 << 30)) |
                    ((((u64)(texture->clut_block_offset + gs_block_base)) << 37) |
                     (((u64)1) << 34));
                texture_word |= ((u64)1) << 63;
                *((u64 *)(&packet->draw_high)) = draw_word;
                *((u64 *)(&packet->material_base)) = material_word;
                *((u64 *)(&packet->material_index)) = mip_word;
                *((u64 *)(((u8 *)packet) + 0x30)) = texture_word;
            } else {
                draw_word = (((u64)draw_high) << 32) | (((u64)draw_shift) << 6) | 0x20 |
                            (D_0019E640[(material_index * 3) + 1] & 0x1C);
                material_word = (((u64)material_index) << 24) |
                                (packet_material_base | (((u64)material_shift) << 2));
                *((u64 *)(((u8 *)packet) + 0x30)) = D_0019E640[material_index * 3];
                *((u64 *)(&packet->material_base)) = material_word;
                *((u64 *)(&packet->draw_high)) = draw_word;
                *((u64 *)(&packet->material_index)) =
                    D_0019E640[(material_index * 3) + 2];
            }
            packet++;
        }
    }
}
