// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "scene/level.h"
namespace openrac::scene {
// Private, versioned renderer cache, generated from the player's disc on device.
inline constexpr unsigned kDiscCacheVersion = 2;
bool read_disc_cache(const std::filesystem::path& file, LevelData& level, std::string& error);
void write_disc_cache(const std::filesystem::path& file, const LevelData& level);
}
