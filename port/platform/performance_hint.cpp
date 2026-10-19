// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// performance_hint.h. On Android the hint session's functions are looked up in libandroid.so at
// run time, since the port's Android build starts at API 29 and they came with API 33.

#include "platform/performance_hint.h"

#if defined(__ANDROID__)

#include <chrono>
#include <cstdint>
#include <dlfcn.h>
#include <unistd.h>

#include "common/log.h"

namespace openrac::platform {
namespace {

struct Manager;
struct Session;

using GetManager = Manager* (*)();
using CreateSession = Session* (*)(Manager*, const std::int32_t*, std::size_t, std::int64_t);
using UpdateTarget = int (*)(Session*, std::int64_t);
using ReportActual = int (*)(Session*, std::int64_t);

struct Hints {
    bool tried = false;
    Session* session = nullptr;
    UpdateTarget update_target = nullptr;
    ReportActual report_actual = nullptr;
    std::int64_t target_ns = 0;
    std::chrono::steady_clock::time_point resumed{};
};

Hints g_hints;

std::int64_t nanoseconds(double seconds) {
    return static_cast<std::int64_t>(seconds * 1e9);
}

void start(std::int64_t target_ns) {
    g_hints.tried = true;
    void* android = dlopen("libandroid.so", RTLD_NOW | RTLD_LOCAL);
    if (android == nullptr) {
        return;
    }
    const auto get_manager = reinterpret_cast<GetManager>(dlsym(android, "APerformanceHint_getManager"));
    const auto create = reinterpret_cast<CreateSession>(dlsym(android, "APerformanceHint_createSession"));
    g_hints.update_target = reinterpret_cast<UpdateTarget>(dlsym(android, "APerformanceHint_updateTargetWorkDuration"));
    g_hints.report_actual = reinterpret_cast<ReportActual>(dlsym(android, "APerformanceHint_reportActualWorkDuration"));
    if (get_manager == nullptr || create == nullptr || g_hints.update_target == nullptr
        || g_hints.report_actual == nullptr) {
        log::info("performance hints: not on this Android (API 33 and later)");
        return;
    }
    Manager* manager = get_manager();
    const std::int32_t thread = static_cast<std::int32_t>(gettid());
    g_hints.session = manager != nullptr ? create(manager, &thread, 1, target_ns) : nullptr;
    if (g_hints.session == nullptr) {
        log::info("performance hints: the system gave no session");
        return;
    }
    g_hints.target_ns = target_ns;
    log::info("performance hints: a session for the game's thread, {:.2f} ms a frame",
              static_cast<double>(target_ns) / 1e6);
}

}  // namespace

void set_frame_target(double seconds) {
    const std::int64_t target = nanoseconds(seconds);
    if (target <= 0) {
        return;
    }
    if (!g_hints.tried) {
        start(target);
        return;
    }
    if (g_hints.session != nullptr && target != g_hints.target_ns) {
        g_hints.update_target(g_hints.session, target);
        g_hints.target_ns = target;
    }
}

void work_resumed() {
    if (g_hints.session != nullptr) {
        g_hints.resumed = std::chrono::steady_clock::now();
    }
}

void frame_work_done() {
    if (g_hints.session == nullptr || g_hints.resumed.time_since_epoch().count() == 0) {
        return;
    }
    const auto work = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now()
                                                                           - g_hints.resumed);
    if (work.count() > 0) {
        g_hints.report_actual(g_hints.session, work.count());
    }
}

}  // namespace openrac::platform

#else

namespace openrac::platform {

void set_frame_target(double) {}

void work_resumed() {}

void frame_work_done() {}

}  // namespace openrac::platform

#endif
