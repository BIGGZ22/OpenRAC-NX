// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The pipeline cache's files (pipeline_cache_file.h): the driver's data and the manifest, each in
// cache_file.h's envelope under a magic of its own.
//
// The manifest's data is little-endian: its format, its key size and its count as 32-bit numbers,
// then each entry as its program (64 bits), its age (32 bits) and its key's bytes.

#include "renderer/vulkan/pipeline_cache_file.h"

#include <climits>
#include <format>

namespace openrac::vkgl {

namespace {

// The pipeline cache's magic: OpenRAC Vulkan pipeline cache, format 1.
constexpr CacheMagic kCacheMagic{'O', 'R', 'V', 'K', 'P', 'C', '0', '1'};

// The manifest's magic: OpenRAC Vulkan pipeline manifest. Its data says its format.
constexpr CacheMagic kManifestMagic{'O', 'R', 'V', 'K', 'P', 'M', 'A', 'N'};

// The bytes of the manifest's header (format, key size, count) and of an entry's numbers.
constexpr std::size_t kHeaderBytes = 3 * sizeof(std::uint32_t);
constexpr std::size_t kEntryNumberBytes = sizeof(std::uint64_t) + sizeof(std::uint32_t);

// Appends the `bytes` low bytes of `value` to `out`, little-endian.
void put(std::vector<char>& out, std::uint64_t value, std::size_t bytes) {
    // Byte n holds bits 8n to 8n+7.
    for (std::size_t n = 0; n < bytes; ++n) {
        out.push_back(static_cast<char>(static_cast<unsigned char>(value >> (CHAR_BIT * n))));
    }
}

// Returns the little-endian value of the `bytes` bytes at `in`.
std::uint64_t get(const char* in, std::size_t bytes) {
    std::uint64_t value = 0;

    // Byte n holds bits 8n to 8n+7.
    for (std::size_t n = 0; n < bytes; ++n) {
        value |= std::uint64_t{static_cast<unsigned char>(in[n])} << (CHAR_BIT * n);
    }

    return value;
}

}  // namespace

SavedCache read_pipeline_cache(const std::filesystem::path& path) {
    return read_cache_file(path, kCacheMagic);
}

bool write_pipeline_cache(
    const std::filesystem::path& path, std::span<const char> data, std::string& error
) {
    return write_cache_file(path, kCacheMagic, data, error);
}

SavedManifest read_pipeline_manifest(const std::filesystem::path& path, std::size_t key_bytes) {
    CacheFile file = read_cache_file(path, kManifestMagic);

    // No file, or not a whole one.
    if (file.state != CacheFileState::Intact) {
        return {file.state, {}};
    }

    const std::vector<char>& data = file.data;

    // Too short for its header: not a manifest this code wrote.
    if (data.size() < kHeaderBytes) {
        return {CacheFileState::Damaged, {}};
    }

    const std::uint64_t format = get(data.data(), sizeof(std::uint32_t));
    const std::uint64_t size = get(data.data() + sizeof(std::uint32_t), sizeof(std::uint32_t));
    const std::uint64_t count = get(data.data() + 2 * sizeof(std::uint32_t), sizeof(std::uint32_t));
    const std::size_t entry_bytes = kEntryNumberBytes + key_bytes;

    // Another format or another key: written by another build, so none of its keys is this one's.
    if (format != kManifestFormat || size != key_bytes
        || data.size() != kHeaderBytes + count * entry_bytes) {
        return {CacheFileState::Damaged, {}};
    }

    SavedManifest manifest{CacheFileState::Intact, {}};
    manifest.entries.reserve(count);

    // Each entry: its program, its age, its key.
    for (std::size_t at = kHeaderBytes; at < data.size(); at += entry_bytes) {
        ManifestEntry& entry = manifest.entries.emplace_back();
        entry.program = get(data.data() + at, sizeof(std::uint64_t));
        entry.age = static_cast<std::uint32_t>(
            get(data.data() + at + sizeof(std::uint64_t), sizeof(std::uint32_t))
        );
        const char* key = data.data() + at + kEntryNumberBytes;
        entry.key.assign(key, key + key_bytes);
    }

    return manifest;
}

bool write_pipeline_manifest(
    const std::filesystem::path& path,
    std::span<const ManifestEntry> entries,
    std::size_t key_bytes,
    std::string& error
) {
    std::vector<char> data;
    data.reserve(kHeaderBytes + entries.size() * (kEntryNumberBytes + key_bytes));
    put(data, kManifestFormat, sizeof(std::uint32_t));
    put(data, key_bytes, sizeof(std::uint32_t));
    put(data, entries.size(), sizeof(std::uint32_t));

    // Each entry: its program, its age, its key.
    for (const ManifestEntry& entry : entries) {
        // A key of another size would make the file unreadable.
        if (entry.key.size() != key_bytes) {
            error =
                std::format("a key of {} bytes in a manifest of {}", entry.key.size(), key_bytes);
            return false;
        }

        put(data, entry.program, sizeof(std::uint64_t));
        put(data, entry.age, sizeof(std::uint32_t));
        data.insert(data.end(), entry.key.begin(), entry.key.end());
    }

    return write_cache_file(path, kManifestMagic, data, error);
}

}  // namespace openrac::vkgl
