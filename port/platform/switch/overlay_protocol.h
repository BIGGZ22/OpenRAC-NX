// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <algorithm>
#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <limits>

namespace openrac::platform::overlay_stats {
// Wire layout documented by masagrator/SaltyNX, NX-FPS-Common.hpp.
// The first 174 bytes also match older NX-FPS readers. Do not advertise NVN counters for EGL.
struct __attribute__((packed)) ResolutionCalls {
    std::uint16_t width, height, calls;
};
struct __attribute__((packed)) FpsBlock {
    std::uint32_t magic;
    std::uint8_t fps;
    float average;
    bool active;
    std::uint8_t fps_locked, fps_mode, zero_sync, patch_applied, api;
    std::uint32_t ticks[10];
    std::uint8_t buffers, set_buffers, active_buffers, set_active_buffers, display_sync;
    ResolutionCalls render[8], viewport[8];
    bool force_original_refresh, dont_force_60_docked, force_suspend;
    std::uint8_t refresh_rate;
    float read_speed;
    std::uint8_t fps_locked_docked;
    std::uint64_t frame_number;
    std::int8_t expected_set_buffers;
    std::uint8_t performance_counters[136];
};
static_assert(sizeof(FpsBlock) == 310);
static_assert(offsetof(FpsBlock, ticks) == 15);
static_assert(offsetof(FpsBlock, render) == 60);
static_assert(offsetof(FpsBlock, frame_number) == 165);
inline constexpr std::uint32_t kFpsMagic = 0x465053;
struct __attribute__((packed)) ReverseBlock {
    std::uint32_t magic;
    bool docked, system_control, active;
    std::uint8_t resolutions;
    bool ddr_used;
};
static_assert(sizeof(ReverseBlock) == 9);
inline constexpr std::uint32_t kReverseMagic = 0x5452584e;

// Pure timing arithmetic, independent of libnx and of the requested frame cap.
struct Timing {
    std::uint64_t previous = 0, frames = 0, window_start = 0, window_frames = 0;
    std::array<std::uint32_t, 10> intervals{};
    unsigned next = 0;
    std::uint8_t fps = 0;
    float average = 0;

    bool present(std::uint64_t tick, std::uint64_t frequency) {
        ++frames;
        if (!previous || !frequency || tick <= previous) {
            previous = window_start = tick;
            window_frames = 0;
            return false;
        }
        const auto delta = std::uint32_t(std::min<std::uint64_t>(tick - previous, UINT32_MAX));
        if (intervals[0] == 0) intervals.fill(delta);
        intervals[next] = delta;
        next = (next + 1) % intervals.size();
        previous = tick;
        ++window_frames;
        const auto elapsed = tick - window_start;
        if (elapsed >= frequency) {
            const double measured = double(window_frames) * double(frequency) / double(elapsed);
            fps = std::uint8_t(std::clamp(measured + 0.5, 0.0, 254.0));
            window_start = tick;
            window_frames = 0;
        }
        std::uint64_t total = 0;
        for (auto interval : intervals) total += interval;
        average = total ? float(double(frequency) * intervals.size() / double(total)) : 0;
        return true;
    }
};

// Both legacy Status Monitor and Deux consume this common prefix. Neither has
// a deko3d API ID: 2 is the non-NVN compatibility channel for our own publisher,
// not a claim that EGL hooks or NVN performance counters are present.
inline void publish(volatile FpsBlock& out, const Timing& timing, std::uint8_t api,
                    int render_width, int render_height, int output_width, int output_height) {
    const auto dimension=[](int value) { return std::uint16_t(std::clamp(value,0,65535)); };
    out.api=api;
    out.frame_number=timing.frames;
    out.fps=timing.fps;
    out.average=timing.average;
    for(unsigned i=0;i<timing.intervals.size();++i) out.ticks[i]=timing.intervals[i];
    for(unsigned i=0;i<8;++i) {
        out.render[i].width=i==0 ? dimension(render_width) : 0;
        out.render[i].height=i==0 ? dimension(render_height) : 0;
        out.render[i].calls=i==0 ? 1 : 0;
        out.viewport[i].width=i==0 ? dimension(output_width) : 0;
        out.viewport[i].height=i==0 ? dimension(output_height) : 0;
        out.viewport[i].calls=i==0 ? 1 : 0;
    }
    // Readers clear active to test our heartbeat, and set render[0].calls to
    // 0xffff to request resolutions. Publish the data before acknowledging it.
    std::atomic_thread_fence(std::memory_order_release);
    out.active=true;
}
}
