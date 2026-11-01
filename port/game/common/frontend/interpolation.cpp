// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The frames drawn between the game's ticks, a slice of frontend/state.h: the kept tick drawn with
// its camera, mobys, poses and shadow part way from the last tick's to its own (draw_kept); when a
// frame is due and whether one is drawn before the next tick (set_tick, interpolating,
// wants_between, between, frame_due); the swap and what it tells the pacing of the display
// (swapped); the paused game drawn again (hold); and the frame's cost switches (OPENRAC_FRAME_COST,
// OPENRAC_SLOW_FRAME). The pacing's pure arithmetic is frontend/pacing.h's; frame() in
// frontend.cpp reads each tick from the game and keeps it (Kept) for what is drawn here.
//
// Pacing, input and presentation stay on the window thread. Vulkan world recording consumes an
// owned snapshot on RenderWorker; finish_render returns exclusive graphics ownership before UI work.

#include <algorithm>
#include <climits>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <format>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>
#include "common/timing_samples.h"

#include "platform/sdl.h"
#ifdef __SWITCH__
#include "platform/switch/overlay_stats.h"
#endif

#include "common/env.h"
#include "common/env_interval.h"
#include "common/log.h"
#include "common/sampler.h"
#include "common/slow_cpu.h"
#include "common/stats.h"
#include "debug_menu.h"
#include "debug_overlay.h"
#include "frame_pacing.h"
#include "frontend.h"
#include "game/common/frontend/geometry.h"
#include "game/common/frontend/pacing.h"
#include "game/common/frontend/state.h"
#include "game/common/frontend/tick_phases.h"
#include "openrac/game_host.h"
#include "platform/performance_hint.h"
#include "platform/pump.h"
#include "platform/window.h"
#include "renderer/direct.h"
#include "renderer/framebuffer.h"
#include "renderer/gl.h"
#include "renderer/renderer.h"
#include "renderer/shadows.h"
#include "renderer/texture_pool.h"
#include "renderer/ui/fps_counter.h"
#include "scene/level_renderer.h"
#include "scene/screenshot.h"
#include "settings.h"

namespace openrac::frontend {

namespace {

// OPENRAC_FRAME_COST=N: every N frames drawn (default 200), in the log, what a frame cost on
// average and where: `scene` (the tick's mobys and poses moved between ticks and sent to their
// buffers, on the CPU), `submit` (the renderers' OpenGL calls, on the CPU), `gpu` (the wait for the
// graphics card to finish them: this switch calls glFinish after each frame, so that the card's
// time is not counted into a later call; it lowers the frame rate a little) and `show` (the
// frame scaled to the window, and the swap). With the draw calls and triangles of the last frame.
// frame_cost is the one; only the window thread accumulates the completed job measurements.
struct FrameCost {
    bool on = false;
    int every = 200;
    int frames = 0;
    double scene = 0.0, submit = 0.0, gpu = 0.0, show = 0.0;
    std::chrono::steady_clock::time_point mark{};

    FrameCost() {
        if (env::value(env::Var::FrameCost)) {
            on = true;
            // Read as std::atoi read it, held to int's range, where atoi's answer was undefined.
            const std::int64_t asked = std::min<std::int64_t>(
                env::integer(env::Var::FrameCost, 0), std::numeric_limits<int>::max()
            );
            every = asked > 1 ? static_cast<int>(asked) : 200;
        }
    }

    // The seconds since the last call, added to `part`.
    void lap(double& part) {
        const auto now = std::chrono::steady_clock::now();
        part += std::chrono::duration<double>(now - mark).count();
        mark = now;
    }
} frame_cost;

// [debug tools] The picture being drawn is a paused game's (hold). Only the thread that opened the
// window touches it.
bool g_held = false;

// Returns the frames a second the frames between ticks are held to (the player's cap, or the
// display's rate), 0 for no limit and with one frame a tick.
//
// The cap and refresh share one deadline schedule. A quick presentation return
// is not proof of a missed VSync: Vulkan queues presentation asynchronously.
double frame_cap() {
    if (!interpolating()) {
        return 0.0;
    }
    const settings::Settings& s = settings::get();
    const double requested =
        s.frame_rate == settings::FrameRate::Capped
            ? static_cast<double>(
                  std::clamp(s.frame_cap, settings::kFrameCapMin, settings::kFrameCapMax)
              )
            : 0.0;
    return frame_pacing::limit(
        requested,
        s.frame_rate == settings::FrameRate::Display,
        g->swap != platform::SwapInterval::Immediate,
        pacing.interval
    );
}

// The refresh a frame started now is shown at: the first after the frame is drawn.
std::chrono::steady_clock::time_point next_display(std::chrono::steady_clock::time_point now) {
    return shown_at(now, pacing.cost, pacing.interval, pacing.last_swap);
}

// How far the scene is from the last tick's to the kept one's at that refresh.
float alpha_at(std::chrono::steady_clock::time_point display) {
    return tick_alpha(display, pacing.tick, pacing.period);
}

// OPENRAC_FRAME_CAP_FROM_HANDOFF=1 (reference): the frame cap counts the next frame's start from
// the handed-back frame's start shifted to its present, as before. That is the same instant for a
// frame drawn on the game thread; with the render worker the present comes a tick after the frame
// started, the next tick's frame, started right after it, was not yet due, the cap held it back
// and, with frames between ticks declined, it was never drawn (sessions 42-43: 0.58-0.90 presents
// a tick while the pacing line counted every tick frame as drawn).
bool frame_cap_from_handoff() {
    static const bool on = [] {
        const char* v = std::getenv("OPENRAC_FRAME_CAP_FROM_HANDOFF");
        const bool set = v != nullptr && std::string_view(v) == "1";
        if (set) {
            log::info("frontend: REFERENCE OPENRAC_FRAME_CAP_FROM_HANDOFF=1: the frame cap counts from "
                      "a handed-back frame's present");
        }
        return set;
    }();
    return on;
}

// OPENRAC_TICK_FRAME_PACING_REFERENCE=1 (reference): the tick's own frame paced as before session
// 55. (1) The frame cap lets it start only within 1 ms of its due time (frame_due), not
// tick_frame_tolerance's half a cap frame when the cap is at least the tick rate. (2) A frame the
// cap held back is declined by OPENRAC_NO_BETWEEN_WHEN_BEHIND like a frame between ticks, and so
// never drawn. (3) That experiment's decline comes before a finished render job is swapped, so the
// tick's frame waits for the next tick's work to be done before it is shown. (4) A tick frame is
// held back even where between() is not run (no camera yet), and so not drawn.
bool tick_frame_pacing_reference() {
    static const bool on = [] {
        const char* v = std::getenv("OPENRAC_TICK_FRAME_PACING_REFERENCE");
        const bool set = v != nullptr && std::string_view(v) == "1";
        if (set) {
            log::info("frontend: REFERENCE OPENRAC_TICK_FRAME_PACING_REFERENCE=1: tick frames paced "
                      "as before session 55 (1 ms cap tolerance, held-back frames declined when behind)");
        }
        return set;
    }();
    return on;
}

// OPENRAC_EVEN_PRESENT=1 (experiment, off by default): a tick's finished frame, swapped between
// ticks (between), is swapped no sooner than a tick less 1 ms after the last swap, and never past
// the next tick's time: the game thread waits there for no longer than it would sleep anyway. With
// vsync 0 the swaps otherwise follow when each tick's work ends, which varies by several ms.
bool even_present() {
    static const bool on = [] {
        const char* v = std::getenv("OPENRAC_EVEN_PRESENT");
        const bool set = v != nullptr && std::string_view(v) == "1";
        if (set) {
            log::info("frontend: EXPERIMENT OPENRAC_EVEN_PRESENT=1: a tick frame's swap held to a tick "
                      "after the last swap, at most to the next tick's time");
        }
        return set;
    }();
    return on;
}

// OPENRAC_PACING_RING=N (diagnostic, 0/unset off, 64..16384): the last N pacing events in memory,
// written to the log once, at close (dump_pacing_ring): no SD writes while the game runs. One
// allocation, at the first event. Window thread only. Times are microseconds from the first event.
struct RingEvent {
    enum Kind : std::uint8_t { Tick, Drawn, Deferred, DeferredDrawn, LostNextTick, LostWorker, LostLate, LostBehind, Swap };
    Kind kind;
    std::uint8_t rate;           // ticks a second
    std::uint16_t tolerance_us;  // Drawn/Deferred: the cap tolerance used
    std::uint32_t tick;          // the kept tick's frame number
    std::int64_t at;             // when (us)
    std::int64_t due;            // Tick: the tick's due time; Drawn/Deferred/Lost*: the cap's due; Swap: the frame's start
    std::int32_t a_us;           // Tick: behind; Swap: the swap call's own time
    std::int32_t b_us;           // Swap: CPU record time of the frame (render thread)
    std::int32_t c_us;           // Swap: GPU time the frame measured, when timed
};

struct Ring {
    std::vector<RingEvent> events;
    std::size_t next = 0, capacity = 0;
    bool wrapped = false;
    std::chrono::steady_clock::time_point origin{};
};

int pacing_ring_size() {
    static const int n = env_interval::parse(std::getenv("OPENRAC_PACING_RING"), 64, 16384);
    return n;
}

Ring& ring() {
    static Ring r;
    return r;
}

std::int64_t ring_us(std::chrono::steady_clock::time_point t) {
    Ring& r = ring();
    if (t.time_since_epoch().count() == 0) {
        return -1;
    }
    return std::chrono::duration_cast<std::chrono::microseconds>(t - r.origin).count();
}

void ring_add(RingEvent e) {
    Ring& r = ring();
    if (r.capacity == 0) {
        r.capacity = static_cast<std::size_t>(pacing_ring_size());
        r.events.resize(r.capacity);
    }
    r.events[r.next] = e;
    r.next = (r.next + 1) % r.capacity;
    r.wrapped = r.wrapped || r.next == 0;
}

void ring_event(RingEvent::Kind kind, std::chrono::steady_clock::time_point at,
                std::chrono::steady_clock::time_point due, std::int64_t a = 0, std::int64_t b = 0,
                std::int64_t c = 0, std::int64_t tolerance_us = 0) {
    if (pacing_ring_size() == 0) {
        return;
    }
    Ring& r = ring();
    if (r.origin.time_since_epoch().count() == 0) {
        r.origin = at;
    }
    const auto clamp32 = [](std::int64_t v) {
        return static_cast<std::int32_t>(std::clamp<std::int64_t>(v, INT32_MIN, INT32_MAX));
    };
    const double period = pacing.period.count();
    ring_add(RingEvent{
        kind,
        static_cast<std::uint8_t>(period > 0.0 ? std::clamp(1.0 / period + 0.5, 0.0, 255.0) : 0.0),
        static_cast<std::uint16_t>(std::clamp<std::int64_t>(tolerance_us, 0, 65535)),
        g && g->kept ? static_cast<std::uint32_t>(g->kept->frame) : 0u,
        ring_us(at), ring_us(due), clamp32(a), clamp32(b), clamp32(c)});
}

std::int64_t us_of(std::chrono::duration<double> d) {
    return static_cast<std::int64_t>(d.count() * 1e6);
}

// Swaps the window's buffers for the frame begun at `started`, and keeps what the swap tells the
// pacing: what a frame costs, the display's refresh interval when it is to be read again, the
// last swap that waited for a refresh, and when the frame cap lets the next frame start, counted
// from `frame_started`, when the frame was begun (draw_kept).
// The frame being swapped's render-thread record and GPU times, for the ring (present_frame).
std::int64_t ring_record_us = 0, ring_gpu_us = 0;

void swapped(
    std::chrono::steady_clock::time_point started, std::chrono::steady_clock::time_point frame_started
) {
    using namespace std::chrono;
    const auto before = steady_clock::now();
    platform::frame_work_done();
    const auto swap_started = steady_clock::now();
    g->window->swap();
    const auto after = steady_clock::now();
    const double cost = duration<double>(before - started).count();
    pacing.cost = 0.8 * pacing.cost + 0.2 * cost;
    if (pacing.last_present.time_since_epoch().count() != 0) {
        const double period = std::min(duration<double>(after - pacing.last_present).count(), 0.25);
        pacing.present_period =
            pacing.present_period > 0.0 ? 0.8 * pacing.present_period + 0.2 * period : period;
    }
    pacing.last_present = after;
    renderer::ui::fps_counter::frame_drawn(
        after, duration<double>(after - swap_started).count(), settings::get().show_fps
    );
    if (pacing.interval_stale) {
        const float rate = g->window->refresh_rate();
        pacing.interval = rate > 20.0f ? 1.0 / rate : 0.0;
        pacing.interval_stale = false;
    }
    // What a frame may take, for the system's CPU clocks (performance_hint.h): a refresh, or the
    // player's frame cap when it is lower.
    const double cap = frame_cap();
    platform::set_frame_target(cap > 0.0 ? std::max(1.0 / cap, pacing.interval) : pacing.interval);
    if (g->swap == platform::SwapInterval::Immediate || gl::is_vulkan()
        || duration<double>(after - before).count() < 0.0005) {
        // Vulkan's return is not a vblank timestamp, and an immediate GL return
        // supplies no display phase either. The shared deadline paces both;
        // never add a second refresh-length sleep after drawing the tick.
        pacing.last_swap = {};
    } else {
        pacing.last_swap = after;
    }
    // A frame rate of the player's own (and the display's, when no swap waits): the next frame is
    // due that long after this one was, so a frame held up by the game's tick is made up for by the
    // next ones; one more than a frame late starts the count again. It holds beside the swap's
    // wait: 60 frames a second on a 120 Hz display are every other refresh. Nothing sleeps here:
    // the game's tick may be due before the next frame is (frame_due).
    if (cap > 0.0) {
        const auto step = duration_cast<steady_clock::duration>(duration<double>(1.0 / cap));
        const auto now = steady_clock::now();
        pacing.due = frame_pacing::next_due(pacing.due, frame_started, now, step);
    } else {
        pacing.due = {};
    }
    ring_event(RingEvent::Swap, swap_started, frame_started,
               std::chrono::duration_cast<std::chrono::microseconds>(after - swap_started).count(),
               ring_record_us, ring_gpu_us);
    // The next frame's work counts from here: the swap, and any wait in it, is the display's.
    platform::work_resumed();
}

// OPENRAC_SLOW_FRAME=MS: every frame drawn takes that much longer, as on a graphics card too slow
// for the settings, for checking that the game's time does not slow with it.
void slow_frame() {
    static const double ms = env::number(env::Var::SlowFrame, 0.0);
    if (ms > 0.0) {
        std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(ms));
    }
}

}  // namespace

// Whether the frame cap lets a frame start now (within a millisecond: a sleep wakes that coarsely).
bool frame_due(std::chrono::steady_clock::time_point now) {
    return frame_may_start(frame_cap(), pacing.due, now);
}

bool tick_frame_pacing_reference_on() {
    return tick_frame_pacing_reference();
}

bool tick_frame_due(std::chrono::steady_clock::time_point now) {
    if (tick_frame_pacing_reference()) {
        return frame_due(now);
    }
    return tick_frame_may_start(frame_cap(), pacing.due, now, pacing.period, pacing.between_since_tick);
}

void ring_tick_frame(std::chrono::steady_clock::time_point now, bool drawn) {
    if (pacing_ring_size() == 0) {
        return;
    }
    const auto tolerance = tick_frame_pacing_reference()
                               ? std::chrono::steady_clock::duration(std::chrono::milliseconds(1))
                               : tick_frame_tolerance(frame_cap(), pacing.period, pacing.between_since_tick);
    ring_event(drawn ? RingEvent::Drawn : RingEvent::Deferred, now, pacing.due, 0, 0, 0,
               std::chrono::duration_cast<std::chrono::microseconds>(tolerance).count());
}

void dump_pacing_ring() {
    Ring& r = ring();
    if (r.capacity == 0) {
        return;
    }
    static constexpr const char* kNames[] = {"tick", "drawn", "deferred", "deferred-drawn", "lost-next-tick",
                                             "lost-worker", "lost-late", "lost-behind", "swap"};
    const std::size_t count = r.wrapped ? r.capacity : r.next;
    const std::size_t first = r.wrapped ? r.next : 0;
    log::info("pacing ring (OPENRAC_PACING_RING): {} events, microseconds from the first; columns: kind, "
              "tick, rate, at, due (tick: its time; drawn/deferred/lost: the cap's due; swap: the frame's "
              "start), a (tick: behind; swap: the swap call), b (swap: CPU record), c (swap: GPU), tolerance "
              "(drawn/deferred). Swap times are the CPU's present call, not the display's scan-out.",
              count);
    std::string lines;
    for (std::size_t i = 0; i < count; ++i) {
        const RingEvent& e = r.events[(first + i) % r.capacity];
        lines += std::format("{},{},{},{},{},{},{},{},{}\n", kNames[e.kind], e.tick, int(e.rate), e.at, e.due,
                             e.a_us, e.b_us, e.c_us, int(e.tolerance_us));
        if (lines.size() > 16384 || i + 1 == count) {
            if (!lines.empty() && lines.back() == '\n') {
                lines.pop_back();
            }
            log::info("pacing ring:\n{}", lines);
            lines.clear();
        }
    }
    r.events = {};
    r.capacity = 0;
    r.next = 0;
    r.wrapped = false;
}

// ---- The kept tick ----

bool enhanced_graphics_available() {
    return g && gl::enhanced_rendering_available() && g->scene.probe_data_available();
}

namespace {

// Writes a requested capture only after the submitted frame is complete.
void capture_frame(const FramePresentation& output) {
    const bool first = output.first;
    const unsigned shown = output.shown;
    const int width = output.width;
    const int height = output.height;
    // OPENRAC_SHOT=FRAME:FILE.png writes that frame (for checking without looking at the screen);
    // OPENRAC_SHOT_BETWEEN also writes the frames drawn between ticks (FILE<frame>_b<n>.png).
    static const bool& shot_between =
        stats::probe("OPENRAC_SHOT_BETWEEN", "OPENRAC_SHOT also writes the frames between ticks");
    static int between_index = 0;
    between_index = first ? 0 : between_index + 1;
    // OPENRAC_SHOT_FROM=FRAME: no shots before that frame.
    static const std::uint64_t shot_from = env_frame(env::Var::ShotFrom).value_or(0);
    if ((first || shot_between) && shots().due(output.tick) && output.tick >= shot_from) {
        const std::string path = shots().prefix + std::to_string(output.tick)
                                 + (first ? std::string() : "_b" + std::to_string(between_index))
                                 + ".png";
        scene::write_png(path, width, height, renderer::read_rgba(shown, width, height));
        log::info("frame {} written to {}", output.tick, path);
    }
}

// Reports the diagnostic's blocking measurements after presentation.
void report_frame_cost(const FramePresentation& output) {
    static const int cpu_every=[] {
        const auto* v=std::getenv("OPENRAC_CPU_TIMING");
        return v && std::atoi(v)>0 ? std::clamp(std::atoi(v),30,3600) : 0;
    }();
    if(cpu_every) {
        static TimingSamples scene,record,total;
        scene.add(output.scene_seconds*1000);record.add(output.record_seconds*1000);
        total.add(std::chrono::duration<double,std::milli>(output.finished-output.started).count());
        if(total.values.size()>=unsigned(cpu_every)) {
            log::info("cpu timing {} frames: scene avg {:.3f}; record avg {:.3f} p95 {:.3f} max {:.3f}; render CPU span avg {:.3f} ms (FRAME_COST active: {}; excludes game update and presentation)",
                total.values.size(),scene.mean(),record.mean(),record.p95(),record.maximum(),total.mean(),output.measure_cost);
            scene.clear();record.clear();total.clear();
        }
    }
    const int width = output.width;
    const int height = output.height;
    if (frame_cost.on) {
        frame_cost.lap(frame_cost.show);
        if (++frame_cost.frames >= frame_cost.every) {
            const double n = frame_cost.frames * 0.001;
            const renderer::FrameStats stats =
                g->renderer ? g->renderer->last_stats() : renderer::FrameStats{};
            log::info(
                "cost tick {}: {:.3f} ms a frame over {} frames (scene {:.3f}, submit {:.3f}, "
                "gpu {:.3f}, show {:.3f}); {} draw calls, {} triangles, frame {}x{} x{}",
                g->kept->frame,
                (frame_cost.scene + frame_cost.submit + frame_cost.gpu + frame_cost.show) / n,
                frame_cost.frames,
                frame_cost.scene / n,
                frame_cost.submit / n,
                frame_cost.gpu / n,
                frame_cost.show / n,
                stats.draw_calls,
                stats.triangles,
                width,
                height,
                g->frame.samples()
            );
            frame_cost.frames = 0;
            frame_cost.scene = frame_cost.submit = frame_cost.gpu = frame_cost.show = 0.0;
        }
    }
}

// Logs the submitted interpolation fraction using window-thread pacing state.
void log_pacing(const FramePresentation& output) {
    // OPENRAC_PACING: each frame shown, when (ms), how far between ticks, the refresh interval.
    static const bool& pacing_log =
        stats::probe("OPENRAC_PACING", "each frame shown and when, in the log");
    if (pacing_log) {
        static const auto t0 = output.started;
        log::info(
            "pace {} {:.2f} ms alpha {:.3f} {} refresh {:.2f} ms cost {:.2f} ms",
            g->kept->frame,
            std::chrono::duration<double, std::milli>(output.started - t0).count(),
            output.alpha,
            output.first ? "tick" : "between",
            pacing.interval * 1000.0,
            pacing.cost * 1000.0
        );
    }
}

// OPENRAC_DK_RENDER_THREAD=1 (experiment, off by default): deko3d records the world on the render
// worker too, as Vulkan does (one owned snapshot in flight; finish_render hands graphics back
// before the UI and the present). Without it every deko3d frame is drawn on the game thread
// between the game's ticks: the game's code, reading the tick, recording it and presenting it run
// one after another (sessions 39-40: about 12 ms of game code, 5 ms of recording and the rest of
// a 28-33 ms tick on one thread while the GPU waited).
bool dk_render_thread() {
    static const bool on = [] {
        const char* v = std::getenv("OPENRAC_DK_RENDER_THREAD");
        const bool set = v != nullptr && std::string_view(v) == "1";
        if (set && gl::is_deko3d()) {
            log::info(
                "frontend: EXPERIMENT OPENRAC_DK_RENDER_THREAD=1: the deko3d world is recorded on "
                "the render worker while the game runs its next tick"
            );
        } else if (set) {
            log::info("frontend: OPENRAC_DK_RENDER_THREAD=1 ignored: the renderer is not deko3d");
        }
        return set && gl::is_deko3d();
    }();
    return on;
}

// Where the render worker runs (RenderWorker::Placement). OPENRAC_RENDER_CORE=N puts it on core N,
// -1 leaves it where the system starts it; unset, the deko3d worker goes to a core other than the
// game thread's (on the Switch a new thread otherwise starts on its creator's core, where it
// would only take turns with the game) and the Vulkan worker is left as it was.
RenderWorker::Placement render_placement() {
    RenderWorker::Placement placement;
    const char* v = std::getenv("OPENRAC_RENDER_CORE");
    if (v != nullptr && *v != '\0') {
        placement.core = std::atoi(v);
        placement.move = placement.core >= 0;
    } else {
        placement.move = dk_render_thread();
    }
    return placement;
}

// Pins the immutable captured scene and transfers graphics ownership to its persistent worker.
void queue_frame(const std::shared_ptr<FramePresentation>& output) {
    // The worker is persistent; every job owns poses, packets, textures and graphics settings.
    if (!g->render_worker) {
        // These renderer capability caches must be populated before the game and worker can query them.
        [[maybe_unused]] const int samples = renderer::FrameBuffer::max_samples();
        [[maybe_unused]] const float anisotropy = renderer::TexturePool::max_anisotropy();
        g->render_worker = std::make_unique<RenderWorker>(render_placement());
        log::info(
            "{} world rendering on a dedicated thread; one snapshot in flight",
            gl::is_vulkan() ? "Vulkan" : "deko3d"
        );
    }
    const auto snapshot_started = std::chrono::steady_clock::now();
    auto snapshot =
        std::make_shared<RenderSnapshot>(RenderSnapshot{g->kept, settings::get(), output});
    stats::gauge(
        "render snapshot ms",
        std::chrono::duration<double, std::milli>(
            std::chrono::steady_clock::now() - snapshot_started
        )
            .count()
    );
    g->pending_frame = output;
    std::string error;
    State& owner = *g;
    const bool queued = g->render_worker->submit(
        [&owner, snapshot = std::move(snapshot)] {
            render_scene(owner, *snapshot->scene, snapshot->graphics, *snapshot->presentation);
            sampler::note_thread("render");  // [debug tools] sampled while a capture runs
            // [debug tools] The render thread's share of a simulated slow processor.
            const stats::Scope slow("simulated slow cpu");
            slow_cpu::pad();
        },
        error
    );
    // Job rejection means a previous failure, never an unbounded queue.
    if (!queued) {
        g->pending_frame.reset();
        g->render_failed = true;
        log::error("render thread: {}", error);
    }
}

// Draws the debug UI after the render worker has released all referenced graphics state.
void draw_overlay(const FramePresentation& output) {
    const int window_width = output.window_width;
    const int window_height = output.window_height;
    const Box box = output.box;
    // [debug tools] The overlay, over the frame in the window.
    debug_overlay::draw(
        {g->window->sdl(),
         window_width,
         window_height,
         box.x,
         box.y,
         box.width,
         box.height,
         g->renderer.get(),
         &output.camera,
         g->frame.width(),
         g->frame.height(),
         g->last_world.width(),
         g->last_world.height(),
         g->swap != platform::SwapInterval::Immediate,
         interpolating(),
         pacing.interval,
         pacing.cost,
         pacing.period.count()}
    );
}

// Composites debug UI and presents on the window thread, after exclusive graphics ownership returns.
void present_frame(FramePresentation& output) {
    // OPENRAC_TICK_PHASES: the present's share of the game thread's tick.
    const tick_phases::Timer phase(tick_phases::Present);
    tick_phases::count_present();
    // What is shown: the frame with one sample a pixel (a multisampled one is resolved here, once),
    // with the port's own overlays over it.
    const unsigned shown = output.shown;
    const int width = output.width;
    const int height = output.height;
    const int window_width = output.window_width;
    const int window_height = output.window_height;
    const float aspect = output.aspect;
    const auto resumed = std::chrono::steady_clock::now();
    const auto started = output.started + (resumed - output.finished);
    frame_cost.scene += output.scene_seconds;
    frame_cost.submit += output.record_seconds;
    frame_cost.gpu += output.gpu_seconds;
    frame_cost.mark = resumed;
    // The window thread's own parts of a frame, measured apart (performance captures).
    {
        const stats::Scope menus("menus");
        debug_menu::draw(shown, width, height);
        if (settings::get().show_fps != settings::FrameCounter::Off) {
            renderer::ui::fps_counter::draw(shown, width, height);
        }
    }
    {
        const stats::Scope capture("capture");
        capture_frame(output);
        slow_frame();
    }
    present(shown, width, height, window_width, window_height, false, aspect);
    {
        const stats::Scope touch("touch pad");
        draw_touch_pad(window_width, window_height);
    }
    draw_overlay(output);
    const stats::Scope swap_scope("swap");
    ring_record_us = us_of(std::chrono::duration<double>(output.record_seconds));
    ring_gpu_us = us_of(std::chrono::duration<double>(output.gpu_seconds));
    swapped(started, frame_cap_from_handoff() ? started : output.started);
#ifdef __SWITCH__
    if (g->window->last_swap_presented())
        platform::overlay_stats::presented(width, height, window_width, window_height);
#endif
    report_frame_cost(output);
}

}  // namespace

bool render_pending() {
    return g && static_cast<bool>(g->pending_frame);
}

bool finish_render() try {
    // [debug tools] A simulated slow processor (common/slow_cpu.h): the window thread's work since
    // it last waited (the game's tick, the next frame's preparation, the frame it hands over),
    // padded before it waits for the render thread, as a slower processor would still be busy
    // while the render thread draws. Measured only when no render job holds the profiler's
    // section stack.
    {
        std::optional<stats::Scope> slow;
        if (!render_pending()) {
            slow.emplace("simulated slow cpu");
        }
        slow_cpu::pad();
    }
    // Synchronous frames and initialisation have no handoff to drain.
    if (!render_pending()) {
        return !g || !g->render_failed;
    }
    const auto started = std::chrono::steady_clock::now();
    std::string error;
    bool complete = false;
    {
        const tick_phases::Timer phase(tick_phases::WorkerWait);
        complete = g->render_worker->finish(error);
    }
    auto output = std::move(g->pending_frame);
    // A worker failure has already relinquished all graphics resources.
    if (!complete) {
        log::error("render thread: {}", error);
        g->render_failed = true;
        return false;
    }
    stats::gauge(
        "render thread wait ms",
        std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - started)
            .count()
    );
    present_frame(*output);
    return true;
} catch (const std::exception& failure) {
    g->render_failed = true;
    log::error("cannot finish render: {}", failure.what());
    return false;
} catch (...) {
    g->render_failed = true;
    log::error("cannot finish render: unknown exception");
    return false;
}

void draw_kept(bool first) try {
    sampler::note_thread("window");  // [debug tools] sampled while a capture runs
    // One in-flight snapshot bounds both latency and ownership of the current scene (and the
    // simulated slow processor's share of the window thread is spent there).
    if (!finish_render()) {
        return;
    }
    auto output = std::make_shared<FramePresentation>();
    output->started = std::chrono::steady_clock::now();
    output->first = first;
    output->alpha = interpolating() ? alpha_at(next_display(output->started)) : 1.0f;
    g->window->drawable_size(output->window_width, output->window_height);
    output->aspect = picture_aspect(output->window_width, output->window_height);
    output->box = frame_box(output->window_width, output->window_height, output->aspect);
    debug_overlay::begin(first, g_held, output->alpha, g->kept->frame);
    pacing.last_alpha = output->alpha;
    frame_cost.mark = output->started;
    output->measure_cost = frame_cost.on;
    log_pacing(*output);
    // The tick's own frame: its image transfers reach the renderer now.
    if (first) {
        image_sends_drawn();
    }
    static const bool requested = env::flag(env::Var::RenderThread);
    // deko3d only with OPENRAC_DK_RENDER_THREAD=1 (dk_render_thread); OPENRAC_RENDER_THREAD=0 still
    // keeps both on this thread.
    const bool threaded = (gl::is_vulkan() || dk_render_thread()) && requested;
    // OpenGL contexts retain the original window-thread path.
    if (!threaded) {
        {
            const tick_phases::Timer phase(tick_phases::Render);
            render_scene(*g, *g->kept, settings::get(), *output);
        }
        present_frame(*output);
        return;
    }
    {
        const tick_phases::Timer phase(tick_phases::Render);
        queue_frame(output);
    }

} catch (const std::exception& failure) {
    g->pending_frame.reset();
    g->render_failed = true;
    log::error("cannot prepare render snapshot: {}", failure.what());
} catch (...) {
    g->pending_frame.reset();
    g->render_failed = true;
    log::error("cannot prepare render snapshot: unknown exception");
}

// ---- Between ticks ----

PacingCounts pacing_counts;

namespace {
int pacing_counts_every() {
    static const int every = [] {
        const char* v = std::getenv("OPENRAC_PACING_COUNTS");
        return v != nullptr && std::atoi(v) > 0 ? std::clamp(std::atoi(v), 30, 3600) : 0;
    }();
    return every;
}

// OPENRAC_NO_BETWEEN_WHEN_BEHIND=1 (experiment, off by default): no frame between ticks while the
// game's tick came half a tick or more after its time. Each such frame is a whole frame of GPU work that does not
// advance the game; when the GPU cannot keep up, it slows the game's own ticks further.
bool no_between_when_behind() {
    static const bool on = [] {
        const char* v = std::getenv("OPENRAC_NO_BETWEEN_WHEN_BEHIND");
        const bool set = v != nullptr && std::string_view(v) == "1";
        if (set) {
            log::info("frontend: EXPERIMENT OPENRAC_NO_BETWEEN_WHEN_BEHIND=1: no frames between ticks while a tick is late");
        }
        return set;
    }();
    return on;
}

// OPENRAC_BETWEEN_PRESENT_COST=1 (experiment, off by default): a frame between ticks is drawn only
// if the display's own pace (Pacing::present_period, which holds the GPU's time when it is the
// slower) would show it before the next tick is due, plus the lag. The default estimate,
// Pacing::cost, is the window thread's CPU time before the swap only: with the GPU taking ~60 ms a
// frame (session 33) it still predicted a refresh in time, and up to 117 frames between 120 ticks
// were drawn while every tick was behind, each a whole GPU frame that does not advance the game.
bool between_present_cost() {
    static const bool on = [] {
        const char* v = std::getenv("OPENRAC_BETWEEN_PRESENT_COST");
        const bool set = v != nullptr && std::string_view(v) == "1";
        if (set) {
            log::info("frontend: EXPERIMENT OPENRAC_BETWEEN_PRESENT_COST=1: frames between ticks only when the display's measured pace shows them before the next tick");
        }
        return set;
    }();
    return on;
}

// One line each OPENRAC_PACING_COUNTS ticks: where the presents went.
void report_pacing_counts() {
    const int every = pacing_counts_every();
    if (every == 0 || pacing_counts.ticks < static_cast<std::uint64_t>(every)) {
        return;
    }
    const PacingCounts& c = pacing_counts;
    const double n = static_cast<double>(c.ticks);
    log::info(
        "pacing over {} ticks: tick frames drawn {} (held back by the frame cap and drawn between ticks {}; "
        "held back and never drawn {}), left out late {}, other ticks undrawn {}, frames between ticks {} "
        "({:.2f} presents a tick); ticks behind their time {} (over 100 ms, where vsync restarts its count: {}); "
        "between frames declined by experiments: present cost {}, behind {}; late ticks still drawn: after 3 left out {}, "
        "2D list uploads an image {}, other {}; display pace {:.1f} ms a frame, CPU cost {:.1f} ms; "
        "held back and never drawn, by why: next tick first {}, render thread busy {}, too late {}, "
        "declined behind (reference) {}; drawn at once by the tick tolerance {}",
        c.ticks, c.tick_frames, c.deferred_drawn,
        c.deferred > c.deferred_drawn ? c.deferred - c.deferred_drawn : 0, c.left_out,
        c.ticks > c.tick_frames + c.left_out ? c.ticks - c.tick_frames - c.left_out : 0, c.between,
        static_cast<double>(c.tick_frames + c.between) / n, c.behind, c.far_behind,
        c.declined_present_cost, c.declined_behind, c.late_kept_limit, c.late_kept_image, c.late_kept_other,
        pacing.present_period * 1000.0, pacing.cost * 1000.0, c.deferred_lost_next_tick,
        c.deferred_lost_worker, c.deferred_lost_late, c.deferred_lost_behind, c.tolerance_admitted
    );
    // The image sends of the late ticks kept because of them, the most frequent first.
    if (!c.image_sends.empty()) {
        std::vector<std::pair<std::array<std::uint32_t, 4>, std::uint32_t>> sends(
            c.image_sends.begin(), c.image_sends.end()
        );
        std::sort(sends.begin(), sends.end(), [](const auto& a, const auto& b) { return a.second > b.second; });
        std::string list;
        for (std::size_t i = 0; i < sends.size() && i < 6; ++i) {
            const auto& [k, n] = sends[i];
            list += std::format("{}GS block {:#x} format {:#x} {}x{}: {}", i ? "; " : "", k[0], k[1], k[2], k[3], n);
        }
        log::info("pacing: image sends in late ticks kept for them, decoded tag by tag ({} destinations): {}", sends.size(), list);
    }
    if (c.late_kept_image != 0 || c.left_out_repeat != 0) {
        log::info(
            "pacing: of {} late ticks kept for an image, {} send none when decoded tag by tag (the "
            "qword reading's false match) and {} could not be decoded cleanly; late ticks left out "
            "although the qword reading saw an image (OPENRAC_LATE_IMAGE_REPEAT_SKIP): {}",
            c.late_kept_image, c.late_image_none, c.late_image_unread, c.left_out_repeat
        );
    }
    if (c.late_image_unread != 0) {
        const auto& u = c.unread_by;  // image_sends::Problem order, None first
        log::info(
            "pacing: late image paths not read cleanly, by first problem: header not DIRECT {}, "
            "DIRECT past the end {}, bytes after the last DIRECT {}, PACKED tag cut by its DIRECT {}, "
            "REGLIST tag cut {}, IMAGE data past its DIRECT with no transfer open {}, image data open "
            "at the end {}",
            u[1], u[2], u[3], u[4], u[5], u[6], u[7]
        );
    }
    {
        const auto& v = c.repeat_verdicts;  // frame.cpp RepeatVerdict order
        std::uint64_t any = 0;
        for (const auto n : v) {
            any += n;
        }
        if (any != 0) {
            log::info(
                "pacing: OPENRAC_LATE_IMAGE_REPEAT_SKIP verdicts on late image ticks: repeat (may be "
                "left out) {}, not read cleanly {}, incomplete transfer {}, read-back {}, copy inside "
                "GS memory {}, render to a texture {}, no drawn reference {}, different sends {} "
                "(repeats whose paths had tags the renderer drops too: {})",
                v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7], c.repeat_with_dropped
            );
        }
    }
    if (c.diff_count + c.diff_place + c.diff_bytes != 0) {
        log::info(
            "pacing: different sends, first difference from the last drawn tick's: number of "
            "transfers {}, a transfer's place {} (of them the same transfers in another order {}), "
            "only a transfer's bytes {}",
            c.diff_count, c.diff_place, c.diff_order_only, c.diff_bytes
        );
    }
    if (!c.destination_changes.empty()) {
        std::vector<std::pair<std::array<std::uint32_t, 5>, PacingCounts::DestinationChanges>> d(
            c.destination_changes.begin(), c.destination_changes.end()
        );
        std::sort(d.begin(), d.end(), [](const auto& a, const auto& b) { return a.second.changed > b.second.changed; });
        std::string list;
        for (std::size_t i = 0; i < d.size() && i < 8; ++i) {
            const auto& [k, v] = d[i];
            list += std::format("{}block {:#x} format {:#x} {}x{} #{}: changed {} of {} ({} contents)",
                                i ? "; " : "", k[0], k[1], k[2], k[3], k[4], v.changed, v.seen, v.distinct);
        }
        log::info("pacing: transfer destinations by how often their bytes change ({} destinations; "
                  "contents counted to 8 per run): {}", d.size(), list);
    }
    pacing_counts = {};
}
}  // namespace

void count_tick_frame(bool left_out) {
    if (pacing_counts_every() != 0) {
        ++(left_out ? pacing_counts.left_out : pacing_counts.tick_frames);
    }
}

void set_tick(
    std::chrono::steady_clock::time_point tick,
    std::chrono::duration<double> period,
    std::chrono::duration<double> late,
    std::chrono::duration<double> behind
) {
    pacing.tick = tick;
    pacing.period = period;
    pacing.late = late;
    pacing.behind = behind;
    if (pacing_ring_size() != 0) {
        ring_event(RingEvent::Tick, std::chrono::steady_clock::now(), tick, us_of(behind));
    }
    if (pacing_counts_every() != 0) {
        report_pacing_counts();
        ++pacing_counts.ticks;
        pacing_counts.behind += behind.count() > 0.0 ? 1 : 0;
        pacing_counts.far_behind += behind.count() > 0.1 ? 1 : 0;
    }
}

bool interpolating() {
    return !one_frame_a_tick() && settings::get().frame_rate != settings::FrameRate::Original && g
           && g->window;
}

bool wants_between() {
    return interpolating() && g->kept->has_state && g->kept->has_camera;
}

int between(std::chrono::steady_clock::time_point next_tick) {
    if (!g) {
        return 0;
    }
    const bool reference = tick_frame_pacing_reference();
    // A tick frame the cap held back, not drawn now: never drawn (the next tick's frame follows).
    const auto lost = [&](std::uint64_t PacingCounts::*why, RingEvent::Kind kind) {
        if (pacing.first_pending) {
            if (pacing_counts_every() != 0) {
                ++(pacing_counts.*why);
            }
            if (pacing_ring_size() != 0) {
                ring_event(kind, std::chrono::steady_clock::now(), pacing.due);
            }
        }
        return 0;
    };
    // Half a tick late or more: not a sleep's wake-up jitter, the game falling behind. (`behind` is
    // measured after the tick's own work and the wait for the last frame, so it holds that work
    // too.) It declines frames between ticks only: a tick's frame the cap held back is that tick's
    // only picture, and a finished frame is still swapped first (below). Before session 55 it came
    // first and declined both (OPENRAC_TICK_FRAME_PACING_REFERENCE=1).
    const bool behind_declines = no_between_when_behind() && pacing.behind > pacing.period * 0.5;
    if (behind_declines && reference) {
        ++pacing_counts.declined_behind;
        return lost(&PacingCounts::deferred_lost_behind, RingEvent::LostBehind);
    }
    // A new interpolation snapshot must not replace an in-flight one past the next tick's deadline.
    if (render_pending()) {
        if (!g->render_worker->ready_until(next_tick)) {
            return lost(&PacingCounts::deferred_lost_worker, RingEvent::LostWorker);
        }
        // OPENRAC_EVEN_PRESENT=1: no sooner than a tick less 1 ms after the last swap, never past
        // the next tick's time (the game thread would sleep until then anyway).
        if (even_present() && !reference && pacing.last_present.time_since_epoch().count() != 0) {
            const auto even = pacing.last_present
                              + std::chrono::duration_cast<std::chrono::steady_clock::duration>(pacing.period)
                              - std::chrono::milliseconds(1);
            const auto until = std::min(even, next_tick);
            if (until > std::chrono::steady_clock::now()) {
                std::this_thread::sleep_until(until);
                platform::work_resumed();
            }
        }
        if (!finish_render()) {
            return -1;
        }
    }
    if (behind_declines && !pacing.first_pending) {
        ++pacing_counts.declined_behind;
        return 0;
    }
    // A frame only for a refresh the kept tick is still shown at (the next tick's, with the lag).
    const auto lag =
        std::chrono::duration_cast<std::chrono::steady_clock::duration>(pacing.period * kLagTicks);
    auto now = std::chrono::steady_clock::now();
    // The frame cap: a frame not due before the next tick is left to after it; one due sooner is
    // waited for here.
    if (!frame_due(now)) {
        if (pacing.due >= next_tick) {
            return lost(&PacingCounts::deferred_lost_next_tick, RingEvent::LostNextTick);
        }
        std::this_thread::sleep_until(pacing.due);
        platform::work_resumed();
        now = std::chrono::steady_clock::now();
    }
    const auto display = next_display(now);
    if (between_present_cost() && pacing.present_period > 0.0
        && now + std::chrono::duration_cast<std::chrono::steady_clock::duration>(
                     std::chrono::duration<double>(pacing.present_period)
                 ) >= next_tick + lag) {
        ++pacing_counts.declined_present_cost;
        return lost(&PacingCounts::deferred_lost_late, RingEvent::LostLate);
    }
    if (display >= next_tick + lag) {
        return lost(&PacingCounts::deferred_lost_late, RingEvent::LostLate);
    }
    // Nothing waits for the display: a frame that would not be done by the next tick's time is not
    // started, so that the tick comes on time whatever a frame costs (with the swap's wait the
    // frame belongs to a refresh, and the tick after it is a little late and made up for).
    if (g->swap == platform::SwapInterval::Immediate
        && now
                   + std::chrono::duration_cast<std::chrono::steady_clock::duration>(
                       std::chrono::duration<double>(pacing.cost)
                   )
               >= next_tick) {
        return lost(&PacingCounts::deferred_lost_late, RingEvent::LostLate);
    }
    // The scene has arrived at the kept tick's: another frame would be the same picture again.
    if (!pacing.first_pending && pacing.last_alpha >= 1.0f && alpha_at(display) >= 1.0f) {
        return 0;
    }
    {
        const stats::Scope events("events");
        SDL_Event event;
        while (platform::poll_event(event)) {
            if (event.type == SDL_EVENT_QUIT) {
                return -1;
            }
            if (window_key(event) || debug_overlay::event(event)
                || debug_menu::event(event, openrac_game_loaded_overlay())) {
                continue;
            }
            g->input.handle_event(event);
        }
    }
    const bool dump = renderer::g_dump_draws;
    renderer::g_dump_draws = false;
    const bool first = pacing.first_pending;
    pacing.first_pending = false;
    pacing.between_since_tick = !first;
    if (first) {
        ring_event(RingEvent::DeferredDrawn, now, pacing.due);
    }
    draw_kept(first);
    const bool displayed = finish_render();
    if (displayed && pacing_counts_every() != 0) {
        // A tick's own frame the frame cap held back (draw_tick_frame), drawn now, is a tick frame.
        if (first) {
            ++pacing_counts.tick_frames;
            ++pacing_counts.deferred_drawn;
        } else {
            ++pacing_counts.between;
        }
    }
    renderer::g_dump_draws = dump;
    return displayed ? 1 : -1;
}

// [debug tools] While the overlay holds the game (pause): the last tick drawn again, the window's
// events read, until it lets a tick through. False once the window was closed.
bool hold() {
    // Pausing requires a complete presented frame before the debug UI reads it.
    if (debug_overlay::holding() && !finish_render()) {
        return false;
    }
    while (g && g->window && debug_overlay::holding()) {
        SDL_Event event;
        while (platform::poll_event(event)) {
            if (event.type == SDL_EVENT_QUIT) {
                return false;
            }
            if (window_key(event) || debug_overlay::event(event)) {
                continue;
            }
            g->input.handle_event(event);
        }
        if (debug_overlay::take_settings_changed()) {
            apply_settings();  // the drawer is open over the paused game
        }
        const auto started = std::chrono::steady_clock::now();
        const bool dump = renderer::g_dump_draws;
        renderer::g_dump_draws = false;
        g_held = true;
        draw_kept(false);
        const bool displayed = finish_render();
        if (!displayed) {
            return false;
        }
        g_held = false;
        renderer::g_dump_draws = dump;
        // No faster than the display shows them (a 120 Hz screen's drawer moves at 120), and no
        // faster than 60 a second where its rate is not known.
        const double interval = pacing.interval > 0.0 ? pacing.interval : 1.0 / 60.0;
        std::this_thread::sleep_until(
            started
            + std::chrono::duration_cast<std::chrono::steady_clock::duration>(
                std::chrono::duration<double>(interval * 0.9)
            )
        );
    }
    return true;
}

}  // namespace openrac::frontend
