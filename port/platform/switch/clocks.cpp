// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The Switch's clocks (clocks.h), through clkrst (system 8.0.0 and later) or pcv (before), the
// services sys-clk uses.
//
// What hardware session 51 showed: a request at start (CPU 1785, GPU 460.8, memory 1600, handheld)
// read back as set at once, and a minute later read back as stock (1020 / 307.2 / 1331.2), with no
// service error, no focus or mode hook and no other clock setter in the port. Something on the
// system side puts its own performance configuration back after an early request (the cause is
// not established: APM applying the program's configuration once it is displayed or in focus, or
// an external clock manager). So:
//  - a request is never reported as "set" without a readback that shows it;
//  - after a request the clocks are read back on a short schedule (0.25, 1, 3, 10 s, then every
//    5 s while the game has the focus), each reset is logged once with what is known (seconds since
//    the request, focus, mode, APM performance mode and configuration), and the request is made
//    again: the profile persists while the game has the focus;
//  - a reset that keeps coming back within seconds of each request (3 in a row) is called an
//    external override, logged plainly, and the requests back off to one every 30 s (every 2 s with
//    OPENRAC_SWITCH_CLOCK_KEEP=1: explicit ownership), never every frame;
//  - nothing is raised while the game is out of focus (HOME, sleep, an applet) or exiting; the
//    clocks are put back then, to the system's rates as they were before the first request,
//    retried while out of focus if the system refuses, and best effort at exit.
// Without any clock setting the program only reads and logs (stock behaviour).

#include "platform/switch/clocks.h"

#include <switch.h>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <format>
#include <iterator>
#include <string>
#include <string_view>

#include "common/log.h"
#include "platform/switch/clock_choice.h"

namespace openrac::platform::switch_clocks {

namespace {

namespace choice = openrac::platform::clock_choice;

enum Domain { kCpu, kGpu, kMem, kDomains };

constexpr const char* kNames[kDomains] = {"CPU", "GPU", "memory"};
constexpr PcvModuleId kModuleIds[kDomains] = {PcvModuleId_CpuBus, PcvModuleId_GPU, PcvModuleId_EMC};
constexpr PcvModule kModules[kDomains] = {PcvModule_CpuBus, PcvModule_GPU, PcvModule_EMC};

// The read-back schedule after a request (ms), then the steady interval; the backoff once an
// override is seen; the most detailed reset lines a run writes.
constexpr std::int64_t kCheckAfterMs[] = {250, 1000, 3000, 10000};
constexpr std::int64_t kSteadyMs = 5000;
constexpr std::int64_t kBackoffMs = 30000;
constexpr std::int64_t kKeepMs = 2000;
constexpr std::int64_t kQuickResetMs = 5000;  // a reset this soon after a request counts as quick
constexpr int kOverrideAfter = 3;             // quick resets in a row: an external override
constexpr int kMostResetLines = 24;
constexpr std::int64_t kRetryMaxMs = 30000;   // a refused request or restore: retried, doubling

std::int64_t now_ms() {
#ifdef OPENRAC_CLOCKS_TEST_NOW
    return OPENRAC_CLOCKS_TEST_NOW();
#else
    using namespace std::chrono;
    return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
#endif
}

struct DomainState {
    std::uint32_t want_hz = 0;      // the request for the current mode (0: leave alone)
    std::uint32_t baseline_hz = 0;  // the system's rate before this program's first request (to put back)
    bool owned = false;             // this program has requested a rate and not yet put it back
    bool requested = false;         // a request was made for it this run
    Result last_set = 0;            // the last request's result
    std::uint32_t read_hz = 0;      // the last read-back
    bool read_ok = false;
};

struct State {
    bool started = false;
    bool use_clkrst = false;
    bool service = false;
    bool apm = false;
    ClkrstSession sessions[kDomains] = {};
    bool session_open[kDomains] = {};
    DomainState d[kDomains];
    // What was asked.
    bool profile = false;  // OPENRAC_SWITCH_CLOCK_PROFILE=raised
    std::uint32_t asked_mhz[kDomains] = {};
    bool keep = false;  // OPENRAC_SWITCH_CLOCK_KEEP=1
    bool asked = false;
    // The lifecycle.
    bool hooked = false;
    AppletHookCookie hook = {};
    bool in_focus = false;
    bool exiting = false;
    std::int64_t start_ms = 0;
    std::int64_t requested_ms = 0;   // the last request
    int schedule = 0;                // the next entry of kCheckAfterMs
    std::int64_t next_check_ms = 0;  // 0: none
    std::int64_t retry_ms = 1000;
    std::int64_t apply_at_ms = 0;    // a request waiting (after a mode change), 0: none
    int quick_resets = 0;
    bool override_seen = false;
    int resets = 0, reset_lines = 0, requests = 0, refused = 0;
    std::int64_t next_minute_ms = 0;
};

State s;

std::uint32_t env_mhz(const char* value) {
    if (value == nullptr) {
        return 0;
    }
    const long v = std::strtol(value, nullptr, 10);
    return v > 0 && v < 100000 ? static_cast<std::uint32_t>(v) : 0;
}

bool docked() {
    return appletGetOperationMode() == AppletOperationMode_Console;
}

Result read_hz(Domain d, std::uint32_t& hz) {
    hz = 0;
    if (!s.service) {
        return MAKERESULT(Module_Libnx, LibnxError_NotInitialized);
    }
    if (s.use_clkrst) {
        return s.session_open[d] ? clkrstGetClockRate(&s.sessions[d], &hz) : MAKERESULT(Module_Libnx, LibnxError_NotInitialized);
    }
    return pcvGetClockRate(kModules[d], &hz);
}

Result write_hz(Domain d, std::uint32_t hz) {
    if (!s.service) {
        return MAKERESULT(Module_Libnx, LibnxError_NotInitialized);
    }
    if (s.use_clkrst) {
        return s.session_open[d] ? clkrstSetClockRate(&s.sessions[d], hz) : MAKERESULT(Module_Libnx, LibnxError_NotInitialized);
    }
    return pcvSetClockRate(kModules[d], hz);
}

// Reads every domain; a domain that cannot be read does not hide the others.
void read_all() {
    for (int i = 0; i < kDomains; ++i) {
        DomainState& x = s.d[i];
        x.read_ok = R_SUCCEEDED(read_hz(static_cast<Domain>(i), x.read_hz));
    }
}

std::string context() {
    std::string text = std::format("{}, {}", s.in_focus ? "in focus" : "out of focus", docked() ? "docked" : "handheld");
    // The program's performance configuration as the applet manager reports it (7.0.0+): a change
    // here next to a reset points at the system's own configuration being applied.
    u32 current = 0;
    if (hosversionAtLeast(7, 0, 0) && R_SUCCEEDED(appletGetCurrentPerformanceConfiguration(&current))) {
        text += std::format(", applet performance configuration {:#010x}", current);
    }
    if (s.apm) {
        ApmPerformanceMode mode = ApmPerformanceMode_Invalid;
        u32 conf = 0;
        if (R_SUCCEEDED(apmGetPerformanceMode(&mode)) && mode != ApmPerformanceMode_Invalid
            && R_SUCCEEDED(apmGetPerformanceConfiguration(mode, &conf))) {
            text += std::format(", APM {} configuration {:#010x}", mode == ApmPerformanceMode_Boost ? "boost" : "normal", conf);
        } else {
            text += ", APM not readable";
        }
    }
    return text;
}

std::string rates_text() {
    std::string text;
    for (int i = 0; i < kDomains; ++i) {
        const DomainState& x = s.d[i];
        text += std::format("{}{} ", i != 0 ? ", " : "", kNames[i]);
        text += x.read_ok ? std::format("{:.1f} MHz", x.read_hz / 1e6) : std::string("not readable");
    }
    return text;
}

// What each domain is: asked, last result, read back, put-back pending. Kept apart on purpose.
std::string ownership_text() {
    std::string text;
    for (int i = 0; i < kDomains; ++i) {
        const DomainState& x = s.d[i];
        if (x.want_hz == 0 && !x.owned) {
            continue;
        }
        const bool verified = x.read_ok && x.read_hz == x.want_hz;
        if (!x.requested) {
            text += std::format("; {}: to request {:.1f}", kNames[i], x.want_hz / 1e6);
        } else if (!x.owned) {
            text += std::format("; {}: requested {:.1f}, not held now (put back, or the system's own again)", kNames[i],
                                x.want_hz / 1e6);
        } else {
            text += std::format("; {}: requested {:.1f}, last request {}, read back {}, to put back to {:.1f}", kNames[i],
                                x.want_hz / 1e6,
                                R_SUCCEEDED(x.last_set) ? std::string("accepted") : std::format("REFUSED ({:#x})", x.last_set),
                                !x.read_ok ? std::string("not readable")
                                           : verified ? std::string("as requested") : std::string("NOT as requested"),
                                x.baseline_hz / 1e6);
        }
    }
    return text;
}

void log_clocks(std::string_view when) {
    read_all();
    log::info("switch clocks ({}): {}; {}{}", when, rates_text(), context(), s.asked ? ownership_text() : std::string("; not requested by the game (the rates are the system's, or another clock manager's)"));
}

// The requests for the current mode.
void choose_wants() {
    choice::Choice c = s.profile ? choice::raised_profile()
                                 : choice::choose(s.asked_mhz[kCpu], s.asked_mhz[kGpu], s.asked_mhz[kMem], docked());
    s.d[kCpu].want_hz = c.cpu_hz;
    s.d[kGpu].want_hz = c.gpu_hz;
    s.d[kMem].want_hz = c.mem_hz;
}

void schedule_checks(std::int64_t now) {
    s.schedule = 0;
    s.next_check_ms = now + kCheckAfterMs[0];
}

// Requests the clocks (only in focus and not exiting). The baseline of a domain is read before its
// first request and kept until it is put back; a domain whose rate cannot be read is left alone.
void request(std::int64_t now, std::string_view why) {
    if (!s.in_focus || s.exiting || !s.service) {
        return;
    }
    bool refused = false;
    for (int i = 0; i < kDomains; ++i) {
        DomainState& x = s.d[i];
        if (x.want_hz == 0) {
            continue;
        }
        if (!x.owned) {
            std::uint32_t hz = 0;
            const Result r = read_hz(static_cast<Domain>(i), hz);
            if (R_FAILED(r) || hz == 0) {
                log::warn("switch clocks: {} left unchanged until the focus comes back: its rate before the request "
                          "could not be read ({:#x})", kNames[i], r);
                x.want_hz = 0;
                continue;
            }
            x.baseline_hz = hz;
        }
        x.last_set = write_hz(static_cast<Domain>(i), x.want_hz);
        x.requested = true;
        x.owned = true;  // a refused request may still have changed it: put back either way
        if (R_FAILED(x.last_set)) {
            refused = true;
            ++s.refused;
            log::warn("switch clocks: the system refused {} at {:.1f} MHz (result {:#x}, module {}, description {})",
                      kNames[i], x.want_hz / 1e6, x.last_set, R_MODULE(x.last_set), R_DESCRIPTION(x.last_set));
        }
    }
    ++s.requests;
    s.requested_ms = now;
    if (refused) {
        // A refusal is retried on its own, doubling (1 s .. 30 s), not on the read-back schedule.
        s.next_check_ms = now + s.retry_ms;
        s.schedule = static_cast<int>(std::size(kCheckAfterMs));
        s.retry_ms = std::min(s.retry_ms * 2, kRetryMaxMs);
    } else {
        schedule_checks(now);
        s.retry_ms = 1000;
    }
    // Reported from a read-back, never from the request alone.
    log_clocks(std::format("requested ({}); read back now", why));
}

// Puts back every owned domain to its baseline (the GPU no higher than the handheld ceiling when
// handheld, unless the raised profile asked for more). Returns whether all of them went back.
bool restore(std::string_view why) {
    bool any = false, pending = false;
    for (int i = 0; i < kDomains; ++i) {
        DomainState& x = s.d[i];
        if (!x.owned) {
            continue;
        }
        any = true;
        std::uint32_t rate = x.baseline_hz;
        if (i == kGpu && !docked()) {
            rate = std::min(rate, choice::kGpuHandheldMaxHz);
        }
        const Result r = rate != 0 ? write_hz(static_cast<Domain>(i), rate) : MAKERESULT(Module_Libnx, LibnxError_NotInitialized);
        if (R_FAILED(r)) {
            pending = true;
            log::warn("switch clocks: could not put {} back to {:.1f} MHz (result {:#x}); will retry", kNames[i], rate / 1e6, r);
            continue;
        }
        x.owned = false;
    }
    if (any) {
        log_clocks(pending ? std::format("put back INCOMPLETE ({})", why) : std::format("put back ({})", why));
    }
    s.next_check_ms = pending ? now_ms() + s.retry_ms : 0;
    if (pending) {
        s.retry_ms = std::min(s.retry_ms * 2, kRetryMaxMs);
    }
    return !pending;
}

bool any_owned() {
    for (const auto& x : s.d) {
        if (x.owned) {
            return true;
        }
    }
    return false;
}

// A scheduled read-back: logs and answers resets.
void check(std::int64_t now) {
    if (!s.in_focus || s.exiting) {
        // Out of focus: only a put-back still owed is retried.
        if (any_owned()) {
            restore("retry while out of focus");
        }
        return;
    }
    read_all();
    bool reset = false, retry = false;
    for (int i = 0; i < kDomains; ++i) {
        const DomainState& x = s.d[i];
        if (x.want_hz == 0) {
            continue;
        }
        if (x.owned && R_FAILED(x.last_set)) {
            retry = true;  // refused
        } else if (x.read_ok && x.read_hz != x.want_hz) {
            reset = true;
        }
    }
    if (retry && !reset) {
        request(now, "retry after a refusal");
        return;
    }
    if (reset) {
        ++s.resets;
        const std::int64_t since = now - s.requested_ms;
        const bool quick = since <= kQuickResetMs;
        s.quick_resets = quick ? s.quick_resets + 1 : 0;
        if (s.reset_lines < kMostResetLines) {
            ++s.reset_lines;
            log::warn("switch clocks: reset #{} found {:.2f} s after request #{}: {}; {} (the system or another clock "
                      "manager changed the rates; making the request again)",
                      s.resets, since / 1000.0, s.requests, rates_text(), context());
        }
        if (!s.override_seen && s.quick_resets >= kOverrideAfter) {
            s.override_seen = true;
            log::warn("switch clocks: the rates are reset within {} s of every request ({} in a row): something "
                      "else (APM's performance configuration, or a clock manager such as sys-clk) keeps setting "
                      "them. Requests now every {} s{}",
                      kQuickResetMs / 1000, s.quick_resets, (s.keep ? kKeepMs : kBackoffMs) / 1000,
                      s.keep ? " (OPENRAC_SWITCH_CLOCK_KEEP=1)" : "; OPENRAC_SWITCH_CLOCK_KEEP=1 keeps them every 2 s, or turn the other manager off");
        }
        const std::int64_t wait = s.override_seen ? (s.keep ? kKeepMs : kBackoffMs) : 0;
        if (wait == 0 || now - s.requested_ms >= wait) {
            request(now, "again after a reset");
            return;
        }
        s.next_check_ms = s.requested_ms + wait;
        return;
    }
    // As requested (or nothing owned): next read-back.
    if (s.schedule + 1 < static_cast<int>(std::size(kCheckAfterMs))) {
        ++s.schedule;
        s.next_check_ms = s.requested_ms + kCheckAfterMs[s.schedule];
    } else {
        s.next_check_ms = now + kSteadyMs;
    }
}

void focus_changed(bool in_focus) {
    s.in_focus = in_focus;
    if (!s.asked) {
        return;
    }
    if (in_focus) {
        log_clocks("focus back");
        choose_wants();
        request(now_ms(), "the game has the focus");
    } else {
        restore("the game lost the focus: HOME, sleep or an applet");
    }
}

void on_applet(AppletHookType type, void*) {
    switch (type) {
        case AppletHookType_OnFocusState:
            focus_changed(appletGetFocusState() == AppletFocusState_InFocus);
            break;
        case AppletHookType_OnOperationMode:
        case AppletHookType_OnPerformanceMode:
            if (!s.asked) {
                log_clocks("mode change");
                break;
            }
            // The system applies its own configuration for the new mode. A domain it has already
            // changed is its own again (no write: our old baseline belongs to the old mode); one
            // still at our rate is put back. Then, after it settles, a new request with the new
            // mode's baseline and ceiling.
            read_all();
            for (int i = 0; i < kDomains; ++i) {
                DomainState& x = s.d[i];
                if (x.owned && x.read_ok && x.read_hz != x.want_hz) {
                    x.owned = false;
                }
            }
            restore("mode change");
            for (auto& x : s.d) {
                if (!x.owned) {
                    x.baseline_hz = 0;
                }
            }
            choose_wants();
            s.apply_at_ms = s.in_focus ? now_ms() + 500 : 0;
            break;
        case AppletHookType_OnExitRequest:
            s.exiting = true;
            restore("the program is asked to exit");
            break;
        default:
            break;
    }
}

}  // namespace

void start() {
    if (s.started) {
        return;
    }
    s = State{};
    s.started = true;
    s.start_ms = now_ms();
    const char* profile = std::getenv("OPENRAC_SWITCH_CLOCK_PROFILE");
    s.profile = profile != nullptr && std::string_view(profile) == "raised";
    s.asked_mhz[kCpu] = env_mhz(std::getenv("OPENRAC_SWITCH_CPU_MHZ"));
    s.asked_mhz[kGpu] = env_mhz(std::getenv("OPENRAC_SWITCH_GPU_MHZ"));
    s.asked_mhz[kMem] = env_mhz(std::getenv("OPENRAC_SWITCH_MEM_MHZ"));
    const char* keep = std::getenv("OPENRAC_SWITCH_CLOCK_KEEP");
    s.keep = keep != nullptr && std::string_view(keep) == "1";
    s.asked = s.profile || s.asked_mhz[kCpu] != 0 || s.asked_mhz[kGpu] != 0 || s.asked_mhz[kMem] != 0;
    s.use_clkrst = hosversionAtLeast(8, 0, 0);
    if (s.use_clkrst) {
        s.service = R_SUCCEEDED(clkrstInitialize());
        for (int i = 0; s.service && i < kDomains; ++i) {
            const Result r = clkrstOpenSession(&s.sessions[i], kModuleIds[i], 3);
            s.session_open[i] = R_SUCCEEDED(r);
            if (R_FAILED(r)) {
                log::warn("switch clocks: no clkrst session for {} (result {:#x})", kNames[i], r);
            }
        }
    } else {
        s.service = R_SUCCEEDED(pcvInitialize());
    }
    s.apm = R_SUCCEEDED(apmInitialize());
    s.in_focus = appletGetFocusState() == AppletFocusState_InFocus;
    choose_wants();
    log_clocks("start");
    // A hint, not proof: sys-clk's boot flag (its program id 00FF0000636C6BFF).
    if (std::FILE* f = std::fopen("sdmc:/atmosphere/contents/00FF0000636C6BFF/flags/boot2.flag", "rb")) {
        std::fclose(f);
        log::info("switch clocks: sys-clk is installed to start at boot; it may set the clocks too");
    }
    s.next_minute_ms = s.start_ms + 60000;
    if (!s.asked) {
        return;
    }
    if (!s.service) {
        log::warn("switch clocks: env.txt asks for clocks but the clock service is unavailable");
        return;
    }
    if (s.profile) {
        log::info("switch clocks: OPENRAC_SWITCH_CLOCK_PROFILE=raised: CPU 1785, GPU 768, memory 1600 MHz in both modes "
                  "(the per-domain settings are ignored). Opt-in; {}",
                  docked() ? "docked" : "HANDHELD: GPU 768 MHz is above every official handheld profile (460.8); watch battery and heat");
    } else {
        log::info("switch clocks: env.txt asks CPU {} / GPU {} / memory {} MHz; for this mode that is CPU {:.1f}, GPU {:.1f}, "
                  "memory {:.1f} MHz (0: left alone; GPU at most 460.8 handheld)",
                  s.asked_mhz[kCpu], s.asked_mhz[kGpu], s.asked_mhz[kMem], s.d[kCpu].want_hz / 1e6,
                  s.d[kGpu].want_hz / 1e6, s.d[kMem].want_hz / 1e6);
    }
    appletHook(&s.hook, on_applet, nullptr);
    s.hooked = true;
    // The L+R+Minus exit goes through std::exit, past the platform's end.
    static bool at_exit = false;
    if (!at_exit) {
        at_exit = true;
        std::atexit([] { stop(); });
    }
    request(s.start_ms, "start");
}

void tick() {
    if (!s.started || !s.service) {
        return;
    }
    const std::int64_t now = now_ms();
    if (s.asked) {
        if (s.apply_at_ms != 0 && now >= s.apply_at_ms) {
            s.apply_at_ms = 0;
            request(now, "after a mode change");
        } else if (s.next_check_ms != 0 && now >= s.next_check_ms) {
            check(now);
        }
    }
    if (now >= s.next_minute_ms) {
        s.next_minute_ms = now + 60000;
        log_clocks(std::format("each minute; {} request(s), {} reset(s), {} refused{}", s.requests, s.resets, s.refused,
                               s.override_seen ? ", external override seen" : ""));
    }
}

void stop() {
    if (!s.started) {
        return;
    }
    s.exiting = true;
    // Best effort, bounded: three attempts, no waiting (a dead service or a killed process cannot
    // be helped from here).
    for (int attempt = 0; attempt < 3 && any_owned(); ++attempt) {
        restore("the program ends");
    }
    if (any_owned()) {
        log::warn("switch clocks: the clocks could not all be put back at exit; the system's next performance "
                  "configuration change (HOME, another program) resets them");
    }
    if (s.hooked) {
        appletUnhook(&s.hook);
        s.hooked = false;
    }
    if (s.use_clkrst) {
        for (int i = 0; i < kDomains; ++i) {
            if (s.session_open[i]) {
                clkrstCloseSession(&s.sessions[i]);
                s.session_open[i] = false;
            }
        }
        if (s.service) {
            clkrstExit();
        }
    } else if (s.service) {
        pcvExit();
    }
    if (s.apm) {
        apmExit();
    }
    s.service = false;
    s.apm = false;
    s.started = false;
}

}  // namespace openrac::platform::switch_clocks
