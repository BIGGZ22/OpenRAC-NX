// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-video/src/mpeg2.rs:
// ISC License, Copyright (c) 2026 ReRAC contributors.
//
// The MPEG-2 video decoder (mpeg2.h). Section numbers are ISO/IEC 13818-2's.

#include "media/mpeg2.h"

#include <algorithm>
#include <format>
#include <span>

#include "media/bit_reader.h"
#include "media/idct.h"
#include "media/vlc.h"

namespace openrac::media {

const std::array<std::uint8_t, 64> kZigZag = {
    0,  1,  8,  16, 9,  2,  3,  10, 17, 24, 32, 25, 18, 11, 4,  5,  12, 19, 26, 33, 40, 48,
    41, 34, 27, 20, 13, 6,  7,  14, 21, 28, 35, 42, 49, 56, 57, 50, 43, 36, 29, 22, 15, 23,
    30, 37, 44, 51, 58, 59, 52, 45, 38, 31, 39, 46, 53, 60, 61, 54, 47, 55, 62, 63,
};

const std::array<std::uint8_t, 64> kAlternateScan = {
    0,  8,  16, 24, 1,  9,  2,  10, 17, 25, 32, 40, 48, 56, 57, 49, 41, 33, 26, 18, 3,  11,
    4,  12, 19, 27, 34, 42, 50, 58, 35, 43, 51, 59, 20, 28, 5,  13, 6,  14, 21, 29, 36, 44,
    52, 60, 37, 45, 53, 61, 22, 30, 7,  15, 23, 31, 38, 46, 54, 62, 39, 47, 55, 63,
};

const std::array<std::uint8_t, 64> kDefaultIntraMatrix = {
    8,  16, 19, 22, 26, 27, 29, 34, 16, 16, 22, 24, 27, 29, 34, 37, 19, 22, 26, 27, 29, 34,
    34, 38, 22, 22, 26, 27, 29, 34, 37, 40, 22, 26, 27, 29, 32, 35, 40, 48, 26, 27, 29, 32,
    35, 40, 48, 58, 26, 27, 29, 34, 38, 46, 56, 69, 27, 29, 35, 38, 46, 56, 69, 83,
};

VideoError::VideoError(std::size_t offset, const std::string& message)
    : std::runtime_error(std::format("mpeg2 at {:#x}: {}", offset, message)),
      m_offset(offset) {}

namespace {

using Bytes = std::span<const std::uint8_t>;
using Block = std::array<std::int32_t, 64>;

template <typename... Args>
[[noreturn]] void error(std::size_t offset, std::format_string<Args...> fmt, Args&&... args) {
    throw VideoError(offset, std::format(fmt, std::forward<Args>(args)...));
}

// quantiser_scale for q_scale_type = 1 (Table 7-6); type 0 is 2 * code.
constexpr std::uint8_t kNonLinearQuantiser[32] = {
    0,  1,  2,  3,  4,  5,  6,  7,  8,  10, 12, 14, 16, 18, 20,  22,
    24, 28, 32, 36, 40, 44, 48, 52, 56, 64, 72, 80, 88, 96, 104, 112,
};

// The current picture's coding parameters.
struct PictureParams {
    PictureType type = PictureType::I;
    // f_code[s][t]: s 0 forward, 1 backward; t 0 horizontal, 1 vertical.
    std::array<std::array<std::uint8_t, 2>, 2> f_code{};
    std::uint8_t dc_precision = 0;
    bool q_scale_type = false;
    bool intra_vlc_format = false;
    bool frame_pred_frame_dct = true;
    const std::array<std::uint8_t, 64>* scan = &kZigZag;
};

// A macroblock's motion: frame prediction (mv[0][s]) or field prediction in
// a frame picture (mv[r][s] for the top (r = 0) and bottom (r = 1) field
// lines, from reference field sel[r][s]; vertical components in field lines).
// Vectors in half samples; s 0 forward, 1 backward.
struct Motion {
    bool field = false;
    std::array<std::array<std::array<int, 2>, 2>, 2> mv{};
    std::array<std::array<bool, 2>, 2> sel{};
};

using Predictors = std::array<std::array<std::array<int, 2>, 2>, 2>;

struct SliceState {
    int qscale = 0;
    std::array<int, 3> dc_pred{};
    Predictors pmv{};  // PMV[r][s][t]
    // The previous macroblock's motion flags (a skipped B macroblock repeats
    // its directions).
    std::int16_t prev_flags = 0;
};

// What a slice decodes into and from.
struct SliceContext {
    VideoFrame& frame;
    const PictureParams& p;
    const std::array<std::uint8_t, 64>& intra_q;
    const std::array<std::uint8_t, 64>& non_intra_q;
    const VideoFrame* fwd;
    const VideoFrame* bwd;
    VideoStats& stats;
};

// A macroblock's samples: luma 16x16, Cb and Cr 8x8, raster order.
struct MacroblockSamples {
    std::array<std::uint8_t, 256> y{};
    std::array<std::uint8_t, 64> cb{};
    std::array<std::uint8_t, 64> cr{};

    // Puts block b's IDCT output: intra blocks are the samples, others are
    // added to the prediction; clamped to 0..255. Luma blocks 0..3 cover the
    // quadrants (frame DCT) or, with field DCT, the left and right halves of
    // the top-field lines (blocks 0, 1) and of the bottom-field lines (2, 3).
    void put_block(int b, const Block& block, bool field_dct, bool intra) {
        std::uint8_t* plane = nullptr;
        std::size_t x0 = 0;
        std::size_t y0 = 0;
        std::size_t step = 1;
        std::size_t stride = 8;

        if (b < 4) {
            plane = y.data();
            stride = 16;
            x0 = static_cast<std::size_t>(b & 1) * 8;

            if (field_dct) {
                y0 = static_cast<std::size_t>(b >> 1);
                step = 2;
            } else {
                y0 = static_cast<std::size_t>(b >> 1) * 8;
            }
        } else {
            plane = b == 4 ? cb.data() : cr.data();
        }

        for (std::size_t i = 0; i < 8; ++i) {
            std::uint8_t* row = plane + (y0 + i * step) * stride + x0;

            for (std::size_t k = 0; k < 8; ++k) {
                const int v = block[i * 8 + k] + (intra ? 0 : row[k]);
                row[k] = static_cast<std::uint8_t>(std::clamp(v, 0, 255));
            }
        }
    }
};

int qscale(const PictureParams& p, int code) {
    return p.q_scale_type ? kNonLinearQuantiser[code] : 2 * code;
}

void store_macroblock(VideoFrame& frame, std::size_t addr, const MacroblockSamples& s) {
    const std::size_t mb_w = frame.width / 16;
    const std::size_t mx = addr % mb_w;
    const std::size_t my = addr / mb_w;
    const std::size_t w = frame.width;

    for (std::size_t y = 0; y < 16; ++y) {
        std::copy_n(s.y.data() + y * 16, 16, frame.y.data() + (my * 16 + y) * w + mx * 16);
    }

    const std::size_t cw = w / 2;

    for (std::size_t y = 0; y < 8; ++y) {
        std::copy_n(s.cb.data() + y * 8, 8, frame.cb.data() + (my * 8 + y) * cw + mx * 8);
        std::copy_n(s.cr.data() + y * 8, 8, frame.cr.data() + (my * 8 + y) * cw + mx * 8);
    }
}

// Half-sample prediction of a w x h block into `dst` (row stride
// dst_stride) whose top-left sample is (x, y) of a pw x ph plane (row k at
// src[off + k * stride]), displaced by (mvx, mvy) half samples. Samples
// outside the plane (never referenced by a valid stream) are clamped to its
// edge.
void predict(
    std::uint8_t* dst,
    std::size_t dst_stride,
    std::size_t w,
    std::size_t h,
    const std::uint8_t* src,
    std::size_t off,
    std::size_t stride,
    std::size_t pw,
    std::size_t ph,
    std::ptrdiff_t x,
    std::ptrdiff_t y,
    int mvx,
    int mvy
) {
    const std::ptrdiff_t ix = x + (mvx >> 1);
    const std::ptrdiff_t iy = y + (mvy >> 1);
    const std::size_t hx = static_cast<std::size_t>(mvx & 1);
    const std::size_t hy = static_cast<std::size_t>(mvy & 1);
    const bool inside = ix >= 0 && iy >= 0 && static_cast<std::size_t>(ix) + w + hx <= pw
                        && static_cast<std::size_t>(iy) + h + hy <= ph;

    if (inside) {
        for (std::size_t r = 0; r < h; ++r) {
            const std::uint8_t* row = src + off + (static_cast<std::size_t>(iy) + r) * stride
                                      + static_cast<std::size_t>(ix);
            const std::uint8_t* below = row + stride;
            std::uint8_t* d = dst + r * dst_stride;

            for (std::size_t k = 0; k < w; ++k) {
                unsigned v = 0;

                if (hx == 0 && hy == 0) {
                    v = row[k];
                } else if (hy == 0) {
                    v = (row[k] + row[k + 1] + 1u) >> 1;
                } else if (hx == 0) {
                    v = (row[k] + below[k] + 1u) >> 1;
                } else {
                    v = (row[k] + row[k + 1] + below[k] + below[k + 1] + 2u) >> 2;
                }

                d[k] = static_cast<std::uint8_t>(v);
            }
        }

        return;
    }

    auto at = [&](std::ptrdiff_t xx, std::ptrdiff_t yy) -> unsigned {
        const auto cy = std::clamp<std::ptrdiff_t>(yy, 0, static_cast<std::ptrdiff_t>(ph) - 1);
        const auto cx = std::clamp<std::ptrdiff_t>(xx, 0, static_cast<std::ptrdiff_t>(pw) - 1);
        return src[off + static_cast<std::size_t>(cy) * stride + static_cast<std::size_t>(cx)];
    };

    for (std::size_t r = 0; r < h; ++r) {
        for (std::size_t k = 0; k < w; ++k) {
            const std::ptrdiff_t sx = ix + static_cast<std::ptrdiff_t>(k);
            const std::ptrdiff_t sy = iy + static_cast<std::ptrdiff_t>(r);
            unsigned v = 0;

            if (hx == 0 && hy == 0) {
                v = at(sx, sy);
            } else if (hy == 0) {
                v = (at(sx, sy) + at(sx + 1, sy) + 1) >> 1;
            } else if (hx == 0) {
                v = (at(sx, sy) + at(sx, sy + 1) + 1) >> 1;
            } else {
                v = (at(sx, sy) + at(sx + 1, sy) + at(sx, sy + 1) + at(sx + 1, sy + 1) + 2) >> 2;
            }

            dst[r * dst_stride + k] = static_cast<std::uint8_t>(v);
        }
    }
}

// The prediction of direction s from `src` into `out`.
void predict_direction(
    MacroblockSamples& out,
    const VideoFrame& src,
    const Motion& m,
    std::size_t s,
    std::ptrdiff_t mx,
    std::ptrdiff_t my
) {
    const std::size_t w = src.width;
    const std::size_t h = src.height;
    const std::size_t cw = w / 2;
    const std::size_t ch = h / 2;

    if (!m.field) {
        const auto& mv = m.mv[0][s];
        predict(out.y.data(), 16, 16, 16, src.y.data(), 0, w, w, h, mx * 16, my * 16, mv[0], mv[1]);
        // Chroma vectors are the luma vectors halved toward zero (4:2:0).
        const int cx = mv[0] / 2;
        const int cy = mv[1] / 2;
        predict(out.cb.data(), 8, 8, 8, src.cb.data(), 0, cw, cw, ch, mx * 8, my * 8, cx, cy);
        predict(out.cr.data(), 8, 8, 8, src.cr.data(), 0, cw, cw, ch, mx * 8, my * 8, cx, cy);
        return;
    }

    // Field prediction in a frame picture: field r's lines (r, r + 2, ...)
    // from reference field sel[r][s].
    for (std::size_t fr = 0; fr < 2; ++fr) {
        const auto& mv = m.mv[fr][s];
        const std::size_t sel = m.sel[fr][s] ? 1 : 0;
        predict(
            out.y.data() + fr * 16,
            32,
            16,
            8,
            src.y.data(),
            sel * w,
            2 * w,
            w,
            h / 2,
            mx * 16,
            my * 8,
            mv[0],
            mv[1]
        );
        const int cx = mv[0] / 2;
        const int cy = mv[1] / 2;
        predict(
            out.cb.data() + fr * 8,
            16,
            8,
            4,
            src.cb.data(),
            sel * cw,
            2 * cw,
            cw,
            ch / 2,
            mx * 8,
            my * 4,
            cx,
            cy
        );
        predict(
            out.cr.data() + fr * 8,
            16,
            8,
            4,
            src.cr.data(),
            sel * cw,
            2 * cw,
            cw,
            ch / 2,
            mx * 8,
            my * 4,
            cx,
            cy
        );
    }
}

// Forward only, backward only, or the rounded average of both.
void predict_macroblock(
    MacroblockSamples& pred,
    const VideoFrame& frame,
    const VideoFrame* fwd,
    const VideoFrame* bwd,
    std::size_t addr,
    const Motion& m
) {
    const std::size_t mb_w = frame.width / 16;
    const auto mx = static_cast<std::ptrdiff_t>(addr % mb_w);
    const auto my = static_cast<std::ptrdiff_t>(addr / mb_w);

    if (fwd && bwd) {
        MacroblockSamples back;
        predict_direction(pred, *fwd, m, 0, mx, my);
        predict_direction(back, *bwd, m, 1, mx, my);
        auto average = [](auto& d, const auto& s) {
            for (std::size_t k = 0; k < d.size(); ++k) {
                d[k] = static_cast<std::uint8_t>((d[k] + s[k] + 1u) >> 1);
            }
        };
        average(pred.y, back.y);
        average(pred.cb, back.cb);
        average(pred.cr, back.cr);
    } else if (fwd) {
        predict_direction(pred, *fwd, m, 0, mx, my);
    } else if (bwd) {
        predict_direction(pred, *bwd, m, 1, mx, my);
    } else {
        // A missing reference (not possible in a decodable picture): mid grey.
        pred.y.fill(128);
        pred.cb.fill(128);
        pred.cr.fill(128);
    }
}

// One motion_vector(r, s): horizontal then vertical, each predicted from
// and stored to `pmv` (7.6.3.1). A field vector in a frame picture predicts
// its vertical component from PMV >> 1 and stores it back doubled (the
// predictors stay in frame units).
std::array<int, 2> motion_vector(
    BitReader& r,
    std::array<std::uint8_t, 2> f_code,
    std::array<int, 2>& pmv,
    bool field,
    std::size_t at
) {
    const Mpeg2VlcTables& t = mpeg2_vlc_tables();
    std::array<int, 2> out{};

    for (std::size_t c = 0; c < 2; ++c) {
        const int fc = f_code[c];

        if (fc < 1 || fc > 9) {
            error(at, "f_code {}", fc);
        }

        const auto magnitude = t.motion_code.decode(r);

        if (!magnitude) {
            error(at, "bad motion_code");
        }

        const int m = *magnitude;
        const int code = m != 0 && r.bit() ? -m : m;
        const auto r_size = static_cast<unsigned>(fc - 1);
        const int f = 1 << r_size;
        int delta = code;

        if (f != 1 && code != 0) {
            const int residual = static_cast<int>(r.read(r_size));
            const int d = (std::abs(code) - 1) * f + residual + 1;
            delta = code < 0 ? -d : d;
        }

        const bool halve = field && c == 1;
        const int prediction = halve ? pmv[c] >> 1 : pmv[c];
        const int low = -16 * f;
        const int high = 16 * f - 1;
        const int range = 32 * f;
        int v = prediction + delta;

        if (v < low) {
            v += range;
        }

        if (v > high) {
            v -= range;
        }

        pmv[c] = halve ? v * 2 : v;
        out[c] = v;
    }

    return out;
}

// One (run, signed level), or none at the end of the block.
std::optional<std::pair<std::size_t, int>> coefficient(
    BitReader& r, const VlcTable& table, std::size_t at
) {
    const auto v = table.decode(r);

    if (!v) {
        error(at, "bad DCT coefficient code {:016b}", r.peek(16));
    }

    if (*v == kDctEndOfBlock) {
        return std::nullopt;
    }

    if (*v == kDctEscape) {
        const std::size_t run = r.read(6);
        const int raw = static_cast<int>(r.read(12));
        const int level = raw >= 2048 ? raw - 4096 : raw;

        if (level == 0) {
            error(at, "escape level 0");
        }

        return std::pair{run, level};
    }

    const std::size_t run = static_cast<std::size_t>(*v >> 8);
    const int level = *v & 0xff;
    return std::pair{run, r.bit() ? -level : level};
}

// Saturation and mismatch control (7.4.3, 7.4.4) over a dequantised block.
void mismatch_control(Block& block, int sum) {
    if ((sum & 1) == 0) {
        block[63] ^= 1;
    }
}

void intra_block(
    BitReader& r,
    const PictureParams& p,
    const std::array<std::uint8_t, 64>& q,
    SliceState& st,
    int b,
    Block& block
) {
    const Mpeg2VlcTables& t = mpeg2_vlc_tables();
    const std::size_t at = r.byte_pos();
    const std::size_t comp = b < 4 ? 0 : static_cast<std::size_t>(b - 3);
    const auto size_code =
        comp == 0 ? t.dc_size_luminance.decode(r) : t.dc_size_chrominance.decode(r);

    if (!size_code) {
        error(at, "bad dct_dc_size");
    }

    const auto size = static_cast<unsigned>(*size_code);
    int diff = 0;

    if (size != 0) {
        const int v = static_cast<int>(r.read(size));
        diff = v < (1 << (size - 1)) ? v - (1 << size) + 1 : v;
    }

    const int dc = st.dc_pred[comp] + diff;
    st.dc_pred[comp] = dc;
    const int f0 = std::clamp(dc * (8 >> p.dc_precision), -2048, 2047);
    block[0] = f0;
    int sum = f0;
    const VlcTable& table = p.intra_vlc_format ? t.dct_one : t.dct_zero;
    std::size_t i = 1;

    while (const auto c = coefficient(r, table, at)) {
        i += c->first;

        if (i >= 64) {
            error(at, "intra block coefficient index past 63");
        }

        const std::size_t pos = (*p.scan)[i];
        const int v = std::clamp((c->second * q[pos] * st.qscale) / 16, -2048, 2047);
        block[pos] = v;
        sum += v;
        ++i;
    }

    mismatch_control(block, sum);
}

int non_intra_value(int level, int q, int qscale) {
    const int sign = level > 0 ? 1 : (level < 0 ? -1 : 0);
    return std::clamp(((2 * level + sign) * q * qscale) / 32, -2048, 2047);
}

void non_intra_block(
    BitReader& r,
    const std::array<std::uint8_t, 64>& scan,
    const std::array<std::uint8_t, 64>& q,
    int qs,
    Block& block,
    std::size_t at
) {
    const Mpeg2VlcTables& t = mpeg2_vlc_tables();
    int sum = 0;
    std::size_t i = 0;

    // The first coefficient: "1s" is (0, +-1).
    if (r.peek(1) == 1) {
        r.skip(1);
        const int level = r.bit() ? -1 : 1;
        const int v = non_intra_value(level, q[0], qs);
        block[0] = v;
        sum += v;
        i = 1;
    }

    while (const auto c = coefficient(r, t.dct_zero, at)) {
        i += c->first;

        if (i >= 64) {
            error(at, "non-intra block coefficient index past 63");
        }

        const std::size_t pos = scan[i];
        const int v = non_intra_value(c->second, q[pos], qs);
        block[pos] = v;
        sum += v;
        ++i;
    }

    mismatch_control(block, sum);
}

// A skipped macroblock: P = forward frame prediction with a zero vector
// (vector predictors reset); B = the previous macroblock's directions with
// frame prediction from the vector predictors PMV[0][s] (7.6.6.4: also after
// a field-predicted macroblock). The DC predictors are reset.
void skipped_macroblock(SliceContext& ctx, SliceState& st, std::size_t addr, std::size_t at) {
    const int dc_reset = 1 << (7 + ctx.p.dc_precision);
    st.dc_pred = {dc_reset, dc_reset, dc_reset};
    MacroblockSamples pred;

    switch (ctx.p.type) {
        case PictureType::I:
            error(at, "skipped macroblock in an I picture");
        case PictureType::P:
            st.pmv = {};
            predict_macroblock(pred, ctx.frame, ctx.fwd, nullptr, addr, Motion{});
            break;
        case PictureType::B: {
            const std::int16_t f = st.prev_flags;

            if ((f & (kMbForward | kMbBackward)) == 0) {
                error(at, "skipped B macroblock after an intra macroblock");
            }

            Motion m;
            m.mv[0][0] = st.pmv[0][0];
            m.mv[0][1] = st.pmv[0][1];
            predict_macroblock(
                pred,
                ctx.frame,
                (f & kMbForward) ? ctx.fwd : nullptr,
                (f & kMbBackward) ? ctx.bwd : nullptr,
                addr,
                m
            );
            break;
        }
    }

    store_macroblock(ctx.frame, addr, pred);
}

void macroblock(SliceContext& ctx, SliceState& st, BitReader& r, std::size_t addr) {
    const Mpeg2VlcTables& t = mpeg2_vlc_tables();
    const PictureParams& p = ctx.p;
    const std::size_t at = r.byte_pos();
    const auto type_code = t.macroblock_type[static_cast<int>(p.type)].decode(r);

    if (!type_code) {
        error(at, "bad macroblock_type in a {} picture", "IPB"[static_cast<int>(p.type)]);
    }

    const std::int16_t flags = *type_code;
    // macroblock_modes(): frame_motion_type and dct_type exist only when
    // frame_pred_frame_dct = 0.
    bool field_motion = false;

    if ((flags & (kMbForward | kMbBackward)) && !p.frame_pred_frame_dct) {
        switch (r.read(2)) {
            case 1:
                field_motion = true;
                break;
            case 2:
                break;
            case 3:
                error(at, "dual-prime prediction is not supported");
            default:
                error(at, "reserved frame_motion_type 0");
        }
    }

    const bool field_dct = (flags & (kMbIntra | kMbPattern)) && !p.frame_pred_frame_dct && r.bit();
    ctx.stats.field_motion_macroblocks += field_motion;
    ctx.stats.field_dct_macroblocks += field_dct;

    if (flags & kMbQuant) {
        const int code = static_cast<int>(r.read(5));

        if (code == 0) {
            error(at, "quantiser_scale_code 0");
        }

        st.qscale = qscale(p, code);
    }

    const int dc_reset = 1 << (7 + p.dc_precision);
    MacroblockSamples pred;

    if (flags & kMbIntra) {
        st.pmv = {};
        st.prev_flags = kMbIntra;

        for (int b = 0; b < 6; ++b) {
            Block block{};
            intra_block(r, p, ctx.intra_q, st, b, block);
            inverse_dct(block);
            ++ctx.stats.coded_blocks;
            pred.put_block(b, block, field_dct, true);
        }

        store_macroblock(ctx.frame, addr, pred);
        return;
    }

    st.dc_pred = {dc_reset, dc_reset, dc_reset};
    Motion motion;
    motion.field = field_motion;

    for (std::size_t s = 0; s < 2; ++s) {
        if ((flags & (s == 0 ? kMbForward : kMbBackward)) == 0) {
            continue;
        }

        if (field_motion) {
            for (std::size_t fr = 0; fr < 2; ++fr) {
                motion.sel[fr][s] = r.bit();
                motion.mv[fr][s] = motion_vector(r, p.f_code[s], st.pmv[fr][s], true, at);
            }
        } else {
            motion.mv[0][s] = motion_vector(r, p.f_code[s], st.pmv[0][s], false, at);
            st.pmv[1][s] = st.pmv[0][s];
        }
    }

    // A P picture's "No MC": forward frame prediction with a zero vector,
    // the predictors reset.
    bool use_forward = (flags & kMbForward) != 0;
    bool use_backward = (flags & kMbBackward) != 0;

    if (p.type == PictureType::P) {
        if (!use_forward) {
            st.pmv = {};
            motion = Motion{};
        }

        use_forward = true;
        use_backward = false;
    }

    st.prev_flags = flags;
    unsigned cbp = 0;

    if (flags & kMbPattern) {
        const auto v = t.coded_block_pattern.decode(r);

        if (!v) {
            error(at, "bad coded_block_pattern");
        }

        if (*v == 0) {
            error(at, "coded_block_pattern 0 (the 4:2:2 code) in 4:2:0");
        }

        cbp = static_cast<unsigned>(*v);
    }

    predict_macroblock(
        pred,
        ctx.frame,
        use_forward ? ctx.fwd : nullptr,
        use_backward ? ctx.bwd : nullptr,
        addr,
        motion
    );

    for (int b = 0; b < 6; ++b) {
        if ((cbp & (32u >> b)) == 0) {
            continue;
        }

        Block block{};
        non_intra_block(r, *p.scan, ctx.non_intra_q, st.qscale, block, at);
        inverse_dct(block);
        ++ctx.stats.coded_blocks;
        pred.put_block(b, block, field_dct, false);
    }

    store_macroblock(ctx.frame, addr, pred);
}

// One slice: `data` ends at the next start code; the slice's start code is at `sc`.
void decode_slice(SliceContext& ctx, Bytes data, std::size_t sc, std::size_t row) {
    const Mpeg2VlcTables& t = mpeg2_vlc_tables();
    const std::size_t mb_w = ctx.frame.width / 16;
    BitReader r(data, sc + 4);
    const int code = static_cast<int>(r.read(5));

    if (code == 0) {
        error(sc, "quantiser_scale_code 0");
    }

    if (r.bit()) {
        r.skip(1 + 7);

        while (r.bit()) {
            r.skip(8);
        }
    }

    const int dc_reset = 1 << (7 + ctx.p.dc_precision);
    SliceState st;
    st.qscale = qscale(ctx.p, code);
    st.dc_pred = {dc_reset, dc_reset, dc_reset};
    auto addr = static_cast<std::ptrdiff_t>(row * mb_w) - 1;
    bool first = true;
    const auto count = static_cast<std::ptrdiff_t>(mb_w * ctx.frame.height / 16);

    for (;;) {
        const std::size_t at = r.byte_pos();
        std::ptrdiff_t increment = 0;

        for (;;) {
            const auto v = t.macroblock_address.decode(r);

            if (!v) {
                error(at, "bad macroblock_address_increment");
            }

            if (*v == kMbaEscape) {
                increment += 33;
            } else if (*v != kMbaStuffing) {
                increment += *v;
                break;
            }
        }

        if (!first) {
            for (std::ptrdiff_t k = 1; k < increment; ++k) {
                const std::ptrdiff_t a = addr + k;

                if (a >= count) {
                    error(at, "skipped macroblocks past the picture");
                }

                skipped_macroblock(ctx, st, static_cast<std::size_t>(a), at);
                ++ctx.stats.skipped_macroblocks;
            }
        }

        first = false;
        addr += increment;

        if (addr >= count) {
            error(at, "macroblock address {} past the picture", addr);
        }

        macroblock(ctx, st, r, static_cast<std::size_t>(addr));
        ++ctx.stats.macroblocks;

        if (r.overrun()) {
            error(r.byte_pos(), "slice data runs past the next start code");
        }

        if (r.at_start_code()) {
            break;
        }
    }
}

std::shared_ptr<VideoFrame> new_frame(
    std::size_t w, std::size_t h, PictureType type, std::uint16_t tr, std::size_t index
) {
    auto f = std::make_shared<VideoFrame>();
    f->width = w;
    f->height = h;
    f->y.assign(w * h, 0);
    f->cb.assign(w * h / 4, 128);
    f->cr.assign(w * h / 4, 128);
    f->type = type;
    f->temporal_reference = tr;
    f->decode_index = index;
    return f;
}

}  // namespace

Mpeg2Decoder::Mpeg2Decoder(std::vector<std::uint8_t> elementary_stream)
    : m_data(std::move(elementary_stream)),
      m_intra_matrix(kDefaultIntraMatrix) {
    m_non_intra_matrix.fill(16);
}

int Mpeg2Decoder::code_at(std::size_t sc) const {
    return sc + 3 < m_data.size() ? m_data[sc + 3] : -1;
}

std::optional<VideoSequence> Mpeg2Decoder::sequence() {
    while (!m_sequence && !m_ended) {
        if (!step()) {
            break;
        }
    }

    // The sequence extension and user data that follow the header.
    while (m_sequence) {
        const auto sc = next_start_code(m_data, m_pos);

        if (!sc) {
            break;
        }

        const int code = code_at(*sc);

        if (code != 0xb5 && code != 0xb2) {
            break;
        }

        step();
    }

    return m_sequence;
}

std::shared_ptr<const VideoFrame> Mpeg2Decoder::next_frame() {
    for (;;) {
        if (!m_out.empty()) {
            auto f = std::move(m_out.front());
            m_out.pop_front();
            ++m_stats.frames_out;
            return f;
        }

        if (m_ended) {
            return nullptr;
        }

        step();
    }
}

bool Mpeg2Decoder::step() {
    const auto sc = next_start_code(m_data, m_pos);

    if (!sc || code_at(*sc) < 0) {
        if (m_held) {
            m_out.push_back(std::move(m_held));
            m_held = nullptr;
        }

        m_ended = true;
        m_pos = m_data.size();
        return false;
    }

    const int code = code_at(*sc);
    m_pos = *sc + 4;

    switch (code) {
        case 0xb3:
            sequence_header(*sc);
            break;
        case 0xb5:
            extension(*sc);
            break;
        case 0xb8: {
            BitReader r(m_data, *sc + 4);
            r.skip(25);
            ++m_stats.gops;
            m_stats.closed_gops += r.bit();
            break;
        }
        case 0x00:
            m_pos = picture(*sc);
            break;
        case 0xb7:
            // sequence_end_code: the held reference frame is displayed.
            if (m_held) {
                m_out.push_back(std::move(m_held));
                m_held = nullptr;
            }

            break;
        default:
            break;
    }

    return true;
}

void Mpeg2Decoder::sequence_header(std::size_t sc) {
    BitReader r(m_data, sc + 4);
    VideoSequence s;
    s.width = r.read(12);
    s.height = r.read(12);
    s.aspect_ratio = static_cast<std::uint8_t>(r.read(4));
    s.frame_rate_code = static_cast<std::uint8_t>(r.read(4));
    s.bit_rate = r.read(18);
    r.skip(1 + 10 + 1);
    s.load_intra = r.bit();
    m_intra_matrix = kDefaultIntraMatrix;

    if (s.load_intra) {
        for (const std::uint8_t z : kZigZag) {
            m_intra_matrix[z] = static_cast<std::uint8_t>(r.read(8));
        }
    }

    s.load_non_intra = r.bit();
    m_non_intra_matrix.fill(16);

    if (s.load_non_intra) {
        for (const std::uint8_t z : kZigZag) {
            m_non_intra_matrix[z] = static_cast<std::uint8_t>(r.read(8));
        }
    }

    if (r.overrun()) {
        error(sc, "truncated sequence header");
    }

    static constexpr std::uint32_t kRates[9][2] = {
        {0, 0},
        {24000, 1001},
        {24, 1},
        {25, 1},
        {30000, 1001},
        {30, 1},
        {50, 1},
        {60000, 1001},
        {60, 1},
    };

    if (s.frame_rate_code < 1 || s.frame_rate_code > 8) {
        error(sc, "frame_rate_code {}", s.frame_rate_code);
    }

    s.fps_num = kRates[s.frame_rate_code][0];
    s.fps_den = kRates[s.frame_rate_code][1];

    if (s.width == 0 || s.height == 0 || s.width % 16 != 0 || s.height % 16 != 0 || s.width > 1920
        || s.height > 1152) {
        error(sc, "unsupported picture size {}x{}", s.width, s.height);
    }

    if (m_sequence) {
        if (m_sequence->width != s.width || m_sequence->height != s.height) {
            error(
                sc,
                "the picture size changes from {}x{} to {}x{}",
                m_sequence->width,
                m_sequence->height,
                s.width,
                s.height
            );
        }

        s.profile_level = m_sequence->profile_level;
        s.progressive = m_sequence->progressive;
        s.chroma_format = m_sequence->chroma_format;
    }

    m_sequence = s;
    ++m_stats.sequences;
}

// Extensions outside a picture: the sequence extension (others are ignored).
void Mpeg2Decoder::extension(std::size_t sc) {
    BitReader r(m_data, sc + 4);

    if (r.read(4) != 1) {
        return;
    }

    if (!m_sequence) {
        error(sc, "a sequence extension before a sequence header");
    }

    VideoSequence& s = *m_sequence;
    s.profile_level = static_cast<std::uint8_t>(r.read(8));
    s.progressive = r.bit();
    s.chroma_format = static_cast<std::uint8_t>(r.read(2));
    const std::uint32_t horizontal_ext = r.read(2);
    const std::uint32_t vertical_ext = r.read(2);
    r.skip(12 + 1 + 8 + 1);
    const std::uint32_t n = r.read(2);
    const std::uint32_t d = r.read(5);

    if (horizontal_ext != 0 || vertical_ext != 0) {
        error(sc, "the size extension bits are set");
    }

    if (s.chroma_format != 1) {
        error(sc, "chroma_format {} (only 4:2:0 is supported)", s.chroma_format);
    }

    s.fps_num *= n + 1;
    s.fps_den *= d + 1;
}

// Decodes the picture whose header starts at `sc`; returns the offset of the
// start code after its slices.
std::size_t Mpeg2Decoder::picture(std::size_t sc) {
    if (!m_sequence) {
        error(sc, "a picture before a sequence header");
    }

    const VideoSequence seq = *m_sequence;
    BitReader r(m_data, sc + 4);
    const auto temporal_reference = static_cast<std::uint16_t>(r.read(10));
    PictureParams params;

    switch (r.read(3)) {
        case 1:
            params.type = PictureType::I;
            break;
        case 2:
            params.type = PictureType::P;
            break;
        case 3:
            params.type = PictureType::B;
            break;
        default:
            error(sc, "unsupported picture_coding_type");
    }

    // The MPEG-1 fields (vbv_delay, full_pel_*, f_codes) are superseded by
    // the picture coding extension.
    for (auto& fs : params.f_code) {
        fs = {15, 15};
    }

    bool have_extension = false;
    std::size_t pos = sc + 4;
    std::size_t first_slice = 0;

    // Extensions and user data up to the first slice.
    for (;;) {
        const auto s = next_start_code(m_data, pos);

        if (!s || code_at(*s) < 0) {
            error(sc, "a picture without slices");
        }

        const int code = code_at(*s);
        pos = *s + 4;

        if (code == 0xb5) {
            BitReader e(m_data, *s + 4);
            const std::uint32_t id = e.read(4);

            if (id == 8) {
                for (auto& fs : params.f_code) {
                    for (auto& ft : fs) {
                        ft = static_cast<std::uint8_t>(e.read(4));
                    }
                }

                params.dc_precision = static_cast<std::uint8_t>(e.read(2));
                const std::uint32_t structure = e.read(2);
                const bool top_field_first = e.bit();
                const bool frame_pred_frame_dct = e.bit();
                const bool concealment = e.bit();
                params.q_scale_type = e.bit();
                params.intra_vlc_format = e.bit();
                const bool alternate_scan = e.bit();
                e.skip(1);  // repeat_first_field
                e.skip(1);  // chroma_420_type
                const bool progressive_frame = e.bit();

                if (structure != 3) {
                    error(
                        *s, "picture_structure {} (only frame pictures are supported)", structure
                    );
                }

                if (concealment) {
                    error(*s, "concealment motion vectors are not supported");
                }

                if (params.dc_precision == 3) {
                    error(*s, "intra_dc_precision 11 bits is not allowed at Main Profile");
                }

                params.frame_pred_frame_dct = frame_pred_frame_dct;
                params.scan = alternate_scan ? &kAlternateScan : &kZigZag;
                ++m_stats.dc_precision[params.dc_precision];
                ++m_stats.q_scale_type[params.q_scale_type];
                ++m_stats.intra_vlc_format[params.intra_vlc_format];
                ++m_stats.top_field_first[top_field_first];
                ++m_stats.frame_pred_frame_dct[frame_pred_frame_dct];
                ++m_stats.alternate_scan[alternate_scan];
                ++m_stats.progressive_frame[progressive_frame];
                have_extension = true;
            } else if (id == 3) {
                // quant_matrix_extension (the luma matrices; the chroma ones
                // are for 4:2:2 and 4:4:4).
                if (e.bit()) {
                    for (const std::uint8_t z : kZigZag) {
                        m_intra_matrix[z] = static_cast<std::uint8_t>(e.read(8));
                    }
                }

                if (e.bit()) {
                    for (const std::uint8_t z : kZigZag) {
                        m_non_intra_matrix[z] = static_cast<std::uint8_t>(e.read(8));
                    }
                }
            }
        } else if (code >= 0x01 && code <= 0xaf) {
            first_slice = *s;
            break;
        } else if (code != 0xb2) {
            error(*s, "start code {:#x} before the picture's first slice", code);
        }
    }

    if (!have_extension) {
        error(sc, "an MPEG-1 picture (no picture coding extension)");
    }

    const std::size_t decode_index =
        m_stats.pictures[0] + m_stats.pictures[1] + m_stats.pictures[2] + m_stats.skipped_b;
    const std::size_t w = seq.width;
    const std::size_t h = seq.height;
    std::shared_ptr<const VideoFrame> fwd;
    std::shared_ptr<const VideoFrame> bwd;

    if (params.type == PictureType::P) {
        if (!m_refs[1]) {
            error(sc, "a P picture without a reference frame");
        }

        fwd = m_refs[1];
    } else if (params.type == PictureType::B) {
        fwd = m_refs[0];
        bwd = m_refs[1];
    }

    // Find the end of the slices either way: a skipped B picture still has
    // to be stepped over.
    auto frame = new_frame(w, h, params.type, temporal_reference, decode_index);
    const bool skip = params.type == PictureType::B && (!fwd || !bwd);
    SliceContext ctx{
        *frame, params, m_intra_matrix, m_non_intra_matrix, fwd.get(), bwd.get(), m_stats
    };
    std::size_t s = first_slice;
    std::size_t end = 0;

    for (;;) {
        const int code = code_at(s);

        if (code < 0x01 || code > 0xaf) {
            end = s;
            break;
        }

        const std::size_t next = next_start_code(m_data, s + 4).value_or(m_data.size());

        if (!skip) {
            const auto row = static_cast<std::size_t>(code - 1);

            if (row >= h / 16) {
                error(s, "slice row {} outside the picture", row);
            }

            decode_slice(ctx, Bytes(m_data.data(), next), s, row);
        }

        if (next >= m_data.size()) {
            end = m_data.size();
            break;
        }

        s = next;
    }

    if (skip) {
        ++m_stats.skipped_b;
        return end;
    }

    ++m_stats.pictures[static_cast<std::size_t>(params.type)];

    if (params.type == PictureType::B) {
        m_out.push_back(std::move(frame));
    } else {
        if (m_held) {
            m_out.push_back(std::move(m_held));
        }

        m_held = frame;
        m_refs[0] = std::move(m_refs[1]);
        m_refs[1] = std::move(frame);
    }

    return end;
}

}  // namespace openrac::media
