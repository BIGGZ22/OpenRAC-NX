// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

/**
 * Reading the table of a game's vector unit programs and summing a unit's use by program (see
 * vu_programs.h).
 */

#include "vu_programs.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <map>
#include <string>

namespace ps2 {
namespace {

/** The bytes of one chunk slot of program memory: one MPG command loads at most this much. */
constexpr u32 kSlotBytes = 0x800;

/** The size of an ELF32 file header, a section header and a program header (documented). */
constexpr u32 kFileHeaderBytes = 52, kSectionHeaderBytes = 40, kProgramHeaderBytes = 32;

/** The size of one record of `.DVP.ovlytab`: name offset, EE address, unit address. */
constexpr u32 kRecordBytes = 12;

/** What the chunk sections' names begin with. */
constexpr char kChunkPrefix[] = ".DVP.overlay..";

/** One section of an ELF file. */
struct Section {
    std::string name;  // Its name, from the section name table.
    u32 offset = 0;    // Where its bytes are in the file.
    u32 size = 0;      // How many bytes it has.
};

/**
 * Reads a little-endian 32-bit word of the file.
 *
 * @param elf The file.
 * @param at The word's offset; the caller has checked that it is inside.
 * @return The word.
 */
u32 word(const std::vector<u8>& elf, std::size_t at) {
    return load<u32>(&elf[at]);
}

/**
 * Reads a zero-terminated string of the file.
 *
 * @param elf The file.
 * @param at Where the string starts.
 * @return The string, up to the end of the file at most.
 */
std::string text(const std::vector<u8>& elf, std::size_t at) {
    std::string out;

    // Up to the terminating zero or the end of the file.
    while (at < elf.size() && elf[at] != 0) {
        out += static_cast<char>(elf[at++]);
    }

    return out;
}

/**
 * Lists the sections of an ELF32 file.
 *
 * @param elf The file.
 * @return Its sections; empty if the section table is missing or does not fit in the file.
 */
std::vector<Section> sections(const std::vector<u8>& elf) {
    std::vector<Section> out;

    // Too short for a file header, or not an ELF.
    if (elf.size() < kFileHeaderBytes
        || std::memcmp(
               elf.data(),
               "\x7F"
               "ELF",
               4
           ) != 0) {
        return out;
    }

    // File header: section table offset at byte 32, count at 48, name table's index at 50.
    u32 table = word(elf, 32);
    u32 count = load<u16>(&elf[48]), names = load<u16>(&elf[50]);

    // No section table, or one that runs past the end of the file.
    if (table == 0 || names >= count
        || u64{table} + u64{count} * kSectionHeaderBytes > elf.size()) {
        return out;
    }

    // Section header: name offset at byte 0, file offset at 16, size at 20.
    u32 name_table = word(elf, table + names * kSectionHeaderBytes + 16);

    for (u32 n = 0; n < count; n++) {
        std::size_t header = table + n * kSectionHeaderBytes;
        Section section;

        section.name = text(elf, u64{name_table} + word(elf, header));
        section.offset = word(elf, header + 16);
        section.size = word(elf, header + 20);
        out.push_back(section);
    }

    return out;
}

/**
 * Finds where an address of the loaded program is in the file.
 *
 * @param elf The file.
 * @param address The address.
 * @param bytes How many bytes from there are wanted.
 * @return The file offset, or 0 if no loadable segment holds all the bytes.
 */
u32 file_offset(const std::vector<u8>& elf, u32 address, u32 bytes) {
    // File header: program header table offset at byte 28, count at 44.
    u32 table = word(elf, 28);
    u32 count = load<u16>(&elf[44]);

    for (u32 n = 0; n < count; n++) {
        std::size_t header = table + n * kProgramHeaderBytes;

        // The table runs past the end of the file.
        if (header + kProgramHeaderBytes > elf.size()) {
            break;
        }

        // Program header: type at byte 0 (1 is loadable), file offset 4, address 8, file size 16.
        u32 type = word(elf, header), offset = word(elf, header + 4);
        u32 start = word(elf, header + 8), size = word(elf, header + 16);
        bool holds = address >= start && u64{address} + bytes <= u64{start} + size;

        // A loadable segment that holds the bytes, and whose bytes are in the file.
        if (type == 1 && holds && u64{offset} + size <= elf.size()) {
            return offset + (address - start);
        }
    }

    return 0;
}

}  // namespace

std::vector<VuChunk> vu_program_chunks(const std::vector<u8>& elf) {
    std::vector<VuChunk> chunks;
    std::vector<Section> all = sections(elf);
    const Section* table = nullptr;
    const Section* names = nullptr;
    std::map<std::string, u32> sizes;

    for (const Section& section : all) {
        // The table of chunks.
        if (section.name == ".DVP.ovlytab") {
            table = &section;
        }

        // The names its records point into.
        if (section.name == ".DVP.ovlystrtab") {
            names = &section;
        }

        // A chunk's own section, which has nothing but the chunk's size.
        if (section.name.rfind(kChunkPrefix, 0) == 0) {
            sizes[section.name] = section.size;
        }
    }

    // No table: the program has no vector unit programs named this way.
    if (!table || !names || u64{table->offset} + table->size > elf.size()) {
        return chunks;
    }

    for (u32 at = 0; at + kRecordBytes <= table->size; at += kRecordBytes) {
        std::size_t record = table->offset + at;
        std::string name = text(elf, u64{names->offset} + word(elf, record));
        u32 address = word(elf, record + 4);
        auto size = sizes.find(name);

        // A record whose chunk has no section of its own: its size is not known.
        if (size == sizes.end() || name.rfind(kChunkPrefix, 0) != 0) {
            continue;
        }

        VuChunk chunk;
        unsigned long line = 0;
        int fields = std::sscanf(
            name.c_str() + sizeof(kChunkPrefix) - 1,
            "%*[^.].%u.%lu.%u",
            &chunk.program,
            &line,
            &chunk.chunk
        );
        u32 offset = file_offset(elf, address, size->second);

        // The name is not of the form address.program.line.chunk, or the bytes are not in the file.
        if (fields != 3 || offset == 0) {
            continue;
        }

        chunk.address = word(elf, record + 8);
        chunk.code.assign(elf.begin() + offset, elf.begin() + offset + size->second);
        chunks.push_back(std::move(chunk));
    }

    return chunks;
}

std::vector<VuProgramUse> vu_program_use(
    const std::vector<VuChunk>& chunks, const std::vector<Vu::Use>& uses
) {
    std::map<u32, VuProgramUse> by_program;

    for (const Vu::Use& use : uses) {
        for (std::size_t slot = 0; slot < use.pairs.size(); slot++) {
            // Nothing ran or started in this slot.
            if (use.pairs[slot] == 0 && use.starts[slot] == 0) {
                continue;
            }

            u32 program = 0;

            for (const VuChunk& chunk : chunks) {
                bool here = chunk.address == slot * kSlotBytes && !chunk.code.empty()
                            && chunk.address + chunk.code.size() <= use.micro.size();

                // This chunk's bytes are what the slot held.
                if (here
                    && std::memcmp(&use.micro[chunk.address], chunk.code.data(), chunk.code.size())
                           == 0) {
                    program = chunk.program;
                    break;
                }
            }

            VuProgramUse& sum = by_program[program];

            sum.program = program;
            sum.pairs += use.pairs[slot];
            sum.starts += use.starts[slot];
        }
    }

    std::vector<VuProgramUse> out;

    for (const auto& [program, sum] : by_program) {
        out.push_back(sum);
    }

    std::sort(out.begin(), out.end(), [](const VuProgramUse& a, const VuProgramUse& b) {
        return a.pairs > b.pairs;
    });

    return out;
}

}  // namespace ps2
