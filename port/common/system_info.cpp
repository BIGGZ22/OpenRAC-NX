// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

#include "common/system_info.h"

#include <cstdio>
#include <format>
#include <fstream>
#include <thread>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#pragma comment(lib, "advapi32.lib")
#elif defined(__APPLE__)
#include <sys/sysctl.h>
#include <sys/utsname.h>
#else
#if !defined(__SWITCH__)
#include <sys/sysinfo.h>
#endif
#if !defined(__SWITCH__)
#include <sys/utsname.h>
#endif
#endif
#if defined(__ANDROID__)
#include <cmath>
#include <dlfcn.h>

#include <sys/system_properties.h>
#endif
#if defined(__linux__)
#include <vector>
#endif

namespace openrac::system_info {
namespace {

#if defined(__APPLE__)
std::string sysctl_text(const char* name) {
    char buf[256] = {};
    std::size_t size = sizeof(buf) - 1;
    if (sysctlbyname(name, buf, &size, nullptr, 0) != 0) {
        return {};
    }
    return std::string(buf);
}
#endif

#if !defined(_WIN32) && !defined(__SWITCH__)
std::string uname_text() {
    utsname u{};
    if (uname(&u) != 0) {
        return {};
    }
    // As C strings: formatted as arrays, the fields kept their trailing NULs (Android, libc++).
    return std::format(
        "{} {}, {}",
        static_cast<const char*>(u.sysname),
        static_cast<const char*>(u.release),
        static_cast<const char*>(u.machine)
    );
}
#endif
#if defined(__SWITCH__)
std::string uname_text() {
    return "Nintendo Switch (Horizon)";
}
#endif

#if defined(__ANDROID__)
// A system property ("ro.build.version.release"), empty when the device has none.
std::string property(const char* name) {
    char buf[PROP_VALUE_MAX] = {};
    return __system_property_get(name, buf) > 0 ? std::string(buf) : std::string();
}
#endif

#if defined(__linux__)
// A value of a `key=value` (os-release) or `key : value` (cpuinfo) file.
std::string file_value(const char* path, const std::string& key, char between) {
    std::ifstream in(path);
    std::string line;
    while (std::getline(in, line)) {
        if (line.rfind(key, 0) != 0) {
            continue;
        }
        const auto at = line.find(between);
        if (at == std::string::npos) {
            continue;
        }
        std::string value = line.substr(at + 1);
        while (!value.empty() && (value.front() == ' ' || value.front() == '\t' || value.front() == '"')) {
            value.erase(value.begin());
        }
        while (!value.empty() && (value.back() == ' ' || value.back() == '"' || value.back() == '\r')) {
            value.pop_back();
        }
        return value;
    }
    return {};
}
#endif

std::string gigabytes(unsigned long long bytes) {
    return std::format("{:.1f} GB", static_cast<double>(bytes) / (1024.0 * 1024.0 * 1024.0));
}

}  // namespace

#ifndef __SWITCH__
std::string os() {
#if defined(_WIN32)
    // GetVersionEx answers what the program's manifest claims; ntdll's RtlGetVersion the truth.
    using RtlGetVersion = LONG(WINAPI*)(OSVERSIONINFOW*);
    OSVERSIONINFOW v{};
    v.dwOSVersionInfoSize = sizeof(v);
    if (HMODULE ntdll = GetModuleHandleW(L"ntdll.dll")) {
        if (auto get = reinterpret_cast<RtlGetVersion>(GetProcAddress(ntdll, "RtlGetVersion"))) {
            get(&v);
        }
    }
    const char* name = v.dwMajorVersion == 10 && v.dwBuildNumber >= 22000 ? "Windows 11" : "Windows";
    SYSTEM_INFO si{};
    GetNativeSystemInfo(&si);
    const char* arch = si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_ARM64 ? "arm64" : "x86-64";
    return std::format("{} ({}.{}.{}, {})", name, v.dwMajorVersion, v.dwMinorVersion, v.dwBuildNumber, arch);
#elif defined(__APPLE__)
    const std::string version = sysctl_text("kern.osproductversion");
    return std::format("macOS {} ({})", version.empty() ? "?" : version, uname_text());
#elif defined(__ANDROID__)
    // Android has no os-release: its version and the device, from the system's properties.
    const std::string device = property("ro.product.manufacturer") + " " + property("ro.product.model");
    return std::format("Android {} (API {}, {}; {})", property("ro.build.version.release"),
                       property("ro.build.version.sdk"), device, uname_text());
#else
    std::string name = file_value("/etc/os-release", "PRETTY_NAME", '=');
    if (name.empty()) {
        name = "Linux";
    }
    return std::format("{} ({})", name, uname_text());
#endif
#else
std::string os() {
    return "Nintendo Switch (Horizon)";
}
#endif

#ifndef __SWITCH__
std::string cpu() {
    std::string name;
#if defined(_WIN32)
    char buf[256] = {};
    DWORD size = sizeof(buf) - 1;
    if (RegGetValueA(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", "ProcessorNameString",
                     RRF_RT_REG_SZ, nullptr, buf, &size) == ERROR_SUCCESS) {
        name = buf;
    }
#elif defined(__APPLE__)
    name = sysctl_text("machdep.cpu.brand_string");
#else
#if defined(__ANDROID__)
    // The chip, where Android names it (12 on): "QTI SM8650".
    name = property("ro.soc.manufacturer");
    if (const std::string model = property("ro.soc.model"); !model.empty()) {
        name = name.empty() ? model : name + " " + model;
    }
    if (name.empty()) {
        name = file_value("/proc/cpuinfo", "Hardware", ':');
    }
#endif
    if (name.empty()) {
        name = file_value("/proc/cpuinfo", "model name", ':');
    }
    if (name.empty()) {
        name = file_value("/proc/cpuinfo", "Model", ':');  // Arm boards
    }
#endif
    while (!name.empty() && name.back() == ' ') {
        name.pop_back();
    }
    if (name.empty()) {
        name = "unknown processor";
    }
    return std::format("{}, {} threads", name, std::thread::hardware_concurrency());
#else
std::string cpu() {
    return "ARM Cortex-A57";
}
#endif

#ifndef __SWITCH__
std::string memory() {
#if defined(_WIN32)
    MEMORYSTATUSEX m{};
    m.dwLength = sizeof(m);
    if (GlobalMemoryStatusEx(&m)) {
        return gigabytes(m.ullTotalPhys);
    }
#elif defined(__APPLE__)
    unsigned long long bytes = 0;
    std::size_t size = sizeof(bytes);
    if (sysctlbyname("hw.memsize", &bytes, &size, nullptr, 0) == 0) {
        return gigabytes(bytes);
    }
#else
    struct sysinfo s{};
    if (sysinfo(&s) == 0) {
        return gigabytes(static_cast<unsigned long long>(s.totalram) * s.mem_unit);
    }
#endif
    return "unknown";
#else
std::string memory() {
    return "1GB";  // Nintendo Switch typically has 1GB RAM
}
#endif

std::string cpu_clocks() {
#if defined(__linux__)
    // A core's clock in kHz from its cpufreq folder; 0 when the system keeps it to itself.
    const auto khz = [](unsigned core, const char* file) {
        std::ifstream in(std::format("/sys/devices/system/cpu/cpu{}/cpufreq/{}", core, file));
        unsigned long long value = 0;
        in >> value;
        return value;
    };

    struct Group {
        unsigned first, last;
        unsigned long long now, top;
    };

    std::vector<Group> groups;
    const unsigned cores = std::thread::hardware_concurrency();
    for (unsigned core = 0; core < cores; ++core) {
        const unsigned long long now = khz(core, "scaling_cur_freq");
        const unsigned long long top = khz(core, "cpuinfo_max_freq");
        if (now == 0) {
            continue;
        }
        if (!groups.empty() && groups.back().last + 1 == core && groups.back().now == now
            && groups.back().top == top) {
            groups.back().last = core;
        } else {
            groups.push_back({core, core, now, top});
        }
    }
    std::string out;
    for (const Group& g : groups) {
        out += out.empty() ? "" : ", ";
        out += g.first == g.last ? std::format("cpu{}", g.first)
                                 : std::format("cpu{}-{}", g.first, g.last);
        out += std::format(
            " {:.2f}/{:.2f} GHz", static_cast<double>(g.now) / 1e6, static_cast<double>(g.top) / 1e6
        );
    }
    return out;
#else
    return {};
#endif
}

std::string thermal() {
#if defined(__ANDROID__)
    // The NDK's thermal manager (API 30, the headroom 31), looked up so that the port still runs
    // on Android 9.
    using Acquire = void* (*)();
    using Status = int (*)(void*);
    using Headroom = float (*)(void*, int);
    static void* const android = dlopen("libandroid.so", RTLD_NOW | RTLD_LOCAL);
    if (android == nullptr) {
        return {};
    }
    static const auto acquire =
        reinterpret_cast<Acquire>(dlsym(android, "AThermal_acquireManager"));
    static const auto status =
        reinterpret_cast<Status>(dlsym(android, "AThermal_getCurrentThermalStatus"));
    static const auto headroom =
        reinterpret_cast<Headroom>(dlsym(android, "AThermal_getThermalHeadroom"));
    static void* const manager = acquire != nullptr ? acquire() : nullptr;
    if (manager == nullptr || status == nullptr) {
        return {};
    }
    static constexpr const char* kStatus[] =
        {"none", "light", "moderate", "severe", "critical", "emergency", "shutdown"};
    const int now = status(manager);
    std::string out = now >= 0 && now < 7 ? kStatus[now] : "unknown";
    if (headroom != nullptr) {
        // Asked at most once a second, as the NDK asks: the capture asks at its start and end.
        if (const float h = headroom(manager, 0); !std::isnan(h)) {
            out += std::format(", headroom {:.2f}", h);
        }
    }
    return out;
#else
    return {};
#endif
}

}  // namespace openrac::system_info
