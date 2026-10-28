// SPDX-License-Identifier: GPL-3.0-or-later
// Controller-operated setup dashboard. The game frontend is not yet ported.
#include "platform/switch/launcher.h"

#include <switch.h>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>

#include "openrac/game_host.h"

namespace openrac::platform {
namespace {
constexpr const char* kRoot = "sdmc:/switch/openrac/data";
enum class Page { Home, Setup, Status };
bool console_active = false;

bool signature(const std::filesystem::path& path, std::streamoff offset,
               const char* expected, std::size_t size) {
    std::ifstream file(path, std::ios::binary);
    char bytes[8]{};
    file.seekg(offset);
    return size <= sizeof(bytes) && file.read(bytes, static_cast<std::streamsize>(size))
        && std::memcmp(bytes, expected, size) == 0;
}

struct Data {
    bool disc = false;
    bool executable = false;
    bool levels = false;
    bool ready() const { return disc && executable; }
};

Data inspect() {
    const auto folder = std::filesystem::path(kRoot) / "iso_data" / openrac_game.game;
    // ISO9660 primary descriptor: type 1, CD001, version 1, at sector 16.
    std::error_code ec;
    const auto levels = std::filesystem::path(kRoot) / "port" / "pal";
    const bool exported = std::filesystem::is_regular_file(levels / "level_99/manifest.json", ec)
        && std::filesystem::is_regular_file(levels / "level_00/manifest.json", ec);
    return {signature(folder / "disc.iso", 16 * 2048, "\1CD001\1", 7),
            signature(folder / openrac_game.serial, 0, "\177ELF", 4), exported};
}

void draw(Page page, const Data& data) {
    consoleClear();
    std::printf("\x1b[36m\n  OPENRAC                                      SWITCH / EARLY BUILD\n");
    std::printf("  ========================================================================\n\x1b[0m");
    std::printf("\n  RATCHET & CLANK\n  %s  |  %s\n", openrac_game.id, openrac_game.serial);
    std::printf("\n\x1b[33m  SETUP DASHBOARD - GAMEPLAY SUPPORT IN PROGRESS\x1b[0m\n\n");

    if (page == Page::Home) {
        std::printf("  YOUR DISC\n  %s\n\n", data.ready()
            ? "Disc and boot executable found. Header checks passed."
            : "Setup incomplete. Open the setup guide, then check again.");
        std::printf("  [A] Check SD-card files\n\n  [X] Disc setup guide\n\n  [Y] Port status\n\n");
        std::printf("  [R] Run 300-frame boot test (no graphics; needs disc files)\n\n");
#ifdef OPENRAC_FRONTEND
        std::printf("  [ZL] Start game - experimental OpenGL build\n");
        std::printf("  In game: hold ZL for lock-strafe; release for normal movement.\n");
        std::printf("  Console testing required; not yet verified playable.\n");
#else
        std::printf("  PLAY: unavailable in this headless build\n");
#endif
    } else if (page == Page::Setup) {
        std::printf("  DISC SETUP / NO MOUNTING REQUIRED\n\n");
        std::printf("  1. On your PC, extract your PAL ISO with tools/extractor.py.\n");
        std::printf("     The full command is in port/platform/switch/README.md.\n\n");
        std::printf("  2. Copy the prepared iso_data folder to this SD-card location:\n");
        std::printf("     /switch/openrac/data/iso_data/%s/\n\n", openrac_game.game);
        std::printf("  3. That folder must contain:\n");
        std::printf("     disc.iso       (the complete ISO; do not split it)\n");
        std::printf("     %-14s (the extracted boot executable)\n", openrac_game.serial);
        std::printf("     buildinfo.json (the extractor's validation record)\n\n");
        std::printf("  Use the matching PAL disc for this build.\n");
        std::printf("  The PC extractor validates the disc; this menu checks headers only.\n");
    } else {
        std::printf("  PORT STATUS\n\n");
        std::printf("  Launcher and controller navigation       AVAILABLE\n");
        std::printf("  ISO9660 header                          %s\n", data.disc ? "FOUND" : "MISSING / INVALID");
        std::printf("  Boot ELF header                         %s\n", data.executable ? "FOUND" : "MISSING / INVALID");
        std::printf("  Title and first level assets            %s\n", data.levels ? "FOUND" : "MISSING");
#ifdef OPENRAC_FRONTEND
        std::printf("  SDL2 / Mesa OpenGL renderer              BUILT / DEVICE TEST NEEDED\n");
#else
        std::printf("  Game renderer                           NOT IN THIS BUILD\n");
#endif
        std::printf("  Game controller input                   BUILT / DEVICE TEST NEEDED\n");
        std::printf("  Compact guest memory (under 50 MB)       HOST TESTS PASSED\n\n");
        std::printf("  Data root: %s\n\n", kRoot);
        std::printf("  A successful build does not mean the game is playable yet.\n");
    }
    std::printf("\n\x1b[36m  ------------------------------------------------------------------------\n");
    std::printf("  [B] Home     [A] Refresh checks     [+] Exit\x1b[0m\n");
}
} // namespace

int switch_launcher() {
    consoleInit(nullptr);
    console_active = true;
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);
    PadState pad;
    padInitializeDefault(&pad);
    Page page = Page::Home;
    Data data = inspect();
    bool dirty = true;
    while (appletMainLoop()) {
        padUpdate(&pad);
        const u64 buttons = padGetButtonsDown(&pad);
        if (buttons & HidNpadButton_Plus) break;
#ifdef OPENRAC_FRONTEND
        if ((buttons & HidNpadButton_ZL) && data.ready() && data.levels) {
            consoleExit(nullptr);
            console_active = false;
            return 2;
        }
#endif
        if ((buttons & HidNpadButton_R) && data.ready()) {
            consoleClear();
            std::printf("OpenRAC boot test: up to 300 game frames, no game graphics.\n"
                        "Hold L + R + Minus to exit.\n"
                        "Session log: /switch/openrac/data/openrac-rac1-pal.log\n");
            consoleUpdate(nullptr);
            // Keep the console alive for the headless game and its diagnostics.
            return 1;
        }
        if (buttons & HidNpadButton_B) { page = Page::Home; dirty = true; }
        if (buttons & HidNpadButton_X) { page = Page::Setup; dirty = true; }
        if (buttons & HidNpadButton_Y) { page = Page::Status; dirty = true; }
        if (buttons & HidNpadButton_A) { data = inspect(); page = Page::Status; dirty = true; }
        if (dirty) { draw(page, data); dirty = false; }
        consoleUpdate(nullptr);
    }
    consoleExit(nullptr);
    console_active = false;
    return 0;
}
void switch_update_console() {
    if (console_active) consoleUpdate(nullptr);
}
} // namespace openrac::platform
