// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-formats/src/texture.rs:
// ISC License, Copyright (c) 2026 ReRAC contributors.
//
// The texture readers (texture.h). A texture entry names its pixels in the core data's textures
// block and its palette in gs_ram; this file slices those out as indexed images (IndexedImage),
// decodes them, and walks the level's tables to produce every texture the level uses. It also
// holds the register values the loader derives for a texture (TEX0, TEX1, CLAMP, MIPTBP1), which
// the game computes in 64-bit signed arithmetic, so the same sign behaviour is reproduced here.

#include "assets/geometry/texture.h"

#include <algorithm>
#include <format>

#include "assets/geometry/gs_adgif.h"

namespace openrac::assets::rac1 {

// The lower-case name of a table, used in texture keys and file names.
std::string_view texture_table_name(TextureTable table) {
    // Every enumerator is handled, so the fallthrough below only guards a corrupt value.
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

// A stable name for the texture, unique within a level: table, index and size (and the mip count
// for table textures).
std::string LevelTexture::key() const {
    // A billboard texture is keyed by its class and size, since it has no table slot of its own.
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

// Whether an entry has pixels and a palette at all; placeholders carry -1 or a zero size.
bool has_pixels(const CoreTextureEntry& e) {
    return e.width > 0 && e.height > 0 && e.data_offset >= 0 && e.palette >= 0;
}

// The byte offset of a gs_ram block: the GS counts memory in blocks of 0x100 bytes here.
std::size_t block(s16 b) {
    return static_cast<std::size_t>(b) * 0x100;
}

}  // namespace

// The indexed pixels and palette of a table entry, undecoded. Throws when the entry has none.
IndexedImage entry_image(ByteView textures_block, ByteView gs_ram, const CoreTextureEntry& e) {
    // An entry with no pixels or palette (a placeholder slot) cannot be turned into an image.
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

// The indexed pixels and palette of a shrub billboard, both in gs_ram.
IndexedImage billboard_image(ByteView gs_ram, const CoreBillboardInfo& b) {
    // A negative block is the "absent" marker, and a zero size leaves nothing to decode.
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

// Decodes every texture of a level: the four tables' entries that have pixels, then the shrub
// classes' billboards.
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

    // Walk the four ordinary tables, then the billboards below.
    for (const auto& [table, entries] : lists) {
        // Keep each entry's position in its table as its index, even when entries are skipped.
        for (std::size_t i = 0; i < entries.size(); ++i) {
            const CoreTextureEntry& e = entries[i];

            // Placeholder entries have no image, so they get no texture.
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

    // Shrub classes carry a billboard texture of their own after the table textures.
    for (std::size_t i = 0; i < tables.shrub_classes.size(); ++i) {
        const CoreShrubClassEntry& c = tables.shrub_classes[i];

        // A class with no billboard size has no billboard texture.
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

// The mip levels of a tfrag texture as indexed images, all sharing the entry's palette.
std::vector<IndexedImage> tfrag_mip_images(
    ByteView textures_block, ByteView gs_ram, const CoreTextureEntry& e
) {
    // Mip levels need the entry's pixels and palette just as the top level does.
    if (!has_pixels(e)) {
        fail("tfrag texture entry has no pixels");
    }

    // The GS has four mip levels at most (TEX1 MXL), and a class with none is corrupt.
    if (e.levels < 1 || e.levels > 4) {
        fail("tfrag texture level count {} outside 1..4", e.levels);
    }

    const ByteView clut = gs_ram.sub(block(e.palette), 1024, "texture palette");
    const auto w = static_cast<std::size_t>(e.width);
    const auto h = static_cast<std::size_t>(e.height);
    std::vector<IndexedImage> out;

    // One image per level, each half the size of the one before.
    for (int level = 0; level < e.levels; ++level) {
        const std::size_t lw = w >> level;
        const std::size_t lh = h >> level;

        // A level that has shrunk to nothing means the entry claims more levels than its size
        // allows.
        if (lw == 0 || lh == 0) {
            fail("tfrag mip level {} is smaller than one texel", level);
        }

        ByteView pixels;
        const auto base = static_cast<std::size_t>(e.data_offset);

        // Levels 0 and 1 follow each other in the textures block; levels 2 and 3 live in gs_ram.
        if (level == 0) {
            pixels = textures_block.sub(base, lw * lh, "mip 0 pixels");
        } else if (level == 1) {
            // Level 1 starts right after level 0's pixels.
            pixels = textures_block.sub(base + w * h, lw * lh, "mip 1 pixels");
        } else if (level == 2 && e.mip2_block >= 0) {
            // Level 2 is in gs_ram, if the entry names a block for it.
            pixels = gs_ram.sub(block(e.mip2_block), lw * lh, "mip 2 pixels");
        } else if (level == 3 && e.mip3_block >= 0) {
            // Level 3 is in gs_ram, if the entry names a block for it.
            pixels = gs_ram.sub(block(e.mip3_block), lw * lh, "mip 3 pixels");
        } else {
            // No block for this level: the entry is inconsistent.
            fail("tfrag mip level {} has no gs_ram block", level);
        }

        out.push_back({static_cast<u32>(lw), static_cast<u32>(lh), pixels, clut});
    }

    return out;
}

// The same mip levels decoded to RGBA.
std::vector<RgbaImage> decode_tfrag_mip_levels(
    ByteView textures_block, ByteView gs_ram, const CoreTextureEntry& e
) {
    std::vector<RgbaImage> out;

    // Decode each level in order.
    for (const IndexedImage& image : tfrag_mip_images(textures_block, gs_ram, e)) {
        out.push_back(image.decode());
    }

    return out;
}

// The mip levels of a billboard as indexed images; every level lives in gs_ram.
std::vector<IndexedImage> billboard_mip_images(ByteView gs_ram, const CoreBillboardInfo& b) {
    // A billboard needs a size and a palette to be decodable.
    if (b.width <= 0 || b.height <= 0 || b.palette_block < 0) {
        fail("billboard has no texture");
    }

    // The GS has four mip levels at most, and a billboard with none is corrupt.
    if (b.levels < 1 || b.levels > 4) {
        fail("billboard level count {} outside 1..4", b.levels);
    }

    const ByteView clut = gs_ram.sub(block(b.palette_block), 1024, "billboard palette");
    const s16 blocks[] = {b.texture_block, b.mip1_block, b.mip2_block, b.mip3_block};
    std::vector<IndexedImage> out;

    // One image per level, each half the size of the one before.
    for (int level = 0; level < b.levels; ++level) {
        const std::size_t lw = static_cast<std::size_t>(b.width) >> level;
        const std::size_t lh = static_cast<std::size_t>(b.height) >> level;

        // A level that has shrunk to nothing means the entry claims more levels than its size
        // allows.
        if (lw == 0 || lh == 0) {
            fail("billboard mip level {} is smaller than one texel", level);
        }

        // A negative block is the "absent" marker, so this level cannot be read.
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

// The same mip levels decoded to RGBA.
std::vector<RgbaImage> decode_billboard_mip_levels(ByteView gs_ram, const CoreBillboardInfo& b) {
    std::vector<RgbaImage> out;

    // Decode each level in order.
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

// TEX1 as the loader builds it: the mip level count and the two filter fields (`lo`, `hi`).
u64 loaded_tex1(s16 levels, s32 lo, s32 hi) {
    const s64 v = (s64{levels} - 1) << 2 | s64{hi} << 6 | 0x20;
    return static_cast<u64>(v) | u64{static_cast<u32>(lo)} << 32;
}

// CLAMP as the loader builds it from the two wrap-mode fields and the texture index.
u64 loaded_clamp(s32 lo, s32 hi, s32 texture_index) {
    return static_cast<u64>(s64{lo} | s64{hi} << 2 | s64{texture_index} << 24);
}

// MIPTBP1 as the loader builds it: the buffer width and the gs_ram blocks of mip levels 2 and 3.
u64 loaded_miptbp1(const CoreTextureEntry& t, u32 gs_base) {
    const s64 base = static_cast<s32>(gs_base) >> 8;
    const s64 tbw1 = std::max(s32{t.width} >> 7, 1);
    const s64 v = tbw1 << 14 | (t.mip2_block + base) << 20 | (t.mip3_block + base) << 40
                  | s64{1} << 34 | s64{1} << 54;
    return static_cast<u64>(v);
}

}  // namespace openrac::assets::rac1
