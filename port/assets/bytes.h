// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Bounds-checked reads of the console's little-endian data. Every reader in
// openrac_assets takes its input as a ByteView and throws AssetError when a
// record runs past the end, so a damaged or unexpected file is reported with
// what was being read and where, never read out of bounds.

#pragma once

#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <format>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace openrac::assets {

static_assert(
    std::endian::native == std::endian::little, "the readers assume a little-endian host"
);

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;
using s8 = std::int8_t;
using s16 = std::int16_t;
using s32 = std::int32_t;
using s64 = std::int64_t;
using f32 = float;

// A file that is not what its reader expects.
class AssetError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

template <typename... Args>
[[noreturn]] void fail(std::format_string<Args...> fmt, Args&&... args) {
    throw AssetError(std::format(fmt, std::forward<Args>(args)...));
}

// A borrowed run of bytes with checked little-endian accessors. Copying a
// view copies the pointer, never the bytes.
class ByteView {
public:
    constexpr ByteView() = default;

    constexpr ByteView(const u8* data, std::size_t size) : m_data(data), m_size(size) {}

    constexpr ByteView(std::span<const u8> bytes) : m_data(bytes.data()), m_size(bytes.size()) {}

    ByteView(const std::vector<u8>& bytes) : m_data(bytes.data()), m_size(bytes.size()) {}

    constexpr const u8* data() const { return m_data; }

    constexpr std::size_t size() const { return m_size; }

    constexpr bool empty() const { return m_size == 0; }

    constexpr std::span<const u8> span() const { return {m_data, m_size}; }

    std::vector<u8> to_vector() const { return {m_data, m_data + m_size}; }

    // Throws unless [offset, offset + length) lies inside the view.
    void check(std::size_t offset, std::size_t length, std::string_view what) const {
        if (offset > m_size || length > m_size - offset) {
            fail(
                "{}: {:#x} bytes at {:#x} run past the end of a {:#x}-byte buffer",
                what,
                length,
                offset,
                m_size
            );
        }
    }

    ByteView sub(std::size_t offset, std::size_t length, std::string_view what = "range") const {
        check(offset, length, what);
        return {m_data + offset, length};
    }

    ByteView tail(std::size_t offset, std::string_view what = "range") const {
        check(offset, 0, what);
        return {m_data + offset, m_size - offset};
    }

    // A trivially copyable record, read unaligned.
    template <typename T>
    T read(std::size_t offset, std::string_view what = "value") const {
        static_assert(std::is_trivially_copyable_v<T>);
        check(offset, sizeof(T), what);
        T value;
        std::memcpy(&value, m_data + offset, sizeof(T));
        return value;
    }

    // `count` records of T, one after the other.
    template <typename T>
    std::vector<T> read_array(
        std::size_t offset, std::size_t count, std::string_view what = "array"
    ) const {
        static_assert(std::is_trivially_copyable_v<T>);

        if (count != 0 && sizeof(T) > (m_size + 1) / count + 1) {
            fail("{}: {} records of {:#x} bytes cannot fit", what, count, sizeof(T));
        }

        check(offset, count * sizeof(T), what);
        std::vector<T> values(count);

        if (count != 0) {
            std::memcpy(values.data(), m_data + offset, count * sizeof(T));
        }

        return values;
    }

    u8 u8_at(std::size_t o) const { return read<u8>(o, "u8"); }

    s8 s8_at(std::size_t o) const { return read<s8>(o, "s8"); }

    u16 u16_at(std::size_t o) const { return read<u16>(o, "u16"); }

    s16 s16_at(std::size_t o) const { return read<s16>(o, "s16"); }

    u32 u32_at(std::size_t o) const { return read<u32>(o, "u32"); }

    s32 s32_at(std::size_t o) const { return read<s32>(o, "s32"); }

    u64 u64_at(std::size_t o) const { return read<u64>(o, "u64"); }

    f32 f32_at(std::size_t o) const { return read<f32>(o, "f32"); }

    // A NUL-terminated string of at most `max` bytes (the whole field when it
    // has no NUL).
    std::string string_at(std::size_t offset, std::size_t max) const {
        check(offset, 0, "string");
        const std::size_t limit = std::min(max, m_size - offset);
        const u8* begin = m_data + offset;
        const void* nul = std::memchr(begin, 0, limit);
        const std::size_t length = nul ? static_cast<const u8*>(nul) - begin : limit;
        return {reinterpret_cast<const char*>(begin), length};
    }

private:
    const u8* m_data = nullptr;
    std::size_t m_size = 0;
};

// Little-endian appends, for writers and for tests that build synthetic data.
class ByteWriter {
public:
    std::vector<u8>& bytes() { return m_bytes; }

    const std::vector<u8>& bytes() const { return m_bytes; }

    std::size_t size() const { return m_bytes.size(); }

    template <typename T>
    void put(const T& value) {
        static_assert(std::is_trivially_copyable_v<T>);
        const auto* p = reinterpret_cast<const u8*>(&value);
        m_bytes.insert(m_bytes.end(), p, p + sizeof(T));
    }

    template <typename T>
    void put_at(std::size_t offset, const T& value) {
        static_assert(std::is_trivially_copyable_v<T>);

        if (m_bytes.size() < offset + sizeof(T)) {
            m_bytes.resize(offset + sizeof(T));
        }

        std::memcpy(m_bytes.data() + offset, &value, sizeof(T));
    }

    void put_bytes(std::span<const u8> data) {
        m_bytes.insert(m_bytes.end(), data.begin(), data.end());
    }

    void pad_to(std::size_t alignment, u8 fill = 0) {
        while (m_bytes.size() % alignment != 0) {
            m_bytes.push_back(fill);
        }
    }

    void resize(std::size_t size) { m_bytes.resize(size); }

private:
    std::vector<u8> m_bytes;
};

}  // namespace openrac::assets
