// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

#include "log.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <string>

#include "common/crash.h"
#include "common/line_ring.h"

#if defined(__ANDROID__)
#include <android/log.h>
#endif

namespace openrac::log {
namespace {

namespace fs = std::filesystem;

Level initial_level() {
    const char* env = std::getenv("OPENRAC_LOG");
    if (env != nullptr && std::strcmp(env, "debug") == 0) {
        return Level::Debug;
    }
    if (env != nullptr && std::strcmp(env, "verbose") == 0) {
        return Level::Verbose;
    }
    return Level::Info;
}

std::string find_home() {
#if defined(_WIN32)
    const char* home = std::getenv("USERPROFILE");
#else
    const char* home = std::getenv("HOME");
#endif
    std::string h = home != nullptr ? home : "";
    while (h.size() > 1 && (h.back() == '/' || h.back() == '\\')) {
        h.pop_back();
    }
    // A home of "/" (a service account) would turn every path into ~: none then.
    return h.size() > 1 ? h : std::string();
}

std::mutex g_lock;
Level g_level = initial_level();
std::FILE* g_file = nullptr;
int g_fd = -1;
std::size_t g_written = 0;
bool g_limit_told = false;

// The last lines, for a crash report (200 lines of up to 300 bytes: 60 KB, kept from the start).
LineRing<200, 300> g_recent;

// The header, kept in a buffer of its own so the crash handler reads it without allocating.
std::string g_header;
char g_header_copy[8192];
std::size_t g_header_length = 0;

const char* prefix(Level level) {
    switch (level) {
        case Level::Debug:
            return "debug";
        case Level::Verbose:
            return "verbose";
        case Level::Info:
            return "info";
        case Level::Warn:
            return "warning";
        case Level::Error:
            return "error";
    }
    return "?";
}

void close_file() {
    if (g_file != nullptr) {
        std::fclose(g_file);
        g_file = nullptr;
        g_fd = -1;
    }
}

void open_file(const char* path, const char* mode) {
    close_file();
    g_written = 0;
    g_limit_told = false;
    if (path != nullptr && path[0] != '\0') {
        g_file = std::fopen(path, mode);
        g_fd = g_file != nullptr ? fileno(g_file) : -1;
    }
}

void to_file(std::string_view line) {
    if (g_file == nullptr) {
        return;
    }
    std::fwrite(line.data(), 1, line.size(), g_file);
    std::fputc('\n', g_file);
    std::fflush(g_file);
    g_written += line.size() + 1;
}

}  // namespace

void set_level(Level level) {
    g_level = level;
}

Level level() {
    return g_level;
}

const std::string& home() {
    static const std::string h = find_home();
    return h;
}

std::string tidy(std::string_view text) {
    const std::string& h = home();
    std::string out(text);
    if (h.empty()) {
        return out;
    }
    for (std::size_t at = out.find(h); at != std::string::npos; at = out.find(h, at + 1)) {
        // Only the folder itself or a path in it ("/Users/ann" in "/Users/anna" is someone else).
        const std::size_t end = at + h.size();
        if (end == out.size() || out[end] == '/' || out[end] == '\\' || out[end] == ' ' || out[end] == '"' ||
            out[end] == '\'' || out[end] == ':' || out[end] == ',' || out[end] == ')') {
            out.replace(at, h.size(), "~");
        }
    }
    return out;
}

void set_file(const char* path) {
    std::scoped_lock hold(g_lock);
    open_file(path, "a");
}

fs::path session_file(const fs::path& dir, const std::string& name, int n) {
    return n == 0 ? dir / (name + ".log") : dir / (name + "." + std::to_string(n) + ".log");
}

void rotate(const fs::path& dir, const std::string& name, int keep) {
    std::error_code ec;
    if (keep <= 0) {
        fs::remove(session_file(dir, name, 0), ec);
        return;
    }
    fs::remove(session_file(dir, name, keep), ec);
    for (int n = keep - 1; n >= 0; --n) {
        const fs::path from = session_file(dir, name, n);
        if (fs::exists(from, ec)) {
            fs::rename(from, session_file(dir, name, n + 1), ec);
        }
    }
}

bool open_session(const fs::path& dir, const std::string& name, int keep) {
    std::scoped_lock hold(g_lock);
    close_file();
    std::error_code ec;
    fs::create_directories(dir, ec);
    rotate(dir, name, keep);
    open_file(session_file(dir, name, 0).string().c_str(), "w");
    return g_file != nullptr;
}

void header(std::string_view line, bool write_it) {
    const std::string text = tidy(line);
    std::scoped_lock hold(g_lock);
    g_header += text;
    g_header += '\n';
    const std::size_t n = std::min(g_header.size(), sizeof(g_header_copy));
    std::memcpy(g_header_copy, g_header.data(), n);
    g_header_length = n;
    if (write_it) {
        to_file(text);
    }
}

std::string header_text() {
    std::scoped_lock hold(g_lock);
    return g_header;
}

void write(Level level, std::string_view message) {
    std::scoped_lock hold(g_lock);
    std::fprintf(
        stderr, "[%s] %.*s\n", prefix(level), static_cast<int>(message.size()), message.data()
    );
#if defined(__ANDROID__)
    // The game's activity has a stderr that goes nowhere (a windowless program's is read by the
    // launcher): Android's log (logcat) has it too.
    const int priority = level == Level::Error  ? ANDROID_LOG_ERROR
                         : level == Level::Warn ? ANDROID_LOG_WARN
                         : level == Level::Info ? ANDROID_LOG_INFO
                                                : ANDROID_LOG_DEBUG;
    __android_log_print(priority, "OpenRAC", "%.*s", static_cast<int>(message.size()), message.data());
#endif
    std::string line = "[";
    line += prefix(level);
    line += "] ";
    line += message;
    line = tidy(line);
    g_recent.push(line);
    if (g_written >= kSessionFileLimit && level < Level::Warn) {
        if (!g_limit_told) {
            g_limit_told = true;
            to_file("[warning] this session's log reached its limit: from here only warnings and errors are written "
                    "to it (OPENRAC_LOG or a probe left on?)");
        }
        return;
    }
    to_file(line);
}

void fatal(std::string_view message) {
    write(Level::Error, message);
    const std::string reason(message);
    crash::report(reason.c_str(), crash::Kind::Fatal);
    std::exit(1);
}

int file_descriptor() {
    return g_fd;
}

std::string_view header_for_crash() {
    return std::string_view(g_header_copy, g_header_length);
}

void recent_lines(void (*emit)(const char*, std::size_t, void*), void* context) {
    g_recent.each([&](const char* text, std::size_t length) { emit(text, length, context); });
}

std::size_t recent_capacity() {
    return decltype(g_recent)::capacity();
}

}  // namespace openrac::log
