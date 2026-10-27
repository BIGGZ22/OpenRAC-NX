// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

/**
 * The sound library's streams (music and speech), decoded and mixed on the machine's schedule.
 *
 * A stream is an ADPCM file on the disc. The file's blocks are decoded as the stream plays and
 * resampled to the output rate. It leaves out the sound effects of the banks and any effect
 * processing.
 *
 * Sources: the console's ADPCM block layout, as publicly documented.
 */

#pragma once

#include <deque>
#include <unordered_map>
#include <vector>

#include "disc.h"
#include "ps2/types.h"

namespace sys {

using ps2::s16;
using ps2::s32;
using ps2::u32;
using ps2::u64;
using ps2::u8;

/**
 * What the games hear, as far as it is made here: the streams of the sound
 * library (music and speech), which are plain ADPCM files on the disc that
 * a program names by sector. Sound effects from the banks are not made yet.
 *
 * Sound is produced in step with the machine: one field's worth after each
 * field. So what the program is told about a stream (still playing, how
 * long is left) and what is heard agree, and a run gives the same sound
 * every time.
 *
 * No locking: the machine calls every method from the thread that runs it. `mix` is called once
 * per field from the vertical blank, and the others from the replaced sound library functions
 * (services.cpp).
 */
class Sound {
public:
    /** Sampling rate in Hz of what `mix` produces, the console's output rate (documented). */
    static constexpr int kRate = 48000;

    /**
     * Makes a mixer that reads its streams from a disc.
     *
     * @param disc The image the streams' sectors are read from; it must outlive the mixer.
     */
    explicit Sound(Disc& disc) : disc_(disc) {}

    /**
     * Plays the file at a sector: at once, as a stream of its own with the handle given, or, when
     * `after` names a stream that is playing, stitched on behind what that one is playing (and in
     * place of its repeating).
     *
     * @param handle The name the new stream is known by in the other calls.
     * @param sector First sector of the file on the disc.
     * @param volume Volume of this file, in 1,024ths.
     * @param group The volume group the stream belongs to, set with `set_group_volume`.
     * @param repeat True to start the file over each time it ends.
     * @param after Handle of a playing stream to stitch the file onto. 0, or a handle that is not
     *     playing, starts a new stream.
     * @return True if the file could be read; false leaves every stream as it was.
     */
    bool play(u32 handle, u32 sector, int volume, u32 group, bool repeat, u32 after);

    /**
     * Holds a stream where it is, or lets it go on.
     *
     * @param handle The stream; an unknown handle is ignored.
     * @param paused True to hold, false to resume.
     */
    void pause(u32 handle, bool paused);

    /**
     * Ends a stream.
     *
     * @param handle The stream; an unknown handle is ignored.
     */
    void stop(u32 handle);

    /** Ends every stream. */
    void stop_all();

    /** True while a stream with this handle exists (paused ones included). */
    bool playing(u32 handle) const { return streams_.count(handle) != 0; }

    /**
     * Sixtieths of a second of the stream's current file still to play.
     *
     * @param handle The stream.
     * @return The time left, or 0 for an unknown handle.
     */
    u32 remaining(u32 handle) const;

    /**
     * Sets the volume that scales every stream of a group.
     *
     * @param group The group number the streams were started with.
     * @param volume The group's volume, in 1,024ths.
     */
    void set_group_volume(u32 group, int volume);

    /**
     * The next `frames` of sound, appended to `out` as left and right.
     *
     * Streams that run out are removed. A paused stream adds nothing.
     *
     * @param frames Number of frames to make, a frame being one left and one right sample.
     * @param[out] out Receives `frames * 2` samples at the end, left first.
     */
    void mix(std::size_t frames, std::vector<s16>& out);

private:
    /** One file of a stream: its blocks, as read from the disc. */
    struct Piece {
        /** The sound data, in 16-byte blocks of 28 samples. */
        std::vector<u8> data;

        /** The file's sampling rate in Hz, from its header. */
        u32 rate = 44100;

        /** The volume of this file, in 1,024ths. */
        int volume = 1024;

        /** True if the file starts over when it ends. */
        bool repeat = false;
    };

    /** A playing stream: files in a row, and the decoder's state in the first. */
    struct Stream {
        /** The files in the order they play; the first is playing. */
        std::deque<Piece> pieces;

        /** The volume group of the stream. */
        u32 group = 0;

        /** True while the stream is held. */
        bool paused = false;

        /** The next block to decode. */
        std::size_t block = 0;

        /** The two samples before it, for the decoder. */
        s32 before[2] = {0, 0};

        /** The block being played. */
        s16 samples[28] = {};

        /** The next of those; 28 means the block is used up and the next one is to be decoded. */
        unsigned at = 28;

        /** How far between two samples the output is. */
        double phase = 0;

        /** The two samples around the output's position. */
        s16 last = 0, next = 0;
    };

    /**
     * Reads a stream file from the disc.
     *
     * A stream file is "VAGp", then in big-endian words a version, nothing, the size of the sound
     * data and the sampling rate, a name, and from byte 48 on the blocks (documented).
     *
     * @param sector First sector of the file.
     * @param[out] piece Receives the file's data and its sampling rate.
     * @return True if the file has a valid header and at least one block to play.
     */
    bool load(u32 sector, Piece& piece);

    /**
     * Moves a stream one sample of its file on.
     *
     * The next sample becomes the last one and a new next one is taken from the block being
     * played, decoding the following block when that one is used up.
     *
     * @param stream The stream to move.
     * @return False when the stream is over.
     */
    bool advance(Stream& stream);

    /** The disc the streams are read from; not owned. */
    Disc& disc_;

    /** The streams that exist, by handle. */
    std::unordered_map<u32, Stream> streams_;

    /** The volume of each group, in 1,024ths; a group not listed plays at full volume. */
    std::unordered_map<u32, int> group_volume_;
};

}  // namespace sys
