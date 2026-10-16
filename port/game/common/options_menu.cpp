// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

#include "options_menu.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <format>
#include <iterator>
#include <string>
#include <vector>

#include "common/log.h"
#include "openrac/game_host.h"
#include "openrac/guest.h"
#include "platform/window.h"
#include "renderer/framebuffer.h"
#include "renderer/texture_pool.h"
#include "settings.h"

namespace openrac::frontend::options_menu {

namespace {

// ---- The rows: one per setting, whatever game shows them ----

enum Row {
    kDisplayMode,
    kMonitor,
    kResolution,
    kAspect,
    kHud,
    kRenderScale,
    kMsaa,
    kAnisotropy,
    kVSync,
    kFrameRate,
    kFrameCap,
    kShowFps,
    kMuteUnfocused,
    kSkipCutscenes,
    kRows
};
constexpr const char* kLabels[kRows] = {
    "Display mode", "Monitor",   "Resolution", "Picture",       "HUD",           "Render scale", "Anti-aliasing",
    "Texture filter", "VSync",   "Frame rate", "Frame cap",     "Frame counter", "Mute in background",
    "Skip cutscenes",
};

// The value after `current` in a list (before it, for by = -1), around the ends; the first when
// `current` is not one of them.
template <typename T, std::size_t N>
T stepped(const T (&list)[N], T current, int by, std::size_t count = N) {
    count = std::min(count, N);
    for (std::size_t i = 0; i < count; ++i) {
        if (list[i] == current) {
            return list[(i + count + static_cast<std::size_t>(by + static_cast<int>(count))) % count];
        }
    }
    return list[0];
}

template <typename E>
E stepped_enum(E current, int by) {
    const int count = static_cast<int>(E::Count);
    return static_cast<E>((static_cast<int>(current) + by + count) % count);
}

// How many of a list's values the graphics card offers (those up to `most`).
template <std::size_t N>
std::size_t offered(const int (&list)[N], int most) {
    std::size_t count = 1;
    while (count < N && list[count] <= most) {
        ++count;
    }
    return count;
}

std::string value_of(int row) {
    const settings::Settings& s = settings::get();
    switch (row) {
        case kDisplayMode:
            return settings::name(s.display_mode);
        case kMonitor: {
            if (s.monitor == 0) {
                return "Window's own";
            }
            const auto all = platform::displays();
            if (static_cast<std::size_t>(s.monitor) > all.size()) {
                return std::format("{} (not connected)", s.monitor);
            }
            const platform::DisplayInfo& d = all[static_cast<std::size_t>(s.monitor - 1)];
            return std::format("{}: {}x{} {:.0f} Hz", s.monitor, d.width, d.height, d.refresh_rate);
        }
        case kResolution:
            // Only the fullscreen display mode changes the display's own.
            return s.fullscreen_width > 0 && s.fullscreen_height > 0
                       ? std::format("{}x{}", s.fullscreen_width, s.fullscreen_height)
                       : std::string("Desktop");
        case kAspect:
            return s.aspect == settings::Aspect::Custom
                       ? std::format("Custom {}:{}", s.aspect_custom_x, s.aspect_custom_y)
                       : std::string(settings::name(s.aspect));
        case kHud:
            return settings::name(s.hud);
        case kRenderScale:
            // As a factor: the game's font draws a pad button for the percent sign.
            return s.render_scale % 100 == 0 ? std::format("{}x", s.render_scale / 100)
                                             : std::format("{:.2f}x", static_cast<float>(s.render_scale) / 100.0f);
        case kMsaa:
            return s.msaa > 1 ? std::format("{}x", s.msaa) : std::string("Off");
        case kAnisotropy:
            return s.anisotropy > 1 ? std::format("Anisotropic {}x", s.anisotropy) : std::string("Trilinear");
        case kVSync:
            return settings::name(s.vsync);
        case kFrameRate:
            return settings::frame_rate_name(s);
        case kFrameCap:
            return s.frame_rate == settings::FrameRate::Capped ? std::to_string(s.frame_cap) : std::string("Off");
        case kShowFps:
            return s.show_fps ? "On" : "Off";
        case kMuteUnfocused:
            return s.mute_unfocused ? "On" : "Off";
        case kSkipCutscenes:
            return s.skip_cutscenes ? "Start" : "Off";
        default:
            return {};
    }
}

// A row moved to its next value (by = 1: Cross or Right) or its last one (by = -1: Left).
void step_value(int row, int by) {
    settings::Settings& s = settings::get();
    switch (row) {
        case kDisplayMode:
            s.display_mode = stepped_enum(s.display_mode, by);
            break;
        case kMonitor: {
            // 0 (the display the window is on), then each display connected.
            const int count = static_cast<int>(platform::displays().size()) + 1;
            s.monitor = ((s.monitor < count ? s.monitor : 0) + by + count) % count;
            // Another display has other modes: back to its desktop's.
            s.fullscreen_width = s.fullscreen_height = 0;
            break;
        }
        case kResolution: {
            // The desktop's own mode, then the display's sizes, largest first.
            std::vector<platform::DisplaySize> sizes = platform::fullscreen_sizes(s.monitor);
            sizes.insert(sizes.begin(), platform::DisplaySize{0, 0});
            const platform::DisplaySize now{s.fullscreen_width, s.fullscreen_height};
            const auto at = std::find(sizes.begin(), sizes.end(), now);
            const std::size_t index = at == sizes.end() ? 0 : static_cast<std::size_t>(at - sizes.begin());
            const platform::DisplaySize next =
                sizes[(index + sizes.size() + static_cast<std::size_t>(by + static_cast<int>(sizes.size())))
                      % sizes.size()];
            s.fullscreen_width = next.width;
            s.fullscreen_height = next.height;
            break;
        }
        case kAspect:
            s.aspect = stepped_enum(s.aspect, by);
            break;
        case kHud:
            s.hud = stepped_enum(s.hud, by);
            break;
        case kRenderScale:
            s.render_scale = stepped(settings::kRenderScales, s.render_scale, by);
            break;
        case kMsaa:
            s.msaa = stepped(settings::kSampleCounts, s.msaa, by,
                             offered(settings::kSampleCounts, renderer::FrameBuffer::max_samples()));
            break;
        case kAnisotropy:
            s.anisotropy = stepped(settings::kAnisotropies, s.anisotropy, by,
                                   offered(settings::kAnisotropies,
                                           static_cast<int>(renderer::TexturePool::max_anisotropy())));
            break;
        case kVSync:
            s.vsync = stepped_enum(s.vsync, by);
            break;
        case kFrameRate: {
            // Original, Display, the caps, Unlimited: one list, the cap's number its place in it.
            constexpr int kCaps = static_cast<int>(std::size(settings::kFrameCaps));
            const int count = kCaps + 3;
            int at = 1;
            if (s.frame_rate == settings::FrameRate::Original) {
                at = 0;
            } else if (s.frame_rate == settings::FrameRate::Unlimited) {
                at = count - 1;
            } else if (s.frame_rate == settings::FrameRate::Capped) {
                // A cap of the player's own between two of the list's: from the one below it.
                at = 2;
                while (at - 2 + 1 < kCaps && settings::kFrameCaps[at - 2 + 1] <= s.frame_cap) {
                    ++at;
                }
                if (by < 0 && settings::kFrameCaps[at - 2] != s.frame_cap && s.frame_cap > settings::kFrameCaps[0]) {
                    ++at;  // going down from between two: to the one below, not past it
                }
            }
            at = (at + by + count) % count;
            if (at == 0) {
                s.frame_rate = settings::FrameRate::Original;
            } else if (at == 1) {
                s.frame_rate = settings::FrameRate::Display;
            } else if (at == count - 1) {
                s.frame_rate = settings::FrameRate::Unlimited;
            } else {
                s.frame_rate = settings::FrameRate::Capped;
                s.frame_cap = settings::kFrameCaps[at - 2];
            }
            break;
        }
        case kFrameCap:
            // A cap of the player's own, five frames a second at a time (Cross: one).
            if (s.frame_rate != settings::FrameRate::Capped) {
                s.frame_rate = settings::FrameRate::Capped;
            } else {
                s.frame_cap = std::clamp(s.frame_cap + by * 5, settings::kFrameCapMin, settings::kFrameCapMax);
            }
            break;
        case kShowFps:
            s.show_fps = !s.show_fps;
            break;
        case kMuteUnfocused:
            s.mute_unfocused = !s.mute_unfocused;
            break;
        case kSkipCutscenes:
            s.skip_cutscenes = !s.skip_cutscenes;
            break;
        default:
            break;
    }
}

/*
 * Ratchet & Clank (PAL): the front end's Video options page.
 *
 * A page is data in the boot program (debug_menu.cpp describes it: seq[14], the frame objects'
 * animations; parent; kind; focus; widget[14]). The Video page's list is the game's row list
 * (update func_00221E60, draw func_00222070): rows of 0x14 bytes {label text, a byte's address,
 * the text shown while the byte is set, the text shown while it is clear, flags}, ended by a row
 * without a label. Cross on a row flips its byte (a row with flag 1 is the game's own video mode,
 * which it handles itself).
 *
 * The port's rows take that list's place, with texts of the port's own (ids the game's table
 * does not have: the text lookup is wrapped, host/options.c) and a byte each, which update() reads
 * as "Cross was pressed here": the setting moves to its next value and the byte is cleared. Both
 * of a row's value texts are the same id, whose string is the setting's value now. Left and Right
 * are read from the pad as the game's menus read it, and move the row under the cursor back and on.
 *
 * The game's own row is not among them. It was "Video mode", 50 Hz or 60 Hz: the PAL game's choice
 * of the television's field rate (it sets D_0016044C, which the title loop func_001EBB48 makes the
 * video mode, sceGsResetGraph's 3 or 2, and the next level's time base, func_002157C0). The port's
 * window shows the game at the display's own rate whatever the game's tick is, so the choice means
 * nothing here, and the port keeps the PAL game in its 60 Hz mode (update(); main.cpp's tick
 * rate is 60 for every version).
 *
 * The page's frame is too low for all the rows, so the list the game is given is a window of
 * kShown rows onto them, which moves with the cursor: at the window's last row with more below, the
 * rows move up one and the cursor stays on the row it reached, and the same at the top.
 *
 * The page's frame for one row is too low for kShown, so the page borrows the Sound options page's
 * frame animations for its list and its hint (seq 0xD2 and 0xD7: three sliders' worth) and that
 * page's hint widget ("Toggle", "Exit").
 */
namespace pal {

constexpr gaddr kMenu = 0x001D5F70u;        // the page menu: +4 the current page
constexpr gaddr kPad = 0x0013CA40u;         // the first pad: +0x1C4 the buttons pressed this tick
constexpr gaddr kVideoPage = 0x001D4E38u;
constexpr gaddr kVideoList = 0x001D4F40u;
constexpr gaddr kVideoRows = 0x001D4F18u;   // the game's one row and its end
constexpr gaddr kSoundHint = 0x001CE720u;   // the Sound page's hint widget
constexpr std::int32_t kSoundListSeq = 0xD2;
constexpr std::int32_t kSoundHintSeq = 0xD7;
constexpr gaddr kRowBytes = 0x14;
// The pad's buttons as the game's menus test them (the pad's two bytes the other way round).
constexpr std::uint32_t kPadLeft = 0x8000;
constexpr std::uint32_t kPadRight = 0x2000;

// The port's text ids: past the game's last (its table's ids are under 0x7000).
constexpr int kTextFirst = 0x7E00;
constexpr int kShown = 4;  // the rows the page's frame holds
constexpr gaddr kTextBytes = 48;

struct Native {
    gaddr rows = 0;    // the kShown rows the game is given, and the end
    gaddr flags = 0;   // a byte per row shown
    gaddr labels = 0;  // kShown strings of kTextBytes
    gaddr values = 0;  // kShown strings of kTextBytes
    int top = 0;       // the first row shown
};
Native n;

void put_text(gaddr at, const std::string& text) {
    char* p = static_cast<char*>(G(at));
    std::snprintf(p, kTextBytes, "%s", text.c_str());
}

void build() {
    if (n.rows != 0) {
        return;
    }
    n.rows = openrac_guest_static(kRowBytes * (kShown + 1));
    n.flags = openrac_guest_static(8);
    n.labels = openrac_guest_static(kTextBytes * kShown);
    n.values = openrac_guest_static(kTextBytes * kShown);
    std::memset(G(n.rows), 0, kRowBytes * (kShown + 1));
    std::memset(G(n.flags), 0, 8);
    for (int i = 0; i < kShown; ++i) {
        const gaddr row = n.rows + kRowBytes * static_cast<gaddr>(i);
        GREF(std::int32_t, row + 0x00) = kTextFirst + i;
        GREF(std::uint32_t, row + 0x04) = n.flags + static_cast<gaddr>(i);
        GREF(std::int32_t, row + 0x08) = kTextFirst + kShown + i;
        GREF(std::int32_t, row + 0x0C) = kTextFirst + kShown + i;
    }
}

bool update() {
    build();
    if (GREF(std::uint32_t, kVideoList + 0x34) == kVideoRows) {
        GREF(std::uint32_t, kVideoList + 0x34) = n.rows;
        GREF(std::int32_t, kVideoList + 0x38) = 0;
        n.top = 0;
        GREF(std::int32_t, kVideoPage + 4 * 3) = kSoundListSeq;
        GREF(std::int32_t, kVideoPage + 4 * 4) = kSoundHintSeq;
        GREF(std::uint32_t, kVideoPage + 0x44 + 4 * 4) = kSoundHint;
    }
    // The cursor, as the game moved it this tick.
    std::int32_t& cursor = GREF(std::int32_t, kVideoList + 0x38);
    cursor = std::clamp(cursor, 0, kShown - 1);
    bool changed = false;
    for (int i = 0; i < kShown; ++i) {
        std::uint8_t& pressed = GREF(std::uint8_t, n.flags + static_cast<gaddr>(i));
        if (pressed != 0) {
            pressed = 0;
            step_value(n.top + i, 1);
            changed = true;
        }
    }
    // Left and Right, while the page is open with its list in hand.
    const gaddr page = GREF(std::uint32_t, kMenu + 4);
    if (page == kVideoPage && GREF(std::uint32_t, page + 0x40) == kVideoList
        && GREF(std::int32_t, kVideoList + 0x3C) == 0) {
        const std::uint32_t pad = GREF(std::uint32_t, kPad + 0x1C4);
        if ((pad & (kPadLeft | kPadRight)) != 0) {
            step_value(n.top + cursor, (pad & kPadRight) != 0 ? 1 : -1);
            changed = true;
        }
    }
    // The window follows the cursor: a row short of each end while there are more that way.
    if (cursor == kShown - 1 && n.top + kShown < kRows) {
        ++n.top;
        --cursor;
    } else if (cursor == 0 && n.top > 0) {
        --n.top;
        ++cursor;
    }
    for (int i = 0; i < kShown; ++i) {
        put_text(n.labels + kTextBytes * static_cast<gaddr>(i), kLabels[n.top + i]);
        put_text(n.values + kTextBytes * static_cast<gaddr>(i), value_of(n.top + i));
    }
    if (changed) {
        settings::save();
    }
    return changed;
}

std::uint32_t text(int id) {
    if (n.rows == 0 || id < kTextFirst || id >= kTextFirst + 2 * kShown) {
        return 0;
    }
    const int i = id - kTextFirst;
    return i < kShown ? n.labels + kTextBytes * static_cast<gaddr>(i)
                      : n.values + kTextBytes * static_cast<gaddr>(i - kShown);
}

}  // namespace pal

bool is_pal(const char* game_id) {
    return game_id != nullptr && std::strcmp(game_id, "rac1-pal") == 0;
}

bool g_pal = false;

}  // namespace

bool update(const char* game_id, int loaded_overlay) {
    g_pal = is_pal(game_id);
    if (g_pal) {
        // The port runs every version at 60 Hz: the PAL game is kept in its own 60 Hz mode
        // (D_0016044C 0; nonzero is 50 Hz), which its title loop makes the video mode and the
        // next level's time base, as choosing 60 Hz in its menu would.
        GREF(std::uint8_t, 0x0016044Cu) = 0;
    }
    if (!g_pal || loaded_overlay != OPENRAC_OVERLAY_EXE) {
        return false;
    }
    return pal::update();
}

std::uint32_t text(int id) {
    return g_pal ? pal::text(id) : 0;
}

}  // namespace openrac::frontend::options_menu
