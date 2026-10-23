// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-formats/src/save_game.rs:
// ISC License, Copyright (c) 2026 ReRAC contributors.
//
// The save file's sections, CRC and tables.

#include "assets/disc/save_game.h"

#include <algorithm>
#include <cstring>

#include "assets/disc/boot.h"

namespace openrac::assets::disc {

namespace {

// NTSC-U boot executable addresses (overlay copies: level 01 0x184a40,
// 0x184d40, 0x1c4318, 0x1c4530).
constexpr SaveLayout kRac1Ntsc = {0x001a04c0, 0x001a07c0, 0x001dfd98, 0x001dffb0};

std::size_t align4(std::size_t n) {
    return (n + 3) & ~std::size_t{3};
}

}  // namespace

const SaveLayout* save_layout(const GameVersion& version) {
    // Only the NTSC-U layout is known so far.
    if (version.id == "rac1-ntsc") {
        return &kRac1Ntsc;
    }

    return nullptr;
}

std::vector<ChunkDesc> read_chunk_table(ByteView elf, u32 vaddr) {
    std::vector<ChunkDesc> out;

    // A chunk table holds at most 256 descriptors.
    for (u32 i = 0; i < 256; ++i) {
        const ByteView e = elf_read(elf, vaddr + i * kDescriptorSize, kDescriptorSize);
        const ChunkDesc d{e.u32_at(0), e.u32_at(4), e.s32_at(8)};

        // A zero address ends the table.
        if (d.address == 0) {
            return out;
        }

        // The terminator id cannot be used by a real chunk.
        if (d.id == kChunkEndId) {
            fail("chunk table {:#x}: entry {} uses the terminator id", vaddr, i);
        }

        out.push_back(d);
    }

    fail("chunk table {:#x}: no terminator in 256 entries", vaddr);
}

std::size_t section_size(std::span<const ChunkDesc> descs) {
    std::size_t n = 8 + 8;

    // Each chunk takes its size, padded to 4 bytes, plus an 8-byte header.
    for (const ChunkDesc& d : descs) {
        n += align4(d.size) + 8;
    }

    return n;
}

ChunkTables ChunkTables::from_boot_executable(ByteView elf, const SaveLayout& layout) {
    return {
        read_chunk_table(elf, layout.global_chunks), read_chunk_table(elf, layout.level_chunks)
    };
}

u16 save_crc16(std::span<const u8> data) {
    // A buffer longer than the checksum covers is not checksummed.
    if (data.size() > kCrcMaxLength) {
        return 0;
    }

    u32 r = 0xedb88320;

    // Feed the bytes one at a time into the checksum.
    for (const u8 b : data) {
        r ^= u32{b} << 8;

        // Eight shifts per byte, each one a step of the CRC.
        for (int i = 0; i < 8; ++i) {
            r = (r & 0x8000) != 0 ? (r << 1) ^ 0x1f45 : r << 1;
        }
    }

    return static_cast<u16>(r & 0xffff);
}

Chunk Chunk::make(s32 id, std::vector<u8> data) {
    std::vector<u8> pad(align4(data.size()) - data.size(), 0);
    return {id, std::move(data), std::move(pad)};
}

const Chunk* Section::chunk(s32 id) const {
    // Search the chunks for the one with this id.
    for (const Chunk& c : chunks) {
        // Chunk ids are unique, so the first match is the chunk.
        if (c.id == id) {
            return &c;
        }
    }

    return nullptr;
}

std::size_t Section::encoded_size() const {
    std::size_t n = 8 + 8;

    // Each chunk takes its size, padded to 4 bytes, plus an 8-byte header.
    for (const Chunk& c : chunks) {
        n += 8 + align4(c.data.size());
    }

    return n;
}

std::vector<u8> Section::encode() const {
    ByteWriter w;
    w.put<u32>(0);
    w.put<u32>(0);

    // Write each chunk as its id, size, data and padding.
    for (const Chunk& c : chunks) {
        w.put<s32>(c.id);
        w.put<u32>(static_cast<u32>(c.data.size()));
        w.put_bytes(c.data);
        const std::size_t n = align4(c.data.size()) - c.data.size();

        // Pad each chunk to 4 bytes, keeping the padding bytes the file had.
        for (std::size_t k = 0; k < n; ++k) {
            w.put<u8>(k < c.pad.size() ? c.pad[k] : 0);
        }
    }

    w.put<s32>(kChunkEndId);
    w.put<u32>(0);
    std::vector<u8>& v = w.bytes();
    const u32 size = static_cast<u32>(v.size() - 8);
    const u32 crc = save_crc16(std::span<const u8>(v).subspan(8));
    w.put_at<u32>(0, size);
    w.put_at<u32>(4, crc);
    return std::move(v);
}

Section Section::decode(ByteView bytes) {
    const u32 data_size = bytes.u32_at(0);
    const u32 stored = bytes.u32_at(4);
    const ByteView data = bytes.sub(8, data_size, "section data");
    Section s;
    s.crc_ok = stored != 0 && save_crc16(data.span()) == stored;
    std::size_t p = 0;

    // Read chunks until the end id; the loop exits from inside.
    while (true) {
        const s32 id = data.s32_at(p);
        const u32 size = data.u32_at(p + 4);

        // The end id closes the section.
        if (id == kChunkEndId) {
            break;
        }

        const ByteView body = data.sub(p + 8, align4(size), "chunk data");
        s.chunks.push_back({id, body.sub(0, size).to_vector(), body.tail(size).to_vector()});
        p += 8 + align4(size);
    }

    return s;
}

SaveFile SaveFile::parse(ByteView bytes) {
    const u32 gs = bytes.u32_at(0);
    const u32 ls = bytes.u32_at(4);

    // A section smaller than its own header is damaged.
    if (gs < 16 || ls < 16) {
        fail("save file: implausible section sizes {:#x}/{:#x}", gs, ls);
    }

    bytes.check(0, 8 + std::size_t{gs} + kSaveLevelSlots * ls, "save file");
    SaveFile f;
    f.global = Section::decode(bytes.sub(8, gs));

    // The global section is followed by one section per level slot.
    for (std::size_t l = 0; l < kSaveLevelSlots; ++l) {
        f.levels[l] = Section::decode(bytes.sub(8 + gs + l * ls, ls));
    }

    return f;
}

std::vector<u8> SaveFile::to_bytes() const {
    const std::vector<u8> g = global.encode();
    std::vector<std::vector<u8>> lv;

    // Encode every level section before the sizes are known.
    for (const Section& s : levels) {
        lv.push_back(s.encode());
    }

    ByteWriter w;
    w.put<u32>(static_cast<u32>(g.size()));
    w.put<u32>(static_cast<u32>(lv[0].size()));
    w.put_bytes(g);

    // Append the level sections in slot order.
    for (const auto& l : lv) {
        w.put_bytes(l);
    }

    return std::move(w.bytes());
}

bool SaveFile::all_crcs_ok() const {
    return global.crc_ok
           && std::all_of(levels.begin(), levels.end(), [](const Section& s) { return s.crc_ok; });
}

void SaveFile::write_incremental(std::size_t level, std::span<u8> card_file) const {
    // A level slot past the table does not exist.
    if (level >= kSaveLevelSlots) {
        fail("level slot {} out of range", level);
    }

    const std::vector<u8> g = global.encode();
    const std::vector<u8> l = levels[level].encode();
    const std::size_t at = 8 + g.size() + level * l.size();

    // The card file must already hold the level slot being rewritten.
    if (card_file.size() < at + l.size()) {
        fail("incremental save: the card file is shorter than a whole save");
    }

    std::memcpy(card_file.data() + 8, g.data(), g.size());
    std::memcpy(card_file.data() + at, l.data(), l.size());
}

std::string save_file_name(std::size_t slot) {
    return std::format("save{}.bin", slot);
}

std::string card_directory(ByteView system_cnf) {
    system_cnf.check(0, 27, "SYSTEM.CNF");
    const u8* s = system_cnf.data();
    std::string n = "/BA****-*****RATCHET";
    n[2] = s[0x12] == 'E' ? 'E' : 'A';

    // Copy the four characters of the boot name's serial number.
    for (std::size_t i = 0; i < 4; ++i) {
        n[3 + i] = static_cast<char>(s[16 + i]);
    }

    // Copy the three characters of the next field.
    for (std::size_t i = 0; i < 3; ++i) {
        n[8 + i] = static_cast<char>(s[21 + i]);
    }

    // Copy the two characters of the last field.
    for (std::size_t i = 0; i < 2; ++i) {
        n[11 + i] = static_cast<char>(s[25 + i]);
    }

    // Check the whole name for characters outside ASCII.
    for (const char c : n) {
        // A byte with the top bit set is not ASCII, so the name is not what the file system
        // expects.
        if (static_cast<unsigned char>(c) >= 0x80) {
            fail("SYSTEM.CNF: non-ASCII boot name");
        }
    }

    return n;
}

SaveGameLump SaveGameLump::parse(ByteView lump) {
    auto part = [&](std::size_t i, std::string_view what) {
        return lump.sub(lump.u32_at(i * 8), lump.u32_at(i * 8 + 4), what).to_vector();
    };
    return {part(0, "icon.sys"), part(1, "static.ico"), part(2, "save template")};
}

bool ItemRecord::has_ammo() const {
    return (bytes[8] | bytes[9]) != 0;
}

u16 ItemRecord::grant_ammo() const {
    return static_cast<u16>(bytes[0x12] | bytes[0x13] << 8);
}

ItemTables ItemTables::load(ByteView boot_elf, ByteView overlay, const SaveLayout& layout) {
    ItemTables t{};
    const ByteView vt = elf_read(boot_elf, layout.vendor_items, kVendorTableLength * 4);

    // Read the vendor item table one word at a time.
    for (std::size_t i = 0; i < kVendorTableLength; ++i) {
        t.vendor_items[i] = vt.u32_at(4 * i);
    }

    const ByteView rec = elf_read(boot_elf, layout.item_records, kItemCount * 0x18);

    // Copy each item's fixed-size record.
    for (std::size_t i = 0; i < kItemCount; ++i) {
        std::memcpy(t.records[i].bytes.data(), rec.data() + i * 0x18, 0x18);
    }

    const std::vector<OverlaySection> sections = parse_overlay_sections(overlay);
    t.item_definitions = find_item_definitions(sections, vt);
    const auto defs = read_overlay(sections, t.item_definitions, kItemCount * kItemDefinitionSize);

    // The definitions must lie inside the overlay.
    if (!defs) {
        fail("item definitions {:#x} lie outside the overlay", t.item_definitions);
    }

    // Read each item's slot type from its definition.
    for (std::size_t i = 0; i < kItemCount; ++i) {
        t.slot_type[i] = defs->s32_at(i * kItemDefinitionSize + 8);
    }

    return t;
}

u32 find_item_definitions(std::span<const OverlaySection> sections, ByteView vendor_table) {
    std::vector<u32> hits;

    // Search the overlay's sections for the vendor table.
    for (const OverlaySection& s : sections) {
        // Skip section kind 8, and sections too small to hold the table.
        if (s.kind == 8 || s.data.size() < vendor_table.size() || vendor_table.empty()) {
            continue;
        }

        // Slide the table over the section, looking for a match.
        for (std::size_t i = 0; i + vendor_table.size() <= s.data.size(); ++i) {
            // A byte-for-byte match is a place where the table is.
            if (std::memcmp(s.data.data() + i, vendor_table.data(), vendor_table.size()) == 0) {
                hits.push_back(s.dest + static_cast<u32>(i));
            }
        }
    }

    // The table must be found exactly once, or its place is ambiguous.
    if (hits.size() != 1) {
        fail("the vendor item table is found {} times in the overlay", hits.size());
    }

    const u32 v = hits[0];
    const u32 hi = ((v + 0x8000) >> 16) & 0xffff;
    const u32 lo = v & 0xffff;
    auto op = [](u32 w) {
        return w >> 26;
    };
    auto rs = [](u32 w) {
        return (w >> 21) & 31;
    };
    auto rt = [](u32 w) {
        return (w >> 16) & 31;
    };
    auto imm = [](u32 w) {
        return w & 0xffff;
    };
    constexpr u32 kLui = 0x0f;
    constexpr u32 kAddiu = 0x09;
    constexpr u32 kLw = 0x23;
    std::vector<u32> found;

    // Search the overlay's code sections for the instructions that set up the definition table.
    for (const OverlaySection& s : sections) {
        // Skip section kind 8.
        if (s.kind == 8) {
            continue;
        }

        std::vector<u32> w(s.data.size() / 4);
        std::memcpy(w.data(), s.data.data(), w.size() * 4);
        const std::size_t n = w.size();

        // Look at every word for the first instruction of the pattern.
        for (std::size_t i = 0; i < n; ++i) {
            // The pattern begins with a lui of the high half of the address.
            if (op(w[i]) != kLui || imm(w[i]) != hi) {
                continue;
            }

            std::size_t j = i + 1;

            // Look for the addiu with the low half within the next few instructions.
            while (j < std::min(n, i + 5)
                   && !(op(w[j]) == kAddiu && rs(w[j]) == rt(w[i]) && imm(w[j]) == lo)) {
                ++j;
            }

            // Without the addiu the pattern is not there.
            if (j >= std::min(n, i + 5)) {
                continue;
            }

            std::size_t k = j + 1;

            // Look for the next lui within a few instructions.
            while (k < std::min(n, j + 9) && op(w[k]) != kLui) {
                ++k;
            }

            // Without the second lui the pattern is not there.
            if (k >= std::min(n, j + 9)) {
                continue;
            }

            const u32 rc = rt(w[k]);
            std::size_t m = k + 1;

            // Look for the addiu that finishes the second address.
            while (m < std::min(n, k + 49) && !(op(w[m]) == kAddiu && rs(w[m]) == rc)) {
                ++m;
            }

            // Without that addiu the pattern is not there.
            if (m >= std::min(n, k + 49)) {
                continue;
            }

            const u32 rd = rt(w[m]);

            // Look for the load of the item field just after the address is formed.
            for (std::size_t q = m + 1; q < std::min(n, m + 4); ++q) {
                // The load must use the register just built, at offset 8.
                if (op(w[q]) == kLw && rs(w[q]) == rd && imm(w[q]) == 8) {
                    const s32 low = static_cast<s16>(imm(w[m]));
                    found.push_back(static_cast<u32>(static_cast<s32>(imm(w[k]) << 16) + low));
                    break;
                }
            }
        }
    }

    found.erase(std::unique(found.begin(), found.end()), found.end());

    // With no match the code does not have the shape this reader expects.
    if (found.empty()) {
        fail("item definition table: GiveItem's pattern was not found");
    }

    // Several different addresses mean the match is ambiguous.
    if (found.size() > 1) {
        fail("item definition table: {} disagreeing candidates", found.size());
    }

    return found.front();
}

}  // namespace openrac::assets::disc
