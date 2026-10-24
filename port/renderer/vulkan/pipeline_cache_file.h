// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The pipeline cache's files on disk, both in cache_file.h's envelope, so a file cut short or
// mixed from two writers is found and dropped rather than read:
//
// - the driver's cache data (vkGetPipelineCacheData). MoltenVK has accepted a damaged one and
//   drawn wrong colours with it (measured: `vulkan_uniform` under `ctest -j8`);
// - the manifest beside it: the keys of the pipelines made, each with the identity of its
//   program, so that a later run makes them again before its first draw needs them (R-10,
//   docs/port/RENDER_PERF.md).
//
// This file knows nothing of Vulkan or of a key's fields; whether the data suits the device and
// the build is the caller's check.

#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <span>
#include <string>
#include <vector>

#include "renderer/vulkan/cache_file.h"

namespace openrac::vkgl {

// What reading a saved pipeline cache found: Missing, Damaged or Intact.
using SavedCacheState = CacheFileState;

// A saved pipeline cache as read back: its state, and the driver's data when it is intact.
using SavedCache = CacheFile;

// Reads the pipeline cache saved at `path`. A damaged file is reported, not removed: the next save
// replaces it.
[[nodiscard]] SavedCache read_pipeline_cache(const std::filesystem::path& path);

// Saves the driver's cache `data` at `path`, making its folder if needed, through a temporary file
// of this call's own. False with `error` set when it cannot; `path` then keeps what it held.
[[nodiscard]] bool write_pipeline_cache(
    const std::filesystem::path& path, std::span<const char> data, std::string& error
);

// The manifest's format: read only when the file's is this one. A change of what a key holds
// raises it, and an older manifest is then dropped whole: 2 since R-11 put the fixed state in a
// struct of its own and added whether it is dynamic.
inline constexpr std::uint32_t kManifestFormat = 2;

// One pipeline of the manifest: its program's identity, how many runs have passed since one made
// or used it, and its key's bytes, as the backend's PipelineKey lays them out.
struct ManifestEntry {
    std::uint64_t program = 0;  // The identity of the key's program (a hash of its SPIR-V).
    std::uint32_t age = 0;      // Runs since this key's pipeline was last made or used.
    std::vector<char> key;      // The key's bytes, `key_bytes` of them.
};

// A manifest as read back: its state, and its entries when it is intact.
struct SavedManifest {
    CacheFileState state = CacheFileState::Missing;  // What the file was.
    std::vector<ManifestEntry> entries;              // Its keys; empty unless Intact.
};

// Reads the manifest at `path`, whose keys must be `key_bytes` long. A manifest of another format
// or of another key size, or whose entries do not fill it exactly, is Damaged.
[[nodiscard]] SavedManifest read_pipeline_manifest(
    const std::filesystem::path& path, std::size_t key_bytes
);

// Saves `entries` as the manifest at `path`, each key `key_bytes` long, as write_pipeline_cache
// saves. False with `error` set when it cannot, or when an entry's key is not `key_bytes` long.
[[nodiscard]] bool write_pipeline_manifest(
    const std::filesystem::path& path,
    std::span<const ManifestEntry> entries,
    std::size_t key_bytes,
    std::string& error
);

}  // namespace openrac::vkgl
