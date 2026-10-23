// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-formats/src/vag.rs:
// ISC License, Copyright (c) 2026 ReRAC contributors.
//
// The VAG file header (vag.h).

#include "assets/sound/vag.h"

#include <algorithm>
#include <cstring>

namespace openrac::assets {

namespace {

u32 big_endian_u32(ByteView bytes, std::size_t offset) {
    bytes.check(offset, 4, "VAG header");
    const u8* p = bytes.data() + offset;
    return static_cast<u32>(p[0]) << 24 | static_cast<u32>(p[1]) << 16 | static_cast<u32>(p[2]) << 8
           | static_cast<u32>(p[3]);
}

}  // namespace

VagFile parse_vag(ByteView bytes) {
    if (bytes.size() < kVagHeaderSize || std::memcmp(bytes.data(), "VAGp", 4) != 0) {
        fail("not a VAGp file");
    }

    VagFile file;
    file.header.version = big_endian_u32(bytes, 0x04);
    file.header.data_size = big_endian_u32(bytes, 0x0c);
    file.header.sample_rate = big_endian_u32(bytes, 0x10);
    file.header.name = bytes.string_at(0x20, 16);
    const std::size_t end =
        std::min<std::size_t>(kVagHeaderSize + file.header.data_size, bytes.size());
    file.body = bytes.sub(kVagHeaderSize, end - kVagHeaderSize, "VAG body");
    return file;
}

}  // namespace openrac::assets
