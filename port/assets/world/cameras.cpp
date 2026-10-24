// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-formats/src/cameras.rs:
// ISC License, Copyright (c) 2026 ReRAC contributors.
//
// Implements cameras.h: reads a level's camera records from the gameplay file and decodes the
// pvar block of each camera class into its own struct. Every decoder reads a block at fixed
// offsets after the shared header and returns nothing when the block is too short, so a record
// of an unknown or truncated class is left to the caller instead of throwing.

#include "assets/world/cameras.h"

#include <cstring>

namespace openrac::assets {

// A level with no camera section has no cameras, which is not an error.
std::vector<CameraRecord> read_camera_records(const GameplayFile& file) {
    const ByteView g = file.bytes();
    const std::size_t s = file.offset(GameplaySection::Cameras);

    // A zero offset means the level does not have this section.
    if (s == 0) {
        return {};
    }

    const s32 count = g.s32_at(s);

    // The runtime copies the records into a fixed set of slots, so more would overflow it.
    if (count < 0 || count > kCameraSlots) {
        fail("camera count {} (the runtime has 48 slots)", count);
    }

    std::vector<CameraRecord> out;

    // Records follow the 0x10-byte section header, in file order.
    for (std::size_t i = 0; i < static_cast<std::size_t>(count); ++i) {
        const std::size_t r = s + 0x10 + kCameraRecordSize * i;
        CameraRecord c;
        c.class_id = g.s32_at(r);

        // Position and rotation are three floats each, one float (4 bytes) per axis.
        for (std::size_t k = 0; k < 3; ++k) {
            c.position[k] = g.f32_at(r + 4 + 4 * k);
            c.rotation[k] = g.f32_at(r + 0x10 + 4 * k);
        }

        c.pvar_index = g.s32_at(r + 0x1c);
        out.push_back(c);
    }

    return out;
}

// Pairs each record with the loader's copy of its pvar block, when it has one.
std::vector<LevelCamera> read_level_cameras(const GameplayFile& file) {
    std::vector<LevelCamera> out;

    // One entry per record, in file order.
    for (const CameraRecord& r : read_camera_records(file)) {
        out.push_back({r, read_pvar_block(file, r.pvar_index)});
    }

    return out;
}

// The block's moby links hold instance indices in the file; the port's moby list is a different
// order, so each is replaced by the runtime index (or -1) that `instance_to_moby` returns.
void remap_camera_moby_links(
    std::vector<LevelCamera>& cameras,
    const GameplayFile& file,
    const std::function<std::optional<std::size_t>(std::size_t)>& instance_to_moby
) {
    const PvarFixups fixups = read_pvar_fixups(file);

    // Only cameras that have a block can hold links.
    for (LevelCamera& c : cameras) {
        // A camera with no block has nothing to remap.
        if (!c.pvar) {
            continue;
        }

        std::vector<u8>& block = *c.pvar;

        // The fixup list covers every block; pick this camera's entries.
        for (const PvarFixup& f : fixups.moby_links) {
            // Skip links of other blocks and any offset that would write past this block's end.
            if (f.pvar_index != c.record.pvar_index || f.offset < 0
                || static_cast<std::size_t>(f.offset) + 4 > block.size()) {
                continue;
            }

            s32 value;
            std::memcpy(&value, block.data() + f.offset, 4);

            // A negative value is the file's "no moby"; leave it as it is.
            if (value < 0) {
                continue;
            }

            const auto moby = instance_to_moby(static_cast<std::size_t>(value));
            const s32 runtime = moby ? static_cast<s32>(*moby) : -1;
            std::memcpy(block.data() + f.offset, &runtime, 4);
        }
    }
}

// The shared header is the first 0x20 bytes of every class's block.
std::optional<CameraHeader> CameraHeader::read(ByteView p) {
    // A block shorter than the header cannot be a camera block.
    if (p.size() < 0x20) {
        return std::nullopt;
    }

    CameraHeader h;
    h.w00 = p.f32_at(0);
    h.sphere = p.s32_at(0x08);
    h.cuboid = p.s32_at(0x0c);
    h.cylinder = p.s32_at(0x10);
    h.path = p.s32_at(0x14);
    h.f18 = p.f32_at(0x18);
    h.priority = p.u8_at(0x1c);
    h.blend = p.u8_at(0x1d);
    h.b1e = p.u8_at(0x1e);
    h.activation = p.u8_at(0x1f);
    return h;
}

// Class 17: the header, then the follow-camera tweak fields, which end at 0x58.
std::optional<CameraRegionTweak> CameraRegionTweak::read(ByteView p) {
    const auto header = CameraHeader::read(p);

    // Too short for the header or for this class's fields.
    if (!header || p.size() < 0x58) {
        return std::nullopt;
    }

    CameraRegionTweak t;
    t.header = *header;
    t.turn = p.f32_at(0);
    t.tolerance = p.f32_at(0x20);
    t.distance = p.f32_at(0x24);
    t.pivot_height = p.f32_at(0x28);
    t.mode = p.s16_at(0x2c);
    t.counter = p.s16_at(0x2e);
    t.look_height = p.f32_at(0x30);
    t.leash = p.s16_at(0x34);
    t.facing = p.s16_at(0x36);
    t.spring_k = p.f32_at(0x38);
    t.spring_d = p.f32_at(0x3c);
    t.from_k = p.f32_at(0x40);
    t.from_d = p.f32_at(0x44);
    t.once = p.s16_at(0x48);
    t.left = p.s16_at(0x4a);
    t.cuboid2 = p.s32_at(0x4c);
    t.pitch = p.f32_at(0x50);
    t.no_pitch = p.s16_at(0x54);
    t.no_yaw = p.s16_at(0x56);
    return t;
}

// Class 23: the header, then the placed view's fields, which end at 0x48.
std::optional<CameraPlacedView> CameraPlacedView::read(ByteView p) {
    const auto header = CameraHeader::read(p);

    // Too short for the header or for this class's fields.
    if (!header || p.size() < 0x48) {
        return std::nullopt;
    }

    CameraPlacedView v;
    v.header = *header;
    v.counter = p.s32_at(0x20);
    v.leash = p.s16_at(0x24);
    v.done = p.s16_at(0x26);
    v.pitch = p.f32_at(0x28);
    v.look_angle = p.f32_at(0x2c);
    v.f30 = p.f32_at(0x30);
    v.still = p.s32_at(0x34);
    v.distance = p.f32_at(0x38);
    v.height = p.f32_at(0x3c);
    v.look_height = p.f32_at(0x40);
    v.not_clank = p.s16_at(0x44);
    v.clank_only = p.s16_at(0x46);
    return v;
}

// Class 18: the header, then the moby focus fields, which end at 0x54.
std::optional<CameraMobyFocus> CameraMobyFocus::read(ByteView p) {
    const auto header = CameraHeader::read(p);

    // Too short for the header or for this class's fields.
    if (!header || p.size() < 0x54) {
        return std::nullopt;
    }

    CameraMobyFocus f;
    f.header = *header;
    f.turn = p.f32_at(0);
    f.counter = p.s16_at(0x20);
    f.near_kind = p.u8_at(0x22);
    f.radius = p.f32_at(0x24);
    f.moby = p.s32_at(0x28);
    f.max_pitch = p.f32_at(0x2c);
    f.max_angle = p.f32_at(0x30);
    f.distance = p.f32_at(0x34);
    f.pivot_height = p.f32_at(0x38);
    f.mode = p.s16_at(0x3c);
    f.counter2 = p.s16_at(0x3e);
    f.yaw = p.f32_at(0x40);
    f.group = p.s32_at(0x44);
    f.look_height = p.f32_at(0x4c);
    f.suppress = p.s32_at(0x50);
    return f;
}

// Class 3: the header, then the rail camera's fields, which end at 0x3c.
std::optional<CameraRail> CameraRail::read(ByteView p) {
    const auto header = CameraHeader::read(p);

    // Too short for the header or for this class's fields.
    if (!header || p.size() < 0x3c) {
        return std::nullopt;
    }

    CameraRail r;
    r.header = *header;
    r.ahead = p.f32_at(0);
    r.path = p.s32_at(0x20);
    r.rail = p.s32_at(0x24);
    r.map_path = p.s32_at(0x28);
    r.map_rail = p.s32_at(0x2c);
    r.mode = p.s32_at(0x30);
    r.mapped = p.s16_at(0x34);
    r.flipped = p.s16_at(0x36);
    r.along = p.f32_at(0x38);
    return r;
}

// Class 14: the header, then two floats, which end at 0x28.
std::optional<CameraSideView> CameraSideView::read(ByteView p) {
    const auto header = CameraHeader::read(p);

    // Too short for the header or for this class's fields.
    if (!header || p.size() < 0x28) {
        return std::nullopt;
    }

    return CameraSideView{*header, p.f32_at(0x20), p.f32_at(0x24)};
}

}  // namespace openrac::assets
