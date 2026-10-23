// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// A thread that its owner joins when it goes: what std::jthread gives, without the stop token. The
// launcher's Linux builds use a C++ library that keeps std::jthread behind an experimental switch
// (libc++ before 18), so the port uses this where it would use std::jthread
// (CODING_CONVENTIONS.md, section 11.6).

#pragma once

#include <thread>
#include <type_traits>
#include <utility>

namespace openrac {

// A std::thread joined by its destructor, as a std::jthread is; no stop token is passed.
//
// Move-only. Destroying or assigning over one that runs a thread waits for that thread's function
// to return, so whatever the function reads must outlive the object. The thread that created it
// joins it; it is not shared between threads.
class JoiningThread {
public:
    // Runs no thread.
    JoiningThread() noexcept = default;

    // Starts a thread running `function(arguments...)`, as std::thread's constructor does.
    template <typename Function, typename... Arguments>
        requires(!std::is_same_v<std::remove_cvref_t<Function>, JoiningThread>)
    explicit JoiningThread(Function&& function, Arguments&&... arguments)
        : m_thread(std::forward<Function>(function), std::forward<Arguments>(arguments)...) {}

    // Waits for the thread's function to return.
    ~JoiningThread() { join(); }

    JoiningThread(const JoiningThread&) = delete;
    JoiningThread& operator=(const JoiningThread&) = delete;
    JoiningThread(JoiningThread&&) noexcept = default;

    // Waits for this object's own thread, then takes `other`'s, which is left running none.
    JoiningThread& operator=(JoiningThread&& other) noexcept {
        // Assigning an object to itself keeps its thread running.
        if (this != &other) {
            join();
            m_thread = std::move(other.m_thread);
        }
        return *this;
    }

    // Waits for the thread's function to return; returns at once when no thread runs.
    void join() {
        // A default-made, moved-from or already joined object has no thread to wait for.
        if (m_thread.joinable()) {
            m_thread.join();
        }
    }

private:
    std::thread m_thread;  // The thread, or none.
};

}  // namespace openrac
