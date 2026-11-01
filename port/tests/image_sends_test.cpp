// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The 2D path's image transfers read tag by tag (port/game/common/frontend/image_sends.h), on paths
// laid out as frame.cpp's direct_packets lays them out: each DIRECT's GIF data after a 16-byte
// NOP NOP NOP DIRECT header. Synthetic packets only.

#include <cstdint>
#include <cstring>
#include <span>
#include <vector>

#include "game/common/frontend/image_sends.h"
#include "renderer/gs.h"
#include "tests/check.h"
#include "tests/renderer_gif_builder.h"

namespace {

namespace gs = openrac::renderer::gs;
namespace sends = openrac::frontend::image_sends;
using openrac::test::GifBuilder;

// One DIRECT of `gif` (whole qwords), with its header, appended to `path`.
void direct(std::vector<std::uint8_t>& path, const std::vector<std::uint8_t>& gif) {
    const std::uint32_t head[4] = {0, 0, 0, 0x50000000u | static_cast<std::uint32_t>(gif.size() / 16)};
    const auto* h = reinterpret_cast<const std::uint8_t*>(head);
    path.insert(path.end(), h, h + sizeof head);
    path.insert(path.end(), gif.begin(), gif.end());
}

std::vector<std::uint8_t> texels(std::size_t bytes, std::uint8_t seed) {
    std::vector<std::uint8_t> v(bytes);
    for (std::size_t i = 0; i < bytes; ++i) {
        v[i] = static_cast<std::uint8_t>(seed + i * 7);
    }
    return v;
}

// The older reading (frame.cpp, sends_image): every qword taken as an A+D write.
bool qword_reading(std::span<const std::uint8_t> packets) {
    for (std::size_t at = 0; at + 16 <= packets.size(); at += 16) {
        std::uint64_t value = 0;
        std::memcpy(&value, packets.data() + at, 8);
        if (packets[at + 8] == gs::kTrxdir && gs::bits(value, 0, 2) == 0) {
            return true;
        }
    }
    return false;
}

}  // namespace

int main() {
    // An upload, decoded with its destination and all its bytes; the same bytes hash the same.
    {
        GifBuilder b;
        b.image(0x3317, 1, gs::kPsmct32, 64, 64, texels(64 * 64 * 4, 1));
        std::vector<std::uint8_t> path;
        direct(path, b.bytes);
        const sends::Scan s = sends::scan(path);
        CHECK(!s.malformed);
        CHECK(s.cut == 0);
        CHECK(s.uploads.size() == 1);
        if (s.uploads.size() == 1) {
            const sends::Transfer& t = s.uploads[0];
            CHECK(t.dbp == 0x3317);
            CHECK(t.dbw == 1);
            CHECK(t.dpsm == gs::kPsmct32);
            CHECK(t.rrw == 64 && t.rrh == 64);
            CHECK(t.bytes == 64 * 64 * 4);
            CHECK(t.complete());
        }
        const sends::Scan again = sends::scan(path);
        CHECK(again.uploads.size() == 1 && s.uploads.size() == 1
              && again.uploads[0].hash == s.uploads[0].hash);

        GifBuilder other;
        other.image(0x3317, 1, gs::kPsmct32, 64, 64, texels(64 * 64 * 4, 2));
        std::vector<std::uint8_t> changed;
        direct(changed, other.bytes);
        const sends::Scan c = sends::scan(changed);
        CHECK(c.uploads.size() == 1 && s.uploads.size() == 1
              && c.uploads[0].hash != s.uploads[0].hash && c.uploads[0].same_place(s.uploads[0]));
    }

    // A palette and an 8-bit texture in one DIRECT, as the HUD sends them: two transfers.
    {
        GifBuilder b;
        b.image(0x3357, 1, gs::kPsmct32, 16, 16, texels(16 * 16 * 4, 3));
        b.image(0x335b, 1, gs::kPsmt8, 64, 64, texels(64 * 64, 4));
        std::vector<std::uint8_t> path;
        direct(path, b.bytes);
        const sends::Scan s = sends::scan(path);
        CHECK(s.uploads.size() == 2);
        if (s.uploads.size() == 2) {
            CHECK(s.uploads[0].dbp == 0x3357 && s.uploads[0].bytes == 1024 && s.uploads[0].complete());
            CHECK(s.uploads[1].dbp == 0x335b && s.uploads[1].dpsm == gs::kPsmt8
                  && s.uploads[1].bytes == 4096 && s.uploads[1].complete());
        }
    }

    // Colours and depths that read as a TRXDIR write when every qword is taken for A+D: a
    // PACKED RGBAQ whose blue is 0x53 and red a multiple of four, an XYZ2 whose depth's low byte
    // is 0x53. The older reading sees a transfer; there is none.
    {
        GifBuilder b;
        b.tag(1, false, gs::GifTag::kPacked, 3,
              std::uint64_t{gs::kPackedRgbaq} | (std::uint64_t{gs::kPackedXyz2} << 4)
                  | (std::uint64_t{gs::kPackedRgbaq} << 8));
        b.qword(std::uint64_t{0x40} | (std::uint64_t{0x10} << 32), std::uint64_t{0x53} | (std::uint64_t{0x80} << 32));
        b.qword(std::uint64_t{0x8000} | (std::uint64_t{0x8000} << 32), std::uint64_t{0x1253});
        b.qword(0, std::uint64_t{0x53});
        std::vector<std::uint8_t> path;
        direct(path, b.bytes);
        CHECK(qword_reading(path));
        const sends::Scan s = sends::scan(path);
        CHECK(s.uploads.empty());
        CHECK(!s.malformed && s.cut == 0);
    }

    // Texels whose ninth byte of a qword is 0x53: image data, not a write.
    {
        std::vector<std::uint8_t> data(64 * 64 * 4, 0);
        for (std::size_t at = 0; at + 16 <= data.size(); at += 16) {
            data[at + 8] = 0x53;
        }
        GifBuilder b;
        b.image(0x1000, 2, gs::kPsmct32, 64, 64, data);
        std::vector<std::uint8_t> path;
        direct(path, b.bytes);
        const sends::Scan s = sends::scan(path);
        CHECK(s.uploads.size() == 1);
        CHECK(s.uploads.size() == 1 && s.uploads[0].complete() && s.uploads[0].dbp == 0x1000);
    }

    // Image data that runs on into the next DIRECT while its transfer is open: one transfer, whole.
    {
        GifBuilder head;
        head.ad({{gs::kBitbltbuf, (std::uint64_t{0x2000} << 32) | (std::uint64_t{1} << 48)},
                 {gs::kTrxpos, 0},
                 {gs::kTrxreg, std::uint64_t{32} | (std::uint64_t{32} << 32)},
                 {gs::kTrxdir, 0}});
        head.tag(32 * 32 * 4 / 16, false, gs::GifTag::kImage, 0, 0);
        const std::vector<std::uint8_t> pixels = texels(32 * 32 * 4, 9);
        std::vector<std::uint8_t> first = head.bytes;
        first.insert(first.end(), pixels.begin(), pixels.begin() + 2048);
        const std::vector<std::uint8_t> second(pixels.begin() + 2048, pixels.end());
        std::vector<std::uint8_t> path;
        direct(path, first);
        direct(path, second);
        const sends::Scan s = sends::scan(path);
        CHECK(s.uploads.size() == 1);
        CHECK(s.uploads.size() == 1 && s.uploads[0].complete() && s.uploads[0].received == 4096);
        CHECK(s.cut == 0 && !s.malformed);

        // The same bytes in one DIRECT hash the same.
        GifBuilder whole;
        whole.image(0x2000, 1, gs::kPsmct32, 32, 32, pixels);
        std::vector<std::uint8_t> one;
        direct(one, whole.bytes);
        const sends::Scan w = sends::scan(one);
        CHECK(w.uploads.size() == 1 && s.uploads.size() == 1 && w.uploads[0].hash == s.uploads[0].hash);
    }

    // A REGLIST tag is stepped over by its size, padded to whole qwords; a transfer after it is
    // read.
    {
        GifBuilder b;
        b.tag(3, false, gs::GifTag::kReglist, 1, gs::kRgbaq);  // three 64-bit words: two qwords
        b.qword(0x53000000ull << 32, 0x5353535353535353ull);
        b.qword(0x53ull, 0);
        b.image(0x3000, 1, gs::kPsmct32, 8, 8, texels(256, 5));
        std::vector<std::uint8_t> path;
        direct(path, b.bytes);
        const sends::Scan s = sends::scan(path);
        CHECK(s.uploads.size() == 1 && s.uploads[0].dbp == 0x3000 && s.uploads[0].complete());
    }

    // A register tag cut off by its DIRECT's end drops the rest of that DIRECT, as the interpreter
    // does; the next DIRECT is read from its first tag.
    {
        GifBuilder cut;
        cut.tag(4, false, gs::GifTag::kPacked, 1, gs::kPackedAd);  // four writes announced
        cut.qword(0, gs::kTrxdir);                                  // one sent
        GifBuilder next;
        next.image(0x3100, 1, gs::kPsmct32, 8, 8, texels(256, 6));
        std::vector<std::uint8_t> path;
        direct(path, cut.bytes);
        direct(path, next.bytes);
        const sends::Scan s = sends::scan(path);
        CHECK(s.cut == 1);
        CHECK(s.cut_packed == 1 && s.cut_reglist == 0 && s.cut_image == 0);
        CHECK(s.first.kind == sends::Problem::CutPacked);
        CHECK(s.first.direct == 0 && s.first.tag_at == 0 && s.first.left == 32);
        CHECK((s.first.lo & 0x7FFF) == 4);
        CHECK(s.directs == 2);
        CHECK(s.uploads.size() == 1 && s.uploads[0].dbp == 0x3100);
    }

    // IMAGE data past its DIRECT with no transfer open is dropped and named; image data of an open
    // transfer still running on at the path's end is named too.
    {
        GifBuilder loose;
        loose.tag(8, false, gs::GifTag::kImage, 0, 0);  // eight qwords announced, one sent
        loose.qword(1, 2);
        std::vector<std::uint8_t> path;
        direct(path, loose.bytes);
        const sends::Scan s = sends::scan(path);
        CHECK(s.cut_image == 1 && s.first.kind == sends::Problem::CutImage);
        CHECK(!s.image_unfinished && s.uploads.empty());

        GifBuilder open;
        open.ad({{gs::kBitbltbuf, std::uint64_t{0x2200} << 32},
                 {gs::kTrxreg, std::uint64_t{16} | (std::uint64_t{16} << 32)},
                 {gs::kTrxdir, 0}});
        open.tag(64, false, gs::GifTag::kImage, 0, 0);  // 1 KiB announced
        open.qword(3, 4);                               // 16 bytes sent
        std::vector<std::uint8_t> unfinished;
        direct(unfinished, open.bytes);
        const sends::Scan u = sends::scan(unfinished);
        CHECK(u.image_unfinished && u.first.kind == sends::Problem::ImageUnfinished);
        CHECK(u.cut == 0 && !u.malformed);
        CHECK(u.uploads.size() == 1 && !u.uploads[0].complete());
    }

    // Copies inside the chip, read-backs and renders to a texture are counted, not taken for
    // uploads.
    {
        GifBuilder b;
        b.ad({{gs::kTrxdir, 2}, {gs::kTrxdir, 1}, {gs::kFrame1, 0x1A0}, {gs::kFrame1, 0x70}, {gs::kFrame2, 0}});
        std::vector<std::uint8_t> path;
        direct(path, b.bytes);
        const sends::Scan s = sends::scan(path);
        CHECK(s.uploads.empty());
        CHECK(s.local_copies == 1);
        CHECK(s.reads == 1);
        CHECK(s.offscreen_frames == 1);
    }

    // Not the layout direct_packets makes: a VIF code other than DIRECT, or a DIRECT past the end.
    {
        std::vector<std::uint8_t> path(32, 0);
        CHECK(sends::scan(path).malformed);
        CHECK(sends::scan(path).first.kind == sends::Problem::NotDirect);
        GifBuilder b;
        b.ad({{gs::kTrxdir, 0}});
        std::vector<std::uint8_t> shortened;
        direct(shortened, b.bytes);
        shortened.resize(shortened.size() - 16);
        CHECK(sends::scan(shortened).malformed);
        CHECK(sends::scan(shortened).first.kind == sends::Problem::DirectPastEnd);
        std::vector<std::uint8_t> trailing;
        direct(trailing, b.bytes);
        trailing.resize(trailing.size() + 8, 0);
        CHECK(sends::scan(trailing).malformed);
        CHECK(sends::scan(trailing).first.kind == sends::Problem::Trailing);
        CHECK(!sends::scan(std::vector<std::uint8_t>{}).malformed);
    }

    // An upload whose format the renderer does not size (bytes 0) is listed, never complete.
    {
        GifBuilder b;
        b.ad({{gs::kBitbltbuf, (std::uint64_t{0x10} << 32) | (std::uint64_t{0x3F} << 56)},
              {gs::kTrxreg, std::uint64_t{8} | (std::uint64_t{8} << 32)},
              {gs::kTrxdir, 0}});
        std::vector<std::uint8_t> path;
        direct(path, b.bytes);
        const sends::Scan s = sends::scan(path);
        CHECK(s.uploads.size() == 1 && s.uploads[0].bytes == 0 && !s.uploads[0].complete());
    }

    // The shape of session 44's captured gameplay path (synthetic bytes; the capture stays local):
    // DIRECTs of three qwords holding texel-like bytes where a GIF tag would be (their first
    // qword reads as IMAGE, NLOOP 5268, past the DIRECT, with no transfer open: dropped, by the
    // renderer as here), between HUD uploads whose IMAGE tag ends one DIRECT and whose texels fill
    // the next. The uploads are read whole; the path is the renderer's unless read strictly.
    {
        std::vector<std::uint8_t> path;
        GifBuilder setup;
        setup.ad({{gs::kFrame1, 0x70}, {gs::kTest1, 0x5360b}});
        direct(path, setup.bytes);
        const auto stale = [&] {
            GifBuilder texels;
            texels.qword(0x0a89898914949494ull, 0x0d8d8d8d0e8e8e8eull);
            texels.qword(0x0b8b8b0d8c8c8c8cull, 0x0187878701878787ull);
            texels.qword(0x0d8c8c8c0e8d8d8dull, 0x0d8c8c8c0d8c8c8cull);
            direct(path, texels.bytes);
        };
        stale();
        stale();
        const auto hud_upload = [&](std::uint32_t dbp, std::uint8_t psm, std::uint32_t w,
                                    std::uint32_t h, std::uint8_t seed) {
            const std::size_t bytes = sends::rectangle_bytes(psm, w, h);
            GifBuilder head;
            head.ad({{gs::kBitbltbuf, (std::uint64_t{dbp} << 32) | (std::uint64_t{1} << 48)
                                          | (std::uint64_t{psm} << 56)},
                     {gs::kTrxreg, std::uint64_t{w} | (std::uint64_t{h} << 32)},
                     {gs::kTrxdir, 0}});
            head.tag(static_cast<std::uint32_t>(bytes / 16), true, gs::GifTag::kImage, 0, 0);
            direct(path, head.bytes);
            direct(path, texels(bytes, seed));
        };
        hud_upload(0x3357, gs::kPsmct32, 16, 16, 1);
        hud_upload(0x335b, gs::kPsmt8, 64, 64, 2);
        stale();
        const sends::Scan s = sends::scan(path);
        CHECK(s.cut == 3 && s.cut_image == 3 && s.cut_packed == 0);
        CHECK(s.first.kind == sends::Problem::CutImage && s.first.direct == 1);
        CHECK((s.first.lo & 0x7FFF) == 5268 && ((s.first.lo >> 58) & 3) == 2);
        CHECK(!s.malformed && !s.image_unfinished);
        CHECK(s.uploads.size() == 2);
        CHECK(s.uploads.size() == 2 && s.uploads[0].complete() && s.uploads[1].complete());
        CHECK(s.uploads.size() == 2 && s.uploads[0].dbp == 0x3357 && s.uploads[1].dbp == 0x335b);
        CHECK(sends::read_as_renderer(s, false));
        CHECK(!sends::read_as_renderer(s, true));

        // The same uploads without the stale DIRECTs: the same transfers, bytes and order.
        std::vector<std::uint8_t> clean;
        direct(clean, setup.bytes);
        std::swap(path, clean);
        hud_upload(0x3357, gs::kPsmct32, 16, 16, 1);
        hud_upload(0x335b, gs::kPsmt8, 64, 64, 2);
        std::swap(path, clean);
        const sends::Scan c = sends::scan(clean);
        CHECK(c.cut == 0 && c.uploads.size() == 2);
        CHECK(c.uploads.size() == 2 && s.uploads.size() == 2 && c.uploads[0].hash == s.uploads[0].hash
              && c.uploads[1].hash == s.uploads[1].hash);
        CHECK(sends::read_as_renderer(c, true));
    }

    // A read back (TRXDIR 1) leaves an open transfer open, as the direct renderer does: the image
    // data after it still fills the transfer (found by comparing with the real interpreter).
    {
        GifBuilder b;
        b.ad({{gs::kBitbltbuf, std::uint64_t{0x2400} << 32},
              {gs::kTrxreg, std::uint64_t{2} | (std::uint64_t{2} << 32)},
              {gs::kTrxdir, 0},
              {gs::kTrxdir, 1}});
        b.tag(1, true, gs::GifTag::kImage, 0, 0);
        b.qword(0x1111111111111111ull, 0x2222222222222222ull);
        std::vector<std::uint8_t> path;
        direct(path, b.bytes);
        const sends::Scan s = sends::scan(path);
        CHECK(s.reads == 1);
        CHECK(s.uploads.size() == 1 && s.uploads[0].complete() && s.uploads[0].received == 16);
    }

    // Never the renderer's view: a misread path, or image data still open at its end.
    {
        sends::Scan m;
        m.malformed = true;
        CHECK(!sends::read_as_renderer(m, false));
        sends::Scan u;
        u.image_unfinished = true;
        CHECK(!sends::read_as_renderer(u, false));
        sends::Scan cut_only;
        cut_only.cut = 1;
        cut_only.cut_packed = 1;
        CHECK(sends::read_as_renderer(cut_only, false));
        CHECK(!sends::read_as_renderer(cut_only, true));
    }

    // Where two ticks' transfer lists first part (diagnostics only).
    {
        auto t = [](std::uint32_t dbp, std::uint64_t hash) {
            sends::Transfer x;
            x.dbp = dbp;
            x.dpsm = gs::kPsmct32;
            x.dbw = 1;
            x.rrw = x.rrh = 16;
            x.bytes = x.received = 1024;
            x.hash = hash;
            return x;
        };
        const std::vector<sends::Transfer> a{t(1, 10), t(2, 20), t(3, 30)};
        CHECK(sends::first_difference(a, a).kind == sends::Difference::Same);
        const std::vector<sends::Transfer> shorter{t(1, 10), t(2, 20)};
        auto d = sends::first_difference(shorter, a);
        CHECK(d.kind == sends::Difference::Count && d.index == 2);
        const std::vector<sends::Transfer> bytes{t(1, 10), t(2, 21), t(3, 30)};
        d = sends::first_difference(bytes, a);
        CHECK(d.kind == sends::Difference::Bytes && d.index == 1);
        const std::vector<sends::Transfer> swapped{t(2, 20), t(1, 10), t(3, 30)};
        d = sends::first_difference(swapped, a);
        CHECK(d.kind == sends::Difference::Place && d.index == 0 && d.order_only);
        const std::vector<sends::Transfer> moved{t(1, 10), t(4, 20), t(3, 30)};
        d = sends::first_difference(moved, a);
        CHECK(d.kind == sends::Difference::Place && d.index == 1 && !d.order_only);
        const std::vector<sends::Transfer> extra{t(1, 10), t(5, 50), t(2, 20), t(3, 30)};
        d = sends::first_difference(extra, a);
        CHECK(d.kind == sends::Difference::Count && d.index == 1);
        auto partial = a;
        partial[2].received = 512;
        d = sends::first_difference(partial, a);
        CHECK(d.kind == sends::Difference::Bytes && d.index == 2);
    }

    return openrac::test::result();
}
