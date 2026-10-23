// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-extract/src/identify.rs
// and crates/rc-formats/src/tfrag_light.rs: ISC License, Copyright (c) 2026 ReRAC
// contributors.
//
// SYSTEM.CNF and ELF segment reads.

#include "assets/disc/boot.h"

#include <cstring>

namespace openrac::assets::disc {

namespace {

std::string_view trim(std::string_view s) {
    while (!s.empty() && (s.front() == ' ' || s.front() == '\t')) {
        s.remove_prefix(1);
    }

    while (!s.empty() && (s.back() == ' ' || s.back() == '\t' || s.back() == '\r')) {
        s.remove_suffix(1);
    }

    return s;
}

}  // namespace

std::optional<std::string> cnf_value(std::string_view text, std::string_view key) {
    std::size_t at = 0;

    while (at <= text.size()) {
        std::size_t end = text.find_first_of("\r\n", at);

        if (end == std::string_view::npos) {
            end = text.size();
        }

        const std::string_view line = text.substr(at, end - at);

        if (const auto eq = line.find('='); eq != std::string_view::npos) {
            if (trim(line.substr(0, eq)) == key) {
                std::string_view value = trim(line.substr(eq + 1));

                if (const auto semi = value.find(';'); semi != std::string_view::npos) {
                    value = trim(value.substr(0, semi));
                }

                return std::string(value);
            }
        }

        at = end + 1;
    }

    return std::nullopt;
}

std::optional<SystemCnf> SystemCnf::parse(std::string_view text) {
    const auto boot = cnf_value(text, "BOOT2");

    if (!boot) {
        return std::nullopt;
    }

    SystemCnf cnf;
    std::string path = *boot;

    if (path.starts_with("cdrom0:")) {
        path.erase(0, 7);
    }

    for (char& c : path) {
        if (c == '\\') {
            c = '/';
        }
    }

    if (path.empty() || path.front() != '/') {
        path.insert(path.begin(), '/');
    }

    cnf.serial = path.substr(path.rfind('/') + 1);
    cnf.boot = std::move(path);
    cnf.version = cnf_value(text, "VER").value_or("");
    cnf.vmode = cnf_value(text, "VMODE").value_or("");
    return cnf;
}

ByteView elf_read(ByteView elf, u32 vaddr, std::size_t length) {
    if (elf.size() < 4
        || std::memcmp(
               elf.data(),
               "\x7f"
               "ELF",
               4
           ) != 0) {
        fail("not an ELF file");
    }
    const u32 phoff = elf.u32_at(0x1c);
    const u16 phentsize = elf.u16_at(0x2a);
    const u16 phnum = elf.u16_at(0x2c);
    for (u32 i = 0; i < phnum; ++i) {
        const std::size_t ph = std::size_t{phoff} + std::size_t{i} * phentsize;
        const u32 type = elf.u32_at(ph);
        const u32 offset = elf.u32_at(ph + 4);
        const u32 va = elf.u32_at(ph + 8);
        const u32 filesz = elf.u32_at(ph + 16);
        if (type == 1 && vaddr >= va && u64{vaddr - va} + length <= filesz) {
            return elf.sub(std::size_t{offset} + (vaddr - va), length, "ELF segment data");
        }
    }
    fail("address {:#x}+{:#x} is not in any ELF load segment", vaddr, length);
}

}  // namespace openrac::assets::disc
