// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-formats/src/sha1.rs:
// ISC License, Copyright (c) 2026 ReRAC contributors.
//
// SHA-1 and SHA-256 block functions and the shared Merkle-Damgard padding.

#include "assets/disc/checksum.h"

#include <bit>
#include <cstring>

namespace openrac::assets::disc {

namespace {

using u8 = std::uint8_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

u32 load_be32(const u8* p) {
    return (u32{p[0]} << 24) | (u32{p[1]} << 16) | (u32{p[2]} << 8) | u32{p[3]};
}

void store_be32(u8* p, u32 v) {
    p[0] = static_cast<u8>(v >> 24);
    p[1] = static_cast<u8>(v >> 16);
    p[2] = static_cast<u8>(v >> 8);
    p[3] = static_cast<u8>(v);
}

void sha1_block(std::array<u32, 5>& h, const u8* block) {
    std::array<u32, 80> w{};

    for (int i = 0; i < 16; ++i) {
        w[i] = load_be32(block + 4 * i);
    }

    for (int i = 16; i < 80; ++i) {
        w[i] = std::rotl(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
    }

    u32 a = h[0], b = h[1], c = h[2], d = h[3], e = h[4];

    for (int i = 0; i < 80; ++i) {
        u32 f = 0;
        u32 k = 0;

        if (i < 20) {
            f = (b & c) | (~b & d);
            k = 0x5a827999;
        } else if (i < 40) {
            f = b ^ c ^ d;
            k = 0x6ed9eba1;
        } else if (i < 60) {
            f = (b & c) | (b & d) | (c & d);
            k = 0x8f1bbcdc;
        } else {
            f = b ^ c ^ d;
            k = 0xca62c1d6;
        }

        const u32 t = std::rotl(a, 5) + f + e + k + w[i];
        e = d;
        d = c;
        c = std::rotl(b, 30);
        b = a;
        a = t;
    }

    h[0] += a;
    h[1] += b;
    h[2] += c;
    h[3] += d;
    h[4] += e;
}

constexpr std::array<u32, 64> kSha256K = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2,
};

void sha256_block(std::array<u32, 8>& h, const u8* block) {
    std::array<u32, 64> w{};

    for (int i = 0; i < 16; ++i) {
        w[i] = load_be32(block + 4 * i);
    }

    for (int i = 16; i < 64; ++i) {
        const u32 s0 = std::rotr(w[i - 15], 7) ^ std::rotr(w[i - 15], 18) ^ (w[i - 15] >> 3);
        const u32 s1 = std::rotr(w[i - 2], 17) ^ std::rotr(w[i - 2], 19) ^ (w[i - 2] >> 10);
        w[i] = w[i - 16] + s0 + w[i - 7] + s1;
    }

    u32 a = h[0], b = h[1], c = h[2], d = h[3], e = h[4], f = h[5], g = h[6], hh = h[7];

    for (int i = 0; i < 64; ++i) {
        const u32 s1 = std::rotr(e, 6) ^ std::rotr(e, 11) ^ std::rotr(e, 25);
        const u32 ch = (e & f) ^ (~e & g);
        const u32 t1 = hh + s1 + ch + kSha256K[i] + w[i];
        const u32 s0 = std::rotr(a, 2) ^ std::rotr(a, 13) ^ std::rotr(a, 22);
        const u32 maj = (a & b) ^ (a & c) ^ (b & c);
        const u32 t2 = s0 + maj;
        hh = g;
        g = f;
        f = e;
        e = d + t1;
        d = c;
        c = b;
        b = a;
        a = t1 + t2;
    }

    h[0] += a;
    h[1] += b;
    h[2] += c;
    h[3] += d;
    h[4] += e;
    h[5] += f;
    h[6] += g;
    h[7] += hh;
}

// Feeds `data` through 64-byte blocks; both hashes share this framing.
template <typename State, typename Block>
void feed(
    State& h,
    std::array<u8, 64>& buffer,
    std::size_t& fill,
    u64& length,
    std::span<const u8> data,
    Block block
) {
    length += data.size();
    std::size_t at = 0;

    if (fill > 0) {
        const std::size_t n = std::min(64 - fill, data.size());
        std::memcpy(buffer.data() + fill, data.data(), n);
        fill += n;
        at = n;

        if (fill < 64) {
            return;
        }

        block(h, buffer.data());
        fill = 0;
    }

    while (data.size() - at >= 64) {
        block(h, data.data() + at);
        at += 64;
    }

    std::memcpy(buffer.data(), data.data() + at, data.size() - at);
    fill = data.size() - at;
}

// The final padding: 0x80, zeros, the bit length big-endian.
template <typename State, typename Block>
State pad(State h, std::array<u8, 64> buffer, std::size_t fill, u64 length, Block block) {
    buffer[fill++] = 0x80;

    if (fill > 56) {
        std::memset(buffer.data() + fill, 0, 64 - fill);
        block(h, buffer.data());
        fill = 0;
    }

    std::memset(buffer.data() + fill, 0, 56 - fill);
    const u64 bits = length * 8;
    store_be32(buffer.data() + 56, static_cast<u32>(bits >> 32));
    store_be32(buffer.data() + 60, static_cast<u32>(bits));
    block(h, buffer.data());
    return h;
}

std::span<const u8> bytes_of(std::string_view text) {
    return {reinterpret_cast<const u8*>(text.data()), text.size()};
}

}  // namespace

void Sha1::update(std::span<const u8> data) {
    feed(m_h, m_block, m_fill, m_length, data, sha1_block);
}

void Sha1::update(std::string_view text) {
    update(bytes_of(text));
}

Sha1Digest Sha1::finish() const {
    const auto h = pad(m_h, m_block, m_fill, m_length, sha1_block);
    Sha1Digest out{};

    for (std::size_t i = 0; i < h.size(); ++i) {
        store_be32(out.data() + 4 * i, h[i]);
    }

    return out;
}

void Sha256::update(std::span<const u8> data) {
    feed(m_h, m_block, m_fill, m_length, data, sha256_block);
}

void Sha256::update(std::string_view text) {
    update(bytes_of(text));
}

Sha256Digest Sha256::finish() const {
    const auto h = pad(m_h, m_block, m_fill, m_length, sha256_block);
    Sha256Digest out{};

    for (std::size_t i = 0; i < h.size(); ++i) {
        store_be32(out.data() + 4 * i, h[i]);
    }

    return out;
}

Sha1Digest sha1(std::span<const u8> data) {
    Sha1 s;
    s.update(data);
    return s.finish();
}

Sha256Digest sha256(std::span<const u8> data) {
    Sha256 s;
    s.update(data);
    return s.finish();
}

std::string to_hex(std::span<const u8> bytes) {
    static constexpr char kDigits[] = "0123456789abcdef";
    std::string out;
    out.reserve(bytes.size() * 2);

    for (const u8 b : bytes) {
        out.push_back(kDigits[b >> 4]);
        out.push_back(kDigits[b & 15]);
    }

    return out;
}

std::optional<Sha1Digest> parse_sha1(std::string_view hex) {
    if (hex.size() != 40) {
        return std::nullopt;
    }

    auto nibble = [](char c) -> int {
        if (c >= '0' && c <= '9') {
            return c - '0';
        }

        if (c >= 'a' && c <= 'f') {
            return c - 'a' + 10;
        }

        if (c >= 'A' && c <= 'F') {
            return c - 'A' + 10;
        }

        return -1;
    };
    Sha1Digest out{};

    for (std::size_t i = 0; i < out.size(); ++i) {
        const int hi = nibble(hex[2 * i]);
        const int lo = nibble(hex[2 * i + 1]);

        if (hi < 0 || lo < 0) {
            return std::nullopt;
        }

        out[i] = static_cast<u8>(hi << 4 | lo);
    }

    return out;
}

}  // namespace openrac::assets::disc
