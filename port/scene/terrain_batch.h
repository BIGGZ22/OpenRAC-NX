// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The index work of LevelScene's terrain batches (OPENRAC_TERRAIN_BATCH, level_renderer.h): the
// pieces (a primitive's index range and material) of several one-instance terrain groups,
// concatenated into one index list grouped by material, so that each material is one draw.
// Within a material the pieces keep the order they were given (groups in draw order, then each
// group's own primitive order). Pure: tests/terrain_batch_test.cpp.

#pragma once

#include <algorithm>
#include <cstdint>
#include <span>
#include <vector>

namespace openrac::scene::terrain_batch {

struct Piece {
    int material = 0;
    std::uint32_t first = 0;  // into the source index list
    std::uint32_t count = 0;
};

struct Run {
    int material = 0;
    std::uint32_t first = 0;  // into the built index list
    std::uint32_t count = 0;
};

// Fills `indices` and `runs` (one run a material, in material order). False, with both left
// empty, when a piece reaches past `source`: a batch must never draw less than its members.
inline bool build(
    std::vector<Piece> pieces, std::span<const std::uint32_t> source,
    std::vector<std::uint32_t>& indices, std::vector<Run>& runs
) {
    indices.clear();
    runs.clear();
    for (const Piece& p : pieces) {
        if (static_cast<std::uint64_t>(p.first) + p.count > source.size()) {
            return false;
        }
    }
    std::stable_sort(pieces.begin(), pieces.end(), [](const Piece& a, const Piece& b) {
        return a.material < b.material;
    });
    for (const Piece& p : pieces) {
        if (p.count == 0) {
            continue;
        }
        if (runs.empty() || runs.back().material != p.material) {
            runs.push_back({p.material, static_cast<std::uint32_t>(indices.size()), 0});
        }
        indices.insert(indices.end(), source.begin() + p.first, source.begin() + p.first + p.count);
        runs.back().count += p.count;
    }
    return true;
}

}  // namespace openrac::scene::terrain_batch
