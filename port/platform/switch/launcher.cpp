// SPDX-License-Identifier: GPL-3.0-or-later
// Controller-operated Switch launcher; shares the game's settings file.
#include "platform/switch/launcher.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <string>
#include <switch.h>
#include <unistd.h>

#include "game/common/lib/card_storage.h"
#include "openrac/game_host.h"
#include "program/options.h"
#ifdef OPENRAC_FRONTEND
#include "fixes.h"
#include "platform/switch/input.h"
#include "platform/switch/launcher_ui.h"
#include "platform/switch/render_resolution.h"
#include "platform/switch/overlay_stats.h"
#include "scene/disc_cache.h"
#include "scene/disc_import.h"
#include "settings.h"
#endif

namespace openrac::platform {
namespace {
constexpr const char* kRoot = "sdmc:/switch/openrac/data";
bool launcher_active = false;
std::string notice;
#ifdef OPENRAC_FRONTEND
std::unique_ptr<launcher_ui::Graphics> ui;
std::unique_ptr<artwork::Library> art_library;
appearance::Settings launcher_appearance;
std::shared_ptr<const artwork::Image> local_background, included_logo;
constexpr const char* kAppearanceFile = "sdmc:/switch/openrac/artwork/appearance.ini";

bool artwork_keyboard(const char* title, const std::string& initial, bool secret, std::string& out) {
    SwkbdConfig keyboard{};
    if (R_FAILED(swkbdCreate(&keyboard, 0))) { notice = "Could not open the keyboard."; return false; }
    swkbdConfigMakePresetDefault(&keyboard);
    swkbdConfigSetHeaderText(&keyboard, title);
    swkbdConfigSetGuideText(&keyboard, secret ? "Your personal SteamGridDB API key" : "Enter your text, or a color as #RRGGBB");
    swkbdConfigSetStringLenMax(&keyboard, secret ? 128 : 100);
    swkbdConfigSetPasswordFlag(&keyboard, secret ? 1 : 0);
    swkbdConfigSetInitialText(&keyboard, initial.c_str());
    char text[512]{};
    const Result result = swkbdShow(&keyboard, text, sizeof(text));
    swkbdClose(&keyboard);
    if (R_FAILED(result)) return false;
    out = text; return true;
}

bool load_banner_background() {
    std::string error;
    for (const char* name : {"background.png", "background.jpg"}) {
        auto image = artwork::load_image(std::filesystem::path("sdmc:/switch/openrac/artwork") / name, error);
        if (image) { local_background = std::move(image); return true; }
    }
    return false;
}

const char* kAppearanceRows[] = {"Theme preset", "Menu background", "Bar color", "Panel color",
    "Text color", "Secondary text color", "Accent / button color", "Border color", "Banner background",
    "Banner color", "Banner image tint", "Banner text color", "Small artwork window", "Banner heading",
    "Game title", "Banner subtitle", "Reload local background", "Restore appearance defaults"};

std::string appearance_value(int row) {
    const auto& s = launcher_appearance;
    const auto p = s.palette();
    const appearance::Color colors[] = {p.background,p.sidebar,p.panel,p.text,p.muted,p.accent,p.border};
    if (row >= 1 && row <= 7) return appearance::hex(colors[row-1]);
    switch (row) {
        case 0: return s.preset == appearance::Preset::Dark ? "Dark" : s.preset == appearance::Preset::Light ? "Light" : "Custom";
        case 8: return s.background == appearance::Background::Solid ? "Solid color"
            : s.background == appearance::Background::SelectedArtwork ? "Selected artwork" : "Local background image";
        case 9: return appearance::hex(s.banner_color);
        case 10: return std::to_string(s.tint) + "%";
        case 11: return appearance::hex(s.banner_text);
        case 12: return s.window_art == appearance::WindowArt::GameLogo ? "Included game logo"
            : s.window_art == appearance::WindowArt::SelectedArtwork ? "Selected artwork" : "Off";
        case 13: return s.heading.empty() ? "Hidden" : s.heading;
        case 14: return s.title.empty() ? "Hidden" : s.title;
        case 15: return s.subtitle.empty() ? "Hidden" : s.subtitle;
        case 16: return "A: background.png / .jpg";
        default: return "A: Restore dark theme";
    }
}

void appearance_action(int row, int by, bool confirm) {
    auto& s = launcher_appearance;
    const auto palette = s.palette();
    const appearance::Color fields[] = {palette.background,palette.sidebar,palette.panel,palette.text,palette.muted,palette.accent,palette.border};
    std::string text;
    if ((row >= 1 && row <= 7) || row == 9 || row == 11) {
        auto color = row <= 7 ? fields[row-1] : row == 9 ? s.banner_color : s.banner_text;
        if (confirm) {
            if (!artwork_keyboard("Choose color (#RRGGBB)", appearance::hex(color), false, text)) return;
            if (!appearance::parse_color(text,color)) { notice = "Use six hex digits, for example #44E8C1."; return; }
        } else {
            constexpr appearance::Color choices[] = {appearance::rgb(9,17,30),appearance::rgb(255,255,255),
                appearance::rgb(236,244,249),appearance::rgb(19,32,49),appearance::rgb(144,166,184),
                appearance::rgb(68,232,193),appearance::rgb(0,111,98),appearance::rgb(64,132,242),
                appearance::rgb(157,97,231),appearance::rgb(237,96,140),appearance::rgb(234,82,65),appearance::rgb(255,177,87)};
            int index = 0;
            for (int i = 0; i < 12; ++i) if (color == choices[i]) index = i;
            color = choices[(index + by + 12) % 12];
        }
        if (row <= 7) s.color(row-1,color); else if (row == 9) s.banner_color = color; else s.banner_text = color;
    } else if (row == 0) s.select(appearance::Preset((int(s.preset)+by+3)%3));
    else if (row == 8) {
        s.background = appearance::Background((int(s.background)+by+3)%3);
        if (s.background == appearance::Background::LocalImage && !load_banner_background()) {
            appearance::save(kAppearanceFile,s);
            notice = "Add background.png or background.jpg to /switch/openrac/artwork/, then reload it."; return;
        }
    } else if (row == 10) s.tint = std::clamp(s.tint+by*5,0,90);
    else if (row == 12) s.window_art = appearance::WindowArt((int(s.window_art)+by+3)%3);
    else if (row >= 13 && row <= 15) {
        if (!confirm) return;
        std::string& field = row == 13 ? s.heading : row == 14 ? s.title : s.subtitle;
        if (!artwork_keyboard(kAppearanceRows[row],field,false,text)) return;
        field = text;
    } else if (row == 16 && confirm) {
        if (!load_banner_background()) { notice = "No valid background.png or background.jpg found in /switch/openrac/artwork/."; return; }
        s.background = appearance::Background::LocalImage;
    } else if (row == 17 && confirm) s = appearance::Settings{};
    else return;
    notice = appearance::save(kAppearanceFile,s) ? "Appearance saved. Press Y to preview your Play banner."
        : "Appearance applied, but could not be saved. Check the SD card.";
}

void artwork_action(int row, int by) {
    if (!art_library || art_library->state().busy) return;
    const auto state = art_library->state();
    std::string text;
    switch (row) {
        case 0:
            if (artwork_keyboard("SteamGridDB API key", "", true, text)) art_library->set_key(text);
            break;
        case 1:
            if (artwork_keyboard("Find game artwork", state.query, false, text)) art_library->search(text);
            break;
        case 2: art_library->select_game(by); break;
        case 3: art_library->browse(); break;
        case 4: art_library->select_image(by); break;
        case 5: art_library->download(); break;
        case 6: art_library->reload_local(); break;
        case 7: art_library->reset(); break;
    }
}

void show_error(const std::string& error) {
    ErrorApplicationConfig dialog;
    if (R_SUCCEEDED(errorApplicationCreate(
            &dialog, "OpenRAC-NX could not open the graphical launcher.", error.c_str()
        ))) {
        errorApplicationShow(&dialog);
    }
}

bool open_graphics() {
    ui = std::make_unique<launcher_ui::Graphics>();
    std::string error;
    if (ui->open(error)) {
        return true;
    }
    show_error(error);
    ui.reset();
    return false;
}
#endif
std::string card_root() {
    return std::string("sdmc:/switch/openrac/memcard/") + openrac_game.serial;
}

bool signature(
    const std::filesystem::path& path, std::streamoff offset, const char* expected, std::size_t size
) {
    std::ifstream file(path, std::ios::binary);
    char bytes[8]{};
    file.seekg(offset);
    return size <= sizeof(bytes) && file.read(bytes, static_cast<std::streamsize>(size))
           && std::memcmp(bytes, expected, size) == 0;
}

struct Data {
    bool disc, executable, levels, writable;

    bool ready() const { return disc && executable && levels && writable; }
};

bool check_card() {
    const auto root = card_root();
    if (!card_directory(root.c_str())) {
        return false;
    }
    const auto path = root + "/.openrac-write-test";
    const char marker[] = "OpenRAC save storage check";
    FILE* f = std::fopen(path.c_str(), "wb");
    if (!f) {
        return false;
    }
    bool ok = std::fwrite(marker, 1, sizeof marker, f) == sizeof marker;
    if (std::fflush(f) != 0 || fsync(fileno(f)) != 0) {
        ok = false;
    }
    if (std::fclose(f) != 0) {
        ok = false;
    }
    f = std::fopen(path.c_str(), "rb");
    if (f) {
        char readback[sizeof marker]{};
        ok = ok && std::fread(readback, 1, sizeof readback, f) == sizeof readback
             && std::memcmp(marker, readback, sizeof marker) == 0;
        std::fclose(f);
    } else {
        ok = false;
    }
    std::remove(path.c_str());
    return ok;
}

Data inspect() {
    const auto folder = std::filesystem::path(kRoot) / "iso_data" / openrac_game.game;
    const auto levels = std::filesystem::path(kRoot) / "port/pal";
    std::error_code ec;
    bool exported = std::filesystem::is_regular_file(levels / "level_99/manifest.json", ec)
                    && std::filesystem::is_regular_file(levels / "level_00/manifest.json", ec);
    if (std::filesystem::exists(levels / "setup-complete", ec)) {
#ifdef OPENRAC_FRONTEND
        std::ifstream stamp(levels / "setup-complete");
        unsigned version = 0;
        exported = exported && (stamp >> version) && version == scene::kDiscCacheVersion;
#endif
        // An on-device install promises all scenes, not only the first one.
        for (int id = 0; id < 21 && exported; ++id) {
            const int level = id < 19 ? id : id == 19 ? 98 : 99;
            char name[16];
            std::snprintf(name, sizeof name, "level_%02d", level);
            const auto folder = levels / name;
            exported = std::filesystem::is_regular_file(folder / "manifest.json", ec)
                       && (std::filesystem::is_regular_file(folder / "scene.orcache", ec)
                           || std::filesystem::is_regular_file(folder / "placements.json", ec));
        }
    }
    return {
        signature(program::installed_disc(kRoot), 16 * 2048, "\1CD001\1", 7),
        signature(folder / openrac_game.serial, 0, "\177ELF", 4),
        exported && !std::filesystem::exists(levels / "setup-in-progress", ec),
        check_card()
    };
}
#ifdef OPENRAC_FRONTEND
using namespace settings;

std::filesystem::path setup_image() {
    std::error_code ec;
    const auto installed = program::installed_disc(kRoot);
    if (std::filesystem::is_regular_file(installed, ec)) {
        return installed;
    }
    const auto legacy = std::filesystem::path(kRoot) / "iso_data/rac1/disc.iso";
    if (std::filesystem::is_regular_file(legacy, ec)) {
        return legacy;
    }
    std::filesystem::path found;
    for (std::filesystem::directory_iterator it("sdmc:/switch/openrac", ec), end; it != end && !ec;
         it.increment(ec)) {
        auto extension = it->path().extension().string();
        for (auto& c : extension) {
            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        }
        if (extension == ".iso" && it->is_regular_file(ec)) {
            if (!found.empty()) {
                throw std::runtime_error(
                    "Multiple ISOs found. Keep only the supported Ratchet 1 PAL ISO in "
                    "/switch/openrac/."
                );
            }
            found = it->path();
        }
    }
    return found;
}

void prepare(PadState& pad) {
    std::string current_step;
    std::ofstream setup_log;
    try {
        if (appletGetAppletType() != AppletType_Application
            && appletGetAppletType() != AppletType_SystemApplication) {
            notice =
                "Disc setup needs full application mode. Exit and relaunch Homebrew Menu that way.";
            return;
        }
        const auto image = setup_image();
        if (image.empty()) {
            notice = "Copy your supported PAL ISO into /switch/openrac/, then press A here.";
            return;
        }
        if (card_directory(kRoot)) {
            setup_log.open(std::filesystem::path(kRoot) / "setup.log", std::ios::trunc);
        }
        auto last_draw = std::chrono::steady_clock::time_point{};
        std::string last_text;
        scene::prepare_switch_disc(
            image, kRoot, [&](const std::string& text, unsigned done, unsigned total) {
                current_step = text;
                if (!appletMainLoop()) {
                    return false;
                }
                padUpdate(&pad);
                if (padGetButtonsDown(&pad) & HidNpadButton_Plus) {
                    return false;
                }
                const auto now = std::chrono::steady_clock::now();
                if (text != last_text || now - last_draw > std::chrono::milliseconds(250)) {
                    if (text != last_text) {
                        setup_log << done << '/' << total << ' ' << text << '\n';
                        setup_log.flush();
                    }
                    std::string graphics_error;
                    if (!ui->progress(text, done, total, graphics_error)) {
                        throw std::runtime_error(graphics_error);
                    }
                    last_draw = now;
                    last_text = text;
                }
                return true;
            }
        );
        notice = "Disc setup complete. Open Play and press A to start.";
    } catch (const std::exception& e) {
        notice = current_step.empty() ? e.what() : current_step + ": " + e.what();
        setup_log << "ERROR: " << notice << '\n';
    }
}

constexpr const char* kDisplay[] =
    {"Picture shape",
     "HUD position",
     "Render resolution",
     "Anti-aliasing",
     "Texture filter",
     "Anisotropy",
     "Scenery detail",
     "VSync",
     "Frame rate",
     "Frame cap",
     "Frame counter",
     "Handheld resolution",
     "Docked resolution"};
constexpr const char* kAudio[] =
    {"Master volume", "Music volume", "Sound effects", "Playback mode"};
constexpr const char* kGameplay[] = {"Language", "Start skips cutscenes", "Detailed logging", "Lock-strafe camera", "Field of view"};

int rows(int tab) {
    if (tab == launcher_ui::kAppearanceTab) return 18;
    if (tab == launcher_ui::kArtworkTab) return 8;
    if (tab == 1) {
        return 13;
    }
    if (tab == 2) {
        return 4;
    }
    if (tab == 3) {
        return 5;
    }
    if (tab == 4) {
        return fixes::count();
    }
    if (tab == 7) {
        return 24;
    }
    return 0;
}

template <class E>
void cycle(E& value, int by, int count) {
    value = static_cast<E>((static_cast<int>(value) + by + count) % count);
}

std::string value(int tab, int row) {
    const auto& s = get();
    if (tab == 1) {
        switch (row) {
            case 0:
                if (const int preset = platform::switch_video::profile_resolution(s.switch_resolution,
                        s.switch_handheld_resolution, s.switch_docked_resolution, overlay_stats::docked()); preset != 0) {
                    const auto& r = platform::switch_video::resolution(preset);
                    return r.width == 640 || r.width == 1024 ? "4:3 (resolution)" : "16:9 (resolution)";
                }
                return name(s.aspect);
            case 1:
                return name(s.hud);
            case 2:
                return platform::switch_video::resolution(s.switch_resolution).label;
            case 3:
                return s.msaa == 1 ? "Off" : std::to_string(s.msaa) + "x";
            case 4:
                return name(s.texture_filter);
            case 5:
                return s.anisotropy ? std::to_string(s.anisotropy) + "x" : "Auto";
            case 6:
                return s.original_lod ? "Original draw distances" : "Full detail";
            case 7:
                return name(s.vsync);
            case 8:
                return frame_rate_name(s);
            case 9:
                return std::to_string(s.frame_cap);
            case 10:
                return name(s.show_fps);
            case 11:
                return s.switch_handheld_resolution < 0 ? "Use render resolution"
                    : platform::switch_video::resolution(s.switch_handheld_resolution).label;
            case 12:
                return s.switch_docked_resolution < 0 ? "Use render resolution"
                    : platform::switch_video::resolution(s.switch_docked_resolution).label;
        }
    }
    if (tab == 2) {
        if (row == 0) {
            return std::to_string(s.volume) + "%";
        }
        if (row == 3) {
            return game_option("game_playback", 0) ? "Mono" : "Stereo";
        }
        return std::to_string(
                   (game_option(row == 1 ? "game_music_volume" : "game_sound_volume", 1024) * 100
                    + 512)
                   / 1024
               )
               + "%";
    }
    if (tab == 3) {
        if (row == 4) return s.switch_fov == 100 ? "Original (100%)" : std::to_string(s.switch_fov) + "%";
        constexpr const char* langs[] = {"English", "French", "Spanish", "German", "Italian"};
        if (row == 0) {
            return langs[std::clamp(game_option("game_language", 1), 1, 5) - 1];
        }
        return (row == 1 ? s.skip_cutscenes : row == 3 ? s.switch_lock_strafe : s.verbose_log) ? "On" : "Off";
    }
    if (tab == 4) {
        return fixes::on(row) ? "On" : "Off";
    }
    if (tab == 7) {
        const auto& c = s.controller;
        switch (row) {
            case 0:
                return std::to_string(c.movement_deadzone) + "%";
            case 1:
                return std::to_string(c.camera_deadzone) + "%";
            case 2:
                return std::to_string(c.camera_x) + "%";
            case 3:
                return std::to_string(c.camera_y) + "%";
            case 4:
                return c.invert_x ? "On" : "Off";
            case 5:
                return c.invert_y ? "On" : "Off";
            case 6:
                return c.swap_sticks ? "On" : "Off";
            case 23:
                return "A  Restore";
            default:
                return controller::binding_name(c.buttons[row - 7]);
        }
    }
    return {};
}

const char* controller_label(int row) {
    constexpr const char* labels[] =
        {"Movement deadzone",
         "Camera deadzone",
         "Camera horizontal speed",
         "Camera vertical speed",
         "Invert camera X",
         "Invert camera Y",
         "Swap sticks"};
    if (row < 7) {
        return labels[row];
    }
    if (row < 23) {
        return controller::kActionNames[row - 7];
    }
    return "Restore controller defaults";
}

void capture_binding(PadState& pad, int action) {
    controller::BindingCapture capture;
    std::string prompt = "Release all buttons, then press the new button.";
    std::string shown;
    const auto end = std::chrono::steady_clock::now() + std::chrono::seconds(20);
    while (appletMainLoop() && std::chrono::steady_clock::now() < end) {
        padUpdate(&pad);
        const auto raw = padGetButtons(&pad);
        if ((raw & (HidNpadButton_Plus | HidNpadButton_Minus))
            == (HidNpadButton_Plus | HidNpadButton_Minus)) {
            notice = "Rebind cancelled. Existing binding kept.";
            return;
        }
        HidTouchScreenState touch{};
        const bool touched = hidGetTouchScreenStates(&touch, 1) > 0 && touch.count;
        int touch_x = touched ? int(touch.touches[0].x) : 0;
        int touch_y = touched ? int(touch.touches[0].y) : 0;
        if (touched && launcher_ui::menu_touch(touch_x, touch_y) && touch_x >= launcher_ui::kCancelX
            && touch_x < launcher_ui::kCancelX + launcher_ui::kCancelW && touch_y >= launcher_ui::kCancelY - 6
            && touch_y < launcher_ui::kCancelY + launcher_ui::kCancelH + 6) {
            notice = "Rebind cancelled. Existing binding kept.";
            return;
        }
        const auto state = capture.update(switch_physical_buttons(raw));
        switch (state) {
            case controller::CaptureState::Cancelled:
                notice = "Rebind cancelled. Existing binding kept.";
                return;
            case controller::CaptureState::ReleaseButtons:
                prompt = "Release all buttons, then press the new button.";
                break;
            case controller::CaptureState::PressButton:
                prompt = "Press one controller button, then release it.";
                break;
            case controller::CaptureState::MultipleButtons:
                prompt = "Use one button at a time. Release and try again.";
                break;
            case controller::CaptureState::ReleaseToConfirm:
                prompt = std::string("Release ") + controller::binding_name(capture.physical)
                         + " to confirm.";
                break;
            case controller::CaptureState::Bound:
                controller::bind(
                    settings::get().controller, static_cast<unsigned>(action), capture.physical
                );
                notice = settings::save()
                             ? std::string("Bound ") + controller::kActionNames[action] + " to "
                                   + controller::binding_name(capture.physical)
                                   + ". Occupied bindings are swapped."
                             : "Binding changed, but settings could not be saved.";
                return;
        }
        if (shown != prompt) {
            std::string error;
            if (!ui->binding(controller::kActionNames[action], prompt, error)) {
                show_error(error);
                return;
            }
            shown = prompt;
        }
        svcSleepThread(16'000'000);
    }
    notice = "Rebind timed out. Existing binding kept.";
}

void change(int tab, int row, int by) {
    auto& s = get();
    if (tab == 1) {
        switch (row) {
            case 0:
                s.switch_resolution = 0;
                s.switch_handheld_resolution = s.switch_docked_resolution = -1;
                cycle(s.aspect, by, static_cast<int>(Aspect::Custom));
                break;
            case 1:
                cycle(s.hud, by, 2);
                break;
            case 2:
                s.switch_resolution = (s.switch_resolution + by + int(platform::switch_video::kResolutions.size()))
                                      % int(platform::switch_video::kResolutions.size());
                s.switch_handheld_resolution = s.switch_docked_resolution = -1;
                s.render_scale = 100;
                break;
            case 3:
                s.msaa = step_listed(kSampleCounts, s.msaa, by);
                break;
            case 4:
                cycle(s.texture_filter, by, 2);
                break;
            case 5: {
                constexpr int a[] = {0, 1, 2, 4, 8, 16};
                s.anisotropy = step_listed(a, s.anisotropy, by);
                break;
            }
            case 6:
                s.original_lod = !s.original_lod;
                break;
            case 7:
                cycle(s.vsync, by, 2);
                break;
            case 8:
                cycle(s.frame_rate, by, static_cast<int>(FrameRate::Count));
                break;
            case 9:
                s.frame_cap = std::clamp(s.frame_cap + by * 5, 20, 120);
                s.frame_rate = FrameRate::Capped;
                break;
            case 10:
                step_frame_counter(by);
                break;
            case 11:
            case 12: {
                int& preset = row == 11 ? s.switch_handheld_resolution : s.switch_docked_resolution;
                constexpr int count = int(platform::switch_video::kResolutions.size()) + 1;
                preset = (preset + 1 + by + count) % count - 1;
                s.render_scale = 100;
                break;
            }
        }
    }
    if (tab == 2) {
        if (row == 0) {
            set_volume(s.volume + by * 5);
        } else if (row == 3) {
            s.game_options["game_playback"] = !game_option("game_playback", 0);
        } else {
            const char* key = row == 1 ? "game_music_volume" : "game_sound_volume";
            s.game_options[key] = std::clamp(game_option(key, 1024) + by * 51, 0, 1024);
        }
    }
    if (tab == 3) {
        if (row == 0) {
            s.game_options["game_language"] =
                (game_option("game_language", 1) - 1 + by + 5) % 5 + 1;
        } else if (row == 1) {
            s.skip_cutscenes = !s.skip_cutscenes;
        } else if (row == 3) {
            s.switch_lock_strafe = !s.switch_lock_strafe;
        } else if (row == 4) {
            s.switch_fov = std::clamp(s.switch_fov + by * 5, 85, 120);
        } else {
            s.verbose_log = !s.verbose_log;
        }
    }
    if (tab == 4) {
        fixes::set(row, !fixes::on(row));
    }
    if (tab == 7) {
        auto& c = s.controller;
        switch (row) {
            case 0:
                c.movement_deadzone = std::clamp(c.movement_deadzone + by, 0, 30);
                break;
            case 1:
                c.camera_deadzone = std::clamp(c.camera_deadzone + by, 0, 30);
                break;
            case 2:
                c.camera_x = std::clamp(c.camera_x + by * 5, 25, 200);
                break;
            case 3:
                c.camera_y = std::clamp(c.camera_y + by * 5, 25, 200);
                break;
            case 4:
                c.invert_x = !c.invert_x;
                break;
            case 5:
                c.invert_y = !c.invert_y;
                break;
            case 6:
                c.swap_sticks = !c.swap_sticks;
                break;
            case 23:
                notice = "Press A to restore controller defaults.";
                return;
            default: {
                int physical = c.buttons[row - 7];
                physical = (physical + 1 + by + 17) % 17 - 1;
                controller::bind(c, static_cast<unsigned>(row - 7), physical);
                break;
            }
        }
    }
    notice = save() ? "Settings saved. Applied when you start the game."
                    : "Settings could not be saved. Check the SD card before exiting.";
}
#endif
#ifdef OPENRAC_FRONTEND
// What a row does, for the launcher's side panel (empty: the panel shows a general hint).
std::string row_help(int tab, int row) {
    if (tab == 1) {
        switch (row) {
            case 0: return "The shape of the picture. Choosing a shape returns Render resolution to Auto.";
            case 2: return "The size the game draws at before it is scaled to the screen. Lower is smoother, "
                           "higher is sharper. Auto keeps the picture shape with a 480p minimum.";
            case 3: return "Smooths jagged edges. Costs GPU time.";
            case 4: return "How textures are smoothed when they are scaled.";
            case 5: return "Keeps textures sharp at steep angles, such as floors in the distance.";
            case 6: return "Full detail draws more of a level in the distance. Original keeps the PS2's draw distances.";
            case 7: return "Waits for the display's refresh, so the picture does not tear.";
            case 9: return "The highest frame rate allowed when the frame rate is capped.";
            case 10: return "Shows the frame rate while you play.";
            case 11: return "Overrides Render resolution while the console is handheld.";
            case 12: return "Overrides Render resolution while the console is docked.";
        }
        return {};
    }
    if (tab == 2) {
        constexpr const char* help[] = {"The overall volume.", "The volume of the music.",
            "The volume of sound effects and voices.", "Stereo, or mono for one speaker."};
        return row < 4 ? help[row] : "";
    }
    if (tab == 3) {
        constexpr const char* help[] = {"The game's language.", "Start skips cutscenes.",
            "Writes more detail to the log on the SD card, for bug reports.",
            "Hold ZL to strafe: the left stick moves without turning Ratchet, the right stick aims. "
            "Off restores the original L2.",
            "Experimental. Widens the on-foot camera from 85% to 120%; 100% is the original view."};
        return row < 5 ? help[row] : "";
    }
    if (tab == 4) {
        return "On is the port's fix. Off restores the console's original behaviour.";
    }
    if (tab == 7) {
        if (row < 2) return "How far the stick moves before it counts.";
        if (row < 4) return "How fast the camera turns.";
        if (row < 7) return {};
        if (row < 23) return "A: press a new button for this action. X clears it. Left / Right cycles through buttons.";
        return "A restores every controller setting to its default.";
    }
    if (tab == launcher_ui::kAppearanceTab) {
        return "A edits a colour or text. Left / Right changes it. Y previews the Play banner.";
    }
    return {};
}

bool draw(int tab, int selected, bool pages_focus, const Data& data) {
    launcher_ui::View view;
    view.tab = tab;
    view.selected = selected;
    view.pages_focus = pages_focus;
    view.disc = data.disc;
    view.executable = data.executable;
    view.levels = data.levels;
    view.writable = data.writable;
    view.notice = notice;
    view.appearance = launcher_appearance;
    if (art_library) {
        const auto art = art_library->state();
        view.art = art.art;
        view.custom_art = art.custom;
        if (launcher_appearance.background == appearance::Background::SelectedArtwork && art.custom)
            view.banner_background = art.art;
        if (launcher_appearance.background == appearance::Background::LocalImage)
            view.banner_background = local_background;
        if (launcher_appearance.window_art == appearance::WindowArt::GameLogo) view.window_art = included_logo;
        if (launcher_appearance.window_art == appearance::WindowArt::SelectedArtwork) view.window_art = art.art;
        if (tab == launcher_ui::kArtworkTab) {
            view.notice = art.message;
            const auto game = art.games.empty() ? "Search first" : art.games[art.game].name;
            const auto image = art.images.empty() ? "Find banners first"
                : std::to_string(art.image + 1) + " / " + std::to_string(art.images.size())
                    + "  " + art.images[art.image].author;
            view.rows = {{"SteamGridDB API key", art.key_set ? "Saved (A to replace)" : "A to add", ""},
                {"Search game", "A: " + art.query, ""}, {"Game", game, ""},
                {"Find banners", art.busy ? "Working..." : "A to search", ""},
                {"Banner choice", image, ""}, {"Download and apply", art.busy ? "B to cancel" : "A to download", ""},
                {"Load local artwork", "custom.png / custom.jpg", ""}, {"Included game art", "A to restore", ""}};
        }
    }
    if (launcher_ui::settings_tab(tab)) {
        for (int i = 0; i < rows(tab); ++i) {
            const char* label = tab == 1   ? kDisplay[i]
                                : tab == 2 ? kAudio[i]
                                : tab == 3 ? kGameplay[i]
                                : tab == 7 ? controller_label(i)
                                : tab == launcher_ui::kAppearanceTab ? kAppearanceRows[i]
                                           : fixes::fix(i).name;
            view.rows.push_back({label, tab == launcher_ui::kAppearanceTab ? appearance_value(i) : value(tab, i),
                                 row_help(tab, i)});
        }
    }
    if (tab == 5) {
        std::error_code ec;
        for (std::filesystem::recursive_directory_iterator it(card_root(), ec), end;
             it != end && !ec;
             it.increment(ec)) {
            if (it->is_regular_file(ec)) {
                ++view.save_files;
            }
        }
    }
    std::string error;
    if (ui->show(view, error)) {
        return true;
    }
    show_error(error);
    return false;
}
#endif
}  // namespace

int switch_launcher() {
#ifndef OPENRAC_FRONTEND
    return 0;
#else
    launcher_active = true;
#ifdef OPENRAC_FRONTEND
    settings::load("sdmc:/switch/openrac/settings/rac1-pal.ini");
    art_library = std::make_unique<artwork::Library>("sdmc:/switch/openrac/artwork");
    launcher_appearance = appearance::load(kAppearanceFile);
    included_logo = artwork::bundled();
    if (launcher_appearance.background == appearance::Background::LocalImage) load_banner_background();
    struct ArtworkLifetime { ~ArtworkLifetime() { art_library.reset(); local_background.reset(); included_logo.reset(); } } artwork_lifetime;
    if (!open_graphics()) {
        return 0;
    }
#endif
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);
    PadState pad;
    padInitializeDefault(&pad);
    hidInitializeTouchScreen();
    bool touching = false;
    bool touch_moved = false;
    int touch_x = 0, touch_y = 0, swipe_y = 0, touch_tab = 0, touch_row = 0;
    u64 touch_action = 0;
    int tab = 0, selected = 0;
    // Settings and Customize open on their page list (Up / Down picks a page, A or Right opens it).
    bool pages_focus = false;
    const auto multi_page = [](int t) { return launcher_ui::section_pages(launcher_ui::section_of(t)).size() > 1; };
    const auto open_section = [&](int section) {
        tab = launcher_ui::section_pages(section)[0];
        selected = 0;
        pages_focus = multi_page(tab);
    };
    Data data = inspect();
#ifdef OPENRAC_FRONTEND
    std::error_code pending;
    std::ifstream complete(std::filesystem::path(kRoot) / "port/pal/setup-complete");
    unsigned cache_version = 0;
    const bool old_cache =
        complete.is_open()
        && (!(complete >> cache_version) || cache_version != scene::kDiscCacheVersion);
    if (!data.levels || old_cache
        || std::filesystem::exists(
            std::filesystem::path(kRoot) / "port/pal/setup-in-progress", pending
        )) {
        prepare(pad);
        data = inspect();
    }
#endif
    bool dirty = true;
    std::uint64_t artwork_revision = 0;
    auto focus = appletGetFocusState();
    u64 previous_direction = 0;
    auto next_repeat = std::chrono::steady_clock::now();
    // A notice shows in the bottom bar (or the side panel) for a few seconds, then the hints return.
    std::string shown_notice;
    auto notice_since = std::chrono::steady_clock::now();
    while (appletMainLoop()) {
        if (art_library->state().revision != artwork_revision) {
            artwork_revision = art_library->state().revision;
            dirty = true;
        }
        if (appletGetFocusState() != focus) {
            focus = appletGetFocusState();
            dirty = true;
        }
        padUpdate(&pad);
        u64 b = padGetButtonsDown(&pad);
        u64 direction =
            padGetButtons(&pad)
            & (HidNpadButton_Up | HidNpadButton_Down | HidNpadButton_Left | HidNpadButton_Right);
        const auto stick = padGetStickPos(&pad, 0);
        if (std::abs(stick.y) > std::abs(stick.x) && std::abs(stick.y) > 16000) {
            direction |= stick.y > 0 ? HidNpadButton_Up : HidNpadButton_Down;
        } else if (std::abs(stick.x) > 16000) {
            direction |= stick.x > 0 ? HidNpadButton_Right : HidNpadButton_Left;
        }
        const auto now = std::chrono::steady_clock::now();
        if (notice != shown_notice) {
            shown_notice = notice;
            notice_since = now;
        } else if (!notice.empty() && now - notice_since > std::chrono::seconds(6)) {
            notice.clear();
            shown_notice.clear();
            dirty = true;
        }
        if (direction != previous_direction) {
            b |= direction;
            next_repeat = now + std::chrono::milliseconds(350);
        } else if (direction && now >= next_repeat) {
            b |= direction;
            next_repeat = now + std::chrono::milliseconds(90);
        }
        previous_direction = direction;
        HidTouchScreenState touch{};
        const bool has_touch = hidGetTouchScreenStates(&touch, 1) > 0 && touch.count > 0;
        int x = has_touch ? int(touch.touches[0].x) : 0;
        int y = has_touch ? int(touch.touches[0].y) : 0;
        if (has_touch && launcher_ui::menu_touch(x, y)) {
            if (!touching) {
                touch_x = x;
                touch_y = swipe_y = y;
                touch_moved = false;
                touch_action = 0;
                namespace ui_layout = launcher_ui;
                const int rows_first = ui_layout::first_visible_row(selected);
                if (y >= ui_layout::kSectionY - 6 && y < ui_layout::kSectionY + ui_layout::kSectionH + 6
                    && x >= ui_layout::kSectionX
                    && x < ui_layout::kSectionX + ui_layout::kSectionCount * ui_layout::kSectionStep) {
                    const int section = std::clamp((x - ui_layout::kSectionX) / ui_layout::kSectionStep, 0,
                                                   ui_layout::kSectionCount - 1);
                    if (section != ui_layout::section_of(tab)) {
                        open_section(section);
                        dirty = true;
                    }
                } else if (multi_page(tab) && x >= ui_layout::kPageX && x < ui_layout::kPageX + ui_layout::kPageW
                           && y >= ui_layout::kPageY) {
                    const auto pages = ui_layout::section_pages(ui_layout::section_of(tab));
                    const int index = (y - ui_layout::kPageY) / ui_layout::kPageStep;
                    if (index < int(pages.size())) {
                        if (pages[index] != tab) selected = 0;
                        tab = pages[index];
                        pages_focus = false;
                        dirty = true;
                    }
                } else if (multi_page(tab) && x >= ui_layout::kRowX && x < ui_layout::kRowX + ui_layout::kRowW
                           && y >= ui_layout::kRowY && y < ui_layout::kRowY + ui_layout::kVisibleRows * ui_layout::kRowStep) {
                    const int row = rows_first + (y - ui_layout::kRowY) / ui_layout::kRowStep;
                    if (row < rows(tab)) {
                        selected = row;
                        pages_focus = false;
                        dirty = true;
                        if (x >= ui_layout::kRowValueX) {
                            touch_action = HidNpadButton_A;
                        }
                    }
                } else if (tab == 0 && y >= ui_layout::kLaunchY && y < ui_layout::kLaunchY + ui_layout::kLaunchH
                           && x >= ui_layout::kLaunchX && x < ui_layout::kLaunchX + ui_layout::kLaunchW) {
                    touch_action = HidNpadButton_A;
                } else if (tab == 0 && y >= ui_layout::kLaunchY && y < ui_layout::kLaunchY + ui_layout::kLaunchH
                           && x >= ui_layout::kBootX && x < ui_layout::kBootX + ui_layout::kBootW) {
                    touch_action = HidNpadButton_Y;
                } else if (tab == 0 && y >= ui_layout::kStatusY && y < ui_layout::kStatusY + ui_layout::kStatusH
                           && x >= ui_layout::kHeroX && x < ui_layout::kHeroX + ui_layout::kHeroW) {
                    touch_action = HidNpadButton_X;
                } else if (tab == 6 && x >= ui_layout::kSetupButtonX
                           && x < ui_layout::kSetupButtonX + ui_layout::kSetupButtonW && y >= ui_layout::kSetupButtonY
                           && y < ui_layout::kSetupButtonY + ui_layout::kSetupButtonH) {
                    touch_action = HidNpadButton_A;
                } else if (x > 1090 && y >= ui_layout::kFooterY) {
                    touch_action = HidNpadButton_Plus;
                }
                touch_tab = tab;
                touch_row = selected;
            } else {
                if (std::abs(x - touch_x) + std::abs(y - touch_y) > 12) {
                    touch_moved = true;
                    touch_action = 0;
                }
                if (multi_page(tab) && !pages_focus && x >= launcher_ui::kRowsX
                    && x < launcher_ui::kRowsX + launcher_ui::kRowsW && std::abs(y - swipe_y) >= 32 && rows(tab) > 0) {
                    selected = std::clamp(selected + (y < swipe_y ? 1 : -1), 0, rows(tab) - 1);
                    swipe_y = y;
                    dirty = true;
                }
            }
            touching = true;
        } else {
            if (touching && !touch_moved && tab == touch_tab && selected == touch_row) {
                b |= touch_action;
            }
            touch_action = 0;
            touching = false;
        }
        if (b & HidNpadButton_Plus) {
            break;
        }
        if (b & (HidNpadButton_L | HidNpadButton_R)) {
            const int count = launcher_ui::kSectionCount;
            open_section((launcher_ui::section_of(tab) + ((b & HidNpadButton_R) ? 1 : count - 1)) % count);
            dirty = true;
        }
        if (b & HidNpadButton_B) {
            if (art_library->state().busy) { art_library->cancel(); dirty = true; continue; }
            if (multi_page(tab) && !pages_focus) {
                pages_focus = true;  // back to the page list
            } else {
                tab = 0;
                selected = 0;
                pages_focus = false;
            }
            dirty = true;
            b &= ~(HidNpadButton_A | HidNpadButton_Left | HidNpadButton_Right | HidNpadButton_Up | HidNpadButton_Down);
        }
        // The page list: Up / Down picks a page, A or Right opens it.
        if (pages_focus && multi_page(tab)) {
            const auto pages = launcher_ui::section_pages(launcher_ui::section_of(tab));
            const int index = int(std::find(pages.begin(), pages.end(), tab) - pages.begin());
            const int count = int(pages.size());
            if (b & (HidNpadButton_Up | HidNpadButton_Down)) {
                tab = pages[(index + ((b & HidNpadButton_Down) ? 1 : count - 1)) % count];
                selected = 0;
                dirty = true;
            }
            if (b & (HidNpadButton_A | HidNpadButton_Right)) {
                pages_focus = false;
                dirty = true;
            }
            b &= ~(HidNpadButton_A | HidNpadButton_Left | HidNpadButton_Right | HidNpadButton_Up | HidNpadButton_Down);
        }
        if (tab == launcher_ui::kAppearanceTab && (b & HidNpadButton_Y)) {
            tab = 0; selected = 0; pages_focus = false; b &= ~HidNpadButton_Y; notice.clear(); dirty = true;
        }
        if (tab == 7 && selected >= 7 && selected < 23 && (b & HidNpadButton_X)) {
            controller::bind(settings::get().controller, static_cast<unsigned>(selected - 7), -1);
            notice = settings::save()
                         ? "Binding cleared. Menu controls and exit shortcut are unchanged."
                         : "Could not save the cleared binding.";
            b &= ~HidNpadButton_X;
            dirty = true;
        }
        if (b & HidNpadButton_X) {
            data = inspect();
            notice = data.ready() ? "Installation and save storage ready."
                                  : "Some checks failed. See the Play / Saves pages.";
            dirty = true;
        }
#ifdef OPENRAC_FRONTEND
        if (tab == 6 && (b & HidNpadButton_A)) {
            prepare(pad);
            data = inspect();
            dirty = true;
        }
        const int n = rows(tab);
        if (n > 0) {
            if (b & HidNpadButton_Up) {
                selected = (selected + n - 1) % n;
            }
            if (b & HidNpadButton_Down) {
                selected = (selected + 1) % n;
            }
            if (b & (HidNpadButton_Left | HidNpadButton_Right | HidNpadButton_A)) {
                if (tab == launcher_ui::kArtworkTab) {
                    if ((b & HidNpadButton_A) || selected == 2 || selected == 4)
                        artwork_action(selected, (b & HidNpadButton_Left) ? -1 : 1);
                } else if (tab == launcher_ui::kAppearanceTab) {
                    appearance_action(selected,(b & HidNpadButton_Left) ? -1 : 1,(b & HidNpadButton_A) != 0);
                } else if (tab == 7 && (b & HidNpadButton_A) && selected >= 7 && selected < 23) {
                    capture_binding(pad, selected - 7);
                } else if (tab == 7 && selected == 23 && (b & HidNpadButton_A)) {
                    settings::get().controller = controller::Profile{};
                    notice = settings::save()
                                 ? "Controller defaults restored."
                                 : "Defaults restored, but settings could not be saved.";
                } else {
                    change(tab, selected, (b & HidNpadButton_Left) ? -1 : 1);
                }
            }
            if (b) {
                dirty = true;
            }
        }
        if ((b & HidNpadButton_ZL) || (tab == 0 && (b & HidNpadButton_A))) {
            if (art_library->state().busy) {
                notice = "Wait for the artwork download, or press B to cancel it.";
                dirty = true;
                continue;
            }
            data = inspect();
            if (data.ready()) {
                ui.reset();
                launcher_active = false;
                return 2;
            }
            if (!data.disc || !data.executable || !data.levels) {
                tab = 6;
                selected = 0;
                pages_focus = false;
            }
            notice = "Cannot start: installation or save-storage check failed.";
            dirty = true;
        }
#endif
        if (tab == 0 && (b & HidNpadButton_Y) && data.disc && data.executable) {
            std::string error;
            ui->progress("Diagnostic boot check", 0, 300, error);
            return 1;
        }
        if (dirty) {
            if (!draw(tab, selected, pages_focus, data)) {
                break;
            }
            dirty = false;
        }
        svcSleepThread(16'000'000);
    }
    ui.reset();
    launcher_active = false;
    return 0;
#endif
}

void switch_update_console() {
#ifdef OPENRAC_FRONTEND
    if (launcher_active && ui) {
        static unsigned frames = 0;
        if ((++frames % 10) == 0) {
            std::string error;
            ui->progress("Diagnostic boot check", std::min(frames, 300u), 300, error);
        }
    }
#endif
}
}  // namespace openrac::platform
