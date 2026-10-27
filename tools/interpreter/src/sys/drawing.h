// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

/**
 * The drawing path as a thread: VIF1, VU1, the GIF and the GS fed from a queue of commands.
 *
 * It leaves out the EE's side of the transfers (the DMA controller and the FIFO writes), which
 * calls in from machine.cpp, and the graphics units themselves, which are in `ps2/graphics.h`.
 */

#pragma once

#include <condition_variable>
#include <deque>
#include <mutex>
#include <thread>
#include <vector>

#include "ps2/graphics.h"

namespace sys {

using ps2::u32;
using ps2::u64;
using ps2::u8;

/**
 * The drawing path (VIF1, VU1, the GIF, the GS) on a thread of its own.
 *
 * On the console those run beside the EE: a program hands the DMA controller
 * a display list and carries on with the next frame while the list is drawn.
 * Here the EE's side copies what a channel sends as it sends it, and this
 * thread takes the copies in order. Nothing on the drawing side reads the
 * EE's memory, so the two never touch the same data; whatever the EE's side
 * needs from the drawing side (a register read, pixels sent back, the
 * picture) waits until everything given so far has been done.
 *
 * Until `start` is called everything runs at once on the caller's thread,
 * which is also how it runs when the thread is not wanted.
 *
 * Threads: the EE's thread calls every public method. The drawing thread runs `loop`, which takes
 * commands off `queue_` and calls `run`. The completion callback of `present` runs on
 * whichever thread the GIF does its work on. The queue and its counters
 * are guarded by `mutex_`, the pictures by `picture_mutex_`; the gathering state is the EE's
 * thread's alone.
 *
 * It owns a thread, so its copy operations are deleted. The destructor stops the thread.
 */
class Drawing {
public:
    /**
     * Makes a drawing path in front of a set of graphics units. No thread runs yet.
     *
     * @param graphics The VIF1, VU1, GIF and GS to feed; it must outlive this object.
     */
    explicit Drawing(ps2::Graphics& graphics) : graphics_(graphics) {}

    /**
     * Stops the drawing thread once the commands already given have run, then waits for the GIF.
     */
    ~Drawing();
    Drawing(const Drawing&) = delete;
    Drawing& operator=(const Drawing&) = delete;

    /**
     * Starts the drawing thread and the GIF's thread; later commands are queued to them.
     *
     * Does nothing when the thread already runs.
     */
    void start();

    /** True once `start` has run, that is while commands are drawn on a thread of their own. */
    bool threaded() const { return thread_.joinable(); }

    /**
     * Gathers data that a DMA channel or a FIFO write sends to VIF1.
     *
     * Pieces gather until `send`.
     *
     * @param data The bytes; they are copied.
     * @param bytes Length of `data` in bytes.
     */
    void vif(const u8* data, std::size_t bytes) {
        // A tool asked for a copy of everything VIF1 is given.
        if (vif_copy) {
            vif_copy->insert(vif_copy->end(), data, data + bytes);
        }
        gather(kVif, data, bytes);
    }

    /**
     * Gathers data that goes to the GIF on path 3.
     *
     * Pieces gather until `send`.
     *
     * @param data The bytes; they are copied.
     * @param bytes Length of `data` in bytes, a multiple of 16.
     */
    void gif(const u8* data, std::size_t bytes) { gather(kGif, data, bytes); }

    /** Hands the gathered pieces to the drawing side as one command, in order. */
    void send();

    /**
     * Queues a write to one of the GS's privileged registers.
     *
     * @param address The register's address.
     * @param value The 64-bit value to write.
     */
    void privileged(u32 address, u64 value);

    /**
     * Queues the start of a vertical blank.
     *
     * @param odd_field True when the field that follows is the odd one.
     */
    void vblank(bool odd_field);

    /**
     * Queues taking the picture the display circuits show now.
     *
     * At most two of these wait at a time: with a third, the caller waits, so the EE's side never
     * runs far ahead of what is drawn.
     */
    void present();

    /**
     * For tools: when set, everything given to VIF1 is also appended here.
     *
     * Not guarded: `vif` appends to it on the EE's thread.
     */
    std::vector<u8>* vif_copy = nullptr;

    /** Waits until everything given has been done, then lets the caller read `graphics`. */
    void sync();

    /**
     * Copies out the latest picture taken by `present`.
     *
     * Safe to call while the drawing thread runs.
     *
     * @param[out] out Receives the picture when there is one.
     * @return False if there is none yet or the display was off.
     */
    bool picture(ps2::Image& out);

private:
    /**
     * What a queued command asks for: data for VIF1 (`kVif`), data for the GIF on path 3
     * (`kGif`), a write to a privileged GS register (`kPrivileged`), the start of a vertical blank
     * (`kVblank`) or the picture now shown (`kPresent`).
     */
    enum Kind : u8 {
        kVif,
        kGif,
        kPrivileged,
        kVblank,
        kPresent
    };

    /** One step of the drawing side's work, queued in the order the EE's side gave it. */
    struct Command {
        /** What is asked. */
        Kind kind = kVif;

        /** The register address of a `kPrivileged` command. */
        u32 address = 0;

        /** The value of a `kPrivileged` command, or 1 for an odd field of a `kVblank`. */
        u64 value = 0;

        /** The bytes of a `kVif` or `kGif` command. */
        std::vector<u8> data;
    };

    /**
     * Adds bytes to the command being gathered, first sending it if it is of another kind.
     *
     * @param kind Whether the bytes are for VIF1 or for the GIF.
     * @param data The bytes; they are copied.
     * @param bytes Length of `data` in bytes.
     */
    void gather(Kind kind, const u8* data, std::size_t bytes);

    /**
     * Runs a command now on a caller's thread, or queues it for the drawing thread.
     *
     * @param command The command; it is moved from.
     */
    void give(Command&& command);

    /**
     * Carries out one command on the graphics units.
     *
     * @param command The command to run.
     */
    void run(Command& command);

    /** The drawing thread's body: takes commands in order and runs them until asked to stop. */
    void loop();

    /** The units that draw; not owned. */
    ps2::Graphics& graphics_;

    /** The drawing thread, not joinable until `start`. Joined by the destructor. */
    std::thread thread_;

    /** Guards `queue_`, `spare_`, `busy_`, `quit_` and `presents_waiting_`. */
    std::mutex mutex_;

    /** Signalled when a command is queued or `quit_` is set; the drawing thread waits on it. */
    std::condition_variable work_;

    /** Signalled when a command has run or a present is counted off; `present` and `sync` wait. */
    std::condition_variable done_;

    /** The commands waiting for the drawing thread, oldest first. Guarded by `mutex_`. */
    std::deque<Command> queue_;

    /** Emptied buffers kept to use again. Guarded by `mutex_`. */
    std::vector<std::vector<u8>> spare_;

    /** True while the drawing thread runs a command it took off the queue. Guarded by `mutex_`. */
    bool busy_ = false;

    /** Set to ask the drawing thread to stop once the queue is empty. Guarded by `mutex_`. */
    bool quit_ = false;

    /** Presents queued or running that count toward the limit of two. Guarded by `mutex_`. */
    unsigned presents_waiting_ = 0;

    /** What `vif` and `gif` have gathered since the last `send`. The EE's thread only. */
    Command open_;

    /** True once `open_` holds gathered bytes. The EE's thread only. */
    bool open_used_ = false;

    /** Guards `picture_`, `spare_picture_` and `picture_shown_`. */
    std::mutex picture_mutex_;

    /** The latest picture taken by a present. Guarded by `picture_mutex_`. */
    ps2::Image picture_;

    /** A picture buffer kept to be filled by the next present. Guarded by `picture_mutex_`. */
    ps2::Image spare_picture_;

    /** False while the latest present found the display off. Guarded by `picture_mutex_`. */
    bool picture_shown_ = false;
};

}  // namespace sys
