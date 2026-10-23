// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-formats/src/tfrag_light.rs:
// ISC License, Copyright (c) 2026 ReRAC contributors.
//
// The shared lighting inputs of RAC1.

#include "assets/geometry/lighting.h"

#include <cmath>
#include <numbers>

namespace openrac::assets::rac1 {

LightBank parse_light_bank(ByteView gameplay) {
    const std::size_t section = gameplay.u32_at(0x04);
    const s32 n = gameplay.s32_at(section);

    if (n < 0) {
        fail("gameplay: negative directional light count {}", n);
    }

    LightBank bank;
    bank.count = std::min(static_cast<std::size_t>(n), kMaxLevelLights);
    const auto sets =
        gameplay.read_array<DirLightSet>(section + 0x10, bank.count, "directional lights");

    for (std::size_t i = 0; i < sets.size(); ++i) {
        bank.sets[i] = sets[i];
    }

    return bank;
}

NormalTable NormalTable::from_elf(ByteView elf, u32 address) {
    const ByteView bytes = elf_read(elf, address, 256 * 8);
    NormalTable table;

    for (std::size_t i = 0; i < 256; ++i) {
        table.entries[i] = {bytes.u32_at(i * 8), bytes.u32_at(i * 8 + 4)};
    }

    return table;
}

NormalTable NormalTable::computed() {
    NormalTable table;

    for (std::size_t i = 0; i < 256; ++i) {
        const double a = static_cast<double>(i) * 2.0 * std::numbers::pi / 256.0;
        table.entries[i] =
            {ps2::bits(static_cast<f32>(std::cos(a))), ps2::bits(static_cast<f32>(std::sin(a)))};
    }

    table.entries[0] = {ps2::kOne, 0};
    table.entries[64] = {0, ps2::kOne};
    table.entries[128] = {ps2::kNegOne, 0};
    table.entries[192] = {0, ps2::kNegOne};
    return table;
}

ByteView elf_read(ByteView elf, u32 address, std::size_t length) {
    const ByteView magic = elf.sub(0, 4, "ELF magic");

    if (magic.data()[0] != 0x7f || magic.data()[1] != 'E' || magic.data()[2] != 'L'
        || magic.data()[3] != 'F') {
        fail("not an ELF file");
    }

    const std::size_t phoff = elf.u32_at(0x1c);
    const std::size_t phentsize = elf.u16_at(0x2a);
    const std::size_t phnum = elf.u16_at(0x2c);

    for (std::size_t i = 0; i < phnum; ++i) {
        const std::size_t ph = phoff + i * phentsize;
        const u32 type = elf.u32_at(ph);
        const std::size_t offset = elf.u32_at(ph + 4);
        const u32 vaddr = elf.u32_at(ph + 8);
        const std::size_t filesz = elf.u32_at(ph + 16);

        if (type == 1 && address >= vaddr && std::size_t{address - vaddr} + length <= filesz) {
            return elf.sub(offset + (address - vaddr), length, "ELF segment data");
        }
    }

    fail("ELF: {:#x}+{:#x} is not in any load segment", address, length);
}

ps2::V4 color_floats(const std::array<u8, 4>& c) {
    return {0x4780'0000u + c[0], 0x4780'0000u + c[1], 0x4780'0000u + c[2], 0x4780'0000u + c[3]};
}

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

    for (std::size_t k = 0; k < 3; ++k) {
        const u32 d = ps2::add(
            ps2::add(ps2::mul(regs.rows[0][k], n[0]), ps2::mul(regs.rows[1][k], n[1])),
            ps2::mul(regs.rows[2][k], n[2])
        );
        f[k] = ps2::max(d, ps2::mul(d, regs.back[k]));
    }

    std::array<u8, 4> out{};

    for (std::size_t c = 0; c < 4; ++c) {
        u32 acc = ps2::mul(ambient[c], ps2::kOne);

        for (std::size_t l = 0; l < 3; ++l) {
            acc = ps2::add(acc, ps2::mul(regs.colors[l][c], f[l]));
        }

        if (c < 3) {
            acc = ps2::min(acc, clamp);
        }

        out[c] = static_cast<u8>(acc);
    }

    return out;
}

MergedPointLight merge_point_lights(
    const ps2::V4& centre, const PointLightBank& bank, u16 list_word
) {
    MergedPointLight merged;
    ps2::V4& dp = merged.direction;
    ps2::V4& cp = merged.color;
    int in_range = 0;
    u32 list = u32{list_word} | 0xf'0000;

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
        if ((ps2::sub(ps2::mul(pos[3], pos[3]), dist2) & ps2::kSign) != 0) {
            continue;
        }

        const u32 dist = ps2::sqrt(dist2);
        // vmulq.w (1.0 * 1/r) * dist; vsubw.w 1 - that; vdiv Q = 1 / (1.0 * dist).
        const u32 a = ps2::sub(ps2::kOne, ps2::mul(ps2::mul(ps2::kOne, inv_r), dist));
        const u32 inv_d = ps2::div(ps2::kOne, ps2::mul(ps2::kOne, dist));

        // vmulaq.xyz ACC = v * Q; vmaddw.xyz dir = ACC + dir * 1.0.
        for (std::size_t k = 0; k < 3; ++k) {
            dp[k] = ps2::add(ps2::mul(v[k], inv_d), ps2::mul(dp[k], ps2::kOne));
        }

        cp = ps2::add(cp, ps2::scale(col, a));
        ++in_range;
    }

    // v1 starts at -2: only two or more lights are renormalised.
    if (in_range >= 2) {
        const u32 q = ps2::rsqrt(ps2::kOne, ps2::dot3(dp, dp));

        for (std::size_t k = 0; k < 3; ++k) {
            dp[k] = ps2::mul(dp[k], q);
        }
    }

    return merged;
}

std::array<u8, 4> pext5(u16 c) {
    return {
        static_cast<u8>((c & 0x1f) << 3),
        static_cast<u8>(((c >> 5) & 0x1f) << 3),
        static_cast<u8>(((c >> 10) & 0x1f) << 3),
        static_cast<u8>((c >> 15) << 7),
    };
}

}  // namespace openrac::assets::rac1
