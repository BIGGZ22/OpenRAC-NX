// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// A zip archive written in one go: a few named files, each deflated by a function the caller
// gives (the port has no compressor of its own in common/; the window's stb_image_write has one)
// or stored as it is when there is none or deflating does not make it smaller. Every system opens
// it (Windows' Explorer, macOS' Archive Utility, Android's Files, unzip). For the performance
// capture's report and frames (game/common/debug/capture.cpp), which shrink about tenfold.

#pragma once

#include <cstdint>
#include <filesystem>
#include <span>
#include <string>
#include <vector>

namespace openrac::zip_writer {

struct Entry {
    std::string name;  // its path in the archive, with '/' between folders
    std::vector<std::uint8_t> data;
};

// Raw deflate (RFC 1951, no zlib header) of `data`; empty when it could not.
using Deflate = std::vector<std::uint8_t> (*)(std::span<const std::uint8_t> data);

// The CRC-32 zip keeps of each file (the IEEE polynomial, as zlib's crc32).
std::uint32_t crc32(std::span<const std::uint8_t> data);

// The archive's bytes. Each file is at most 4 GiB less a byte and there are at most 65535 (no
// zip64); `deflate` may be null.
std::vector<std::uint8_t> archive(const std::vector<Entry>& entries, Deflate deflate);

// Writes archive(entries, deflate) to `path`; false with `error` set when it could not.
bool write(
    const std::filesystem::path& path,
    const std::vector<Entry>& entries,
    Deflate deflate,
    std::string& error
);

}  // namespace openrac::zip_writer
