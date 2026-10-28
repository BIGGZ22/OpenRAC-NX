// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "scene/level.h"
#include "assets/disc/disc.h"
#include "assets/geometry/lighting.h"
#include <functional>
namespace openrac::scene {
using SetupProgress = std::function<bool(const std::string&, unsigned, unsigned)>;
LevelData import_disc_level(const assets::disc::Disc& disc, int id,
    const assets::rac1::NormalTable& normals, const SetupProgress& progress = {});
// The ISO is kept at data/iso_data/rac1/disc.iso, never duplicated. Completed
// level caches are reused after cancellation; saves and settings are untouched.
void prepare_switch_disc(const std::filesystem::path& image,
    const std::filesystem::path& data, const SetupProgress& progress = {});
}
