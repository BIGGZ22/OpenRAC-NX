// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The gadget readers (gadget.h). A gadget is a moby class stored as its own WAD stream, so this
// file finds that stream, unpacks it and parses the blob with the ordinary moby reader. It also
// resolves a class's texture slots to decoded images and reads the joint lists that say which
// joint a weapon attaches to. Every offset comes from untrusted disc data, so each one is
// checked and a bad one throws an AssetError that names the gadget or class.

#include "assets/geometry/gadget.h"

#include "assets/disc/wad.h"

namespace openrac::assets::rac1 {

// Reads every gadget's class: validates the table entry, unpacks the stream, and parses the
// blob. The result is in table order and each element owns its decompressed blob.
std::vector<GadgetClass> parse_gadget_classes(
    std::span<const CoreGadgetEntry> gadgets,
    std::span<const CoreClassEntry> moby_classes,
    ByteView core_data
) {
    std::vector<GadgetClass> out;
    out.reserve(gadgets.size());

    // One gadget at a time: validate its table entry, unpack its stream, attach its class entry.
    for (const CoreGadgetEntry& g : gadgets) {
        const s32 o = g.o_class;

        // An entry with no offset or less than a WAD header's worth of bytes cannot be a stream.
        if (g.offset <= 0 || g.compressed_size < 16) {
            fail("gadget {}: bad table entry", o);
        }

        const ByteView stream = core_data.sub(
            static_cast<std::size_t>(g.offset),
            static_cast<std::size_t>(g.compressed_size),
            "gadget stream"
        );

        // The stream must be a WAD whose own size field agrees with the table, or the table is
        // wrong.
        if (!disc::is_wad(stream)
            || disc::wad_compressed_size(stream) != static_cast<u32>(g.compressed_size)) {
            fail("gadget {}: not a WAD stream of the table's size", o);
        }

        const CoreClassEntry* entry = nullptr;

        // Find the class table entry that shares this gadget's class number.
        for (const CoreClassEntry& e : moby_classes) {
            // Match on the class number, not the position: the two tables are not in the same
            // order.
            if (e.o_class == o) {
                entry = &e;
                break;
            }
        }

        // A gadget is useless without its class entry (textures, scale, ...), so refuse it.
        if (entry == nullptr) {
            fail("gadget {}: no moby class table entry", o);
        }

        GadgetClass gc{g, disc::wad_decompress(stream), {}};

        // The blob is untrusted data: turn a parse failure into an error that names the gadget.
        try {
            gc.moby = {*entry, parse_moby_class(gc.blob)};
        } catch (const AssetError& error) {
            fail("gadget {}: {}", o, error.what());
        }

        out.push_back(std::move(gc));
    }

    return out;
}

// Decodes the textures of a class: one entry per used slot, with the image absent when the
// texture table entry has no pixels.
std::vector<ClassTexture> class_textures(
    std::span<const CoreTextureEntry> moby_textures,
    ByteView textures_block,
    ByteView gs_ram,
    const CoreClassEntry& entry
) {
    std::vector<ClassTexture> out;

    // One pass per texture slot of the class.
    for (std::size_t slot = 0; slot < entry.textures.size(); ++slot) {
        const u8 ix = entry.textures[slot];

        // 0xff marks an unused slot.
        if (ix == 0xff) {
            continue;
        }

        // The slot's index must lie inside the moby texture table.
        if (ix >= moby_textures.size()) {
            fail("class {}: texture slot {} past the moby texture table", entry.o_class, slot);
        }

        const CoreTextureEntry& e = moby_textures[ix];
        ClassTexture t{slot, ix, std::nullopt};

        // A texture with no pixels or no palette stays without an image rather than failing.
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

// Reads one joint list: the chain of joints from the root and the second list that follows it,
// both checked against the 0xff terminator.
std::pair<std::vector<u8>, std::vector<u8>> joint_list(
    ByteView blob, const MobyClassHeader& header, std::size_t list
) {
    // The joints pointer is 0 when the class has no joint lists at all.
    if (header.joints <= 0) {
        fail("class has no joint lists");
    }

    const auto base = static_cast<std::size_t>(header.joints);
    const s32 count = blob.s32_at(base);

    // The list index must lie inside the count stored at the head of the list table.
    if (count < 0 || list >= static_cast<std::size_t>(count)) {
        fail("joint list {} of {}", list, count);
    }

    const s32 p = blob.s32_at(base + 4 + 4 * list);

    // A null pointer means the list was never written.
    if (p <= 0) {
        fail("null joint list pointer");
    }

    const auto at = static_cast<std::size_t>(p);
    const s16 n1 = blob.s16_at(at);
    const s16 n2 = blob.s16_at(at + 2);

    // Lengths are signed on disc, so a negative one means corruption.
    if (n1 < 0 || n2 < 0) {
        fail("negative joint list length");
    }

    const auto a = static_cast<std::size_t>(n1);
    const auto b = static_cast<std::size_t>(n2);
    std::vector<u8> first = blob.sub(at + 4, a, "joint list").to_vector();
    std::vector<u8> second = blob.sub(at + 4 + a, b, "joint list").to_vector();

    // Every list ends in a 0xff byte, which doubles as a check that the lengths were right.
    if (blob.u8_at(at + 4 + a + b) != 0xff) {
        fail("joint list without its 0xff terminator");
    }

    return {std::move(first), std::move(second)};
}

// The joint a gadget attaches at: the last joint of the first chain of a joint list.
u8 attachment_joint(ByteView blob, const MobyClassHeader& header, std::size_t list) {
    const auto [chain, rest] = joint_list(blob, header, list);

    // An attachment is the end of a chain, so an empty chain has nothing to attach to.
    if (chain.empty()) {
        fail("empty attachment chain");
    }

    // The joint must be a real joint of this class, not a stale index.
    if (chain.back() >= header.joint_count) {
        fail("attachment joint {} >= joint count {}", chain.back(), header.joint_count);
    }

    return chain.back();
}

}  // namespace openrac::assets::rac1
