// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

/**
 * The disc image: sector reads and the ISO 9660 path lookup declared in `disc.h`.
 *
 * Only the primary volume descriptor and the directory records it leads to are read. Names are
 * compared in upper case with any version suffix cut off.
 *
 * Sources: the ISO 9660 layout as publicly documented.
 */

#include "disc.h"

#include <algorithm>
#include <cctype>

namespace sys {

using namespace ps2;

namespace {

/**
 * Turns a file name into the form names are compared in: no version suffix, upper case.
 *
 * @param name A name as a path part or a directory record gives it, such as "Boot.Elf;1".
 * @return The name up to any ';' and in upper case.
 */
std::string canonical(std::string name) {
    // A record's name ends in ";1" (the file's version), which a path leaves out.
    std::size_t semicolon = name.find(';');
    if (semicolon != std::string::npos) {
        name.erase(semicolon);
    }

    // Names on the disc are compared without regard to case.
    std::transform(name.begin(), name.end(), name.begin(), [](unsigned char c) {
        return std::toupper(c);
    });
    return name;
}

}  // namespace

Disc::~Disc() {
    // Only an image that opened holds a host file.
    if (file_) {
        std::fclose(file_);
    }
}

bool Disc::open(const std::string& path) {
    // Opening a second image replaces the first, which is closed now.
    if (file_) {
        std::fclose(file_);
    }

    file_ = std::fopen(path.c_str(), "rb");

    // The host could not open the file.
    if (!file_) {
        return false;
    }

    // The size of the image is the offset of its end; a partial last sector does not count.
    fseeko(file_, 0, SEEK_END);
    sectors_ = static_cast<u64>(ftello(file_)) / kSector;
    return true;
}

bool Disc::read(u32 sector, u32 count, u8* out) {
    // No image, or the range runs past the last sector (summed in 64 bits, so it cannot wrap).
    if (!file_ || static_cast<u64>(sector) + count > sectors_) {
        return false;
    }

    fseeko(file_, static_cast<off_t>(sector) * static_cast<off_t>(kSector), SEEK_SET);
    return std::fread(out, kSector, count, file_) == count;
}

std::optional<Disc::File> Disc::find(const std::string& path) {
    /*
     * The primary volume descriptor is sector 16; its root directory record is
     * at offset 156. A record holds the extent's sector at 2, its length at 10,
     * flags at 25 (bit 1: directory), the name's length at 32 and the name at 33 (documented).
     */
    std::vector<u8> sector(kSector);

    // Not an ISO 9660 volume: the descriptor's standard identifier "CD001" is at bytes 1-5.
    if (!read(16, 1, sector.data())
        || std::string(sector.begin() + 1, sector.begin() + 6) != "CD001") {
        return std::nullopt;
    }

    // Start at the root directory: its extent sector and length come from the root record.
    File current{load<u32>(&sector[156 + 2]), load<u32>(&sector[156 + 10])};

    std::size_t at = 0;

    // One pass per path part; the loop ends when the path is used up or a part is missing.
    while (at < path.size()) {
        // Skip the separators in front of the next part.
        while (at < path.size() && (path[at] == '/' || path[at] == '\\')) {
            at++;
        }

        // The part runs to the next separator, or to the end of the path.
        std::size_t end = path.find_first_of("/\\", at);
        if (end == std::string::npos) {
            end = path.size();
        }

        // Only separators were left: the path is finished.
        if (end == at) {
            break;
        }

        std::string want = canonical(path.substr(at, end - at));
        at = end;

        // Read the whole current directory, rounded up to whole sectors.
        std::vector<u8> dir((current.bytes + kSector - 1) / kSector * kSector);

        // The directory could not be read.
        if (!read(current.sector, static_cast<u32>(dir.size() / kSector), dir.data())) {
            return std::nullopt;
        }

        bool found = false;

        // Walk the records until the name is found; the smallest record is 34 bytes.
        for (std::size_t r = 0; r + 34 <= dir.size();) {
            unsigned length = dir[r];

            // A zero length is padding: records do not cross sectors, so go on in the next one.
            if (length == 0) {
                r = (r / kSector + 1) * kSector;
                continue;
            }

            // The name sits at byte 33 of the record and its length is the byte at 32.
            std::string name(
                dir.begin() + static_cast<long>(r) + 33,
                dir.begin() + static_cast<long>(r) + 33 + dir[r + 32]
            );

            // This record is the part of the path being looked for: descend into it.
            if (canonical(name) == want) {
                current = File{load<u32>(&dir[r + 2]), load<u32>(&dir[r + 10])};
                found = true;
                break;
            }
            r += length;
        }

        // No record had the name: the path does not exist on the disc.
        if (!found) {
            return std::nullopt;
        }
    }

    return current;
}

std::vector<u8> Disc::read_file(const File& file) {
    // Sectors come whole: round the buffer up, then cut it to the length of the file.
    std::vector<u8> bytes((file.bytes + kSector - 1) / kSector * kSector);

    // The sectors could not be read: an empty file stands for the failure.
    if (!read(file.sector, static_cast<u32>(bytes.size() / kSector), bytes.data())) {
        return {};
    }

    bytes.resize(file.bytes);
    return bytes;
}

}  // namespace sys
