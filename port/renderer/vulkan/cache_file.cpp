// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The backend's cache files (cache_file.h): the envelope, the read that checks it, and the save
// through a temporary file of its own.
//
// The envelope is 24 bytes before the data: the eight letters of the file's magic, the data's
// length and its 64-bit FNV-1a hash, both little-endian.

#include "renderer/vulkan/cache_file.h"

#include <algorithm>
#include <array>
#include <climits>
#include <cstdint>
#include <format>
#include <fstream>
#include <functional>
#include <ios>
#include <random>
#include <system_error>
#include <thread>
#include <tuple>

namespace openrac::vkgl {

namespace {

// The size of each of the envelope's two numbers, the data's length and its hash.
constexpr std::size_t kFieldBytes = sizeof(std::uint64_t);

// Where the envelope's numbers are, and its length: the magic, the length, the hash.
constexpr std::size_t kLengthAt = std::tuple_size_v<CacheMagic>;
constexpr std::size_t kHashAt = kLengthAt + kFieldBytes;
constexpr std::size_t kEnvelopeBytes = kHashAt + kFieldBytes;

// The largest file read. A driver's cache of the port's pipelines is far smaller (assumed), as is
// every other cache file, so a larger file is not one of these, and is not read into memory.
constexpr std::uintmax_t kLargestFileBytes = std::uintmax_t{256} << 20U;  // 256 MiB

// The 64-bit FNV-1a hash's offset basis and prime (documented: the FNV reference).
constexpr std::uint64_t kFnvOffset = 0xcbf29ce484222325ULL;
constexpr std::uint64_t kFnvPrime = 0x100000001b3ULL;

// Returns the 64-bit FNV-1a hash of `data`.
std::uint64_t fnv1a(std::span<const char> data) {
    std::uint64_t hash = kFnvOffset;
    for (const char byte : data) {
        hash = (hash ^ std::uint64_t{static_cast<unsigned char>(byte)}) * kFnvPrime;
    }
    return hash;
}

// Writes `value` little-endian into the kFieldBytes bytes at `out`: byte n holds bits 8n to 8n+7.
void put_u64(char* out, std::uint64_t value) {
    for (std::size_t n = 0; n < kFieldBytes; ++n) {
        out[n] = static_cast<char>(static_cast<unsigned char>(value >> (CHAR_BIT * n)));
    }
}

// Returns the little-endian value of the kFieldBytes bytes at `in`.
std::uint64_t get_u64(const char* in) {
    std::uint64_t value = 0;
    for (std::size_t n = 0; n < kFieldBytes; ++n) {
        value |= std::uint64_t{static_cast<unsigned char>(in[n])} << (CHAR_BIT * n);
    }
    return value;
}

// Returns a name beside `path` that no other save uses at the same time: a random 64-bit number
// from the system, with the calling thread's id mixed in, as `<file>.<hex>.tmp`.
std::filesystem::path temporary_beside(const std::filesystem::path& path) {
    std::random_device device;
    // random_device gives 32 bits at a time: two make the 64.
    const std::uint64_t random = (std::uint64_t{device()} << 32U) ^ std::uint64_t{device()};
    const std::uint64_t thread = std::hash<std::thread::id>{}(std::this_thread::get_id());
    std::filesystem::path temporary = path;
    temporary += std::format(".{:016x}.tmp", random ^ thread);
    return temporary;
}

// Writes the envelope of `magic` and `data` to `path`, created afresh. False when any byte is not
// written.
bool write_file(
    const std::filesystem::path& path, const CacheMagic& magic, std::span<const char> data
) {
    std::array<char, kEnvelopeBytes> envelope{};
    std::copy(magic.begin(), magic.end(), envelope.begin());
    put_u64(envelope.data() + kLengthAt, data.size());
    put_u64(envelope.data() + kHashAt, fnv1a(data));

    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    out.write(envelope.data(), static_cast<std::streamsize>(envelope.size()));
    out.write(data.data(), static_cast<std::streamsize>(data.size()));
    out.close();
    return !out.fail();
}

}  // namespace

CacheFile read_cache_file(const std::filesystem::path& path, const CacheMagic& magic) {
    // Opened first and measured through the open file: a save renamed over `path` meanwhile is
    // another file, and cannot change the length or bytes this read sees.
    std::ifstream in(path, std::ios::binary | std::ios::ate);
    const std::streamoff size = in.is_open() ? std::streamoff{in.tellg()} : -1;

    // No file (or no access to it) is the first run's case.
    if (size < 0) {
        return {};
    }

    // Too short for the envelope, or larger than any cache: not a file this code wrote.
    if (static_cast<std::uintmax_t>(size) < kEnvelopeBytes
        || static_cast<std::uintmax_t>(size) > kLargestFileBytes) {
        return {CacheFileState::Damaged, {}};
    }

    std::vector<char> bytes(static_cast<std::size_t>(size));
    in.seekg(0);
    in.read(bytes.data(), static_cast<std::streamsize>(bytes.size()));

    // A read error on a file that opened: treated as no file, so it is neither used nor reported.
    if (!in) {
        return {};
    }

    const std::span<const char> data(bytes.data() + kEnvelopeBytes, bytes.size() - kEnvelopeBytes);
    const bool same_magic = std::equal(magic.begin(), magic.end(), bytes.begin());
    const bool length = get_u64(bytes.data() + kLengthAt) == data.size();

    // Another format, or a save cut short or mixed from two writers: its hash tells.
    if (!same_magic || !length || get_u64(bytes.data() + kHashAt) != fnv1a(data)) {
        return {CacheFileState::Damaged, {}};
    }

    return {CacheFileState::Intact, std::vector<char>(data.begin(), data.end())};
}

bool write_cache_file(
    const std::filesystem::path& path,
    const CacheMagic& magic,
    std::span<const char> data,
    std::string& error
) {
    std::error_code ec;
    std::filesystem::create_directories(path.parent_path(), ec);

    // Without its folder there is nowhere to save; the next run starts without the file.
    if (ec) {
        error =
            std::format("cannot make the folder {}: {}", path.parent_path().string(), ec.message());
        return false;
    }

    // Written whole under a name of its own first, so no reader or other writer sees it half done.
    const std::filesystem::path temporary = temporary_beside(path);

    // A full disk or a folder without write access: nothing may be left behind.
    if (!write_file(temporary, magic, data)) {
        std::filesystem::remove(temporary, ec);
        error = std::format("cannot write {}", temporary.string());
        return false;
    }

    std::filesystem::rename(temporary, path, ec);

    // On Windows a reader holding `path` open refuses the rename; that save is lost, not the file.
    if (ec) {
        const std::string reason = ec.message();
        std::filesystem::remove(temporary, ec);
        error =
            std::format("cannot rename {} to {}: {}", temporary.string(), path.string(), reason);
        return false;
    }

    return true;
}

}  // namespace openrac::vkgl
