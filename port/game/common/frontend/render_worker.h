// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// One render job in flight: its owned snapshot cannot be replaced until completion.
#pragma once

#include <cfenv>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <string>

#include "common/joining_thread.h"

namespace openrac::frontend {

// A persistent worker with one slot, including the running job. The producer owns this object.
// Jobs capture their scene data by value. finish() transfers graphics ownership back to the producer.
class RenderWorker {
public:
    // Starts the worker with the producer's floating-point environment; startup errors reach the frontend's frame boundary.
    RenderWorker();
    // Finishes the last job, stops and joins before any captured graphics owners are destroyed.
    ~RenderWorker();
    RenderWorker(const RenderWorker&) = delete;
    RenderWorker& operator=(const RenderWorker&) = delete;
    RenderWorker(RenderWorker&&) = delete;
    RenderWorker& operator=(RenderWorker&&) = delete;

    // Waits for an empty slot and transfers an owned job. False after a worker failure.
    [[nodiscard]] bool submit(std::function<void()> job, std::string& error);
    // Waits for completion, returning false with the caught exception's message on failure.
    [[nodiscard]] bool finish(std::string& error);
    // Waits until completion or a pacing deadline, without consuming the completed job.
    [[nodiscard]] bool ready_until(std::chrono::steady_clock::time_point deadline);

private:
    // Executes jobs without holding the handoff lock; exceptions are returned to the producer.
    void run();
    std::mutex m_mutex;                 // Guards every handoff field below.
    std::condition_variable m_changed;  // Job arrival, completion or shutdown.
    std::function<void()> m_job;        // The queued job owns its snapshot.
    bool m_busy = false;                // Includes queued and executing work.
    bool m_stopping = false;            // Set only after the final job completes.
    std::string m_error;                // A worker failure, reported by finish/submit.
    const std::fenv_t
        m_environment;       // Producer's rounding and denormal mode, fixed before thread startup.
    JoiningThread m_thread;  // Joined before the handoff fields disappear.
};

}  // namespace openrac::frontend
