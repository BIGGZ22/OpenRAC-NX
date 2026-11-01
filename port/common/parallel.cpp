// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

#include "common/parallel.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <format>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#include "common/joining_thread.h"
#include "common/log.h"
#ifdef __SWITCH__
#include <switch.h>
#endif

namespace openrac::parallel {

namespace {

// One call's work. A thread that finishes its last piece after the call returned still holds it,
// so a later call never hands it this call's body.
struct Job {
    const std::function<void(std::size_t)>* body = nullptr;
    std::size_t count = 0;
    std::atomic<std::size_t> next{0};
    std::atomic<std::size_t> done{0};
    std::mutex error_mutex;
    std::exception_ptr error;
    // OPENRAC_POOL_STATS: what the helpers did with this call (pool_stats_every), written by them.
    std::chrono::steady_clock::time_point published{};
    std::atomic<std::size_t> helper_items{0};
    std::atomic<std::int64_t> helper_busy_ns{0};
    std::atomic<std::int64_t> helper_start_ns{-1};  // publish to a helper's first piece
};

// OPENRAC_POOL_STATS=N (diagnostic, 30..3600 calls): every N calls that had the pool's threads (or,
// with none, every N calls), how the pieces were shared and where the time went: pieces a call,
// pieces the caller ran, pieces the helpers ran, the call's wall time, the caller's time a piece
// and the helpers', how long a helper took to start, and how long the caller waited for the
// last piece after running out of its own. A few clock reads a call; nothing when unset.
int pool_stats_every() {
    static const int n = [] {
        const char* v = std::getenv("OPENRAC_POOL_STATS");
        const int asked = v != nullptr ? std::atoi(v) : 0;
        return asked > 0 ? std::clamp(asked, 30, 3600) : 0;
    }();
    return n;
}

double ms_between(std::chrono::steady_clock::time_point a, std::chrono::steady_clock::time_point b) {
    return std::chrono::duration<double, std::milli>(b - a).count();
}

// How many helper threads the pool keeps, and the core of each (-1: where the system starts it).
//  - OPENRAC_POOL_CORES=N (Switch, experiment, off by default; N 1..3, "1" the usual choice): N
//    helpers, on the cores the process may use other than the creating thread's, highest first.
//    A new Switch thread starts on its creator's core and stays there, so without this the pool's
//    threads would only take turns with the game's. The render worker (OPENRAC_DK_RENDER_THREAD)
//    takes the lowest such core, so one helper takes the core it leaves free (mask 0x7: game 0 ->
//    worker 1, helper 2; game 1 -> worker 0, helper 2); a second shares the worker's core.
//  - OPENRAC_POOL_THREADS=N (0..3, any platform): N helpers wherever the system starts them, in
//    place of the hardware count; for tests on small hosts and for comparing.
//  - Neither: two fewer than the hardware threads reported, at most three (session 44, Switch:
//    0 reported, so 0 helpers).
struct PoolPlan {
    std::size_t threads = 0;
    std::vector<int> cores;  // one per thread, or empty
};

// A count of 0..3 from a variable's value, or -1 when it is unset or empty.
int env_count(const char* v) {
    if (v == nullptr || *v == '\0') {
        return -1;
    }
    return std::clamp(std::atoi(v), 0, 3);
}

PoolPlan pool_plan(unsigned hardware) {
    PoolPlan plan;
    plan.threads = hardware > 2 ? std::min<std::size_t>(hardware - 2, 3) : 0;
    if (const int asked = env_count(std::getenv("OPENRAC_POOL_THREADS")); asked >= 0) {
        plan.threads = static_cast<std::size_t>(asked);
        log::info("parallel: OPENRAC_POOL_THREADS={}: {} helper thread(s)", asked, asked);
    }
#ifdef __SWITCH__
    const int helpers = env_count(std::getenv("OPENRAC_POOL_CORES"));
    if (helpers <= 0) {
        return plan;
    }
    u64 allowed = 0;
    if (R_FAILED(svcGetInfo(&allowed, InfoType_CoreMask, CUR_PROCESS_HANDLE, 0))) {
        log::warn("parallel: the process's cores are unknown; OPENRAC_POOL_CORES ignored");
        return plan;
    }
    const int creator = static_cast<int>(svcGetCurrentProcessorNumber());
    for (int c = 31; c >= 0 && plan.cores.size() < static_cast<std::size_t>(helpers); --c) {
        if (((allowed >> c) & 1) != 0 && c != creator) {
            plan.cores.push_back(c);
        }
    }
    plan.threads = plan.cores.size();
    std::string list;
    for (const int c : plan.cores) {
        list += std::format("{}{}", list.empty() ? "" : ",", c);
    }
    log::info("parallel: EXPERIMENT OPENRAC_POOL_CORES={}: {} helper(s) on core(s) {} (mask {:#x}, "
              "creating thread on core {})", helpers, plan.cores.size(), list, allowed, creator);
#endif
    return plan;
}

// Moves the calling pool thread to `core`, or leaves it (-1).
void move_to_core([[maybe_unused]] int core) {
#ifdef __SWITCH__
    if (core >= 0 && core < 32) {
        const Result result =
            svcSetThreadCoreMask(CUR_THREAD_HANDLE, core, static_cast<u32>(1u << core));
        if (R_FAILED(result)) {
            log::warn("parallel: a pool thread cannot move to core {} (result {:#x})", core, result);
        }
    }
#endif
}

class Pool {
public:
    Pool() {
        const unsigned threads = std::thread::hardware_concurrency();
        const PoolPlan plan = pool_plan(threads);
        const std::size_t count = plan.threads;
        for (std::size_t i = 0; i < count; ++i) {
            const int core = i < plan.cores.size() ? plan.cores[i] : -1;
            m_threads.emplace_back([this, core] {
                move_to_core(core);
                work();
            });
        }
        // Once, for reading the logs: how many threads share the poses with the caller.
        log::info("parallel: {} pool thread(s) beside the caller (hardware threads reported: {})",
                  count, threads);
    }

    ~Pool() {
        {
            const std::lock_guard lock(m_mutex);
            m_stopping = true;
        }
        m_wake.notify_all();
        m_threads.clear();  // joins them
    }

    Pool(const Pool&) = delete;
    Pool& operator=(const Pool&) = delete;

    std::size_t size() const { return m_threads.size(); }

    void run(std::size_t count, const std::function<void(std::size_t)>& body) {
        auto job = std::make_shared<Job>();
        job->body = &body;
        job->count = count;
        const bool stats = pool_stats_every() != 0;
        // Another thread's call (the render thread's beside the game's) or one from inside a body
        // has the threads: this one runs on its caller alone.
        std::unique_lock call(m_call, std::try_to_lock);
        const bool share = call.owns_lock() && !m_threads.empty() && count > 1;
        const auto started = stats ? std::chrono::steady_clock::now() : std::chrono::steady_clock::time_point{};
        if (share) {
            {
                const std::lock_guard lock(m_mutex);
                job->published = started;
                m_job = job;
                ++m_generation;
            }
            m_wake.notify_all();
        }
        const std::size_t own = drain(*job, false);
        const auto drained = stats ? std::chrono::steady_clock::now() : std::chrono::steady_clock::time_point{};
        if (share) {
            std::unique_lock lock(m_mutex);
            m_finished.wait(lock, [&] { return job->done.load() == count; });
            m_job.reset();
        }
        // Counted by the call that had the threads (or, with none, by any caller of the game's
        // usual one at a time; another thread's concurrent call is left out).
        if (stats && (share || (m_threads.empty() && call.owns_lock()))) {
            note_stats(*job, own, started, drained, std::chrono::steady_clock::now());
        }
        if (job->error) {
            std::rethrow_exception(job->error);
        }
    }

private:
    // Runs pieces of `job` until none is left; returns how many this thread ran. `helper`: a
    // pool thread, whose share is recorded for OPENRAC_POOL_STATS.
    std::size_t drain(Job& job, bool helper) {
        const bool stats = helper && pool_stats_every() != 0;
        std::size_t ran = 0;
        for (;;) {
            const std::size_t i = job.next.fetch_add(1);
            if (i >= job.count) {
                return ran;
            }
            ++ran;
            // A helper's share, recorded before its piece counts as done, so that the caller,
            // which reads it after the last piece is done, sees all of it.
            const auto began = stats ? std::chrono::steady_clock::now() : std::chrono::steady_clock::time_point{};
            if (stats && ran == 1) {
                std::int64_t none = -1;
                job.helper_start_ns.compare_exchange_strong(
                    none, std::chrono::duration_cast<std::chrono::nanoseconds>(began - job.published).count());
            }
            try {
                (*job.body)(i);
            } catch (...) {
                const std::lock_guard lock(job.error_mutex);
                if (!job.error) {
                    job.error = std::current_exception();
                }
            }
            if (stats) {
                job.helper_items.fetch_add(1);
                job.helper_busy_ns.fetch_add(std::chrono::duration_cast<std::chrono::nanoseconds>(
                                                 std::chrono::steady_clock::now() - began)
                                                 .count());
            }
            if (job.done.fetch_add(1) + 1 == job.count) {
                const std::lock_guard lock(m_mutex);
                m_finished.notify_all();
            }
        }
    }

    void work() {
        std::uint64_t seen = 0;
        for (;;) {
            std::shared_ptr<Job> job;
            {
                std::unique_lock lock(m_mutex);
                m_wake.wait(lock, [&] { return m_stopping || m_generation != seen; });
                if (m_stopping) {
                    return;
                }
                seen = m_generation;
                job = m_job;
            }
            if (job) {
                drain(*job, true);
            }
        }
    }

    // OPENRAC_POOL_STATS sums since the last report (the caller that has the threads only).
    struct Stats {
        std::size_t calls = 0, items = 0, caller_items = 0, helper_items = 0, helper_idle_calls = 0;
        double wall_ms = 0, caller_ms = 0, helper_busy_ms = 0, wait_ms = 0, start_ms = 0;
        std::size_t starts = 0;
    } m_stats;

    void note_stats(Job& job, std::size_t own, std::chrono::steady_clock::time_point started,
                    std::chrono::steady_clock::time_point drained, std::chrono::steady_clock::time_point ended) {
        Stats& s = m_stats;
        ++s.calls;
        s.items += job.count;
        s.caller_items += own;
        const std::size_t helped = job.helper_items.load();
        s.helper_items += helped;
        s.helper_idle_calls += helped == 0 ? 1 : 0;
        s.wall_ms += ms_between(started, ended);
        s.caller_ms += ms_between(started, drained);
        s.wait_ms += ms_between(drained, ended);
        s.helper_busy_ms += static_cast<double>(job.helper_busy_ns.load()) / 1e6;
        if (const std::int64_t start = job.helper_start_ns.load(); start >= 0) {
            s.start_ms += static_cast<double>(start) / 1e6;
            ++s.starts;
        }
        if (s.calls < static_cast<std::size_t>(pool_stats_every())) {
            return;
        }
        const double n = static_cast<double>(s.calls);
        log::info(
            "parallel stats over {} calls ({} helper(s)): {:.1f} pieces a call, caller ran {:.1f}, "
            "helpers {:.1f} (none in {} calls); a call {:.3f} ms, of which the caller's own pieces "
            "{:.3f} ms ({:.1f} us a piece) and waiting for the helpers' last {:.3f} ms; helpers busy "
            "{:.3f} ms a call ({:.1f} us a piece), first piece {:.3f} ms after the call began",
            s.calls, m_threads.size(), static_cast<double>(s.items) / n,
            static_cast<double>(s.caller_items) / n, static_cast<double>(s.helper_items) / n,
            s.helper_idle_calls, s.wall_ms / n, s.caller_ms / n,
            s.caller_items != 0 ? s.caller_ms * 1000.0 / static_cast<double>(s.caller_items) : 0.0,
            s.wait_ms / n, s.helper_busy_ms / n,
            s.helper_items != 0 ? s.helper_busy_ms * 1000.0 / static_cast<double>(s.helper_items) : 0.0,
            s.starts != 0 ? s.start_ms / static_cast<double>(s.starts) : 0.0
        );
        s = {};
    }

    std::mutex m_call;  // held by the call that has the threads
    std::mutex m_mutex;
    std::condition_variable m_wake;      // a job or the end
    std::condition_variable m_finished;  // a job's last piece done
    std::shared_ptr<Job> m_job;
    std::uint64_t m_generation = 0;
    bool m_stopping = false;
    std::vector<JoiningThread> m_threads;  // last: joined first, while the rest still exists
};

Pool& pool() {
    static Pool instance;
    return instance;
}

}  // namespace

void for_each(std::size_t count, const std::function<void(std::size_t)>& body) {
    pool().run(count, body);
}

std::size_t workers() {
    return pool().size();
}

}  // namespace openrac::parallel
