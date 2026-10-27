// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

#include "common/sampler.h"

#if defined(__linux__) && (defined(__aarch64__) || defined(__x86_64__))
#define OPENRAC_SAMPLER 1
#endif

#if defined(OPENRAC_SAMPLER)
#include <algorithm>
#include <array>
#include <atomic>
#include <cstdint>
#include <cstdlib>
#include <cxxabi.h>
#include <dlfcn.h>
#include <format>
#include <map>
#include <memory>
#include <mutex>
#include <pthread.h>
#include <signal.h>
#include <string>
#include <time.h>
#include <ucontext.h>
#include <unistd.h>
#include <unordered_map>
#include <utility>
#include <vector>

#include <sys/syscall.h>

// glibc before 2.35 names the thread of SIGEV_THREAD_ID only by its union member.
#if !defined(sigev_notify_thread_id)
#define sigev_notify_thread_id _sigev_un._tid
#endif
#endif

namespace openrac::sampler {

#if defined(OPENRAC_SAMPLER)
namespace {

constexpr std::size_t kThreads = 4;           // threads sampled at once, at most
constexpr std::uint32_t kSamples = 1u << 18;  // a thread's samples kept: over four minutes
constexpr long kIntervalNs = 1'000'000;       // one sample a millisecond of processor time
constexpr std::size_t kShown = 30;            // functions listed per thread

// One sampled thread. The signal handler finds its slot by the thread's id and appends the
// interrupted instruction's address and the return address register's (aarch64; 0 elsewhere);
// everything else happens outside the handler.
struct Slot {
    std::atomic<pid_t> tid{0};
    const char* name = nullptr;
    timer_t timer{};
    std::atomic<bool> left_out{false};  // leave_out()
    std::atomic<std::uint32_t> count{0};
    std::unique_ptr<std::uintptr_t[]> pcs;
    std::unique_ptr<std::uintptr_t[]> returns;
};

std::array<Slot, kThreads> g_slots;
std::atomic<bool> g_running{false};
std::atomic<unsigned> g_epoch{0};  // a new one each start: threads arm again
std::mutex g_mutex;                // start, stop and arming, never the handler

pid_t thread_id() {
    return static_cast<pid_t>(syscall(SYS_gettid));
}

std::uintptr_t interrupted_pc(void* context) {
    const auto* uc = static_cast<const ucontext_t*>(context);
#if defined(__aarch64__)
    return static_cast<std::uintptr_t>(uc->uc_mcontext.pc);
#else
    return static_cast<std::uintptr_t>(uc->uc_mcontext.gregs[REG_RIP]);
#endif
}

// Where the interrupted function returns to, when the processor keeps it in a register: a leaf
// function's caller (memcpy's, say). 0 where it is on the stack instead.
std::uintptr_t interrupted_return(void* context) {
#if defined(__aarch64__)
    return static_cast<std::uintptr_t>(
        static_cast<const ucontext_t*>(context)->uc_mcontext.regs[30]
    );
#else
    (void)context;
    return 0;
#endif
}

// Async-signal-safe: atomics, a system call and a store into memory made beforehand.
void on_signal(int, siginfo_t*, void* context) {
    if (!g_running.load(std::memory_order_relaxed)) {
        return;
    }
    const pid_t tid = thread_id();
    for (Slot& slot : g_slots) {
        if (slot.tid.load(std::memory_order_relaxed) == tid) {
            if (slot.left_out.load(std::memory_order_relaxed)) {
                return;
            }
            const std::uint32_t at = slot.count.fetch_add(1, std::memory_order_relaxed);
            if (at < kSamples) {
                slot.pcs[at] = interrupted_pc(context);
                slot.returns[at] = interrupted_return(context);
            }
            return;
        }
    }
}

// The handler stays installed once it is: a late signal after stop() must not end the program,
// which SIGPROF's default action would.
void install_handler() {
    static const bool installed = [] {
        struct sigaction action{};
        action.sa_sigaction = on_signal;
        action.sa_flags = SA_SIGINFO | SA_RESTART;
        sigemptyset(&action.sa_mask);
        return sigaction(SIGPROF, &action, nullptr) == 0;
    }();
    (void)installed;
}

// A function's name for the report: demangled, or the library and offset when it has no symbol.
std::string function_name(std::uintptr_t pc) {
    Dl_info info{};
    if (dladdr(reinterpret_cast<void*>(pc), &info) == 0) {
        return "(unknown)";
    }
    std::string library = info.dli_fname != nullptr ? info.dli_fname : "?";
    library = library.substr(library.find_last_of('/') + 1);
    if (info.dli_sname == nullptr) {
        return std::format("({} without a symbol)", library);
    }
    int status = 0;
    const std::unique_ptr<char, decltype(&std::free)> demangled(
        abi::__cxa_demangle(info.dli_sname, nullptr, nullptr, &status), &std::free
    );
    std::string name = status == 0 && demangled ? demangled.get() : info.dli_sname;
    // Template and argument lists make lines too long to read; the name says enough.
    if (name.size() > 70) {
        name = name.substr(0, 67) + "...";
    }
    return std::format("{} ({})", name, library);
}

}  // namespace

bool available() {
    return true;
}

void start() {
    const std::lock_guard lock(g_mutex);
    if (g_running) {
        return;
    }
    install_handler();
    for (Slot& slot : g_slots) {
        if (!slot.pcs) {
            slot.pcs = std::make_unique<std::uintptr_t[]>(kSamples);
            slot.returns = std::make_unique<std::uintptr_t[]>(kSamples);
        }
        slot.count = 0;
        slot.left_out = false;
        slot.tid = 0;
        slot.name = nullptr;
    }
    ++g_epoch;
    g_running = true;
}

void note_thread(const char* name) {
    if (!g_running.load(std::memory_order_relaxed)) {
        return;
    }
    thread_local unsigned armed = 0;
    const unsigned epoch = g_epoch.load();
    if (armed == epoch) {
        return;
    }
    const std::lock_guard lock(g_mutex);
    armed = epoch;
    if (!g_running) {
        return;
    }
    const auto free =
        std::find_if(g_slots.begin(), g_slots.end(), [](const Slot& s) { return s.tid == 0; });
    clockid_t clock{};
    if (free == g_slots.end() || pthread_getcpuclockid(pthread_self(), &clock) != 0) {
        return;
    }
    sigevent event{};
    event.sigev_notify = SIGEV_THREAD_ID;
    event.sigev_signo = SIGPROF;
    event.sigev_notify_thread_id = thread_id();
    timer_t timer{};
    if (timer_create(clock, &event, &timer) != 0) {
        return;
    }
    free->name = name;
    free->timer = timer;
    free->tid = thread_id();  // the handler counts from here
    itimerspec every{};
    every.it_interval.tv_nsec = kIntervalNs;
    every.it_value.tv_nsec = kIntervalNs;
    timer_settime(timer, 0, &every, nullptr);
}

void leave_out(bool out) {
    if (!g_running.load(std::memory_order_relaxed)) {
        return;
    }
    const pid_t tid = thread_id();
    for (Slot& slot : g_slots) {
        if (slot.tid.load(std::memory_order_relaxed) == tid) {
            slot.left_out.store(out, std::memory_order_relaxed);
            return;
        }
    }
}

std::string stop() {
    const std::lock_guard lock(g_mutex);
    if (!g_running) {
        return {};
    }
    g_running = false;
    std::string out;
    std::unordered_map<std::uintptr_t, std::string> names;  // each address's function, once
    for (Slot& slot : g_slots) {
        if (slot.tid == 0) {
            continue;
        }
        timer_delete(slot.timer);
        const std::uint32_t taken = std::min(slot.count.load(), kSamples);
        const std::uint32_t lost = slot.count.load() - taken;
        slot.tid = 0;
        if (taken == 0) {
            continue;
        }
        auto name_of = [&names](std::uintptr_t pc) -> const std::string& {
            auto found = names.find(pc);
            if (found == names.end()) {
                found = names.emplace(pc, function_name(pc)).first;
            }
            return found->second;
        };
        // Each sample to its function's name; a function without a symbol (a library's memcpy, one
        // of our static functions) also by the function it returns to, which names the work.
        std::map<std::string, std::uint32_t> by_name;
        for (std::uint32_t i = 0; i < taken; ++i) {
            const std::string& name = name_of(slot.pcs[i]);
            if (slot.returns[i] != 0 && name.ends_with("without a symbol)")) {
                ++by_name[std::format("{} from {}", name, name_of(slot.returns[i]))];
            } else {
                ++by_name[name];
            }
        }
        std::vector<std::pair<std::string, std::uint32_t>> ranked(by_name.begin(), by_name.end());
        std::sort(ranked.begin(), ranked.end(), [](const auto& a, const auto& b) {
            return a.second > b.second;
        });
        if (out.empty()) {
            out += "\n## Hot functions (the processor time in each function itself, sampled once "
                   "a millisecond of the thread's own time)\n";
        }
        out += std::format(
            "\n{} thread, {} samples{}:\n\n```\n",
            slot.name != nullptr ? slot.name : "?",
            taken,
            lost > 0 ? std::format(" ({} more not kept)", lost) : std::string()
        );
        for (std::size_t k = 0; k < std::min(kShown, ranked.size()); ++k) {
            out += std::format(
                "{:>6.1f}%  {}\n",
                100.0 * static_cast<double>(ranked[k].second) / static_cast<double>(taken),
                ranked[k].first
            );
        }
        out += "```\n";
    }
    return out;
}

#else

bool available() {
    return false;
}

void start() {}

void note_thread(const char*) {}

void leave_out(bool) {}

std::string stop() {
    return {};
}

#endif

}  // namespace openrac::sampler
