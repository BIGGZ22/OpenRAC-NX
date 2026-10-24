// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The envelope of the backend's cache files: a file's data behind its magic, its length and its
// 64-bit FNV-1a hash, so that a file cut short, mixed from two writers or of another format is
// found and dropped rather than read. The pipeline cache (pipeline_cache_file.h) and the pipeline
// manifest keep their data in it; each names its own magic, whose last bytes are its format.
//
// Several programs save the same file at once (parallel tests, a game and a tool): each writes a
// temporary file of its own and renames it into place, so a reader sees one whole save or another.
// This file knows nothing of Vulkan or of what the data holds.

#pragma once

#include <array>
#include <filesystem>
#include <span>
#include <string>
#include <vector>

namespace openrac::vkgl {

// The first bytes of a cache file: what it is and the format of its data, as eight letters.
using CacheMagic = std::array<char, 8>;

// What reading a cache file found.
enum class CacheFileState {
    Missing,  // No file, or one that cannot be read.
    Damaged,  // A file this code did not write whole: cut short, mixed, or of another magic.
    Intact,   // A whole save of this magic; its data is the writer's.
};

// A cache file as read back: its state, and its data when it is intact.
struct CacheFile {
    CacheFileState state = CacheFileState::Missing;  // What the file was.
    std::vector<char> data;                          // The data saved; empty unless Intact.
};

// Reads the cache file at `path` whose magic must be `magic`. A damaged file is reported, not
// removed: the next save replaces it.
[[nodiscard]] CacheFile read_cache_file(const std::filesystem::path& path, const CacheMagic& magic);

// Saves `data` behind `magic` at `path`, making its folder if needed: written whole to a temporary
// file of this call's own beside it, then renamed over `path`. False with `error` set when it
// cannot; `path` then keeps what it held, and the temporary file is removed.
[[nodiscard]] bool write_cache_file(
    const std::filesystem::path& path,
    const CacheMagic& magic,
    std::span<const char> data,
    std::string& error
);

}  // namespace openrac::vkgl
