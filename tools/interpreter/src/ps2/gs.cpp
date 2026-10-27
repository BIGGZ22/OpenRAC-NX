// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

/**
 * The Graphics Synthesizer declared in gs.h: registers, primitive assembly, the rasterisers, the
 * pixel pipeline, texturing, the colour table, transfers and the display read-out.
 *
 * Coordinates are in the GS's 12.4 fixed point until a rasteriser turns them into pixels. A
 * primitive is drawn when its last vertex arrives, or gathered into a batch and drawn later,
 * possibly by other threads (see the class comment in gs.h). It leaves out edge antialiasing,
 * dithering, the scan line mask and the feedback write, which are reported once when met.
 *
 * Sample points are at whole pixel coordinates. A sprite covers the pixels
 * from its first corner up to, not including, its second; a triangle does
 * not draw its right and bottom edges.
 *
 * Sources: the GS's registers, formats and drawing rules as publicly documented.
 */

#include "gs.h"

#include <algorithm>
#include <bit>
#include <climits>
#include <cmath>
#include <condition_variable>
#include <cstdio>
#include <thread>

#include "fp_quad.h"

namespace ps2 {
namespace {

// The register numbers of `gsreg` are used by their bare names below.
using namespace gsreg;

/** The primitive types of the PRIM register, field PRIM (documented). */
enum : u32 {
    kPoint,
    kLine,
    kLineStrip,  // LINESTRIP
    kTriangle,
    kTriangleStrip,  // TRISTRIP
    kTriangleFan,    // TRIFAN
    kSprite
};

/** Masks of a 32-bit colour: the alpha byte (`kA`) and the three colour bytes (`kRgb`). */
constexpr u32 kA = 0xFF000000u, kRgb = 0x00FFFFFFu;

/** How a stored texel becomes a colour. */
enum : unsigned {
    kTex32,
    kTex24,
    kTex16,
    kTex8,
    kTex4,
    kTex8H,
    kTex4HL,
    kTex4HH
};

/**
 * Returns one byte of a colour.
 *
 * @param rgba A colour with R in the low byte.
 * @param n Channel number: 0 R, 1 G, 2 B, 3 A.
 * @return The channel, 0-255.
 */
inline u32 ch(u32 rgba, unsigned n) {
    return (rgba >> (n * 8)) & 0xFF;
}

/**
 * Builds a colour from its channels.
 *
 * @param r Red, 0-255.
 * @param g Green, 0-255.
 * @param b Blue, 0-255.
 * @param a Alpha, 0-255.
 * @return The colour with R in the low byte.
 */
inline u32 pack(u32 r, u32 g, u32 b, u32 a) {
    return r | (g << 8) | (b << 16) | (a << 24);
}

/**
 * Returns the smallest pixel whose sample point is at or after a 12.4 coordinate.
 *
 * @param v A coordinate in 12.4 fixed point.
 * @return The pixel.
 */
inline s32 ceil16(s64 v) {
    // Sixteen units a pixel: add 15 and divide by 16, rounding down.
    return static_cast<s32>((v + 15) >> 4);
}

/**
 * Extends the sign of a field of `width` bits to 32 bits.
 *
 * @param v The value; only its low `width` bits are used.
 * @param width Number of bits in the field.
 * @return The signed value.
 */
inline s32 sign_extend(u32 v, unsigned width) {
    // Flipping the sign bit, then subtracting it, is the standard way to extend it.
    u32 m = 1u << (width - 1);
    return static_cast<s32>((v ^ m) - m);
}

/**
 * Tells whether a format is in the 16-bit layout, which includes the Z formats used as frame
 * buffers.
 *
 * @param psm A pixel storage format.
 * @return True for PSMCT16, PSMCT16S, PSMZ16 and PSMZ16S.
 */
inline bool is16(u32 psm) {
    // The low nibble is 2 or 0xA for the four of them (documented).
    return (psm & 0xF) == 2 || (psm & 0xF) == 0xA;
}

/**
 * Tells whether a format is a 24-bit one.
 *
 * @param psm A pixel storage format.
 * @return True for PSMCT24 and PSMZ24.
 */
inline bool is24(u32 psm) {
    // The low nibble is 1 for both.
    return (psm & 0xF) == 1;
}

/**
 * Applies the alpha test.
 *
 * @param test The TEST ATST field: how the alpha is compared with the reference.
 * @param a The pixel's alpha.
 * @param ref The reference value, TEST AREF.
 * @return Whether the pixel passes.
 */
inline bool alpha_passes(u32 test, u32 a, u32 ref) {
    switch (test) {
        case 0:  // NEVER
            return false;
        case 1:  // ALWAYS
            return true;
        case 2:  // LESS
            return a < ref;
        case 3:  // LEQUAL
            return a <= ref;
        case 4:  // EQUAL
            return a == ref;
        case 5:  // GEQUAL
            return a >= ref;
        case 6:  // GREATER
            return a > ref;
        default:  // NOTEQUAL
            return a != ref;
    }
}

/**
 * Blends two colours channel by channel.
 *
 * @param c0 The colour at f = 0.
 * @param c1 The colour at f = 1.
 * @param f The weight of `c1`, 0 to 1.
 * @return The mix, each channel rounded to the nearest value.
 */
inline u32 lerp_colour(u32 c0, u32 c1, float f) {
    u32 out = 0;

    for (unsigned n = 0; n < 4; n++) {
        float a = static_cast<float>(ch(c0, n)), b = static_cast<float>(ch(c1, n));

        // Adding 0.5 before the truncation rounds to nearest.
        out |= static_cast<u32>(a + (b - a) * f + 0.5f) << (n * 8);
    }

    return out;
}

/**
 * Pixels drawn by this thread since it last reported them. A thread adds it to `pixels_` and
 * clears it after each batch, band or directly drawn primitive.
 */
thread_local u64 tls_pixels = 0;

}  // namespace

/**
 * Threads that share out the bands of a batch. `run` returns when every
 * item is done and every thread has let go of the job.
 *
 * `run` is called by the thread that draws batches. The pool's own threads call `loop`. The job
 * and the counts that go with it are guarded by `mutex_`; the item counters are atomic.
 */
class Gs::Pool {
public:
    /**
     * Starts the threads.
     *
     * @param threads How many threads to start; they wait for the first job.
     */
    explicit Pool(unsigned threads) {
        for (unsigned n = 0; n < threads; n++) {
            threads_.emplace_back([this] { loop(); });
        }
    }

    /** Tells the threads to quit and waits for them. */
    ~Pool() {
        {
            std::lock_guard lock(mutex_);
            quit_ = true;
        }

        // Notify after the lock is released, so the threads do not wake only to block on it.
        start_.notify_all();
        for (std::thread& t : threads_) {
            t.join();
        }
    }

    /**
     * Runs a job for each item from 0 to count - 1, spread over the pool's threads and the caller.
     *
     * @param count Number of items.
     * @param job Called once for each item number, on any of the threads.
     */
    void run(unsigned count, const std::function<void(unsigned)>& job) {
        // Publish the job and reset the counters under the lock, so no thread sees half of it.
        {
            std::lock_guard lock(mutex_);
            job_ = &job;
            count_ = count;
            next_.store(0);
            left_.store(count);
            generation_++;
        }

        start_.notify_all();

        // The caller takes items too, instead of only waiting.
        work(job, count);

        // Every item is done and no thread still holds the job, which is about to go.
        std::unique_lock lock(mutex_);
        done_.wait(lock, [this] { return left_.load() == 0 && busy_ == 0; });
        job_ = nullptr;
    }

private:
    /**
     * Takes items one after another until none is left.
     *
     * @param job The job.
     * @param count Number of items.
     */
    void work(const std::function<void(unsigned)>& job, unsigned count) {
        // Ends when another thread has taken the last item.
        for (;;) {
            unsigned item = next_.fetch_add(1);

            // No item left to take.
            if (item >= count) {
                break;
            }

            job(item);
            left_.fetch_sub(1);
        }
    }

    /** The body of a pool thread: waits for a job, works on it, waits for the next. */
    void loop() {
        // The generation of the last job this thread worked on.
        u64 seen = 0;

        // Ends when the pool is destroyed.
        for (;;) {
            const std::function<void(unsigned)>* job;
            unsigned count;
            {
                std::unique_lock lock(mutex_);

                // Wait for a job this thread has not seen, or for the end.
                start_.wait(lock, [&] { return quit_ || (generation_ != seen && job_); });

                // The pool is being destroyed.
                if (quit_) {
                    return;
                }

                seen = generation_;
                job = job_;
                count = count_;
                busy_++;
            }

            work(*job, count);
            {
                std::lock_guard lock(mutex_);
                busy_--;
            }

            done_.notify_one();
        }
    }

    /** The pool's threads. */
    std::vector<std::thread> threads_;

    /** Guards `job_`, `count_`, `busy_`, `generation_` and `quit_`. */
    std::mutex mutex_;

    /**
     * Signalled when a job is published or the pool quits (`start_`), and when a thread lets go
     * of a job (`done_`).
     */
    std::condition_variable start_, done_;

    /** The job being run, or null. Guarded by `mutex_`. */
    const std::function<void(unsigned)>* job_ = nullptr;

    /**
     * The number of items in the job (`count_`), and the threads working on it (`busy_`).
     * Guarded by `mutex_`.
     */
    unsigned count_ = 0, busy_ = 0;

    /**
     * The next item to take (`next_`), and the items not yet finished (`left_`). Atomic; any
     * thread.
     */
    std::atomic<unsigned> next_{0}, left_{0};

    /** Counts jobs, so a thread can tell a new one from one it has done. Guarded by `mutex_`. */
    u64 generation_ = 0;

    /** Set when the pool is to quit. Guarded by `mutex_`. */
    bool quit_ = false;
};

/**
 * The thread that draws batches, one after the other, in the order given.
 *
 * The gathering thread calls `give`, `wait` and `idle`. The raster thread runs `loop` and, with
 * it, `Gs::render`. The queue and the flags are guarded by `mutex_`.
 */
class Gs::Raster {
public:
    /**
     * Starts the thread.
     *
     * @param gs The GS whose batches the thread draws.
     */
    explicit Raster(Gs& gs) : gs_(gs), thread_([this] { loop(); }) {}

    /** Tells the thread to quit, after it has drawn what is queued, and waits for it. */
    ~Raster() {
        {
            std::lock_guard lock(mutex_);
            quit_ = true;
        }

        // Notify after the lock is released, so the thread does not wake only to block on it.
        work_.notify_all();
        thread_.join();
    }

    /**
     * Takes the batch; gives back an empty one to gather into.
     *
     * Waits while too many batches are queued.
     *
     * @param batch The batch to draw; the raster thread owns it from now on.
     * @return An empty batch, reused from a drawn one when there is one.
     */
    std::unique_ptr<Batch> give(std::unique_ptr<Batch> batch) {
        std::unique_ptr<Batch> next;
        {
            std::unique_lock lock(mutex_);
            done_.wait(lock, [this] { return queue_.size() < 8; });  // no further ahead than this
            queue_.push_back(std::move(batch));

            // A drawn batch is reused if there is one, to keep its buffers.
            if (!spare_.empty()) {
                next = std::move(spare_.back());
                spare_.pop_back();
            }
        }

        // Notify after the lock is released, so the thread does not wake only to block on it.
        work_.notify_one();
        return next ? std::move(next) : std::make_unique<Batch>();
    }

    /** Waits until every batch given has been drawn. */
    void wait() {
        std::unique_lock lock(mutex_);
        done_.wait(lock, [this] { return queue_.empty() && !busy_; });
    }

    /**
     * Tells whether nothing is queued or being drawn.
     *
     * @return True when every batch given has been drawn.
     */
    bool idle() {
        std::lock_guard lock(mutex_);
        return queue_.empty() && !busy_;
    }

private:
    /** The body of the raster thread: draws queued batches in order until told to quit. */
    void loop() {
        std::unique_lock lock(mutex_);

        // Ends when `quit_` is set and the queue is empty.
        for (;;) {
            work_.wait(lock, [this] { return quit_ || !queue_.empty(); });

            // Told to quit, with nothing left to draw.
            if (queue_.empty()) {
                return;
            }

            std::unique_ptr<Batch> batch = std::move(queue_.front());
            queue_.pop_front();
            busy_ = true;

            // Drawing needs no lock, and the gathering thread can queue more meanwhile.
            lock.unlock();
            gs_.render(*batch);
            batch->clear();
            lock.lock();
            busy_ = false;

            // Keep up to 8 drawn batches for `give` to reuse.
            if (spare_.size() < 8) {
                spare_.push_back(std::move(batch));
            }

            done_.notify_all();
        }
    }

    /** The GS whose batches are drawn. */
    Gs& gs_;

    /** Guards `queue_`, `spare_`, `busy_` and `quit_`. */
    std::mutex mutex_;

    /**
     * Signalled when a batch is queued or the thread is to quit (`work_`), and when one has been
     * drawn (`done_`).
     */
    std::condition_variable work_, done_;

    /** Batches waiting to be drawn. Guarded by `mutex_`. */
    std::deque<std::unique_ptr<Batch>> queue_;

    /** Drawn batches kept for reuse. Guarded by `mutex_`. */
    std::vector<std::unique_ptr<Batch>> spare_;

    /**
     * True while a batch is being drawn (`busy_`), and once the thread is to quit (`quit_`).
     * Guarded by `mutex_`.
     */
    bool busy_ = false, quit_ = false;

    /** The raster thread; last: it starts using the rest at once. */
    std::thread thread_;
};

void Gs::Batch::clear() {
    serial = false;
    task = nullptr;
    primitives.clear();

    // Only the bands that were used have anything to empty.
    for (u16 band : used_bands) {
        bands[band].clear();
    }

    used_bands.clear();
    envs.clear();
    cluts.clear();
    textures.clear();
}

namespace {

/** A band is 16 scan lines: the unit the pool shares out. */
constexpr unsigned kBandShift = 4;

}  // namespace

Gs::Gs() {
    reset();
}

Gs::~Gs() = default;

void Gs::set_threads(unsigned threads) {
    // What is gathered is drawn before the threads that would draw it change.
    finish();
    raster_.reset();
    pool_.reset();
    threads_ = threads;

    // Two or more threads: one draws batches in order, the rest share out the bands of each.
    if (threads > 1) {
        pool_ =
            std::make_unique<Pool>(threads - 1);  // the thread that draws batches is one of them
        raster_ = std::make_unique<Raster>(*this);
    }
}

void Gs::reset() {
    // Power-on state: registers and the colour table zero.
    reg_.fill(0);
    priv_.fill(0);
    csr_ = imr_ = busdir_ = siglblid_ = 0;

    // PRMODECONT starts by taking the attributes from PRIM, COLCLAMP with clamping on (assumed).
    reg_[PRMODECONT] = 1;
    reg_[COLCLAMP] = 1;
    count_ = 0;
    fog_ = 0;
    clut_raw_.fill(0);
    clut_.fill(0);
    clut_psm_ = PSMCT32;
    clut_cbp_.fill(0);
    in_ = Transfer{};
    out_ = Transfer{};

    // Batches already handed over finish before the state they use is dropped.
    if (raster_) {
        raster_->wait();
    }

    // Nothing decoded survives, and no page counts as written.
    texture_cache_.clear();
    page_stamp_.fill(0);
    clock_ = 1;

    // A fresh batch with no pending work, and no current state.
    batch_ = std::make_unique<Batch>();
    clut_load_ = ClutLoad{};
    pending_write_.clear();
    pending_read_.clear();
    pending_colour_.clear();
    pending_depth_.clear();
    inflight_write_.clear();
    inflight_read_.clear();
    env_ = nullptr;
    env_dirty_ = clut_copy_dirty_ = true;
}

void Gs::note(u32 what, const char* text) {
    // Reported before: say nothing more.
    if (todo & what) {
        return;
    }

    todo |= what;
    std::fprintf(stderr, "gs: not modelled yet: %s\n", text);
}

// --- Registers ---

void Gs::write(u8 reg, u64 data) {
    // Past the end of the register file: nothing lives there.
    if (reg >= reg_.size()) {
        return;
    }

    // Anything but a vertex's own registers changes what the next primitive is
    // drawn with.
    switch (reg) {
        case RGBAQ:
        case ST:
        case UV:
        case XYZF2:
        case XYZ2:
        case XYZF3:
        case XYZ3:
        case FOG:
            break;

        default:
            env_dirty_ = true;
            break;
    }

    switch (reg) {
        // A new PRIM starts a new primitive: the vertices queued so far are dropped.
        case PRIM:
            reg_[reg] = data;
            count_ = 0;
            break;

        // XYZF: X bits 0-15, Y bits 16-31, Z bits 32-55, F bits 56-63 (documented).
        case XYZF2:
        case XYZF3:
            fog_ = static_cast<u8>(bits(data, 56, 8));

            // Only XYZF2 gives a drawing kick; XYZF3 queues the vertex without drawing.
            vertex(
                static_cast<u16>(bits(data, 0, 16)),
                static_cast<u16>(bits(data, 16, 16)),
                static_cast<u32>(bits(data, 32, 24)),
                reg == XYZF2
            );
            break;

        // XYZ: X bits 0-15, Y bits 16-31, Z bits 32-63; only XYZ2 kicks (documented).
        case XYZ2:
        case XYZ3:
            vertex(
                static_cast<u16>(bits(data, 0, 16)),
                static_cast<u16>(bits(data, 16, 16)),
                static_cast<u32>(data >> 32),
                reg == XYZ2
            );
            break;

        // FOG: F is bits 56-63 (documented).
        case FOG:
            reg_[reg] = data;
            fog_ = static_cast<u8>(bits(data, 56, 8));
            break;

        case TEX0_1:
        case TEX0_2:
            reg_[reg] = data;

            // TEX1 of the same context, bit 9: MTBA.
            if (bits(reg_[TEX1_1 + (reg - TEX0_1)], 9, 1)) {
                /*
                 * MTBA: the first three mipmap levels follow the texture in memory,
                 * each a square of the larger side, packed one after the other at
                 * half the buffer width of the one before.
                 */

                // TEX0: TBP0 bits 0-13, TBW 14-19, PSM 20-25, TW 26-29, TH 30-33 (documented).
                u32 bp = static_cast<u32>(bits(data, 0, 14)),
                    bw = static_cast<u32>(bits(data, 14, 6));
                u32 side = std::max(1u << bits(data, 26, 4), 1u << bits(data, 30, 4));
                u32 bpp = transfer_bits(static_cast<u32>(bits(data, 20, 6)));

                // These formats sit in the top of a 32-bit pixel, which takes the room.
                if (bits(data, 20, 6) == PSMT8H) {
                    bpp = 32;
                }
                if (bits(data, 20, 6) == PSMT4HL || bits(data, 20, 6) == PSMT4HH) {
                    bpp = 32;
                }

                // MIPTBP1 holds three levels, each a TBP of 14 bits and a TBW of 6, 20 bits apart.
                u64 mip = 0;
                for (unsigned level = 0; level < 3; level++) {
                    // The level starts after the one before, in blocks of 256 bytes, rounded up.
                    bp += ((side * side * bpp >> 3) + 255) >> 8;
                    bw = std::max(bw >> 1, 1u);
                    side = std::max(side >> 1, 1u);
                    mip |= (static_cast<u64>(bp & 0x3FFF) | (static_cast<u64>(bw) << 14))
                           << (level * 20);
                }
                reg_[MIPTBP1_1 + (reg - TEX0_1)] = mip;
            }

            load_clut(data);
            break;

        case TEX2_1:
        case TEX2_2: {
            /*
             * TEX2 carries the format and colour table fields of TEX0 only. The mask is PSM,
             * bits 20-25, and the colour table fields, bits 37-63 (documented).
             */
            const u64 mask = (u64{0x3F} << 20) | (~u64{0} << 37);
            u64& tex0 = reg_[TEX0_1 + (reg - TEX2_1)];
            tex0 = (tex0 & ~mask) | (data & mask);
            reg_[reg] = data;
            load_clut(tex0);
            break;
        }

        // New alpha values change the colours the table gives.
        case TEXA:
            reg_[reg] = data;
            rebuild_clut();
            break;

        // Writing the direction starts the transfer that BITBLTBUF, TRXPOS and TRXREG describe.
        case TRXDIR:
            reg_[reg] = data;
            start_transfer();
            break;

        case SIGNAL: {
            // SIGID is bits 0-31 and IDMSK bits 32-63; only the masked bits of the id change.
            u32 id = static_cast<u32>(data), mask = static_cast<u32>(data >> 32);
            siglblid_ = (siglblid_ & ~u64{mask}) | (id & mask);

            // CSR bit 0 is the SIGNAL event.
            csr_ |= 1;
            break;
        }

        // CSR bit 1 is the FINISH event.
        case FINISH:
            csr_ |= 2;
            break;

        case LABEL: {
            // LBLID is bits 0-31 and IDMSK bits 32-63; it lands in the high half of SIGLBLID.
            u64 id = data & 0xFFFFFFFFu, mask = data >> 32;
            siglblid_ = (siglblid_ & ~(mask << 32)) | ((id & mask) << 32);
            break;
        }

        // The mask itself is not modelled; asking for it is reported once.
        case SCANMSK:
            reg_[reg] = data;
            if (data & 3) {
                note(gstodo::SCANMASK, "scan line mask (SCANMSK)");
            }
            break;

        // DTHE bit 0 turns dithering on, which is not modelled; reported once.
        case DTHE:
            reg_[reg] = data;
            if (data & 1) {
                note(gstodo::DITHER, "dithering (DTHE)");
            }
            break;

        // Every other register is stored and read when a primitive needs it.
        default:
            reg_[reg] = data;
            break;
    }
}

void Gs::write_privileged(u32 address, u64 data) {
    switch (address) {
        case gspriv::CSR:
            // Writing 1 to an event bit clears it; bit 9 resets the GS.
            if (data & 0x200) {
                reset();
            }
            csr_ &= ~(data & 0x1F);
            break;

        case gspriv::IMR:
            imr_ = data;
            break;

        case gspriv::BUSDIR:
            busdir_ = data;
            break;

        case gspriv::SIGLBLID:
            siglblid_ = data;
            break;

        // PMODE to BGCOLOR are kept for the display read-out; other addresses are dropped.
        default:
            if (address >= gspriv::PMODE && address <= gspriv::BGCOLOR) {
                priv_[(address >> 4) & 0xF] = data;

                // EXTWRITE bit 0 starts the feedback write, which is not modelled.
                if (address == gspriv::EXTWRITE && (data & 1)) {
                    note(gstodo::FEEDBACK_WRITE, "feedback write (EXTWRITE)");
                }
            }
            break;
    }
}

u64 Gs::read_privileged(u32 address) const {
    switch (address) {
        case gspriv::CSR:
            // Revision 0x1B, id 0x55, FIFO empty (bit 14).
            return csr_ | (u64{0x1B} << 16) | (u64{0x55} << 24) | (u64{1} << 14);

        case gspriv::IMR:
            return imr_;

        case gspriv::BUSDIR:
            return busdir_;

        case gspriv::SIGLBLID:
            return siglblid_;

        // The stored registers from PMODE to BGCOLOR; anything else reads as 0.
        default:
            if (address >= gspriv::PMODE && address <= gspriv::BGCOLOR) {
                return priv_[(address >> 4) & 0xF];
            }
            return 0;
    }
}

u32 Gs::prim_bits() const {
    // PRMODECONT chooses whether the attributes come from PRIM or PRMODE; the
    // primitive type always comes from PRIM.
    if (reg_[PRMODECONT] & 1) {
        return static_cast<u32>(reg_[PRIM]);
    }

    // The type is PRIM bits 0-2; the attributes are the bits above them.
    return static_cast<u32>((reg_[PRMODE] & ~u64{7}) | (reg_[PRIM] & 7));
}

Gs::Env Gs::environment() {
    Env e;
    u32 prim = prim_bits();

    // PRIM bit 9, CTXT, picks context 1 or 2; the registers of context 2 follow those of 1.
    unsigned ctx = (prim >> 9) & 1;

    // FRAME: FBP bits 0-8 in units of 32 blocks, FBW bits 16-21, PSM bits 24-29, FBMSK bits 32-63.
    u64 frame = reg_[FRAME_1 + ctx];
    e.fbp = static_cast<u32>(bits(frame, 0, 9)) * 32;
    e.fbw = static_cast<u32>(bits(frame, 16, 6));
    e.fpsm = static_cast<u32>(bits(frame, 24, 6));
    e.fbmsk = static_cast<u32>(frame >> 32);

    // ZBUF: ZBP bits 0-8 in units of 32 blocks, PSM bits 24-27, ZMSK bit 32 (documented).
    u64 zbuf = reg_[ZBUF_1 + ctx];
    e.zbp = static_cast<u32>(bits(zbuf, 0, 9)) * 32;

    // The Z formats are 0x30 plus the field.
    e.zpsm = 0x30 | static_cast<u32>(bits(zbuf, 24, 4));
    e.zmsk = bits(zbuf, 32, 1) != 0;

    // The largest depth value the format holds: 16, 24 or 32 bits.
    e.zmax = is16(e.zpsm) ? 0xFFFFu : is24(e.zpsm) ? 0xFFFFFFu : 0xFFFFFFFFu;

    // XYOFFSET: OFX bits 0-15, OFY bits 32-47, in 12.4 fixed point.
    u64 offset = reg_[XYOFFSET_1 + ctx];
    e.ofx = static_cast<s32>(bits(offset, 0, 16));
    e.ofy = static_cast<s32>(bits(offset, 32, 16));

    // SCISSOR: X0 bits 0-10, X1 bits 16-26, Y0 bits 32-42, Y1 bits 48-58 (documented).
    u64 scissor = reg_[SCISSOR_1 + ctx];
    e.sx0 = static_cast<s32>(bits(scissor, 0, 11));
    e.sx1 = static_cast<s32>(bits(scissor, 16, 11));
    e.sy0 = static_cast<s32>(bits(scissor, 32, 11));
    e.sy1 = static_cast<s32>(bits(scissor, 48, 11));

    // TEST: ATE bit 0, ATST 1-3, AREF 4-11, AFAIL 12-13, DATE 14, DATM 15, ZTE 16, ZTST 17-18.
    u64 test = reg_[TEST_1 + ctx];
    e.ate = bits(test, 0, 1) != 0;
    e.atst = static_cast<u32>(bits(test, 1, 3));
    e.aref = static_cast<u32>(bits(test, 4, 8));
    e.afail = static_cast<u32>(bits(test, 12, 2));
    e.date = bits(test, 14, 1) != 0;
    e.datm = bits(test, 15, 1) != 0;
    e.zte = bits(test, 16, 1) != 0;
    e.ztst = static_cast<u32>(bits(test, 17, 2));

    // ALPHA: A-D are 2 bits each from bit 0, FIX is bits 32-39; PABE, COLCLAMP, FBA use bit 0.
    u64 alpha = reg_[ALPHA_1 + ctx];
    e.ba = static_cast<u32>(bits(alpha, 0, 2));
    e.bb = static_cast<u32>(bits(alpha, 2, 2));
    e.bc = static_cast<u32>(bits(alpha, 4, 2));
    e.bd = static_cast<u32>(bits(alpha, 6, 2));
    e.fix = static_cast<u32>(bits(alpha, 32, 8));
    e.pabe = (reg_[PABE] & 1) != 0;
    e.colclamp = (reg_[COLCLAMP] & 1) != 0;
    e.fba = (reg_[FBA_1 + ctx] & 1) != 0;

    // PRIM: IIP bit 3, TME bit 4, FGE bit 5, ABE bit 6, FST bit 8 (documented).
    e.iip = (prim >> 3) & 1;
    e.tme = (prim >> 4) & 1;
    e.fge = (prim >> 5) & 1;
    e.abe = (prim >> 6) & 1;
    e.fst = (prim >> 8) & 1;
    e.fogcol = static_cast<u32>(reg_[FOGCOL] & kRgb);

    // The texture state matters only when texturing is on.
    if (e.tme) {
        Texture& t = e.tex;

        /*
         * TEX0: TBP0 bits 0-13, TBW 14-19, PSM 20-25, TW 26-29, TH 30-33, TCC 34, TFX 35-36,
         * CSA 56-60 (documented).
         */
        u64 tex0 = reg_[TEX0_1 + ctx];
        t.tbp[0] = static_cast<u32>(bits(tex0, 0, 14));
        t.tbw[0] = static_cast<u32>(bits(tex0, 14, 6));
        t.psm = static_cast<u32>(bits(tex0, 20, 6));

        // A texture is 1,024 texels at most on a side.
        t.tw = std::min<u32>(static_cast<u32>(bits(tex0, 26, 4)), 10);
        t.th = std::min<u32>(static_cast<u32>(bits(tex0, 30, 4)), 10);
        t.tcc = bits(tex0, 34, 1) != 0;
        t.tfx = static_cast<u32>(bits(tex0, 35, 2));
        t.csa = static_cast<u32>(bits(tex0, 56, 5));

        // MIPTBP1 holds levels 1-3 and MIPTBP2 levels 4-6: TBP 14 bits and TBW 6 bits, 20 apart.
        u64 mip1 = reg_[MIPTBP1_1 + ctx], mip2 = reg_[MIPTBP2_1 + ctx];
        for (unsigned n = 0; n < 3; n++) {
            t.tbp[1 + n] = static_cast<u32>(bits(mip1, n * 20, 14));
            t.tbw[1 + n] = static_cast<u32>(bits(mip1, n * 20 + 14, 6));
            t.tbp[4 + n] = static_cast<u32>(bits(mip2, n * 20, 14));
            t.tbw[4 + n] = static_cast<u32>(bits(mip2, n * 20 + 14, 6));
        }

        // CLAMP: WMS bits 0-1, WMT 2-3, MINU 4-13, MAXU 14-23, MINV 24-33, MAXV 34-43.
        u64 clamp = reg_[CLAMP_1 + ctx];
        t.wms = static_cast<u32>(bits(clamp, 0, 2));
        t.wmt = static_cast<u32>(bits(clamp, 2, 2));
        t.minu = static_cast<u32>(bits(clamp, 4, 10));
        t.maxu = static_cast<u32>(bits(clamp, 14, 10));
        t.minv = static_cast<u32>(bits(clamp, 24, 10));
        t.maxv = static_cast<u32>(bits(clamp, 34, 10));

        // TEX1: LCM bit 0, MXL 2-4, MMAG 5, MMIN 6-8, L 19-20, K 32-43 (signed, in sixteenths).
        u64 tex1 = reg_[TEX1_1 + ctx];
        t.lcm = bits(tex1, 0, 1) != 0;

        // Six levels above the first at most.
        t.mxl = std::min<u32>(static_cast<u32>(bits(tex1, 2, 3)), 6);
        t.mmag = static_cast<u32>(bits(tex1, 5, 1));
        t.mmin = static_cast<u32>(bits(tex1, 6, 3));
        t.l = static_cast<u32>(bits(tex1, 19, 2));
        t.k = static_cast<float>(sign_extend(static_cast<u32>(bits(tex1, 32, 12)), 12)) / 16.0f;

        t.layout = &GsMemory::layout(t.psm);

        // A Z format used as a texture reads like the colour format with the same low nibble.
        switch (t.psm >= 0x30 ? (t.psm & 0xF) : t.psm) {
            case PSMCT32:
                t.kind = kTex32;
                break;

            case PSMCT24:
                t.kind = kTex24;
                break;

            case PSMCT16:
            case PSMCT16S:
                t.kind = kTex16;
                break;

            case PSMT8:
                t.kind = kTex8;
                break;

            case PSMT4:
                t.kind = kTex4;
                break;

            case PSMT8H:
                t.kind = kTex8H;
                break;

            case PSMT4HL:
                t.kind = kTex4HL;
                break;

            // PSMT4HH, and any value that is no format.
            default:
                t.kind = kTex4HH;
                break;
        }

        // TEXA: TA0 bits 0-7, AEM bit 15, TA1 bits 32-39.
        u64 texa = reg_[TEXA];
        t.ta0 = static_cast<u32>(bits(texa, 0, 8));
        t.ta1 = static_cast<u32>(bits(texa, 32, 8));
        t.aem = bits(texa, 15, 1) != 0;

        // The formats that index the colour table need a copy of it.
        if (t.kind >= kTex8) {
            // The table as it is now: it may be loaded again before this is drawn.
            if (clut_copy_dirty_ || batch_->cluts.empty()) {
                batch_->cluts.push_back(ClutCopy{clut_, 0, false});
                clut_copy_dirty_ = false;
            }

            t.clut_source = &batch_->cluts.back();
            t.clut = t.clut_source->colours.data();
        }

        // MMIN 1 and 4-5 filter linearly.
        bool min_linear = t.mmin == 1 || t.mmin >= 4;

        /*
         * The level of detail comes from each pixel's Q only when it is not
         * fixed (LCM), the coordinates carry a Q, and something depends on it:
         * mipmap levels, or different filters for enlarging and reducing.
         */
        t.lod_per_pixel =
            !e.fst && !t.lcm && ((t.mxl > 0 && t.mmin >= 2) || (t.mmag != 0) != min_linear);
    }

    e.flayout = &GsMemory::layout(e.fpsm);
    e.zlayout = &GsMemory::layout(e.zpsm);

    // The buffers it draws to as one number: FBP at bit 0, FBW 14, FPSM 20, ZBP 26, ZPSM 40.
    e.target = e.fbp | (static_cast<u64>(e.fbw) << 14) | (static_cast<u64>(e.fpsm) << 20)
               | (static_cast<u64>(e.zbp) << 26) | (static_cast<u64>(e.zpsm) << 40);
    e.f16 = is16(e.fpsm);
    e.f24 = is24(e.fpsm);
    e.z16 = is16(e.zpsm);
    e.z24 = is24(e.zpsm);

    // PRIM bit 7, AA1: edge antialiasing is not modelled; reported once.
    if ((prim >> 7) & 1) {
        note(gstodo::ANTIALIAS, "edge antialiasing (PRIM AA1)");
    }

    return e;
}

// --- Primitive assembly ---

void Gs::vertex(u16 x, u16 y, u32 z, bool draw) {
    Vertex& v = queue_[count_++];
    v.x = x;
    v.y = y;
    v.z = z;

    /*
     * The vertex takes the attributes the registers hold now: RGBA in the low half of RGBAQ and Q
     * in the high half, S and T as floats in ST (documented).
     */
    u64 rgbaq = reg_[RGBAQ];
    v.rgba = static_cast<u32>(rgbaq);
    v.q = as_float(static_cast<u32>(rgbaq >> 32));
    u64 st = reg_[ST];
    v.s = as_float(static_cast<u32>(st));
    v.t = as_float(static_cast<u32>(st >> 32));

    // UV: U is bits 0-13 and V bits 16-29, in 10.4 fixed point (documented).
    u64 uv = reg_[UV];
    v.u = static_cast<u16>(bits(uv, 0, 14));
    v.v = static_cast<u16>(bits(uv, 16, 14));
    v.fog = fog_;

    // A vertex written through XYZ3 or XYZF3 moves the queue along without
    // drawing, which is how strips skip a triangle.
    switch (reg_[PRIM] & 7) {
        // One vertex is a primitive.
        case kPoint:
            if (draw) {
                submit(kPoint, 1);
            }
            count_ = 0;
            break;

        // Two vertices are a line; the queue starts empty again.
        case kLine:
            if (count_ == 2) {
                if (draw) {
                    submit(kLine, 2);
                }
                count_ = 0;
            }
            break;

        // Every vertex after the first completes a line from the one before it.
        case kLineStrip:
            if (count_ == 2) {
                if (draw) {
                    submit(kLine, 2);
                }
                queue_[0] = queue_[1];
                count_ = 1;
            }
            break;

        // Three vertices are a triangle; the queue starts empty again.
        case kTriangle:
            if (count_ == 3) {
                if (draw) {
                    submit(kTriangle, 3);
                }
                count_ = 0;
            }
            break;

        // Every vertex after the second completes a triangle with the two before it.
        case kTriangleStrip:
            if (count_ == 3) {
                if (draw) {
                    submit(kTriangle, 3);
                }
                queue_[0] = queue_[1];
                queue_[1] = queue_[2];
                count_ = 2;
            }
            break;

        // Every vertex after the second completes a triangle with the first and the one before.
        case kTriangleFan:
            if (count_ == 3) {
                if (draw) {
                    submit(kTriangle, 3);
                }
                queue_[1] = queue_[2];
                count_ = 2;
            }
            break;

        // Two vertices are the corners of a sprite.
        case kSprite:
            if (count_ == 2) {
                if (draw) {
                    submit(kSprite, 2);
                }
                count_ = 0;
            }
            break;

        // Type 7 is reserved: drop the vertex.
        default:
            count_ = 0;
            break;
    }
}

// --- Gathering and drawing ---

void Gs::ensure_env() {
    // The current state is still the one the registers describe.
    if (!env_dirty_ && env_) {
        return;
    }

    if (clut_load_.waiting) {
        // The colour table is needed now if this state's texture uses one.
        u32 prim = prim_bits();

        // PRIM bit 4 is TME; indexed formats have a PSM whose low three bits are 3 or more.
        if (((prim >> 4) & 1) && (bits(reg_[TEX0_1 + ((prim >> 9) & 1)], 20, 6) & 7) >= 3) {
            do_clut_load();  // (may hand the batch over)
        }
    }

    Batch& batch = *batch_;
    if (batch.primitives.empty()) {
        // Nothing gathered refers to the older states: let them go.
        batch.envs.clear();
        batch.cluts.clear();
        batch.textures.clear();
        clut_copy_dirty_ = true;
    }

    batch.envs.push_back(environment());
    env_ = &batch.envs.back();
    env_dirty_ = false;
}

void Gs::submit(unsigned kind, unsigned count) {
    ensure_env();
    stats.primitives++;

    // Only a tool that asked to see primitives pays for describing them.
    if (on_primitive) {
        report(*env_, count);
    }

    // The pixels it can reach, generously, inside the scissor rectangle.
    s32 x0 = INT_MAX, y0 = INT_MAX, x1 = INT_MIN, y1 = INT_MIN;
    Pages written, depth;
    u32 need = 0;
    {
        const Env& e = *env_;

        // The bounding box of the vertices, less the drawing offset, still in 12.4.
        for (unsigned n = 0; n < count; n++) {
            s32 x = static_cast<s32>(queue_[n].x) - e.ofx,
                y = static_cast<s32>(queue_[n].y) - e.ofy;
            x0 = std::min(x0, x);
            y0 = std::min(y0, y);
            x1 = std::max(x1, x);
            y1 = std::max(y1, y);
        }

        // To whole pixels, rounding outwards, and cut to the scissor rectangle.
        x0 = std::max(x0 >> 4, e.sx0);
        y0 = std::max(y0 >> 4, e.sy0);
        x1 = std::min((x1 + 15) >> 4, e.sx1);
        y1 = std::min((y1 + 15) >> 4, e.sy1);

        // The scissor leaves nothing to draw.
        if (x0 > x1 || y0 > y1) {
            return;
        }

        add_pages(written, e.fpsm, e.fbp, e.fbw, x0, y0, x1, y1);

        // The depth buffer is touched only when depth values are tested and written.
        if (e.zte && !e.zmsk) {
            add_pages(depth, e.zpsm, e.zbp, e.fbw, x0, y0, x1, y1);
        }

        /*
         * The levels of the texture it can read. A game leaves the levels it
         * does not need unloaded (their addresses point anywhere, often at the
         * frame), so only these count.
         */
        if (e.tme) {
            need = levels_needed(e, count);
        }
    }

    // A colour table load put off must see memory as it was before this.
    if (clut_load_.waiting
        && (written.intersects(clut_load_.source) || depth.intersects(clut_load_.source))) {
        do_clut_load();
    }

    /*
     * From here on the state may be replaced by a copy in the next batch:
     * whenever what was gathered is handed over to be drawn.
     */
    prepare_levels(need);

    /*
     * A primitive whose texture is in the memory it draws to (the games blur
     * and distort the frame that way) is drawn alone, top to bottom. So is one
     * whose depth values go where its colours go (a game clears a buffer
     * through the depth side that way): the two are laid out differently in
     * memory, so one row's depth is another row's colour.
     */
    Pages reach, reads = env_->decoded_pages;
    bool in_place = need && (need & env_->in_place);

    /*
     * (Of a level read in place, a frame as a rule, only the part the
     * primitive's coordinates reach counts, where that can be told.)
     */
    if (in_place) {
        if (!in_place_reach(*env_, count, need, reach)) {
            reach = env_->in_place_pages;
        }
        reads.add(reach);
    }

    bool feeds_itself = (need && (reads.intersects(written) || reads.intersects(depth)))
                        || depth.intersects(written);

    /*
     * And colours and depth values of different primitives must not meet in
     * one batch, whose bands are drawn side by side.
     */
    if (depth.intersects(pending_colour_) || written.intersects(pending_depth_)) {
        flush();
        prepare_levels(need);
    }

    // From here `written` covers both sides; `colour` keeps the colour side alone.
    Pages colour = written;
    written.add(depth);

    // Drawn at once here: nothing is gathered, or it feeds itself and no thread orders batches.
    if (threads_ == 0 || (feeds_itself && !raster_)) {
        flush();
        wait_for_drawing();
        prepare_levels(need);
        stamp(written);
        Queued q{env_, static_cast<u8>(kind), {queue_[0], queue_[1], queue_[2]}};

        // Rows 0 to 2047 are all the rows there are.
        draw(q, 0, 2047);
        stats.pixels = pixels_.fetch_add(tls_pixels, std::memory_order_relaxed) + tls_pixels;
        tls_pixels = 0;
        return;
    }

    /*
     * With a thread that draws batches, such primitives go to it in a batch of
     * their own kind, drawn one primitive after the other.
     */
    if (feeds_itself != batch_->serial && !batch_->primitives.empty()) {
        flush();
        prepare_levels(need);
    }

    // A primitive that feeds itself joins a serial batch.
    if (feeds_itself) {
        batch_->serial = true;
        stamp(written);
        pending_target_ = env_->target;
        pending_write_.add(written);
        pending_colour_.add(colour);
        pending_depth_.add(depth);
        if (in_place) {
            pending_read_.add(reach);  // (decoded levels were taken when they were looked at)
        }

        batch_->primitives.push_back(
            Queued{env_, static_cast<u8>(kind), {queue_[0], queue_[1], queue_[2]}}
        );

        // A serial batch is handed over at 1,024 primitives.
        if (batch_->primitives.size() >= 1024) {
            flush();
        }
        return;
    }

    /*
     * Start another batch if this primitive reads in place what the gathered
     * ones write, writes what they read in place, or draws to other buffers.
     * (Decoded levels were taken when they were looked at.)
     */
    if ((in_place && reach.intersects(pending_write_)) || written.intersects(pending_read_)
        || (!batch_->primitives.empty() && env_->target != pending_target_)) {
        flush();
        prepare_levels(need);
    }

    Batch& batch = *batch_;
    const Env& e = *env_;
    pending_target_ = e.target;
    stamp(written);
    pending_write_.add(written);
    pending_colour_.add(colour);
    pending_depth_.add(depth);
    if (in_place) {
        pending_read_.add(reach);
    }

    u32 index = static_cast<u32>(batch.primitives.size());
    batch.primitives.push_back(
        Queued{&e, static_cast<u8>(kind), {queue_[0], queue_[1], queue_[2]}}
    );

    // List the primitive in every band of 16 scan lines it reaches.
    for (s32 band = y0 >> kBandShift; band <= (y1 >> kBandShift); band++) {
        if (batch.bands[static_cast<unsigned>(band)].empty()) {
            batch.used_bands.push_back(static_cast<u16>(band));
        }
        batch.bands[static_cast<unsigned>(band)].push_back(index);
    }

    // A batch is handed over at 16,384 primitives.
    if (batch.primitives.size() >= 16384) {
        flush();
    }
}

void Gs::draw(const Queued& q, s32 clip0, s32 clip1) {
    switch (q.kind) {
        case kPoint:
            draw_point(*q.env, q.v[0], clip0, clip1);
            break;

        case kLine:
            draw_line(*q.env, q.v[0], q.v[1], clip0, clip1);
            break;

        case kTriangle:
            draw_triangle(*q.env, q.v[0], q.v[1], q.v[2], clip0, clip1);
            break;

        // The queue holds only these four types; the last is the sprite.
        default:
            draw_sprite(*q.env, q.v[0], q.v[1], clip0, clip1);
            break;
    }
}

void Gs::render_band(const Batch& batch, unsigned band) {
    // The band's first and last scan line.
    s32 first = static_cast<s32>(band << kBandShift), last = first + (1 << kBandShift) - 1;

    for (u32 index : batch.bands[band]) {
        draw(batch.primitives[index], first, last);
    }
}

void Gs::render(const Batch& batch) {
    /*
     * Draw a batch: on the calling thread alone when it is small or there is no
     * pool, else with the pool, a band at a time each.
     */
    // Something to do in its place in the order, not primitives.
    if (batch.task) {
        batch.task();
    } else if (batch.serial) {
        // Primitives that depend on each other's pixels: one after the other, whole.
        for (const Queued& q : batch.primitives) {
            draw(q, 0, 2047);
        }
        pixels_.fetch_add(tls_pixels, std::memory_order_relaxed);
        tls_pixels = 0;
    } else if (!pool_ || batch.primitives.size() < 8) {
        // No pool, or fewer than 8 primitives: sharing them out would cost more than it saves.
        for (u16 band : batch.used_bands) {
            render_band(batch, band);
        }
        pixels_.fetch_add(tls_pixels, std::memory_order_relaxed);
        tls_pixels = 0;
    } else {
        // Called by the pool's threads and by this one, one band for each call.
        pool_->run(static_cast<unsigned>(batch.used_bands.size()), [&](unsigned item) {
            render_band(batch, batch.used_bands[item]);
            pixels_.fetch_add(tls_pixels, std::memory_order_relaxed);
            tls_pixels = 0;
        });
    }
}

void Gs::flush() {
    // Nothing gathered.
    if (batch_->primitives.empty()) {
        return;
    }

    stats.flushes++;

    if (raster_) {
        // With the raster thread idle nothing is in flight, so those sets start empty.
        if (raster_->idle()) {
            inflight_write_.clear();
            inflight_read_.clear();
        }

        inflight_write_.add(pending_write_);
        inflight_read_.add(pending_read_);
        batch_ = raster_->give(std::move(batch_));
    } else {
        render(*batch_);
        batch_->clear();
    }

    pending_write_.clear();
    pending_read_.clear();
    pending_colour_.clear();
    pending_depth_.clear();

    // The states went with the batch.
    env_ = nullptr;
    env_dirty_ = clut_copy_dirty_ = true;
}

void Gs::wait_for_drawing() {
    if (raster_) {
        raster_->wait();
    }

    // Everything handed over has been drawn.
    inflight_write_.clear();
    inflight_read_.clear();
}

void Gs::finish() {
    flush();
    wait_for_drawing();
    stats.pixels = pixels_.load();
}

void Gs::before_read(const Pages& pages) {
    // What is gathered writes there: hand it over.
    if (pages.intersects(pending_write_)) {
        flush();
    }

    // What was handed over writes there: wait for it.
    if (pages.intersects(inflight_write_)) {
        wait_for_drawing();
    }
}

void Gs::before_write(const Pages& pages) {
    if (clut_load_.waiting && pages.intersects(clut_load_.source)) {
        do_clut_load();  // a colour table load put off: it reads this memory as it was
    }

    // What is gathered reads or writes there: hand it over.
    if (pages.intersects(pending_write_) || pages.intersects(pending_read_)) {
        flush();
    }

    // What was handed over reads or writes there: wait for it.
    if (pages.intersects(inflight_write_) || pages.intersects(inflight_read_)) {
        wait_for_drawing();
    }
}

void Gs::report(const Env& e, unsigned count) const {
    // Room for the longest line the formats below make.
    char text[448];
    u32 prim = prim_bits();

    // The frame, depth, scissor and test state in one line; n is how much has been written.
    int n = std::snprintf(
        text,
        sizeof(text),
        "prim %u%s%s%s%s ctx%u | frame %u/%u psm %02x mask %08x | z %u psm %02x%s | scissor "
        "%d-%d,%d-%d | test %s%u/%02x/%u%s z%s%u | ",
        prim & 7,
        e.iip ? " gouraud" : "",
        e.fge ? " fog" : "",
        e.abe ? " blend" : "",
        e.fst ? " uv" : "",
        (prim >> 9) & 1,
        e.fbp / 32,
        e.fbw,
        e.fpsm,
        e.fbmsk,
        e.zbp / 32,
        e.zpsm,
        e.zmsk ? " nowrite" : "",
        e.sx0,
        e.sx1,
        e.sy0,
        e.sy1,
        e.ate ? "a" : "-",
        e.atst,
        e.aref,
        e.afail,
        e.date ? (e.datm ? " date1" : " date0") : "",
        e.zte ? "" : "-",
        e.ztst
    );

    // The blend equation, only when blending is on.
    if (e.abe) {
        n += std::snprintf(
            text + n,
            sizeof(text) - static_cast<std::size_t>(n),
            "alpha %u%u%u%u fix %02x | ",
            e.ba,
            e.bb,
            e.bc,
            e.bd,
            e.fix
        );
    }

    if (e.tme) {
        const Texture& t = e.tex;
        n += std::snprintf(
            text + n,
            sizeof(text) - static_cast<std::size_t>(n),
            "tex %u/%u psm %02x %ux%u tfx %u%s wrap %u%u filter %u%u mxl %u l %u k %.2f%s",
            t.tbp[0],
            t.tbw[0],
            t.psm,
            1u << t.tw,
            1u << t.th,
            t.tfx,
            t.tcc ? " tcc" : "",
            t.wms,
            t.wmt,
            t.mmag,
            t.mmin,
            t.mxl,
            t.l,
            static_cast<double>(t.k),
            t.lcm ? " lcm" : ""
        );

        // One entry for each mipmap level; the 16 bytes kept free hold the longest entry.
        for (u32 level = 1; level <= t.mxl && n < static_cast<int>(sizeof(text)) - 16; level++) {
            n += std::snprintf(
                text + n,
                sizeof(text) - static_cast<std::size_t>(n),
                "%s%u/%u",
                level == 1 ? " mips " : ",",
                t.tbp[level],
                t.tbw[level]
            );
        }
    } else {
        std::snprintf(text + n, sizeof(text) - static_cast<std::size_t>(n), "no texture");
    }

    // The bounding box in pixels, starting from values every vertex beats (2^30).
    int x0 = 1 << 30, y0 = 1 << 30, x1 = -(1 << 30), y1 = -(1 << 30);
    for (unsigned v = 0; v < count; v++) {
        int x = (static_cast<s32>(queue_[v].x) - e.ofx) >> 4,
            y = (static_cast<s32>(queue_[v].y) - e.ofy) >> 4;
        x0 = std::min(x0, x);
        y0 = std::min(y0, y);
        x1 = std::max(x1, x);
        y1 = std::max(y1, y);
    }

    // The level of detail at the vertices, where the texture has levels.
    float lod0 = 0, lod1 = 0;
    if (e.tme && e.tex.lod_per_pixel) {
        // Sentinels every vertex's value beats.
        lod0 = 1e9f;
        lod1 = -1e9f;
        for (unsigned v = 0; v < count; v++) {
            // LOD = -log2(|Q|) * 2^L + K (documented).
            float lod = static_cast<float>(
                            -std::log2(std::fabs(static_cast<double>(queue_[v].q)))
                            * static_cast<double>(1u << e.tex.l)
                        )
                        + e.tex.k;
            lod0 = std::min(lod0, lod);
            lod1 = std::max(lod1, lod);
        }
    }

    on_primitive(text, x0, y0, x1, y1, lod0, lod1);
}

// --- Decoded textures ---

void Gs::add_pages(Pages& pages, u32 psm, u32 bp, u32 bw, s32 x0, s32 y0, s32 x1, s32 y1) {
    // The rectangle is cut to the 2048 by 2048 pixels a buffer can have.
    x0 = std::max(x0, 0);
    y0 = std::max(y0, 0);
    x1 = std::min(x1, 2047);
    y1 = std::min(y1, 2047);

    // Nothing of it is inside.
    if (x1 < x0 || y1 < y0) {
        return;
    }

    /*
     * Pages are tiles of the buffer: 64 by 32 pixels in the 32-bit formats, 64
     * by 64 in the 16-bit, 128 by 64 in the 8-bit and 128 by 128 in the 4-bit.
     */
    unsigned bits_per_pixel = transfer_bits(psm);

    // Log2 of the page's width and height: 7 is 128, 6 is 64, 5 is 32.
    unsigned wshift = bits_per_pixel <= 8 ? 7 : 6,
             hshift = bits_per_pixel == 32 || bits_per_pixel == 24 ? 5
                      : bits_per_pixel == 4                        ? 7
                                                                   : 6;

    // These formats live in a 32-bit pixel, so their pages are the 32-bit ones.
    if (psm == PSMT8H || psm == PSMT4HL || psm == PSMT4HH) {
        wshift = 6;
        hshift = 5;
    }

    // BW counts pages of 64 pixels; a 128-pixel page takes two of them.
    u32 pages_across = wshift == 7 ? std::max(bw >> 1, 1u) : std::max(bw, 1u);

    for (u32 ty = static_cast<u32>(y0) >> hshift; ty <= static_cast<u32>(y1) >> hshift; ty++) {
        for (u32 tx = static_cast<u32>(x0) >> wshift; tx <= static_cast<u32>(x1) >> wshift; tx++) {
            // A page is 32 blocks.
            u32 block = bp + (ty * pages_across + tx) * 32;

            // A buffer need not start on a page: its tile can lie across two (512 pages in all).
            pages.set((block >> 5) & 511);
            pages.set(((block + 31) >> 5) & 511);
        }
    }
}

void Gs::stamp(const Pages& pages) {
    clock_++;

    // Visit each set bit of the 8 words, lowest first, and give its page the new time.
    for (unsigned n = 0; n < 8; n++) {
        for (u64 w = pages.bits[n]; w; w &= w - 1) {
            page_stamp_[n * 64 + static_cast<unsigned>(std::countr_zero(w))] = clock_;
        }
    }
}

u32 Gs::levels_needed(const Env& e, unsigned count) const {
    /*
     * Which levels of its texture the primitive in the queue can read. The level
     * of detail follows Q, and Q anywhere in a primitive lies between its
     * vertices' values; this mirrors `sample`, with a little room for rounding.
     */
    const Texture& t = e.tex;

    // No mipmaps in use: only the first level.
    if (t.mxl == 0 || t.mmin < 2) {
        return 1;
    }

    float low = t.k, high = t.k;

    // The level of detail varies with Q: take its range over the vertices.
    if (t.lod_per_pixel) {
        float least = std::fabs(queue_[0].q), most = least;
        for (unsigned n = 1; n < count; n++) {
            float q = std::fabs(queue_[n].q);
            least = std::min(least, q);
            most = std::max(most, q);
        }

        // LOD = -log2(Q) * 2^L + K; 1/32 of a level is the room for rounding.
        float scale = static_cast<float>(1u << t.l);
        low = -std::log2(most) * scale + t.k - 1.0f / 32;
        high = -std::log2(least) * scale + t.k + 1.0f / 32;
        if (!(low <= high)) {
            return (2u << t.mxl) - 1;  // not numbers: any level
        }
    }

    // A level of detail of 0 or less magnifies: the first level alone.
    if (high <= 0.0f) {
        return 1;
    }

    float top = static_cast<float>(t.mxl);
    u32 first, last;

    // MMIN 2 and 4 pick the nearest level, so a level is chosen by rounding.
    if (t.mmin == 2 || t.mmin == 4) {
        first = low <= 0.0f ? 0 : static_cast<u32>(std::min(low + 0.5f, top));
        last = static_cast<u32>(std::min(high + 0.5f, top));
    } else {
        // MMIN 3 and 5 blend two levels: the one below and the one above.
        first = low <= 0.0f ? 0 : static_cast<u32>(std::min(low, top));
        last = std::min(static_cast<u32>(std::min(high, top)) + 1, t.mxl);
    }

    // A bit for each level from `first` to `last`.
    return ((2u << last) - 1) & ~((1u << first) - 1);
}

void Gs::prepare_levels(u32 need) {
    /*
     * Have a current state, with the levels a primitive needs found. A level is
     * decoded now, from memory as it is, so what is still to be drawn there is
     * drawn first. A large one in a format a frame can have is cheaper read in
     * place: that is a frame buffer read as a texture, new every time.
     */

    // Ends when every level needed has been found; a flush starts it over with a new state.
    for (;;) {
        ensure_env();
        Env& e = *env_;
        u32 missing = need & ~e.looked_at;
        if (!missing) {
            break;
        }

        Texture& t = e.tex;

        // The lowest level not found yet, and its size: half as big on each side for each level.
        u32 level = static_cast<u32>(std::countr_zero(missing)), bit = 1u << level;
        u32 w = std::max(1u, (1u << t.tw) >> level), h = std::max(1u, (1u << t.th) >> level);
        Pages& pages = e.level_pages[level];
        pages.clear();
        add_pages(
            pages,
            t.psm,
            t.tbp[level],
            t.tbw[level],
            0,
            0,
            static_cast<s32>(w) - 1,
            static_cast<s32>(h) - 1
        );

        // Decode up to 512 by 512 texels of an indexed format and 256 by 256 of the others.
        if (w * h <= (t.kind >= kTex8 ? 512u * 512u : 256u * 256u)) {
            // What is gathered writes these pages: hand it over, then look again.
            if (pages.intersects(pending_write_)) {
                flush();  // and start again with the state's copy in the next batch
                continue;
            }

            // What was handed over writes these pages: wait for it.
            if (pages.intersects(inflight_write_)) {
                wait_for_drawing();
            }

            batch_->textures.push_back(cached_level(t, level));
            t.decoded[level] = batch_->textures.back()->data();
        } else {
            // Too big to decode: it is read from GS memory texel by texel.
            e.in_place |= bit;
        }

        e.looked_at |= bit;
    }

    Env& e = *env_;

    // The set of levels changed: work out which pages are read decoded and which in place.
    if (need != e.last_need) {
        e.last_need = need;
        e.decoded_pages.clear();
        e.in_place_pages.clear();
        for (u32 level = 0; level < 7; level++) {
            if (need & (1u << level)) {
                (e.in_place & (1u << level) ? e.in_place_pages : e.decoded_pages)
                    .add(e.level_pages[level]);
            }
        }
    }
}

bool Gs::in_place_reach(const Env& e, unsigned count, u32 need, Pages& pages) const {
    /*
     * The pages a primitive reads of a level it reads in place, when its
     * coordinates say so plainly: texel coordinates (UV), the first level only.
     * A texture declared larger than what was drawn into it (a 512 by 448 frame
     * read as 1,024 by 1,024) then counts for the part that is read. False: the
     * whole level counts.
     */
    const Texture& t = e.tex;

    // Only UV coordinates on the first level say plainly where they read.
    if (!e.fst || need != 1) {
        return false;
    }

    // The range of texel coordinates over the vertices; UV is in 10.4, so shift down by 4.
    s32 u0 = INT_MAX, v0 = INT_MAX, u1 = INT_MIN, v1 = INT_MIN;
    for (unsigned n = 0; n < count; n++) {
        s32 u = queue_[n].u >> 4, v = queue_[n].v >> 4;
        u0 = std::min(u0, u);
        v0 = std::min(v0, v);
        u1 = std::max(u1, u);
        v1 = std::max(v1, v);
    }

    // A texel either side for filtering; then where those fall in the level.
    struct Span {
        s32 from[2], to[2];
        int count;
    };

    // Called below for each axis; fills `out` with the one or two ranges the axis reads, or fails.
    auto spans = [](s32 a, s32 b, s32 size, u32 mode, Span& out) {
        a -= 1;
        b += 1;

        // CLAMP: the range, cut to the level.
        if (mode == 1) {
            out = {{std::clamp(a, 0, size - 1), 0}, {std::clamp(b, 0, size - 1), 0}, 1};
        } else if (mode != 0) {
            return false;  // a region inside the texture: leave it to the caller
        } else if (b - a + 1 >= size) {
            // REPEAT, and the range covers the whole level.
            out = {{0, 0}, {size - 1, 0}, 1};
        } else if (a < 0) {
            // REPEAT, and the range wraps below 0: the start of the level and the end.
            out = {{0, a + size}, {b, size - 1}, 2};
        } else if (b >= size) {
            // REPEAT, and the range wraps past the end: the end of the level and the start.
            out = {{a, 0}, {size - 1, b - size}, 2};
        } else {
            // REPEAT, and the range is inside the level.
            out = {{a, 0}, {b, 0}, 1};
        }
        return true;
    };

    Span across, down;
    if (!spans(u0, u1, 1 << t.tw, t.wms, across) || !spans(v0, v1, 1 << t.th, t.wmt, down)) {
        return false;
    }

    // Each range across with each range down is one rectangle of the level.
    for (int i = 0; i < across.count; i++) {
        for (int j = 0; j < down.count; j++) {
            add_pages(
                pages,
                t.psm,
                t.tbp[0],
                t.tbw[0],
                across.from[i],
                down.from[j],
                across.to[i],
                down.to[j]
            );
        }
    }

    return true;
}

std::shared_ptr<std::vector<u32>> Gs::cached_level(const Texture& t, u32 level) {
    // The level's size in log2 texels: half as big on each side for each level.
    u32 wl = t.tw > level ? t.tw - level : 0, hl = t.th > level ? t.th - level : 0;

    // Where the level is: TBP bits 0-13, TBW 14-19, PSM 20-25, width 26-29, height 30-33, CSA 34-.
    TextureKey key;
    key.place = t.tbp[level] | (static_cast<u64>(t.tbw[level]) << 14)
                | (static_cast<u64>(t.psm) << 20) | (static_cast<u64>(wl) << 26)
                | (static_cast<u64>(hl) << 30) | (static_cast<u64>(t.csa) << 34);

    // What turns texels into colours: nothing, the TEXA fields, or the table entries used.
    switch (t.kind) {
        case kTex32:
            break;

        // TA0 bits 0-7, TA1 bits 8-15, AEM bit 16.
        case kTex24:
        case kTex16:
            key.colours = t.ta0 | (static_cast<u64>(t.ta1) << 8) | (static_cast<u64>(t.aem) << 16);
            break;

        // The 256 entries, hashed once for each copy of the table.
        case kTex8:
        case kTex8H: {
            ClutCopy& copy = *t.clut_source;
            if (!copy.hashed) {
                // FNV-1a over the 256 entries.
                u64 h = 0xCBF29CE484222325ull;
                for (u32 i = 0; i < 256; i++) {
                    h = (h ^ copy.colours[i]) * 0x100000001B3ull;
                }
                copy.hash = h;
                copy.hashed = true;
            }
            key.colours = copy.hash;
            break;
        }

        // The 16 entries a 4-bit texture reads, from entry CSA * 16.
        default: {
            u64 h = 0xCBF29CE484222325ull;
            for (u32 i = 0; i < 16; i++) {
                h = (h ^ t.clut[((t.csa & 15) * 16 + i) & 0xFF]) * 0x100000001B3ull;
            }
            key.colours = h;
            break;
        }
    }

    auto found = texture_cache_.find(key);
    if (found != texture_cache_.end()) {
        CachedTexture& c = found->second;

        // The copy is good unless one of its pages was written after it was decoded.
        bool fresh = true;
        for (unsigned n = 0; n < 8 && fresh; n++) {
            for (u64 w = c.pages.bits[n]; w; w &= w - 1) {
                if (page_stamp_[n * 64 + static_cast<unsigned>(std::countr_zero(w))] > c.stamp) {
                    fresh = false;
                    break;
                }
            }
        }

        if (fresh) {
            return c.texels;
        }
    } else if (texture_cache_.size() > 4096) {
        // Not found, and the cache has grown past 4,096 levels: start it again.
        texture_cache_.clear();  // (the batches that use a copy hold on to it)
    }

    // Decode the level and keep it.
    CachedTexture& c = texture_cache_[key];
    u32 w = 1u << wl, h = 1u << hl;
    stats.texture_decodes++;
    stats.texels_decoded += static_cast<u64>(w) * h;
    c.texels = std::make_shared<std::vector<u32>>(static_cast<std::size_t>(w) * h);
    Texture direct = t;
    direct.wms = direct.wmt = 1;  // plain coordinates: wrapping is applied when the copy is read
    u32* out = c.texels->data();
    for (u32 y = 0; y < h; y++) {
        for (u32 x = 0; x < w; x++) {
            *out++ = texel(direct, level, static_cast<s32>(x), static_cast<s32>(y), nullptr);
        }
    }

    // The copy is valid until one of these pages is written.
    c.pages.clear();
    add_pages(
        c.pages,
        t.psm,
        t.tbp[level],
        t.tbw[level],
        0,
        0,
        static_cast<s32>(w) - 1,
        static_cast<s32>(h) - 1
    );
    c.stamp = clock_;

    return c.texels;
}

// --- Rasterisers ---

void Gs::draw_point(const Env& e, const Vertex& a, s32 clip0, s32 clip1) {
    // From 12.4 to whole pixels, less the drawing offset.
    s32 x = (static_cast<s32>(a.x) - e.ofx) >> 4;
    s32 y = (static_cast<s32>(a.y) - e.ofy) >> 4;

    // Outside the scissor rectangle, or in another band.
    if (x < e.sx0 || x > e.sx1 || y < e.sy0 || y > e.sy1 || y < clip0 || y > clip1) {
        return;
    }

    // UV is in 10.4 fixed point; ST is divided by Q and scaled to the texture's size in texels.
    float u = e.fst ? a.u / 16.0f : a.s / a.q * static_cast<float>(1u << e.tex.tw);
    float v = e.fst ? a.v / 16.0f : a.t / a.q * static_cast<float>(1u << e.tex.th);
    GsMemory::Row frow = GsMemory::row(*e.flayout, e.fbp, e.fbw, static_cast<u32>(y));
    GsMemory::Row zrow = GsMemory::row(*e.zlayout, e.zbp, e.fbw, static_cast<u32>(y));
    pixel(e, frow, zrow, x, std::min(a.z, e.zmax), shade(e, a.rgba, u, v, e.tex.k, a.fog));
}

void Gs::draw_line(const Env& e, const Vertex& a, const Vertex& b, s32 clip0, s32 clip1) {
    s32 x0 = static_cast<s32>(a.x) - e.ofx, y0 = static_cast<s32>(a.y) - e.ofy;
    s32 x1 = static_cast<s32>(b.x) - e.ofx, y1 = static_cast<s32>(b.y) - e.ofy;

    // One step for each pixel along the longer axis, from 12.4 rounded up.
    s32 steps = (std::max(std::abs(x1 - x0), std::abs(y1 - y0)) + 15) >> 4;

    // A line shorter than a pixel draws nothing.
    if (steps <= 0) {
        return;
    }

    float tw = static_cast<float>(1u << e.tex.tw), th = static_cast<float>(1u << e.tex.th);

    for (s32 i = 0; i < steps; i++) {
        float f = static_cast<float>(i) / static_cast<float>(steps);

        // The point at fraction f, rounded to the nearest pixel (8 is half a pixel in 12.4).
        s32 x = (x0 + static_cast<s32>(std::lround(static_cast<float>(x1 - x0) * f)) + 8) >> 4;
        s32 y = (y0 + static_cast<s32>(std::lround(static_cast<float>(y1 - y0) * f)) + 8) >> 4;

        // Outside the scissor rectangle or in another band: skip this pixel.
        if (x < e.sx0 || x > e.sx1 || y < e.sy0 || y > e.sy1 || y < clip0 || y > clip1) {
            continue;
        }

        // Gouraud shading blends the end colours; flat shading takes the last vertex's.
        u32 colour = e.iip ? lerp_colour(a.rgba, b.rgba, f) : b.rgba;
        double z = static_cast<double>(a.z) + (static_cast<double>(b.z) - a.z) * f;
        float u, v;

        // UV is in 10.4; ST is interpolated with Q and divided by it, perspective-correct.
        if (e.fst) {
            u = (a.u + (b.u - a.u) * f) / 16.0f;
            v = (a.v + (b.v - a.v) * f) / 16.0f;
        } else {
            float q = a.q + (b.q - a.q) * f;
            u = (a.s + (b.s - a.s) * f) / q * tw;
            v = (a.t + (b.t - a.t) * f) / q * th;
        }

        // The fog coefficient, rounded to the nearest value.
        u32 fog = static_cast<u32>(a.fog + (b.fog - a.fog) * f + 0.5f);
        GsMemory::Row frow = GsMemory::row(*e.flayout, e.fbp, e.fbw, static_cast<u32>(y));
        GsMemory::Row zrow = GsMemory::row(*e.zlayout, e.zbp, e.fbw, static_cast<u32>(y));
        pixel(
            e,
            frow,
            zrow,
            x,
            std::min(static_cast<u32>(z + 0.5), e.zmax),
            shade(e, colour, u, v, e.tex.k, fog)
        );
    }
}

void Gs::draw_sprite(const Env& e, const Vertex& a, const Vertex& b, s32 clip0, s32 clip1) {
    s32 x0 = static_cast<s32>(a.x) - e.ofx, y0 = static_cast<s32>(a.y) - e.ofy;
    s32 x1 = static_cast<s32>(b.x) - e.ofx, y1 = static_cast<s32>(b.y) - e.ofy;
    float u0, v0, u1, v1;

    // UV is in 10.4 fixed point.
    if (e.fst) {
        u0 = a.u / 16.0f;
        v0 = a.v / 16.0f;
        u1 = b.u / 16.0f;
        v1 = b.v / 16.0f;
    } else {
        // Both corners are divided by the second vertex's Q.
        float tw = static_cast<float>(1u << e.tex.tw), th = static_cast<float>(1u << e.tex.th);
        u0 = a.s / b.q * tw;
        v0 = a.t / b.q * th;
        u1 = b.s / b.q * tw;
        v1 = b.t / b.q * th;
    }

    // The corners may come in either order; the texture coordinates travel with them.
    if (x0 > x1) {
        std::swap(x0, x1);
        std::swap(u0, u1);
    }
    if (y0 > y1) {
        std::swap(y0, y1);
        std::swap(v0, v1);
    }

    // No width or no height: no pixel has its sample point inside.
    if (x0 == x1 || y0 == y1) {
        return;
    }

    // The pixels whose sample points lie inside, cut to the scissor rectangle and the band.
    s32 px0 = std::max(ceil16(x0), e.sx0), px1 = std::min(ceil16(x1) - 1, e.sx1);
    s32 py0 = std::max({ceil16(y0), e.sy0, clip0}), py1 = std::min({ceil16(y1) - 1, e.sy1, clip1});

    // Texture coordinate step for one pixel, from the corner values over 12.4 distances.
    float du = (u1 - u0) / static_cast<float>(x1 - x0),
          dv = (v1 - v0) / static_cast<float>(y1 - y0);
    u32 z = std::min(b.z, e.zmax);

    // An untextured, unfogged sprite is one colour: work it out once.
    bool flat = !e.tme && !e.fge;
    u32 flat_colour = flat ? shade(e, b.rgba, 0, 0, 0, b.fog) : 0;

    for (s32 y = py0; y <= py1; y++) {
        // V at this row's sample point; the factor 16 brings the pixel into 12.4.
        float v = v0 + static_cast<float>(y * 16 - y0) * dv;
        GsMemory::Row frow = GsMemory::row(*e.flayout, e.fbp, e.fbw, static_cast<u32>(y));
        GsMemory::Row zrow = GsMemory::row(*e.zlayout, e.zbp, e.fbw, static_cast<u32>(y));

        for (s32 x = px0; x <= px1; x++) {
            if (flat) {
                pixel(e, frow, zrow, x, z, flat_colour);
            } else {
                float u = u0 + static_cast<float>(x * 16 - x0) * du;
                pixel(e, frow, zrow, x, z, shade(e, b.rgba, u, v, e.tex.k, b.fog));
            }
        }
    }
}

void Gs::draw_triangle(
    const Env& e, const Vertex& a, const Vertex& b, const Vertex& c, s32 clip0, s32 clip1
) {
    // The vertices in 12.4, less the drawing offset.
    const Vertex* v[3] = {&a, &b, &c};
    s64 x[3], y[3];
    for (int i = 0; i < 3; i++) {
        x[i] = static_cast<s32>(v[i]->x) - e.ofx;
        y[i] = static_cast<s32>(v[i]->y) - e.ofy;
    }

    // Twice the signed area; no area, nothing to draw.
    s64 area = (x[1] - x[0]) * (y[2] - y[0]) - (y[1] - y[0]) * (x[2] - x[0]);
    if (area == 0) {
        return;
    }

    // Wind the triangle one way, so that inside always means the three edge functions are positive.
    if (area < 0) {
        std::swap(v[1], v[2]);
        std::swap(x[1], x[2]);
        std::swap(y[1], y[2]);
        area = -area;
    }

    s32 px0 = std::max(ceil16(std::min({x[0], x[1], x[2]})), e.sx0);
    s32 px1 = std::min(static_cast<s32>(std::max({x[0], x[1], x[2]}) >> 4), e.sx1);

    /*
     * Everything below is worked out from the triangle's own first row, so
     * that a band of it comes out the same as the whole.
     */
    s32 py0 = std::max(ceil16(std::min({y[0], y[1], y[2]})), e.sy0);
    s32 py1 = std::min({static_cast<s32>(std::max({y[0], y[1], y[2]}) >> 4), e.sy1, clip1});
    s32 first_row = std::max(py0, clip0);

    // No pixel column or no row of this band is left to draw.
    if (px0 > px1 || first_row > py1) {
        return;
    }

    /*
     * Edge i runs from vertex i+1 to vertex i+2 and its function is the
     * weight of vertex i. With the area positive the inside is where all
     * three are positive; a sample exactly on an edge belongs to the triangle
     * only if the edge is a top or a left one.
     */
    s64 ex[3], ey[3], bias[3], row[3];
    for (int i = 0; i < 3; i++) {
        int p = (i + 1) % 3, n = (i + 2) % 3;
        ex[i] = x[n] - x[p];
        ey[i] = y[n] - y[p];

        // A top edge is level and runs right; a left edge runs up the screen.
        bool top_left = ey[i] < 0 || (ey[i] == 0 && ex[i] > 0);
        bias[i] = top_left ? 0 : 1;

        // The edge function at the first pixel's sample point; a pixel is 16 units in 12.4.
        row[i] = ex[i] * (static_cast<s64>(py0) * 16 - y[p])
                 - ey[i] * (static_cast<s64>(px0) * 16 - x[p]);
    }

    /*
     * Everything interpolated is linear across the screen: its value at the
     * first pixel, and what it changes by a pixel across and a pixel down.
     */
    struct Plane {
        double at, dx, dy;
    };

    double inv_area = 1.0 / static_cast<double>(area);

    // The plane through the values a0, a1, a2 at the vertices; the edge functions are the weights.
    auto plane = [&](double a0, double a1, double a2) -> Plane {
        return {
            (static_cast<double>(row[0]) * a0 + static_cast<double>(row[1]) * a1
             + static_cast<double>(row[2]) * a2)
                * inv_area,
            -(static_cast<double>(ey[0]) * a0 + static_cast<double>(ey[1]) * a1
              + static_cast<double>(ey[2]) * a2)
                * 16.0 * inv_area,
            (static_cast<double>(ex[0]) * a0 + static_cast<double>(ex[1]) * a1
             + static_cast<double>(ex[2]) * a2)
                * 16.0 * inv_area
        };
    };

    // Depth is exact when it is the same at all three vertices.
    bool flat_z = v[0]->z == v[1]->z && v[1]->z == v[2]->z;
    Plane pz = plane(v[0]->z, v[1]->z, v[2]->z);
    Plane pc[4]{}, ps{}, pt{}, pq{}, pf{};

    // Gouraud shading interpolates each colour channel.
    if (e.iip) {
        for (unsigned n = 0; n < 4; n++) {
            pc[n] = plane(ch(v[0]->rgba, n), ch(v[1]->rgba, n), ch(v[2]->rgba, n));
        }
    }

    if (e.tme) {
        // UV is in 10.4 fixed point.
        if (e.fst) {
            ps = plane(v[0]->u / 16.0, v[1]->u / 16.0, v[2]->u / 16.0);
            pt = plane(v[0]->v / 16.0, v[1]->v / 16.0, v[2]->v / 16.0);
        } else {
            // S, T and Q are linear on screen; their ratios are what is perspective-correct.
            ps = plane(v[0]->s, v[1]->s, v[2]->s);
            pt = plane(v[0]->t, v[1]->t, v[2]->t);
            pq = plane(v[0]->q, v[1]->q, v[2]->q);
        }
    }

    if (e.fge) {
        pf = plane(v[0]->fog, v[1]->fog, v[2]->fog);
    }

    const double tw = static_cast<double>(1u << e.tex.tw), th = static_cast<double>(1u << e.tex.th);
    const double lod_scale = static_cast<double>(1u << e.tex.l);

    // A flat, untextured, unfogged triangle is one colour.
    bool constant = !e.iip && !e.tme && !e.fge;
    u32 constant_colour = constant ? shade(e, c.rgba, 0, 0, 0, 0) : 0;

    // Move the edge functions down to the first row of this band; one row is 16 units.
    for (int i = 0; i < 3; i++) {
        row[i] += ex[i] * 16 * (first_row - py0);
    }

    for (s32 py = first_row; py <= py1; py++) {
        double dy = static_cast<double>(py - py0);
        s64 w0 = row[0], w1 = row[1], w2 = row[2];
        GsMemory::Row frow = GsMemory::row(*e.flayout, e.fbp, e.fbw, static_cast<u32>(py));
        GsMemory::Row zrow = GsMemory::row(*e.zlayout, e.zbp, e.fbw, static_cast<u32>(py));

        for (s32 px = px0; px <= px1; px++) {
            // The sample point is inside, or on a top or left edge.
            if (w0 >= bias[0] && w1 >= bias[1] && w2 >= bias[2]) {
                double dx = static_cast<double>(px - px0);

                // Flat depth is used as it is; otherwise the plane, rounded and held in 32 bits.
                u32 zv =
                    flat_z
                        ? v[0]->z
                        : static_cast<u32>(
                              std::clamp(pz.at + pz.dx * dx + pz.dy * dy + 0.5, 0.0, 4294967295.0)
                          );

                // A 16 or 24-bit depth buffer holds less than the plane can reach.
                if (zv > e.zmax) {
                    zv = e.zmax;
                }

                u32 colour;
                if (constant) {
                    colour = constant_colour;
                } else {
                    u32 vertex_colour = c.rgba;

                    // Gouraud: each channel from its plane, rounded and held to 0-255.
                    if (e.iip) {
                        vertex_colour = 0;
                        for (unsigned n = 0; n < 4; n++) {
                            double value = pc[n].at + pc[n].dx * dx + pc[n].dy * dy;
                            vertex_colour |= static_cast<u32>(std::clamp(value + 0.5, 0.0, 255.0))
                                             << (n * 8);
                        }
                    }

                    float tu = 0, tv = 0, lod = e.tex.k;
                    if (e.tme) {
                        double s = ps.at + ps.dx * dx + ps.dy * dy,
                               t = pt.at + pt.dx * dx + pt.dy * dy;
                        if (e.fst) {
                            tu = static_cast<float>(s);
                            tv = static_cast<float>(t);
                        } else {
                            // Divide by Q for perspective, and scale to texels.
                            double q = pq.at + pq.dx * dx + pq.dy * dy;
                            double inv_q = 1.0 / q;
                            tu = static_cast<float>(s * inv_q * tw);
                            tv = static_cast<float>(t * inv_q * th);

                            // LOD = -log2(|Q|) * 2^L + K, when it varies with Q.
                            if (e.tex.lod_per_pixel) {
                                lod = static_cast<float>(-std::log2(std::fabs(q)) * lod_scale)
                                      + e.tex.k;
                            }
                        }
                    }

                    // Fog coefficient, rounded and held to 0-255.
                    u32 fog =
                        e.fge ? static_cast<u32>(
                                    std::clamp(pf.at + pf.dx * dx + pf.dy * dy + 0.5, 0.0, 255.0)
                                )
                              : 0;
                    colour = shade(e, vertex_colour, tu, tv, lod, fog);
                }

                pixel(e, frow, zrow, px, zv, colour);
            }

            // One pixel right: each edge function drops by its step (16 units in 12.4).
            w0 -= ey[0] * 16;
            w1 -= ey[1] * 16;
            w2 -= ey[2] * 16;
        }

        // One row down.
        for (int i = 0; i < 3; i++) {
            row[i] += ex[i] * 16;
        }
    }
}

// --- Texturing ---

u32 Gs::expand16(u16 c, u32 ta0, u32 ta1, bool aem) {
    // Alpha bit 15 picks TA1 or TA0; with AEM a black colour is transparent whatever its bit.
    u32 a = (c & 0x8000) ? ta1 : (aem && c == 0) ? 0 : ta0;

    // R is bits 0-4, G bits 5-9, B bits 10-14, each widened to 8 bits by a shift of 3.
    return pack((c & 0x1F) << 3, ((c >> 5) & 0x1F) << 3, ((c >> 10) & 0x1F) << 3, a);
}

u32 Gs::texel(const Texture& t, u32 level, s32 iu, s32 iv, const u32* decoded) const {
    // The level's size in texels: half as big on each side for each level.
    s32 w = std::max(1, (1 << t.tw) >> level), h = std::max(1, (1 << t.th) >> level);

    // Called for each axis below; applies the wrap mode of CLAMP (WMS or WMT) with its limits.
    auto wrap = [level](s32 c, s32 size, u32 mode, u32 lo, u32 hi) -> s32 {
        switch (mode) {
            // REPEAT: the coordinate wraps within the (power of two) size.
            case 0:
                return c & (size - 1);

            // CLAMP: the coordinate stays inside the level.
            case 1:
                return std::clamp(c, 0, size - 1);

            // REGION_CLAMP: the coordinate stays between MIN and MAX, which are given for level 0.
            case 2:
                return std::clamp(c, static_cast<s32>(lo >> level), static_cast<s32>(hi >> level));

            // REGION_REPEAT: the bits MIN selects wrap, the bits of MAX are forced on.
            default:
                return (c & static_cast<s32>(lo)) | static_cast<s32>(hi);
        }
    };
    iu = wrap(iu, w, t.wms, t.minu, t.maxu);
    iv = wrap(iv, h, t.wmt, t.minv, t.maxv);

    // A decoded copy covers the texel; the unsigned compare also rejects negative coordinates.
    if (decoded && static_cast<u32>(iu) < static_cast<u32>(w)
        && static_cast<u32>(iv) < static_cast<u32>(h)) {
        return decoded[static_cast<u32>(iv) * static_cast<u32>(w) + static_cast<u32>(iu)];
    }

    u32 at = GsMemory::index(
        *t.layout, t.tbp[level], t.tbw[level], static_cast<u32>(iu), static_cast<u32>(iv)
    );

    // Read the texel from GS memory and make a colour of it, by format.
    switch (t.kind) {
        // The word is the colour.
        case kTex32:
            return memory.word(at);

        // The word's low 24 bits are the colour; alpha is TA0, or 0 for black with AEM.
        case kTex24: {
            u32 raw = memory.word(at) & kRgb;
            return raw | ((t.aem && raw == 0) ? 0u : t.ta0 << 24);
        }

        case kTex16:
            return expand16(memory.half(at), t.ta0, t.ta1, t.aem);

        // An index into the table, from entry CSA * 16; the table has 256 entries.
        case kTex8:
            return t.clut[(t.csa * 16 + memory.byte(at)) & 0xFF];

        case kTex4:
            return t.clut[(t.csa * 16 + memory.nibble(at)) & 0xFF];

        // The index is bits 24-31 of the word.
        case kTex8H:
            return t.clut[(t.csa * 16 + (memory.word(at) >> 24)) & 0xFF];

        // The index is bits 24-27 of the word.
        case kTex4HL:
            return t.clut[(t.csa * 16 + ((memory.word(at) >> 24) & 0xF)) & 0xFF];

        // The index is bits 28-31 of the word.
        default:
            return t.clut[(t.csa * 16 + (memory.word(at) >> 28)) & 0xFF];
    }
}

u32 Gs::sample_level(const Texture& t, u32 level, float u, float v, bool linear) const {
    const u32* decoded = t.decoded[level];

    // Coordinates are given for the first level; a level is half as big for each step.
    if (level) {
        float scale = 1.0f / static_cast<float>(1u << level);
        u *= scale;
        v *= scale;
    }

    // Nearest: the texel the point is in.
    if (!linear) {
        return texel(
            t, level, static_cast<s32>(std::floor(u)), static_cast<s32>(std::floor(v)), decoded
        );
    }

    /*
     * Weights in sixteenths, as the hardware has them. The point is measured
     * from the texel's centre, half a texel (8 sixteenths) in.
     */
    s32 fu = static_cast<s32>(std::floor(u * 16.0f)) - 8,
        fv = static_cast<s32>(std::floor(v * 16.0f)) - 8;
    s32 iu = fu >> 4, iv = fv >> 4;
    u32 a = static_cast<u32>(fu & 15), b = static_cast<u32>(fv & 15);
    u32 c00 = texel(t, level, iu, iv, decoded), c10 = texel(t, level, iu + 1, iv, decoded);
    u32 c01 = texel(t, level, iu, iv + 1, decoded), c11 = texel(t, level, iu + 1, iv + 1, decoded);

    // Blend the four texels channel by channel; the two weights multiply to 16 * 16, hence >> 8.
    u32 out = 0;
    for (unsigned n = 0; n < 4; n++) {
        u32 top = ch(c00, n) * (16 - a) + ch(c10, n) * a,
            bottom = ch(c01, n) * (16 - a) + ch(c11, n) * a;
        out |= ((top * (16 - b) + bottom * b) >> 8) << (n * 8);
    }
    return out;
}

u32 Gs::sample(const Texture& t, float u, float v, float lod) const {
    // A level of detail of 0 or less magnifies: MMAG picks nearest (0) or linear (1).
    if (lod <= 0.0f) {
        return sample_level(t, 0, u, v, t.mmag != 0);
    }

    // MMIN 1 and 4-5 filter linearly; 0 and 2-3 take the nearest texel.
    bool linear = t.mmin == 1 || t.mmin >= 4;

    // No mipmaps in use: the first level only.
    if (t.mxl == 0 || t.mmin < 2) {
        return sample_level(t, 0, u, v, linear);
    }

    float top = static_cast<float>(t.mxl);

    // MMIN 2 and 4 use the nearest level, chosen by rounding.
    if (t.mmin == 2 || t.mmin == 4) {
        return sample_level(t, static_cast<u32>(std::min(lod + 0.5f, top)), u, v, linear);
    }

    // MMIN 3 and 5 blend the level below with the one above.
    float l = std::min(lod, top);
    u32 l0 = static_cast<u32>(l), l1 = std::min(l0 + 1, t.mxl);
    return lerp_colour(
        sample_level(t, l0, u, v, linear),
        sample_level(t, l1, u, v, linear),
        l - static_cast<float>(l0)
    );
}

u32 Gs::shade(const Env& e, u32 rgba, float u, float v, float lod, u32 fog) const {
    u32 r = ch(rgba, 0), g = ch(rgba, 1), b = ch(rgba, 2), a = ch(rgba, 3);

    if (e.tme) {
        u32 t = sample(e.tex, u, v, lod);
        u32 tr = ch(t, 0), tg = ch(t, 1), tb = ch(t, 2), ta = ch(t, 3);

        // Texture times colour, where 0x80 stands for 1.0; held to 255.
        auto mod = [](u32 x, u32 y) {
            return std::min<u32>((x * y) >> 7, 255);
        };

        switch (e.tex.tfx) {
            case 0:  // modulate
                r = mod(tr, r);
                g = mod(tg, g);
                b = mod(tb, b);
                if (e.tex.tcc) {
                    a = mod(ta, a);
                }
                break;

            case 1:  // decal
                r = tr;
                g = tg;
                b = tb;
                if (e.tex.tcc) {
                    a = ta;
                }
                break;

            default:  // highlight, highlight 2
                r = std::min<u32>(mod(tr, r) + a, 255);
                g = std::min<u32>(mod(tg, g) + a, 255);
                b = std::min<u32>(mod(tb, b) + a, 255);

                // Highlight 2 adds the vertex alpha to the texture's; highlight keeps it.
                if (e.tex.tcc) {
                    a = e.tex.tfx == 2 ? std::min<u32>(ta + a, 255) : ta;
                }
                break;
        }
    }

    if (e.fge) {
        // Fog colour plus the difference to it scaled by the coefficient, which is out of 256.
        auto mix = [fog](u32 c, u32 f) {
            return static_cast<u32>(
                static_cast<s32>(f)
                + (((static_cast<s32>(c) - static_cast<s32>(f)) * static_cast<s32>(fog)) >> 8)
            );
        };
        r = mix(r, ch(e.fogcol, 0));
        g = mix(g, ch(e.fogcol, 1));
        b = mix(b, ch(e.fogcol, 2));
    }

    return pack(r, g, b, a);
}

// --- Colour lookup table ---

void Gs::load_clut(u64 tex0) {
    /*
     * The GS keeps its own copy of the table and loads it when TEX0 or TEX2 is
     * written, as the CLD field says. Later changes to the memory the table came
     * from do not reach a primitive until the table is loaded again.
     */

    /*
     * A write to TEX0 or TEX2 can ask for the colour table to be loaded from
     * memory. The load is put off until the table is used (or the memory it
     * comes from is about to change), because a game also sets textures it then
     * draws nothing with, with addresses that point anywhere, and reading memory
     * means waiting for whatever is still to be drawn there.
     */

    // TEX0: PSM bits 20-25, CBP 37-50, CPSM 51-54, CSM bit 55, CSA 56-60, CLD 61-63 (documented).
    u32 psm = static_cast<u32>(bits(tex0, 20, 6));
    u32 cbp = static_cast<u32>(bits(tex0, 37, 14));
    u32 cpsm = static_cast<u32>(bits(tex0, 51, 4));
    bool csm2 = bits(tex0, 55, 1) != 0;
    u32 csa = static_cast<u32>(bits(tex0, 56, 5));
    u32 cld = static_cast<u32>(bits(tex0, 61, 3));

    /*
     * Only a texture format that uses the table loads it (and only then is
     * the address remembered for the "if it changed" kinds of load).
     */
    if ((psm & 7) < 3) {
        return;
    }

    switch (cld) {
        // CLD 1: load.
        case 1:
            break;

        // CLD 2 and 3: load, and remember CBP in CBP0 or CBP1.
        case 2:
        case 3:
            clut_cbp_[cld - 2] = cbp;
            break;

        // CLD 4 and 5: load only if CBP differs from CBP0 or CBP1, and remember it.
        case 4:
        case 5:
            if (clut_cbp_[cld - 4] == cbp) {
                return;
            }
            clut_cbp_[cld - 4] = cbp;
            break;

        // CLD 0 asks for no load; 6 and 7 are not defined, and load nothing here.
        default:
            return;
    }

    ClutLoad load;
    load.waiting = true;
    load.tex0 = tex0;
    load.texclut = reg_[TEXCLUT];

    // A table has 256 entries for an 8-bit format and 16 for a 4-bit one; CSA picks the first.
    load.count = (psm == PSMT8 || psm == PSMT8H) ? 256 : 16;
    load.first = (csa * 16) & 0xFF;

    if (csm2) {
        // CSM2: 16-bit entries from a buffer of width TEXCLUT CBW (bits 0-5), 1,024 pixels square.
        add_pages(
            load.source, PSMCT16, cbp, static_cast<u32>(bits(load.texclut, 0, 6)), 0, 0, 1023, 1023
        );
    } else {
        // CSM1: a block of 16 by 16 pixels at CBP.
        add_pages(load.source, cpsm, cbp, 1, 0, 0, 15, 15);
    }

    // One still waiting is done first, unless this one replaces all it would load.
    if (clut_load_.waiting
        && !(
            load.first <= clut_load_.first
            && load.first + load.count >= clut_load_.first + clut_load_.count
        )) {
        do_clut_load();
    }

    clut_load_ = load;
}

void Gs::do_clut_load() {
    // Nothing was put off.
    if (!clut_load_.waiting) {
        return;
    }

    clut_load_.waiting = false;
    const ClutLoad load = clut_load_;

    // The same TEX0 fields as in `load_clut`.
    u64 tex0 = load.tex0;
    u32 cbp = static_cast<u32>(bits(tex0, 37, 14));
    u32 cpsm = static_cast<u32>(bits(tex0, 51, 4));
    bool csm2 = bits(tex0, 55, 1) != 0;
    u32 csa = static_cast<u32>(bits(tex0, 56, 5));

    // The table is read from memory: what is still to be drawn there comes first.
    before_read(load.source);

    for (u32 i = 0; i < load.count; i++) {
        u32 raw;
        if (csm2) {
            // TEXCLUT: CBW bits 0-5, COU bits 6-11 (in units of 16 pixels), COV bits 12-21.
            u32 x = static_cast<u32>(bits(load.texclut, 6, 6)) * 16 + i;
            raw = memory.read(
                PSMCT16,
                cbp,
                static_cast<u32>(bits(load.texclut, 0, 6)),
                x,
                static_cast<u32>(bits(load.texclut, 12, 10))
            );
        } else if (load.count == 256) {
            // Entries 8-15 and 16-23 of every 32 are stored the other way round.
            u32 p = (i & 0xE7) | ((i & 0x08) << 1) | ((i & 0x10) >> 1);

            // The 256 entries lie in a block of 16 by 16 pixels, 16 to a row.
            raw = memory.read(cpsm, cbp, 1, p & 15, p >> 4);
        } else {
            // The 16 entries lie in 8 by 2 pixels, 8 to a row.
            raw = memory.read(cpsm, cbp, 1, i & 7, i >> 3);
        }

        // The table has 256 entries and CSA picks the first; it wraps.
        clut_raw_[(csa * 16 + i) & 0xFF] = raw;
    }

    clut_psm_ = csm2 ? static_cast<u32>(PSMCT16) : cpsm;
    rebuild_clut();
}

void Gs::rebuild_clut() {
    // TEXA: TA0 bits 0-7, AEM bit 15, TA1 bits 32-39.
    u64 texa = reg_[TEXA];
    u32 ta0 = static_cast<u32>(bits(texa, 0, 8)), ta1 = static_cast<u32>(bits(texa, 32, 8));
    bool aem = bits(texa, 15, 1) != 0;

    for (u32 i = 0; i < 256; i++) {
        // 32-bit entries are colours already; 16-bit ones get their alpha from TEXA.
        u32 colour = clut_psm_ == PSMCT32 ? clut_raw_[i]
                                          : expand16(static_cast<u16>(clut_raw_[i]), ta0, ta1, aem);

        // Only a change makes the table's copies and the state stale.
        if (colour != clut_[i]) {
            clut_[i] = colour;
            clut_copy_dirty_ = true;
            env_dirty_ = true;
        }
    }
}

// --- Pixel pipeline ---

u32 Gs::frame_read(const Env& e, const GsMemory::Row& row, s32 x) const {
    u32 at = GsMemory::index(row, static_cast<u32>(x));

    // 16-bit: 5:5:5:1 widened to 8 bits a channel; the alpha bit becomes 0 or 0x80.
    if (e.f16) {
        u32 v = memory.half(at);
        return pack(
            (v & 0x1F) << 3,
            ((v >> 5) & 0x1F) << 3,
            ((v >> 10) & 0x1F) << 3,
            (v & 0x8000) ? 0x80 : 0
        );
    }

    // 24-bit has no alpha: it reads as 0x80 (opaque in the GS's scale) in the top byte.
    u32 v = memory.word(at);
    return e.f24 ? (v | 0x80000000u) : v;
}

void Gs::frame_write(const Env& e, const GsMemory::Row& row, s32 x, u32 rgba, u32 mask) {
    u32 at = GsMemory::index(row, static_cast<u32>(x));

    if (e.f16) {
        // 8-bit channels to 5:5:5:1: the top five bits of each, and alpha bit 7 as bit 15.
        auto to16 = [](u32 c) {
            return ((c >> 3) & 0x1F) | (((c >> 11) & 0x1F) << 5) | (((c >> 19) & 0x1F) << 10)
                   | ((c >> 16) & 0x8000);
        };
        u32 m = to16(mask), v = to16(rgba);

        // A write of every bit needs no read of the old value.
        u32 old = m == 0xFFFF ? 0 : memory.half(at);
        memory.set_half(at, static_cast<u16>((old & ~m) | (v & m)));
        return;
    }

    // A 24-bit buffer has no alpha byte to write.
    if (e.f24) {
        mask &= kRgb;
    }

    // A write of every bit needs no read of the old value.
    if (mask == 0xFFFFFFFFu) {
        memory.set_word(at, rgba);
    } else {
        memory.set_word(at, (memory.word(at) & ~mask) | (rgba & mask));
    }
}

void Gs::pixel(
    const Env& e, const GsMemory::Row& frow, const GsMemory::Row& zrow, s32 x, u32 z, u32 rgba
) {
    u32 sa = rgba >> 24;

    /*
     * With the depth test switched off the Z buffer is not touched at all: the
     * games draw to targets that share memory with it that way.
     */
    bool write_rgb = true, write_a = true, write_z = !e.zmsk && e.zte;

    // Alpha test: a pixel that fails it is handled as AFAIL says.
    if (e.ate && !alpha_passes(e.atst, sa, e.aref)) {
        switch (e.afail) {
            case 0:  // keep
                return;

            case 1:  // frame buffer only
                write_z = false;
                break;

            case 2:  // Z buffer only
                write_rgb = write_a = false;
                break;

            default:  // colour only; without an alpha channel it is "frame buffer only"
                write_z = false;
                write_a = !((e.fpsm & 0xF) == 0);
                break;
        }
    }

    // Blending and the destination alpha test both need the pixel that is there.
    bool need_dest = e.date || e.abe;
    u32 dest = need_dest ? frame_read(e, frow, x) : 0;

    /*
     * Destination alpha test: the pixel passes if the destination's alpha MSB (bit 31) equals
     * DATM. A 24-bit buffer has no alpha to test.
     */
    if (e.date && !e.f24 && ((dest >> 31) != 0) != e.datm) {
        return;
    }

    u32 zat = 0;
    if (e.zte) {
        // ZTST 0, NEVER: no pixel passes.
        if (e.ztst == 0) {
            return;
        }

        zat = GsMemory::index(zrow, static_cast<u32>(x));

        // ZTST 1 is ALWAYS and needs no read; 2 is GEQUAL and 3 is GREATER.
        if (e.ztst >= 2) {
            u32 stored = e.z16   ? memory.half(zat)
                         : e.z24 ? (memory.word(zat) & kRgb)
                                 : memory.word(zat);

            // The pixel fails when it is behind the stored depth (or level with it, for GREATER).
            if (e.ztst == 2 ? z < stored : z <= stored) {
                return;
            }
        }
    }

    // The pixel passed every test.
    tls_pixels++;

    if (write_rgb || write_a) {
        u32 out = rgba;

        // Blending, unless PABE is on and this pixel's source alpha MSB (bit 7) is clear.
        if (e.abe && !(e.pabe && !(sa & 0x80))) {
            u32 da = dest >> 24;

            // The blend factor C: source alpha, destination alpha or the fixed value FIX.
            s32 factor = e.bc == 0   ? static_cast<s32>(sa)
                         : e.bc == 1 ? static_cast<s32>(da)
                                     : static_cast<s32>(e.fix);

            // The colour is worked out below; the alpha stays the source's.
            out &= kA;

            /*
             * Cv = ((A - B) * C >> 7) + D, where A, B, D are the source colour, the destination
             * colour or zero (selectors 0, 1, 2); 0x80 in C stands for 1.0 (documented).
             */
            for (unsigned n = 0; n < 3; n++) {
                s32 cs = static_cast<s32>(ch(rgba, n)), cd = static_cast<s32>(ch(dest, n));
                s32 a = e.ba == 0 ? cs : e.ba == 1 ? cd : 0;
                s32 b = e.bb == 0 ? cs : e.bb == 1 ? cd : 0;
                s32 d = e.bd == 0 ? cs : e.bd == 1 ? cd : 0;
                s32 c = (((a - b) * factor) >> 7) + d;

                // COLCLAMP on holds the result to 0-255; off keeps the low 8 bits.
                c = e.colclamp ? std::clamp(c, 0, 255) : (c & 0xFF);
                out |= static_cast<u32>(c) << (n * 8);
            }
        }

        // FBA forces the alpha MSB (bit 31) on.
        if (e.fba) {
            out |= 0x80000000u;
        }

        // The bits to write: those FBMSK leaves open, in the parts the alpha test allows.
        u32 mask = ~e.fbmsk & ((write_rgb ? kRgb : 0) | (write_a ? kA : 0));
        if (mask) {
            frame_write(e, frow, x, out, mask);
        }
    }

    if (write_z) {
        // A 24-bit depth buffer shares its word with the top byte, which is kept.
        if (e.z16) {
            memory.set_half(zat, static_cast<u16>(z));
        } else if (e.z24) {
            memory.set_word(zat, (memory.word(zat) & kA) | (z & kRgb));
        } else {
            memory.set_word(zat, z);
        }
    }
}

// --- Transfers ---

void Gs::start_transfer() {
    // TRXDIR XDIR bits 0-1: 0 host to local, 1 local to host, 2 local to local, 3 none.
    u32 dir = static_cast<u32>(reg_[TRXDIR] & 3);

    // TRXREG: width RRW bits 0-11, height RRH bits 32-43.
    u64 size = reg_[TRXREG];
    u32 width = static_cast<u32>(bits(size, 0, 12)), height = static_cast<u32>(bits(size, 32, 12));
    in_ = Transfer{};
    out_ = Transfer{};

    // An empty area transfers nothing.
    if (width == 0 || height == 0) {
        return;
    }

    stats.transfers++;

    if (dir == 0) {
        in_.active = true;
        in_.width = width;
        in_.height = height;

        /*
         * The transfer's pixels are compared with what is there: have what is
         * waiting to be drawn there drawn first.
         * BITBLTBUF: DBP bits 32-45, DBW 48-53, DPSM 56-61. TRXPOS: DSAX bits 32-42, DSAY 48-58.
         */
        u64 to = reg_[BITBLTBUF], where = reg_[TRXPOS];
        s32 x = static_cast<s32>(bits(where, 32, 11)), y = static_cast<s32>(bits(where, 48, 11));
        in_pages_.clear();
        add_pages(
            in_pages_,
            static_cast<u32>(bits(to, 56, 6)),
            static_cast<u32>(bits(to, 32, 14)),
            static_cast<u32>(bits(to, 48, 6)),
            x,
            y,
            x + static_cast<s32>(width) - 1,
            y + static_cast<s32>(height) - 1
        );
        before_read(in_pages_);
    } else if (dir == 1) {
        finish();

        /*
         * Local to host: produce the whole image now, hand it out as it is asked for.
         * BITBLTBUF: SBP bits 0-13, SBW 16-21, SPSM 24-29. TRXPOS: SSAX bits 0-10, SSAY 16-26.
         */
        u64 buffer = reg_[BITBLTBUF], position = reg_[TRXPOS];
        u32 bp = static_cast<u32>(bits(buffer, 0, 14)), bw = static_cast<u32>(bits(buffer, 16, 6));
        u32 psm = static_cast<u32>(bits(buffer, 24, 6));
        u32 sx = static_cast<u32>(bits(position, 0, 11)),
            sy = static_cast<u32>(bits(position, 16, 11));
        unsigned bpp = transfer_bits(psm);

        // Pixels are packed into bytes without gaps: `acc` holds `have` bits not yet stored.
        u64 acc = 0;
        unsigned have = 0;
        for (u32 y = 0; y < height; y++) {
            for (u32 x = 0; x < width; x++) {
                // Coordinates wrap at 2048.
                acc |= static_cast<u64>(memory.read(psm, bp, bw, (sx + x) & 2047, (sy + y) & 2047))
                       << have;
                have += bpp;

                // Move every whole byte out of the accumulator.
                while (have >= 8) {
                    out_.pending.push_back(static_cast<u8>(acc));
                    acc >>= 8;
                    have -= 8;
                }
            }
        }

        // A last half byte (4-bit pixels, odd count) still goes out.
        if (have) {
            out_.pending.push_back(static_cast<u8>(acc));
        }

        // The data leaves in quadwords of 16 bytes: round up.
        out_.pending.resize((out_.pending.size() + 15) & ~std::size_t{15});
        out_.active = true;
    } else if (dir == 2) {
        copy_local();
    }
}

void Gs::transfer_in(const u8* data, std::size_t bytes) {
    // No host to local transfer is running: the data has nowhere to go.
    if (!in_.active) {
        return;
    }

    // BITBLTBUF: DBP bits 32-45, DBW 48-53, DPSM 56-61; TRXPOS: DSAX bits 32-42, DSAY 48-58.
    u64 buffer = reg_[BITBLTBUF], position = reg_[TRXPOS];
    u32 bp = static_cast<u32>(bits(buffer, 32, 14)), bw = static_cast<u32>(bits(buffer, 48, 6));
    u32 psm = static_cast<u32>(bits(buffer, 56, 6));
    u32 dx = static_cast<u32>(bits(position, 32, 11)),
        dy = static_cast<u32>(bits(position, 48, 11));
    unsigned bpp = transfer_bits(psm);

    /*
     * The games send the textures in view again every frame. Only a transfer
     * that changes something makes the decoded copies of that memory stale.
     */
    auto put = [&](u32 value) {
        // Coordinates wrap at 2048.
        u32 x = (dx + in_.x) & 2047, y = (dy + in_.y) & 2047;

        // A value is written only if it differs from what is there, compared at the pixel's width.
        if (memory.read(psm, bp, bw, x, y)
            != (value & (bpp == 32 ? 0xFFFFFFFFu : (1u << bpp) - 1))) {
            // The first change: draw what reads or writes there, and mark the pages written.
            if (!in_.changed) {
                in_.changed = true;
                before_write(
                    in_pages_
                );  // what is still to be drawn reads or writes this memory as it was
                stamp(in_pages_);
                env_dirty_ = true;  // a texture there is to be looked at again
            }
            memory.write(psm, bp, bw, x, y, value);
        }

        // Next pixel: along the row, then down; the transfer ends after the last row.
        if (++in_.x == in_.width) {
            in_.x = 0;
            if (++in_.y == in_.height) {
                in_.active = false;
            }
        }
    };

    in_.pending.insert(in_.pending.end(), data, data + bytes);
    std::size_t used = 0;
    const std::vector<u8>& p = in_.pending;

    if (bpp == 4) {
        // Two pixels to a byte, the low nibble first.
        while (used < p.size() && in_.active) {
            put(p[used] & 0xF);
            if (in_.active) {
                put(p[used] >> 4);
            }
            used++;
        }
    } else {
        // Whole bytes to a pixel, least significant first; a pixel split across calls waits.
        unsigned step = bpp / 8;
        while (used + step <= p.size() && in_.active) {
            u32 value = 0;
            for (unsigned n = 0; n < step; n++) {
                value |= static_cast<u32>(p[used + n]) << (n * 8);
            }
            put(value);
            used += step;
        }
    }

    // Keep the bytes not used yet, or all are done with when the transfer has ended.
    if (in_.active) {
        in_.pending
            .erase(in_.pending.begin(), in_.pending.begin() + static_cast<std::ptrdiff_t>(used));
    } else {
        in_.pending.clear();
    }
}

std::size_t Gs::transfer_out(u8* data, std::size_t bytes) {
    // No local to host transfer is running.
    if (!out_.active) {
        return 0;
    }

    // `out_.x` is the read position in the produced bytes.
    std::size_t n = std::min(bytes, out_.pending.size() - out_.x);
    std::memcpy(data, out_.pending.data() + out_.x, n);
    out_.x += static_cast<u32>(n);

    // Everything has been handed out: the transfer is over.
    if (out_.x == out_.pending.size()) {
        out_ = Transfer{};
    }

    return n;
}

void Gs::copy_local() {
    // BITBLTBUF: SBP bits 0-13, SBW 16-21, SPSM 24-29, DBP 32-45, DBW 48-53, DPSM 56-61.
    u64 buffer = reg_[BITBLTBUF], position = reg_[TRXPOS], size = reg_[TRXREG];
    u32 sbp = static_cast<u32>(bits(buffer, 0, 14)), sbw = static_cast<u32>(bits(buffer, 16, 6));
    u32 spsm = static_cast<u32>(bits(buffer, 24, 6));
    u32 dbp = static_cast<u32>(bits(buffer, 32, 14)), dbw = static_cast<u32>(bits(buffer, 48, 6));
    u32 dpsm = static_cast<u32>(bits(buffer, 56, 6));

    // TRXPOS: SSAX bits 0-10, SSAY 16-26, DSAX 32-42, DSAY 48-58, DIR 59-60.
    u32 sx = static_cast<u32>(bits(position, 0, 11)), sy = static_cast<u32>(bits(position, 16, 11));
    u32 dx = static_cast<u32>(bits(position, 32, 11)),
        dy = static_cast<u32>(bits(position, 48, 11));
    u32 order =
        static_cast<u32>(bits(position, 59, 2));  // bit 0: rows bottom up, bit 1: right to left
    u32 width = static_cast<u32>(bits(size, 0, 12)), height = static_cast<u32>(bits(size, 32, 12));

    // An empty area copies nothing.
    if (width == 0 || height == 0) {
        return;
    }

    // Everything gathered is drawn before the copy reads or writes the memory.
    finish();
    Pages to;
    add_pages(
        to,
        dpsm,
        dbp,
        dbw,
        static_cast<s32>(dx),
        static_cast<s32>(dy),
        static_cast<s32>(dx + width) - 1,
        static_cast<s32>(dy + height) - 1
    );
    before_write(to);
    stamp(to);

    // Pixel by pixel, in the order TRXPOS gives; coordinates wrap at 2048.
    for (u32 j = 0; j < height; j++) {
        u32 y = (order & 1) ? height - 1 - j : j;
        for (u32 i = 0; i < width; i++) {
            u32 x = (order & 2) ? width - 1 - i : i;
            u32 value = memory.read(spsm, sbp, sbw, (sx + x) & 2047, (sy + y) & 2047);
            memory.write(dpsm, dbp, dbw, (dx + x) & 2047, (dy + y) & 2047, value);
        }
    }
}

// --- Output ---

Image Gs::snapshot(u32 bp, u32 bw, u32 psm, int width, int height) {
    // Drawing computes as the host does; a vector unit may have changed the rounding.
    fp::want_nearest();
    finish();

    Image image;
    image.width = width;
    image.height = height;
    image.pixels.resize(static_cast<std::size_t>(width) * static_cast<std::size_t>(height));

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            u32 v = memory.read(psm, bp, bw, static_cast<u32>(x), static_cast<u32>(y));

            // 16-bit: 5:5:5:1 widened to 8 bits a channel; the alpha bit is dropped.
            if (is16(psm)) {
                v = pack((v & 0x1F) << 3, ((v >> 5) & 0x1F) << 3, ((v >> 10) & 0x1F) << 3, 0);
            }

            // The picture is opaque whatever the buffer's alpha.
            image.pixels
                [static_cast<std::size_t>(y) * static_cast<std::size_t>(width)
                 + static_cast<std::size_t>(x)] = v | kA;
        }
    }

    return image;
}

// What the display circuits are set to show now.
Gs::Shown Gs::shown() const {
    Shown what;

    // PMODE: bit 0 enables read circuit 1, bit 1 circuit 2; circuit 1 wins when both are on.
    u64 pmode = priv_[0];
    int circuit = (pmode & 1) ? 0 : (pmode & 2) ? 1 : -1;

    // No circuit is enabled: nothing is shown.
    if (circuit < 0) {
        return what;
    }

    // DISPFB1/2 and DISPLAY1/2 sit at indices 7 to 10 of the privileged registers (address >> 4).
    u64 fb = priv_[circuit == 0 ? 0x7 : 0x9], disp = priv_[circuit == 0 ? 0x8 : 0xA];

    // DISPFB: FBP bits 0-8 in units of 32 blocks, FBW 9-14, PSM 15-19, DBX 32-42, DBY 43-53.
    what.bp = static_cast<u32>(bits(fb, 0, 9)) * 32;
    what.bw = static_cast<u32>(bits(fb, 9, 6));
    what.psm = static_cast<u32>(bits(fb, 15, 5));
    what.x = static_cast<u32>(bits(fb, 32, 11));
    what.y = static_cast<u32>(bits(fb, 43, 11));

    // DISPLAY: MAGH bits 23-26, MAGV 27-28, DW 32-43, DH 44-54; size is (DW + 1) / (MAGH + 1).
    what.width = static_cast<int>((bits(disp, 32, 12) + 1) / (bits(disp, 23, 4) + 1));
    what.height = static_cast<int>((bits(disp, 44, 11) + 1) / (bits(disp, 27, 2) + 1));

    // No buffer width or no size: the circuit shows nothing.
    if (what.bw == 0 || what.width <= 0 || what.height <= 0) {
        return what;
    }

    // The picture cannot be wider than the buffer (64 pixels for each unit of FBW).
    what.width = std::min(what.width, static_cast<int>(what.bw * 64));
    what.on = true;
    return what;
}

void Gs::copy_shown(const Shown& what, Image& out) const {
    int width = what.width, height = what.height;
    out.width = width;
    out.height = height;
    out.pixels.resize(static_cast<std::size_t>(width) * static_cast<std::size_t>(height));

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Coordinates wrap at 2048.
            u32 v = memory.read(
                what.psm,
                what.bp,
                what.bw,
                (what.x + static_cast<u32>(x)) & 2047,
                (what.y + static_cast<u32>(y)) & 2047
            );

            // 16-bit: 5:5:5:1 widened to 8 bits a channel; the alpha bit is dropped.
            if (is16(what.psm)) {
                v = pack((v & 0x1F) << 3, ((v >> 5) & 0x1F) << 3, ((v >> 10) & 0x1F) << 3, 0);
            }

            // The picture is opaque whatever the buffer's alpha.
            out.pixels
                [static_cast<std::size_t>(y) * static_cast<std::size_t>(width)
                 + static_cast<std::size_t>(x)] = v | kA;
        }
    }
}

bool Gs::display(Image& out) {
    // Drawing computes as the host does; a vector unit may have changed the rounding.
    fp::want_nearest();
    finish();
    Shown what = shown();

    if (what.on) {
        copy_shown(what, out);
    }

    return what.on;
}

void Gs::display_later(Image&& buffer, std::function<void(bool shown, Image& picture)> done) {
    // Drawing computes as the host does; a vector unit may have changed the rounding.
    fp::want_nearest();
    Shown what = shown();
    flush();

    // Nothing is drawn in the background: take the picture now.
    if (!raster_) {
        if (what.on) {
            copy_shown(what, buffer);
        }
        done(what.on, buffer);
        return;
    }

    /*
     * In its place in the order of the batches; the pages it reads count as
     * read in place by a batch on its way.
     */
    if (what.on) {
        add_pages(
            inflight_read_,
            what.psm,
            what.bp,
            what.bw,
            static_cast<s32>(what.x),
            static_cast<s32>(what.y),
            static_cast<s32>(what.x) + what.width - 1,
            static_cast<s32>(what.y) + what.height - 1
        );
    }

    // The task is copied around, so the buffer is shared rather than moved into it.
    auto picture = std::make_shared<Image>(std::move(buffer));

    // Runs on the thread that draws batches, after the batches given before it.
    batch_->task = [this, what, picture, done = std::move(done)] {
        if (what.on) {
            copy_shown(what, *picture);
        }
        done(what.on, *picture);
    };

    batch_ = raster_->give(std::move(batch_));
    env_ = nullptr;
    env_dirty_ = clut_copy_dirty_ = true;
}

}  // namespace ps2
