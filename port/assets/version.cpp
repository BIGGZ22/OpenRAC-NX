// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

#include "assets/version.h"

#include <array>

namespace openrac::assets {

namespace {

// Written by cmake/Assets.cmake from games/*/game.json.
constexpr GameVersion kVersions[] = {
#include "assets_versions.inc"
};

struct SerialGame {
    std::string_view serial;
    Game game;
};

// Every boot executable name (the product serial) of the series' releases:
// other regions, reprints, demos. A disc with one of these is recognised as
// the game but is not a build OpenRAC can set up yet. From ReRAC's build
// table (crates/rc-extract/src/build_db.rs, ISC, THIRD_PARTY_NOTICES.md).
constexpr SerialGame kOtherSerials[] = {
    {"SCUS_971.99", Game::Rac1}, {"SCUS_972.09", Game::Rac1}, {"SCUS_972.40", Game::Rac1},
    {"SCES_509.16", Game::Rac1}, {"SCED_510.75", Game::Rac1}, {"SCPS_150.37", Game::Rac1},
    {"SCPS_150.56", Game::Rac2}, {"SCES_516.07", Game::Rac2}, {"SCUS_972.68", Game::Rac2},
    {"SCUS_973.22", Game::Rac2}, {"SCUS_973.23", Game::Rac2}, {"SCUS_973.74", Game::Rac2},
    {"SCKA_200.11", Game::Rac2}, {"PAPX_905.20", Game::Rac3}, {"SCED_528.47", Game::Rac3},
    {"SCED_528.48", Game::Rac3}, {"SCES_524.56", Game::Rac3}, {"SCPS_150.84", Game::Rac3},
    {"SCUS_973.53", Game::Rac3}, {"SCUS_974.11", Game::Rac3}, {"SCUS_974.13", Game::Rac3},
    {"TCES_524.56", Game::Rac3}, {"SCKA_200.37", Game::Rac3}, {"PCPX_980.17", Game::Rac4},
    {"SCED_536.60", Game::Rac4}, {"SCES_532.85", Game::Rac4}, {"SCPS_150.99", Game::Rac4},
    {"SCPS_151.00", Game::Rac4}, {"SCUS_974.65", Game::Rac4}, {"SCUS_974.85", Game::Rac4},
    {"SCUS_974.87", Game::Rac4}, {"SCKA_200.60", Game::Rac4},
};

}  // namespace

std::string_view game_name(Game game) {
    switch (game) {
        case Game::Rac1:
            return "rac1";
        case Game::Rac2:
            return "rac2";
        case Game::Rac3:
            return "rac3";
        case Game::Rac4:
            return "rac4";
    }

    return "unknown";
}

std::string_view game_title(Game game) {
    switch (game) {
        case Game::Rac1:
            return "Ratchet & Clank";
        case Game::Rac2:
            return "Ratchet & Clank: Going Commando";
        case Game::Rac3:
            return "Ratchet & Clank: Up Your Arsenal";
        case Game::Rac4:
            return "Ratchet: Deadlocked";
    }

    return "unknown";
}

std::string_view region_name(Region region) {
    switch (region) {
        case Region::NtscU:
            return "NTSC-U";
        case Region::Pal:
            return "PAL";
        case Region::NtscJ:
            return "NTSC-J";
    }

    return "unknown";
}

std::span<const GameVersion> versions() {
    return kVersions;
}

const GameVersion* find_version_by_boot(std::string_view serial, std::string_view boot_sha1) {
    for (const GameVersion& v : kVersions) {
        if (v.serial == serial && v.boot_sha1 == boot_sha1) {
            return &v;
        }
    }

    return nullptr;
}

const GameVersion* find_version(std::string_view id_or_key) {
    for (const GameVersion& v : kVersions) {
        if (v.id == id_or_key || v.key == id_or_key) {
            return &v;
        }
    }

    return nullptr;
}

bool game_of_serial(std::string_view serial, Game& game) {
    for (const GameVersion& v : kVersions) {
        if (v.serial == serial) {
            game = v.game;
            return true;
        }
    }

    for (const SerialGame& s : kOtherSerials) {
        if (s.serial == serial) {
            game = s.game;
            return true;
        }
    }

    return false;
}

}  // namespace openrac::assets
