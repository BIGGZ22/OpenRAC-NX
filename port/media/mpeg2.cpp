// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-video/src/mpeg2.rs:
// ISC License, Copyright (c) 2026 ReRAC contributors.
//
// The MPEG-2 video decoder (mpeg2.h). Section numbers are ISO/IEC 13818-2's, the public
// standard, which is this file's only source.
//
// Decoding goes from the outside in. Mpeg2Decoder walks the stream's start codes and reads the
// sequence, group and picture headers; each slice is then decoded macroblock by macroblock. A
// macroblock is a prediction (from a reference frame, or none when it is intra) plus six 8x8
// blocks of dequantised, inverse-transformed coefficients. The sizes are those of 4:2:0: a
// macroblock is 16x16 luma samples and two 8x8 chroma blocks, and a block has 64 coefficients.
// The 8, 16 and 64 below are those sizes.
//
// A stream the decoder does not support or cannot parse throws VideoError with the byte offset.

#include "media/mpeg2.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <format>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <utility>
#include <vector>

#include "media/bit_reader.h"
#include "media/idct.h"
#include "media/vlc.h"

namespace openrac::media {

// ---- Scan and matrix tables ----

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

// Start codes (the byte after `00 00 01`, Table 6-1) and extension ids (Table 6-2).
constexpr int kPictureStartCode = 0x00;
constexpr int kFirstSliceStartCode = 0x01;
constexpr int kLastSliceStartCode = 0xaf;
constexpr int kUserDataStartCode = 0xb2;
constexpr int kSequenceHeaderStartCode = 0xb3;
constexpr int kExtensionStartCode = 0xb5;
constexpr int kSequenceEndStartCode = 0xb7;
constexpr int kGroupStartCode = 0xb8;
constexpr std::uint32_t kSequenceExtensionId = 1;
constexpr std::uint32_t kQuantMatrixExtensionId = 3;
constexpr std::uint32_t kPictureCodingExtensionId = 8;

// A read-only view of bytes.
using Bytes = std::span<const std::uint8_t>;
// One 8x8 block of coefficients or samples, in raster order.
using Block = std::array<std::int32_t, 64>;

// Throws a VideoError for the byte at `offset`, with a message formatted like std::format.
// Never returns.
template <typename... Args>
[[noreturn]] void error(std::size_t offset, std::format_string<Args...> fmt, Args&&... args) {
    throw VideoError(offset, std::format(fmt, std::forward<Args>(args)...));
}

// ---- Parameters and state ----

// quantiser_scale for q_scale_type = 1 (Table 7-6); type 0 is 2 * code.
constexpr std::uint8_t kNonLinearQuantiser[32] = {
    0,  1,  2,  3,  4,  5,  6,  7,  8,  10, 12, 14, 16, 18, 20,  22,
    24, 28, 32, 36, 40, 44, 48, 52, 56, 64, 72, 80, 88, 96, 104, 112,
};

// The current picture's coding parameters.
struct PictureParams {
    PictureType type = PictureType::I;  // I, P or B.
    // f_code[s][t]: s 0 forward, 1 backward; t 0 horizontal, 1 vertical.
    std::array<std::array<std::uint8_t, 2>, 2> f_code{};
    std::uint8_t dc_precision = 0;     // intra_dc_precision: 0 to 2 for 8 to 10 bits.
    bool q_scale_type = false;         // True selects kNonLinearQuantiser.
    bool intra_vlc_format = false;     // True: DCT table B-15 for intra blocks, else B-14.
    bool frame_pred_frame_dct = true;  // True: frame prediction and frame DCT only.
    const std::array<std::uint8_t, 64>* scan = &kZigZag;  // The zig-zag or the alternate scan.
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

// What carries from one macroblock of a slice to the next.
struct SliceState {
    int qscale = 0;                // The quantiser_scale in force.
    std::array<int, 3> dc_pred{};  // The DC predictors of Y, Cb and Cr.
    Predictors pmv{};              // PMV[r][s][t]
    // The previous macroblock's motion flags (a skipped B macroblock repeats
    // its directions).
    std::int16_t prev_flags = 0;
};

// What a slice decodes into and from.
struct SliceContext {
    VideoFrame& frame;                                // The frame being decoded.
    const PictureParams& p;                           // The picture's parameters.
    const std::array<std::uint8_t, 64>& intra_q;      // The quantiser matrix for intra blocks.
    const std::array<std::uint8_t, 64>& non_intra_q;  // The matrix for the other blocks.
    const VideoFrame* fwd;                            // The forward reference, or null.
    const VideoFrame* bwd;                            // The backward reference, or null.
    VideoStats& stats;                                // Where the slice counts what it decodes.
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

        // Luma: blocks 0 to 3 are the four 8x8 quadrants of the 16x16 luma plane.
        if (b < 4) {
            plane = y.data();
            stride = 16;
            x0 = static_cast<std::size_t>(b & 1) * 8;

            // Field DCT: blocks 0 and 1 hold the top field's lines, 2 and 3 the bottom field's.
            if (field_dct) {
                y0 = static_cast<std::size_t>(b >> 1);
                step = 2;
            } else {
                // Frame DCT: blocks 2 and 3 are the lower 8 rows.
                y0 = static_cast<std::size_t>(b >> 1) * 8;
            }
        } else {
            // Chroma: block 4 is Cb and block 5 is Cr, each an 8x8 plane.
            plane = b == 4 ? cb.data() : cr.data();
        }

        // One row of the block at a time.
        for (std::size_t i = 0; i < 8; ++i) {
            std::uint8_t* row = plane + (y0 + i * step) * stride + x0;

            // An intra block is stored as it is; any other is added to the prediction.
            for (std::size_t k = 0; k < 8; ++k) {
                const int v = block[i * 8 + k] + (intra ? 0 : row[k]);
                row[k] = static_cast<std::uint8_t>(std::clamp(v, 0, 255));
            }
        }
    }
};

// ---- Reconstruction and prediction ----

// The quantiser_scale for a quantiser_scale_code from 1 to 31 (Table 7-6).
int qscale(const PictureParams& p, int code) {
    return p.q_scale_type ? kNonLinearQuantiser[code] : 2 * code;
}

// Copies a macroblock into `frame` at macroblock address `addr`, counted in raster order.
void store_macroblock(VideoFrame& frame, std::size_t addr, const MacroblockSamples& s) {
    const std::size_t mb_w = frame.width / 16;
    const std::size_t mx = addr % mb_w;
    const std::size_t my = addr / mb_w;
    const std::size_t w = frame.width;

    // Sixteen rows of luma.
    for (std::size_t y = 0; y < 16; ++y) {
        std::copy_n(s.y.data() + y * 16, 16, frame.y.data() + (my * 16 + y) * w + mx * 16);
    }

    // The chroma planes are half the width of the luma plane.
    const std::size_t cw = w / 2;

    // Eight rows of each chroma plane.
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
    // The vector is in half samples: its upper bits are whole samples, its low bit a half sample.
    const std::ptrdiff_t ix = x + (mvx >> 1);
    const std::ptrdiff_t iy = y + (mvy >> 1);
    const std::size_t hx = static_cast<std::size_t>(mvx & 1);
    const std::size_t hy = static_cast<std::size_t>(mvy & 1);
    // The block plus the extra sample that half-sample averaging reads must lie within the plane.
    const bool inside = ix >= 0 && iy >= 0 && static_cast<std::size_t>(ix) + w + hx <= pw
                        && static_cast<std::size_t>(iy) + h + hy <= ph;

    // Fast path: every sample needed is inside the plane, so none is clamped.
    if (inside) {
        // One row of the block at a time.
        for (std::size_t r = 0; r < h; ++r) {
            const std::uint8_t* row = src + off + (static_cast<std::size_t>(iy) + r) * stride
                                      + static_cast<std::size_t>(ix);
            const std::uint8_t* below = row + stride;
            std::uint8_t* d = dst + r * dst_stride;

            // One sample of the row.
            for (std::size_t k = 0; k < w; ++k) {
                unsigned v = 0;

                // Whole-sample vectors copy; half-sample ones average the
                // neighbours, rounding up (7.6.4).
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

    // A sample of the plane, with the position clamped to the plane's edge.
    auto at = [&](std::ptrdiff_t xx, std::ptrdiff_t yy) -> unsigned {
        const auto cy = std::clamp<std::ptrdiff_t>(yy, 0, static_cast<std::ptrdiff_t>(ph) - 1);
        const auto cx = std::clamp<std::ptrdiff_t>(xx, 0, static_cast<std::ptrdiff_t>(pw) - 1);
        return src[off + static_cast<std::size_t>(cy) * stride + static_cast<std::size_t>(cx)];
    };

    // Slow path: the block reaches outside the plane, so each sample is fetched through `at`.
    for (std::size_t r = 0; r < h; ++r) {
        // One sample of the row.
        for (std::size_t k = 0; k < w; ++k) {
            const std::ptrdiff_t sx = ix + static_cast<std::ptrdiff_t>(k);
            const std::ptrdiff_t sy = iy + static_cast<std::ptrdiff_t>(r);
            unsigned v = 0;

            // The same four cases as above.
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

// The prediction of direction `s` (0 forward, 1 backward) from `src` into `out`, for the
// macroblock at (mx, my) in macroblock units.
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

    // Frame prediction: one 16x16 luma block and two 8x8 chroma blocks.
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
        // The reference field the lines are taken from: 0 is the top field, 1 the bottom.
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

// Predicts the macroblock at `addr` into `pred`: forward only, backward only, or the rounded
// average of both. `fwd` or `bwd` is null when that reference is absent.
void predict_macroblock(
    MacroblockSamples& pred,
    const VideoFrame& frame,
    const VideoFrame* fwd,
    const VideoFrame* bwd,
    std::size_t addr,
    const Motion& m
) {
    // The macroblock's position in macroblock units.
    const std::size_t mb_w = frame.width / 16;
    const auto mx = static_cast<std::ptrdiff_t>(addr % mb_w);
    const auto my = static_cast<std::ptrdiff_t>(addr / mb_w);

    // Both references: average the two predictions.
    if (fwd && bwd) {
        MacroblockSamples back;
        predict_direction(pred, *fwd, m, 0, mx, my);
        predict_direction(back, *bwd, m, 1, mx, my);
        // Averages `s` into `d` sample by sample, rounding up.
        auto average = [](auto& d, const auto& s) {
            // One sample at a time.
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

// ---- Vectors, coefficients and blocks ----

// One motion_vector(r, s): horizontal then vertical, each predicted from
// and stored to `pmv` (7.6.3.1). A field vector in a frame picture predicts
// its vertical component from PMV >> 1 and stores it back doubled (the
// predictors stay in frame units). `f_code` is the picture's pair for this direction, `field`
// says the vector is a field vector and `at` is the byte offset for errors. Returns the vector
// in half samples.
std::array<int, 2> motion_vector(
    BitReader& r,
    std::array<std::uint8_t, 2> f_code,
    std::array<int, 2>& pmv,
    bool field,
    std::size_t at
) {
    const Mpeg2VlcTables& t = mpeg2_vlc_tables();
    std::array<int, 2> out{};

    // Horizontal first (c = 0), then vertical.
    for (std::size_t c = 0; c < 2; ++c) {
        const int fc = f_code[c];

        // An f_code outside 1 to 9 is not one the decoder supports.
        if (fc < 1 || fc > 9) {
            error(at, "f_code {}", fc);
        }

        const auto magnitude = t.motion_code.decode(r);

        // The bits match no code of Table B-10.
        if (!magnitude) {
            error(at, "bad motion_code");
        }

        const int m = *magnitude;
        // A sign bit follows a non-zero magnitude.
        const int code = m != 0 && r.bit() ? -m : m;
        // f_code - 1 residual bits follow, and the vector is scaled by f = 2^(f_code -
        // 1) (7.6.3.1).
        const auto r_size = static_cast<unsigned>(fc - 1);
        const int f = 1 << r_size;
        int delta = code;

        // With f = 1 or a zero motion_code there is no residual to read.
        if (f != 1 && code != 0) {
            const int residual = static_cast<int>(r.read(r_size));
            const int d = (std::abs(code) - 1) * f + residual + 1;
            delta = code < 0 ? -d : d;
        }

        // A field vector's vertical predictor is halved into field units, then stored back doubled.
        const bool halve = field && c == 1;
        const int prediction = halve ? pmv[c] >> 1 : pmv[c];
        // The vector wraps modulo 32 * f into the range -16 * f to 16 * f - 1 (7.6.3.1).
        const int low = -16 * f;
        const int high = 16 * f - 1;
        const int range = 32 * f;
        int v = prediction + delta;

        // Below the range: wrap up.
        if (v < low) {
            v += range;
        }

        // Above the range: wrap down.
        if (v > high) {
            v -= range;
        }

        pmv[c] = halve ? v * 2 : v;
        out[c] = v;
    }

    return out;
}

// Reads one DCT coefficient from `table`: a (run, signed level) pair, or none at the end of the
// block. Throws VideoError at offset `at` for bits that match no code.
std::optional<std::pair<std::size_t, int>> coefficient(
    BitReader& r, const VlcTable& table, std::size_t at
) {
    const auto v = table.decode(r);

    // The bits match no coefficient code.
    if (!v) {
        error(at, "bad DCT coefficient code {:016b}", r.peek(16));
    }

    // End of block: no more coefficients.
    if (*v == kDctEndOfBlock) {
        return std::nullopt;
    }

    // Escape: a 6-bit run and a 12-bit level follow, written out in full.
    if (*v == kDctEscape) {
        const std::size_t run = r.read(6);
        const int raw = static_cast<int>(r.read(12));
        // The 12-bit level is two's complement.
        const int level = raw >= 2048 ? raw - 4096 : raw;

        // An escaped level of 0 is not allowed.
        if (level == 0) {
            error(at, "escape level 0");
        }

        return std::pair{run, level};
    }

    // A table entry packs the run above the level; a sign bit follows the code.
    const std::size_t run = static_cast<std::size_t>(*v >> 8);
    const int level = *v & 0xff;
    return std::pair{run, r.bit() ? -level : level};
}

// Mismatch control (7.4.4) over a dequantised block whose coefficients add up to `sum`: when
// the sum is even, the last coefficient's lowest bit is flipped so that it is odd.
void mismatch_control(Block& block, int sum) {
    // An even sum of coefficients is made odd.
    if ((sum & 1) == 0) {
        block[63] ^= 1;
    }
}

// Decodes one intra block into `block`: the DC coefficient by prediction (7.4.1), then the AC
// coefficients dequantised with matrix `q` and the slice's quantiser scale. `b` is the block's
// index in the macroblock: 0 to 3 luma, 4 Cb, 5 Cr.
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
    // The colour component: 0 luma, 1 Cb, 2 Cr.
    const std::size_t comp = b < 4 ? 0 : static_cast<std::size_t>(b - 3);
    const auto size_code =
        comp == 0 ? t.dc_size_luminance.decode(r) : t.dc_size_chrominance.decode(r);

    // The bits match no dct_dc_size code.
    if (!size_code) {
        error(at, "bad dct_dc_size");
    }

    const auto size = static_cast<unsigned>(*size_code);
    int diff = 0;

    // A size of 0 means the difference is zero and no bits follow.
    if (size != 0) {
        const int v = static_cast<int>(r.read(size));
        // The difference is sent in `size` bits; a value below half the range is negative.
        diff = v < (1 << (size - 1)) ? v - (1 << size) + 1 : v;
    }

    // The DC value is the predictor plus the difference, then scaled by the DC precision.
    const int dc = st.dc_pred[comp] + diff;
    st.dc_pred[comp] = dc;
    // intra_dc_mult is 8, 4 or 2 for 8, 9 or 10 bits of precision (7.4.1).
    const int f0 = std::clamp(dc * (8 >> p.dc_precision), -2048, 2047);
    block[0] = f0;
    int sum = f0;
    const VlcTable& table = p.intra_vlc_format ? t.dct_one : t.dct_zero;
    std::size_t i = 1;

    // Read AC coefficients until the end of the block.
    while (const auto c = coefficient(r, table, at)) {
        i += c->first;

        // A run that moves past the last coefficient means a corrupt stream.
        if (i >= 64) {
            error(at, "intra block coefficient index past 63");
        }

        const std::size_t pos = (*p.scan)[i];
        // Dequantise: level * matrix * quantiser_scale / 16, saturated to 12 bits (7.4.2).
        const int v = std::clamp((c->second * q[pos] * st.qscale) / 16, -2048, 2047);
        block[pos] = v;
        sum += v;
        ++i;
    }

    mismatch_control(block, sum);
}

// Dequantises a non-intra `level`: (2 * level + sign) * q * qscale / 32, saturated (7.4.2).
int non_intra_value(int level, int q, int qscale) {
    const int sign = level > 0 ? 1 : (level < 0 ? -1 : 0);
    return std::clamp(((2 * level + sign) * q * qscale) / 32, -2048, 2047);
}

// Decodes one non-intra block into `block`, dequantised with matrix `q` and scale `qs`, and
// coded in scan order `scan`. `at` is the byte offset for errors.
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

    // Read the remaining coefficients until the end of the block.
    while (const auto c = coefficient(r, t.dct_zero, at)) {
        i += c->first;

        // A run that moves past the last coefficient means a corrupt stream.
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

// ---- Macroblocks ----

// A skipped macroblock: P = forward frame prediction with a zero vector
// (vector predictors reset); B = the previous macroblock's directions with
// frame prediction from the vector predictors PMV[0][s] (7.6.6.4: also after
// a field-predicted macroblock). The DC predictors are reset.
void skipped_macroblock(SliceContext& ctx, SliceState& st, std::size_t addr, std::size_t at) {
    // The DC predictors go back to 2^(7 + dc_precision).
    const int dc_reset = 1 << (7 + ctx.p.dc_precision);
    st.dc_pred = {dc_reset, dc_reset, dc_reset};
    MacroblockSamples pred;

    // One case for each picture type, and no default, so the compiler warns when one is added.
    switch (ctx.p.type) {
        // An I picture has no skipped macroblocks.
        case PictureType::I:
            error(at, "skipped macroblock in an I picture");
        // Forward prediction with a zero vector.
        case PictureType::P:
            st.pmv = {};
            predict_macroblock(pred, ctx.frame, ctx.fwd, nullptr, addr, Motion{});
            break;
        // Repeat the previous macroblock's directions with the vectors in PMV[0][s].
        case PictureType::B: {
            const std::int16_t f = st.prev_flags;

            // The previous macroblock must have had a direction to repeat; an intra one has none.
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

// Decodes the macroblock at address `addr` into `ctx.frame`, reading it from `r`.
void macroblock(SliceContext& ctx, SliceState& st, BitReader& r, std::size_t addr) {
    const Mpeg2VlcTables& t = mpeg2_vlc_tables();
    const PictureParams& p = ctx.p;
    const std::size_t at = r.byte_pos();
    const auto type_code = t.macroblock_type[static_cast<int>(p.type)].decode(r);

    // The bits match no macroblock_type code of this picture type.
    if (!type_code) {
        error(at, "bad macroblock_type in a {} picture", "IPB"[static_cast<int>(p.type)]);
    }

    const std::int16_t flags = *type_code;
    // macroblock_modes(): frame_motion_type and dct_type exist only when
    // frame_pred_frame_dct = 0.
    bool field_motion = false;

    // A macroblock with motion in a picture that allows field prediction says which it uses.
    if ((flags & (kMbForward | kMbBackward)) && !p.frame_pred_frame_dct) {
        // frame_motion_type.
        switch (r.read(2)) {
            // Field-based prediction.
            case 1:
                field_motion = true;
                break;
            // Frame-based prediction.
            case 2:
                break;
            // Dual-prime prediction is not supported.
            case 3:
                error(at, "dual-prime prediction is not supported");
            // Zero is reserved.
            default:
                error(at, "reserved frame_motion_type 0");
        }
    }

    // dct_type is sent with the blocks, when frame_pred_frame_dct is 0; set means field DCT.
    const bool field_dct = (flags & (kMbIntra | kMbPattern)) && !p.frame_pred_frame_dct && r.bit();
    ctx.stats.field_motion_macroblocks += field_motion;
    ctx.stats.field_dct_macroblocks += field_dct;

    // The macroblock carries a new quantiser_scale_code.
    if (flags & kMbQuant) {
        const int code = static_cast<int>(r.read(5));

        // Code 0 is not allowed.
        if (code == 0) {
            error(at, "quantiser_scale_code 0");
        }

        st.qscale = qscale(p, code);
    }

    // The DC predictors go back to 2^(7 + dc_precision) after a macroblock that is not intra.
    const int dc_reset = 1 << (7 + p.dc_precision);
    MacroblockSamples pred;

    // An intra macroblock has no prediction and resets the vector predictors.
    if (flags & kMbIntra) {
        st.pmv = {};
        st.prev_flags = kMbIntra;

        // Six blocks: four luma, then Cb and Cr.
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

    // An inter macroblock resets the DC predictors and reads the vectors of the directions it uses.
    st.dc_pred = {dc_reset, dc_reset, dc_reset};
    Motion motion;
    motion.field = field_motion;

    // Direction 0 is forward, 1 backward.
    for (std::size_t s = 0; s < 2; ++s) {
        // Skip a direction this macroblock does not use.
        if ((flags & (s == 0 ? kMbForward : kMbBackward)) == 0) {
            continue;
        }

        // Field prediction: a vector for each field, each with a bit that selects the
        // reference field.
        if (field_motion) {
            // The top field, then the bottom.
            for (std::size_t fr = 0; fr < 2; ++fr) {
                motion.sel[fr][s] = r.bit();
                motion.mv[fr][s] = motion_vector(r, p.f_code[s], st.pmv[fr][s], true, at);
            }
        } else {
            // Frame prediction: one vector, which also becomes the second field's predictor.
            motion.mv[0][s] = motion_vector(r, p.f_code[s], st.pmv[0][s], false, at);
            st.pmv[1][s] = st.pmv[0][s];
        }
    }

    // A P picture's "No MC": forward frame prediction with a zero vector,
    // the predictors reset.
    bool use_forward = (flags & kMbForward) != 0;
    bool use_backward = (flags & kMbBackward) != 0;

    // A P picture is always forward-predicted, with a zero vector when there is none.
    if (p.type == PictureType::P) {
        // No vector was sent: "No MC", and the predictors reset.
        if (!use_forward) {
            st.pmv = {};
            motion = Motion{};
        }

        use_forward = true;
        use_backward = false;
    }

    st.prev_flags = flags;
    // The coded_block_pattern: bit 5 - b is set when block b has coefficients.
    unsigned cbp = 0;

    // The macroblock says which of its blocks have coefficients.
    if (flags & kMbPattern) {
        const auto v = t.coded_block_pattern.decode(r);

        // The bits match no coded_block_pattern code.
        if (!v) {
            error(at, "bad coded_block_pattern");
        }

        // Pattern 0 is the 4:2:2 code, which has no meaning in 4:2:0.
        if (*v == 0) {
            error(at, "coded_block_pattern 0 (the 4:2:2 code) in 4:2:0");
        }

        cbp = static_cast<unsigned>(*v);
    }

    // Predict the whole macroblock first; the coded blocks are added to the prediction.
    predict_macroblock(
        pred,
        ctx.frame,
        use_forward ? ctx.fwd : nullptr,
        use_backward ? ctx.bwd : nullptr,
        addr,
        motion
    );

    // Only the blocks with coefficients are decoded; the others keep the prediction.
    for (int b = 0; b < 6; ++b) {
        // Block b is coded when bit 5 - b of the pattern is set.
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

// ---- Slices ----

// Decodes one slice into `ctx.frame`: `data` ends at the next start code, the slice's start code
// is at `sc` and `row` is its macroblock row.
void decode_slice(SliceContext& ctx, Bytes data, std::size_t sc, std::size_t row) {
    const Mpeg2VlcTables& t = mpeg2_vlc_tables();
    const std::size_t mb_w = ctx.frame.width / 16;
    BitReader r(data, sc + 4);
    const int code = static_cast<int>(r.read(5));

    // Code 0 is not allowed.
    if (code == 0) {
        error(sc, "quantiser_scale_code 0");
    }

    // slice_extension_flag: when set, intra_slice, slice_picture_id_enable and the 6-bit
    // slice_picture_id follow, then bytes of extra information for as long as a 1 bit precedes
    // each. None of it is needed.
    if (r.bit()) {
        r.skip(1 + 7);

        // Each 1 bit announces one more byte of extra information.
        while (r.bit()) {
            r.skip(8);
        }
    }

    // A slice starts with the DC predictors at 2^(7 + dc_precision).
    const int dc_reset = 1 << (7 + ctx.p.dc_precision);
    SliceState st;
    st.qscale = qscale(ctx.p, code);
    st.dc_pred = {dc_reset, dc_reset, dc_reset};
    // The address of the last macroblock decoded: one before the slice's first.
    auto addr = static_cast<std::ptrdiff_t>(row * mb_w) - 1;
    bool first = true;
    // The number of macroblocks in the picture.
    const auto count = static_cast<std::ptrdiff_t>(mb_w * ctx.frame.height / 16);

    // Each turn decodes one macroblock, and the ones skipped before it. The slice ends at a
    // start code.
    for (;;) {
        const std::size_t at = r.byte_pos();
        std::ptrdiff_t increment = 0;

        // macroblock_address_increment: each escape adds 33, and the first other code ends it.
        for (;;) {
            const auto v = t.macroblock_address.decode(r);

            // The bits match no macroblock_address_increment code.
            if (!v) {
                error(at, "bad macroblock_address_increment");
            }

            // An escape adds 33 and goes on, stuffing adds nothing, and any other
            // code ends the increment.
            if (*v == kMbaEscape) {
                increment += 33;
            } else if (*v != kMbaStuffing) {
                increment += *v;
                break;
            }
        }

        // The first macroblock of a slice is placed by its increment alone; later ones
        // skip what lies between.
        if (!first) {
            // Skip the macroblocks between the previous one and this one.
            for (std::ptrdiff_t k = 1; k < increment; ++k) {
                const std::ptrdiff_t a = addr + k;

                // A skipped macroblock has to be inside the picture.
                if (a >= count) {
                    error(at, "skipped macroblocks past the picture");
                }

                skipped_macroblock(ctx, st, static_cast<std::size_t>(a), at);
                ++ctx.stats.skipped_macroblocks;
            }
        }

        first = false;
        addr += increment;

        // The macroblock has to be inside the picture.
        if (addr >= count) {
            error(at, "macroblock address {} past the picture", addr);
        }

        macroblock(ctx, st, r, static_cast<std::size_t>(addr));
        ++ctx.stats.macroblocks;

        // Reading past the data means the slice is cut short.
        if (r.overrun()) {
            error(r.byte_pos(), "slice data runs past the next start code");
        }

        // The next 23 bits are zero: a start code follows, and the slice is over.
        if (r.at_start_code()) {
            break;
        }
    }
}

// Makes an empty frame of `w` x `h` with the given type, temporal reference and decode index:
// luma 0 and chroma 128 (grey).
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

// Reads the body of a picture coding extension from `e` into `params` and counts what it
// chose in `stats`. `at` is the byte offset for errors. Throws VideoError for the field
// pictures, concealment vectors and 11-bit DC precision that the decoder does not support.
void read_picture_coding_extension(
    BitReader& e, std::size_t at, PictureParams& params, VideoStats& stats
) {
    // f_code[s][t]: four 4-bit values, forward then backward, horizontal then vertical.
    for (auto& fs : params.f_code) {
        // Horizontal then vertical.
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

    // Only frame pictures (structure 3), not field pictures, are supported.
    if (structure != 3) {
        error(at, "picture_structure {} (only frame pictures are supported)", structure);
    }

    // Concealment motion vectors are not supported.
    if (concealment) {
        error(at, "concealment motion vectors are not supported");
    }

    // 11-bit DC precision is not allowed at Main Profile.
    if (params.dc_precision == 3) {
        error(at, "intra_dc_precision 11 bits is not allowed at Main Profile");
    }

    params.frame_pred_frame_dct = frame_pred_frame_dct;
    params.scan = alternate_scan ? &kAlternateScan : &kZigZag;
    ++stats.dc_precision[params.dc_precision];
    ++stats.q_scale_type[params.q_scale_type];
    ++stats.intra_vlc_format[params.intra_vlc_format];
    ++stats.top_field_first[top_field_first];
    ++stats.frame_pred_frame_dct[frame_pred_frame_dct];
    ++stats.alternate_scan[alternate_scan];
    ++stats.progressive_frame[progressive_frame];
}

// Reads the body of a quantiser matrix extension from `e`: the luma matrices, which replace
// `intra` and `non_intra` when the extension carries them (the chroma ones are for 4:2:2 and
// 4:4:4).
void read_quant_matrices(
    BitReader& e, std::array<std::uint8_t, 64>& intra, std::array<std::uint8_t, 64>& non_intra
) {
    // quant_matrix_extension (the luma matrices; the chroma ones
    // are for 4:2:2 and 4:4:4).
    // load_intra_quantiser_matrix: 64 bytes in zig-zag order follow.
    if (e.bit()) {
        // Each byte goes to the raster position the zig-zag scan names.
        for (const std::uint8_t z : kZigZag) {
            intra[z] = static_cast<std::uint8_t>(e.read(8));
        }
    }

    // load_non_intra_quantiser_matrix: 64 bytes in zig-zag order follow.
    if (e.bit()) {
        // Each byte goes to the raster position the zig-zag scan names.
        for (const std::uint8_t z : kZigZag) {
            non_intra[z] = static_cast<std::uint8_t>(e.read(8));
        }
    }
}

}  // namespace

// ---- The decoder ----

Mpeg2Decoder::Mpeg2Decoder(std::vector<std::uint8_t> elementary_stream)
    : m_data(std::move(elementary_stream)),
      m_intra_matrix(kDefaultIntraMatrix) {
    // Until a header loads one, the non-intra matrix is 16 everywhere.
    m_non_intra_matrix.fill(16);
}

int Mpeg2Decoder::code_at(std::size_t sc) const {
    return sc + 3 < m_data.size() ? m_data[sc + 3] : -1;
}

std::optional<VideoSequence> Mpeg2Decoder::sequence() {
    // Read headers until the first sequence header, or until the stream ends.
    while (!m_sequence && !m_ended) {
        // The stream ended before a sequence header.
        if (!step()) {
            break;
        }
    }

    // The sequence extension and user data that follow the header.
    while (m_sequence) {
        const auto sc = next_start_code(m_data, m_pos);

        // No more start codes.
        if (!sc) {
            break;
        }

        const int code = code_at(*sc);

        // Anything but an extension or user data ends the header's tail.
        if (code != kExtensionStartCode && code != kUserDataStartCode) {
            break;
        }

        step();
    }

    return m_sequence;
}

std::shared_ptr<const VideoFrame> Mpeg2Decoder::next_frame() {
    // Hand out a ready frame; otherwise decode further until one is ready or the stream ends.
    for (;;) {
        // A frame is ready: hand it out.
        if (!m_out.empty()) {
            auto f = std::move(m_out.front());
            m_out.pop_front();
            ++m_stats.frames_out;
            return f;
        }

        // Nothing is ready and nothing more will be: the end of the movie.
        if (m_ended) {
            return nullptr;
        }

        step();
    }
}

bool Mpeg2Decoder::step() {
    const auto sc = next_start_code(m_data, m_pos);

    // No more start codes: the stream is over, and the held reference frame is shown last.
    if (!sc || code_at(*sc) < 0) {
        // A reference frame is held back until the next reference arrives; the end of
        // the stream releases it.
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

    // One case for each start code the decoder acts on; user data and the rest are skipped.
    switch (code) {
        // A sequence header.
        case kSequenceHeaderStartCode:
            sequence_header(*sc);
            break;
        // An extension outside a picture.
        case kExtensionStartCode:
            extension(*sc);
            break;
        // A group of pictures: counted; its closed_gop bit follows the 25-bit time code.
        case kGroupStartCode: {
            BitReader r(m_data, *sc + 4);
            r.skip(25);
            ++m_stats.gops;
            m_stats.closed_gops += r.bit();
            break;
        }
        // A picture, which decodes its slices too.
        case kPictureStartCode:
            m_pos = picture(*sc);
            break;
        case kSequenceEndStartCode:
            // sequence_end_code: the held reference frame is displayed.
            // Show the held reference frame.
            if (m_held) {
                m_out.push_back(std::move(m_held));
                m_held = nullptr;
            }

            break;
        // Anything else is skipped.
        default:
            break;
    }

    return true;
}

void Mpeg2Decoder::sequence_header(std::size_t sc) {
    BitReader r(m_data, sc + 4);
    VideoSequence s;
    // Size, aspect ratio, frame rate code and bit rate (6.2.2.1).
    s.width = r.read(12);
    s.height = r.read(12);
    s.aspect_ratio = static_cast<std::uint8_t>(r.read(4));
    s.frame_rate_code = static_cast<std::uint8_t>(r.read(4));
    s.bit_rate = r.read(18);
    // Skip a marker bit, vbv_buffer_size_value (10 bits) and constrained_parameters_flag.
    r.skip(1 + 10 + 1);
    s.load_intra = r.bit();
    // Each header starts from the default matrices and may load its own.
    m_intra_matrix = kDefaultIntraMatrix;

    // A loaded intra matrix is sent as 64 bytes in zig-zag order.
    if (s.load_intra) {
        // Each byte goes to the raster position the zig-zag scan names.
        for (const std::uint8_t z : kZigZag) {
            m_intra_matrix[z] = static_cast<std::uint8_t>(r.read(8));
        }
    }

    s.load_non_intra = r.bit();
    m_non_intra_matrix.fill(16);

    // A loaded non-intra matrix is sent as 64 bytes in zig-zag order.
    if (s.load_non_intra) {
        // Each byte goes to the raster position the zig-zag scan names.
        for (const std::uint8_t z : kZigZag) {
            m_non_intra_matrix[z] = static_cast<std::uint8_t>(r.read(8));
        }
    }

    // The header runs past the data.
    if (r.overrun()) {
        error(sc, "truncated sequence header");
    }

    // frame_rate_code to the rate as numerator and denominator (Table 6-4); code 0 is forbidden.
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

    // Codes 0 and 9 to 15 are forbidden or reserved.
    if (s.frame_rate_code < 1 || s.frame_rate_code > 8) {
        error(sc, "frame_rate_code {}", s.frame_rate_code);
    }

    s.fps_num = kRates[s.frame_rate_code][0];
    s.fps_den = kRates[s.frame_rate_code][1];

    // The size has to be whole macroblocks and no larger than 1920x1152.
    if (s.width == 0 || s.height == 0 || s.width % 16 != 0 || s.height % 16 != 0 || s.width > 1920
        || s.height > 1152) {
        error(sc, "unsupported picture size {}x{}", s.width, s.height);
    }

    // A repeated sequence header must not change the picture size, and has no extension of its own.
    if (m_sequence) {
        // The frame buffers are sized from the first header.
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

// An extension outside a picture. Only the sequence extension (the sequence header's second
// half) is read; the others are ignored.
void Mpeg2Decoder::extension(std::size_t sc) {
    BitReader r(m_data, sc + 4);

    // Only the sequence extension is read here.
    if (r.read(4) != kSequenceExtensionId) {
        return;
    }

    // The extension extends a header, so a header must come first.
    if (!m_sequence) {
        error(sc, "a sequence extension before a sequence header");
    }

    VideoSequence& s = *m_sequence;
    s.profile_level = static_cast<std::uint8_t>(r.read(8));
    s.progressive = r.bit();
    s.chroma_format = static_cast<std::uint8_t>(r.read(2));
    const std::uint32_t horizontal_ext = r.read(2);
    const std::uint32_t vertical_ext = r.read(2);
    // Skip bit_rate_extension (12 bits), a marker bit, vbv_buffer_size_extension (8) and low_delay.
    r.skip(12 + 1 + 8 + 1);
    const std::uint32_t n = r.read(2);
    const std::uint32_t d = r.read(5);

    // The size extension bits would make the picture larger than the header says.
    if (horizontal_ext != 0 || vertical_ext != 0) {
        error(sc, "the size extension bits are set");
    }

    // Only 4:2:0 (code 1) is supported.
    if (s.chroma_format != 1) {
        error(sc, "chroma_format {} (only 4:2:0 is supported)", s.chroma_format);
    }

    // The extension scales the rate by (n + 1) / (d + 1).
    s.fps_num *= n + 1;
    s.fps_den *= d + 1;
}

std::size_t Mpeg2Decoder::picture(std::size_t sc) {
    // A picture needs a sequence header first.
    if (!m_sequence) {
        error(sc, "a picture before a sequence header");
    }

    const VideoSequence seq = *m_sequence;
    BitReader r(m_data, sc + 4);
    const auto temporal_reference = static_cast<std::uint16_t>(r.read(10));
    PictureParams params;

    // picture_coding_type; D pictures and the reserved values are not supported.
    switch (r.read(3)) {
        // An I picture.
        case 1:
            params.type = PictureType::I;
            break;
        // A P picture.
        case 2:
            params.type = PictureType::P;
            break;
        // A B picture.
        case 3:
            params.type = PictureType::B;
            break;
        // Anything else cannot be decoded.
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

        // The stream ends, or its start codes are broken, before any slice.
        if (!s || code_at(*s) < 0) {
            error(sc, "a picture without slices");
        }

        const int code = code_at(*s);
        pos = *s + 4;

        // An extension: the picture coding extension or the quantiser matrix extension.
        if (code == kExtensionStartCode) {
            BitReader e(m_data, *s + 4);
            const std::uint32_t id = e.read(4);

            // The picture coding extension sets how this picture is coded.
            if (id == kPictureCodingExtensionId) {
                read_picture_coding_extension(e, *s, params, m_stats);
                have_extension = true;
            } else if (id == kQuantMatrixExtensionId) {
                read_quant_matrices(e, m_intra_matrix, m_non_intra_matrix);
            }
        } else if (code >= kFirstSliceStartCode && code <= kLastSliceStartCode) {
            // A slice start code: the headers are over.
            first_slice = *s;
            break;
        } else if (code != kUserDataStartCode) {
            // Anything but user data is unexpected before the first slice.
            error(*s, "start code {:#x} before the picture's first slice", code);
        }
    }

    // Without the picture coding extension this is an MPEG-1 picture, which is not supported.
    if (!have_extension) {
        error(sc, "an MPEG-1 picture (no picture coding extension)");
    }

    // The picture's place in decoding order, counting the B pictures that were skipped.
    const std::size_t decode_index =
        m_stats.pictures[0] + m_stats.pictures[1] + m_stats.pictures[2] + m_stats.skipped_b;
    const std::size_t w = seq.width;
    const std::size_t h = seq.height;
    std::shared_ptr<const VideoFrame> fwd;
    std::shared_ptr<const VideoFrame> bwd;

    // A P picture predicts forward from the latest reference.
    if (params.type == PictureType::P) {
        // There is no reference yet to predict from.
        if (!m_refs[1]) {
            error(sc, "a P picture without a reference frame");
        }

        fwd = m_refs[1];
    } else if (params.type == PictureType::B) {
        // A B picture predicts forward from the older reference and backward from the newer.
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

    // Each turn handles one slice; the slices end at the first start code that is not one.
    for (;;) {
        const int code = code_at(s);

        // A start code that is not a slice ends the picture.
        if (code < kFirstSliceStartCode || code > kLastSliceStartCode) {
            end = s;
            break;
        }

        const std::size_t next = next_start_code(m_data, s + 4).value_or(m_data.size());

        // A skipped picture's slices are stepped over without being decoded.
        if (!skip) {
            const auto row = static_cast<std::size_t>(code - 1);

            // The slice's row has to be inside the picture.
            if (row >= h / 16) {
                error(s, "slice row {} outside the picture", row);
            }

            decode_slice(ctx, Bytes(m_data.data(), next), s, row);
        }

        // The last slice runs to the end of the data.
        if (next >= m_data.size()) {
            end = m_data.size();
            break;
        }

        s = next;
    }

    // A skipped B picture produces no frame.
    if (skip) {
        ++m_stats.skipped_b;
        return end;
    }

    ++m_stats.pictures[static_cast<std::size_t>(params.type)];

    // A B picture is shown at once; a reference frame is held back until the next reference
    // arrives.
    if (params.type == PictureType::B) {
        m_out.push_back(std::move(frame));
    } else {
        // The reference frame held back until now is output.
        if (m_held) {
            m_out.push_back(std::move(m_held));
        }

        // The new reference frame is held back; the previous newer reference becomes the older one.
        m_held = frame;
        m_refs[0] = std::move(m_refs[1]);
        m_refs[1] = std::move(frame);
    }

    return end;
}

}  // namespace openrac::media
