// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-video/src/mpeg2.rs:
// ISC License, Copyright (c) 2026 ReRAC contributors.
//
// MPEG-2 video decoding (ISO/IEC 13818-2, the public standard), scoped to
// what the games' movies use. ReRAC surveyed all 84 RAC1 (NTSC-U) movies:
// Main Profile at Main Level, 4:2:0, frame pictures only, I, P and B
// pictures, intra_dc_precision 8 to 10 bits, both q_scale_types, both intra
// VLC tables, loaded or default quantiser matrices. 82 files are progressive
// (frame DCT and frame prediction, zig-zag scan); two are interlaced and also
// use field prediction in frame pictures, field DCT and the alternate scan.
// Anything else (field pictures, dual-prime prediction, 4:2:2, concealment
// vectors, D pictures) is a VideoError, never a guess. The later games'
// movies are the same Sony container and decoder chip, but nobody has
// surveyed their streams yet.
//
// next_frame() returns the frames in display order: an I or P frame is held
// back until the next reference picture, a B frame comes out at once. The
// output is exact given the IDCT (idct.h) and deterministic. The decoder is
// ReRAC's own; nothing here comes from Sony's sample players.

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace openrac::media {

// A stream the decoder cannot read, with the byte offset in the elementary
// stream where that was found.
class VideoError : public std::runtime_error {
public:
    // Formats `message` and the byte `offset` into what().
    VideoError(std::size_t offset, const std::string& message);

    // The byte offset in the elementary stream where the problem was found.
    std::size_t offset() const { return m_offset; }

private:
    // The offset the error carries.
    std::size_t m_offset;
};

// The type of a picture (picture_coding_type).
enum class PictureType : std::uint8_t {
    I,  // Intra: coded on its own.
    P,  // Predicted from the previous reference picture.
    B,  // Predicted from the previous and the next reference pictures.
};

// The sequence header and its extension.
struct VideoSequence {
    std::uint32_t width = 0;           // Luma width in samples; a multiple of 16.
    std::uint32_t height = 0;          // Luma height in samples; a multiple of 16.
    std::uint8_t aspect_ratio = 0;     // The aspect_ratio_information code.
    std::uint8_t frame_rate_code = 0;  // The frame_rate_code (Table 6-4).
    // Frames per second as a fraction (frame_rate_code with the extension's factor).
    std::uint32_t fps_num = 0;
    std::uint32_t fps_den = 1;
    std::uint32_t bit_rate = 0;      // bit_rate_value, in units of 400 bits per second.
    std::uint8_t profile_level = 0;  // The profile_and_level_indication.
    bool progressive = false;        // progressive_sequence.
    std::uint8_t chroma_format = 0;  // 1 is 4:2:0, the only one supported.
    bool load_intra = false;         // The header carried an intra quantiser matrix.
    bool load_non_intra = false;     // The header carried a non-intra quantiser matrix.

    // The frame rate as a number.
    double fps() const { return static_cast<double>(fps_num) / fps_den; }
};

// A decoded picture: 8-bit Y, Cb, Cr planes, 4:2:0 (strides width, width / 2).
struct VideoFrame {
    std::size_t width = 0;                 // In luma samples.
    std::size_t height = 0;                // In luma samples.
    std::vector<std::uint8_t> y;           // Luma plane, row by row.
    std::vector<std::uint8_t> cb;          // Cb plane, half the width and height.
    std::vector<std::uint8_t> cr;          // Cr plane, half the width and height.
    PictureType type = PictureType::I;     // How the picture was coded.
    std::uint16_t temporal_reference = 0;  // The picture's display position within its group.
    std::size_t decode_index = 0;          // The picture's place in decoding order.
};

// What the stream used: reports and the header survey.
struct VideoStats {
    std::size_t sequences = 0;              // Sequence headers read.
    std::size_t gops = 0;                   // Groups of pictures read.
    std::size_t closed_gops = 0;            // Of those, the closed ones.
    std::array<std::size_t, 3> pictures{};  // decoded, by type (I, P, B)
    // B pictures without both references (the leading B pictures of an open
    // GOP at the start): skipped.
    std::size_t skipped_b = 0;
    std::size_t frames_out = 0;                         // Frames handed out by next_frame().
    std::array<std::size_t, 4> dc_precision{};          // Pictures by intra_dc_precision.
    std::array<std::size_t, 2> q_scale_type{};          // Pictures by q_scale_type.
    std::array<std::size_t, 2> intra_vlc_format{};      // Pictures by intra_vlc_format.
    std::array<std::size_t, 2> top_field_first{};       // Pictures by top_field_first.
    std::array<std::size_t, 2> frame_pred_frame_dct{};  // Pictures by frame_pred_frame_dct.
    std::array<std::size_t, 2> alternate_scan{};        // Pictures by alternate_scan.
    std::array<std::size_t, 2> progressive_frame{};     // Pictures by progressive_frame.
    std::size_t field_motion_macroblocks = 0;           // Macroblocks with field prediction.
    std::size_t field_dct_macroblocks = 0;              // Macroblocks with field DCT.
    std::size_t macroblocks = 0;                        // Macroblocks decoded.
    std::size_t skipped_macroblocks = 0;  // Macroblocks skipped (predicted, with no coefficients).
    std::size_t coded_blocks = 0;         // Blocks with coefficients.
};

// Zig-zag scan: coefficient i in transmission order is at raster position kZigZag[i].
extern const std::array<std::uint8_t, 64> kZigZag;
// The alternate (vertical) scan.
extern const std::array<std::uint8_t, 64> kAlternateScan;
// The default intra quantiser matrix, raster order.
extern const std::array<std::uint8_t, 64> kDefaultIntraMatrix;

// The decoder over one elementary stream.
class Mpeg2Decoder {
public:
    // Takes the whole stream. Nothing is decoded until the sequence or a frame is asked for.
    explicit Mpeg2Decoder(std::vector<std::uint8_t> elementary_stream);

    // The first sequence header (parsed on demand), or none when the stream
    // has none. Throws VideoError.
    std::optional<VideoSequence> sequence();

    // The next frame in display order; null at the end. Throws VideoError.
    std::shared_ptr<const VideoFrame> next_frame();

    // What has been counted so far.
    const VideoStats& stats() const { return m_stats; }

private:
    // Reads the next start code and what it introduces. False when the stream has run out.
    bool step();
    // Reads the sequence header whose start code is at `sc`.
    void sequence_header(std::size_t sc);
    // Reads an extension that is outside a picture; `sc` is its start code.
    void extension(std::size_t sc);
    // Decodes the picture whose header starts at `sc`; returns the offset of the start code
    // after its slices.
    std::size_t picture(std::size_t sc);
    // The start code value (the byte after `00 00 01`) at `sc`, or -1 past the end of the data.
    int code_at(std::size_t sc) const;

    // The elementary stream.
    std::vector<std::uint8_t> m_data;
    // Where the search for the next start code begins.
    std::size_t m_pos = 0;
    // The sequence header and its extension, once read.
    std::optional<VideoSequence> m_sequence;
    // The quantiser matrices in force, in raster order: the defaults or the ones a header loaded.
    std::array<std::uint8_t, 64> m_intra_matrix;
    std::array<std::uint8_t, 64> m_non_intra_matrix;
    // Past and most recent reference frames.
    std::array<std::shared_ptr<const VideoFrame>, 2> m_refs;
    // The most recent I or P frame, output when the next reference picture
    // arrives (or at the end).
    std::shared_ptr<const VideoFrame> m_held;
    // Frames ready to hand out, in display order.
    std::deque<std::shared_ptr<const VideoFrame>> m_out;
    // True once the stream has run out.
    bool m_ended = false;
    // What the decoder has counted.
    VideoStats m_stats;
};

}  // namespace openrac::media
