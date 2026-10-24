// SPDX-License-Identifier: GPL-3.0-or-later
#include "enhanced/viewer/hero_shadow.h"

#include <cmath>
#include <cstring>
#include <stdexcept>

namespace openrac::scene {
std::optional<bool> hero_airborne(std::span<const std::uint8_t> ram, std::uint32_t hero) {
    // Collision/ground-probe update clears air_frames on landing and
    // increments it while off the ground, including a zero-speed jump apex.
    // 0x30A instead flags particular mobys hit by the ground probe.
    constexpr std::size_t offset = 0x30e;
    if (!hero || hero > ram.size() || ram.size() - hero < offset + 2) {
        return {};
    }
    std::uint16_t airborne = 0;
    std::memcpy(&airborne, ram.data() + hero + offset, sizeof(airborne));
    return airborne != 0;
}

HeroShadowProjection hero_shadow_projection(
    renderer::Vec3 centre, renderer::Vec3 main_direction, bool airborne
) {
    using namespace renderer;
    for (float n : centre) {
        if (!std::isfinite(n)) {
            throw std::runtime_error("invalid hero shadow centre");
        }
    }
    HeroShadowProjection result;
    bool valid = true;
    for (float n : main_direction) {
        valid = valid && std::isfinite(n);
    }
    if (!airborne && valid && dot(main_direction, main_direction) > 1e-8f
        && main_direction[2] < -1e-4f) {
        result.direction = normalize(main_direction);
    }
    constexpr float half_extent = 4, upstream = 4, reach = 72;
    Vec3 eye = centre;
    for (unsigned axis = 0; axis < 3; ++axis) {
        eye[axis] -= result.direction[axis] * upstream;
    }
    const Vec3 up = std::abs(result.direction[2]) > 0.95f ? Vec3{0, 1, 0} : Vec3{0, 0, 1};
    const auto view = look_along(eye, result.direction, up);
    // OpenGL reverse Z: near z=0 -> +1, far z=-reach -> -1.
    Mat4 projection = identity();
    projection[0] = projection[5] = 1 / half_extent;
    projection[10] = 2 / reach;
    projection[14] = 1;
    result.matrix = multiply(projection, view);
    return result;
}
}  // namespace openrac::scene
