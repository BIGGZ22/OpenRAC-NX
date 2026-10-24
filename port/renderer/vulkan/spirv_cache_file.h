// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The SPIR-V cache: what glslang made of each program and each stage the backend compiled, kept
// beside the pipeline cache in cache_file.h's envelope so that a warm start and a level load run
// no glslang (R-12, docs/port/RENDER_PERF.md). shader_compiler.cpp keys it and decides what to
// keep; this file holds the cache in memory, its file, and the encoding of a CompiledProgram.
//
// An entry is found by a 64-bit key, and taken only when a second hash of the same text and its
// length match too, so a collision of the key alone cannot hand one shader another's code. The
// file says its format and the compiler it was made with; another of either drops it whole. Only
// sources the port itself compiled are kept: nothing of the player's files.

#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "renderer/vulkan/cache_file.h"
#include "renderer/vulkan/shader_compiler.h"

namespace openrac::vkgl {

// The SPIR-V cache file's format: read only when the file's is this one.
inline constexpr std::uint32_t kSpirvCacheFormat = 1;

// The entries a SPIR-V cache holds at most; past it a new result is not kept (assumed far above
// the port's few hundred programs and stages).
inline constexpr std::size_t kSpirvCacheEntries = 8192;

// One compiled result: what was compiled, as a key, a check and a length, and the result.
struct SpirvCacheEntry {
    std::uint64_t key = 0;     // The text's 64-bit hash, which finds the entry.
    std::uint64_t check = 0;   // A second hash of the same text, compared on a hit.
    std::uint64_t length = 0;  // The text's length in bytes, compared on a hit.
    std::vector<char>
        value;  // The result: encode_program's bytes, or none for a stage that compiled.
};

// A SPIR-V cache file as read back: its state, and its entries when it is intact.
struct SavedSpirvCache {
    CacheFileState state = CacheFileState::Missing;  // What the file was.
    std::vector<SpirvCacheEntry> entries;            // Its entries; empty unless Intact.
};

// Reads the SPIR-V cache at `path`. A file of another format, made by another `compiler` (its
// version text), cut short, or whose entries do not fill it exactly is Damaged.
[[nodiscard]] SavedSpirvCache read_spirv_cache(
    const std::filesystem::path& path, std::string_view compiler
);

// Saves `entries` as the SPIR-V cache at `path`, made by `compiler`, whole through a temporary
// file of its own. False with `error` set when it cannot.
[[nodiscard]] bool write_spirv_cache(
    const std::filesystem::path& path,
    std::string_view compiler,
    std::span<const SpirvCacheEntry> entries,
    std::string& error
);

// Returns `program` as bytes: every field of a CompiledProgram, in order, little-endian.
[[nodiscard]] std::vector<char> encode_program(const CompiledProgram& program);

// Reads bytes encode_program wrote into `program`. False, with `program` unspecified, when they
// are not a whole encoding of one.
[[nodiscard]] bool decode_program(std::span<const char> bytes, CompiledProgram& program);

// The SPIR-V cache of one open backend: its entries in memory, read from its file when the
// backend opens and written back when it closes, if anything was added.
//
// Owned by the backend's Context. Only the thread that draws, which compiles, uses it.
class SpirvCache {
public:
    // Reads the cache at `path` for `compiler`; an empty `path` keeps no cache. A damaged or
    // stale file is dropped and its replacement saved at close. Returns the file's state.
    CacheFileState open(const std::filesystem::path& path, std::string compiler);

    // True when a cache is kept (open was given a path).
    [[nodiscard]] bool active() const { return !m_path.empty(); }

    // Returns the result kept for the text of `key`, `check` and `length`, or null.
    [[nodiscard]] const std::vector<char>* find(
        std::uint64_t key, std::uint64_t check, std::uint64_t length
    ) const;

    // Keeps `entry` for the rest of the run and the next, unless the cache is full; replaces an
    // entry of the same key (a collision is rare and the newer text wins).
    void insert(SpirvCacheEntry entry);

    // Writes the cache back when an entry was added since open. False with `error` set when it
    // cannot; the next run then compiles again what it lacks.
    [[nodiscard]] bool save(std::string& error);

private:
    std::filesystem::path m_path;            // The file; empty when no cache is kept.
    std::string m_compiler;                  // The compiler's version text the file must name.
    std::vector<SpirvCacheEntry> m_entries;  // Every entry.
    std::unordered_map<std::uint64_t, std::size_t> m_index;  // Key to its place in m_entries.
    bool m_changed = false;                                  // An entry was added since open.
};

}  // namespace openrac::vkgl
