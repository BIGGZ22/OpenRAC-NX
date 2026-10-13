// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

#include "settings.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <system_error>

#include "common/log.h"

namespace openrac::settings {

namespace {

Settings g_settings;
std::filesystem::path g_file;

// The names in the file, by value.
constexpr const char* kAspect[] = {"4:3", "16:10", "16:9", "21:9", "window"};
constexpr const char* kFrameRate[] = {"original", "display", "60", "120", "144", "240", "unlimited"};

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

}  // namespace

Settings& get() {
    return g_settings;
}

void load(const std::filesystem::path& file) {
    g_file = file;
    g_settings = Settings{};
    std::ifstream in(file);
    if (!in) {
        return;
    }
    std::string line;
    while (std::getline(in, line)) {
        const auto eq = line.find('=');
        if (eq == std::string::npos || trimmed(line).starts_with('#')) {
            continue;
        }
        const std::string key = trimmed(line.substr(0, eq));
        const std::string value = trimmed(line.substr(eq + 1));
        bool known = true;
        if (key == "aspect") {
            known = parse(value, kAspect, g_settings.aspect);
        } else if (key == "frame_rate") {
            known = parse(value, kFrameRate, g_settings.frame_rate);
        } else if (key == "fullscreen") {
            g_settings.fullscreen = value == "1" || value == "true" || value == "on";
        }
        if (!known) {
            log::warn("{}: {} = {} is not a value the port knows; the default is used", file.string(), key, value);
        }
    }
    log::info("settings from {}: picture {}, frames {}, {}", file.string(), kAspect[static_cast<int>(g_settings.aspect)],
              kFrameRate[static_cast<int>(g_settings.frame_rate)], g_settings.fullscreen ? "fullscreen" : "windowed");
}

void save() {
    if (g_file.empty()) {
        return;
    }
    std::error_code made;
    std::filesystem::create_directories(g_file.parent_path(), made);
    std::ofstream out(g_file, std::ios::trunc);
    if (!out) {
        log::warn("the settings could not be written to {}", g_file.string());
        return;
    }
    out << "# OpenRAC: the port's settings (the Video options page of the game's menu changes them)\n"
        << "aspect = " << kAspect[static_cast<int>(g_settings.aspect)] << "\n"
        << "frame_rate = " << kFrameRate[static_cast<int>(g_settings.frame_rate)] << "\n"
        << "fullscreen = " << (g_settings.fullscreen ? 1 : 0) << "\n";
}

float aspect_ratio(Aspect aspect, int window_width, int window_height) {
    switch (aspect) {
        case Aspect::Wide16x10:
            return 16.0f / 10.0f;
        case Aspect::Wide16x9:
            return 16.0f / 9.0f;
        case Aspect::Wide21x9:
            return 21.0f / 9.0f;
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

int frame_cap(FrameRate rate) {
    switch (rate) {
        case FrameRate::Cap60:
            return 60;
        case FrameRate::Cap120:
            return 120;
        case FrameRate::Cap144:
            return 144;
        case FrameRate::Cap240:
            return 240;
        default:
            return 0;
    }
}

const char* name(Aspect aspect) {
    constexpr const char* names[] = {"4:3", "16:10", "16:9", "21:9", "Window"};
    return names[std::min<int>(static_cast<int>(aspect), 4)];
}

const char* name(FrameRate rate) {
    constexpr const char* names[] = {"Original", "Display", "60", "120", "144", "240", "Unlimited"};
    return names[std::min<int>(static_cast<int>(rate), 6)];
}

}  // namespace openrac::settings
