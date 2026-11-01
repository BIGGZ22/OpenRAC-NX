// SPDX-License-Identifier: GPL-3.0-or-later
// Optional NX-FPS telemetry for homebrew; no injection or graphics-setting overrides.
#include "platform/switch/overlay_stats.h"
#include "platform/switch/overlay_protocol.h"
#include <switch.h>
#include <cstring>
#include <cstdio>
#include <cstdarg>
#if defined(OPENRAC_VULKAN)
#include "renderer/vulkan/vkgl.h"
#endif

namespace openrac::platform::overlay_stats {
namespace {
constexpr std::size_t kPageSize = 0x1000;
SharedMemory memory{};
std::uint8_t* page = nullptr;
volatile FpsBlock* block = nullptr;
volatile ReverseBlock* reverse = nullptr;
Timing timing;
std::uint64_t next_attempt = 0;
unsigned attempts = 0;
bool startup = false;
bool reported_measurement = false;
std::array<int,4> reported_sizes{};

void diagnostic(const char* format, ...) {
    FILE* file = std::fopen("sdmc:/switch/openrac/overlay-status.log", "a");
    if (!file) return;
    va_list args;
    va_start(args, format);
    std::vfprintf(file, format, args);
    va_end(args);
    std::fputc('\n', file);
    std::fclose(file);
}

struct SessionRoom {
    bool restore_sm = false;
    ~SessionRoom() { if (restore_sm) diagnostic("restore sm: rc=0x%X", smInitialize()); }
    void prepare() {
        u64 raw = 0;
        if (R_FAILED(svcGetInfo(&raw, InfoType_ResourceLimit, INVALID_HANDLE, 0))
            && R_FAILED(svcGetInfo(&raw, InfoType_ResourceLimit, CUR_PROCESS_HANDLE, 0))) return;
        const Handle limit = Handle(raw);
        s64 used = 0, max = 0;
        if (R_SUCCEEDED(svcGetResourceLimitCurrentValue(&used, limit, LimitableResource_Sessions))
            && R_SUCCEEDED(svcGetResourceLimitLimitValue(&max, limit, LimitableResource_Sessions)) && used >= max) {
            diagnostic("named-port sessions exhausted: %lld/%lld", (long long)used, (long long)max);
            if (envIsSyscallHinted(0x7e)) {
                const Result rc = svcSetResourceLimitLimitValue(limit, LimitableResource_Sessions, u64(max + 4));
                diagnostic("raise session limit: rc=0x%X", rc);
                if (R_SUCCEEDED(rc)) { svcCloseHandle(limit); return; }
            }
            // Safe only during explicit single-threaded initialization, before SDL is opened.
            if (startup) {
                smExit();
                restore_sm = true;
                diagnostic("temporarily released startup sm session");
            }
        }
        svcCloseHandle(limit);
    }
};

volatile FpsBlock* find_block() {
    // Reuse the common prefix only: an existing allocation may be the older 174-byte block.
    for (std::size_t at = 0; page && at + 174 <= kPageSize; at += 4) {
        std::uint32_t magic = 0;
        std::memcpy(&magic, page + at, sizeof(magic));
        if (magic == kFpsMagic) return reinterpret_cast<volatile FpsBlock*>(page + at);
    }
    return nullptr;
}

volatile ReverseBlock* find_reverse() {
    for (std::size_t at = 0; page && at + sizeof(ReverseBlock) <= kPageSize; at += 4) {
        std::uint32_t magic = 0;
        std::memcpy(&magic, page + at, sizeof(magic));
        if (magic == kReverseMagic) return reinterpret_cast<volatile ReverseBlock*>(page + at);
    }
    return nullptr;
}

void attach(std::uint64_t tick, std::uint64_t frequency) {
    if (block && block->magic != kFpsMagic) block = nullptr;
    if (reverse && reverse->magic != kReverseMagic) reverse = nullptr;
    if (block && reverse) return;
    if (tick < next_attempt) return;
    next_attempt = tick + frequency * 5;
    const bool report = attempts++ < 4 || attempts % 12 == 0;
    SessionRoom room;
    room.prepare();
    Handle port = INVALID_HANDLE;
    const Result connect = svcConnectToNamedPort(&port, "SaltySD");
    if (R_FAILED(connect)) {
        if (report) diagnostic("SaltySD connection: rc=0x%X", connect);
        return;
    }
    Service service{};
    service.session = port;
    if (!page) {
        Handle handle = INVALID_HANDLE;
        const Result rc = serviceDispatch(&service, 7, .in_send_pid = true,
            .out_handle_attrs = {SfOutHandleAttr_HipcCopy}, .out_handles = &handle);
        if (report) diagnostic("shared memory handle: rc=0x%X", rc);
        if (R_SUCCEEDED(rc)) {
            shmemLoadRemote(&memory, handle, kPageSize, Perm_Rw);
            const Result mapped = shmemMap(&memory);
            if (report) diagnostic("map shared memory: rc=0x%X", mapped);
            if (R_SUCCEEDED(mapped)) page = static_cast<std::uint8_t*>(shmemGetAddr(&memory));
            else shmemClose(&memory);
        }
    }
    block = find_block();
    if (page && !block) {
        const std::uint64_t bytes = sizeof(FpsBlock);
        std::uint64_t offset = 0;
        const Result reserved = serviceDispatchInOut(&service, 6, bytes, offset, .in_send_pid = true);
        if (report) diagnostic("reserve FPS block: rc=0x%X offset=%llu", reserved, (unsigned long long)offset);
        if (R_SUCCEEDED(reserved)
            && offset % 4 == 0 && offset <= kPageSize - sizeof(FpsBlock)) {
            auto* fresh = reinterpret_cast<FpsBlock*>(page + offset);
            std::memset(fresh, 0, sizeof(*fresh));
            fresh->api = 2;  // EGL/OpenGL
            fresh->expected_set_buffers = -1;
            fresh->magic = kFpsMagic;
            block = fresh;
            std::fprintf(stderr, "[OpenRAC-NX] SaltyNX shared-memory telemetry connected.\n");
        }
    }
    if (!reverse) {
        reverse = find_reverse();
        if (!reverse && page) {
            const std::uint64_t bytes = sizeof(ReverseBlock);
            std::uint64_t offset = 0;
            if (R_SUCCEEDED(serviceDispatchInOut(&service, 6, bytes, offset, .in_send_pid = true))
                && offset % 4 == 0 && offset <= kPageSize - sizeof(ReverseBlock)) {
                auto* fresh = reinterpret_cast<ReverseBlock*>(page + offset);
                std::memset(fresh, 0, sizeof(*fresh));
                fresh->system_control = true;
                fresh->magic = kReverseMagic;
                reverse = fresh;
            }
        }
        if (reverse) {
            // Do not inherit another game's forced profile from the shared page.
            reverse->system_control = true;
            reverse->ddr_used = false;  // We expose profile switching, not DDR overrides.
        }
    }
    const std::uint64_t zero = 0;
    serviceDispatchIn(&service, 0, zero, .in_send_pid = true);
    svcCloseHandle(port);
    if (report) diagnostic("attachment: FPS=%s ReverseNX=%s", block ? "ready" : "missing", reverse ? "ready" : "missing");
}

}

void initialize() {
    diagnostic("--- OpenRAC-NX overlay initialization ---");
    u64 program_id = 0;
    svcGetInfo(&program_id, InfoType_ProgramId, CUR_PROCESS_HANDLE, 0);
    diagnostic("program id=%016llX", (unsigned long long)program_id);
    diagnostic("applet type=%d (Status Monitor requires an application process)", int(appletGetAppletType()));
#if defined(OPENRAC_DKGL)
    diagnostic("renderer=deko3d; direct FPS/resolution publisher, compatibility API=2; no NVN counters");
#endif
    startup = true;
    for (unsigned retry = 0; retry < 4 && (!block || !reverse); ++retry) {
        next_attempt = 0;
        attach(armGetSystemTick(), armGetSystemTickFreq());
        if (!block || !reverse) svcSleepThread(2'000'000);
    }
    startup = false;
}

void presented(int render_width, int render_height, int output_width, int output_height) {
    const auto tick = armGetSystemTick();
    const auto frequency = armGetSystemTickFreq();
    timing.present(tick, frequency);
    attach(tick, frequency);
    if (!block) return;
    // Only our reported fields are written. Overlay control fields remain untouched.
#if defined(OPENRAC_VULKAN)
    const std::uint8_t api = vkgl::active() ? 3 : 2;
#else
    const std::uint8_t api = 2; // Also the compatibility channel for native deko3d.
#endif
    publish(*block,timing,api,render_width,render_height,output_width,output_height);
    const std::array<int,4> sizes{render_width,render_height,output_width,output_height};
    if(sizes!=reported_sizes) {
        diagnostic("published render=%dx%d output=%dx%d API=%u frame=%llu",
                   render_width,render_height,output_width,output_height,unsigned(api),
                   (unsigned long long)timing.frames);
        reported_sizes=sizes;
    }
    if(!reported_measurement && timing.fps>0) {
        diagnostic("measured FPS=%u rolling=%.2f heartbeat=active",unsigned(timing.fps),double(timing.average));
        reported_measurement=true;
    }
}

bool docked() {
    attach(armGetSystemTick(), armGetSystemTickFreq());
    const bool physical = appletGetOperationMode() == AppletOperationMode_Console;
    if (!reverse || reverse->magic != kReverseMagic) return physical;
    reverse->active = true;
    if (reverse->system_control) reverse->docked = physical;
    return reverse->docked;
}

void shutdown() {
    if (block && block->magic == kFpsMagic) block->active = false;
    block = nullptr;
    if (reverse && reverse->magic == kReverseMagic) reverse->active = false;
    reverse = nullptr;
    if (page) shmemClose(&memory);
    page = nullptr;
    timing = {};
    next_attempt = 0;
    reported_sizes={};
    reported_measurement=false;
}
}
