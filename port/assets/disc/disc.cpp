// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-formats/src/disc.rs:
// ISC License, Copyright (c) 2026 ReRAC contributors.
//
// Reading the table of contents and planning the lumps.

#include "assets/disc/disc.h"

#include <algorithm>
#include <cstring>
#include <map>

#include "assets/disc/boot.h"

namespace openrac::assets::disc {

namespace {

constexpr u64 kSector = kSectorSize;

// The data container's members in plan order: overlay, sound bank, core
// index, GS RAM, HUD header, HUD banks 0..4, core data.
std::vector<std::pair<std::string, ByteRange>> data_members(const LevelDataHeader& dh) {
    std::vector<std::pair<std::string, ByteRange>> m = {
        {"overlay", dh.overlay},
        {"sound_bank", dh.sound_bank},
        {"core_index", dh.core_index},
        {"gs_ram", dh.gs_ram},
        {"hud_header", dh.hud_header},
    };

    // Name each HUD bank by its index in the list.
    for (std::size_t i = 0; i < dh.hud_banks.size(); ++i) {
        m.emplace_back(std::format("hud_bank_{}", i), dh.hud_banks[i]);
    }

    m.emplace_back("core_data", dh.core_data);
    return m;
}

}  // namespace

std::vector<std::pair<std::string, ByteView>> LevelFiles::files() const {
    std::vector<std::pair<std::string, ByteView>> out;
    out.emplace_back("level_header.bin", ByteView(level_header));
    auto add = [&](std::string name, const std::optional<std::vector<u8>>& m) {
        // Optional lumps are listed only when the level has them.
        if (m) {
            out.emplace_back(std::move(name), ByteView(*m));
        }
    };
    add("overlay.bin", overlay);
    add("sound_bank.bin", sound_bank);
    add("core_index.bin", core_index);
    add("gs_ram.bin", gs_ram);
    add("hud_header.bin", hud_header);

    // Add each HUD bank that the level has.
    for (std::size_t i = 0; i < hud_banks.size(); ++i) {
        add(std::format("hud_bank_{}.bin", i), hud_banks[i]);
    }

    add("core_data.bin", core_data);
    add("gameplay_ntsc.bin", gameplay_ntsc);
    add("gameplay_pal.bin", gameplay_pal);
    add("occlusion.bin", occlusion);
    return out;
}

std::optional<ByteView> LevelFiles::file(std::string_view name) const {
    // Search the level's files for the name asked for.
    for (const auto& [n, b] : files()) {
        // File names are unique, so the first match is the file.
        if (n == name) {
            return b;
        }
    }

    return std::nullopt;
}

Disc Disc::open(const std::filesystem::path& image, const DiscLayout& layout) {
    return Disc(IsoImage::open(image), layout);
}

Disc::Disc(IsoImage iso, const DiscLayout& layout) : m_iso(std::move(iso)), m_layout(&layout) {
    // An unknown layout has no table offsets to read from.
    if (!layout.known) {
        fail("{}", layout.source);
    }

    const std::vector<u8> head = m_iso.read_sectors(layout.toc_sector, 1);
    const ByteView hb(head);

    // The table of contents starts with a version number, which must be 1.
    if (hb.s32_at(0) != 1) {
        fail(
            "table of contents: version {} is not 1; not a {} disc?",
            hb.s32_at(0),
            game_name(layout.game)
        );
    }

    const s32 size = hb.s32_at(4);

    // The header size must cover its 8-byte preamble and stay within a plausible bound.
    if (size <= 8 || size > 0x200000) {
        fail("table of contents: implausible header size {:#x}", size);
    }

    m_toc = m_iso.read_bytes(u64{layout.toc_sector} * kSector, static_cast<u64>(size));
    const auto sectors = level_header_sectors(m_toc, layout);

    // Read the header of each level the table lists.
    for (std::size_t i = 0; i < sectors.size(); ++i) {
        // An empty slot has no level.
        if (!sectors[i]) {
            continue;
        }

        const u32 sector = *sectors[i];

        // A level whose header would lie past the end of the image is not on this disc.
        if (u64{sector} + 5 > m_iso.sector_count()) {
            continue;
        }

        const std::vector<u8> bytes =
            m_iso.read_bytes(u64{sector} * kSector, layout.level_header_size);
        const auto header = ByteView(bytes).read<LevelHeader>(0, "level header");

        // Slots without the header's signature are skipped, as ReRAC does.
        if (header.header_size != static_cast<s32>(layout.level_header_size)) {
            continue;
        }

        m_levels.push_back({i, sector, header});
    }
}

std::vector<u32> Disc::level_ids() const {
    std::vector<u32> ids;

    // Collect the level ids in table order.
    for (const TocLevel& l : m_levels) {
        ids.push_back(static_cast<u32>(l.header.id));
    }

    return ids;
}

std::string Disc::boot_executable_path() const {
    const IsoEntry* cnf = m_iso.find("/SYSTEM.CNF");

    // Every PlayStation 2 disc names its boot program in SYSTEM.CNF.
    if (!cnf) {
        fail("SYSTEM.CNF missing");
    }

    const std::vector<u8> bytes = m_iso.read_file(*cnf);
    const auto parsed =
        SystemCnf::parse({reinterpret_cast<const char*>(bytes.data()), bytes.size()});

    // A SYSTEM.CNF without a BOOT2 line does not name a boot program.
    if (!parsed) {
        fail("SYSTEM.CNF has no BOOT2 line");
    }

    return parsed->boot;
}

std::vector<u8> Disc::boot_executable() const {
    // Look for the boot program by the path SYSTEM.CNF gives.
    try {
        // A missing path falls through to the search by ELF magic below.
        if (const IsoEntry* e = m_iso.find(boot_executable_path())) {
            return m_iso.read_file(*e);
        }
    } catch (const AssetError&) {
        // fall back to the first ELF in the root
    }

    // Fall back to the files in the root directory, in case SYSTEM.CNF's path does not resolve.
    for (const IsoEntry& e : m_iso.entries()) {
        // Skip directories, files too small to hold a header, and files below the root.
        if (e.is_directory || e.size <= 4 || std::count(e.path.begin(), e.path.end(), '/') != 1) {
            continue;
        }

        const std::vector<u8> magic = m_iso.read_bytes(u64{e.lba} * kSector, 4);

        // A boot program is an ELF file, which starts with the bytes 0x7f "ELF".
        if (std::memcmp(
                magic.data(),
                "\x7f"
                "ELF",
                4
            )
            == 0) {
            return m_iso.read_file(e);
        }
    }

    fail("no boot executable in the file system");
}

const TocLevel& Disc::toc_level(u32 id) const {
    // Look the level up by its id in the table.
    for (const TocLevel& l : m_levels) {
        // Level ids are unique, so the first match is the level.
        if (static_cast<u32>(l.header.id) == id) {
            return l;
        }
    }

    fail("no level {} in the table of contents", id);
}

std::optional<std::vector<u8>> Disc::sector_range(SectorRange r) const {
    // An empty range has no sectors to read.
    if (r.empty()) {
        return std::nullopt;
    }

    // Negative values would wrap when cast to unsigned; the table is damaged.
    if (r.offset < 0 || r.size < 0) {
        fail("negative sector range {}+{}", r.offset, r.size);
    }

    return m_iso
        .read_bytes(static_cast<u64>(r.offset) * kSector, static_cast<u64>(r.size) * kSector);
}

u64 Disc::probe(u32 sector) const {
    return probe_lump_size(m_iso.read_sectors(sector, 1)).bytes;
}

LevelFiles Disc::level(u32 id) const {
    const TocLevel& lv = toc_level(id);
    const LevelHeader& h = lv.header;
    LevelFiles f;
    f.id = id;
    f.header = h;
    f.level_header = m_iso.read_bytes(u64{lv.header_sector} * kSector, m_layout->level_header_size);

    // A level without a data range has nothing to read.
    if (h.data.offset <= 0 || h.data.size <= 0) {
        fail("level {}: empty data range", id);
    }

    // The data container: uncompressed, byte-offset lumps, read in one go.
    const std::vector<u8> data = m_iso.read_bytes(
        static_cast<u64>(h.data.offset) * kSector, static_cast<u64>(h.data.size) * kSector
    );
    const LevelDataHeader dh = parse_level_data_header(data);
    auto lump = [&](ByteRange r, std::string_view what) -> std::optional<std::vector<u8>> {
        // A lump that is not present has no bytes.
        if (!r.present()) {
            return std::nullopt;
        }

        return ByteView(data)
            .sub(static_cast<std::size_t>(r.offset), static_cast<std::size_t>(r.size), what)
            .to_vector();
    };
    f.overlay = lump(dh.overlay, "overlay");
    f.sound_bank = lump(dh.sound_bank, "sound bank");
    f.core_index = lump(dh.core_index, "core index");
    f.gs_ram = lump(dh.gs_ram, "gs ram");
    f.hud_header = lump(dh.hud_header, "hud header");

    // Read each HUD bank the data header names.
    for (std::size_t i = 0; i < f.hud_banks.size(); ++i) {
        f.hud_banks[i] = lump(dh.hud_banks[i], "hud bank");
    }

    f.core_data = lump(dh.core_data, "core data");
    f.gameplay_ntsc = sector_range(h.gameplay_ntsc);
    f.gameplay_pal = sector_range(h.gameplay_pal);
    f.occlusion = sector_range(h.occlusion);
    return f;
}

std::vector<DiscFile> Disc::level_group_files(u32 id) const {
    const TocLevel& lv = toc_level(id);
    const LevelHeader& h = lv.header;
    const std::string dir = std::format("levels/{:02}", id);
    std::vector<DiscFile> out;
    out.push_back(
        {dir + "/level_header.bin", u64{lv.header_sector} * kSector, m_layout->level_header_size}
    );

    // A level without a data range has nothing to read.
    if (h.data.offset <= 0 || h.data.size <= 0) {
        fail("level {}: empty data range", id);
    }

    const u64 data_at = static_cast<u64>(h.data.offset) * kSector;
    const u64 data_length = static_cast<u64>(h.data.size) * kSector;
    const std::vector<u8> first = m_iso.read_bytes(data_at, std::min(kSector, data_length));
    const LevelDataHeader dh = parse_level_data_header(first);

    // Check every range the data header names.
    for (const auto& [name, r] : data_members(dh)) {
        // Ranges that are not present are skipped.
        if (!r.present()) {
            continue;
        }

        // A range must stay inside the data container.
        if (static_cast<u64>(r.offset) + static_cast<u64>(r.size) > data_length) {
            fail("level {}: {} runs past the data container", id, name);
        }

        out.push_back(
            {std::format("{}/{}.bin", dir, name),
             data_at + static_cast<u64>(r.offset),
             static_cast<u64>(r.size)}
        );
    }

    const std::pair<std::string_view, SectorRange> ranges[] = {
        {"gameplay_ntsc", h.gameplay_ntsc},
        {"gameplay_pal", h.gameplay_pal},
        {"occlusion", h.occlusion},
    };

    // Check every range the level names.
    for (const auto& [name, r] : ranges) {
        // Empty ranges are skipped.
        if (r.empty()) {
            continue;
        }

        // Negative values would wrap when cast to unsigned.
        if (r.offset < 0 || r.size < 0) {
            fail("level {}: negative {} range", id, name);
        }

        out.push_back(
            {std::format("{}/{}.bin", dir, name),
             static_cast<u64>(r.offset) * kSector,
             static_cast<u64>(r.size) * kSector}
        );
    }

    return out;
}

std::vector<StreamLump> Disc::global_lumps() const {
    const ByteView b(m_toc);
    std::vector<StreamLump> out;
    std::map<std::string, u32> seen;

    // Walk the global fields the layout names.
    for (const GlobalField& f : m_layout->global_fields) {
        // A field can hold several entries, one per index.
        for (u32 i = 0; i < f.count; ++i) {
            const std::string base =
                f.count == 1 ? std::string(f.name) : std::format("{}/{:03}", f.name, i);
            u32 sector = 0;
            u64 bytes = 0;

            // Sector fields hold only a start sector; range fields hold a start and a size.
            if (f.kind == FieldKind::Sector) {
                const s32 off = b.s32_at(f.offset + i * 4);

                // A zero sector means the entry is absent.
                if (off == 0) {
                    continue;
                }

                // A negative sector is a damaged table.
                if (off < 0) {
                    fail("table of contents: negative sector for global {}", base);
                }

                sector = static_cast<u32>(off);
                bytes = probe(sector);
            } else {
                const s32 off = b.s32_at(f.offset + i * 8);
                const s32 size = b.s32_at(f.offset + i * 8 + 4);

                // A zero start and size means the entry is absent.
                if (off == 0 && size == 0) {
                    continue;
                }

                // Negative values would wrap when cast to unsigned.
                if (off < 0 || size < 0) {
                    fail("table of contents: negative range for global {}", base);
                }

                sector = static_cast<u32>(off);
                bytes = f.kind == FieldKind::SectorRange ? static_cast<u64>(size) * kSector
                                                         : static_cast<u64>(size);
            }

            const u32 n = ++seen[base];
            out.push_back({n > 1 ? std::format("{}.{}", base, n) : base, sector, bytes});
        }
    }

    return out;
}

std::vector<StreamLump> Disc::level_stream_lumps(u32 id) const {
    return disc::level_stream_lumps(toc_level(id).header, [&](u32 sector) {
        return probe(sector);
    });
}

std::vector<u8> Disc::read_lump(const StreamLump& lump) const {
    return m_iso.read_bytes(u64{lump.sector} * kSector, lump.bytes);
}

std::vector<DiscFile> Disc::archive_files() const {
    std::vector<DiscFile> out;
    std::vector<const IsoEntry*> entries;

    // List the files the ISO names, leaving out directories.
    for (const IsoEntry& e : m_iso.entries()) {
        // Directories have no bytes of their own.
        if (!e.is_directory) {
            entries.push_back(&e);
        }
    }

    std::sort(entries.begin(), entries.end(), [](const IsoEntry* a, const IsoEntry* b) {
        return a->path < b->path;
    });

    // Each file the ISO names becomes a lump under boot/.
    for (const IsoEntry* e : entries) {
        out.push_back({"boot" + e->path, u64{e->lba} * kSector, e->size});
    }

    out.push_back({"toc.bin", u64{m_layout->toc_sector} * kSector, m_toc.size()});

    // Then the global lumps from the table of contents.
    for (const StreamLump& l : global_lumps()) {
        out.push_back({std::format("global/{}.bin", l.name), u64{l.sector} * kSector, l.bytes});
    }

    // Then each level's files and streams.
    for (const TocLevel& lv : m_levels) {
        const u32 id = static_cast<u32>(lv.header.id);

        // The level's data container is split into named files.
        for (DiscFile& f : level_group_files(id)) {
            out.push_back(std::move(f));
        }

        // The level's streamed lumps follow the data files.
        for (const StreamLump& l : level_stream_lumps(id)) {
            out.push_back(
                {std::format("levels/{:02}/{}.bin", id, l.name), u64{l.sector} * kSector, l.bytes}
            );
        }
    }

    const u64 end = u64{m_iso.sector_count()} * kSector;

    // Check every lump against the image's size.
    for (const DiscFile& f : out) {
        // A lump must lie wholly inside the image.
        if (f.offset > end || f.bytes > end - f.offset) {
            fail("{} ({:#x}+{:#x}) lies beyond the end of the image", f.path, f.offset, f.bytes);
        }
    }

    return out;
}

std::vector<u8> Disc::save_game_lump() const {
    auto r = sector_range(global_sector_range(m_toc, kSaveGameField));

    // The save game range must not be empty.
    if (!r) {
        fail("table of contents: empty save_game range");
    }

    return std::move(*r);
}

std::optional<std::vector<u8>> Disc::scene_region(u32 id, std::size_t k, bool pal) const {
    const LevelHeader& h = toc_level(id).header;

    // A scene index past the table has no record.
    if (k >= h.scenes.size()) {
        fail("no scene record {}", k);
    }

    const auto run = h.scenes[k].region_sectors(pal);

    // A scene with no chunks in the region has no run.
    if (!run) {
        return std::nullopt;
    }

    return m_iso.read_bytes(u64{run->first} * kSector, u64{run->second} * kSector);
}

std::optional<std::vector<u8>> Disc::scene_speech(u32 id, std::size_t k, std::size_t lang) const {
    const LevelHeader& h = toc_level(id).header;

    // A scene or language past the table has no speech.
    if (k >= h.scenes.size() || lang >= h.scenes[k].speech.size()) {
        fail("no scene record {} / language {}", k, lang);
    }

    const s32 s = h.scenes[k].speech[lang];

    // A zero sector means the scene has no speech in this language.
    if (s <= 0) {
        return std::nullopt;
    }

    const u32 sector = static_cast<u32>(s);
    return m_iso.read_bytes(u64{sector} * kSector, probe(sector));
}

}  // namespace openrac::assets::disc
