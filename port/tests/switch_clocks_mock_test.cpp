// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// platform/switch/clocks.cpp over fake clock services (tests/switch_clocks_fake/switch.h) and a fake
// clock: the lifecycle, not Horizon. Covers stock (no request), the raised profile in handheld (no
// 460.8 cap), a reset some seconds after an accepted request (session 51's case) answered by a new
// request, an override that resets every request (backoff to 30 s, or 2 s with
// OPENRAC_SWITCH_CLOCK_KEEP=1), focus lost and back, dock and undock, a refused domain (retried,
// doubling) beside accepted ones, an unreadable domain, a refused put-back retried out of focus,
// the exit request, stop (bounded, even when the put-back is refused) and the pcv path.

#include <cstdint>
#include <cstdlib>

namespace test_clock {
inline std::int64_t now = 0;
inline std::int64_t read() { return now; }
}  // namespace test_clock
#define OPENRAC_CLOCKS_TEST_NOW test_clock::read

#include "platform/switch/clocks.cpp"
#include "tests/check.h"

namespace {

namespace sc = openrac::platform::switch_clocks;
namespace ch = openrac::platform::clock_choice;

void set_env(const char* name, const char* value) {
#ifdef _WIN32
    _putenv_s(name, value != nullptr ? value : "");
#else
    if (value != nullptr) {
        setenv(name, value, 1);
    } else {
        unsetenv(name);
    }
#endif
}

void clear_env() {
    for (const char* n : {"OPENRAC_SWITCH_CLOCK_PROFILE", "OPENRAC_SWITCH_CPU_MHZ", "OPENRAC_SWITCH_GPU_MHZ",
                          "OPENRAC_SWITCH_MEM_MHZ", "OPENRAC_SWITCH_CLOCK_KEEP"}) {
        set_env(n, nullptr);
    }
}

void reset_fake() {
    fake::mode = AppletOperationMode_Handheld;
    fake::focus = AppletFocusState_InFocus;
    fake::new_system = true;
    for (int i = 0; i < 3; ++i) {
        fake::read_fail[i] = fake::write_fail[i] = false;
    }
    fake::stock();
    fake::writes = fake::clkrst_writes = fake::pcv_writes = 0;
    test_clock::now = 1000000;
}

// Advances the fake clock in steps, ticking as the event pump would.
void run_for(std::int64_t ms, std::int64_t step = 50) {
    for (std::int64_t t = 0; t < ms; t += step) {
        test_clock::now += step;
        sc::tick();
    }
}

bool raised() {
    return fake::rates[0] == 1785000000u && fake::rates[1] == 768000000u && fake::rates[2] == 1600000000u;
}

bool handheld_stock() {
    return fake::rates[0] == 1020000000u && fake::rates[1] == 307200000u && fake::rates[2] == 1331200000u;
}

void focus(AppletFocusState f) {
    fake::focus = f;
    if (fake::hook) {
        fake::hook(AppletHookType_OnFocusState, nullptr);
    }
}

void stock_without_request() {
    reset_fake();
    clear_env();
    sc::start();
    run_for(180000, 1000);
    CHECK(fake::writes == 0);
    CHECK(handheld_stock());
    CHECK(fake::hook == nullptr);
    sc::stop();
    CHECK(fake::writes == 0);
}

void raised_profile_handheld_and_delayed_reset() {
    reset_fake();
    clear_env();
    set_env("OPENRAC_SWITCH_CLOCK_PROFILE", "raised");
    sc::start();
    // Both modes ask 1785 / 768 / 1600: handheld too, no 460.8 cap.
    CHECK(raised());
    CHECK(sc::s.d[sc::kGpu].want_hz == ch::kGpuDockedMaxHz);
    // Session 51: accepted, then reset to stock some seconds later.
    run_for(2000);
    fake::stock();
    run_for(12000);
    CHECK(raised());
    CHECK(sc::s.resets == 1);
    CHECK(!sc::s.override_seen);
    // Stays raised, read back every 5 s, no more requests while it holds.
    const int requests = sc::s.requests;
    run_for(60000);
    CHECK(raised() && sc::s.requests == requests);
    // Focus lost (HOME or sleep): put back to the baseline read before the first request; nothing is
    // raised while out of focus, whatever the ticks do.
    focus(AppletFocusState_Background);
    CHECK(handheld_stock());
    run_for(30000);
    CHECK(handheld_stock());
    // Back in focus: raised again.
    focus(AppletFocusState_InFocus);
    CHECK(raised());
    sc::stop();
    CHECK(handheld_stock());
}

void override_backoff(bool keep) {
    reset_fake();
    clear_env();
    set_env("OPENRAC_SWITCH_CLOCK_PROFILE", "raised");
    if (keep) {
        set_env("OPENRAC_SWITCH_CLOCK_KEEP", "1");
    }
    sc::start();
    // Something resets the clocks 100 ms after every change of ours, for two minutes.
    int seen_writes = fake::writes;
    std::int64_t changed_at = test_clock::now;
    for (std::int64_t t = 0; t < 120000; t += 50) {
        test_clock::now += 50;
        if (fake::writes != seen_writes) {
            seen_writes = fake::writes;
            changed_at = test_clock::now;
        }
        if (changed_at != 0 && test_clock::now - changed_at >= 100) {
            fake::stock();
            changed_at = 0;
        }
        sc::tick();
    }
    CHECK(sc::s.override_seen);
    // Bounded: 3 quick answers, then one request per backoff interval (30 s, or 2 s with KEEP).
    const int most = keep ? 3 + 120 / 2 + 2 : 3 + 120 / 30 + 2;
    CHECK(sc::s.requests <= most);
    CHECK(sc::s.requests >= (keep ? 40 : 4));
    sc::stop();
    CHECK(handheld_stock());
    clear_env();
}

void dock_undock_with_domain_requests() {
    reset_fake();
    clear_env();
    set_env("OPENRAC_SWITCH_GPU_MHZ", "768");
    sc::start();
    // Per-domain requests keep the official handheld cap: 460.8.
    CHECK(fake::rates[1] == 460800000u);
    // Docked: the system applies its docked rates; ours are put back, then asked again at 768.
    fake::mode = AppletOperationMode_Console;
    fake::stock();
    fake::hook(AppletHookType_OnOperationMode, nullptr);
    run_for(1000);
    CHECK(fake::rates[1] == 768000000u);
    // Undocked: back to 460.8, and the put-back never leaves the docked baseline above the cap.
    fake::mode = AppletOperationMode_Handheld;
    fake::hook(AppletHookType_OnOperationMode, nullptr);
    CHECK(fake::rates[1] <= ch::kGpuHandheldMaxHz);
    fake::stock();
    run_for(1000);
    CHECK(fake::rates[1] == 460800000u);
    sc::stop();
    CHECK(fake::rates[1] == 307200000u);
}

void refused_and_unreadable_domains() {
    reset_fake();
    clear_env();
    set_env("OPENRAC_SWITCH_CLOCK_PROFILE", "raised");
    fake::write_fail[1] = true;  // the GPU request is refused
    fake::read_fail[0] = true;   // the CPU cannot be read: left alone
    sc::start();
    CHECK(fake::rates[0] == 1020000000u);  // CPU untouched
    CHECK(fake::rates[1] == 307200000u);   // refused
    CHECK(fake::rates[2] == 1600000000u);  // memory set
    // The refusal is retried, doubling, not every frame.
    const int requests = sc::s.requests;
    run_for(10000);
    CHECK(sc::s.requests > requests && sc::s.requests <= requests + 4);
    fake::write_fail[1] = false;
    run_for(40000);
    CHECK(fake::rates[1] == 768000000u);
    CHECK(fake::rates[0] == 1020000000u);
    sc::stop();
    CHECK(handheld_stock());
}

void refused_put_back_and_exit() {
    reset_fake();
    clear_env();
    set_env("OPENRAC_SWITCH_CLOCK_PROFILE", "raised");
    sc::start();
    CHECK(raised());
    // Out of focus with every put-back refused: still owed, retried while out of focus.
    for (int i = 0; i < 3; ++i) {
        fake::write_fail[i] = true;
    }
    focus(AppletFocusState_Background);
    CHECK(sc::any_owned());
    run_for(5000);
    for (int i = 0; i < 3; ++i) {
        fake::write_fail[i] = false;
    }
    run_for(70000);
    CHECK(handheld_stock());
    CHECK(!sc::any_owned());
    // Exit asked while in focus: put back, and never raised after it.
    focus(AppletFocusState_InFocus);
    CHECK(raised());
    fake::hook(AppletHookType_OnExitRequest, nullptr);
    CHECK(handheld_stock());
    focus(AppletFocusState_InFocus);
    run_for(20000);
    CHECK(handheld_stock());
    sc::stop();
    // stop with the put-back refused: bounded, returns, services closed.
    sc::start();
    fake::focus = AppletFocusState_InFocus;
    CHECK(raised());
    for (int i = 0; i < 3; ++i) {
        fake::write_fail[i] = true;
    }
    const int closed = fake::closed;
    sc::stop();
    CHECK(fake::closed == closed + 1);
    CHECK(!sc::s.started);
    for (int i = 0; i < 3; ++i) {
        fake::write_fail[i] = false;
    }
}

void pcv_path() {
    reset_fake();
    clear_env();
    fake::new_system = false;
    set_env("OPENRAC_SWITCH_CPU_MHZ", "1785");
    sc::start();
    CHECK(fake::rates[0] == 1785000000u);
    CHECK(fake::pcv_writes > 0 && fake::clkrst_writes == 0);
    sc::stop();
    CHECK(fake::rates[0] == 1020000000u);
    clear_env();
}

void started_out_of_focus() {
    reset_fake();
    clear_env();
    set_env("OPENRAC_SWITCH_CLOCK_PROFILE", "raised");
    fake::focus = AppletFocusState_Background;
    sc::start();
    run_for(5000);
    CHECK(handheld_stock());
    focus(AppletFocusState_InFocus);
    CHECK(raised());
    sc::stop();
    clear_env();
}

}  // namespace

int main() {
    stock_without_request();
    raised_profile_handheld_and_delayed_reset();
    override_backoff(false);
    override_backoff(true);
    dock_undock_with_domain_requests();
    refused_and_unreadable_domains();
    refused_put_back_and_exit();
    pcv_path();
    started_out_of_focus();
    return openrac::test::result();
}
