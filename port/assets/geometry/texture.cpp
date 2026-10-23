// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-formats/src/texture.rs:
// ISC License, Copyright (c) 2026 ReRAC contributors.
//
// Indexed texture decoding and the RAC1 level texture tables.

#include "assets/geometry/texture.h"

#include <algorithm>
#include <format>

#include "assets/geometry/gs_adgif.h"

namespace openrac::assets::rac1 {

std::string_view texture_table_name(TextureTable table) {
    switch (table) {
        case TextureTable::Tfrag:
            return "tfrag";
        case TextureTable::Moby:
            return "moby";
        case TextureTable::Tie:
            return "tie";
        case TextureTable::Shrub:
            return "shrub";
        case TextureTable::Billboard:
            return "billboard";
    }

    return "unknown";
}

std::string LevelTexture::key() const {
    if (billboard) {
        return std::format(
            "billboard/{:04}_{}x{}", billboard_class, billboard->width, billboard->height
        );
    }

    return std::format(
        "{}/{:03}_{}x{}_t{}",
        texture_table_name(table),
        index,
        entry.width,
        entry.height,
        entry.levels
    );
}

namespace {

bool has_pixels(const CoreTextureEntry& e) {
    return e.width > 0 && e.height > 0 && e.data_offset >= 0 && e.palette >= 0;
}

std::size_t block(s16 b) {
    return static_cast<std::size_t>(b) * 0x100;
}

}  // namespace

IndexedImage entry_image(ByteView textures_block, ByteView gs_ram, const CoreTextureEntry& e) {
    if (!has_pixels(e)) {
        fail("texture entry has no pixels");
    }

    const auto w = static_cast<u32>(e.width);
    const auto h = static_cast<u32>(e.height);
    return {
        w,
        h,
        textures_block
            .sub(static_cast<std::size_t>(e.data_offset), std::size_t{w} * h, "texture pixels"),
        gs_ram.sub(block(e.palette), 1024, "texture palette"),
    };
}

IndexedImage billboard_image(ByteView gs_ram, const CoreBillboardInfo& b) {
    if (b.texture_block < 0 || b.palette_block < 0 || b.width <= 0 || b.height <= 0) {
        fail("billboard: negative gs_ram block or no size");
    }

    const auto w = static_cast<u32>(b.width);
    const auto h = static_cast<u32>(b.height);
    return {
        w,
        h,
        gs_ram.sub(block(b.texture_block), std::size_t{w} * h, "billboard pixels"),
        gs_ram.sub(block(b.palette_block), 1024, "billboard palette"),
    };
}

std::vector<LevelTexture> decode_level_textures(
    const TextureTables& tables, ByteView textures_block, ByteView gs_ram
) {
    std::vector<LevelTexture> out;
    const std::pair<TextureTable, std::span<const CoreTextureEntry>> lists[] = {
        {TextureTable::Tfrag, tables.tfrag},
        {TextureTable::Moby, tables.moby},
        {TextureTable::Tie, tables.tie},
        {TextureTable::Shrub, tables.shrub},
    };

    for (const auto& [table, entries] : lists) {
        for (std::size_t i = 0; i < entries.size(); ++i) {
            const CoreTextureEntry& e = entries[i];

            if (!has_pixels(e)) {
                continue;
            }

            LevelTexture t;
            t.table = table;
            t.index = i;
            t.entry = e;
            t.image = entry_image(textures_block, gs_ram, e).decode();
            out.push_back(std::move(t));
        }
    }

    for (std::size_t i = 0; i < tables.shrub_classes.size(); ++i) {
        const CoreShrubClassEntry& c = tables.shrub_classes[i];

        if (c.billboard.width <= 0 || c.billboard.height <= 0) {
            continue;
        }

        LevelTexture t;
        t.table = TextureTable::Billboard;
        t.index = i;
        t.billboard = c.billboard;
        t.billboard_class = c.base.o_class;
        t.image = billboard_image(gs_ram, c.billboard).decode();
        out.push_back(std::move(t));
    }

    return out;
}

std::vector<IndexedImage> tfrag_mip_images(
    ByteView textures_block, ByteView gs_ram, const CoreTextureEntry& e
) {
    if (!has_pixels(e)) {
        fail("tfrag texture entry has no pixels");
    }

    if (e.levels < 1 || e.levels > 4) {
        fail("tfrag texture level count {} outside 1..4", e.levels);
    }

    const ByteView clut = gs_ram.sub(block(e.palette), 1024, "texture palette");
    const auto w = static_cast<std::size_t>(e.width);
    const auto h = static_cast<std::size_t>(e.height);
    std::vector<IndexedImage> out;

    for (int level = 0; level < e.levels; ++level) {
        const std::size_t lw = w >> level;
        const std::size_t lh = h >> level;

        if (lw == 0 || lh == 0) {
            fail("tfrag mip level {} is smaller than one texel", level);
        }

        ByteView pixels;
        const auto base = static_cast<std::size_t>(e.data_offset);

        if (level == 0) {
            pixels = textures_block.sub(base, lw * lh, "mip 0 pixels");
        } else if (level == 1) {
            pixels = textures_block.sub(base + w * h, lw * lh, "mip 1 pixels");
        } else if (level == 2 && e.mip2_block >= 0) {
            pixels = gs_ram.sub(block(e.mip2_block), lw * lh, "mip 2 pixels");
        } else if (level == 3 && e.mip3_block >= 0) {
            pixels = gs_ram.sub(block(e.mip3_block), lw * lh, "mip 3 pixels");
        } else {
            fail("tfrag mip level {} has no gs_ram block", level);
        }

        out.push_back({static_cast<u32>(lw), static_cast<u32>(lh), pixels, clut});
    }

    return out;
}

std::vector<RgbaImage> decode_tfrag_mip_levels(
    ByteView textures_block, ByteView gs_ram, const CoreTextureEntry& e
) {
    std::vector<RgbaImage> out;

    for (const IndexedImage& image : tfrag_mip_images(textures_block, gs_ram, e)) {
        out.push_back(image.decode());
    }

    return out;
}

std::vector<IndexedImage> billboard_mip_images(ByteView gs_ram, const CoreBillboardInfo& b) {
    if (b.width <= 0 || b.height <= 0 || b.palette_block < 0) {
        fail("billboard has no texture");
    }

    if (b.levels < 1 || b.levels > 4) {
        fail("billboard level count {} outside 1..4", b.levels);
    }

    const ByteView clut = gs_ram.sub(block(b.palette_block), 1024, "billboard palette");
    const s16 blocks[] = {b.texture_block, b.mip1_block, b.mip2_block, b.mip3_block};
    std::vector<IndexedImage> out;

    for (int level = 0; level < b.levels; ++level) {
        const std::size_t lw = static_cast<std::size_t>(b.width) >> level;
        const std::size_t lh = static_cast<std::size_t>(b.height) >> level;

        if (lw == 0 || lh == 0) {
            fail("billboard mip level {} is smaller than one texel", level);
        }

        if (blocks[level] < 0) {
            fail("billboard mip level {} has no gs_ram block", level);
        }

        out.push_back(
            {static_cast<u32>(lw),
             static_cast<u32>(lh),
             gs_ram.sub(block(blocks[level]), lw * lh, "billboard mip pixels"),
             clut}
        );
    }

    return out;
}

std::vector<RgbaImage> decode_billboard_mip_levels(ByteView gs_ram, const CoreBillboardInfo& b) {
    std::vector<RgbaImage> out;

    for (const IndexedImage& image : billboard_mip_images(gs_ram, b)) {
        out.push_back(image.decode());
    }

    return out;
}

// The loader computes these in 64-bit signed registers; a negative block
// number (-1 for an absent mip level) spreads its sign into the high bits,
// as on the console.
u64 loaded_tex0(const CoreTextureEntry& t, u32 gs_base) {
    const s64 base = static_cast<s32>(gs_base) >> 8;
    const s32 w = t.width;  // a signed 16-bit width, as the loader reads it
    const s64 tbw = std::max(w >> 6, 1);
    const s64 v = tbw << 14 | ee_log2(w) << 26 | 0x0130'0000 | ee_log2(t.height) << 30
                  | (t.palette + base) << 37 | s64{1} << 34;
    return static_cast<u64>(v) | u64{1} << 63;
}

u64 loaded_tex1(s16 levels, s32 lo, s32 hi) {
    const s64 v = (s64{levels} - 1) << 2 | s64{hi} << 6 | 0x20;
    return static_cast<u64>(v) | u64{static_cast<u32>(lo)} << 32;
}

u64 loaded_clamp(s32 lo, s32 hi, s32 texture_index) {
    return static_cast<u64>(s64{lo} | s64{hi} << 2 | s64{texture_index} << 24);
}

u64 loaded_miptbp1(const CoreTextureEntry& t, u32 gs_base) {
    const s64 base = static_cast<s32>(gs_base) >> 8;
    const s64 tbw1 = std::max(s32{t.width} >> 7, 1);
    const s64 v = tbw1 << 14 | (t.mip2_block + base) << 20 | (t.mip3_block + base) << 40
                  | s64{1} << 34 | s64{1} << 54;
    return static_cast<u64>(v);
}

}  // namespace openrac::assets::rac1
