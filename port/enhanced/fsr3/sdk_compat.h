// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Standard includes and bounded CRT equivalents used only while compiling AMD's SDK.
#pragma once
#include <algorithm>
#include <bit>
#include <cfloat>
#include <climits>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cwchar>
#include <iterator>
#include <locale>
#include <volk.h>

#include <FidelityFX/host/ffx_util.h>

#if !defined(_WIN32)
// Matches the SDK's bounded diagnostic-name copies without depending on a Windows CRT.
inline int wcscpy_s(wchar_t* destination, std::size_t capacity, const wchar_t* source) {
    const auto length = std::wcslen(source);

    // Leave no partial name on a failed bounds check.
    if (length >= capacity) {
        return 1;
    }

    std::wmemcpy(destination, source, length + 1);
    return 0;
}

// Infers capacity for SDK fixed-size resource labels.
template <std::size_t Capacity>
int wcscpy_s(wchar_t (&destination)[Capacity], const wchar_t* source) {
    return wcscpy_s(destination, Capacity, source);
}

// Copies narrow diagnostic names with the same bounded contract.
inline int strcpy_s(char* destination, std::size_t capacity, const char* source) {
    const auto length = std::strlen(source);

    // Reject names that do not leave space for a terminator.
    if (length >= capacity) {
        return 1;
    }

    std::memcpy(destination, source, length + 1);
    return 0;
}

// The SDK uses its Windows spelling for compile-time array lengths.
#define _countof(array) std::size(array)

// Formats SDK breadcrumb numbers within the supplied destination capacity.
template <typename... Args>
int sprintf_s(char* destination, std::size_t capacity, const char* format, Args... args) {
    return std::snprintf(destination, capacity, format, args...);
}
#endif
