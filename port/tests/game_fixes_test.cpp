// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The game fixes (game/common/fixes.h) of Ratchet & Clank's PAL disc, from port/game/rac1/differences.json
// as cmake/Differences.cmake wrote them (the configure checked the file itself): a default reaches
// its setting; the settings file switches one off and keeps it; the flag the code reads follows.
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

#include "fixes.h"
#include "openrac/game_host.h"
#include "settings.h"

#define CHECK(c)                                                    \
    do {                                                            \
        if (!(c)) {                                                 \
            std::fprintf(stderr, "%d: %s\n", __LINE__, #c);         \
            std::exit(1);                                           \
        }                                                           \
    } while (0)

namespace fs = std::filesystem;
using namespace openrac;

namespace {

std::string read(const fs::path& path) {
    std::ifstream in(path);
    std::stringstream text;
    text << in.rdbuf();
    return text.str();
}

void write(const fs::path& path, const std::string& text) {
    std::ofstream(path, std::ios::trunc) << text;
}

}  // namespace

int main() {
    // The table: rac1's three wide-screen fixes, all on by default (on is the port's fix).
    CHECK(fixes::count() >= 3);
    for (const char* key : {"fix_wide_moby_effects", "fix_wide_nanotech_orbs", "fix_wide_liquids"}) {
        const int i = fixes::find(key);
        CHECK(i >= 0);
        CHECK(fixes::fix(i).on);
        CHECK(std::strlen(fixes::fix(i).name) > 0);
    }
    CHECK(fixes::find("fix_not_a_fix") < 0);

    const fs::path dir = fs::temp_directory_path() / "openrac_game_fixes_test";
    fs::remove_all(dir);
    fs::create_directories(dir);
    const fs::path file = dir / "rac1-pal.ini";
    const unsigned char* liquids = openrac_game_fix("fix_wide_liquids");
    CHECK(liquids != nullptr);

    // No file: the defaults, and the flag the code reads says so.
    settings::load(file);
    CHECK(fixes::on(fixes::find("fix_wide_liquids")));
    CHECK(*liquids == 1);

    // Saved: every fix is written with its value.
    settings::save();
    const std::string saved = read(file);
    CHECK(saved.find("fix_wide_moby_effects = 1") != std::string::npos);
    CHECK(saved.find("fix_wide_liquids = 1") != std::string::npos);

    // Switched off in the file (as the launcher writes it): the console's behaviour, through the
    // same pointer; the other fixes keep their defaults; an unknown fix key is ignored.
    write(file, "aspect = 16:9\nfix_wide_liquids = 0\nfix_from_elsewhere = 0\n");
    settings::load(file);
    CHECK(*liquids == 0);
    CHECK(!fixes::on(fixes::find("fix_wide_liquids")));
    CHECK(fixes::on(fixes::find("fix_wide_moby_effects")));
    CHECK(settings::summary().find("as the console: fix_wide_liquids") != std::string::npos);

    // A value the port does not know: the default.
    write(file, "fix_wide_moby_effects = maybe\n");
    settings::load(file);
    CHECK(fixes::on(fixes::find("fix_wide_moby_effects")));

    // Off, saved and read again: still off.
    fixes::set(fixes::find("fix_wide_nanotech_orbs"), false);
    settings::save();
    CHECK(read(file).find("fix_wide_nanotech_orbs = 0") != std::string::npos);
    settings::load(file);
    CHECK(!fixes::on(fixes::find("fix_wide_nanotech_orbs")));

    // A key the table does not have reads as the port's behaviour.
    CHECK(*openrac_game_fix("fix_not_a_fix") == 1);

    fs::remove_all(dir);
    std::puts("game fixes: ok");
    return 0;
}
