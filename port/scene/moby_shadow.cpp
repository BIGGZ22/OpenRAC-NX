// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
#include "scene/moby_shadow.h"

#include <algorithm>
#include <cmath>
#include <cstring>

#include "assets/geometry/moby_shadow.h"
#include "scene/moby_pose.h"

namespace openrac::scene {
MobyShadowState moby_shadow_state(
    std::span<const std::uint8_t> ram,
    std::uint32_t moby,
    std::uint32_t directions,
    const renderer::Mat4& view
) {
    MobyShadowState state;
    if (moby > ram.size() || ram.size() - moby < 0x100) {
        return state;
    }
    auto read = [&]<typename T>(std::uint32_t at) {
        T value{};
        if (at <= ram.size() && ram.size() - at >= sizeof(T)) {
            std::memcpy(&value, ram.data() + at, sizeof(T));
        }
        return value;
    };
    const auto mode = read.template operator()<std::uint16_t>(moby + 0x34);
    if (!(mode & 0x400) || (mode & 0x801)) {
        return state;
    }
    state.direction_index = ram[moby + 0xbd];
    // MobyProc copies four direction vectors into its scratch work area.
    if (state.direction_index >= 4 || directions > ram.size() || ram.size() - directions < 64) {
        return state;
    }
    const auto direction =
        read.template operator()<renderer::Vec3>(directions + state.direction_index * 16);
    const float length2 = renderer::dot(direction, direction);
    if (!std::isfinite(length2) || length2 < 1e-8f || !std::isfinite(direction[2])
        || direction[2] >= -1e-5f) {
        return state;
    }
    state.heights =
    { read.template operator()<float>(moby + 0x84),
      read.template operator()<float>(moby + 0x88) };
    if (!std::isfinite(state.heights[0]) || !std::isfinite(state.heights[1])
        || state.heights[1] <= 0 || state.heights[1] <= state.heights[0]) {
        return state;
    }
    auto centre = read.template operator()<renderer::Vec3>(moby);
    for (auto& x : centre) {
        x /= 1024.0f;
    }
    const float middle = (state.heights[0] + state.heights[1]) * 0.5f;
    const float t = (middle - centre[2]) / direction[2];
    for (int i = 0; i < 3; ++i) {
        centre[i] += direction[i] * t;
    }
    const float radius = read.template operator()<float>(moby + 0xc) / 1024.0f
                         + (state.heights[1] - state.heights[0]) * 0.5f;
    const float far_depth = (-renderer::transform_point(view, centre)[2] + radius) * 1024.0f;
    if (!std::isfinite(far_depth) || std::fabs(far_depth) > 1e8f || !std::isfinite(radius)
        || radius < 0) {
        return state;
    }
    // Retail truncates to integer, halves the distance remaining, caps at 0x1000,
    // then scales joint-local proxy coordinates (not the darkness).
    const int remaining =
        (static_cast<int>(ram[moby + 0x7f]) * 1024 - static_cast<int>(far_depth)) / 2;
    state.scale = static_cast<float>(std::clamp(remaining, 0, 4096)) / 4096.0f;
    state.extrusion = renderer::normalize(direction);
    for (auto& x : state.extrusion) {
        x *= 1000.0f;
    }
    state.enabled = state.scale > 0;
    return state;
}

std::vector<renderer::ShadowCapsule> moby_shadows(
    std::span<const std::uint8_t> ram,
    std::uint32_t moby,
    const renderer::Mat4& model,
    float proxy_scale
) {
    std::vector<renderer::ShadowCapsule> out;
    if (!std::isfinite(proxy_scale) || proxy_scale <= 0 || proxy_scale > 1 || moby > ram.size()
        || ram.size() - moby < 0x28) {
        return out;
    }
    const auto word = [&](std::uint32_t at) {
        std::uint32_t value = 0;
        if (at <= ram.size() && ram.size() - at >= 4) {
            std::memcpy(&value, ram.data() + at, 4);
        }
        return value & 0x01ffffffu;
    };
    const auto cls = word(moby + 0x24);
    if (!cls || cls > ram.size() || ram.size() - cls < 0x48) {
        return out;
    }
    const auto size = static_cast<std::uint32_t>(ram[cls + 0xf]) * 16u;
    const auto skeleton = word(cls + 0x14);
    if (!size || skeleton < size || skeleton > ram.size()) {
        return out;
    }
    const auto pose = moby_pose_matrices(ram, moby);
    if (pose.empty()) {
        return out;
    }
    try {
        const auto block =
            assets::rac1::parse_shadow_block(assets::ByteView(ram).sub(skeleton - size, size));
        const auto endpoint = [&](int joint,
                                  const std::array<float, 4>& p,
                                  renderer::Vec3& centre,
                                  float& radius) {
            if (joint < 0 || static_cast<std::size_t>(joint) >= pose.size()) {
                return false;
            }
            // Proxies are joint-local, so use P_j rather than the mesh palette
            // P_j * inverse_bind. Packed translations and proxy points use 1/1024 units.
            renderer::Mat4 local = renderer::identity();
            for (int col = 0; col < 4; ++col) {
                for (int row = 0; row < 3; ++row) {
                    local[4 * col + row] = pose[static_cast<std::size_t>(joint)][col][row]
                                           / (col == 3 ? 1024.0f : 1.0f);
                }
            }
            for (int col = 0; col < 4; ++col) {
                for (int row = 0; row < 3; ++row) {
                    local[4 * col + row] *= proxy_scale;
                }
            }
            const auto world = renderer::multiply(model, local);
            centre =
                renderer::transform_point(world, {p[0] / 1024.0f, p[1] / 1024.0f, p[2] / 1024.0f});
            float scale = 0;
            for (int col = 0; col < 3; ++col) {
                const renderer::Vec3 axis{world[4 * col], world[4 * col + 1], world[4 * col + 2]};
                scale = std::max(scale, std::sqrt(renderer::dot(axis, axis)));
            }
            radius = p[3] * scale / 1024.0f;
            return true;
        };
        for (const auto& primitive : block.primitives) {
            renderer::ShadowCapsule c;
            if (const auto* s = std::get_if<assets::rac1::ShadowSphere>(&primitive)) {
                if (!endpoint(s->joint, s->centre, c.a, c.radius_a)) {
                    continue;
                }
                c.b = c.a;
                c.radius_b = c.radius_a;
                c.segments_a = c.segments_b = s->segments;
            } else {
                const auto& capsule = std::get<assets::rac1::ShadowCapsule>(primitive);
                if (!endpoint(capsule.joints[0], capsule.a, c.a, c.radius_a)
                    || !endpoint(capsule.joints[1], capsule.b, c.b, c.radius_b)) {
                    continue;
                }
                c.segments_a = capsule.segments[0];
                c.segments_b = capsule.segments[1];
            }
            out.push_back(c);
        }
    } catch (const assets::AssetError&) {
        // A loading frame or a class without a complete proxy block.
    }
    return out;
}
}  // namespace openrac::scene
