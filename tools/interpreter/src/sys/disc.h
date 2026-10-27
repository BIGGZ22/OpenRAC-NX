// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

/**
 * The disc image the user supplies, read by sector and by file name.
 *
 * An image is a file of 2,048-byte sectors. The games read it two ways: by sector number, and by
 * the path of a file, which is looked up in the ISO 9660 directory. Only what the games use is
 * modelled: no Joliet or Rock Ridge names and no files in more than one extent.
 *
 * Sources: the ISO 9660 layout as publicly documented.
 */

#pragma once

#include <cstdio>
#include <optional>
#include <string>
#include <vector>

#include "ps2/types.h"

namespace sys {

/**
 * The user's own disc image: sectors of 2,048 bytes, and the ISO 9660
 * directory for the few files the games open by name.
 *
 * A read is a seek followed by a read on one host file, so two threads must not call it at once.
 * It owns the host file and closes it in the destructor.
 */
class Disc {
public:
    /** Bytes in one sector of user data on a data disc (documented). */
    static constexpr std::size_t kSector = 2048;

    /** Where a file lies on the disc: its first sector and its length. */
    struct File {
        /** First sector of the file's extent. */
        ps2::u32 sector = 0;

        /** Length of the file in bytes; the last sector is only partly used. */
        ps2::u32 bytes = 0;
    };

    /** Closes the image if one is open. */
    ~Disc();

    /**
     * Opens an image from the host's file system, closing the one open before.
     *
     * @param path Host path of the image file.
     * @return True when the file opened; false leaves the disc closed.
     */
    bool open(const std::string& path);

    /** True once an image has been opened. */
    bool is_open() const { return file_ != nullptr; }

    /** The number of whole sectors in the image. */
    ps2::u64 sectors() const { return sectors_; }

    /**
     * Reads whole sectors; false if any lies outside the image.
     *
     * @param sector First sector to read.
     * @param count Number of sectors to read.
     * @param[out] out Receives `count * kSector` bytes.
     * @return True when every sector was read; false when no image is open, when the range ends
     *     past the last sector or when the host read came up short.
     * @pre `out` has room for `count * kSector` bytes.
     */
    bool read(ps2::u32 sector, ps2::u32 count, ps2::u8* out);

    /**
     * A file by its path from the root, in any case, with or without ";1".
     *
     * Either slash separates the parts of the path.
     *
     * @param path Path of the file from the root of the disc.
     * @return Where the file lies, or nothing when the image has no ISO 9660 volume or the path
     *     names nothing on it.
     */
    std::optional<File> find(const std::string& path);

    /**
     * Reads a whole file into memory.
     *
     * @param file A file found by `find`.
     * @return The file's bytes, or an empty vector when its sectors could not be read.
     */
    std::vector<ps2::u8> read_file(const File& file);

private:
    /** The open image, or null while none is open; owned, closed by the destructor and `open`. */
    std::FILE* file_ = nullptr;

    /** Whole sectors in the image, set by `open`. */
    ps2::u64 sectors_ = 0;
};

}  // namespace sys
