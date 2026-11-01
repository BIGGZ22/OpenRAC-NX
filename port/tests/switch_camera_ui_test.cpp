// SPDX-License-Identifier: GPL-3.0-or-later
#include "check.h"
#include "platform/switch/launcher_ui.h"
#include "platform/switch/camera_options.h"
#include <cmath>
int main() {
    using namespace openrac::platform;
    int x = 640, y = 360;
    CHECK(launcher_ui::menu_touch(x, y) && x == 640 && y == 360);
    x = 0; y = 0;
    CHECK(launcher_ui::menu_touch(x, y) && x == 0 && y == 0);
    x = launcher_ui::kMenuX; y = launcher_ui::kMenuY;
    CHECK(launcher_ui::menu_touch(x, y) && x == 0 && y == 0);
    x = launcher_ui::kMenuX + launcher_ui::kMenuWidth; y = 360;
    CHECK(!launcher_ui::menu_touch(x, y));
    // Transform a logical About-page touch into the safe area and back.
    x = launcher_ui::kMenuX + 50 * launcher_ui::kMenuWidth / launcher_ui::kWidth;
    y = launcher_ui::kMenuY + 582 * launcher_ui::kMenuHeight / launcher_ui::kHeight;
    CHECK(launcher_ui::menu_touch(x, y));
    CHECK(x >= 48 && x <= 50 && y >= 580 && y <= 582);
    CHECK(switch_camera::fov_scale(120, false) == 1.0f);
    CHECK(switch_camera::fov_scale(100, true) == 1.0f);
    CHECK(std::abs(switch_camera::fov_scale(999, true) - 1.2f) < .0001f);
    CHECK(std::abs(switch_camera::fov_scale(0, true) - .85f) < .0001f);
    return openrac::test::result();
}
