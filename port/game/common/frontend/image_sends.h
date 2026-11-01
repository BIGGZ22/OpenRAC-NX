// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The image transfers in a tick's 2D path (Kept::packets), read as the direct renderer reads the
// path (renderer::GifInterpreter::vif and gif): each DIRECT's GIF data after its 16-byte header
// (NOP NOP NOP DIRECT, as frame.cpp's direct_packets lays the path out), tag by tag. Only an A+D
// write in a PACKED tag sets BITBLTBUF, TRXPOS, TRXREG or TRXDIR (REGLIST's 4-bit descriptors
// cannot name them); the other PACKED descriptors (colours, positions, texture coordinates) and
// IMAGE data are stepped over, not read as register writes. Image data runs on into the next
// DIRECT only while a transfer is open; a register tag cut off by its DIRECT's end drops the rest
// of that DIRECT, as the interpreter drops it.
//
// The older reading (frame.cpp, sends_image) takes every 16-byte step of the path as an A+D write:
// it finds every real TRXDIR (an A+D write is always a whole aligned qword), but a colour, a depth
// or a texel whose ninth byte is 0x53 is taken for one too, and such values were logged as
// destinations like "block 0x2406 format 0x31 578x578" and "block 0x0 0x0" (session 39).
//
// Register layouts are the GS manual's: BITBLTBUF DBP bits 32-45, DBW 48-53, DPSM 56-61; TRXPOS
// DSAX 32-42, DSAY 48-58; TRXREG RRW 0-11, RRH 32-43; TRXDIR XDIR 0-1 (0 host to local, 1 local
// to host, 2 local to local); FRAME_n FBP 0-8.

#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <span>
#include <utility>
#include <vector>

#include "renderer/gs.h"

namespace openrac::frontend::image_sends {

namespace gs = renderer::gs;

// One host-to-local transfer (TRXDIR 0) and the image data that came with it.
struct Transfer {
    std::uint32_t dbp = 0;   // destination block
    std::uint32_t dbw = 0;   // destination width, in 64-pixel units
    std::uint32_t dpsm = 0;  // destination format
    std::uint32_t dsax = 0;  // destination rectangle's corner
    std::uint32_t dsay = 0;
    std::uint32_t rrw = 0;  // rectangle's size
    std::uint32_t rrh = 0;
    std::size_t bytes = 0;     // what the rectangle takes (0: a format the renderer does not size)
    std::size_t received = 0;  // the image data that came with it in these packets
    std::uint64_t hash = 0;    // of the received bytes (64-bit FNV-1a over 8-byte words)

    [[nodiscard]] bool complete() const { return bytes > 0 && received >= bytes; }
    // Same destination rectangle and format.
    [[nodiscard]] bool same_place(const Transfer& o) const {
        return dbp == o.dbp && dbw == o.dbw && dpsm == o.dpsm && dsax == o.dsax && dsay == o.dsay
               && rrw == o.rrw && rrh == o.rrh;
    }
};

// Why a path could not be read cleanly: the first such place is kept (Scan::first), for the log.
enum class Problem : std::uint8_t {
    None,
    NotDirect,      // a header whose fourth VIF code is not DIRECT/DIRECTHL
    DirectPastEnd,  // a DIRECT whose data runs past the path's end
    Trailing,       // bytes after the last whole DIRECT
    CutPacked,      // a PACKED tag cut off by its DIRECT's end (the interpreter drops the rest)
    CutReglist,     // the same for a REGLIST tag
    CutImage,       // IMAGE data past its DIRECT's end with no transfer open (dropped too)
    ImageUnfinished // a transfer's image data still running on at the path's end
};

inline const char* problem_name(Problem p) {
    switch (p) {
        case Problem::None: return "none";
        case Problem::NotDirect: return "header not DIRECT";
        case Problem::DirectPastEnd: return "DIRECT past the end";
        case Problem::Trailing: return "bytes after the last DIRECT";
        case Problem::CutPacked: return "PACKED tag cut by its DIRECT";
        case Problem::CutReglist: return "REGLIST tag cut by its DIRECT";
        case Problem::CutImage: return "IMAGE data past its DIRECT with no transfer open";
        case Problem::ImageUnfinished: return "image data still open at the end";
    }
    return "?";
}

// Where the first problem was: the DIRECT (counted from 0), the tag's place in that DIRECT's data
// (a tag carried in from the DIRECT before counts from the start of that carried data), the bytes
// the DIRECT had from there, and the tag's two words.
struct ProblemAt {
    Problem kind = Problem::None;
    std::size_t direct = 0;
    std::size_t direct_bytes = 0;
    std::size_t tag_at = 0;
    std::size_t left = 0;
    std::uint64_t lo = 0;
    std::uint64_t hi = 0;
};

struct Scan {
    std::vector<Transfer> uploads;     // host to local, in the order sent
    unsigned reads = 0;                // TRXDIR 1: local to host
    unsigned local_copies = 0;         // TRXDIR 2: inside the chip's memory
    unsigned offscreen_frames = 0;     // FRAME_1/2 writes away from pages 0 and 0x70
    unsigned cut = 0;                  // tags cut off by their DIRECT's end (all three kinds)
    unsigned cut_packed = 0, cut_reglist = 0, cut_image = 0;
    bool malformed = false;            // a header that is not a DIRECT, or data past the end
    bool image_unfinished = false;     // a transfer's IMAGE data still carried at the end
    unsigned directs = 0;              // DIRECTs read
    ProblemAt first;                   // the first problem, if any
};

// Bytes a transfer of `psm` takes per pixel, in bits (renderer/texture.cpp, raster_bits).
constexpr int raster_bits(std::uint32_t psm) {
    switch (psm) {
        case gs::kPsmct24:
        case gs::kPsmz24:
            return 24;
        case gs::kPsmt8h:
            return 8;
        case gs::kPsmt4hl:
        case gs::kPsmt4hh:
            return 4;
        default:
            return gs::bits_per_pixel(static_cast<std::uint8_t>(psm));
    }
}

constexpr std::size_t rectangle_bytes(std::uint32_t psm, std::uint32_t w, std::uint32_t h) {
    return (std::size_t{w} * h * static_cast<std::size_t>(raster_bits(psm)) + 7) / 8;
}

inline std::uint64_t read64(const std::uint8_t* p) {
    std::uint64_t v = 0;
    std::memcpy(&v, p, 8);
    return v;
}

class Reader {
public:
    Scan scan(std::span<const std::uint8_t> packets) {
        std::size_t at = 0;
        while (at + 16 <= packets.size()) {
            const std::uint64_t hi = read64(packets.data() + at + 8);
            const auto code = static_cast<std::uint32_t>(hi >> 32);  // the fourth VIF code
            const std::uint32_t command = (code >> 24) & 0x7F;
            if (command != 0x50 && command != 0x51) {
                m_scan.malformed = true;
                note(Problem::NotDirect, at, packets.size() - at, read64(packets.data() + at), hi);
                break;
            }
            const std::uint32_t qwords = (code & 0xFFFF) == 0 ? 65536u : (code & 0xFFFF);
            const std::size_t size = std::size_t{qwords} * 16;
            at += 16;
            if (at + size > packets.size()) {
                m_scan.malformed = true;
                note(Problem::DirectPastEnd, at, packets.size() - at, read64(packets.data() + at - 16), hi);
                break;
            }
            m_direct_bytes = size;
            direct(packets.subspan(at, size));
            ++m_scan.directs;
            at += size;
        }
        if (at != packets.size()) {
            if (!m_scan.malformed) {
                note(Problem::Trailing, at, packets.size() - at, 0, 0);
            }
            m_scan.malformed = true;
        }
        if (!m_pending.empty()) {
            m_scan.image_unfinished = true;
            note(Problem::ImageUnfinished, 0, m_pending.size(), read64(m_pending.data()),
                 read64(m_pending.data() + 8));
        }
        return std::move(m_scan);
    }

private:
    Scan m_scan;
    std::size_t m_direct_bytes = 0;  // the DIRECT being read

    void note(Problem kind, std::size_t tag_at, std::size_t left, std::uint64_t lo, std::uint64_t hi) {
        if (m_scan.first.kind != Problem::None) {
            return;
        }
        m_scan.first = {kind, m_scan.directs, m_direct_bytes, tag_at, left, lo, hi};
    }
    std::vector<std::uint8_t> m_pending;  // an IMAGE tag that runs on into the next DIRECT
    std::uint64_t m_bitbltbuf = 0;
    std::uint64_t m_trxpos = 0;
    std::uint64_t m_trxreg = 0;
    std::ptrdiff_t m_open = -1;  // the upload image data goes to while it wants more, or -1

    void write(std::uint32_t address, std::uint64_t value) {
        switch (address) {
            case gs::kBitbltbuf:
                m_bitbltbuf = value;
                break;
            case gs::kTrxpos:
                m_trxpos = value;
                break;
            case gs::kTrxreg:
                m_trxreg = value;
                break;
            case gs::kTrxdir: {
                const std::uint32_t direction = gs::bits(value, 0, 2);
                if (direction == 0) {
                    Transfer t;
                    t.dbp = gs::bits(m_bitbltbuf, 32, 14);
                    t.dbw = gs::bits(m_bitbltbuf, 48, 6);
                    t.dpsm = gs::bits(m_bitbltbuf, 56, 6);
                    t.dsax = gs::bits(m_trxpos, 32, 11);
                    t.dsay = gs::bits(m_trxpos, 48, 11);
                    t.rrw = gs::bits(m_trxreg, 0, 12);
                    t.rrh = gs::bits(m_trxreg, 32, 12);
                    t.bytes = rectangle_bytes(t.dpsm, t.rrw, t.rrh);
                    t.hash = 0xcbf29ce484222325ull;
                    m_scan.uploads.push_back(t);
                    m_open = t.bytes > 0 ? static_cast<std::ptrdiff_t>(m_scan.uploads.size()) - 1 : -1;
                } else if (direction == 1) {
                    // A read back leaves an open transfer open, as the direct renderer does (it acts
                    // on directions 0 and 2 only); such a path is never vouched for anyway.
                    ++m_scan.reads;
                } else if (direction == 2) {
                    ++m_scan.local_copies;
                }
                break;
            }
            case gs::kFrame1:
            case gs::kFrame2: {
                const std::uint32_t page = gs::bits(value, 0, 9);
                if (page != 0 && page != 0x70) {
                    ++m_scan.offscreen_frames;
                }
                break;
            }
            default:
                break;
        }
    }

    void image(const std::uint8_t* data, std::size_t size) {
        if (m_open < 0) {
            return;  // image data without a transfer: the interpreter drops it too
        }
        Transfer& t = m_scan.uploads[static_cast<std::size_t>(m_open)];
        const std::size_t take = std::min(size, t.bytes - t.received);
        std::uint64_t h = t.hash;
        std::size_t i = 0;
        for (; i + 8 <= take; i += 8) {
            h = (h ^ read64(data + i)) * 0x100000001b3ull;
        }
        for (; i < take; ++i) {
            h = (h ^ data[i]) * 0x100000001b3ull;
        }
        t.hash = h;
        t.received += take;
        if (t.received >= t.bytes) {
            m_open = -1;
        }
    }

    void direct(std::span<const std::uint8_t> input) {
        std::vector<std::uint8_t> joined;
        std::span<const std::uint8_t> packet = input;
        if (!m_pending.empty()) {
            // Sized once and copied in two parts (GCC 13's -O2 flagged the vector insert's
            // copy with a false -Wstringop-overflow).
            joined.resize(m_pending.size() + input.size());
            std::memcpy(joined.data(), m_pending.data(), m_pending.size());
            if (!input.empty()) {
                std::memcpy(joined.data() + m_pending.size(), input.data(), input.size());
            }
            m_pending.clear();
            packet = joined;
        }
        std::size_t at = 0;
        while (at + 16 <= packet.size()) {
            const gs::GifTag tag =
                gs::GifTag::decode(read64(packet.data() + at), read64(packet.data() + at + 8));
            const std::size_t tag_at = at;
            at += 16;
            if (tag.flg == gs::GifTag::kPacked) {
                const std::size_t size = std::size_t{tag.nloop} * tag.nreg * 16;
                if (at + size > packet.size()) {
                    ++m_scan.cut;
                    ++m_scan.cut_packed;
                    note(Problem::CutPacked, tag_at, packet.size() - tag_at,
                         read64(packet.data() + tag_at), read64(packet.data() + tag_at + 8));
                    return;
                }
                for (std::uint32_t loop = 0; loop < tag.nloop; ++loop) {
                    for (std::uint32_t r = 0; r < tag.nreg; ++r, at += 16) {
                        if (tag.reg(r) == gs::kPackedAd) {
                            write(
                                static_cast<std::uint32_t>(packet[at + 8]),
                                read64(packet.data() + at)
                            );
                        }
                    }
                }
            } else if (tag.flg == gs::GifTag::kReglist) {
                const std::size_t words = std::size_t{tag.nloop} * tag.nreg;
                const std::size_t size = ((words + 1) / 2) * 16;
                if (at + size > packet.size()) {
                    ++m_scan.cut;
                    ++m_scan.cut_reglist;
                    note(Problem::CutReglist, tag_at, packet.size() - tag_at,
                         read64(packet.data() + tag_at), read64(packet.data() + tag_at + 8));
                    return;
                }
                at += size;  // 4-bit descriptors: none of the transfer registers
            } else {
                const std::size_t size = std::size_t{tag.nloop} * 16;
                if (at + size > packet.size()) {
                    if (m_open >= 0) {
                        m_pending.assign(packet.begin() + static_cast<std::ptrdiff_t>(tag_at),
                                         packet.end());
                    } else {
                        ++m_scan.cut;
                        ++m_scan.cut_image;
                        note(Problem::CutImage, tag_at, packet.size() - tag_at,
                             read64(packet.data() + tag_at), read64(packet.data() + tag_at + 8));
                    }
                    return;
                }
                image(packet.data() + at, size);
                at += size;
            }
        }
    }
};

inline Scan scan(std::span<const std::uint8_t> packets) {
    return Reader().scan(packets);
}

// Where two ticks' transfer lists first part (diagnostics): the same lists; one is the other with
// transfers added or missing at its end, or the counts differ; a transfer at another place (block,
// width, format, corner, rectangle); or only a transfer's bytes at the same place. `order_only`:
// the place differs but the lists hold the same transfers in another order.
enum class Difference : std::uint8_t { Same, Count, Place, Bytes };
struct FirstDifference {
    Difference kind = Difference::Same;
    std::size_t index = 0;  // the first transfer that differs
    bool order_only = false;
};

inline FirstDifference first_difference(const std::vector<Transfer>& now, const std::vector<Transfer>& was) {
    FirstDifference d;
    std::size_t i = 0;
    while (i < now.size() && i < was.size() && now[i].same_place(was[i]) && now[i].bytes == was[i].bytes
           && now[i].received == was[i].received && now[i].hash == was[i].hash) {
        ++i;
    }
    d.index = i;
    if (i == now.size() && i == was.size()) {
        return d;
    }
    if (i == now.size() || i == was.size()) {
        d.kind = Difference::Count;
    } else if (now[i].same_place(was[i]) && now[i].bytes == was[i].bytes) {
        d.kind = Difference::Bytes;
    } else if (now.size() != was.size()) {
        d.kind = Difference::Count;
    } else {
        d.kind = Difference::Place;
        auto key = [](const Transfer& t) {
            return std::array<std::uint64_t, 5>{t.dbp, (std::uint64_t{t.dbw} << 32) | t.dpsm,
                                                (std::uint64_t{t.dsax} << 32) | t.dsay,
                                                (std::uint64_t{t.rrw} << 32) | t.rrh, t.hash};
        };
        std::vector<std::array<std::uint64_t, 5>> a, b;
        for (const auto& t : now) a.push_back(key(t));
        for (const auto& t : was) b.push_back(key(t));
        std::sort(a.begin(), a.end());
        std::sort(b.begin(), b.end());
        d.order_only = a == b;
    }
    return d;
}

// Whether `scan` saw what the direct renderer takes from the path, so that its uploads are the
// renderer's. A tag cut off by the end of its DIRECT (a register tag running past it, or IMAGE
// data running past it with no transfer open) is dropped by the renderer too, with the rest of
// that DIRECT, before any of it is written (GifInterpreter::gif, cut(), in a VIF DIRECT): nothing
// of it reaches the GS state, the texture pool or a draw, and Reader drops exactly the same bytes.
// Session 44's captured gameplay path (kept local, not in the repository): the real interpreter
// and Reader delivered the same 13 uploads byte for byte, and its 12 cuts were 48-byte DIRECTs of
// texel-like bytes (grey RGBA) where a GIF tag would be. `strict` (the old reading) refuses any
// cut. Never accepted: a header that is not a DIRECT, data past the path's end (direct_packets
// makes neither: they would mean a misread path), or image data still open at the end (the
// renderer carries it into the next frame).
inline bool read_as_renderer(const Scan& scan, bool strict) {
    if (scan.malformed || scan.image_unfinished) {
        return false;
    }
    return scan.cut == 0 || !strict;
}

}  // namespace openrac::frontend::image_sends
