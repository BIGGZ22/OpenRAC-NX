// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The pipeline cache's file on disk: the driver's cache data (vkGetPipelineCacheData) in an
// envelope that says how long it is and what it hashes to, so a file cut short or mixed from two
// writers is found and dropped rather than handed to the driver. MoltenVK has accepted such a file
// and drawn wrong colours with it (measured: `vulkan_uniform` under `ctest -j8`).
//
// Several programs save the same file at once (parallel tests, a game and a tool): each writes a
// temporary file of its own and renames it into place, so a reader sees one whole save or another.
// This file knows nothing of Vulkan; whether the data suits the device is the caller's check.

#pragma once

#include <cstddef>
#include <filesystem>
#include <span>
#include <string>
#include <vector>

namespace openrac::vkgl {

// What reading a saved pipeline cache found.
enum class SavedCacheState {
    Missing,  // No file, or one that cannot be read.
    Damaged,  // A file this code did not write whole: cut short, mixed, or of another format.
    Intact,   // A whole save; its data is the driver's.
};

// A saved pipeline cache as read back: its state, and the driver's data when it is intact.
struct SavedCache {
    SavedCacheState state = SavedCacheState::Missing;  // What the file was.
    std::vector<char> data;                            // The driver's data; empty unless Intact.
};

// Reads the pipeline cache saved at `path`. A damaged file is reported, not removed: the next save
// replaces it.
[[nodiscard]] SavedCache read_pipeline_cache(const std::filesystem::path& path);

// Saves the driver's cache `data` at `path`, making its folder if needed: written whole to a
// temporary file of this call's own beside it, then renamed over `path`. False with `error` set
// when it cannot; `path` then keeps what it held, and the temporary file is removed.
[[nodiscard]] bool write_pipeline_cache(
    const std::filesystem::path& path, std::span<const char> data, std::string& error
);

}  // namespace openrac::vkgl
