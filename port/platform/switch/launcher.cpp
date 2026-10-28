// SPDX-License-Identifier: GPL-3.0-or-later
// Controller-operated Switch launcher; shares the game's settings file.
#include "platform/switch/launcher.h"
#include <switch.h>
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <string>
#include <unistd.h>
#include "openrac/game_host.h"
#include "game/common/lib/card_storage.h"
#ifdef OPENRAC_FRONTEND
#include "settings.h"
#include "fixes.h"
#endif

namespace openrac::platform {
namespace {
constexpr const char* kRoot = "sdmc:/switch/openrac/data";
constexpr const char* kTabs[] = {"Play", "Display", "Audio", "Gameplay", "Game fixes", "Saves", "Setup"};
bool console_active = false;
std::string card_root() { return std::string("sdmc:/switch/openrac/memcard/") + openrac_game.serial; }
std::string notice;
bool signature(const std::filesystem::path& path, std::streamoff offset, const char* expected, std::size_t size) {
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
    if (!card_directory(root.c_str())) return false;
    const auto path = root + "/.openrac-write-test";
    const char marker[] = "OpenRAC save storage check";
    FILE* f = std::fopen(path.c_str(), "wb");
    if (!f) return false;
    bool ok = std::fwrite(marker, 1, sizeof marker, f) == sizeof marker;
    if (std::fflush(f) != 0 || fsync(fileno(f)) != 0) ok = false;
    if (std::fclose(f) != 0) ok = false;
    f = std::fopen(path.c_str(), "rb");
    if (f) {
        char readback[sizeof marker]{};
        ok = ok && std::fread(readback, 1, sizeof readback, f) == sizeof readback
            && std::memcmp(marker, readback, sizeof marker) == 0;
        std::fclose(f);
    } else ok = false;
    std::remove(path.c_str());
    return ok;
}
Data inspect() {
    const auto folder = std::filesystem::path(kRoot) / "iso_data" / openrac_game.game;
    const auto levels = std::filesystem::path(kRoot) / "port/pal";
    std::error_code ec;
    return {signature(folder / "disc.iso", 16 * 2048, "\1CD001\1", 7),
        signature(folder / openrac_game.serial, 0, "\177ELF", 4),
        std::filesystem::is_regular_file(levels / "level_99/manifest.json", ec)
            && std::filesystem::is_regular_file(levels / "level_00/manifest.json", ec), check_card()};
}
#ifdef OPENRAC_FRONTEND
using namespace settings;
constexpr const char* kDisplay[] = {"Picture shape", "HUD position", "Render scale", "Anti-aliasing", "Texture filter", "Anisotropy", "Scenery detail", "VSync", "Frame rate", "Frame cap", "Frame counter"};
constexpr const char* kAudio[] = {"Master volume", "Music volume", "Sound effects", "Playback mode"};
constexpr const char* kGameplay[] = {"Language", "Start skips cutscenes", "Detailed logging"};
int rows(int tab) {
    if (tab == 1) return 11;
    if (tab == 2) return 4;
    if (tab == 3) return 3;
    if (tab == 4) return fixes::count();
    return 0;
}
template<class E> void cycle(E& value, int by, int count) {
    value = static_cast<E>((static_cast<int>(value) + by + count) % count);
}
std::string value(int tab, int row) {
    const auto& s = get();
    if (tab == 1) switch (row) {
        case 0: return name(s.aspect); case 1: return name(s.hud);
        case 2: return std::to_string(s.render_scale) + "%";
        case 3: return s.msaa == 1 ? "Off" : std::to_string(s.msaa) + "x";
        case 4: return name(s.texture_filter);
        case 5: return s.anisotropy ? std::to_string(s.anisotropy) + "x" : "Auto";
        case 6: return s.original_lod ? "Original draw distances" : "Full detail";
        case 7: return name(s.vsync); case 8: return frame_rate_name(s);
        case 9: return std::to_string(s.frame_cap); case 10: return name(s.show_fps);
    }
    if (tab == 2) {
        if (row == 0) return std::to_string(s.volume) + "%";
        if (row == 3) return game_option("game_playback", 0) ? "Mono" : "Stereo";
        return std::to_string((game_option(row == 1 ? "game_music_volume" : "game_sound_volume", 1024) * 100 + 512) / 1024) + "%";
    }
    if (tab == 3) {
        constexpr const char* langs[] = {"English", "French", "Spanish", "German", "Italian"};
        if (row == 0) return langs[std::clamp(game_option("game_language", 1), 1, 5) - 1];
        return (row == 1 ? s.skip_cutscenes : s.verbose_log) ? "On" : "Off";
    }
    if (tab == 4) return fixes::on(row) ? "On" : "Off";
    return {};
}
void change(int tab, int row, int by) {
    auto& s = get();
    if (tab == 1) switch (row) {
        case 0: cycle(s.aspect, by, static_cast<int>(Aspect::Custom)); break;
        case 1: cycle(s.hud, by, 2); break;
        case 2: s.render_scale = step_listed(kRenderScales, s.render_scale, by); break;
        case 3: s.msaa = step_listed(kSampleCounts, s.msaa, by); break;
        case 4: cycle(s.texture_filter, by, 2); break;
        case 5: { constexpr int a[] = {0,1,2,4,8,16}; s.anisotropy = step_listed(a, s.anisotropy, by); break; }
        case 6: s.original_lod = !s.original_lod; break;
        case 7: cycle(s.vsync, by, 2); break;
        case 8: cycle(s.frame_rate, by, static_cast<int>(FrameRate::Count)); break;
        case 9: s.frame_cap = std::clamp(s.frame_cap + by * 5, 20, 120); break;
        case 10: step_frame_counter(by); break;
    }
    if (tab == 2) {
        if (row == 0) set_volume(s.volume + by * 5);
        else if (row == 3) s.game_options["game_playback"] = !game_option("game_playback", 0);
        else {
            const char* key = row == 1 ? "game_music_volume" : "game_sound_volume";
            s.game_options[key] = std::clamp(game_option(key, 1024) + by * 51, 0, 1024);
        }
    }
    if (tab == 3) {
        if (row == 0) s.game_options["game_language"] = (game_option("game_language", 1) - 1 + by + 5) % 5 + 1;
        else if (row == 1) s.skip_cutscenes = !s.skip_cutscenes;
        else s.verbose_log = !s.verbose_log;
    }
    if (tab == 4) fixes::set(row, !fixes::on(row));
    notice = save() ? "Settings saved. Applied when you start the game."
        : "Settings could not be saved. Check the SD card before exiting.";
}
#endif
void draw(int tab, int selected, const Data& d) {
    consoleClear();
    std::printf("\x1b[36m\n  OPENRAC-NX  /  RATCHET & CLANK\n  ================================================================\x1b[0m\n");
    std::printf("  %s  |  Native Switch port\n\n", openrac_game.serial);
    for (int i = 0; i < 7; ++i) std::printf(i == tab ? "\x1b[30;46m %s \x1b[0m" : " %s ", kTabs[i]);
    std::printf("\n\n");
#ifdef OPENRAC_FRONTEND
    if (tab >= 1 && tab <= 4) {
        for (int i = 0; i < rows(tab); ++i) {
            const char* label = tab == 1 ? kDisplay[i] : tab == 2 ? kAudio[i] : tab == 3 ? kGameplay[i] : fixes::fix(i).name;
            if (i == selected) std::printf("\x1b[30;46m");
            std::printf("  %c %-36.36s  %-25.25s\x1b[0m\n\n", i == selected ? '>' : ' ', label, value(tab, i).c_str());
        }
        if (tab == 1) std::printf("  Higher resolution / AA costs performance. Vulkan/FSR unavailable.\n");
    } else
#endif
    if (tab == 0) {
        std::printf("  YOUR INSTALLATION\n\n  Disc image        %s\n  Boot executable   %s\n  Exported levels   %s\n  Save storage      %s\n\n", d.disc ? "Ready" : "Missing", d.executable ? "Ready" : "Missing", d.levels ? "Ready" : "Missing", d.writable ? "Write/read check passed" : "FAILED - check SD card");
        std::printf("  [A / ZL] Start game\n\n  [X] Recheck installation and save storage\n\n  [Y] Run 300-frame headless diagnostic\n\n  Hold ZL in game for lock-strafe.\n  Hold L + R + Minus in game to exit.\n");
    } else if (tab == 5) {
        std::printf("  MEMORY CARD 1\n\n  %s\n\n", card_root().c_str());
        std::printf("  SD write, close and read-back: %s\n\n", d.writable ? "PASSED" : "FAILED");
        std::error_code ec;
        int count = 0;
        for (std::filesystem::recursive_directory_iterator it(card_root(), ec), end; it != end && !ec; it.increment(ec)) {
            if (it->is_regular_file(ec)) ++count;
        }
        std::printf("  Stored files: %d%s\n\n", count, ec ? " (unable to read all folders)" : "");
        std::printf("  Save through the game's own Save menu.\n  Wait for saving to finish before exiting or powering off.\n\n  Back up the entire memcard folder to keep your progress.\n  Existing saves are kept when replacing the NRO.\n\n  [X] Recheck storage (does not change your saves)\n");
    } else if (tab == 6) {
        std::printf("  INSTALLATION\n\n  Prepare your own PAL ISO on a PC; no mounting needed.\n  Run tools/extractor.py, then editor/extract.py.\n\n  SD:/switch/openrac/\n    openrac-rac1-pal.nro\n    data/iso_data/rac1/disc.iso\n    data/iso_data/rac1/SCES_509.16\n    data/iso_data/rac1/buildinfo.json\n    data/port/pal/level_XX/  (all exported levels)\n\n  Full instructions: github.com/BIGGZ22/OpenRAC-NX\n  Branch: codex/switch-port\n\n  Start Homebrew Menu in full application mode.\n");
    }
    std::printf("\n\x1b[33m  %.76s\x1b[0m\n", notice.c_str());
    std::printf("\n\x1b[36m  [L/R] Tabs  [Up/Down] Select  [Left/Right/A] Change\n  [B] Play page   [X] Check files   [+] Exit\x1b[0m\n");
}
} // namespace
int switch_launcher() {
    consoleInit(nullptr);
    console_active = true;
#ifdef OPENRAC_FRONTEND
    settings::load("sdmc:/switch/openrac/settings/rac1-pal.ini");
#endif
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);
    PadState pad;
    padInitializeDefault(&pad);
    int tab = 0, selected = 0;
    Data data = inspect();
    bool dirty = true;
    while (appletMainLoop()) {
        padUpdate(&pad);
        const u64 b = padGetButtonsDown(&pad);
        if (b & HidNpadButton_Plus) break;
        if (b & (HidNpadButton_L | HidNpadButton_R)) {
            tab = (tab + ((b & HidNpadButton_R) ? 1 : 6)) % 7;
            selected = 0; dirty = true;
        }
        if (b & HidNpadButton_B) { tab = 0; selected = 0; dirty = true; }
        if (b & HidNpadButton_X) { data = inspect(); notice = data.ready() ? "Installation and save storage ready." : "Some checks failed. See the Play / Saves pages."; dirty = true; }
#ifdef OPENRAC_FRONTEND
        const int n = rows(tab);
        if (n > 0) {
            if (b & HidNpadButton_Up) selected = (selected + n - 1) % n;
            if (b & HidNpadButton_Down) selected = (selected + 1) % n;
            if (b & (HidNpadButton_Left | HidNpadButton_Right | HidNpadButton_A)) change(tab, selected, (b & HidNpadButton_Left) ? -1 : 1);
            if (b) dirty = true;
        }
        if ((b & HidNpadButton_ZL) || (tab == 0 && (b & HidNpadButton_A))) {
            data = inspect();
            if (data.ready()) { consoleExit(nullptr); console_active = false; return 2; }
            notice = "Cannot start: installation or save-storage check failed."; dirty = true;
        }
#endif
        if (tab == 0 && (b & HidNpadButton_Y) && data.disc && data.executable) return 1;
        if (dirty) { draw(tab, selected, data); dirty = false; }
        consoleUpdate(nullptr);
    }
    consoleExit(nullptr);
    console_active = false;
    return 0;
}
void switch_update_console() { if (console_active) consoleUpdate(nullptr); }
} // namespace openrac::platform
