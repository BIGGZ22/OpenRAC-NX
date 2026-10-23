// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-formats/src/frontend.rs:
// ISC License, Copyright (c) 2026 ReRAC contributors.
//
// The title lump, the boot pictures and the title world as level data.

#include "assets/disc/frontend.h"

#include <algorithm>
#include <cstring>

#include "assets/disc/wad.h"

namespace openrac::assets::disc {

namespace {

std::vector<u8> maybe_decompress(ByteView lump) {
    return is_wad(lump) ? wad_decompress(lump) : lump.to_vector();
}

}  // namespace

TitleLump TitleLump::parse(ByteView lump) {
    TitleLump t;
    t.bytes = maybe_decompress(lump);
    const ByteView b(t.bytes);
    auto h = [&](std::size_t i) -> std::size_t {
        return b.u32_at(4 * i);
    };
    const std::size_t base = h(1);
    const std::size_t logo_size = std::size_t{kTitleLogoWidth} * kTitleLogoHeight * 4;
    t.logo =
        {kTitleLogoWidth,
         kTitleLogoHeight,
         b.sub(base + h(0x21), logo_size, "title logo").to_vector()};
    const std::size_t count = h(0x16);

    if (count > 256) {
        fail("title world: {} FX textures", count);
    }

    t.fx = b.read_array<FxTextureEntry>(h(0x17), count, "FX texture table");
    t.fx_bank = std::min(base + h(0x1a), t.bytes.size());
    return t;
}

std::size_t TitleLump::press_start_index(u32 language) const {
    return (language > 0 ? language - 1 : 0) + 4;
}

BootPictures BootPictures::parse(ByteView lump) {
    BootPictures p;
    p.m_bytes = maybe_decompress(lump);
    ByteView(p.m_bytes).check(0, 0x70, "IRX lump header");
    return p;
}

GsImage BootPictures::picture(BootPictureKind kind, bool pal, u32 language) const {
    std::size_t field = 0;

    switch (kind) {
        case BootPictureKind::Still:
            field = pal ? 8 : 0;
            break;
        case BootPictureKind::NoCard:
            field = 0x10 + 8 * std::size_t{language};
            break;
        case BootPictureKind::NoSpace:
            field = 0x40 + 8 * std::size_t{language};
            break;
    }

    const ByteView b(m_bytes);
    const std::vector<u8> raw =
        wad_decompress(b.sub(b.u32_at(field), b.u32_at(field + 4), "boot picture"));
    const std::size_t n = std::size_t{kBootPictureWidth} * kBootPictureHeight * 4;

    if (raw.size() < n) {
        fail("boot picture: {} bytes", raw.size());
    }

    return {
        kBootPictureWidth,
        kBootPictureHeight,
        std::vector<u8>(raw.begin(), raw.begin() + static_cast<std::ptrdiff_t>(n))
    };
}

TitleWorld TitleWorld::parse(ByteView lump) {
    const std::vector<u8> bytes = maybe_decompress(lump);
    const ByteView b(bytes);
    auto h = [&](std::size_t i) {
        return b.s32_at(4 * i);
    };
    const s32 base = h(1);

    if (base <= 0 || static_cast<std::size_t>(base) > bytes.size()) {
        fail("title world: data base {:#x}", base);
    }

    const auto base_u = static_cast<std::size_t>(base);
    const s32 p = static_cast<s32>(kIndexTables);
    auto range = [&](std::size_t n, std::size_t o) {
        return ArrayRange{h(n), h(o) + p};
    };
    LevelCoreHeader hd{};
    hd.gs_ram = range(2, 3);
    hd.tfrags = h(4);
    hd.sky = h(5);
    hd.moby_classes = range(6, 7);
    hd.tie_classes = range(8, 9);
    hd.shrub_classes = range(0xa, 0xb);
    hd.tfrag_textures = range(0xc, 0xd);
    hd.moby_textures = range(0xe, 0xf);
    hd.tie_textures = range(0x10, 0x11);
    hd.shrub_textures = range(0x12, 0x13);
    hd.part_textures = range(0x14, 0x15);
    hd.fx_textures = range(0x16, 0x17);
    hd.textures_base_offset = h(0x18);
    hd.part_bank_offset = h(0x19);
    hd.fx_bank_offset = h(0x1a);
    hd.part_defs_offset = h(0x1b) + p;
    hd.chrome_map_texture = h(0x1c);
    hd.chrome_map_palette = h(0x1d);

    TitleWorld w;
    w.data.assign(bytes.begin() + base, bytes.end());
    hd.assets_decompressed_size = static_cast<s32>(w.data.size());
    w.index.assign(kIndexTables, 0);
    std::memcpy(w.index.data(), &hd, sizeof hd);
    w.index.insert(w.index.end(), bytes.begin(), bytes.begin() + base);
    w.core = parse_level_core(w.index, w.data.size());
    const auto gs_at = static_cast<std::size_t>(h(0));
    w.gs = b.sub(gs_at, base_u > gs_at ? base_u - gs_at : 0, "title world GS image").to_vector();
    const ByteView data(w.data);
    w.gameplay =
        data.tail(static_cast<std::size_t>(h(0x1f)), "title world gameplay block").to_vector();
    const auto scene_at = static_cast<std::size_t>(h(0x20));
    const auto logo_at = static_cast<std::size_t>(h(0x21));
    w.scene =
        data.sub(scene_at, logo_at > scene_at ? logo_at - scene_at : 0, "title world scene chunks")
            .to_vector();
    return w;
}

}  // namespace openrac::assets::disc
