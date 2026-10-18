// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The last lines of the log, kept in memory for a crash report (common/crash.h): a fixed number of
// lines of a fixed width, in storage that is part of the object, so reading it back allocates
// nothing and can be done from a signal handler. A longer line is cut at the width.

#pragma once

#include <cstddef>
#include <cstring>
#include <string_view>

namespace openrac::log {

template <std::size_t Lines, std::size_t Width>
class LineRing {
    static_assert(Lines > 0 && Width > 1);

public:
    // Keeps `line` (its first Width - 1 bytes), forgetting the oldest once full.
    void push(std::string_view line) {
        const std::size_t n = line.size() < Width - 1 ? line.size() : Width - 1;
        std::memcpy(m_text[m_next], line.data(), n);
        m_text[m_next][n] = '\0';
        m_length[m_next] = n;
        m_next = (m_next + 1) % Lines;
        if (m_count < Lines) {
            ++m_count;
        }
    }

    std::size_t size() const { return m_count; }
    static constexpr std::size_t capacity() { return Lines; }

    // Calls f(text, length) for each line kept, the oldest first.
    template <typename F>
    void each(F&& f) const {
        const std::size_t first = (m_next + Lines - m_count) % Lines;
        for (std::size_t i = 0; i < m_count; ++i) {
            const std::size_t at = (first + i) % Lines;
            f(static_cast<const char*>(m_text[at]), m_length[at]);
        }
    }

    void clear() {
        m_next = 0;
        m_count = 0;
    }

private:
    char m_text[Lines][Width]{};
    std::size_t m_length[Lines]{};
    std::size_t m_next = 0;
    std::size_t m_count = 0;
};

}  // namespace openrac::log
