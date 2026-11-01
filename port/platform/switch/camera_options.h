// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <atomic>
#include <algorithm>
namespace openrac::platform::switch_camera {
// Published by the native camera/input hooks, before lock-strafe's own opt-out.
inline std::atomic<bool> gameplay{false};
inline float fov_scale(int percent, bool playing) {
    return playing ? float(std::clamp(percent, 85, 120)) / 100.0f : 1.0f;
}
}
