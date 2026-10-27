// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The zip writer (common/zip_writer.h): zlib's CRC-32, an archive's headers and central directory
// read back field by field, a deflated entry kept and one that deflating would grow stored.

#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "check.h"
#include "common/zip_writer.h"

namespace {

namespace zip = openrac::zip_writer;

std::vector<std::uint8_t> bytes(std::string_view text) {
    return {text.begin(), text.end()};
}

std::uint32_t read16(const std::vector<std::uint8_t>& b, std::size_t at) {
    return static_cast<std::uint32_t>(b[at] | b[at + 1] << 8);
}

std::uint32_t read32(const std::vector<std::uint8_t>& b, std::size_t at) {
    return read16(b, at) | read16(b, at + 2) << 16;
}

void crc() {
    CHECK(zip::crc32(bytes("123456789")) == 0xCBF43926u);
    CHECK(zip::crc32({}) == 0u);
}

// A "compressor" that halves a run of one byte, enough to see the archive keep what it gives.
std::vector<std::uint8_t> halve(std::span<const std::uint8_t> data) {
    return std::vector<std::uint8_t>(data.size() / 2, 0x55);
}

// One that only grows its input: the entry is stored instead.
std::vector<std::uint8_t> grow(std::span<const std::uint8_t> data) {
    return std::vector<std::uint8_t>(data.size() + 5, 0);
}

void stored() {
    const std::vector<zip::Entry> entries =
        {{"a.md", bytes("report")}, {"b/c.jsonl", bytes("{}\n")}};
    const std::vector<std::uint8_t> b = zip::archive(entries, nullptr);
    // The first local header and its file.
    CHECK(read32(b, 0) == 0x04034B50u);
    CHECK(read16(b, 8) == 0);  // stored
    CHECK(read32(b, 14) == zip::crc32(bytes("report")));
    CHECK(read32(b, 18) == 6 && read32(b, 22) == 6);
    CHECK(read16(b, 26) == 4);
    CHECK(std::string(b.begin() + 30, b.begin() + 34) == "a.md");
    CHECK(std::string(b.begin() + 34, b.begin() + 40) == "report");
    // The end record: two entries, and the central directory where it says.
    const std::size_t end = b.size() - 22;
    CHECK(read32(b, end) == 0x06054B50u);
    CHECK(read16(b, end + 8) == 2 && read16(b, end + 10) == 2);
    const std::size_t directory = read32(b, end + 16);
    CHECK(directory + read32(b, end + 12) == end);
    CHECK(read32(b, directory) == 0x02014B50u);
    CHECK(read32(b, directory + 42) == 0);  // the first file's local header
    const std::size_t second = directory + 46 + 4;
    CHECK(read32(b, second) == 0x02014B50u);
    CHECK(read32(b, second + 42) == 40);
    CHECK(
        std::string(
            b.begin() + static_cast<std::ptrdiff_t>(second + 46),
            b.begin() + static_cast<std::ptrdiff_t>(second + 55)
        )
        == "b/c.jsonl"
    );
}

void deflated() {
    const std::vector<zip::Entry> entries = {{"x", std::vector<std::uint8_t>(100, 'x')}};
    const std::vector<std::uint8_t> b = zip::archive(entries, halve);
    CHECK(read16(b, 8) == 8);  // deflate
    CHECK(read32(b, 18) == 50 && read32(b, 22) == 100);
    CHECK(read32(b, 14) == zip::crc32(entries[0].data));  // of the file, not of what was packed
    const std::vector<std::uint8_t> kept = zip::archive(entries, grow);
    CHECK(read16(kept, 8) == 0);
    CHECK(read32(kept, 18) == 100);
}

}  // namespace

int main() {
    crc();
    stored();
    deflated();
    return openrac::test::result();
}
