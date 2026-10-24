// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-formats/src/wad.rs and
// docs/formats/disc_layout.md section 3: ISC License, Copyright (c) 2026 ReRAC contributors.
//
// WAD decompression and compression. Packet kinds, by flag byte:
//
//   0x00..0x0f  literal run (4..273 bytes); never followed by another one
//   0x10..0x1f  far match (displacement 0x4000 * (A + 1) + 14 bits), or, with
//               a zero displacement, the pad (skip to the next 0x1000 bytes of
//               the stream) and dummy (only a little literal) packets
//   0x20..0x3f  medium and big match (displacement 1..16384, 3..288 bytes)
//   0x40..0xff  little match (displacement 1..2048, 3..8 bytes)
//
// Every match and the dummy packet end with 0..3 "little literal" bytes, their
// count in the low two bits of the packet's second-to-last byte.

#include "assets/disc/wad.h"

#include <algorithm>
#include <array>
#include <cstring>

namespace openrac::assets::disc {

namespace {

constexpr u8 kMagic[3] = {'W', 'A', 'D'};

// Encoder limits (ReRAC's disc_layout.md 3.5).
constexpr std::size_t kChunk = 0x2000;         // no packet crosses a chunk of the stream
constexpr std::size_t kMaxLiteral = 273;       // one literal packet
constexpr std::size_t kMaxMatch = 288;         // a big match
constexpr std::size_t kMaxFarMatch = 264;      // a big far match
constexpr std::size_t kMediumWindow = 0x4000;  // medium and big matches
constexpr std::size_t kFarWindow = 32704;      // far matches with A = 0, as Wrench's matcher
constexpr std::size_t kLittleWindow = 2048;
constexpr std::size_t kLittleMaxLength = 8;
constexpr std::size_t kHashBits = 15;
constexpr int kChainLimit = 32;
constexpr u8 kFiller = 0xee;  // the conventional filler after a pad packet

}  // namespace

bool is_wad(ByteView bytes) {
    return bytes.size() >= kWadHeaderSize && std::memcmp(bytes.data(), kMagic, 3) == 0;
}

u32 wad_compressed_size(ByteView bytes) {
    // Only streams that start with the WAD header have a compressed size.
    if (!is_wad(bytes)) {
        fail("not a WAD stream");
    }

    return bytes.u32_at(3);
}

std::vector<u8> wad_decompress(ByteView bytes) {
    const std::size_t total = wad_compressed_size(bytes);

    // The size must cover the header and fit the buffer given.
    if (total < kWadHeaderSize || total > bytes.size()) {
        fail("WAD: compressed size {:#x} exceeds the {:#x}-byte buffer", total, bytes.size());
    }

    const u8* stream = bytes.data() + kWadHeaderSize;
    const std::size_t end = total - kWadHeaderSize;
    std::size_t ptr = 0;
    std::vector<u8> out;
    out.reserve(total * 2);

    auto next = [&]() -> u8 {
        // A read past the stream's end means it was cut short.
        if (ptr >= end) {
            fail("WAD: read past the end of the stream");
        }

        return stream[ptr++];
    };
    auto literals = [&](std::size_t n) {
        // A literal run past the end means the stream was cut short.
        if (end - ptr < n) {
            fail("WAD: a literal runs past the end of the stream");
        }

        out.insert(out.end(), stream + ptr, stream + ptr + n);
        ptr += n;
    };
    auto copy_match = [&](std::size_t displacement, std::size_t length) {
        // A match cannot point before the start of the output.
        if (displacement == 0 || displacement > out.size()) {
            fail("WAD: a match starts before the output");
        }

        // Byte by byte: overlapping copies are run-length expansion.
        const std::size_t source = out.size() - displacement;

        // Copy byte by byte so a match that overlaps its own output repeats it.
        for (std::size_t i = 0; i < length; ++i) {
            out.push_back(out[source + i]);
        }
    };

    // Decode packets until the stream is used up.
    while (ptr < end) {
        const u8 flag = next();

        // A flag below 0x10 starts a literal run.
        if (flag < 0x10) {
            const std::size_t n = flag == 0 ? std::size_t{next()} + 18 : std::size_t{flag} + 3;
            literals(n);

            // The format never puts two literal runs in a row, so a second one means a damaged
            // stream.
            if (ptr < end && stream[ptr] < 0x10) {
                fail("WAD: two literal packets in a row");
            }

            continue;
        }

        std::size_t length = 0;
        std::size_t displacement = 0;
        u8 little = 0;

        // Flags 0x10 to 0x1f are far matches.
        if (flag < 0x20) {
            std::size_t ml = flag & 7;

            // A zero length field means the length follows in the next byte.
            if (ml == 0) {
                ml = std::size_t{next()} + 7;
            }

            const u8 b0 = next();
            const u8 b1 = next();
            const std::size_t a = (flag >> 3) & 1;
            const std::size_t far = std::size_t{b1} * 0x40 + (b0 >> 2);

            // A far match with no displacement is the padding code or the end of the stream.
            if (a == 0 && far == 0) {
                // A length other than 1 is padding: the stream goes on at the next 0x1000 boundary.
                if (ml != 1) {
                    ptr = (ptr + 0xfff) & ~std::size_t{0xfff};  // pad: on at the next 0x1000
                    continue;
                }

                literals(b0 & 3);  // dummy
                continue;
            }

            // Bit 3 extends the window (the form verified on the NTSC-U disc).
            displacement = 0x4000 * (a + 1) + far;
            length = ml + 2;
            little = b0;
        } else if (flag < 0x40) {
            std::size_t ml = flag & 0x1f;

            // A zero length field means the length follows in the next byte.
            if (ml == 0) {
                ml = std::size_t{next()} + 0x1f;
            }

            length = ml + 2;
            const u8 b1 = next();
            const u8 b2 = next();
            displacement = std::size_t{b2} * 0x40 + (b1 >> 2) + 1;
            little = b1;
        } else {
            const u8 b1 = next();
            length = (flag >> 5) + 1;
            displacement = std::size_t{b1} * 8 + ((flag >> 2) & 7) + 1;
            little = flag;
        }

        copy_match(displacement, length);
        literals(little & 3);
    }

    return out;
}

namespace {

class Encoder {
public:
    explicit Encoder(ByteView data) : m_data(data) {
        static constexpr u8 kHeader[kWadHeaderSize] =
            {'W', 'A', 'D', 0, 0, 0, 0, 'O', 'P', 'E', 'N', 'R', 'A', 'C', 0, 0};
        m_out.assign(std::begin(kHeader), std::end(kHeader));
    }

    // A literal run with no packet before it to carry it.
    void free_literals(std::size_t at, std::size_t n) {
        // No bytes need no packet.
        if (n == 0) {
            return;
        }

        // A run of 3 bytes or less is carried by a dummy packet.
        if (n <= 3) {
            dummy(at, n);
            return;
        }

        bool first = true;

        // Cut long runs into pieces a literal packet can hold.
        while (n > 0) {
            std::size_t chunk = std::min(n, kMaxLiteral);

            // The rest of a run may not be 1 to 3 bytes, which no literal packet could carry.
            if (n - chunk > 0 && n - chunk < 4) {
                chunk = n - 4;  // leave a run a literal packet can carry
            }

            // Literal packets are never adjacent, so a dummy goes between pieces.
            if (!first) {
                dummy(at, 0);  // literal packets are never adjacent
            }

            literal_packet(at, chunk);
            at += chunk;
            n -= chunk;
            first = false;
        }
    }

    // A match followed by `little` (0..3) literal bytes from `at`.
    void match(std::size_t length, std::size_t displacement, std::size_t at, std::size_t little) {
        const u8 l = static_cast<u8>(little);
        u8 head[4];
        std::size_t size = 0;

        // The shortest form fits a short match at a short distance.
        if (length <= kLittleMaxLength && displacement <= kLittleWindow) {
            const std::size_t d = displacement - 1;
            head[size++] = static_cast<u8>(((length - 1) << 5) | ((d & 7) << 2) | l);
            head[size++] = static_cast<u8>(d >> 3);
        } else if (displacement <= kMediumWindow) {
            const std::size_t d = displacement - 1;

            // A medium match of up to 33 bytes puts its length in the flag.
            if (length <= 33) {
                head[size++] = static_cast<u8>(0x20 | (length - 2));
            } else {
                head[size++] = 0x20;
                head[size++] = static_cast<u8>(length - 33);
            }

            head[size++] = static_cast<u8>(((d & 63) << 2) | l);
            head[size++] = static_cast<u8>(d >> 6);
        } else {
            const std::size_t far = displacement - 0x4000;

            // A far match of up to 9 bytes puts its length in the flag.
            if (length <= 9) {
                head[size++] = static_cast<u8>(0x10 | (length - 2));
            } else {
                head[size++] = 0x10;
                head[size++] = static_cast<u8>(length - 9);
            }

            head[size++] = static_cast<u8>(((far & 63) << 2) | l);
            head[size++] = static_cast<u8>(far >> 6);
        }

        packet({head, size}, at, little);
    }

    std::vector<u8> finish() {
        const u32 total = static_cast<u32>(m_out.size());
        std::memcpy(m_out.data() + 3, &total, sizeof total);
        return std::move(m_out);
    }

private:
    void literal_packet(std::size_t at, std::size_t n) {
        u8 head[2];
        std::size_t size = 0;

        // A run of up to 18 bytes puts its length in the flag.
        if (n <= 18) {
            head[size++] = static_cast<u8>(n - 3);
        } else {
            head[size++] = 0;
            head[size++] = static_cast<u8>(n - 18);
        }

        packet({head, size}, at, n);
    }

    void dummy(std::size_t at, std::size_t n) {
        const u8 head[3] = {0x11, static_cast<u8>(n), 0x00};
        packet(head, at, n);
    }

    // Writes a packet and its trailing bytes, after a pad when it would cross
    // a chunk (with room left for the 3-byte pad itself).
    void packet(std::span<const u8> head, std::size_t at, std::size_t trailing) {
        const std::size_t size = head.size() + trailing;
        const std::size_t offset = m_out.size() - kWadHeaderSize;

        // A packet must not straddle a chunk, so pad out the chunk first.
        if (offset % kChunk + size > kChunk - 3) {
            m_out.insert(m_out.end(), {0x12, 0x00, 0x00});

            // Fill with the filler byte until the chunk boundary.
            while ((m_out.size() - kWadHeaderSize) % kChunk != 0) {
                m_out.push_back(kFiller);
            }
        }

        m_out.insert(m_out.end(), head.begin(), head.end());
        m_out.insert(m_out.end(), m_data.data() + at, m_data.data() + at + trailing);
    }

    ByteView m_data;
    std::vector<u8> m_out;
};

struct Match {
    std::size_t at;
    std::size_t length;
    std::size_t displacement;
};

// Greedy hash-chain matching over three-byte prefixes.
std::vector<Match> find_matches(ByteView data) {
    const u8* d = data.data();
    const std::size_t n = data.size();
    std::vector<Match> matches;

    // Data shorter than the minimum match has none.
    if (n < 3) {
        return matches;
    }

    std::vector<s64> head(std::size_t{1} << kHashBits, -1);
    std::vector<s64> prev(n, -1);
    auto hash = [&](std::size_t i) {
        const u32 v = u32{d[i]} | u32{d[i + 1]} << 8 | u32{d[i + 2]} << 16;
        return static_cast<std::size_t>((v * 2654435761u) >> (32 - kHashBits));
    };
    auto insert = [&](std::size_t i) {
        // Only positions with three bytes left can be hashed.
        if (i + 3 <= n) {
            const std::size_t h = hash(i);
            prev[i] = head[h];
            head[h] = static_cast<s64>(i);
        }
    };
    std::size_t i = 0;

    // Walk the data, looking for the longest match at each position.
    while (i + 3 <= n) {
        std::size_t best = 0;
        std::size_t best_displacement = 0;
        s64 candidate = head[hash(i)];

        // Follow the hash chain of earlier positions with the same three bytes, up to a limit.
        for (int chain = 0; candidate >= 0 && chain < kChainLimit; ++chain) {
            const std::size_t c = static_cast<std::size_t>(candidate);
            const std::size_t displacement = i - c;

            // Positions beyond the far window cannot be reached, and the chain only gets farther.
            if (displacement > kFarWindow) {
                break;
            }

            const std::size_t limit =
                std::min(n - i, displacement > kMediumWindow ? kMaxFarMatch : kMaxMatch);
            std::size_t length = 0;

            // Extend the match for as long as the bytes agree.
            while (length < limit && d[c + length] == d[i + length]) {
                ++length;
            }

            // Keep the longest match found.
            if (length > best) {
                best = length;
                best_displacement = displacement;

                // A match at the limit cannot be beaten.
                if (length == limit) {
                    break;
                }
            }

            candidate = prev[c];
        }

        // Matches shorter than 3 bytes are not worth a packet.
        if (best >= 3) {
            matches.push_back({i, best, best_displacement});

            // Hash the bytes inside the match so later positions can match them.
            for (std::size_t k = 0; k < best; ++k) {
                insert(i + k);
            }

            i += best;
        } else {
            insert(i);
            ++i;
        }
    }

    return matches;
}

}  // namespace

std::vector<u8> wad_compress(ByteView data) {
    const std::vector<Match> matches = find_matches(data);
    Encoder e(data);
    e.free_literals(0, matches.empty() ? data.size() : matches.front().at);

    // Emit each match with the literals that follow it.
    for (std::size_t k = 0; k < matches.size(); ++k) {
        const Match& m = matches[k];
        const std::size_t after = m.at + m.length;
        const std::size_t next = k + 1 < matches.size() ? matches[k + 1].at : data.size();
        const std::size_t run = next - after;

        // A run of up to 3 literals rides in the match's own packet.
        if (run <= 3) {
            e.match(m.length, m.displacement, after, run);
        } else {
            e.match(m.length, m.displacement, after, 0);
            e.free_literals(after, run);
        }
    }

    return e.finish();
}

}  // namespace openrac::assets::disc
