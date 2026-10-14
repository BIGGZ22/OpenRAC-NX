// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Reports an access to game memory that is not mapped: the game address and
// the kind of access, then ends the program. Anything else is left to the
// system's own handling.

#include <cstdint>
#include <cstdio>
#include <cstdlib>

#include "openrac/guest.h"
#include "openrac/memory.h"

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#else
#include <csignal>
#include <execinfo.h>
#include <sys/mman.h>
#include <ucontext.h>
#include <unistd.h>
#endif

namespace openrac::runtime {
namespace {

void report(const void* host, const char* what) {
    gaddr address = 0;
    if (Memory::get().contains(host, &address)) {
        std::fprintf(
            stderr, "[error] %s at game address 0x%08X, which is not mapped\n", what, address
        );
    } else {
        std::fprintf(stderr, "[error] %s at host address %p\n", what, host);
    }
    std::fflush(stderr);
}

#if defined(_WIN32)

LONG WINAPI on_exception(EXCEPTION_POINTERS* info) {
    if (info->ExceptionRecord->ExceptionCode != EXCEPTION_ACCESS_VIOLATION) {
        return EXCEPTION_CONTINUE_SEARCH;
    }
    const auto kind = info->ExceptionRecord->ExceptionInformation[0];
    const auto* host =
        reinterpret_cast<const void*>(info->ExceptionRecord->ExceptionInformation[1]);
    report(host, kind == 1 ? "write" : "read");
    std::_Exit(3);
}

#else

// OPENRAC_GUARD (guard_writes): the guarded range and the pages it lies in, read-only while armed.
gaddr g_guard_low = 0, g_guard_high = 0;
std::uint8_t* g_guard_page = nullptr;
std::size_t g_guard_span = 0;
std::uint32_t g_guard_skip = 0;

// After a write the guard let through: the page is read-only again once that one instruction ran
// (x86-64 Linux: the trap flag single-steps it).
void on_step(int, siginfo_t*, void* context) {
#if defined(__linux__) && defined(__x86_64__)
    auto* uc = static_cast<ucontext_t*>(context);
    uc->uc_mcontext.gregs[REG_EFL] &= ~0x100LL;
    if (g_guard_page != nullptr) {
        mprotect(g_guard_page, g_guard_span, PROT_READ);
    }
#else
    (void)context;
#endif
}

void on_signal(int sig, siginfo_t* info, void* context) {
    auto* at = static_cast<std::uint8_t*>(info->si_addr);
    if (g_guard_page != nullptr && at >= g_guard_page && at < g_guard_page + g_guard_span) {
        gaddr address = 0;
        if (Memory::get().contains(at, &address) && address >= g_guard_low && address < g_guard_high
            && g_guard_skip-- == 0) {
            std::fprintf(stderr, "[error] a write into the guarded range, at game address 0x%08X, by:\n", address);
            void* frames[32];
            const int count = backtrace(frames, 32);
            backtrace_symbols_fd(frames, count, STDERR_FILENO);
            _exit(3);
        }
        // Another write to the page: let this one through, then guard again (on_step).
        mprotect(g_guard_page, g_guard_span, PROT_READ | PROT_WRITE);
#if defined(__linux__) && defined(__x86_64__)
        static_cast<ucontext_t*>(context)->uc_mcontext.gregs[REG_EFL] |= 0x100LL;
#else
        (void)context;
#endif
        return;
    }
    report(info->si_addr, sig == SIGBUS ? "bus error" : "access");
    // The host functions on the way there (the translated game code is named after the game's).
    void* frames[32];
    const int count = backtrace(frames, 32);
    backtrace_symbols_fd(frames, count, STDERR_FILENO);
    _exit(3);
}

#endif

}  // namespace

void guard_writes(gaddr address, std::uint32_t bytes, std::uint32_t skip) {
#if !defined(_WIN32)
    const auto page = static_cast<std::uintptr_t>(sysconf(_SC_PAGESIZE));
    const auto first = reinterpret_cast<std::uintptr_t>(openrac_guest_base + address) & ~(page - 1);
    const auto last = reinterpret_cast<std::uintptr_t>(openrac_guest_base + address + bytes - 1) & ~(page - 1);
    if (g_guard_high == 0) {
        g_guard_skip = skip;
    }
    g_guard_low = address;
    g_guard_high = address + bytes;
    g_guard_page = reinterpret_cast<std::uint8_t*>(first);
    g_guard_span = last - first + page;
    static bool stepping = false;
    if (!stepping) {
        stepping = true;
        struct sigaction step {};
        step.sa_sigaction = on_step;
        step.sa_flags = SA_SIGINFO;
        sigemptyset(&step.sa_mask);
        sigaction(SIGTRAP, &step, nullptr);
    }
    mprotect(g_guard_page, g_guard_span, PROT_READ);
#else
    (void)address;
    (void)bytes;
    (void)skip;
#endif
}

void install_crash_handler() {
#if defined(_WIN32)
    AddVectoredExceptionHandler(1, on_exception);
#else
    struct sigaction action {};

    action.sa_sigaction = on_signal;
    action.sa_flags = SA_SIGINFO;
    sigemptyset(&action.sa_mask);
    sigaction(SIGSEGV, &action, nullptr);
    sigaction(SIGBUS, &action, nullptr);
#endif
}

}  // namespace openrac::runtime
