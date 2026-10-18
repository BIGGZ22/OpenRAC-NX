// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Crash reports: when the program stops on a fault (runtime/crash.cpp's signal and exception
// handlers), a fatal error (log::fatal) or a trap the game raises on purpose, a text file
// `<data>/crashes/crash-<date>_<time>.txt` says why, with the native backtrace, what the game was
// doing (the level, its game mode, the frame, where the hero was), the log's last lines and the
// session's header (common/log.h). The reason, the backtrace and the state also go to the session
// log and to stderr, so the log alone is enough.
//
// report() can run inside a signal handler: it writes with write() from buffers allocated before
// (configure()), and never allocates or takes a lock.

#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>

namespace openrac::crash {

enum class Kind {
    Signal,     // a fault: an access to memory that is not there, an illegal instruction...
    Exception,  // the same on Windows (a structured exception), or a C++ exception nobody caught
    Fatal,      // the port gave up (log::fatal)
    Trap,       // the game's own code raised a trap: a failed assertion of its own
};

// What the game was doing, sampled each frame (cheap: plain stores).
struct GameState {
    long frame = -1;
    int level = -1;  // the level loaded, or -1 (the boot program: title and menus)
    bool has_mode = false;
    std::uint32_t mode = 0;  // the game's mode word (rac1: 0x15F6A8)
    bool has_hero = false;
    float hero[3] = {0.0f, 0.0f, 0.0f};
};

// Where reports go (`folder` is made when the first is written), the program's name and the
// session's id (the time it started, as the log's header has it). Prepares what report() needs.
void configure(const std::filesystem::path& folder, const std::string& program, const std::string& session);

// An alternate stack for this thread's signal handler, so a stack that overflowed can still be
// reported (POSIX; nothing on Windows). Each thread that runs game code calls it once.
void prepare_thread();

void set_state(const GameState& state);
const GameState& state();

// Writes the report, once (a second call while one is being written, or after, does nothing and
// returns null). The path of the file written, or null when none could be (no folder yet).
const char* report(const char* reason, Kind kind = Kind::Signal);

// ---- Pieces, for the handlers and the tests ----

// "crash-2026-10-17_14-03-22.txt" for that local time (utc + offset seconds).
void file_name(long long utc_seconds, long offset_seconds, char* out, std::size_t size);
// "2026-10-17 14:03:22 +0200".
void format_time(long long utc_seconds, long offset_seconds, char* out, std::size_t size);
// The local time's offset from UTC now, in seconds.
long local_offset();

// The whole report to `fd`, as report() writes it to the file (for the tests: no file is made).
void write_report(int fd, const char* reason, Kind kind, long long utc_seconds);

// Text built in a fixed buffer without allocating (a reason in a signal handler).
class Text {
public:
    Text& str(const char* s);
    Text& dec(long long v);
    Text& hex(std::uint64_t v, int digits = 0);
    const char* c_str() const { return m_buf; }
    std::size_t size() const { return m_len; }

private:
    char m_buf[512] = {};
    std::size_t m_len = 0;
};

const char* kind_name(Kind kind);

}  // namespace openrac::crash
