// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

/**
 * The GIF declared in gif.h: packet decoding, and the thread that can run it.
 *
 * Sources: the GIF tag and the PACKED, REGLIST and IMAGE layouts as publicly documented.
 */

#include "gif.h"

#include <algorithm>

#include "fp_quad.h"

namespace ps2 {
namespace {

/** The FLG field of a GIF tag, bits 58-59: how the data after the tag is laid out (documented). */
enum : u32 {
    kPacked = 0,
    kReglist = 1,
    kImage = 2,
    kImage2 = 3  // also IMAGE
};

}  // namespace

void Gif::reset() {
    paths_ = {};
}

void Gif::advance(Path& p) {
    // The last register of the loop is done: the next quadword starts a new loop.
    if (++p.reg == p.nreg) {
        p.reg = 0;
        p.loops--;
    }
}

void Gif::packed(Path& p, u64 lo, u64 hi) {
    // The tag's REGS field holds one 4-bit descriptor for each register of the loop.
    u32 desc = static_cast<u32>((p.regs >> (p.reg * 4)) & 0xF);

    switch (desc) {
        // PRIM is 11 bits wide.
        case gsreg::PRIM:
            gs_.write(gsreg::PRIM, lo & 0x7FF);
            break;

        // R is bits 0-7, G bits 32-39, B bits 64-71, A bits 96-103 (documented).
        case gsreg::RGBAQ: {
            // One colour component per word; Q is the one the last ST carried.
            u64 rgba = (lo & 0xFF) | ((lo >> 32 & 0xFF) << 8) | ((hi & 0xFF) << 16)
                       | ((hi >> 32 & 0xFF) << 24);
            gs_.write(gsreg::RGBAQ, rgba | (static_cast<u64>(as_u32(p.q)) << 32));
            break;
        }

        // S and T are the low half as the register has them; Q is the float in the high half.
        case gsreg::ST:
            gs_.write(gsreg::ST, lo);
            p.q = as_float(static_cast<u32>(hi));
            break;

        // U is bits 0-13 and V bits 32-45; the register has them at bits 0-13 and 16-29.
        case gsreg::UV:
            gs_.write(gsreg::UV, (lo & 0x3FFF) | ((lo >> 32 & 0x3FFF) << 16));
            break;

        case gsreg::XYZF2: {
            /*
             * X bits 0-15, Y bits 32-47, Z bits 68-91, F bits 100-107, ADC bit 111; the register
             * has X, Y at bits 0-31, Z at 32-55 and F at 56-63 (documented).
             */
            u64 xy = (lo & 0xFFFF) | ((lo >> 32 & 0xFFFF) << 16);
            u64 z = (hi >> 4) & 0xFFFFFF, f = (hi >> 36) & 0xFF;
            bool no_kick = (hi >> 47) & 1;

            // With ADC set the vertex goes to the register that does not start a drawing kick.
            gs_.write(no_kick ? gsreg::XYZF3 : gsreg::XYZF2, xy | (z << 32) | (f << 56));
            break;
        }

        case gsreg::XYZ2: {
            // X bits 0-15, Y bits 32-47, Z bits 64-95, ADC bit 111 (documented).
            u64 xy = (lo & 0xFFFF) | ((lo >> 32 & 0xFFFF) << 16);
            bool no_kick = (hi >> 47) & 1;

            // With ADC set the vertex goes to the register that does not start a drawing kick.
            gs_.write(no_kick ? gsreg::XYZ3 : gsreg::XYZ2, xy | ((hi & 0xFFFFFFFFu) << 32));
            break;
        }

        // F is bits 100-107, at bits 56-63 of the register (documented).
        case gsreg::FOG:
            gs_.write(gsreg::FOG, ((hi >> 36) & 0xFF) << 56);
            break;

        // The address is in bits 64-71 and the data in the low half.
        case 0xE:  // A+D: any register, by address
            gs_.write(static_cast<u8>(hi & 0xFF), lo);
            break;

        // No operation.
        case 0xF:
            break;

        /*
         * Any other descriptor is the number of a register whose packed form is its own 64 bits,
         * such as TEX0_1: the low half goes in as it is.
         */
        default:
            gs_.write(static_cast<u8>(desc), lo);
            break;
    }
}

namespace {

/**
 * How much is copied before the thread is given it (`kChunkBytes`), and how far ahead of the
 * thread the writer may get (`kMostQueuedBytes`).
 */
constexpr std::size_t kChunkBytes = 64 * 1024, kMostQueuedBytes = 16 * 1024 * 1024;

}  // namespace

Gif::~Gif() {
    // Only a started GIF has a thread to stop.
    if (thread_.joinable()) {
        // What was written but not yet handed over is drawn before the thread stops.
        hand_over();
        {
            std::lock_guard lock(mutex_);
            quit_ = true;
        }

        // Notify after the lock is released, so the thread does not wake only to block on it.
        work_.notify_all();
        thread_.join();
    }
}

void Gif::start() {
    // A second call would start a second thread.
    if (!thread_.joinable()) {
        thread_ = std::thread([this] { loop(); });
    }
}

std::size_t Gif::packet_quadwords(const u8* memory, u32 at, u32 mask) {
    std::size_t total = 0;

    // Tag after tag, until one has EOP; a packet without it runs on to the memory's size.
    while (total <= mask) {
        // The tag sits `total` quadwords on, wrapping at the end of the memory.
        u64 lo = load<u64>(memory + ((at + total) & mask) * 16);
        total++;

        // NLOOP is bits 0-14 and NREG bits 60-63; NREG 0 means 16 registers (documented).
        u64 loops = bits(lo, 0, 15), nreg = bits(lo, 60, 4);
        if (nreg == 0) {
            nreg = 16;
        }

        /*
         * FLG is bits 58-59. The data after the tag is a quadword for each register of each loop in
         * PACKED, two registers to a quadword in REGLIST, and NLOOP quadwords in IMAGE.
         */
        switch (bits(lo, 58, 2)) {
            case kPacked:
                total += loops * nreg;
                break;

            // Rounded up, as a loop that ends on an odd register still fills its quadword.
            case kReglist:
                total += (loops * nreg + 1) / 2;
                break;

            default:
                total += loops;
                break;
        }

        // EOP, bit 15, marks the last tag of the packet.
        if (bits(lo, 15, 1)) {
            break;
        }
    }

    return std::min<std::size_t>(total, std::size_t{mask} + 1);
}

void Gif::write(int path, const u8* data, std::size_t quadwords) {
    // Without a thread the packet is taken apart here and now.
    if (!thread_.joinable()) {
        take(path, data, quadwords);
        return;
    }

    // Nothing to copy.
    if (quadwords == 0) {
        return;
    }

    // Copy the data, because the caller's buffer is reused as soon as this returns.
    std::size_t at = open_.data.size();
    open_.data.insert(open_.data.end(), data, data + quadwords * 16);

    // Data for the same path as the last piece extends that piece, to keep the pieces few.
    if (!open_.pieces.empty() && open_.pieces.back().path == path) {
        open_.pieces.back().quadwords += quadwords;
    } else {
        open_.pieces.push_back(Piece{path, at, quadwords});
    }

    // A chunk this big is worth the thread's attention.
    if (open_.data.size() >= kChunkBytes) {
        hand_over();
    }
}

void Gif::run(std::function<void()> what) {
    // Without a thread nothing is pending, so "after everything so far" is now.
    if (!thread_.joinable()) {
        what();
        return;
    }

    // The work rides on the open chunk, so it runs after that chunk's data.
    open_.then = std::move(what);
    hand_over();
}

void Gif::hand_over() {
    // Nothing to hand over.
    if (open_.data.empty() && !open_.then) {
        return;
    }

    {
        std::unique_lock lock(mutex_);

        // The writer may not get further ahead of the thread than this.
        done_.wait(lock, [this] { return queued_bytes_ < kMostQueuedBytes; });
        queued_bytes_ += open_.data.size();
        queue_.push_back(std::move(open_));

        // Take an emptied chunk back, to reuse its buffers, or start a new one.
        if (!spare_.empty()) {
            open_ = std::move(spare_.back());
            spare_.pop_back();
        } else {
            open_ = Chunk{};
        }
    }

    // Notify after the lock is released, so the thread does not wake only to block on it.
    work_.notify_one();
}

void Gif::sync() {
    // Without a thread everything was done by `write`.
    if (!thread_.joinable()) {
        return;
    }

    hand_over();
    std::unique_lock lock(mutex_);
    done_.wait(lock, [this] { return queue_.empty() && !busy_; });
}

void Gif::loop() {
    std::unique_lock lock(mutex_);

    // Runs until `quit_` is set and the queue has been taken empty.
    for (;;) {
        work_.wait(lock, [this] { return quit_ || !queue_.empty(); });

        // Told to quit, with nothing left to do.
        if (queue_.empty()) {
            return;
        }

        Chunk chunk = std::move(queue_.front());
        queue_.pop_front();
        busy_ = true;

        // The chunk is this thread's now: drawing it needs no lock, and the writer can queue more.
        lock.unlock();

        for (const Piece& piece : chunk.pieces) {
            take(piece.path, chunk.data.data() + piece.at, piece.quadwords);
        }

        // Work asked for with `run` comes after the data written before it.
        if (chunk.then) {
            chunk.then();
            chunk.then = nullptr;
        }

        lock.lock();
        busy_ = false;
        queued_bytes_ -= chunk.data.size();
        chunk.data.clear();
        chunk.pieces.clear();

        // Keep up to 16 emptied chunks for `hand_over` to reuse.
        if (spare_.size() < 16) {
            spare_.push_back(std::move(chunk));
        }

        // Wakes a writer waiting for room and `sync` waiting for the queue to drain.
        done_.notify_all();
    }
}

void Gif::take(int path, const u8* data, std::size_t quadwords) {
    fp::want_nearest();  // the GS computes as the host does; a vector unit may have been running
    Path& p = paths_[path - 1];

    // Each pass takes one tag or one run of data; it ends when the quadwords are used up.
    while (quadwords) {
        // A path with no loops left in its packet expects a tag next.
        if (p.loops == 0) {
            // A tag.
            u64 lo = load<u64>(data), hi = load<u64>(data + 8);
            data += 16;
            quadwords--;

            /*
             * GIF tag, low 64 bits: NLOOP bits 0-14, EOP bit 15, FLG bits 58-59, NREG bits 60-63.
             * The high 64 bits are REGS, 16 descriptors of 4 bits (documented).
             */
            p.loops = static_cast<u32>(bits(lo, 0, 15));
            p.eop = bits(lo, 15, 1) != 0;
            p.flg = static_cast<u32>(bits(lo, 58, 2));
            p.nreg = static_cast<u32>(bits(lo, 60, 4));

            // NREG 0 means 16 registers.
            if (p.nreg == 0) {
                p.nreg = 16;
            }

            p.regs = hi;
            p.reg = 0;
            p.q = 1.0f;

            // PRE, bit 46, asks for the tag's PRIM field, bits 47-57, to be written to PRIM.
            if (p.flg == kPacked && bits(lo, 46, 1)) {
                gs_.write(gsreg::PRIM, bits(lo, 47, 11));
            }

            // A tag with NLOOP 0 and EOP is a whole packet with no data.
            p.in_packet = !(p.loops == 0 && p.eop);

            // The data of the tag comes next; with NLOOP 0 the next quadword is a tag again.
            continue;
        }

        switch (p.flg) {
            // One register for each quadword.
            case kPacked:
                packed(p, load<u64>(data), load<u64>(data + 8));
                advance(p);
                data += 16;
                quadwords--;
                break;

            // Two registers for each quadword, 64 bits each, until the loops run out.
            case kReglist:
                for (int half = 0; half < 2 && p.loops; half++) {
                    u32 desc = static_cast<u32>((p.regs >> (p.reg * 4)) & 0xF);

                    // Descriptors 0xE and 0xF name no register here: the half is skipped.
                    if (desc < 0xE) {
                        gs_.write(static_cast<u8>(desc), load<u64>(data + half * 8));
                    }

                    advance(p);
                }
                data += 16;
                quadwords--;
                break;

            // IMAGE: NLOOP quadwords of pixels, in as many pieces as they arrive in.
            default: {
                std::size_t n = std::min<std::size_t>(p.loops, quadwords);
                gs_.transfer_in(data, n * 16);
                p.loops -= static_cast<u32>(n);
                data += n * 16;
                quadwords -= n;
                break;
            }
        }

        // The packet's data is complete: the path is between packets if that tag had EOP.
        if (p.loops == 0) {
            p.in_packet = !p.eop;
        }
    }
}

}  // namespace ps2
