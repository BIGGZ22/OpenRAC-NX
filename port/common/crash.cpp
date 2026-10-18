// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The crash report (crash.h). Everything report() touches is set up by configure(): the folder's
// name, the program's, the version line, the time zone and the program's load address; it then
// formats with its own digits into fixed buffers and writes with write(). This file includes no
// header of the port's but its own, log.h and build_info.h (windows.h's macros clash with the
// port's names).

#include "common/crash.h"

#include <atomic>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <string>

#include "common/build_info.h"
#include "common/log.h"

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <dbghelp.h>
#include <direct.h>
#include <fcntl.h>
#include <io.h>
#include <sys/stat.h>
#pragma comment(lib, "dbghelp.lib")
#else
#include <dlfcn.h>
#include <execinfo.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace openrac::crash {
namespace {

// ---- The system's calls, the same on each ----

#if defined(_WIN32)
void put_fd(int fd, const char* s, std::size_t n) {
    while (n > 0) {
        const int done = _write(fd, s, static_cast<unsigned>(n > 0x10000 ? 0x10000 : n));
        if (done <= 0) {
            return;
        }
        s += done;
        n -= static_cast<std::size_t>(done);
    }
}
int open_new(const char* path) {
    return _open(path, _O_WRONLY | _O_CREAT | _O_EXCL | _O_BINARY, _S_IREAD | _S_IWRITE);
}
void make_dir(const char* path) {
    _mkdir(path);
}
void close_fd(int fd) {
    _close(fd);
}
constexpr int kStderr = 2;
#else
void put_fd(int fd, const char* s, std::size_t n) {
    while (n > 0) {
        const ssize_t done = ::write(fd, s, n);
        if (done <= 0) {
            return;
        }
        s += done;
        n -= static_cast<std::size_t>(done);
    }
}
int open_new(const char* path) {
    return ::open(path, O_WRONLY | O_CREAT | O_EXCL, 0644);
}
void make_dir(const char* path) {
    ::mkdir(path, 0755);
}
void close_fd(int fd) {
    ::close(fd);
}
constexpr int kStderr = STDERR_FILENO;
#endif

// ---- What configure() prepared ----

char g_folder[1024] = {};       // where reports go, as the system names it
char g_folder_shown[1024] = {};  // the same with the home folder as ~
char g_program[128] = "openrac";
char g_version[160] = {};  // "OpenRAC 0.7.0, commit f3df7852"
char g_session[64] = {};
long g_offset = 0;
std::uintptr_t g_base = 0;  // where the program is loaded (to turn backtrace addresses into its own)
GameState g_state;
std::atomic<int> g_reporting{0};
char g_written[1100] = {};

void copy(char* to, std::size_t size, const std::string& from) {
    const std::size_t n = from.size() < size - 1 ? from.size() : size - 1;
    std::memcpy(to, from.data(), n);
    to[n] = '\0';
}

// ---- Writing to several files at once, through a small buffer ----

class Out {
public:
    Out(const int* fds, int count) : m_count(count) {
        for (int i = 0; i < count && i < 3; ++i) {
            m_fds[i] = fds[i];
        }
    }
    ~Out() { flush(); }

    Out& put(const char* s, std::size_t n) {
        while (n > 0) {
            if (m_len == sizeof(m_buf)) {
                flush();
            }
            const std::size_t room = sizeof(m_buf) - m_len;
            const std::size_t k = n < room ? n : room;
            std::memcpy(m_buf + m_len, s, k);
            m_len += k;
            s += k;
            n -= k;
        }
        return *this;
    }
    Out& str(const char* s) { return put(s, std::strlen(s)); }
    Out& text(const Text& t) { return put(t.c_str(), t.size()); }
    void flush() {
        for (int i = 0; i < m_count; ++i) {
            if (m_fds[i] >= 0) {
                put_fd(m_fds[i], m_buf, m_len);
            }
        }
        m_len = 0;
    }
    const int* fds() const { return m_fds; }
    int count() const { return m_count; }

private:
    int m_fds[3] = {-1, -1, -1};
    int m_count = 0;
    char m_buf[2048];
    std::size_t m_len = 0;
};

// Days since 1970-01-01 to a civil date (Howard Hinnant's algorithm): no library call, so it can
// run in a signal handler.
void civil(long long days, int& y, int& m, int& d) {
    days += 719468;
    const long long era = (days >= 0 ? days : days - 146096) / 146097;
    const long long doe = days - era * 146097;
    const long long yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    const long long doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    const long long mp = (5 * doy + 2) / 153;
    d = static_cast<int>(doy - (153 * mp + 2) / 5 + 1);
    m = static_cast<int>(mp < 10 ? mp + 3 : mp - 9);
    y = static_cast<int>(yoe + era * 400 + (m <= 2 ? 1 : 0));
}

void two(char* at, int v) {
    at[0] = static_cast<char>('0' + (v / 10) % 10);
    at[1] = static_cast<char>('0' + v % 10);
}

struct Clock {
    int year, month, day, hour, minute, second;
};

Clock clock_at(long long utc, long offset) {
    const long long local = utc + offset;
    long long days = local / 86400;
    long long rest = local % 86400;
    if (rest < 0) {
        rest += 86400;
        --days;
    }
    Clock c{};
    civil(days, c.year, c.month, c.day);
    c.hour = static_cast<int>(rest / 3600);
    c.minute = static_cast<int>(rest / 60 % 60);
    c.second = static_cast<int>(rest % 60);
    return c;
}

// One float as "-123.45" (two decimals), without printf.
void decimal(Text& t, float f) {
    if (std::isnan(f)) {
        t.str("nan");
        return;
    }
    if (std::isinf(f)) {
        t.str(f < 0 ? "-inf" : "inf");
        return;
    }
    if (f < 0) {
        t.str("-");
        f = -f;
    }
    if (f >= 1e15f) {
        t.str("huge");
        return;
    }
    const long long hundredths = static_cast<long long>(static_cast<double>(f) * 100.0 + 0.5);
    t.dec(hundredths / 100).str(".");
    const int frac = static_cast<int>(hundredths % 100);
    char d[3] = {};
    two(d, frac);
    t.str(d);
}

void write_state(Out& out) {
    const GameState& s = g_state;
    Text t;
    t.str("Game:     ");
    if (s.frame < 0) {
        t.str("not started");
    } else {
        if (s.level >= 0) {
            t.str("level ").dec(s.level);
        } else {
            t.str("no level (title or menus)");
        }
        if (s.has_mode) {
            t.str(", game mode ").dec(static_cast<long long>(s.mode));
        }
        t.str(", frame ").dec(s.frame);
        if (s.has_hero) {
            t.str(", hero at ");
            decimal(t, s.hero[0]);
            t.str(" ");
            decimal(t, s.hero[1]);
            t.str(" ");
            decimal(t, s.hero[2]);
        }
    }
    t.str("\n");
    out.text(t);
}

// The frames, taken once by capture() and written to each file from there.
void* g_frames[64];
int g_frame_count = 0;

void capture() {
#if defined(_WIN32)
    g_frame_count = CaptureStackBackTrace(0, 62, g_frames, nullptr);
#else
    g_frame_count = backtrace(g_frames, 64);
#endif
}

void write_backtrace(Out& out) {
    out.str("\nBacktrace (the newest call first):\n");
    out.flush();
    void** frames = g_frames;
#if defined(_WIN32)
    const USHORT count = static_cast<USHORT>(g_frame_count);
    HANDLE process = GetCurrentProcess();
    alignas(SYMBOL_INFO) char storage[sizeof(SYMBOL_INFO) + 256];
    for (USHORT i = 0; i < count; ++i) {
        Text t;
        t.dec(i).str("  ");
        const auto address = reinterpret_cast<DWORD64>(frames[i]);
        HMODULE module = nullptr;
        char path[MAX_PATH];
        if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                               static_cast<LPCSTR>(frames[i]), &module) != 0 &&
            GetModuleFileNameA(module, path, static_cast<DWORD>(sizeof(path))) != 0) {
            const char* file = std::strrchr(path, '\\');
            t.str(file != nullptr ? file + 1 : path).str("+");
            t.hex(address - reinterpret_cast<DWORD64>(module));
        } else {
            t.hex(address);
        }
        auto* symbol = reinterpret_cast<SYMBOL_INFO*>(storage);
        std::memset(storage, 0, sizeof(storage));
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        symbol->MaxNameLen = 255;
        DWORD64 displacement = 0;
        if (SymFromAddr(process, address, &displacement, symbol)) {
            t.str(" ").str(symbol->Name).str(" + ").dec(static_cast<long long>(displacement));
            IMAGEHLP_LINE64 line{};
            line.SizeOfStruct = sizeof(line);
            DWORD column = 0;
            if (SymGetLineFromAddr64(process, address, &column, &line)) {
                const char* file = std::strrchr(line.FileName, '\\');
                t.str(" (").str(file != nullptr ? file + 1 : line.FileName).str(":").dec(line.LineNumber).str(")");
            }
        }
        t.str("\n");
        out.text(t);
    }
    out.flush();
#else
    const int count = g_frame_count;
    for (int i = 0; i < out.count(); ++i) {
        if (out.fds()[i] >= 0) {
            backtrace_symbols_fd(frames, count, out.fds()[i]);
        }
    }
#endif
    if (g_base != 0) {
        Text t;
        t.str("(the program is loaded at ").hex(g_base).str("; less that, an address is the one in its symbols)\n");
        out.text(t);
    }
}

void write_head(Out& out, const char* reason, Kind kind, long long now) {
    char when[40];
    format_time(now, g_offset, when, sizeof(when));
    out.str("Reason:   ").str(reason).str("\n");
    out.str("Kind:     ").str(kind_name(kind)).str("\n");
    out.str("When:     ").str(when);
    if (g_session[0] != '\0') {
        out.str(" (the session began ").str(g_session).str(")");
    }
    out.str("\n");
    out.str("Program:  ").str(g_program).str(", ").str(g_version).str("\n");
    write_state(out);
}

void emit_line(const char* text, std::size_t length, void* context) {
    auto* out = static_cast<Out*>(context);
    out->put("  ", 2).put(text, length).put("\n", 1);
}

void write_tail(Out& out) {
    out.str("\nThe log's last lines (the oldest first):\n");
    log::recent_lines(emit_line, &out);
    out.str("\nThe session:\n");
    const std::string_view header = log::header_for_crash();
    out.put(header.data(), header.size());
    out.str("==== end of the report ====\n");
}

}  // namespace

// ---- Text ----

Text& Text::str(const char* s) {
    while (s != nullptr && *s != '\0' && m_len + 1 < sizeof(m_buf)) {
        m_buf[m_len++] = *s++;
    }
    m_buf[m_len] = '\0';
    return *this;
}

Text& Text::dec(long long v) {
    char digits[24];
    int n = 0;
    const bool negative = v < 0;
    unsigned long long u = negative ? 0ull - static_cast<unsigned long long>(v) : static_cast<unsigned long long>(v);
    do {
        digits[n++] = static_cast<char>('0' + u % 10);
        u /= 10;
    } while (u != 0);
    if (negative) {
        digits[n++] = '-';
    }
    char text[24];
    for (int i = 0; i < n; ++i) {
        text[i] = digits[n - 1 - i];
    }
    text[n] = '\0';
    return str(text);
}

Text& Text::hex(std::uint64_t v, int digits) {
    char text[24] = "0x";
    char rev[17];
    int n = 0;
    do {
        rev[n++] = "0123456789ABCDEF"[v & 15];
        v >>= 4;
    } while (v != 0 && n < 16);
    while (n < digits && n < 16) {
        rev[n++] = '0';
    }
    for (int i = 0; i < n; ++i) {
        text[2 + i] = rev[n - 1 - i];
    }
    text[2 + n] = '\0';
    return str(text);
}

const char* kind_name(Kind kind) {
    switch (kind) {
        case Kind::Signal:
            return "fault";
        case Kind::Exception:
            return "exception";
        case Kind::Fatal:
            return "fatal error";
        case Kind::Trap:
            return "game trap";
    }
    return "?";
}

// ---- Times ----

void file_name(long long utc, long offset, char* out, std::size_t size) {
    const Clock c = clock_at(utc, offset);
    char name[] = "crash-0000-00-00_00-00-00.txt";
    two(name + 6, c.year / 100);
    two(name + 8, c.year % 100);
    two(name + 11, c.month);
    two(name + 14, c.day);
    two(name + 17, c.hour);
    two(name + 20, c.minute);
    two(name + 23, c.second);
    if (size > 0) {
        const std::size_t n = sizeof(name) - 1 < size - 1 ? sizeof(name) - 1 : size - 1;
        std::memcpy(out, name, n);
        out[n] = '\0';
    }
}

void format_time(long long utc, long offset, char* out, std::size_t size) {
    const Clock c = clock_at(utc, offset);
    char text[] = "0000-00-00 00:00:00 +0000";
    two(text, c.year / 100);
    two(text + 2, c.year % 100);
    two(text + 5, c.month);
    two(text + 8, c.day);
    two(text + 11, c.hour);
    two(text + 14, c.minute);
    two(text + 17, c.second);
    const long a = offset < 0 ? -offset : offset;
    text[20] = offset < 0 ? '-' : '+';
    two(text + 21, static_cast<int>(a / 3600));
    two(text + 23, static_cast<int>(a / 60 % 60));
    if (size > 0) {
        const std::size_t n = sizeof(text) - 1 < size - 1 ? sizeof(text) - 1 : size - 1;
        std::memcpy(out, text, n);
        out[n] = '\0';
    }
}

long local_offset() {
    const std::time_t now = std::time(nullptr);
    std::tm utc{};
#if defined(_WIN32)
    gmtime_s(&utc, &now);
#else
    gmtime_r(&now, &utc);
#endif
    utc.tm_isdst = -1;  // mktime reads it as local time, its summer time found
    return static_cast<long>(now - std::mktime(&utc));
}

// ---- Set-up ----

void configure(const std::filesystem::path& folder, const std::string& program, const std::string& session) {
    copy(g_folder, sizeof(g_folder), folder.string());
    copy(g_folder_shown, sizeof(g_folder_shown), log::tidy(folder.string()));
    copy(g_program, sizeof(g_program), program);
    copy(g_version, sizeof(g_version),
         std::string("OpenRAC ") + build_info::version() + ", commit " + build_info::commit());
    copy(g_session, sizeof(g_session), session);
    g_offset = local_offset();
#if defined(_WIN32)
    SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES);
    SymInitialize(GetCurrentProcess(), nullptr, TRUE);
    g_base = reinterpret_cast<std::uintptr_t>(GetModuleHandleA(nullptr));
#else
    // The first backtrace() loads what it unwinds with (glibc: libgcc_s), which allocates: here,
    // not in the handler.
    void* frames[4];
    backtrace(frames, 4);
    Dl_info info{};
    if (dladdr(reinterpret_cast<void*>(&configure), &info) != 0) {
        g_base = reinterpret_cast<std::uintptr_t>(info.dli_fbase);
    }
#endif
}

void prepare_thread() {
#if !defined(_WIN32)
    thread_local bool done = false;
    if (done) {
        return;
    }
    done = true;
    constexpr std::size_t kSize = 256 * 1024;  // backtrace_symbols_fd and the report's buffers fit
    stack_t stack{};
    stack.ss_sp = std::malloc(kSize);  // kept for the thread's life
    stack.ss_size = kSize;
    stack.ss_flags = 0;
    if (stack.ss_sp != nullptr) {
        sigaltstack(&stack, nullptr);
    }
#endif
}

void set_state(const GameState& s) {
    g_state = s;
}

const GameState& state() {
    return g_state;
}

// ---- The report ----

void write_report(int fd, const char* reason, Kind kind, long long now) {
    const int fds[1] = {fd};
    Out out(fds, 1);
    out.str("==== OpenRAC crash report ====\n");
    write_head(out, reason, kind, now);
    capture();
    write_backtrace(out);
    write_tail(out);
}

const char* report(const char* reason, Kind kind) {
    if (g_reporting.exchange(1) != 0) {
        return nullptr;
    }
    capture();
    const long long now = static_cast<long long>(std::time(nullptr));
    int file = -1;
    if (g_folder[0] != '\0') {
        make_dir(g_folder);
        char name[64];
        file_name(now, g_offset, name, sizeof(name));
        for (int attempt = 0; attempt < 10 && file < 0; ++attempt) {
            Text path;
            path.str(g_folder).str("/");
            if (attempt == 0) {
                path.str(name);
            } else {
                // Two in one second (a fault while the first was reported cannot get here): -2...
                char stem[64];
                std::memcpy(stem, name, sizeof(stem));
                stem[std::strlen(stem) - 4] = '\0';
                path.str(stem).str("-").dec(attempt + 1).str(".txt");
            }
            file = open_new(path.c_str());
            if (file >= 0) {
                std::memcpy(g_written, path.c_str(), path.size() + 1);
            }
        }
    }
    // The file gets the whole report; the session log and stderr the reason, the backtrace and the
    // state (the log has its own header and lines already).
    const int log_fd = log::file_descriptor();
    {
        const int fds[2] = {log_fd, kStderr};
        Out out(fds, 2);
        out.str("[error] ==== the program stopped: ").str(kind_name(kind)).str(" ====\n");
        write_head(out, reason, kind, now);
        write_backtrace(out);
        if (file >= 0) {
            const char* slash = std::strrchr(g_written, '/');
            out.str("Crash report: ").str(g_folder_shown).str(slash != nullptr ? slash : "").str("\n");
        }
        out.str("[error] ==== end of the crash ====\n");
    }
    if (file < 0) {
        return nullptr;
    }
    {
        const int fds[1] = {file};
        Out out(fds, 1);
        out.str("==== OpenRAC crash report ====\n");
        write_head(out, reason, kind, now);
        write_backtrace(out);
        write_tail(out);
    }
    close_fd(file);
    return g_written;
}

}  // namespace openrac::crash
