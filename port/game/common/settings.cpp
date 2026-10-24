// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

#include "settings.h"

#include <algorithm>
#include <atomic>
#include <cmath>
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
constexpr const char* kGtaoQuality[] = {"performance", "quality"};
constexpr const char* kTextureFilter[] = {"normal", "edge-aware"};
constexpr const char* kVSync[] = {"off", "on", "adaptive"};

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

bool parse_float(const std::string& value, float low, float high, float& out) {
    float number = 0.0f;
    char after = 0;
    if (std::sscanf(value.c_str(), "%f%c", &number, &after) != 1 || !std::isfinite(number)
        || number < low || number > high) {
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

// A whole number from a list's first to its last (any between, not only the list's own).
bool parse_within(const std::string& value, std::span<const int> list, int& out) {
    return parse_int(value, list.front(), list.back(), out);
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
    // A file from before the strafe was a mod (2026-10-21): its `strafe` or `lock_strafe` is left
    // out, and said once.
    bool old_strafe = false;
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
        } else if (key == "enhanced_rendering") {
            known = parse_bool(value, s.enhanced_rendering);
        } else if (key == "enhanced_graphics") {
            known = parse_bool(value, s.enhanced_graphics);
        } else if (key == "enhanced_shadows") {
            known = parse_bool(value, s.enhanced_shadows);
        } else if (key == "probe_lighting") {
            known = parse_bool(value, s.probe_lighting);
        } else if (key == "probe_spheres") {
            known = parse_bool(value, s.probe_spheres);
        } else if (key == "probe_normal_bias") {
            known = parse_float(value, 0.0f, 2.0f, s.probe_normal_bias);
        } else if (key == "probe_view_bias") {
            known = parse_float(value, 0.0f, 2.0f, s.probe_view_bias);
        } else if (key == "probe_gi_multiplier") {
            known = parse_float(value, 0.0f, 5.0f, s.probe_gi_multiplier);
        } else if (key == "gtao") {
            known = parse_bool(value, s.gtao);
        } else if (key == "gtao_quality") {
            known = parse(value, kGtaoQuality, s.gtao_settings.quality);
        } else if (key == "gtao_slices") {
            known = parse_int(value, 1, 12, s.gtao_settings.slices);
        } else if (key == "gtao_steps") {
            known = parse_int(value, 4, 24, s.gtao_settings.steps);
        } else if (key == "gtao_half_resolution") {
            known = parse_bool(value, s.gtao_settings.half_resolution);
        } else if (key == "gtao_radius") {
            known = parse_float(value, 0.05f, 8.0f, s.gtao_settings.radius);
        } else if (key == "gtao_strength") {
            known = parse_float(value, 0.0f, 3.0f, s.gtao_settings.strength);
        } else if (key == "temporal_quality") {
            known = parse(value, renderer::kTemporalKeys, s.temporal_quality);
        } else if (key == "temporal_sharpness") {
            known = parse_float(value, 0.0f, 1.0f, s.temporal_sharpness);
        } else if (key == "texture_filter") {
            known = parse(value, kTextureFilter, s.texture_filter);
        } else if (key == "anisotropy") {
            if (value == "auto") {
                s.anisotropy = 0;
            } else {
                known = parse_listed(value, kAnisotropies, s.anisotropy);
            }
        } else if (key == "vsync") {
            known = parse(value, kVSync, s.vsync);
            has_vsync = has_vsync || known;
        } else if (key == "frame_rate") {
            known = parse_frame_rate(value, s);
            has_frame_rate = has_frame_rate || known;
        } else if (key == "show_fps") {
            bool enabled = false;
            // Existing files also accept boolean words for the original counter.
            if (value == "2") {
                s.show_fps = FrameCounter::Timings;
            } else if (parse_bool(value, enabled)) {
                s.show_fps = enabled ? FrameCounter::Basic : FrameCounter::Off;
            } else {
                known = false;
            }
        } else if (key == "menu_bar") {
            known = parse_bool(value, s.menu_bar);
        } else if (key == "mute_unfocused") {
            known = parse_bool(value, s.mute_unfocused);
        } else if (key == "skip_cutscenes") {
            known = parse_bool(value, s.skip_cutscenes);
        } else if (key == "strafe" || key == "lock_strafe") {
            old_strafe = true;
        } else if (key == "volume") {
            known = parse_int(value, 0, 100, s.volume);
        } else if (key == "touch_scale") {
            known = parse_within(value, kTouchScales, s.touch_scale);
        } else if (key == "touch_camera") {
            known = parse_within(value, kTouchCameraSpeeds, s.touch_camera);
        } else if (key == "touch_layout") {
            s.touch_layout = value;  // TouchPad::set_layout leaves out what it does not know
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
    if (old_strafe) {
        log::info("{}: the strafe is a mod now, lock-strafe (OpenRAC/rac1-mods): add it in the launcher's "
                  "Mods tab. This file's strafe setting is left out.",
                  file.string());
    }
    g_gain = gain_now();
    log::info("settings from {}: {}", file.string(), summary());
}

std::string summary() {
    const Settings& s = g_settings;
    const std::string window = s.window_width > 0 && s.window_height > 0
                                   ? std::format("{}x{}", s.window_width, s.window_height)
                                   : std::string("default size");
    return std::format(
        "{} ({}, display {}), picture {}, {}% scale, {} sample(s), anisotropy {}, frames {}, vsync "
        "{}, "
        "volume {}{}{}",
        kDisplayMode[static_cast<int>(s.display_mode)],
        window,
        s.monitor,
        kAspect[static_cast<int>(s.aspect)],
        s.render_scale,
        s.msaa,
        s.anisotropy ? std::to_string(s.anisotropy) : "auto",
        frame_rate_value(s),
        kVSync[static_cast<int>(s.vsync)],
        s.volume,
        s.verbose_log ? ", detailed log" : "",
        fixes_off()
    );
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
        << "enhanced_rendering = " << (s.enhanced_rendering ? 1 : 0) << "\n"
        << "enhanced_graphics = " << (s.enhanced_graphics ? 1 : 0) << "\n"
        << "enhanced_shadows = " << (s.enhanced_shadows ? 1 : 0) << "\n"
        << "probe_lighting = " << (s.probe_lighting ? 1 : 0) << "\n"
        << "probe_spheres = " << (s.probe_spheres ? 1 : 0) << "\n"
        << "probe_gi_multiplier = " << s.probe_gi_multiplier << "\n"
        << "probe_normal_bias = " << s.probe_normal_bias << "\n"
        << "probe_view_bias = " << s.probe_view_bias << "\n"
        << "gtao = " << (s.gtao ? 1 : 0) << "\n"
        << "gtao_quality = " << kGtaoQuality[static_cast<int>(s.gtao_settings.quality)] << "\n"
        << "gtao_slices = " << s.gtao_settings.slices << "\n"
        << "gtao_steps = " << s.gtao_settings.steps << "\n"
        << "gtao_half_resolution = " << (s.gtao_settings.half_resolution ? 1 : 0) << "\n"
        << "gtao_radius = " << s.gtao_settings.radius << "\n"
        << "gtao_strength = " << s.gtao_settings.strength << "\n"
        << "texture_filter = " << kTextureFilter[static_cast<int>(s.texture_filter)] << "\n"
        << "temporal_quality = " << renderer::kTemporalKeys[static_cast<int>(s.temporal_quality)]
        << "\n"
        << "temporal_sharpness = " << s.temporal_sharpness << "\n"
        << "anisotropy = " << (s.anisotropy ? std::to_string(s.anisotropy) : "auto") << "\n"
        << "vsync = " << kVSync[static_cast<int>(s.vsync)] << "\n"
        << "frame_rate = " << frame_rate_value(s) << "\n"
        << "show_fps = " << static_cast<int>(s.show_fps) << "\n"
        << "menu_bar = " << (s.menu_bar ? 1 : 0) << "\n"
        << "mute_unfocused = " << (s.mute_unfocused ? 1 : 0) << "\n"
        << "skip_cutscenes = " << (s.skip_cutscenes ? 1 : 0) << "\n"
        << "volume = " << s.volume << "\n"
        << "touch_scale = " << s.touch_scale << "\n"
        << "touch_camera = " << s.touch_camera << "\n";
    if (!s.touch_layout.empty()) {
        out << "touch_layout = " << s.touch_layout << "\n";
    }
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

int step_listed(std::span<const int> list, int now, int by) {
    if (by > 0) {
        const auto next = std::upper_bound(list.begin(), list.end(), now);
        return next != list.end() ? *next : list.back();
    }
    const auto at = std::lower_bound(list.begin(), list.end(), now);
    return at != list.begin() ? *(at - 1) : list.front();
}

void step_frame_counter(int by) {
    constexpr int kModes = static_cast<int>(FrameCounter::Count);
    const int next = (static_cast<int>(g_settings.show_fps) + by % kModes + kModes) % kModes;
    g_settings.show_fps = static_cast<FrameCounter>(next);
}

const char* name(FrameCounter mode) {
    constexpr const char* kNames[] = {"Off", "On", "CPU / GPU"};
    return kNames[std::clamp(static_cast<int>(mode), 0, static_cast<int>(FrameCounter::Count) - 1)];
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

const char* name(renderer::TextureFilter filter) {
    constexpr const char* names[] = {"Normal", "Edge-aware (2x)"};
    return names[std::clamp(static_cast<int>(filter), 0, static_cast<int>(std::size(names)) - 1)];
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
