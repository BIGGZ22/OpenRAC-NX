// SPDX-License-Identifier: GPL-3.0-or-later
#include "platform/switch/launcher_appearance.h"
#include <algorithm>
#include <charconv>
#include <cstdio>
#include <fstream>
#include <sstream>
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

namespace openrac::platform::appearance {
// Deep space navy with an amber accent (the 2026-10-29 launcher redesign).
Palette dark() { return {rgb(10,15,30), rgb(13,19,38), rgb(18,25,51), rgb(238,242,250), rgb(143,155,184), rgb(255,181,71), rgb(31,40,71)}; }
Palette light() { return {rgb(238,242,247), rgb(226,233,241), rgb(255,255,255), rgb(22,36,53), rgb(78,98,119), rgb(0,111,98), rgb(188,202,216)}; }
Palette Settings::palette() const { return preset == Preset::Dark ? dark() : preset == Preset::Light ? light() : custom; }
void Settings::select(Preset value) {
    preset = value;
    if (value != Preset::Custom) {
        banner_color = palette().panel; banner_text = palette().text;
    }
}
void Settings::color(int index, Color value) {
    if (preset != Preset::Custom) custom = palette();
    preset = Preset::Custom;
    Color* fields[] = {&custom.background, &custom.sidebar, &custom.panel, &custom.text, &custom.muted, &custom.accent, &custom.border};
    if (index >= 0 && index < 7) *fields[index] = value | 0xff000000u;
}
bool parse_color(std::string_view text, Color& color) {
    if (text.starts_with('#')) text.remove_prefix(1);
    if (text.size() != 6) return false;
    unsigned value = 0;
    auto [end, ec] = std::from_chars(text.data(), text.data() + text.size(), value, 16);
    if (ec != std::errc{} || end != text.data() + text.size()) return false;
    color = rgb(value >> 16, (value >> 8) & 255, value & 255); return true;
}
std::string hex(Color color) {
    char text[8]; std::snprintf(text, sizeof(text), "#%02X%02X%02X", unsigned(color & 255), unsigned((color >> 8) & 255), unsigned((color >> 16) & 255)); return text;
}
Color contrasting(Color color) {
    const unsigned lightness = (color & 255) * 299 + ((color >> 8) & 255) * 587 + ((color >> 16) & 255) * 114;
    return lightness >= 145000 ? rgb(9,17,30) : rgb(255,255,255);
}
namespace {
int number(const std::string& text, int fallback) {
    int value = 0; auto [end, ec] = std::from_chars(text.data(), text.data() + text.size(), value);
    return ec == std::errc{} && end == text.data() + text.size() ? value : fallback;
}
std::string clean(std::string text, std::size_t limit) {
    text.erase(std::remove_if(text.begin(), text.end(), [](unsigned char c) { return c < 32 || c == 127; }), text.end());
    if (text.size() > limit) {
        std::size_t end = limit;
        while (end && (static_cast<unsigned char>(text[end]) & 0xc0) == 0x80) --end;
        text.resize(end);
    }
    return text;
}
}
Settings load(const std::filesystem::path& path) {
    Settings s;
    std::ifstream file(path);
    if (!file) file.open(path.string() + ".previous");
    std::string line;
    while (std::getline(file, line)) {
        if (line.size() > 1024) continue;
        if (!line.empty() && line.back() == '\r') line.pop_back();
        const auto split = line.find('='); if (split == std::string::npos) continue;
        const auto key = line.substr(0, split), value = line.substr(split + 1);
        if (key == "preset") s.preset = Preset(std::clamp(number(value, 0), 0, 2));
        else if (key == "background") s.background = Background(std::clamp(number(value, 1), 0, 2));
        else if (key == "window_art") s.window_art = WindowArt(std::clamp(number(value, 0), 0, 2));
        else if (key == "tint") s.tint = std::clamp(number(value, 65), 0, 90);
        else if (key == "heading") s.heading = clean(value, 80);
        else if (key == "title") s.title = clean(value, 120);
        else if (key == "subtitle") s.subtitle = clean(value, 180);
        else {
            const char* names[] = {"background_color","sidebar_color","panel_color","text_color","muted_color","accent_color","border_color","banner_color","banner_text"};
            Color* fields[] = {&s.custom.background,&s.custom.sidebar,&s.custom.panel,&s.custom.text,&s.custom.muted,&s.custom.accent,&s.custom.border,&s.banner_color,&s.banner_text};
            for (int i = 0; i < 9; ++i) if (key == names[i]) parse_color(value, *fields[i]);
        }
    }
    return s;
}
bool save(const std::filesystem::path& path, const Settings& s) {
    std::error_code ec; std::filesystem::create_directories(path.parent_path(), ec);
    std::ostringstream body;
    body << "preset=" << int(s.preset) << "\nbackground=" << int(s.background) << "\nwindow_art=" << int(s.window_art)
         << "\ntint=" << std::clamp(s.tint,0,90) << "\nheading=" << clean(s.heading,80)
         << "\ntitle=" << clean(s.title,120) << "\nsubtitle=" << clean(s.subtitle,180) << '\n';
    const char* names[] = {"background_color","sidebar_color","panel_color","text_color","muted_color","accent_color","border_color","banner_color","banner_text"};
    const Color fields[] = {s.custom.background,s.custom.sidebar,s.custom.panel,s.custom.text,s.custom.muted,s.custom.accent,s.custom.border,s.banner_color,s.banner_text};
    for (int i = 0; i < 9; ++i) body << names[i] << '=' << hex(fields[i]) << '\n';
    const std::string text = body.str(), temp = path.string()+".new", backup = path.string()+".previous";
    FILE* file = std::fopen(temp.c_str(), "wb"); if (!file) return false;
    bool ok = std::fwrite(text.data(),1,text.size(),file) == text.size();
    if (std::fflush(file) != 0) ok = false;
#ifdef _WIN32
    if (_commit(fileno(file)) != 0) ok = false;
#else
    if (fsync(fileno(file)) != 0) ok = false;
#endif
    if (std::fclose(file) != 0) ok = false;
    if (ok && std::rename(temp.c_str(),path.string().c_str()) == 0) return true;
    if (ok) {
        // Restore a stranded previous file before trying another replacement.
        if (!std::filesystem::exists(path,ec) && std::filesystem::exists(backup,ec)) std::rename(backup.c_str(),path.string().c_str());
        std::remove(backup.c_str());
        if (std::rename(path.string().c_str(),backup.c_str()) == 0) {
            if (std::rename(temp.c_str(),path.string().c_str()) == 0) { std::remove(backup.c_str()); return true; }
            std::rename(backup.c_str(),path.string().c_str());
        }
    }
    std::remove(temp.c_str()); return false;
}
}
