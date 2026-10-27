// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

/**
 * The sound library's streams: reading the files, decoding their ADPCM and mixing them.
 *
 * Each stream is decoded one sample at a time and resampled by a straight line between two
 * neighbouring samples, so the output is the same on every run.
 *
 * Sources: the console's ADPCM block layout, as publicly documented.
 */

#include "sound.h"

#include <algorithm>
#include <cstring>

namespace sys {
namespace {

/**
 * The console's ADPCM: each 16-byte block has a shift and a predictor
 * number, a flag byte and 28 four-bit samples; a sample is predicted from
 * the two before it with one of five pairs of weights (in 64ths) (documented).
 */
constexpr s32 kWeights[5][2] = {{0, 0}, {60, 0}, {115, -52}, {98, -55}, {122, -60}};

/**
 * Reads a 32-bit word that is stored most significant byte first, as the stream header does.
 *
 * @param p Address of the word's first byte.
 * @return The word's value.
 */
u32 big32(const u8* p) {
    return (u32{p[0]} << 24) | (u32{p[1]} << 16) | (u32{p[2]} << 8) | p[3];
}

}  // namespace

bool Sound::load(u32 sector, Piece& piece) {
    u8 head[Disc::kSector];

    // The first sector is unreadable, or it does not start with the stream file's magic "VAGp".
    if (!disc_.read(sector, 1, head) || std::memcmp(head, "VAGp", 4) != 0) {
        return false;
    }

    // Data size at byte 12 and sampling rate at byte 16 of the header.
    u32 bytes = big32(head + 12);
    piece.rate = big32(head + 16);

    // Refuse a header that is empty, over 64 MiB, or has a rate outside 4 kHz to 96 kHz.
    if (bytes == 0 || bytes > 64u * 1024 * 1024 || piece.rate < 4000 || piece.rate > 96000) {
        return false;
    }

    // The whole file: the 48-byte header and the data, rounded up to whole sectors.
    u32 sectors = static_cast<u32>((48 + u64{bytes} + Disc::kSector - 1) / Disc::kSector);
    std::vector<u8> file(std::size_t{sectors} * Disc::kSector);

    // The disc ends before the file does.
    if (!disc_.read(sector, sectors, file.data())) {
        return false;
    }

    // Keep the data after the header, as whole 16-byte blocks.
    piece.data.assign(file.begin() + 48, file.begin() + 48 + (bytes & ~15u));

    // A last block flagged "the end, and not to be played" is not sound.
    while (piece.data.size() >= 16 && piece.data[piece.data.size() - 15] == 7) {
        piece.data.resize(piece.data.size() - 16);
    }

    return !piece.data.empty();
}

bool Sound::play(u32 handle, u32 sector, int volume, u32 group, bool repeat, u32 after) {
    Piece piece;

    // The file cannot be played, so nothing changes.
    if (!load(sector, piece)) {
        return false;
    }

    piece.volume = volume;
    piece.repeat = repeat;

    // Only an `after` that names a playing stream is looked for; 0 means a new stream.
    auto behind = after ? streams_.find(after) : streams_.end();

    // Stitch the file on behind the stream that `after` names.
    if (behind != streams_.end()) {
        // Only what is playing stays ahead of it.
        Stream& stream = behind->second;
        stream.pieces.resize(1);
        stream.pieces.push_back(std::move(piece));
        return true;
    }

    Stream stream;
    stream.group = group;
    stream.pieces.push_back(std::move(piece));
    streams_[handle] = std::move(stream);
    return true;
}

void Sound::pause(u32 handle, bool paused) {
    auto found = streams_.find(handle);

    // An unknown handle is ignored.
    if (found != streams_.end()) {
        found->second.paused = paused;
    }
}

void Sound::stop(u32 handle) {
    streams_.erase(handle);
}

void Sound::stop_all() {
    streams_.clear();
}

u32 Sound::remaining(u32 handle) const {
    auto found = streams_.find(handle);

    // An unknown handle has nothing left to play.
    if (found == streams_.end()) {
        return 0;
    }

    const Stream& stream = found->second;
    const Piece& piece = stream.pieces.front();

    // Samples left: 28 per block not yet decoded, plus those left in the block being played.
    u64 samples = (piece.data.size() / 16 - std::min(stream.block, piece.data.size() / 16)) * 28
                  + (28 - stream.at);

    // Samples to sixtieths of a second at the file's rate.
    return static_cast<u32>(samples * 60 / piece.rate);
}

void Sound::set_group_volume(u32 group, int volume) {
    group_volume_[group] = volume;
}

bool Sound::advance(Stream& stream) {
    // All 28 samples of the block have been played: decode the next block.
    if (stream.at == 28) {
        Piece* piece = &stream.pieces.front();

        // The file has no blocks left: go on to the next file, start over, or end.
        if (stream.block * 16 >= piece->data.size()) {
            // Another file was stitched on behind this one.
            if (stream.pieces.size() > 1) {
                stream.pieces.pop_front();
                piece = &stream.pieces.front();
            } else if (!piece->repeat) {
                // The last file does not repeat: the stream is over.
                return false;
            }

            // A new file, or the same one again, starts with a clean decoder.
            stream.block = 0;
            stream.before[0] = stream.before[1] = 0;
        }

        const u8* block = piece->data.data() + stream.block * 16;
        stream.block++;

        // Block byte 0: shift in bits 0-3, predictor in bits 4-6, capped at 4 (documented).
        unsigned shift = block[0] & 15, predictor = std::min<unsigned>((block[0] >> 4) & 7, 4);
        s32 w0 = kWeights[predictor][0], w1 = kWeights[predictor][1];

        // Block bytes 2-15 hold 28 nibbles, the low nibble of each byte first (documented).
        for (unsigned n = 0; n < 28; n++) {
            s32 nibble = (block[2 + n / 2] >> ((n & 1) * 4)) & 15;

            // Put the nibble in the top of 16 bits to sign it, then scale it down by the shift.
            s32 value = static_cast<s16>(nibble << 12) >> shift;

            // Add the prediction from the two samples before, with the weights in 64ths.
            value += (stream.before[0] * w0 + stream.before[1] * w1) >> 6;

            // The result is a signed 16-bit sample.
            value = std::clamp(value, -32768, 32767);
            stream.samples[n] = static_cast<s16>(value);
            stream.before[1] = stream.before[0];
            stream.before[0] = value;
        }
        stream.at = 0;
    }

    stream.last = stream.next;
    stream.next = stream.samples[stream.at++];
    return true;
}

void Sound::mix(std::size_t frames, std::vector<s16>& out) {
    std::size_t first = out.size();
    out.resize(first + frames * 2, 0);

    // The sum of all streams, one value per frame; the streams are mono.
    std::vector<s32> sum(frames, 0);

    // One pass per stream; the iterator moves on, or the stream is erased when it is over.
    for (auto it = streams_.begin(); it != streams_.end();) {
        Stream& stream = it->second;
        bool over = false;

        // A paused stream adds nothing and keeps its place.
        if (!stream.paused) {
            auto group = group_volume_.find(stream.group);

            // A group that never got a volume plays at 1,024 (full volume).
            s32 master = group == group_volume_.end() ? 1024 : group->second;

            // One output frame per pass, until the frames are made or the stream ends.
            for (std::size_t n = 0; n < frames && !over; n++) {
                const Piece& piece = stream.pieces.front();
                // The output's rate against the file's: a straight line between
                // the two samples either side.
                stream.phase += static_cast<double>(piece.rate) / kRate;

                // Take as many file samples as the output has passed since the last frame.
                while (stream.phase >= 1.0 && !over) {
                    stream.phase -= 1.0;
                    over = !advance(stream);
                }

                // The stream ended in this frame: it adds nothing more.
                if (!over) {
                    double value = stream.last + (stream.next - stream.last) * stream.phase;

                    // Volumes are in 1,024ths, so each product is shifted down by 10 bits.
                    s32 gain = (stream.pieces.front().volume * master) >> 10;
                    sum[n] += static_cast<s32>(value) * gain >> 10;
                }
            }
        }

        // An ended stream is removed; the rest stay.
        it = over ? streams_.erase(it) : std::next(it);
    }

    // Clip the sum to 16 bits and write it to both channels.
    for (std::size_t n = 0; n < frames; n++) {
        s16 value = static_cast<s16>(std::clamp(sum[n], -32768, 32767));
        out[first + n * 2] = value;
        out[first + n * 2 + 1] = value;
    }
}

}  // namespace sys
