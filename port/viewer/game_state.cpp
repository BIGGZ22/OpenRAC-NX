// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

#include "viewer/game_state.h"

#include <cmath>
#include <cstring>
#include <format>
#include <fstream>
#include <string_view>

namespace openrac::viewer {

/*
 * PAL addresses (the US ones, documented in docs/engine and in ReRAC's game_camera_fog.md, are
 * 0x100 lower here):
 * - 0x187180: the camera's position (the camera block at 0x187040 + 0x140), game units.
 * - 0x187390: the camera's rotation rows forward, left, up, which the per-frame camera matrices
 *   function (func_001F2608) turns into the view matrix at 0x187040.
 * - 0x18CE00: UpdateViewContext's block: +0xB0 the tangent of half the horizontal field of view,
 *   +0xB4 the vertical one (the horizontal one times the TV correction).
 * - 0x16001C / 0x160020: the first moby and the end of the table, as CreateMoby (func_0020D348)
 *   walks it; a moby is 0x100 bytes and a state of 0xFE or more marks a free slot.
 */
const GameAddresses kRac1Pal{0x00187180, 0x00187390, 0x0018CE00, 0x0016001C, 0x00160020, 0.756f, 0x00160FF8, 0x00161010};

/*
 * The same globals in each level's program, found by comparing the boot program's camera matrices
 * function (func_001F2608) and CreateMoby (func_0020D348) with their copies in the level's code,
 * instruction by instruction: the copies differ only in the addresses.
 */
const GameAddresses kRac1PalLevels[19] = {
    {0x00166EC0, 0x001670D0, 0x0016CB40, 0x0016009C, 0x001600A0, 0.756f, 0x00161278, 0x00161290},  // 0
    {0x001672C0, 0x001674D0, 0x0016CF40, 0x0016005C, 0x00160060, 0.756f, 0x00161238, 0x00161250},  // 1
    {0x00167440, 0x00167650, 0x0016D0C0, 0x0016005C, 0x00160060, 0.756f, 0x00161238, 0x00161250},  // 2
    {0x00166F40, 0x00167150, 0x0016CBC0, 0x0016005C, 0x00160060, 0.756f, 0x00161238, 0x00161250},  // 3
    {0x00166FC0, 0x001671D0, 0x0016CC40, 0x0016005C, 0x00160060, 0.756f, 0x00161238, 0x00161250},  // 4
    {0x001672C0, 0x001674D0, 0x0016CF40, 0x0016009C, 0x001600A0, 0.756f, 0x00161278, 0x00161290},  // 5
    {0x00167640, 0x00167850, 0x0016D2C0, 0x0016005C, 0x00160060, 0.756f, 0x00161238, 0x00161250},  // 6
    {0x00166EC0, 0x001670D0, 0x0016CB40, 0x0016005C, 0x00160060, 0.756f, 0x00161238, 0x00161250},  // 7
    {0x00167640, 0x00167850, 0x0016D2C0, 0x0016005C, 0x00160060, 0.756f, 0x00161238, 0x00161250},  // 8
    {0x00166FC0, 0x001671D0, 0x0016CC40, 0x0016005C, 0x00160060, 0.756f, 0x00161238, 0x00161250},  // 9
    {0x001672C0, 0x001674D0, 0x0016CF40, 0x0016005C, 0x00160060, 0.756f, 0x00161238, 0x00161250},  // 10
    {0x00167840, 0x00167A50, 0x0016D4C0, 0x0016005C, 0x00160060, 0.756f, 0x00161238, 0x00161250},  // 11
    {0x001672C0, 0x001674D0, 0x0016CF40, 0x0016005C, 0x00160060, 0.756f, 0x00161238, 0x00161250},  // 12
    {0x00167140, 0x00167350, 0x0016CDC0, 0x0016005C, 0x00160060, 0.756f, 0x00161238, 0x00161250},  // 13
    {0x001675C0, 0x001677D0, 0x0016D240, 0x0016009C, 0x001600A0, 0.756f, 0x00161278, 0x00161290},  // 14
    {0x00167440, 0x00167650, 0x0016D0C0, 0x0016005C, 0x00160060, 0.756f, 0x00161238, 0x00161250},  // 15
    {0x00167240, 0x00167450, 0x0016CEC0, 0x0016009C, 0x001600A0, 0.756f, 0x00161278, 0x00161290},  // 16
    {0x00167740, 0x00167950, 0x0016D3C0, 0x0016005C, 0x00160060, 0.756f, 0x00161238, 0x00161250},  // 17
    {0x00167840, 0x00167A50, 0x0016D4C0, 0x0016005C, 0x00160060, 0.756f, 0x00161238, 0x00161250},  // 18
};

namespace {

template <typename T>
T at(std::span<const std::uint8_t> ram, std::uint32_t address) {
    T value{};
    address &= 0x01FFFFFF;
    if (address + sizeof(T) <= ram.size()) {
        std::memcpy(&value, ram.data() + address, sizeof(T));
    }
    return value;
}

renderer::Vec3 vec3(std::span<const std::uint8_t> ram, std::uint32_t address) {
    return {at<float>(ram, address), at<float>(ram, address + 4), at<float>(ram, address + 8)};
}

// Column-major rotations about X, Y and Z.
renderer::Mat4 rotation_x(float a) {
    const float c = std::cos(a), s = std::sin(a);
    return {1, 0, 0, 0, 0, c, s, 0, 0, -s, c, 0, 0, 0, 0, 1};
}

renderer::Mat4 rotation_y(float a) {
    const float c = std::cos(a), s = std::sin(a);
    return {c, 0, -s, 0, 0, 1, 0, 0, s, 0, c, 0, 0, 0, 0, 1};
}

renderer::Mat4 rotation_z(float a) {
    const float c = std::cos(a), s = std::sin(a);
    return {c, s, 0, 0, -s, c, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
}

}  // namespace

renderer::Mat4 GameState::view() const {
    return renderer::look_along(camera_position, forward, up);
}

renderer::Mat4 GameState::projection(float aspect, float near_z, float far_z) const {
    // The game's vertical field of view; the horizontal one follows the target's shape.
    return renderer::perspective_reversed(2.0f * std::atan(tan_half_fov_y), aspect, near_z, far_z);
}

GameState read_game_state(std::span<const std::uint8_t> ram, const GameAddresses& a) {
    GameState s;
    s.camera_position = vec3(ram, a.camera_position);
    s.forward = vec3(ram, a.camera_rows);
    s.left = vec3(ram, a.camera_rows + 0x10);
    s.up = vec3(ram, a.camera_rows + 0x20);

    const float tx = at<float>(ram, a.view_context + 0xB0);
    const float ty = at<float>(ram, a.view_context + 0xB4);
    if (tx > 0.05f && tx < 10.0f) {
        s.tan_half_fov_x = tx;
        s.tan_half_fov_y = ty > 0.05f && ty < 10.0f ? ty : tx * a.vertical_factor;
    }

    // UpdateFog (func_001F2930): the colour as three words at +0x230, then near and far depth
    // at +0x218 and +0x21C, near and far F at +0x228 and +0x22C.
    for (int c = 0; c < 3; ++c) {
        s.fog_colour[c] = static_cast<float>(at<std::uint32_t>(ram, a.view_context + 0x230 + 4 * c) & 0xFF) / 255.0f;
    }
    s.fog_near = at<float>(ram, a.view_context + 0x218);
    s.fog_far = at<float>(ram, a.view_context + 0x21C);
    s.fog_near_f = at<float>(ram, a.view_context + 0x228);
    s.fog_far_f = at<float>(ram, a.view_context + 0x22C);

    // The table starts at the word before moby_first (the mobys the level places, the hero first);
    // moby_first itself is where the spawned ones begin.
    auto first = at<std::uint32_t>(ram, a.moby_first) & 0x01FFFFFF;
    const auto end = at<std::uint32_t>(ram, a.moby_end) & 0x01FFFFFF;
    const auto start = at<std::uint32_t>(ram, a.moby_first - 4) & 0x01FFFFFF;
    if (start != 0 && start < first && (first - start) % 0x100 == 0) {
        first = start;
    }
    if (first == 0 || end <= first || end > ram.size() || (end - first) % 0x100 != 0) {
        return s;
    }
    for (std::uint32_t m = first; m < end; m += 0x100) {
        // A state with its sign bit set is no moby any more: 0xFD a deleted moby of the level's own
        // (its slot is never used again, and it keeps its last position: a bolt picked up), 0xFE a
        // deleted spawned one, 0xFF the end of the used part. The game's own loops skip them by the
        // sign (the update's activity gate, MobyBuildMatrix, the groups' counts).
        if (at<std::uint8_t>(ram, m + 0x20) >= 0x80) {
            continue;
        }
        const renderer::Vec3 pos = vec3(ram, m + 0x10);
        const renderer::Vec3 rot = vec3(ram, m + 0x40);
        const float scale = at<float>(ram, m + 0x2C);
        if (!std::isfinite(pos[0]) || !std::isfinite(scale) || scale <= 0.0f || scale > 1000.0f) {
            continue;
        }
        LiveMoby live;
        live.address = m;
        live.class_id = at<std::int16_t>(ram, m + 0xA6);
        // The rotation rows at +0xC0 (each the image of a model axis), as MobyProc draws it:
        // MobyBuildMatrix (func_L00_00251E30) makes them from the Euler angles at +0x40 every
        // update, unless the moby keeps rows of its own (mode 0x100, or an item placed after its
        // update: the wrench in Ratchet's hand), and negates row 1 with mode 0x8000. The class
        // mesh is in unscaled units and the moby's scale already includes the class's.
        const renderer::Vec3 r0 = vec3(ram, m + 0xC0), r1 = vec3(ram, m + 0xD0), r2 = vec3(ram, m + 0xE0);
        const auto usable = [](const renderer::Vec3& r) {
            return std::isfinite(r[0]) && std::isfinite(r[1]) && std::isfinite(r[2])
                && r[0] * r[0] + r[1] * r[1] + r[2] * r[2] > 1.0e-6f;
        };
        if (usable(r0) && usable(r1) && usable(r2)) {
            live.matrix = {r0[0] * scale, r0[1] * scale, r0[2] * scale, 0,
                           r1[0] * scale, r1[1] * scale, r1[2] * scale, 0,
                           r2[0] * scale, r2[1] * scale, r2[2] * scale, 0,
                           pos[0], pos[1], pos[2], 1};
            s.mobys.push_back(live);
            continue;
        }
        // No rows yet (a moby not built since it was made): X first, then Y, then Z, as the
        // game's Euler matrices.
        live.matrix = renderer::multiply(
            renderer::translation(pos),
            renderer::multiply(
                rotation_z(rot[2]),
                renderer::multiply(
                    rotation_y(rot[1]),
                    renderer::multiply(rotation_x(rot[0]), renderer::scale(scale))
                )
            )
        );
        s.mobys.push_back(live);
    }
    return s;
}

std::uint32_t shown_chain(std::span<const std::uint8_t> ram, const GameAddresses& a) {
    const auto building = at<std::uint32_t>(ram, a.chain_index) & 1;
    return at<std::uint32_t>(ram, a.chain_bases + 4 * (1 - building));
}

/*
 * NTSC-U (SCUS-97199): the same globals, each found from PAL's by the version map
 * (port/tools/versionmap): the address the same instruction of the same function forms in that
 * version's program. The vertical factor is PAL's, not yet measured on NTSC-U.
 */
const GameAddresses kRac1Ntsc{0x00187080, 0x00187290, 0x0018CD00, 0x0015FF1C, 0x0015FF20, 0.756f, 0x00160EF8, 0x00160F10};
const GameAddresses kRac1NtscLevels[19] = {
    {0x00166DC0, 0x00166FD0, 0x0016CA40, 0x0015FFDC, 0x0015FFE0, 0.756f, 0x001611B8, 0x001611D0},  // 0
    {0x00167240, 0x00167450, 0x0016CEC0, 0x0015FFDC, 0x0015FFE0, 0.756f, 0x001611B8, 0x001611D0},  // 1
    {0x001673C0, 0x001675D0, 0x0016D040, 0x0015FFDC, 0x0015FFE0, 0.756f, 0x001611B8, 0x001611D0},  // 2
    {0x00166EC0, 0x001670D0, 0x0016CB40, 0x0015FFDC, 0x0015FFE0, 0.756f, 0x001611B8, 0x001611D0},  // 3
    {0x00166F40, 0x00167150, 0x0016CBC0, 0x0015FFDC, 0x0015FFE0, 0.756f, 0x001611B8, 0x001611D0},  // 4
    {0x001671C0, 0x001673D0, 0x0016CE40, 0x0015FFDC, 0x0015FFE0, 0.756f, 0x001611B8, 0x001611D0},  // 5
    {0x00167540, 0x00167750, 0x0016D1C0, 0x0015FFDC, 0x0015FFE0, 0.756f, 0x001611B8, 0x001611D0},  // 6
    {0x00166E40, 0x00167050, 0x0016CAC0, 0x0015FFDC, 0x0015FFE0, 0.756f, 0x001611B8, 0x001611D0},  // 7
    {0x001675C0, 0x001677D0, 0x0016D240, 0x0015FFDC, 0x0015FFE0, 0.756f, 0x001611B8, 0x001611D0},  // 8
    {0x00166F40, 0x00167150, 0x0016CBC0, 0x0015FFDC, 0x0015FFE0, 0.756f, 0x001611B8, 0x001611D0},  // 9
    {0x00167240, 0x00167450, 0x0016CEC0, 0x0015FFDC, 0x0015FFE0, 0.756f, 0x001611B8, 0x001611D0},  // 10
    {0x001677C0, 0x001679D0, 0x0016D440, 0x0015FFDC, 0x0015FFE0, 0.756f, 0x001611B8, 0x001611D0},  // 11
    {0x00167240, 0x00167450, 0x0016CEC0, 0x0015FFDC, 0x0015FFE0, 0.756f, 0x001611B8, 0x001611D0},  // 12
    {0x001670C0, 0x001672D0, 0x0016CD40, 0x0015FFDC, 0x0015FFE0, 0.756f, 0x001611B8, 0x001611D0},  // 13
    {0x001674C0, 0x001676D0, 0x0016D140, 0x0015FFDC, 0x0015FFE0, 0.756f, 0x001611B8, 0x001611D0},  // 14
    {0x001673C0, 0x001675D0, 0x0016D040, 0x0015FFDC, 0x0015FFE0, 0.756f, 0x001611B8, 0x001611D0},  // 15
    {0x001671C0, 0x001673D0, 0x0016CE40, 0x0015FFDC, 0x0015FFE0, 0.756f, 0x001611B8, 0x001611D0},  // 16
    {0x001676C0, 0x001678D0, 0x0016D340, 0x0015FFDC, 0x0015FFE0, 0.756f, 0x001611B8, 0x001611D0},  // 17
    {0x001677C0, 0x001679D0, 0x0016D440, 0x0015FFDC, 0x0015FFE0, 0.756f, 0x001611B8, 0x001611D0},  // 18
};

const GameAddresses& rac1_addresses(std::string_view game, int level) {
    const bool ntsc = game == "rac1-ntsc";
    if (level >= 0 && level < 19) {
        return ntsc ? kRac1NtscLevels[level] : kRac1PalLevels[level];
    }
    return ntsc ? kRac1Ntsc : kRac1Pal;
}

const GameAddresses& rac1_pal_addresses(std::span<const std::uint8_t> ram) {
    const auto level = at<std::int32_t>(ram, 0x0015EE84);
    return level >= 0 && level < 19 ? kRac1PalLevels[level] : kRac1Pal;
}

bool load_memory(const std::filesystem::path& path, std::vector<std::uint8_t>& out, std::string& error) {
    std::ifstream f(path, std::ios::binary);
    out.assign(std::istreambuf_iterator<char>(f), {});
    if (out.size() != 32u * 1024 * 1024) {
        error = std::format("{}: not a 32 MB memory picture ({} bytes)", path.string(), out.size());
        return false;
    }
    return true;
}

}  // namespace openrac::viewer
