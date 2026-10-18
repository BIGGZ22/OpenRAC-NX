// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The port's log: one line per message, to stderr and, once a session is open, to the session's
// file too. Messages are formatted with std::format.
//
// A session is one run of a game: its log is `<folder>/<name>.log`, begun afresh each run, with
// the runs before it moved along to `<name>.1.log` (the last one) ... `<name>.<keep>.log` (the
// oldest kept), so a log a player sends is one run and small. It starts with a header (the
// versions, the system, the command line) that a crash report (common/crash.h) repeats. The last
// lines are also kept in memory, for the same report. In the file and in memory the player's home
// folder reads `~`.

#pragma once

#include <cstddef>
#include <cstdio>
#include <filesystem>
#include <format>
#include <string>
#include <string_view>

namespace openrac::log {

enum class Level {
    Debug,    // everything: each frame's DMA chains, texture uploads (OPENRAC_LOG=debug)
    Verbose,  // more about what the game does: level loads, pad changes, callers (OPENRAC_LOG=verbose)
    Info,
    Warn,
    Error
};

// Messages below this level are dropped. Info by default; OPENRAC_LOG=verbose or debug in the
// environment lowers it.
void set_level(Level level);
Level level();
inline bool verbose_on() {
    return level() <= Level::Verbose;
}

// Also write every message to this file (appended). An empty path stops it.
void set_file(const char* path);

// Moves the earlier sessions' logs along (`<name>.log` to `<name>.1.log`, ..., the one past
// `<name>.<keep>.log` removed) and opens `<dir>/<name>.log` new for this one. False when it cannot
// be written (the messages still go to stderr).
bool open_session(const std::filesystem::path& dir, const std::string& name, int keep = 5);

// The renames open_session makes, on their own (for tests).
void rotate(const std::filesystem::path& dir, const std::string& name, int keep);

// The file of session `n` (0: this one): `<dir>/<name>.log`, `<dir>/<name>.<n>.log`.
std::filesystem::path session_file(const std::filesystem::path& dir, const std::string& name, int n);

// A line of the session's header: written to the log as it is (unless `to_file` is false: a fact
// learnt later and logged in its own words, the graphics card), and kept for crash reports.
void header(std::string_view line, bool to_file = true);
// The header so far, a line each ('\n').
std::string header_text();

// After this many bytes in a session's file, only warnings and errors are written to it (the rest
// goes on to stderr): a debugging switch left on cannot fill the disc.
inline constexpr std::size_t kSessionFileLimit = 32u * 1024 * 1024;

// `text` with the player's home folder ($HOME, %USERPROFILE%) as `~`.
std::string tidy(std::string_view text);
// The home folder tidy() replaces (empty: none known).
const std::string& home();

void write(Level level, std::string_view message);

template <typename... Args>
void debug(std::format_string<Args...> fmt, Args&&... args) {
    if (level() <= Level::Debug) {
        write(Level::Debug, std::format(fmt, std::forward<Args>(args)...));
    }
}

template <typename... Args>
void verbose(std::format_string<Args...> fmt, Args&&... args) {
    if (level() <= Level::Verbose) {
        write(Level::Verbose, std::format(fmt, std::forward<Args>(args)...));
    }
}

template <typename... Args>
void info(std::format_string<Args...> fmt, Args&&... args) {
    if (level() <= Level::Info) {
        write(Level::Info, std::format(fmt, std::forward<Args>(args)...));
    }
}

template <typename... Args>
void warn(std::format_string<Args...> fmt, Args&&... args) {
    if (level() <= Level::Warn) {
        write(Level::Warn, std::format(fmt, std::forward<Args>(args)...));
    }
}

template <typename... Args>
void error(std::format_string<Args...> fmt, Args&&... args) {
    write(Level::Error, std::format(fmt, std::forward<Args>(args)...));
}

// Logs the message as an error, writes a crash report (common/crash.h) once the session has a
// folder for them, and ends the program.
[[noreturn]] void fatal(std::string_view message);

template <typename... Args>
[[noreturn]] void fatalf(std::format_string<Args...> fmt, Args&&... args) {
    fatal(std::format(fmt, std::forward<Args>(args)...));
}

// ---- For the crash report: no allocation, no lock (common/crash.cpp) ----

// The session file's descriptor, or -1.
int file_descriptor();
// The header as it was last written (a buffer that lives as long as the program).
std::string_view header_for_crash();
// Calls emit(text, length, context) for each of the last lines, the oldest first.
void recent_lines(void (*emit)(const char* text, std::size_t length, void* context), void* context);
// How many lines recent_lines keeps.
std::size_t recent_capacity();

}  // namespace openrac::log
