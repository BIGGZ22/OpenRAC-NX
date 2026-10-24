// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The C-boundary wrapper: a C entry point implemented in C++ runs its body through
// c_boundary::call, which stops every exception there, reports it and returns the entry point's
// failure value (CODING_CONVENTIONS.md, section 13.4). An exception that reached the C caller
// would be undefined behaviour; in practice std::terminate, with no word of what was thrown.
//
// common is the lowest layer, so the wrapper knows no reporter above it: the caller names one. The
// game side passes `openrac_game_crash` (game_host.h), which writes the crash report and ends the
// game; the host side keeps the default, report_to_log, and goes on with the failure value.
//
//     extern "C" int openrac_sound_play(int voice, int sound) {
//         return c_boundary::call("sound", 0, [&] { return engine().play(voice, sound) ? 1 : 0; });
//     }

#pragma once

#include <array>
#include <concepts>
#include <cstddef>
#include <exception>
#include <format>
#include <functional>
#include <string_view>
#include <type_traits>
#include <utility>

#include "common/log.h"

namespace openrac::c_boundary {

// The longest message a report carries, terminator included; a longer one is cut. A fixed buffer,
// so that reporting a std::bad_alloc does not need memory of its own.
constexpr std::size_t kMessageBytes = 512;

// Hears of an exception stopped at a C boundary. `message` is `<where>: <what>`, in a buffer that
// lives until the reporter returns. It must not throw: the wrapper is noexcept, so an exception
// from the reporter ends the program through std::terminate.
using Reporter = void (*)(const char* message);

// Writes `message` to the log as an error: the host side's reporter, and the default.
inline void report_to_log(const char* message) {
    log::error("{}", message);
}

// Builds `<where>: <what>` without allocating, cut to kMessageBytes, and gives it to `reporter`
// (the log's when it is null). A reporter that throws ends the program through std::terminate, as
// Reporter says; clang-tidy cannot see that rule, so its finding here is switched off.
// NOLINTNEXTLINE(bugprone-exception-escape)
inline void report(std::string_view where, std::string_view what, Reporter reporter) noexcept {
    std::array<char, kMessageBytes> message{};

    // Leave the last byte for the terminator, which the zeroed buffer already holds.
    std::format_to_n(message.data(), message.size() - 1, "{}: {}", where, what);

    // A caller that names no reporter still gets the report, in the log.
    if (reporter == nullptr) {
        reporter = report_to_log;
    }
    reporter(message.data());
}

// Reports the exception being handled as report() does: its what() for a std::exception,
// `unknown exception` for anything else thrown. Called only from inside a catch block.
inline void report_current_exception(std::string_view where, Reporter reporter) noexcept {
    try {
        throw;
    } catch (const std::exception& exception) {
        report(where, exception.what(), reporter);
    } catch (...) {
        report(where, "unknown exception", reporter);
    }
}

// Runs `body()` and returns its value. If it throws, reports the exception to `reporter` as
// `<where>: <what>` (`<where>: unknown exception` when it is not a std::exception) and returns
// `failure` instead.
//
// `where` names the entry point or its module for the report. `failure` is the value the entry
// point's contract documents for a failure (0, NULL, -1). Never throws.
template <typename Body>
    requires std::invocable<Body> && (!std::is_void_v<std::invoke_result_t<Body>>)
std::invoke_result_t<Body> call(
    std::string_view where,
    std::type_identity_t<std::invoke_result_t<Body>> failure,
    Body&& body,
    Reporter reporter = report_to_log
) noexcept {
    try {
        return std::invoke(std::forward<Body>(body));
    } catch (...) {
        report_current_exception(where, reporter);
    }
    return failure;
}

// Runs `body()`, for an entry point that returns nothing. If it throws, reports the exception as
// the overload with a failure value does, and returns. Never throws.
template <typename Body>
    requires std::invocable<Body> && std::is_void_v<std::invoke_result_t<Body>>
void call(std::string_view where, Body&& body, Reporter reporter = report_to_log) noexcept {
    try {
        std::invoke(std::forward<Body>(body));
    } catch (...) {
        report_current_exception(where, reporter);
    }
}

}  // namespace openrac::c_boundary
