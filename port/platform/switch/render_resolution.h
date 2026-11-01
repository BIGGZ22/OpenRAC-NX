// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <algorithm>
#include <array>
#include <cmath>

namespace openrac::platform::switch_video {
struct Resolution {
    int width, height;
    const char* label;
};
// Internal game targets, not HDMI/display modes. Zero retains the saved picture shape.
inline constexpr std::array kResolutions = {
    Resolution{0, 0, "Auto (480p minimum)"},
    Resolution{640, 480, "640x480 (4:3)"},
    Resolution{854, 480, "854x480 (16:9)"},
    Resolution{960, 540, "960x540 (16:9)"},
    Resolution{1280, 720, "1280x720 (16:9)"},
    Resolution{1024, 768, "1024x768 (4:3)"},
    Resolution{1920, 1080, "1920x1080 (16:9)"}
};
inline const Resolution& resolution(int index) {
    return kResolutions[std::clamp(index, 0, int(kResolutions.size()) - 1)];
}
inline int profile_resolution(int common, int handheld, int docked, bool is_docked) {
    const int selected = is_docked ? docked : handheld;
    return std::clamp(selected < 0 ? common : selected, 0, int(kResolutions.size()) - 1);
}
inline void target_size(int index, int& width, int& height) {
    const auto& selected = resolution(index);
    if (selected.height != 0) {
        width = selected.width;
        height = selected.height;
    } else if (height < 480) {
        width = std::max(640, int(std::lround(double(std::max(width, 1)) * 480 / std::max(height, 1))));
        height = 480;
    } else {
        width = std::max(640, width);
    }
}
}  // namespace openrac::platform::switch_video
