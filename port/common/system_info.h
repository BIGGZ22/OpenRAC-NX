// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// What a report says of the computer: the system and its version, the processor, the memory. Each
// system has its own way; this file holds all three.

#pragma once

#include <string>

namespace openrac::system_info {

// "macOS 26.1 (Darwin 25.1.0, arm64)", "Ubuntu 24.04.1 LTS (Linux 6.8.0, x86_64)", "Windows 11 (10.0.26100, x86-64)".
std::string os();
// "Apple M3 Max, 16 threads".
std::string cpu();
// "36.0 GB".
std::string memory();

// The processor's clocks now, cores of the same clock and top clock together, where the system
// lets them be read (Linux, Android): "cpu0-3 1.80/1.80 GHz, cpu4-7 1.21/2.80 GHz" (now/top). A
// phone that is hot runs below its top. Empty elsewhere.
std::string cpu_clocks();

// The system's thermal state, where it says (Android 11 on): "none", "light", "moderate",
// "severe", ..., with the headroom (Android 12 on: 1.0 is where the system starts to throttle).
// Empty elsewhere.
std::string thermal();

}  // namespace openrac::system_info
