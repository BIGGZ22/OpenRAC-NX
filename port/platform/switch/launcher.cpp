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
     "Render scale",
     "Anti-aliasing",
     "Texture filter",
     "Anisotropy",
     "Scenery detail",
     "VSync",
     "Frame rate",
     "Frame cap",
     "Frame counter"};
constexpr const char* kAudio[] =
    {"Master volume", "Music volume", "Sound effects", "Playback mode"};
constexpr const char* kGameplay[] = {"Language", "Start skips cutscenes", "Detailed logging"};

int rows(int tab) {
    if (tab == 1) {
        return 11;
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
        if (hidGetTouchScreenStates(&touch, 1) > 0 && touch.count && touch.touches[0].x >= 328
            && touch.touches[0].x < 650 && touch.touches[0].y >= 465 && touch.touches[0].y < 525) {
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
bool draw(int tab, int selected, const Data& data) {
    launcher_ui::View view;
    view.tab = tab;
    view.selected = selected;
    view.disc = data.disc;
    view.executable = data.executable;
    view.levels = data.levels;
    view.writable = data.writable;
    view.notice = notice;
    if ((tab >= 1 && tab <= 4) || tab == 7) {
        for (int i = 0; i < rows(tab); ++i) {
            const char* label = tab == 1   ? kDisplay[i]
                                : tab == 2 ? kAudio[i]
                                : tab == 3 ? kGameplay[i]
                                : tab == 7 ? controller_label(i)
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
            b |= direction;
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
                if (x < 250 && y >= 144 && y < 628) {
                    tab = std::clamp((y - 144) / 60, 0, 7);
                    selected = 0;
                    dirty = true;
                } else if (
                    ((tab >= 1 && tab <= 4) || tab == 7) && x >= 296 && x < 1218 && y >= 143
                    && y < 530
                ) {
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
                if (((tab >= 1 && tab <= 4) || tab == 7) && x >= 296 && std::abs(y - swipe_y) >= 32
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
            tab = (tab + ((b & HidNpadButton_R) ? 1 : 7)) % 8;
            selected = 0;
            dirty = true;
        }
        if (b & HidNpadButton_B) {
            tab = 0;
            selected = 0;
            dirty = true;
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
                if (tab == 7 && (b & HidNpadButton_A) && selected >= 7 && selected < 23) {
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
