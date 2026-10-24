// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-formats/src/iso9660.rs:
// ISC License, Copyright (c) 2026 ReRAC contributors.
//
// The ISO 9660 primary volume descriptor, the directory walk and sector
// reads through an ImageSource.

#include "assets/disc/iso9660.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <fstream>

namespace openrac::assets::disc {

namespace {

constexpr u32 kPvdSector = 16;
constexpr int kMaxDepth = 8;
constexpr u32 kMaxDirectoryBytes = 16u << 20;
constexpr u8 kSync[12] = {0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00};

class FileSource final : public ImageSource {
public:
    explicit FileSource(const std::filesystem::path& path)
        : m_path(path.string()),
          m_file(path, std::ios::binary) {
        // A file that cannot be opened is reported with its path.
        if (!m_file) {
            throw IoError(std::format("cannot open {}", m_path));
        }

        m_file.seekg(0, std::ios::end);
        const std::streamoff end = m_file.tellg();

        // A failed seek or a negative size means the size cannot be known.
        if (!m_file || end < 0) {
            throw IoError(std::format("cannot read the size of {}", m_path));
        }

        m_size = static_cast<u64>(end);
    }

    u64 size() const override { return m_size; }

    void read(u64 offset, std::span<u8> dst) override {
        // An empty read needs no seek and cannot fail.
        if (dst.empty()) {
            return;
        }

        m_file.clear();
        m_file.seekg(static_cast<std::streamoff>(offset));
        m_file.read(reinterpret_cast<char*>(dst.data()), static_cast<std::streamsize>(dst.size()));

        // A short read means the file ends before the range asked for.
        if (!m_file || static_cast<std::size_t>(m_file.gcount()) != dst.size()) {
            throw IoError(
                std::format("cannot read {:#x} bytes at {:#x} of {}", dst.size(), offset, m_path)
            );
        }
    }

private:
    std::string m_path;
    std::ifstream m_file;
    u64 m_size = 0;
};

class MemorySource final : public ImageSource {
public:
    explicit MemorySource(std::vector<u8> bytes) : m_bytes(std::move(bytes)) {}

    u64 size() const override { return m_bytes.size(); }

    void read(u64 offset, std::span<u8> dst) override {
        // The range is checked as size > length - offset so the sum cannot overflow.
        if (offset > m_bytes.size() || dst.size() > m_bytes.size() - offset) {
            throw IoError(
                std::format("read of {:#x} bytes at {:#x} past the end", dst.size(), offset)
            );
        }

        std::memcpy(dst.data(), m_bytes.data() + offset, dst.size());
    }

private:
    std::vector<u8> m_bytes;
};

std::string upper(std::string_view s) {
    std::string out(s);

    // ISO 9660 names are compared in upper case.
    for (char& c : out) {
        c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    }

    return out;
}

}  // namespace

std::unique_ptr<ImageSource> open_image_file(const std::filesystem::path& path) {
    return std::make_unique<FileSource>(path);
}

std::unique_ptr<ImageSource> image_in_memory(std::vector<u8> bytes) {
    return std::make_unique<MemorySource>(std::move(bytes));
}

IsoImage IsoImage::open(const std::filesystem::path& path) {
    return IsoImage(open_image_file(path));
}

IsoImage::IsoImage(std::unique_ptr<ImageSource> source) : m_source(std::move(source)) {
    const u64 file_size = m_source->size();
    std::array<u8, 16> head{};

    // An image too short for a sector header cannot be raw.
    if (file_size >= head.size()) {
        m_source->read(0, head);

        // A raw CD image starts every sector with the 12-byte sync pattern, followed by the header.
        if (std::memcmp(head.data(), kSync, sizeof kSync) == 0) {
            m_raw_sector_size = kRawSectorSize;
            m_raw_user_offset = head[15] == 2 ? 24 : 16;  // mode 2 form 1, or mode 1
        }
    }

    const u64 sectors = file_size / m_raw_sector_size;

    // The sector number must fit in 32 bits.
    if (sectors > 0xffffffffull) {
        fail("ISO 9660: the image is too large");
    }

    m_sector_count = static_cast<u32>(sectors);

    // The volume descriptor sits at sector 16, so the image must reach past it.
    if (m_sector_count <= kPvdSector) {
        fail("ISO 9660: the image is too small to hold a volume descriptor");
    }

    const std::vector<u8> pvd = read_sectors(kPvdSector, 1);
    const ByteView b(pvd);

    // The primary volume descriptor has type 1 and the identifier "CD001".
    if (b.u8_at(0) != 1 || std::memcmp(pvd.data() + 1, "CD001", 5) != 0) {
        fail("no ISO 9660 primary volume descriptor at sector 16");
    }

    m_volume_id = b.string_at(40, 32);

    // The volume id is padded with blanks, which are not part of the name.
    while (!m_volume_id.empty() && m_volume_id.back() == ' ') {
        m_volume_id.pop_back();
    }

    m_volume_sectors = b.u32_at(80);

    // The reader handles only 2048-byte logical blocks, as every PlayStation 2 disc has.
    if (b.u16_at(128) != kSectorSize) {
        fail("ISO 9660: unexpected logical block size {}", b.u16_at(128));
    }

    // The root directory record is embedded at PVD+156.
    walk_directory(b.u32_at(156 + 2), b.u32_at(156 + 10), "", 0);
}

void IsoImage::walk_directory(u32 lba, u32 size, const std::string& prefix, int depth) {
    // A deeper directory tree than this is a loop in a damaged image.
    if (depth > kMaxDepth) {
        fail("ISO 9660: directory nesting too deep");
    }

    // A directory of more than this many bytes is not real.
    if (size > kMaxDirectoryBytes) {
        fail("ISO 9660: implausible directory size {:#x}", size);
    }

    const u32 count = static_cast<u32>((size + kSectorSize - 1) / kSectorSize);
    const std::vector<u8> dir = read_sectors(lba, count);
    const ByteView b(dir);
    std::size_t pos = 0;

    // Read records until the directory's size is used up.
    while (pos < size) {
        const std::size_t length = b.u8_at(pos);

        // A zero length marks the padding at the end of a sector.
        if (length == 0) {
            // Records never straddle sectors: zero means "go on at the next sector".
            pos = (pos / kSectorSize + 1) * kSectorSize;
            continue;
        }

        const u32 entry_lba = b.u32_at(pos + 2);
        const u32 entry_size = b.u32_at(pos + 10);
        const u8 flags = b.u8_at(pos + 25);
        const std::size_t name_length = b.u8_at(pos + 32);
        const ByteView raw = b.sub(pos + 33, name_length, "directory record name");
        pos += length;

        // "." and ".." are the single bytes 0 and 1.
        if (raw.empty() || raw.u8_at(0) == 0 || raw.u8_at(0) == 1) {
            continue;
        }

        std::string name = upper({reinterpret_cast<const char*>(raw.data()), raw.size()});

        // Names carry a ";1" version suffix, which is dropped.
        if (const auto semi = name.find(';'); semi != std::string::npos) {
            name.resize(semi);
        }

        IsoEntry entry;
        entry.path = prefix + "/" + name;
        entry.name = std::move(name);
        entry.lba = entry_lba;
        entry.size = entry_size;
        entry.is_directory = (flags & 2) != 0;
        m_entries.push_back(entry);

        // Descend into sub-directories so their files are listed too.
        if (entry.is_directory) {
            walk_directory(entry.lba, entry.size, entry.path, depth + 1);
        }
    }
}

const IsoEntry* IsoImage::find(std::string_view path) const {
    std::string want = upper(path);

    // Drop a version suffix from the path asked for.
    if (const auto semi = want.rfind(';'); semi != std::string::npos) {
        want.resize(semi);
    }

    // Entry paths are absolute, so make the lookup path absolute too.
    if (want.empty() || want.front() != '/') {
        want.insert(want.begin(), '/');
    }

    // Search all entries; the list is short.
    for (const IsoEntry& e : m_entries) {
        // Paths are stored in upper case with no version suffix, as the wanted path now is.
        if (e.path == want) {
            return &e;
        }
    }

    return nullptr;
}

std::vector<u8> IsoImage::read_file(const IsoEntry& entry) const {
    return read_bytes(u64{entry.lba} * kSectorSize, entry.size);
}

std::vector<u8> IsoImage::read_sectors(u32 lba, u32 count) const {
    return read_bytes(u64{lba} * kSectorSize, u64{count} * kSectorSize);
}

std::vector<u8> IsoImage::read_bytes(u64 offset, u64 size) const {
    // A read of a terabyte is not a real request; stop before allocating.
    if (size > (u64{1} << 40)) {
        fail("ISO 9660: implausible read of {:#x} bytes", size);
    }

    std::vector<u8> out(static_cast<std::size_t>(size));
    read_into(offset, out);
    return out;
}

void IsoImage::read_into(u64 offset, std::span<u8> dst) const {
    const u64 end = u64{m_sector_count} * kSectorSize;

    // The range is checked as size > end - offset so the sum cannot overflow.
    if (offset > end || dst.size() > end - offset) {
        fail(
            "byte range {:#x}+{:#x} lies beyond the end of the image ({} sectors)",
            offset,
            dst.size(),
            m_sector_count
        );
    }

    std::lock_guard guard(*m_lock);

    // A cooked image has no sector headers, so the source can be read directly.
    if (m_raw_sector_size == kSectorSize) {
        m_source->read(offset, dst);
        return;
    }

    std::size_t done = 0;

    // A raw image skips the sync and header bytes of every sector, so read it sector by sector.
    while (done < dst.size()) {
        const u64 pos = offset + done;
        const u64 sector = pos / kSectorSize;
        const std::size_t within = static_cast<std::size_t>(pos % kSectorSize);
        const std::size_t n = std::min(kSectorSize - within, dst.size() - done);
        m_source
            ->read(sector * m_raw_sector_size + m_raw_user_offset + within, dst.subspan(done, n));
        done += n;
    }
}

}  // namespace openrac::assets::disc
