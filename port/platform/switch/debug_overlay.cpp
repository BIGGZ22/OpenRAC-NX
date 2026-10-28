// SPDX-License-Identifier: GPL-3.0-or-later
// Desktop inspection tools are omitted from the initial Switch renderer.
#include "game/common/debug_overlay.h"
#include "common/log.h"
namespace openrac::frontend::debug_overlay {
void request() {}
bool visible() { return false; }
bool event(const SDL_Event&) { return false; }
void tick(const Tick&) {}
void begin(bool, bool, float, std::uint64_t) {}
void before_render(renderer::Renderer*) {}
void draw(const Shown&) {}
void pad(std::uint16_t, const std::uint8_t[4]) {}
bool holding() { return false; }
double tick_stretch() { return 1.0; }
bool drawer_available() { return false; }
void toggle_drawer() {}
bool touch_editing() { return false; }
bool take_touch_reset() { return false; }
void set_touch_guides(float, float) {}
bool take_settings_changed() { return false; }
void notice(const std::string& text) { if (!text.empty()) log::warn("{}", text); }
void release() {}
}
