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
    // Strip leading blanks so "BOOT2 = x" and "BOOT2=x" parse alike.
    while (!s.empty() && (s.front() == ' ' || s.front() == '\t')) {
        s.remove_prefix(1);
    }

    // Strip trailing blanks and the carriage return that a CRLF text file leaves.
    while (!s.empty() && (s.back() == ' ' || s.back() == '\t' || s.back() == '\r')) {
        s.remove_suffix(1);
    }

    return s;
}

}  // namespace

std::optional<std::string> cnf_value(std::string_view text, std::string_view key) {
    std::size_t at = 0;

    // Walk the text one line at a time, stopping when the cursor passes the end.
    while (at <= text.size()) {
        std::size_t end = text.find_first_of("\r\n", at);

        // The last line may have no terminator; it then runs to the end of the text.
        if (end == std::string_view::npos) {
            end = text.size();
        }

        const std::string_view line = text.substr(at, end - at);

        // Only lines of the form key = value matter; other lines are skipped.
        if (const auto eq = line.find('='); eq != std::string_view::npos) {
            // The key is whatever stands before the equals sign, without its blanks.
            if (trim(line.substr(0, eq)) == key) {
                std::string_view value = trim(line.substr(eq + 1));

                // SYSTEM.CNF lines may carry a trailing ";" comment, which is not part of the
                // value.
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

    // A disc with no BOOT2 line has no boot program to identify.
    if (!boot) {
        return std::nullopt;
    }

    SystemCnf cnf;
    std::string path = *boot;

    // The boot path usually starts with the device name "cdrom0:", which the file system does not
    // know.
    if (path.starts_with("cdrom0:")) {
        path.erase(0, 7);
    }

    // SYSTEM.CNF paths use backslashes; the file system walk uses slashes.
    for (char& c : path) {
        // Convert one separator at a time, leaving other characters as they are.
        if (c == '\\') {
            c = '/';
        }
    }

    // The file system lookup wants an absolute path, so add the root when the path has none.
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
    // An ELF image starts with the four bytes 0x7f "ELF"; anything else is not one.
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
    // Walk the program headers to find the load segment that holds the address.
    for (u32 i = 0; i < phnum; ++i) {
        const std::size_t ph = std::size_t{phoff} + std::size_t{i} * phentsize;
        const u32 type = elf.u32_at(ph);
        const u32 offset = elf.u32_at(ph + 4);
        const u32 va = elf.u32_at(ph + 8);
        const u32 filesz = elf.u32_at(ph + 16);
        // Only loadable segments (type 1) count, and the range must lie in the segment's file data.
        if (type == 1 && vaddr >= va && u64{vaddr - va} + length <= filesz) {
            return elf.sub(std::size_t{offset} + (vaddr - va), length, "ELF segment data");
        }
    }
    fail("address {:#x}+{:#x} is not in any ELF load segment", vaddr, length);
}

}  // namespace openrac::assets::disc
