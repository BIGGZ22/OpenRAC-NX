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
#include "platform/switch/launcher_ui.h"
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
constexpr const char* kGraphicsPending = "sdmc:/switch/openrac/settings/launcher-deko-pending";

void graphics_presented() {
    std::error_code ec;
    std::filesystem::remove(kGraphicsPending, ec);
}

bool start_graphics(bool native, std::string& error) {
    if (native) {
        if (!card_directory("sdmc:/switch/openrac/settings")) {
            error = "Cannot create graphics recovery marker";
            return false;
        }
        FILE* file = std::fopen(kGraphicsPending, "wb");
        if (!file) {
            error = "Cannot create graphics recovery marker";
            return false;
        }
        bool ok =
            std::fputc('1', file) != EOF && std::fflush(file) == 0 && fsync(fileno(file)) == 0;
        if (std::fclose(file) != 0) {
            ok = false;
        }
        if (!ok) {
            error = "Cannot flush graphics recovery marker";
            return false;
        }
    }
    return ui->open(native, error);
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
    if (!ui) {
        ui = std::make_unique<launcher_ui::Graphics>();
    }
    std::string error;
    auto& choice = settings::get().switch_launcher_deko3d;
    if (start_graphics(choice, error)) {
        return true;
    }
    const auto first_error = error;
    choice = !choice;
    if (start_graphics(choice, error)) {
        settings::save();
        notice = "Menu graphics fallback: using " + std::string(choice ? "deko3d" : "OpenGL") + ". "
                 + first_error;
        return true;
    }
    show_error(first_error + " / " + error);
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
                    graphics_presented();
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
     "Render scale",
     "Anti-aliasing",
     "Texture filter",
     "Anisotropy",
     "Scenery detail",
     "VSync",
     "Frame rate",
     "Frame cap",
     "Frame counter",
     "Launcher graphics"};
constexpr const char* kAudio[] =
    {"Master volume", "Music volume", "Sound effects", "Playback mode"};
constexpr const char* kGameplay[] = {"Language", "Start skips cutscenes", "Detailed logging"};

int rows(int tab) {
    if (tab == 1) {
        return 12;
    }
    if (tab == 2) {
        return 4;
    }
    if (tab == 3) {
        return 3;
    }
    if (tab == 4) {
        return fixes::count();
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
                return name(s.aspect);
            case 1:
                return name(s.hud);
            case 2:
                return std::to_string(s.render_scale) + "%";
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
                return s.switch_launcher_deko3d ? "deko3d (experimental)" : "OpenGL";
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
        constexpr const char* langs[] = {"English", "French", "Spanish", "German", "Italian"};
        if (row == 0) {
            return langs[std::clamp(game_option("game_language", 1), 1, 5) - 1];
        }
        return (row == 1 ? s.skip_cutscenes : s.verbose_log) ? "On" : "Off";
    }
    if (tab == 4) {
        return fixes::on(row) ? "On" : "Off";
    }
    return {};
}

void change(int tab, int row, int by) {
    auto& s = get();
    if (tab == 1) {
        switch (row) {
            case 0:
                cycle(s.aspect, by, static_cast<int>(Aspect::Custom));
                break;
            case 1:
                cycle(s.hud, by, 2);
                break;
            case 2:
                s.render_scale = step_listed(kRenderScales, s.render_scale, by);
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
                break;
            case 10:
                step_frame_counter(by);
                break;
            case 11:
                s.switch_launcher_deko3d = !s.switch_launcher_deko3d;
                break;
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
        } else {
            s.verbose_log = !s.verbose_log;
        }
    }
    if (tab == 4) {
        fixes::set(row, !fixes::on(row));
    }
    notice = save() ? "Settings saved. Applied when you start the game."
                    : "Settings could not be saved. Check the SD card before exiting.";
}
#endif
#ifdef OPENRAC_FRONTEND
bool draw(int tab, int selected, const Data& data) {
    launcher_ui::View view;
    view.tab = tab;
    view.selected = selected;
    view.disc = data.disc;
    view.executable = data.executable;
    view.levels = data.levels;
    view.writable = data.writable;
    view.notice = notice;
    view.graphics = settings::get().switch_launcher_deko3d ? "deko3d (test)" : "OpenGL";
    if (tab >= 1 && tab <= 4) {
        for (int i = 0; i < rows(tab); ++i) {
            const char* label = tab == 1   ? kDisplay[i]
                                : tab == 2 ? kAudio[i]
                                : tab == 3 ? kGameplay[i]
                                           : fixes::fix(i).name;
            view.rows.push_back({label, value(tab, i)});
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
        graphics_presented();
        return true;
    }
    notice = "Menu graphics error: " + error;
    if (settings::get().switch_launcher_deko3d) {
        settings::get().switch_launcher_deko3d = false;
        settings::save();
        if (open_graphics()) {
            view.graphics = "OpenGL";
            view.notice = notice;
            if (ui->show(view, error)) {
                graphics_presented();
                return true;
            }
        }
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
    std::error_code recovery;
    if (std::filesystem::exists(kGraphicsPending, recovery)) {
        settings::get().switch_launcher_deko3d = false;
        settings::save();
        notice = "The previous deko3d menu start did not finish. Restored OpenGL.";
    }
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
    auto focus = appletGetFocusState();
    u64 previous_direction = 0;
    auto next_repeat = std::chrono::steady_clock::now();
    while (appletMainLoop()) {
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
        if (direction != previous_direction) {
            b |= direction;
            next_repeat = now + std::chrono::milliseconds(350);
        } else if (direction && now >= next_repeat) {
            // Backend changes are deliberate single presses, never held repeats.
            b |= (tab == 1 && selected == 11) ? direction & (HidNpadButton_Up | HidNpadButton_Down)
                                              : direction;
            next_repeat = now + std::chrono::milliseconds(90);
        }
        previous_direction = direction;
        HidTouchScreenState touch{};
        if (hidGetTouchScreenStates(&touch, 1) > 0 && touch.count > 0) {
            const int x = touch.touches[0].x, y = touch.touches[0].y;
            if (!touching) {
                touch_x = x;
                touch_y = swipe_y = y;
                touch_moved = false;
                touch_action = 0;
                if (x < 250 && y >= 144 && y < 568) {
                    tab = std::clamp((y - 144) / 60, 0, 6);
                    selected = 0;
                    dirty = true;
                } else if (tab >= 1 && tab <= 4 && x >= 296 && x < 1218 && y >= 143 && y < 530) {
                    const int row = std::max(0, selected - 8) + (y - 143) / 43;
                    if (row < rows(tab)) {
                        selected = row;
                        dirty = true;
                        if (x >= 850) {
                            touch_action = HidNpadButton_A;
                        }
                    }
                } else if (
                    (tab == 0 && x >= 328 && x < 600 && y >= 322 && y < 372)
                    || (tab == 6 && x >= 328 && x < 645 && y >= 470 && y < 520)
                ) {
                    touch_action = HidNpadButton_A;
                } else if (tab == 0 && y >= 558 && y < 626 && x >= 296 && x < 1000) {
                    touch_action = x < 700 ? HidNpadButton_X : HidNpadButton_Y;
                } else if (x > 1090 && y >= 650) {
                    touch_action = HidNpadButton_Plus;
                }
                touch_tab = tab;
                touch_row = selected;
            } else {
                if (std::abs(x - touch_x) + std::abs(y - touch_y) > 12) {
                    touch_moved = true;
                    touch_action = 0;
                }
                if (tab >= 1 && tab <= 4 && x >= 296 && std::abs(y - swipe_y) >= 32
                    && rows(tab) > 0) {
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
            tab = (tab + ((b & HidNpadButton_R) ? 1 : 6)) % 7;
            selected = 0;
            dirty = true;
        }
        if (b & HidNpadButton_B) {
            tab = 0;
            selected = 0;
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
                const bool previous = settings::get().switch_launcher_deko3d;
                change(tab, selected, (b & HidNpadButton_Left) ? -1 : 1);
                if (previous != settings::get().switch_launcher_deko3d) {
                    if (!open_graphics()) {
                        return 0;
                    }
                    if (notice == "Settings saved. Applied when you start the game.") {
                        notice = "Menu graphics changed. Gameplay continues to use OpenGL.";
                    }
                }
            }
            if (b) {
                dirty = true;
            }
        }
        if ((b & HidNpadButton_ZL) || (tab == 0 && (b & HidNpadButton_A))) {
            data = inspect();
            if (data.ready()) {
                ui.reset();
                launcher_active = false;
                return 2;
            }
            if (!data.disc || !data.executable || !data.levels) {
                tab = 6;
                selected = 0;
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
            if (!draw(tab, selected, data)) {
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
