// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// RenderWorker's bounded handoff and exception boundary.
#include "game/common/frontend/render_worker.h"

#include <exception>
#include <stdexcept>
#include <utility>

#include "common/crash.h"
#include "common/log.h"
#ifdef __SWITCH__
#include <switch.h>
#endif

namespace openrac::frontend {

namespace {

// Captures the producer's arithmetic mode before the worker can run any renderer calculations.
std::fenv_t capture_environment() {
    std::fenv_t environment{};

    // A default worker mode can change culling, interpolation and uniform values on Windows.
    if (std::fegetenv(&environment) != 0) {
        throw std::runtime_error("cannot capture render floating-point environment");
    }

    return environment;
}

// The core a worker with `placement` goes to, chosen on the creating thread: -1 to stay.
int choose_core(const RenderWorker::Placement& placement) {
    if (!placement.move) {
        return -1;
    }
#ifdef __SWITCH__
    // The cores this process may run on, and the creating thread's own.
    u64 allowed = 0;
    if (R_FAILED(svcGetInfo(&allowed, InfoType_CoreMask, CUR_PROCESS_HANDLE, 0))) {
        log::warn("render worker: the process's cores are unknown; the worker stays where it starts");
        return -1;
    }
    const int creator = static_cast<int>(svcGetCurrentProcessorNumber());
    int core = placement.core;
    if (core < 0) {
        for (int c = 0; c < 32; ++c) {
            if (((allowed >> c) & 1) != 0 && c != creator) {
                core = c;
                break;
            }
        }
    }
    if (core < 0 || core >= 32 || ((allowed >> core) & 1) == 0) {
        log::warn(
            "render worker: no core to move to (asked {}, allowed mask {:#x}, game thread on {}); "
            "it stays where it starts",
            placement.core, allowed, creator
        );
        return -1;
    }
    log::info(
        "render worker: moving to core {} (allowed mask {:#x}, game thread on core {})",
        core, allowed, creator
    );
    return core;
#else
    return -1;
#endif
}

// Moves the calling (worker) thread to `core`; a failure leaves it where it is.
void move_to_core(int core) {
    if (core < 0) {
        return;
    }
#ifdef __SWITCH__
    const Result result = svcSetThreadCoreMask(CUR_THREAD_HANDLE, core, static_cast<u32>(1u << core));
    if (R_FAILED(result)) {
        log::warn("render worker: cannot move to core {} (result {:#x})", core, result);
    } else {
        log::info("render worker: running on core {}", svcGetCurrentProcessorNumber());
    }
#endif
}

}  // namespace

RenderWorker::RenderWorker() : RenderWorker(Placement{}) {}

RenderWorker::RenderWorker(Placement placement)
    : m_environment(capture_environment()),
      m_core(choose_core(placement)),
      m_thread([this] { run(); }) {}

RenderWorker::~RenderWorker() {
    {
        std::unique_lock lock(m_mutex);
        m_changed.wait(lock, [this] { return !m_busy; });
        m_stopping = true;
    }
    m_changed.notify_one();
    m_thread.join();
}

bool RenderWorker::submit(std::function<void()> job, std::string& error) {
    // An empty function cannot wake a consumer waiting for an owned job.
    if (!job) {
        error = "empty render job";
        return false;
    }

    std::unique_lock lock(m_mutex);
    m_changed.wait(lock, [this] { return !m_busy; });

    // A failed renderer cannot accept another snapshot.
    if (!m_error.empty()) {
        error = m_error;
        return false;
    }

    m_job = std::move(job);
    m_busy = true;
    lock.unlock();
    m_changed.notify_all();
    return true;
}

bool RenderWorker::finish(std::string& error) {
    std::unique_lock lock(m_mutex);
    m_changed.wait(lock, [this] { return !m_busy; });
    error = m_error;
    return error.empty();
}

bool RenderWorker::ready_until(std::chrono::steady_clock::time_point deadline) {
    std::unique_lock lock(m_mutex);
    return m_changed.wait_until(lock, deadline, [this] { return !m_busy; });
}

void RenderWorker::run() {
    // A render fault needs its own alternate signal stack on POSIX hosts.
    crash::prepare_thread();
    // Off the game thread's core first, when asked (Placement).
    move_to_core(m_core);

    // Destruction ends the loop after the final accepted snapshot has completed.
    for (;;) {
        std::unique_lock lock(m_mutex);
        m_changed.wait(lock, [this] { return m_stopping || static_cast<bool>(m_job); });

        // Shutdown follows completion, so no snapshot is discarded.
        if (m_stopping) {
            return;
        }

        auto job = std::exchange(m_job, {});
        lock.unlock();
        std::string error;

        // Render failures return to the producer instead of escaping the thread entry point.
        try {
            // Every snapshot uses the same rounding and denormal handling as synchronous recording.
            if (std::fesetenv(&m_environment) != 0) {
                throw std::runtime_error("cannot restore render floating-point environment");
            }

            job();
        } catch (const std::exception& failure) {
            error = failure.what();
        } catch (...) {
            error = "unknown render-thread exception";
        }

        // Release captured owners before publishing completion.
        job = {};
        lock.lock();
        m_error = std::move(error);
        m_busy = false;
        lock.unlock();
        m_changed.notify_all();
    }
}

}  // namespace openrac::frontend
