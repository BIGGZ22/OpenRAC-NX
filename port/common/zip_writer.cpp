// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

#include "common/zip_writer.h"

#include <array>
#include <fstream>

namespace openrac::zip_writer {

namespace {

constexpr std::array<std::uint32_t, 256> kCrcTable = [] {
    std::array<std::uint32_t, 256> table{};
    for (std::uint32_t i = 0; i < 256; ++i) {
        std::uint32_t c = i;
        for (int k = 0; k < 8; ++k) {
            c = (c & 1u) != 0 ? 0xEDB88320u ^ (c >> 1) : c >> 1;
        }
        table[i] = c;
    }
    return table;
}();

void put16(std::vector<std::uint8_t>& out, std::uint32_t value) {
    out.push_back(static_cast<std::uint8_t>(value));
    out.push_back(static_cast<std::uint8_t>(value >> 8));
}

void put32(std::vector<std::uint8_t>& out, std::uint32_t value) {
    put16(out, value & 0xFFFFu);
    put16(out, value >> 16);
}

}  // namespace

std::uint32_t crc32(std::span<const std::uint8_t> data) {
    std::uint32_t c = 0xFFFFFFFFu;
    for (const std::uint8_t byte : data) {
        c = kCrcTable[(c ^ byte) & 0xFFu] ^ (c >> 8);
    }
    return c ^ 0xFFFFFFFFu;
}

std::vector<std::uint8_t> archive(const std::vector<Entry>& entries, Deflate deflate) {
    // 1980-01-01 00:00, the earliest time zip has: the files' own times say nothing here.
    constexpr std::uint32_t kTime = 0, kDate = (0u << 9) | (1u << 5) | 1u;
    std::vector<std::uint8_t> out, directory;
    for (const Entry& entry : entries) {
        const std::uint32_t crc = crc32(entry.data);
        std::vector<std::uint8_t> packed = deflate != nullptr && !entry.data.empty()
                                               ? deflate(entry.data)
                                               : std::vector<std::uint8_t>{};
        const bool deflated = !packed.empty() && packed.size() < entry.data.size();
        const std::span<const std::uint8_t> body = deflated
                                                       ? std::span<const std::uint8_t>(packed)
                                                       : std::span<const std::uint8_t>(entry.data);
        const std::uint32_t method = deflated ? 8 : 0;
        const auto offset = static_cast<std::uint32_t>(out.size());
        // The local header, then the file.
        put32(out, 0x04034B50u);
        put16(out, 20);        // the version needed: 2.0, deflate
        put16(out, 1u << 11);  // the name is UTF-8
        put16(out, method);
        put16(out, kTime);
        put16(out, kDate);
        put32(out, crc);
        put32(out, static_cast<std::uint32_t>(body.size()));
        put32(out, static_cast<std::uint32_t>(entry.data.size()));
        put16(out, static_cast<std::uint32_t>(entry.name.size()));
        put16(out, 0);
        out.insert(out.end(), entry.name.begin(), entry.name.end());
        out.insert(out.end(), body.begin(), body.end());
        // Its entry in the central directory.
        put32(directory, 0x02014B50u);
        put16(directory, 20);  // made by: 2.0, MS-DOS attributes
        put16(directory, 20);
        put16(directory, 1u << 11);
        put16(directory, method);
        put16(directory, kTime);
        put16(directory, kDate);
        put32(directory, crc);
        put32(directory, static_cast<std::uint32_t>(body.size()));
        put32(directory, static_cast<std::uint32_t>(entry.data.size()));
        put16(directory, static_cast<std::uint32_t>(entry.name.size()));
        put16(directory, 0);  // extra field
        put16(directory, 0);  // comment
        put16(directory, 0);  // disk
        put16(directory, 0);  // internal attributes
        put32(directory, 0);  // external attributes
        put32(directory, offset);
        directory.insert(directory.end(), entry.name.begin(), entry.name.end());
    }
    const auto directory_offset = static_cast<std::uint32_t>(out.size());
    out.insert(out.end(), directory.begin(), directory.end());
    put32(out, 0x06054B50u);
    put16(out, 0);
    put16(out, 0);
    put16(out, static_cast<std::uint32_t>(entries.size()));
    put16(out, static_cast<std::uint32_t>(entries.size()));
    put32(out, static_cast<std::uint32_t>(directory.size()));
    put32(out, directory_offset);
    put16(out, 0);
    return out;
}

bool write(
    const std::filesystem::path& path,
    const std::vector<Entry>& entries,
    Deflate deflate,
    std::string& error
) {
    const std::vector<std::uint8_t> bytes = archive(entries, deflate);
    std::ofstream out(path, std::ios::binary);
    if (out) {
        out.write(
            reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size())
        );
    }
    if (!out) {
        error = "could not write " + path.string();
        return false;
    }
    return true;
}

}  // namespace openrac::zip_writer
