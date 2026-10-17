// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

#include "openrac/elf.h"

#include <algorithm>
#include <cstring>
#include <format>
#include <fstream>

#include "openrac/memory.h"

namespace openrac::runtime {
namespace {

std::uint16_t u16(const std::vector<std::uint8_t>& f, std::size_t at) {
    return static_cast<std::uint16_t>(f[at] | f[at + 1] << 8);
}

std::uint32_t u32(const std::vector<std::uint8_t>& f, std::size_t at) {
    return static_cast<std::uint32_t>(f[at]) | static_cast<std::uint32_t>(f[at + 1]) << 8
           | static_cast<std::uint32_t>(f[at + 2]) << 16
           | static_cast<std::uint32_t>(f[at + 3]) << 24;
}

constexpr std::uint32_t kPtLoad = 1;
constexpr std::uint16_t kMachineMips = 8;

// The segments as load_elf placed them, for openrac_guest_restore_executable.
struct Loaded {
    gaddr address;
    std::vector<std::uint8_t> bytes;
};
std::vector<Loaded> g_loaded;

}  // namespace

bool read_elf(const std::vector<std::uint8_t>& file, ElfImage* image, std::string* reason) {
    if (file.size() < 52
        || std::memcmp(
               file.data(),
               "\x7f"
               "ELF",
               4
           ) != 0) {
        *reason = "not an ELF file";
        return false;
    }
    if (file[4] != 1 || file[5] != 1) {
        *reason = "not a 32-bit little-endian ELF file";
        return false;
    }
    if (u16(file, 18) != kMachineMips) {
        *reason = "not a MIPS program";
        return false;
    }
    image->entry = u32(file, 24);
    const std::uint32_t phoff = u32(file, 28);
    const std::uint16_t phentsize = u16(file, 42);
    const std::uint16_t phnum = u16(file, 44);
    if (phentsize < 32 || phoff + static_cast<std::uint64_t>(phentsize) * phnum > file.size()) {
        *reason = "its program headers run past the end of the file";
        return false;
    }
    image->segments.clear();
    for (std::uint16_t i = 0; i < phnum; i++) {
        const std::size_t at = phoff + static_cast<std::size_t>(i) * phentsize;
        if (u32(file, at) != kPtLoad) {
            continue;
        }
        const std::uint32_t offset = u32(file, at + 4);
        ElfSegment seg{
            u32(file, at + 8), u32(file, at + 16), u32(file, at + 20), u32(file, at + 24)
        };
        if (static_cast<std::uint64_t>(offset) + seg.file_size > file.size()
            || seg.file_size > seg.memory_size) {
            *reason = std::format("segment {} runs past the end of the file", i);
            return false;
        }
        image->segments.push_back(seg);
    }
    if (image->segments.empty()) {
        *reason = "no loadable segments";
        return false;
    }
    return true;
}

bool load_elf(const std::vector<std::uint8_t>& file, const ElfImage& image, std::string* reason) {
    Memory& memory = Memory::get();
    const std::uint32_t phoff = u32(file, 28);
    const std::uint16_t phentsize = u16(file, 42);
    const std::uint16_t phnum = u16(file, 44);
    std::size_t loaded = 0;
    std::vector<Loaded> placed;
    for (std::uint16_t i = 0; i < phnum && loaded < image.segments.size(); i++) {
        const std::size_t at = phoff + static_cast<std::size_t>(i) * phentsize;
        if (u32(file, at) != kPtLoad) {
            continue;
        }
        const ElfSegment& seg = image.segments[loaded++];
        if (!memory.mapped(seg.address, seg.memory_size)) {
            *reason = std::format(
                "segment at {:#x} ({:#x} bytes) is outside game memory",
                seg.address,
                seg.memory_size
            );
            return false;
        }
        auto dst = memory.bytes(seg.address, seg.memory_size);
        std::memcpy(dst.data(), file.data() + u32(file, at + 4), seg.file_size);
        std::memset(dst.data() + seg.file_size, 0, seg.memory_size - seg.file_size);
        placed.push_back(Loaded{seg.address, std::vector<std::uint8_t>(dst.begin(), dst.end())});
    }
    g_loaded = std::move(placed);
    return true;
}

bool read_file(const std::filesystem::path& path, std::vector<std::uint8_t>* out) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        return false;
    }
    in.seekg(0, std::ios::end);
    const auto size = in.tellg();
    in.seekg(0, std::ios::beg);
    out->resize(static_cast<std::size_t>(size));
    return static_cast<bool>(in.read(reinterpret_cast<char*>(out->data()), size));
}

}  // namespace openrac::runtime

extern "C" uint32_t openrac_guest_restore_executable(gaddr low, gaddr high) {
    using openrac::runtime::Memory;
    std::uint32_t restored = 0;
    for (const auto& seg : openrac::runtime::g_loaded) {
        const gaddr end = seg.address + static_cast<gaddr>(seg.bytes.size());
        const gaddr from = std::max(low, seg.address), to = std::min(high, end);
        if (from >= to) {
            continue;
        }
        auto dst = Memory::get().bytes(from, to - from);
        std::memcpy(dst.data(), seg.bytes.data() + (from - seg.address), to - from);
        restored += to - from;
    }
    return restored;
}
