// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// RenderWorker's bounded handoff and exception boundary.
#include "game/common/frontend/render_worker.h"

#include <exception>
#include <stdexcept>
#include <utility>

#include "common/crash.h"

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

}  // namespace

RenderWorker::RenderWorker() : m_environment(capture_environment()), m_thread([this] { run(); }) {}

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
