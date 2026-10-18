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
#include <sys/sysinfo.h>
#include <sys/utsname.h>
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

#if !defined(_WIN32)
std::string uname_text() {
    utsname u{};
    if (uname(&u) != 0) {
        return {};
    }
    return std::format("{} {}, {}", u.sysname, u.release, u.machine);
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
#else
    std::string name = file_value("/etc/os-release", "PRETTY_NAME", '=');
    if (name.empty()) {
        name = "Linux";
    }
    return std::format("{} ({})", name, uname_text());
#endif
}

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
    name = file_value("/proc/cpuinfo", "model name", ':');
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
}

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
}

}  // namespace openrac::system_info
