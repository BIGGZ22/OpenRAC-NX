// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The program's fatal faults: an access to game memory that is not mapped (with the game address
// and the kind of access), an illegal instruction, an arithmetic fault, an abort, and on Windows
// the structured exceptions that are these; and a C++ exception nobody caught. Each writes a crash
// report (common/crash.h) and ends the program with status 3.

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <exception>

#include "common/crash.h"
#include "openrac/guest.h"
#include "openrac/memory.h"

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <csignal>
#else
#include <csignal>
#include <sys/mman.h>
#include <unistd.h>
#if defined(__linux__)
#include <ucontext.h>
#endif
#endif

namespace openrac::runtime {
namespace {

// The fault's words, built without allocating, then the report; the program ends here.
[[noreturn]] void report(const void* host, const char* what, crash::Kind kind) {
    crash::Text reason;
    gaddr address = 0;
    if (host != nullptr && Memory::get().contains(host, &address)) {
        reason.str(what).str(" at game address ").hex(address, 8).str(", which is not mapped");
    } else if (host != nullptr) {
        reason.str(what).str(" at host address ").hex(reinterpret_cast<std::uintptr_t>(host));
    } else {
        reason.str(what);
    }
    crash::report(reason.c_str(), kind);
    std::_Exit(3);
}

// std::terminate: a C++ exception nobody caught (or a noexcept one broken).
[[noreturn]] void on_terminate() {
    crash::Text reason;
    reason.str("a C++ exception nobody caught");
    if (const std::exception_ptr e = std::current_exception()) {
        try {
            std::rethrow_exception(e);
        } catch (const std::exception& x) {
            reason.str(": ").str(x.what());
        } catch (...) {
        }
    }
    crash::report(reason.c_str(), crash::Kind::Exception);
    std::_Exit(3);
}

#if defined(_WIN32)

// A first look at every exception, before any handler of the program's: only the hardware's faults
// are reported here (C++ exceptions pass through it too, and are caught where they are meant to be).
LONG WINAPI on_exception(EXCEPTION_POINTERS* info) {
    const DWORD code = info->ExceptionRecord->ExceptionCode;
    if (code == EXCEPTION_ACCESS_VIOLATION || code == EXCEPTION_IN_PAGE_ERROR) {
        const auto kind = info->ExceptionRecord->ExceptionInformation[0];
        const auto* host =
            reinterpret_cast<const void*>(info->ExceptionRecord->ExceptionInformation[1]);
        report(host, kind == 1 ? "write" : kind == 8 ? "execution" : "read", crash::Kind::Exception);
    }
    const char* what = nullptr;
    switch (code) {
        case EXCEPTION_ILLEGAL_INSTRUCTION:
            what = "illegal instruction";
            break;
        case EXCEPTION_PRIV_INSTRUCTION:
            what = "privileged instruction";
            break;
        case EXCEPTION_INT_DIVIDE_BY_ZERO:
            what = "integer division by zero";
            break;
        case EXCEPTION_STACK_OVERFLOW:
            what = "stack overflow";
            break;
        case EXCEPTION_DATATYPE_MISALIGNMENT:
            what = "misaligned access";
            break;
        case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
            what = "array bounds exceeded";
            break;
        default:
            return EXCEPTION_CONTINUE_SEARCH;
    }
    report(info->ExceptionRecord->ExceptionAddress, what, crash::Kind::Exception);
}

// Any other exception nothing handled.
LONG WINAPI on_unhandled(EXCEPTION_POINTERS* info) {
    crash::Text what;
    what.str("an unhandled exception, code ").hex(info->ExceptionRecord->ExceptionCode, 8);
    report(info->ExceptionRecord->ExceptionAddress, what.c_str(), crash::Kind::Exception);
}

void on_abort(int) {
    report(nullptr, "the program aborted (abort)", crash::Kind::Signal);
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
            crash::Text reason;
            reason.str("OPENRAC_GUARD: a write into the guarded range, at game address ").hex(address, 8);
            crash::report(reason.c_str(), crash::Kind::Signal);
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
    // The host functions on the way there are in the report's backtrace (the translated game code
    // is named after the game's).
    report(info->si_addr, sig == SIGBUS ? "access (SIGBUS)" : "access (SIGSEGV)", crash::Kind::Signal);
}

// The other faults: no game address to give.
void on_fault(int sig, siginfo_t* info, void*) {
    const char* what = "a fatal signal";
    switch (sig) {
        case SIGILL:
            what = "illegal instruction (SIGILL)";
            break;
        case SIGFPE:
            what = "arithmetic fault (SIGFPE)";
            break;
        case SIGABRT:
            what = "the program aborted (SIGABRT)";
            break;
        default:
            break;
    }
    report(sig == SIGABRT ? nullptr : info->si_addr, what, crash::Kind::Signal);
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
    std::set_terminate(on_terminate);
#if defined(_WIN32)
    AddVectoredExceptionHandler(1, on_exception);
    SetUnhandledExceptionFilter(on_unhandled);
    std::signal(SIGABRT, on_abort);
#else
    crash::prepare_thread();
    struct sigaction action {};

    action.sa_sigaction = on_signal;
    action.sa_flags = SA_SIGINFO | SA_ONSTACK;
    sigemptyset(&action.sa_mask);
    sigaction(SIGSEGV, &action, nullptr);
    sigaction(SIGBUS, &action, nullptr);
    action.sa_sigaction = on_fault;
    sigaction(SIGILL, &action, nullptr);
    sigaction(SIGFPE, &action, nullptr);
    sigaction(SIGABRT, &action, nullptr);
#endif
}

}  // namespace openrac::runtime
