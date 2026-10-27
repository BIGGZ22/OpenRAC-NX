// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

/**
 * The GIF, the interface that turns packets into register writes and image data for the GS.
 *
 * It models the three layouts of a packet (PACKED, REGLIST, IMAGE) and the three paths into the
 * GIF, and it can run the GIF and the GS on a thread of its own. It leaves out the arbitration
 * between the paths (a packet on one path is never interrupted by another) and the GIF's FIFO and
 * timing.
 *
 * Sources: the GIF and its packet formats as publicly documented.
 */

#pragma once

#include <array>
#include <condition_variable>
#include <deque>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

#include "gs.h"
#include "types.h"

namespace ps2 {

/**
 * The GS interface: turns GIF packets into register writes and image data.
 * A packet is a tag quadword followed by its data, in one of three layouts:
 * PACKED (a quadword per register), REGLIST (64 bits per register) and IMAGE
 * (raw pixels for a transfer). Each of the three paths (1: VU1's XGKICK,
 * 2: VIF1's DIRECT, 3: the GIF DMA channel) keeps its own position, because a
 * packet may arrive in several pieces.
 *
 * The GIF and the GS behind it can have a thread of their own (`start`):
 * then `write` copies its data and returns, and the packets are taken apart
 * and drawn on that thread, in the order given. Whatever else is to happen
 * on the GS's side in that order goes through `run`, and whoever needs the
 * GS itself waits with `sync` first.
 *
 * With a thread, one thread feeds it (`start`, `write`, `run`, `sync`, the destructor); the GIF's
 * own thread runs `loop`, and with it `take` and everything the GS does. The queue and the flags
 * that go with it are guarded by `mutex_`; the paths belong to whichever thread calls `take`.
 */
class Gif {
public:
    /**
     * Makes a GIF that writes to a GS. It starts without a thread.
     *
     * @param gs The GS that receives the register writes and the image data.
     */
    explicit Gif(Gs& gs) : gs_(gs) {}

    /** Stops the GIF's thread, after it has taken everything handed to it. */
    ~Gif();
    Gif(const Gif&) = delete;
    Gif& operator=(const Gif&) = delete;

    /** Forgets the position on every path: the next quadword of each is a tag. */
    void reset();

    /**
     * Gives the GIF whole quadwords for one path.
     *
     * Without a thread the packets are taken apart at once. With one, the data is copied and
     * handed to the thread in pieces of at least 64 KB, or when `run` or `sync` is called.
     *
     * @param path The path the data came by: 1, 2 or 3.
     * @param data The first quadword, 16 bytes each; it need not stay valid after the call.
     * @param quadwords How many quadwords there are.
     */
    void write(int path, const u8* data, std::size_t quadwords);

    /**
     * True when the path is between packets (the last tag seen had EOP and its data is complete).
     * Only without a thread, where `write` has done its work.
     *
     * @param path The path to ask about: 1, 2 or 3.
     * @return Whether the path is between packets.
     */
    bool idle(int path) const { return !paths_[path - 1].in_packet; }

    /**
     * Counts the quadwords of the packet at `at` in a memory of `mask + 1` quadwords, up to the end
     * of the data of its first tag with EOP, wrapping at the memory's end. For XGKICK, which names
     * only the start.
     *
     * @param memory The first byte of the memory, which holds `mask + 1` quadwords.
     * @param at Quadword the packet starts at.
     * @param mask Number of quadwords in the memory, minus one; a power of two minus one.
     * @return The packet's length in quadwords, tag included, at most the size of the memory.
     */
    static std::size_t packet_quadwords(const u8* memory, u32 at, u32 mask);

    /** From now on the packets are handled on a thread of their own. */
    void start();

    /**
     * Does something on the GS's side, after everything written so far.
     *
     * Without a thread it runs at once, on the caller's thread.
     *
     * @param what The work. It runs on the GIF's thread, or on the caller's when there is none.
     */
    void run(std::function<void()> what);

    /** Waits until everything written and asked has been done. */
    void sync();

private:
    /** Where a path is in the packet it is taking. */
    struct Path {
        /** The REGS field of the tag: 16 descriptors of 4 bits. */
        u64 regs = 0;

        /** Loops (or image quadwords) still to come. */
        u32 loops = 0;

        /** Number of registers in the tag's loop, 1-16 (the tag's 0 is already turned into 16). */
        u32 nreg = 0;

        /** Next register descriptor in the loop. */
        u32 reg = 0;

        /** The tag's FLG field: how the data after it is laid out. */
        u32 flg = 0;

        /** The tag's EOP bit: this is the last tag of its packet. */
        bool eop = true;

        /** True from a tag until the end of the packet it starts. */
        bool in_packet = false;

        /** The Q the last ST of a PACKED packet carried; the next RGBAQ takes it. */
        float q = 1.0f;
    };

    /**
     * Writes the register that one quadword of a PACKED packet stands for.
     *
     * @param p The path; its `reg` says which descriptor the quadword is for.
     * @param lo Bits 0-63 of the quadword.
     * @param hi Bits 64-127 of the quadword.
     */
    void packed(Path& p, u64 lo, u64 hi);

    /**
     * Moves to the next register of the loop, and to the next loop after the last register.
     *
     * @param p The path.
     */
    void advance(Path& p);

    /**
     * Takes apart the quadwords of a path, tags first, and passes them to the GS.
     *
     * @param path The path they came by: 1, 2 or 3.
     * @param data The first quadword.
     * @param quadwords How many quadwords there are.
     */
    void take(int path, const u8* data, std::size_t quadwords);

    /** The GS that receives what the paths produce. */
    Gs& gs_;

    /** The position on each of the three paths. */
    std::array<Path, 3> paths_{};

    /**
     * Data copied for the thread: pieces of the paths, in order, and optionally something to do
     * after them.
     */
    struct Piece {
        /** The path the data came by. */
        int path;

        /**
         * Byte offset into the chunk's data where this piece starts (`at`), and how many
         * quadwords it has (`quadwords`).
         */
        std::size_t at, quadwords;
    };

    /** The unit handed to the thread: the copied bytes, who each belongs to, and what to run. */
    struct Chunk {
        /** The copied quadwords of all pieces, one after another. */
        std::vector<u8> data;

        /** Which part of `data` belongs to which path, in order. */
        std::vector<Piece> pieces;

        /** Work to run after the pieces, on the GS's side; empty if there is none. */
        std::function<void()> then;
    };

    /** Hands the chunk being filled to the thread, waiting while too much is queued. */
    void hand_over();

    /** The body of the GIF's thread: takes chunks from the queue until told to quit. */
    void loop();

    /** The GIF's thread; not joinable until `start`. */
    std::thread thread_;

    /** Guards `queue_`, `spare_`, `queued_bytes_`, `busy_` and `quit_`. */
    std::mutex mutex_;

    /**
     * Signalled when a chunk is queued or the thread is to quit (`work_`), and when the thread has
     * finished a chunk (`done_`). Both wait on `mutex_`.
     */
    std::condition_variable work_, done_;

    /** Chunks waiting for the thread. Guarded by `mutex_`. */
    std::deque<Chunk> queue_;

    /** Emptied chunks kept to reuse their buffers. Guarded by `mutex_`. */
    std::vector<Chunk> spare_;

    /** Bytes of data in `queue_`. Guarded by `mutex_`. */
    std::size_t queued_bytes_ = 0;

    /**
     * True while the thread is working on a chunk (`busy_`), and true once it is to stop
     * (`quit_`). Guarded by `mutex_`.
     */
    bool busy_ = false, quit_ = false;

    /** The chunk being filled by `write`. Only the feeding thread touches it. */
    Chunk open_;
};

}  // namespace ps2
