// SPDX-License-Identifier: GPL-3.0-or-later
#include "check.h"
#include "game/common/particle_rotation.h"
#include "game/common/particle_texture_cache.h"
int main() {
    for (unsigned i = 0; i < 256; ++i) {
        const float theta = float(i) * 6.2831853f / 256.0f;
        const auto& rotation = openrac::game::particle_rotation(std::uint8_t(i));
        CHECK(rotation[0] == std::cos(theta));
        CHECK(rotation[1] == std::sin(theta));
    }
    unsigned resolves = 0;
    auto resolve = [&](std::uint32_t, std::uint32_t) { return ++resolves; };
    openrac::game::ParticleTextureCache pass;
    for (unsigned i = 0; i < 10000; ++i) CHECK(pass.resolve(0x1000, 0x2000, resolve) == 1);
    CHECK(resolves == 1);
    CHECK(pass.resolve(0x1000, 0x3000, resolve) == 2);
    // A new pass must resolve again: the same RAM addresses can contain new pixels.
    openrac::game::ParticleTextureCache next_pass;
    CHECK(next_pass.resolve(0x1000, 0x2000, resolve) == 3);
    return openrac::test::result();
}
