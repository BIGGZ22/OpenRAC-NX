// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Coverage reports and output files for offline probe tools.
#pragma once
#include <filesystem>
#include <ostream>
#include <span>
#include <string>

#include "enhanced/tools/probes/level_scene.h"

namespace openrac::lighting {
// Shared human-readable transport/grid description for both CLI paths.
std::string describe_probe_bake(
    const scene::LevelData& level,
    const BakeSceneExport& exported,
    const ProbeGrid& grid,
    const BakeSettings& settings,
    unsigned light_set
);

// Preserve the original geometric coverage diagnostics under runtime rounding.
void report_terrain_coverage(
    const scene::LevelData& level,
    const ProbeVolume& volume,
    const ProbeVolume* coarse,
    std::ostream& output
);

// Write through a temporary file, refusing to overwrite either file.
void write_bake_file(const std::filesystem::path& path, std::span<const std::uint8_t> bytes);
}  // namespace openrac::lighting
