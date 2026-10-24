// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The game program's command line and default paths (program/options.h): the parser, the usage
// it prints, and where a player's data lives when no option says (the folders the launcher uses).

#include "program/options.h"

#include <cstdlib>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>

#include "common/env.h"
#include "common/log.h"
#include "openrac/game_host.h"

namespace openrac::program {
namespace {

// Logs `why` and the usage line, and ends the program with exit code 64 (EX_USAGE, the usage
// error of BSD's sysexits.h).
[[noreturn]] void usage(const std::string& why) {
    log::error("{}", why);
    log::error(
        "usage: {} --data <install>/active/{}/data [--cards DIR] [--frames N] [--stop-on-missing] "
        "[--level N] [--skip-movies] [--mods DIR] [--settings FILE] [--plugins FILE] [--probes "
        "DIR] [--no-mods] [--debug-overlay] [--debug-dump FILE]",
        program_name(),
        openrac_game.game
    );
    std::exit(64);
}

// OpenRAC's folder in the player's data home, as the launcher finds it
// (launcher/core/src/saves.rs, get_memcard_dir).
fs::path data_home() {
#if defined(__APPLE__)
    const std::optional<std::string> home = env::system_variable("HOME");
    fs::path base = fs::path(home.value_or(".")) / "Library/Application Support/OpenRAC";
#else
    fs::path base;

    if (const std::optional<std::string> xdg = env::system_variable("XDG_DATA_HOME");
        xdg && !xdg->empty()) {
        base = fs::path(*xdg) / "openrac";
    } else {
        const std::optional<std::string> home = env::system_variable("HOME");
        base = fs::path(home.value_or(".")) / ".local/share/openrac";
    }
#endif
    return base;
}

// Where the first memory card lives unless --cards says otherwise: the folder
// the launcher's save manager reads and backs up: <data home>/openrac/memcard/<serial>.
fs::path default_cards() {
    return data_home() / "memcard" / openrac_game.serial;
}

// The player's settings unless --settings says otherwise: <data home>/openrac/settings/<id>.ini.
fs::path default_settings() {
    return data_home() / "settings" / (std::string(openrac_game.id) + ".ini");
}

}  // namespace

std::string program_name() {
    return std::string("openrac-") + openrac_game.id;
}

std::string game_version() {
    const std::string id = openrac_game.id;
    const auto dash = id.find('-');
    return dash == std::string::npos ? id : id.substr(dash + 1);
}

fs::path levels_or_flat(const fs::path& own) {
    std::error_code ec;
    const fs::path flat = own.parent_path();

    if (game_version() == "pal" && own.filename() == "pal"
        && !fs::exists(own / "level_99" / "manifest.json", ec)
        && fs::exists(flat / "level_99" / "manifest.json", ec)) {
        log::warn(
            "levels: reading the ones extracted before each version had a folder, in {} (extract "
            "again to "
            "move them into {})",
            flat.string(),
            own.string()
        );
        return flat;
    }

    return own;
}

fs::path default_levels(const fs::path& data) {
    return levels_or_flat(data / "port" / game_version());
}

Options parse(int argc, char** argv) {
    Options o;

    for (int i = 1; i < argc; i++) {
        const std::string a = argv[i];
        auto value = [&]() -> std::string {
            if (i + 1 >= argc) {
                usage("missing value for " + a);
            }

            return argv[++i];
        };

        if (a == "--data") {
            o.data = value();
        } else if (a == "--cards") {
            o.cards = value();
        } else if (a == "--no-card") {
            openrac_game_no_card = 1;
        } else if (a == "--window") {
            o.window = true;
        } else if (a == "--mods") {
            o.mods = value();
        } else if (a == "--settings") {
            o.settings = value();
        } else if (a == "--plugins") {
            o.plugins = value();
        } else if (a == "--probes") {
            o.probes = value();
        } else if (a == "--no-mods") {
            o.no_mods = true;
        } else if (a == "--levels") {
            o.levels = value();
        } else if (a == "--frames") {
            o.frames = std::stol(value());
        } else if (a == "--level") {
            openrac_game_start_level = std::stoi(value());
        } else if (a == "--skip-movies") {
            o.skip_movies = true;
        } else if (a == "--keep-going") {
            o.keep_going = true;
        } else if (a == "--stop-on-missing") {
            o.keep_going = false;
        } else if (a == "--debug-dump") {
            o.debug_dump = value();
        } else if (a == "--debug-overlay") {
            o.debug_overlay = true;
        } else if (a == "--check-levels") {
            o.check_levels = true;
        } else if (a == "--help" || a == "-h") {
            usage(program_name() + ": " + openrac_game.title + ", native");
        } else {
            usage("unknown option " + a);
        }
    }

    if (o.data.empty()) {
        if (const std::optional<std::string_view> data = env::value(env::Var::Data)) {
            o.data = *data;
        } else {
            usage(
                std::string("no data folder: pass --data, the folder the extractor wrote for ")
                + openrac_game.game
            );
        }
    }

    if (o.cards.empty()) {
        o.cards = default_cards();
    }

    if (o.settings.empty()) {
        o.settings = default_settings();
    }

    if (o.plugins.empty()) {
        o.plugins = o.data / "plugins" / "plugins.json";
    }

    return o;
}

std::string command_line(int argc, char** argv) {
    std::string line;

    for (int i = 0; i < argc; ++i) {
        std::string a = i == 0 ? program_name() : std::string(argv[i]);

        if (a.find_first_of(" \"'") != std::string::npos || a.empty()) {
            a = "\"" + a + "\"";
        }

        line += (i == 0 ? "" : " ") + a;
    }

    return line;
}

}  // namespace openrac::program
