// SPDX-License-Identifier: GPL-3.0-or-later
#include "check.h"
#include "platform/switch/render_resolution.h"

int main() {
    using namespace openrac::platform::switch_video;
    for (int preset = 1; preset < int(kResolutions.size()); ++preset) {
        int width = 1, height = 1;
        target_size(preset, width, height);
        CHECK(width == kResolutions[preset].width);
        CHECK(height == kResolutions[preset].height);
        CHECK(width >= 640 && height >= 480);
    }
    // Old 50% settings must no longer yield a sub-480p Switch target.
    int width = 640, height = 360;
    target_size(0, width, height);
    CHECK(width == 853 && height == 480);
    width = 480; height = 360;
    target_size(0, width, height);
    CHECK(width == 640 && height == 480);
    width = 1280; height = 720;
    target_size(0, width, height);
    CHECK(width == 1280 && height == 720);
    width = 0; height = 0;
    target_size(0, width, height);
    CHECK(width >= 640 && height == 480);
    CHECK(resolution(-1).height == 0);
    CHECK(resolution(999).height == 1080);
    CHECK(profile_resolution(4, -1, -1, false) == 4);
    CHECK(profile_resolution(4, 2, 6, false) == 2);
    CHECK(profile_resolution(4, 2, 6, true) == 6);
    CHECK(profile_resolution(4, 0, -1, false) == 0);
    CHECK(profile_resolution(4, 2, -1, true) == 4);
    return openrac::test::result();
}
