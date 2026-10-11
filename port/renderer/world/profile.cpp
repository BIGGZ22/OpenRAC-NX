// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

#include "renderer/world/profile.h"

namespace openrac::renderer::world {

std::optional<WorldProfile> world_profile(assets::Game game, assets::Region region) {
    if (game != assets::Game::Rac1) {
        return std::nullopt;  // not known yet
    }
    switch (region) {
        case assets::Region::Pal:
            return WorldProfile{kRac1Constants, 512.0f, 448.0f, 0.756f};
        case assets::Region::NtscU:
            return WorldProfile{kRac1Constants, 512.0f, 416.0f, 0.775f};
        case assets::Region::NtscJ:
            return std::nullopt;
    }
    return std::nullopt;
}

std::optional<WorldProfile> world_profile(const assets::GameVersion& version) {
    return world_profile(version.game, version.region);
}

}  // namespace openrac::renderer::world
