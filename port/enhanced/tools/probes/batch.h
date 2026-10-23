// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
// The offline tool's prepared-level batch and shared diagnostics.
#pragma once
#include <filesystem>
#include <ostream>
#include <span>
#include <string>

#include "enhanced/tools/probes/level_scene.h"

namespace openrac::lighting {

// One request from the planner; transport options remain fixed for the whole level.
struct ProbeBatchOptions {
    std::filesystem::path output_directory;
    BakeSettings settings;
    unsigned light_set = 0, threads = 0;
    std::uint64_t source_snapshot = 0;
    std::string plan;
};

// Bake and read back all planned grids, rejecting changed inputs before successful completion.
// Throws on malformed plans, tracing/verification errors, or existing output files.
void run_probe_batch(
    const viewer::LevelData& level,
    const BakeSceneExport& exported,
    const ProbeGrid& sparse_grid,
    const ProbeBatchOptions& options,
    std::ostream& output
);

// Shared human-readable transport/grid description for both CLI paths.
std::string describe_probe_bake(
    const viewer::LevelData& level,
    const BakeSceneExport& exported,
    const ProbeGrid& grid,
    const BakeSettings& settings,
    unsigned light_set
);

// Preserve the original geometric coverage diagnostics under runtime rounding.
void report_terrain_coverage(
    const viewer::LevelData& level,
    const ProbeVolume& volume,
    const ProbeVolume* coarse,
    std::ostream& output
);

// Write through a temporary file, refusing to overwrite either file.
void write_bake_file(const std::filesystem::path& path, std::span<const std::uint8_t> bytes);
}  // namespace openrac::lighting
