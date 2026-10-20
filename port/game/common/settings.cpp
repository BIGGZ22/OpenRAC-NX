// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

#include "settings.h"

#include <algorithm>
#include <atomic>
#include <cstdio>
#include <cstring>
#include <format>
#include <fstream>
#include <system_error>

#include "common/log.h"
#include "fixes.h"

namespace openrac::settings {

namespace {

Settings g_settings;
std::atomic<float> g_gain{1.0f};  // g_settings.volume, read by the sound's thread
std::atomic<bool> g_muted{false};
std::filesystem::path g_file;

// The names in the file, by value.
constexpr const char* kDisplayMode[] = {"windowed", "borderless", "fullscreen"};
constexpr const char* kAspect[] = {"4:3", "16:10", "16:9", "21:9", "32:9", "window", "custom"};
constexpr const char* kHud[] = {"centre", "edges"};
constexpr const char* kVSync[] = {"off", "on", "adaptive"};
constexpr const char* kStrafe[] = {"off", "hold", "lock"};

std::string trimmed(const std::string& s) {
    const auto first = s.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
        return {};
    }
    return s.substr(first, s.find_last_not_of(" \t\r\n") - first + 1);
}

template <typename E, std::size_t N>
bool parse(const std::string& value, const char* const (&names)[N], E& out) {
    for (std::size_t i = 0; i < N; ++i) {
        if (value == names[i]) {
            out = static_cast<E>(i);
            return true;
        }
    }
    return false;
}

bool parse_bool(const std::string& value, bool& out) {
    if (value == "1" || value == "true" || value == "on" || value == "yes") {
        out = true;
        return true;
    }
    if (value == "0" || value == "false" || value == "off" || value == "no") {
        out = false;
        return true;
    }
    return false;
}

// A whole number within [low, high], with nothing after it.
bool parse_int(const std::string& value, int low, int high, int& out) {
    int number = 0;
    char after = 0;
    if (std::sscanf(value.c_str(), "%d%c", &number, &after) != 1 || number < low || number > high) {
        return false;
    }
    out = number;
    return true;
}

// Two whole numbers with `between` between them ("1920x1080", "64:27"), each at least 1.
bool parse_pair(const std::string& value, char between, int& a, int& b) {
    int first = 0;
    int second = 0;
    char sep = 0;
    char after = 0;
    if (std::sscanf(value.c_str(), "%d%c%d%c", &first, &sep, &second, &after) != 3 || sep != between || first < 1
        || second < 1 || first > 100000 || second > 100000) {
        return false;
    }
    a = first;
    b = second;
    return true;
}

// One of a list's numbers ("off" is 1: a sample a pixel, no anisotropy).
template <std::size_t N>
bool parse_listed(const std::string& value, const int (&list)[N], int& out) {
    int number = 0;
    if (value == "off") {
        number = 1;
    } else if (!parse_int(value, 0, 1000, number)) {
        return false;
    }
    number = std::max(number, 1);  // 0 is "off" too
    if (std::find(std::begin(list), std::end(list), number) == std::end(list)) {
        return false;
    }
    out = number;
    return true;
}

// frame_rate: a name, or a number of frames a second (any within the limits, not only the ones the
// options page offers: a display's own rate, say).
bool parse_frame_rate(const std::string& value, Settings& s) {
    if (value == "original") {
        s.frame_rate = FrameRate::Original;
    } else if (value == "display") {
        s.frame_rate = FrameRate::Display;
    } else if (value == "unlimited") {
        s.frame_rate = FrameRate::Unlimited;
    } else if (parse_int(value, kFrameCapMin, kFrameCapMax, s.frame_cap)) {
        s.frame_rate = FrameRate::Capped;
    } else {
        return false;
    }
    return true;
}

std::string frame_rate_value(const Settings& s) {
    switch (s.frame_rate) {
        case FrameRate::Original:
            return "original";
        case FrameRate::Capped:
            return std::to_string(s.frame_cap);
        case FrameRate::Unlimited:
            return "unlimited";
        case FrameRate::Display:
        case FrameRate::Count:
            break;
    }
    return "display";
}

// The game fixes switched off, for the summary: ", as the console: fix_a, fix_b", or nothing.
std::string fixes_off() {
    std::string off;
    for (int i = 0; i < fixes::count(); ++i) {
        if (!fixes::on(i)) {
            off += (off.empty() ? ", as the console: " : ", ") + std::string(fixes::fix(i).key);
        }
    }
    return off;
}

float gain_now() {
    return g_muted ? 0.0f : static_cast<float>(g_settings.volume) / 100.0f;
}

}  // namespace

Settings& get() {
    return g_settings;
}

void load(const std::filesystem::path& file) {
    g_file = file;
    g_settings = Settings{};
    fixes::reset();
    Settings& s = g_settings;
    std::ifstream in(file);
    // A file from before `vsync` was a key of its own: a frame waited for the display only at the
    // "display" frame rate, and so it does until the player says otherwise.
    bool has_vsync = false;
    bool has_frame_rate = false;
    // A file from before `strafe`: `lock_strafe = 1` was the strafe while firing, now Lock. A file
    // with both is the newer one's.
    bool has_strafe = false;
    bool old_lock_strafe = false;
    std::string line;
    while (in && std::getline(in, line)) {
        const auto eq = line.find('=');
        if (eq == std::string::npos || trimmed(line).starts_with('#')) {
            continue;
        }
        const std::string key = trimmed(line.substr(0, eq));
        const std::string value = trimmed(line.substr(eq + 1));
        bool known = true;
        if (key == "display_mode") {
            known = parse(value, kDisplayMode, s.display_mode);
        } else if (key == "fullscreen") {
            // The key display_mode replaced: a window covering the desktop, or not. A file that
            // has both is the newer one's.
            bool on = false;
            known = parse_bool(value, on);
            if (known && on && s.display_mode == DisplayMode::Windowed) {
                s.display_mode = DisplayMode::Borderless;
            }
        } else if (key == "monitor") {
            known = parse_int(value, 0, 64, s.monitor);
        } else if (key == "resolution") {
            if (value == "desktop") {
                s.fullscreen_width = s.fullscreen_height = 0;
            } else {
                known = parse_pair(value, 'x', s.fullscreen_width, s.fullscreen_height);
            }
        } else if (key == "window_size") {
            known = parse_pair(value, 'x', s.window_width, s.window_height);
        } else if (key == "aspect") {
            known = parse(value, kAspect, s.aspect);
        } else if (key == "aspect_custom") {
            known = parse_pair(value, ':', s.aspect_custom_x, s.aspect_custom_y);
        } else if (key == "hud") {
            known = parse(value, kHud, s.hud);
        } else if (key == "render_scale") {
            known = parse_int(value, 25, 400, s.render_scale);
        } else if (key == "msaa") {
            known = parse_listed(value, kSampleCounts, s.msaa);
        } else if (key == "anisotropy") {
            known = parse_listed(value, kAnisotropies, s.anisotropy);
        } else if (key == "vsync") {
            known = parse(value, kVSync, s.vsync);
            has_vsync = has_vsync || known;
        } else if (key == "frame_rate") {
            known = parse_frame_rate(value, s);
            has_frame_rate = has_frame_rate || known;
        } else if (key == "show_fps") {
            known = parse_bool(value, s.show_fps);
        } else if (key == "menu_bar") {
            known = parse_bool(value, s.menu_bar);
        } else if (key == "mute_unfocused") {
            known = parse_bool(value, s.mute_unfocused);
        } else if (key == "skip_cutscenes") {
            known = parse_bool(value, s.skip_cutscenes);
        } else if (key == "strafe") {
            known = parse(value, kStrafe, s.strafe);
            has_strafe = has_strafe || known;
        } else if (key == "lock_strafe") {
            known = parse_bool(value, old_lock_strafe);
        } else if (key == "volume") {
            known = parse_int(value, 0, 100, s.volume);
        } else if (key == "verbose_log") {
            known = parse_bool(value, s.verbose_log);
        } else if (const int fix = fixes::find(key.c_str()); fix >= 0) {
            // A game fix (fixes.h): on is the port's, off the console's behaviour.
            bool on = true;
            known = parse_bool(value, on);
            if (known) {
                fixes::set(fix, on);
            }
        }
        if (!known) {
            log::warn("{}: {} = {} is not a value the port knows; the default is used", file.string(), key, value);
        }
    }
    if (has_frame_rate && !has_vsync) {
        s.vsync = s.frame_rate == FrameRate::Display ? VSync::On : VSync::Off;
    }
    if (!has_strafe && old_lock_strafe) {
        s.strafe = Strafe::Lock;
    }
    g_gain = gain_now();
    log::info("settings from {}: {}", file.string(), summary());
}

std::string summary() {
    const Settings& s = g_settings;
    const std::string window = s.window_width > 0 && s.window_height > 0
                                   ? std::format("{}x{}", s.window_width, s.window_height)
                                   : std::string("default size");
    return std::format("{} ({}, display {}), picture {}, {}% scale, {} sample(s), anisotropy {}, frames {}, vsync {}, "
                       "volume {}{}{}{}",
                       kDisplayMode[static_cast<int>(s.display_mode)], window, s.monitor,
                       kAspect[static_cast<int>(s.aspect)], s.render_scale, s.msaa, s.anisotropy, frame_rate_value(s),
                       kVSync[static_cast<int>(s.vsync)], s.volume,
                       s.strafe == Strafe::Off ? std::string() : std::format(", strafe {}", kStrafe[static_cast<int>(s.strafe)]),
                       s.verbose_log ? ", detailed log" : "", fixes_off());
}

void save() {
    if (g_file.empty()) {
        return;
    }
    std::error_code made;
    std::filesystem::create_directories(g_file.parent_path(), made);
    // Through a file beside it, then over the old one: a stop mid-write (Android ends a game in
    // the background) leaves the settings as they were, not an empty file.
    std::filesystem::path written = g_file;
    written += ".new";
    std::ofstream out(written, std::ios::trunc);
    if (!out) {
        log::warn("the settings could not be written to {}", g_file.string());
        return;
    }
    const Settings& s = g_settings;
    out << "# OpenRAC: the port's settings (the Video options page of the game's menu changes them;\n"
        << "# docs/port/SETTINGS.md says what each is)\n"
        << "display_mode = " << kDisplayMode[static_cast<int>(s.display_mode)] << "\n"
        << "monitor = " << s.monitor << "\n"
        << "resolution = "
        << (s.fullscreen_width > 0 && s.fullscreen_height > 0
                ? std::format("{}x{}", s.fullscreen_width, s.fullscreen_height)
                : std::string("desktop"))
        << "\n";
    if (s.window_width > 0 && s.window_height > 0) {
        out << "window_size = " << s.window_width << "x" << s.window_height << "\n";
    }
    out << "aspect = " << kAspect[static_cast<int>(s.aspect)] << "\n"
        << "aspect_custom = " << s.aspect_custom_x << ":" << s.aspect_custom_y << "\n"
        << "hud = " << kHud[static_cast<int>(s.hud)] << "\n"
        << "render_scale = " << s.render_scale << "\n"
        << "msaa = " << s.msaa << "\n"
        << "anisotropy = " << s.anisotropy << "\n"
        << "vsync = " << kVSync[static_cast<int>(s.vsync)] << "\n"
        << "frame_rate = " << frame_rate_value(s) << "\n"
        << "show_fps = " << (s.show_fps ? 1 : 0) << "\n"
        << "menu_bar = " << (s.menu_bar ? 1 : 0) << "\n"
        << "mute_unfocused = " << (s.mute_unfocused ? 1 : 0) << "\n"
        << "skip_cutscenes = " << (s.skip_cutscenes ? 1 : 0) << "\n"
        << "strafe = " << kStrafe[static_cast<int>(s.strafe)] << "\n"
        << "volume = " << s.volume << "\n";
    if (s.verbose_log) {
        out << "verbose_log = 1\n";
    }
    if (fixes::count() > 0) {
        out << "# Game fixes: 1 the port's, 0 the console's behaviour (Options, Game fixes)\n";
        for (int i = 0; i < fixes::count(); ++i) {
            out << fixes::fix(i).key << " = " << (fixes::on(i) ? 1 : 0) << "\n";
        }
    }
    out.close();
    std::error_code moved;
    if (!out.fail()) {
        std::filesystem::rename(written, g_file, moved);
    }
    if (out.fail() || moved) {
        log::warn("the settings could not be written to {}", g_file.string());
        std::filesystem::remove(written, moved);
    }
}

void set_volume(int volume) {
    g_settings.volume = std::clamp(volume, 0, 100);
    g_gain = gain_now();
}

void set_muted(bool muted) {
    g_muted = muted;
    g_gain = gain_now();
}

float output_gain() {
    return g_gain;
}

float aspect_ratio(const Settings& settings, int window_width, int window_height) {
    switch (settings.aspect) {
        case Aspect::Wide16x10:
            return 16.0f / 10.0f;
        case Aspect::Wide16x9:
            return 16.0f / 9.0f;
        case Aspect::Wide21x9:
            return 21.0f / 9.0f;
        case Aspect::Wide32x9:
            return 32.0f / 9.0f;
        case Aspect::Custom:
            // Never narrower than the game's own picture, as the window's shape is not.
            return std::max(4.0f / 3.0f, static_cast<float>(settings.aspect_custom_x)
                                             / static_cast<float>(std::max(1, settings.aspect_custom_y)));
        case Aspect::Window:
            // The game's picture is never narrower than its own: a tall window gets bars.
            return window_width > 0 && window_height > 0
                       ? std::max(4.0f / 3.0f, static_cast<float>(window_width) / static_cast<float>(window_height))
                       : 4.0f / 3.0f;
        case Aspect::Standard:
        case Aspect::Count:
            break;
    }
    return 4.0f / 3.0f;
}

const char* name(DisplayMode mode) {
    constexpr const char* names[] = {"Window", "Borderless", "Fullscreen"};
    return names[std::min<int>(static_cast<int>(mode), 2)];
}

const char* name(Aspect aspect) {
    constexpr const char* names[] = {"4:3", "16:10", "16:9", "21:9", "32:9", "Window", "Custom"};
    return names[std::min<int>(static_cast<int>(aspect), 6)];
}

const char* name(Hud hud) {
    constexpr const char* names[] = {"Centre", "Edges"};
    return names[std::min<int>(static_cast<int>(hud), 1)];
}

const char* name(VSync vsync) {
    constexpr const char* names[] = {"Off", "On", "Adaptive"};
    return names[std::min<int>(static_cast<int>(vsync), 2)];
}

const char* name(Strafe strafe) {
    constexpr const char* names[] = {"Off", "Hold L2-R2", "Lock with weapons"};
    return names[std::min<int>(static_cast<int>(strafe), 2)];
}

std::string frame_rate_name(const Settings& settings) {
    switch (settings.frame_rate) {
        case FrameRate::Original:
            return "Original";
        case FrameRate::Capped:
            return std::to_string(settings.frame_cap);
        case FrameRate::Unlimited:
            return "Unlimited";
        case FrameRate::Display:
        case FrameRate::Count:
            break;
    }
    return "Display";
}

}  // namespace openrac::settings
