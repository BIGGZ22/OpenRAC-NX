// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <array>
#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>

namespace openrac::platform::appearance {
using Color = std::uint32_t;
constexpr Color rgb(unsigned r, unsigned g, unsigned b) { return r | g << 8 | b << 16 | 0xff000000u; }
struct Palette {
    Color background, sidebar, panel, text, muted, accent, border;
};
enum class Preset { Dark, Light, Custom };
enum class Background { Solid, SelectedArtwork, LocalImage };
enum class WindowArt { GameLogo, SelectedArtwork, Hidden };
Palette dark();
Palette light();
struct Settings {
    Preset preset = Preset::Dark;
    Palette custom = dark();
    Background background = Background::SelectedArtwork;
    WindowArt window_art = WindowArt::GameLogo;
    Color banner_color = rgb(13, 19, 38), banner_text = rgb(238, 242, 250);
    int tint = 65;
    std::string heading = "YOUR NEXT ADVENTURE", title = "Ratchet & Clank",
        subtitle = "Native Switch port  |  Europe / Australia v2.00";
    Palette palette() const;
    void select(Preset value);
    void color(int index, Color value);
};
bool parse_color(std::string_view text, Color& color);
std::string hex(Color color);
Color contrasting(Color color);
Settings load(const std::filesystem::path& path);
bool save(const std::filesystem::path& path, const Settings& settings);
}
