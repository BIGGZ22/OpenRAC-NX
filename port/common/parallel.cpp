// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

#include "common/parallel.h"

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <exception>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "common/joining_thread.h"

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
};

class Pool {
public:
    Pool() {
        const unsigned threads = std::thread::hardware_concurrency();
        const std::size_t count = threads > 2 ? std::min<std::size_t>(threads - 2, 3) : 0;
        for (std::size_t i = 0; i < count; ++i) {
            m_threads.emplace_back([this] { work(); });
        }
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
        // Another thread's call (the render thread's beside the game's) or one from inside a body
        // has the threads: this one runs on its caller alone.
        std::unique_lock call(m_call, std::try_to_lock);
        const bool share = call.owns_lock() && !m_threads.empty() && count > 1;
        if (share) {
            {
                const std::lock_guard lock(m_mutex);
                m_job = job;
                ++m_generation;
            }
            m_wake.notify_all();
        }
        drain(*job);
        if (share) {
            std::unique_lock lock(m_mutex);
            m_finished.wait(lock, [&] { return job->done.load() == count; });
            m_job.reset();
        }
        if (job->error) {
            std::rethrow_exception(job->error);
        }
    }

private:
    void drain(Job& job) {
        for (;;) {
            const std::size_t i = job.next.fetch_add(1);
            if (i >= job.count) {
                return;
            }
            try {
                (*job.body)(i);
            } catch (...) {
                const std::lock_guard lock(job.error_mutex);
                if (!job.error) {
                    job.error = std::current_exception();
                }
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
                drain(*job);
            }
        }
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
