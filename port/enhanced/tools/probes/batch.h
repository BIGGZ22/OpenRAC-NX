// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
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
    // Keep each grid already written and verified in the output folder (a bake that was stopped
    // and started again): it is read back and checked against this plan, and baked again only
    // when it does not match.
    bool resume = false;
};

// Bake and read back all planned grids, rejecting changed inputs before successful completion.
// Throws on malformed plans, tracing/verification errors, or existing output files (unless
// resuming). Prints "Probe batch probes N" first, then "Probes DONE/N" as the probes are baked
// (each half per cent at most), for the planner's progress.
void run_probe_batch(
    const scene::LevelData& level,
    const BakeSceneExport& exported,
    const ProbeGrid& sparse_grid,
    const ProbeBatchOptions& options,
    std::ostream& output
);

}  // namespace openrac::lighting
