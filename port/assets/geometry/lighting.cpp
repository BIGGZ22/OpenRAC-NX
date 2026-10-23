// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-formats/src/tfrag_light.rs:
// ISC License, Copyright (c) 2026 ReRAC contributors.
//
// The implementation of lighting.h: the shared inputs of RAC1's baked-lighting passes.
//
// The file has four parts: reading the light bank out of the gameplay file, building or reading the
// normal table, the ELF segment reader the table needs, and the VU0 lighting arithmetic the tie and
// shrub passes share. All arithmetic goes through the ps2 float helpers so results are bit-exact
// with the console's. Nothing here holds state; every function is safe to call from any thread.

#include "assets/geometry/lighting.h"

#include <cmath>
#include <numbers>

namespace openrac::assets::rac1 {

// Reads the section whose offset is the u32 at 0x04 of the gameplay file; the records start 0x10
// bytes in (after the s32 count and 12 bytes of padding).
LightBank parse_light_bank(ByteView gameplay) {
    const std::size_t section = gameplay.u32_at(0x04);
    const s32 n = gameplay.s32_at(section);

    // A negative count is a corrupt file, and the cap below would turn it into a huge size_t.
    if (n < 0) {
        fail("gameplay: negative directional light count {}", n);
    }

    LightBank bank;
    bank.count = std::min(static_cast<std::size_t>(n), kMaxLevelLights);
    const auto sets =
        gameplay.read_array<DirLightSet>(section + 0x10, bank.count, "directional lights");

    // Copy the file's records into the fixed-size bank; the remaining sets stay zero.
    for (std::size_t i = 0; i < sets.size(); ++i) {
        bank.sets[i] = sets[i];
    }

    return bank;
}

// The table is 256 entries of two raw float words (cos, then sin), 8 bytes each.
NormalTable NormalTable::from_elf(ByteView elf, u32 address) {
    const ByteView bytes = elf_read(elf, address, 256 * 8);
    NormalTable table;

    // Copy each (cos, sin) pair out of the little-endian image.
    for (std::size_t i = 0; i < 256; ++i) {
        table.entries[i] = {bytes.u32_at(i * 8), bytes.u32_at(i * 8 + 4)};
    }

    return table;
}

// A stand-in for the game's table: it is not bit-exact (see lighting.h), so it is only for tests
// and renderers that do not need bit-exactness.
NormalTable NormalTable::computed() {
    NormalTable table;

    // Entry i is the angle i * 2 pi / 256, computed in double and rounded once to float.
    for (std::size_t i = 0; i < 256; ++i) {
        const double a = static_cast<double>(i) * 2.0 * std::numbers::pi / 256.0;
        table.entries[i] =
            {ps2::bits(static_cast<f32>(std::cos(a))), ps2::bits(static_cast<f32>(std::sin(a)))};
    }

    // Pin the quarter turns to exact values; std::cos of pi / 2 would leave a rounding residue.
    table.entries[0] = {ps2::kOne, 0};
    table.entries[64] = {0, ps2::kOne};
    table.entries[128] = {ps2::kNegOne, 0};
    table.entries[192] = {0, ps2::kNegOne};
    return table;
}

// Walks the program headers (offset, entry size and count come from the ELF header at 0x1c, 0x2a
// and 0x2c) and returns the bytes from the first PT_LOAD segment that holds the whole range.
// Throws if no segment does.
ByteView elf_read(ByteView elf, u32 address, std::size_t length) {
    const ByteView magic = elf.sub(0, 4, "ELF magic");

    // Refuse anything that is not an ELF image before trusting its header fields.
    if (magic.data()[0] != 0x7f || magic.data()[1] != 'E' || magic.data()[2] != 'L'
        || magic.data()[3] != 'F') {
        fail("not an ELF file");
    }

    const std::size_t phoff = elf.u32_at(0x1c);
    const std::size_t phentsize = elf.u16_at(0x2a);
    const std::size_t phnum = elf.u16_at(0x2c);

    // Look for a loadable segment (type 1) that contains [address, address + length) in the file.
    for (std::size_t i = 0; i < phnum; ++i) {
        const std::size_t ph = phoff + i * phentsize;
        const u32 type = elf.u32_at(ph);
        const std::size_t offset = elf.u32_at(ph + 4);
        const u32 vaddr = elf.u32_at(ph + 8);
        const std::size_t filesz = elf.u32_at(ph + 16);

        // Take the segment only if it is loadable and holds the whole requested range.
        if (type == 1 && address >= vaddr && std::size_t{address - vaddr} + length <= filesz) {
            return elf.sub(offset + (address - vaddr), length, "ELF segment data");
        }
    }

    fail("ELF: {:#x}+{:#x} is not in any load segment", address, length);
}

// Adding the byte to the bit pattern of 65536.0 (0x47800000) gives the float 65536 + c / 128,
// because the unit in the last place at 65536.0 is 1/128.
ps2::V4 color_floats(const std::array<u8, 4>& c) {
    return {0x4780'0000u + c[0], 0x4780'0000u + c[1], 0x4780'0000u + c[2], 0x4780'0000u + c[3]};
}

// The float result's low byte per lane is the colour byte, so the final cast keeps only that byte.
std::array<u8, 4> light_instance_normal(
    const InstanceLightRegs& regs,
    const std::array<s16, 4>& normal,
    const ps2::V4& ambient,
    u32 clamp
) {
    // itof15 is exact: |n| < 2^24 and the scale is a power of two.
    const std::array<u32, 3> n =
        {ps2::bits(normal[0] / 32768.0f),
         ps2::bits(normal[1] / 32768.0f),
         ps2::bits(normal[2] / 32768.0f)};
    std::array<u32, 3> f{};

    // For each of the three lights, dot the normal with its to-light vector (rows hold the x, y and
    // z components lane by lane), then apply the back-face factor.
    for (std::size_t k = 0; k < 3; ++k) {
        const u32 d = ps2::add(
            ps2::add(ps2::mul(regs.rows[0][k], n[0]), ps2::mul(regs.rows[1][k], n[1])),
            ps2::mul(regs.rows[2][k], n[2])
        );
        f[k] = ps2::max(d, ps2::mul(d, regs.back[k]));
    }

    std::array<u8, 4> out{};

    // Sum ambient and the three lit colours for each output lane.
    for (std::size_t c = 0; c < 4; ++c) {
        u32 acc = ps2::mul(ambient[c], ps2::kOne);

        // Add each light's colour scaled by its dot product.
        for (std::size_t l = 0; l < 3; ++l) {
            acc = ps2::add(acc, ps2::mul(regs.colors[l][c], f[l]));
        }

        // Only the colour lanes are clamped; the alpha lane (3) is passed through.
        if (c < 3) {
            acc = ps2::min(acc, clamp);
        }

        out[c] = static_cast<u8>(acc);
    }

    return out;
}

// The list is a nibble-packed queue of slot numbers; setting the bits above it makes the 0xf
// terminator appear once the real entries are consumed.
MergedPointLight merge_point_lights(
    const ps2::V4& centre, const PointLightBank& bank, u16 list_word
) {
    MergedPointLight merged;
    ps2::V4& dp = merged.direction;
    ps2::V4& cp = merged.color;
    int in_range = 0;
    u32 list = u32{list_word} | 0xf'0000;

    // Visit each listed light until the 0xf terminator reaches the low nibble.
    while ((list & 0xf) != 0xf) {
        const PointLight& light = bank[(list & 0xf) % kPointLightSlots];
        list >>= 4;
        const ps2::V4 col = ps2::bits(light.color);
        const ps2::V4 pos = ps2::bits(light.position);
        // vdiv Q, vf0w, r; vsub.xyz v = centre - pos (w stays r).
        const u32 inv_r = ps2::div(ps2::kOne, pos[3]);
        const ps2::V4 v =
            {ps2::sub(centre[0], pos[0]),
             ps2::sub(centre[1], pos[1]),
             ps2::sub(centre[2], pos[2]),
             pos[3]};
        const u32 dist2 = ps2::dot3(v, v);

        // vsubx.w vf0, r * r - |v|^2: the sign flag skips an out-of-range light.
        // Skip a light whose radius does not reach the centre (r * r - |v|^2 is negative).
        if ((ps2::sub(ps2::mul(pos[3], pos[3]), dist2) & ps2::kSign) != 0) {
            continue;
        }

        const u32 dist = ps2::sqrt(dist2);
        // vmulq.w (1.0 * 1/r) * dist; vsubw.w 1 - that; vdiv Q = 1 / (1.0 * dist).
        const u32 a = ps2::sub(ps2::kOne, ps2::mul(ps2::mul(ps2::kOne, inv_r), dist));
        const u32 inv_d = ps2::div(ps2::kOne, ps2::mul(ps2::kOne, dist));

        // vmulaq.xyz ACC = v * Q; vmaddw.xyz dir = ACC + dir * 1.0.
        // Accumulate the unit vector towards the centre into the merged direction.
        for (std::size_t k = 0; k < 3; ++k) {
            dp[k] = ps2::add(ps2::mul(v[k], inv_d), ps2::mul(dp[k], ps2::kOne));
        }

        cp = ps2::add(cp, ps2::scale(col, a));
        ++in_range;
    }

    // v1 starts at -2: only two or more lights are renormalised.
    // One light's vector is already unit length; only a sum of several needs renormalising.
    if (in_range >= 2) {
        const u32 q = ps2::rsqrt(ps2::kOne, ps2::dot3(dp, dp));

        // Scale the summed direction by the reciprocal of its length.
        for (std::size_t k = 0; k < 3; ++k) {
            dp[k] = ps2::mul(dp[k], q);
        }
    }

    return merged;
}

// The 5-bit channels are widened to 8 bits by a shift, as PEXT5 does, so full intensity is 0xf8.
std::array<u8, 4> pext5(u16 c) {
    return {
        static_cast<u8>((c & 0x1f) << 3),
        static_cast<u8>(((c >> 5) & 0x1f) << 3),
        static_cast<u8>(((c >> 10) & 0x1f) << 3),
        static_cast<u8>((c >> 15) << 7),
    };
}

}  // namespace openrac::assets::rac1
