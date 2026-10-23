// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

#include "assets/geometry/gadget.h"

#include "assets/disc/wad.h"

namespace openrac::assets::rac1 {

std::vector<GadgetClass> parse_gadget_classes(
    std::span<const CoreGadgetEntry> gadgets,
    std::span<const CoreClassEntry> moby_classes,
    ByteView core_data
) {
    std::vector<GadgetClass> out;
    out.reserve(gadgets.size());

    for (const CoreGadgetEntry& g : gadgets) {
        const s32 o = g.o_class;

        if (g.offset <= 0 || g.compressed_size < 16) {
            fail("gadget {}: bad table entry", o);
        }

        const ByteView stream = core_data.sub(
            static_cast<std::size_t>(g.offset),
            static_cast<std::size_t>(g.compressed_size),
            "gadget stream"
        );

        if (!disc::is_wad(stream)
            || disc::wad_compressed_size(stream) != static_cast<u32>(g.compressed_size)) {
            fail("gadget {}: not a WAD stream of the table's size", o);
        }

        const CoreClassEntry* entry = nullptr;

        for (const CoreClassEntry& e : moby_classes) {
            if (e.o_class == o) {
                entry = &e;
                break;
            }
        }

        if (entry == nullptr) {
            fail("gadget {}: no moby class table entry", o);
        }

        GadgetClass gc{g, disc::wad_decompress(stream), {}};

        try {
            gc.moby = {*entry, parse_moby_class(gc.blob)};
        } catch (const AssetError& error) {
            fail("gadget {}: {}", o, error.what());
        }

        out.push_back(std::move(gc));
    }

    return out;
}

std::vector<ClassTexture> class_textures(
    std::span<const CoreTextureEntry> moby_textures,
    ByteView textures_block,
    ByteView gs_ram,
    const CoreClassEntry& entry
) {
    std::vector<ClassTexture> out;

    for (std::size_t slot = 0; slot < entry.textures.size(); ++slot) {
        const u8 ix = entry.textures[slot];

        if (ix == 0xff) {
            continue;
        }

        if (ix >= moby_textures.size()) {
            fail("class {}: texture slot {} past the moby texture table", entry.o_class, slot);
        }

        const CoreTextureEntry& e = moby_textures[ix];
        ClassTexture t{slot, ix, std::nullopt};

        if (e.width > 0 && e.height > 0 && e.data_offset >= 0 && e.palette >= 0) {
            const auto w = static_cast<u32>(e.width);
            const auto h = static_cast<u32>(e.height);
            t.image = decode_indexed8(
                textures_block.sub(
                    static_cast<std::size_t>(e.data_offset), std::size_t{w} * h, "texture pixels"
                ),
                w,
                h,
                gs_ram.sub(static_cast<std::size_t>(e.palette) * 0x100, 1024, "texture palette")
            );
        }

        out.push_back(std::move(t));
    }

    return out;
}

std::pair<std::vector<u8>, std::vector<u8>> joint_list(
    ByteView blob, const MobyClassHeader& header, std::size_t list
) {
    if (header.joints <= 0) {
        fail("class has no joint lists");
    }

    const auto base = static_cast<std::size_t>(header.joints);
    const s32 count = blob.s32_at(base);

    if (count < 0 || list >= static_cast<std::size_t>(count)) {
        fail("joint list {} of {}", list, count);
    }

    const s32 p = blob.s32_at(base + 4 + 4 * list);

    if (p <= 0) {
        fail("null joint list pointer");
    }

    const auto at = static_cast<std::size_t>(p);
    const s16 n1 = blob.s16_at(at);
    const s16 n2 = blob.s16_at(at + 2);

    if (n1 < 0 || n2 < 0) {
        fail("negative joint list length");
    }

    const auto a = static_cast<std::size_t>(n1);
    const auto b = static_cast<std::size_t>(n2);
    std::vector<u8> first = blob.sub(at + 4, a, "joint list").to_vector();
    std::vector<u8> second = blob.sub(at + 4 + a, b, "joint list").to_vector();

    if (blob.u8_at(at + 4 + a + b) != 0xff) {
        fail("joint list without its 0xff terminator");
    }

    return {std::move(first), std::move(second)};
}

u8 attachment_joint(ByteView blob, const MobyClassHeader& header, std::size_t list) {
    const auto [chain, rest] = joint_list(blob, header, list);

    if (chain.empty()) {
        fail("empty attachment chain");
    }

    if (chain.back() >= header.joint_count) {
        fail("attachment joint {} >= joint count {}", chain.back(), header.joint_count);
    }

    return chain.back();
}

}  // namespace openrac::assets::rac1
