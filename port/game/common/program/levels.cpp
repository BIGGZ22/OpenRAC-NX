// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// The levels as the program reads them: each level's program, for the runtime's relocation of the
// functions levels share (load_level_program), and --check-levels, which reads every extracted
// level as the window would and stamps the folder when all are whole (check_levels).

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <format>
#include <optional>
#include <string>
#include <system_error>
#include <vector>

#include "common/log.h"
#include "openrac/elf.h"
#include "openrac/game_host.h"
#include "openrac/guest.h"
#include "openrac/memory.h"
#include "program/program.h"

#ifdef OPENRAC_HAS_DISC
#include "assets/disc/disc.h"
#endif
#ifdef OPENRAC_FRONTEND
#include "scene/level.h"
#endif

namespace openrac::program {
namespace {

// A level's program straight from the player's image (<data>/iso_data/<game>/disc.iso, kept by the
// set-up), for rac1, whose disc layout is known. The image is opened once.
bool level_program_from_disc(int level, std::vector<std::uint8_t>* bytes) {
#ifdef OPENRAC_HAS_DISC
    static std::optional<assets::disc::Disc> disc;
    static bool tried = false;

    if (!tried) {
        tried = true;
        const fs::path image = installed_disc(g_options.data);

        if (std::string(openrac_game.game) == "rac1") {
            try {
                disc.emplace(assets::disc::Disc::open(image, assets::disc::rac1_layout()));
            } catch (const std::exception& e) {
                log::warn(
                    "the level programs cannot be read from {}: {}", image.string(), e.what()
                );
            }
        }
    }

    if (!disc) {
        return false;
    }

    try {
        assets::disc::LevelFiles files = disc->level(static_cast<std::uint32_t>(level));

        if (files.overlay) {
            *bytes = std::move(*files.overlay);
            return true;
        }
    } catch (const std::exception& e) {
        log::warn("level {}'s program cannot be read from the disc image: {}", level, e.what());
    }
#else
    (void)level;
    (void)bytes;
#endif
    return false;
}

}  // namespace

void load_level_program(int level) {
    char name[8];
    std::snprintf(name, sizeof(name), "%02d", level);
    const fs::path path =
        g_options.data / "decompiler_out" / openrac_game.game / "levels" / name / "overlay.bin";
    std::vector<std::uint8_t> bytes;

    if (!runtime::read_file(path, &bytes) && !level_program_from_disc(level, &bytes)) {
        log::error(
            "level {}'s program is in neither {} nor the disc image: the functions levels share "
            "are not "
            "relocated, and the game will misbehave (the hero's heading ignores the camera)",
            level,
            path.string()
        );
        return;
    }

    log::verbose("level {}'s program: {} bytes", level, bytes.size());
    std::uint32_t entry = 0;

    for (std::size_t at = 0; at + 16 <= bytes.size();) {
        std::uint32_t head[4];
        std::memcpy(head, bytes.data() + at, sizeof(head));

        if (entry == 0) {
            entry = head[3];
        }

        if (head[3] != entry || at + 16 + head[1] > bytes.size()) {
            break;
        }

        openrac_guest_add_level_program(level, head[0], bytes.data() + at + 16, head[1]);
        at += 16 + head[1];
    }
}

#ifdef OPENRAC_FRONTEND
int check_levels(const fs::path& levels) {
    const fs::path stamp = levels / std::format("verified-{}.json", scene::kManifestFormat);
    std::error_code ignored;
    fs::remove(stamp, ignored);
    fs::remove(levels / "verified.json", ignored);  // the stamp before formats were named
    std::vector<int> ids;

    for (int i = 0; i < 19; ++i) {
        ids.push_back(i);
    }

    ids.push_back(98);
    ids.push_back(99);
    int bad = 0;
    int newer = 0;  // levels of a newer format than this game reads: the game is out of date

    for (const int id : ids) {
        const fs::path dir = levels / std::format("level_{:02d}", id);
        scene::LevelData level;
        std::string error;

        if (!scene::load_level(dir, level, error)) {
            log::error("level {:02d}: {}", id, error);
            ++bad;
            newer += error.find("newer than this game") != std::string::npos;
            continue;
        }

        // The flight world has only its ships' textures; every other one has ground to stand on.
        if (id != 98 && (level.vertices.empty() || level.models.empty())) {
            log::error(
                "level {:02d}: it has nothing to draw ({} models, {} vertices)",
                id,
                level.models.size(),
                level.vertices.size()
            );
            ++bad;
            continue;
        }

        if (level.missing_images > 0) {
            log::error("level {:02d}: {} of its textures are missing", id, level.missing_images);
            ++bad;
            continue;
        }

        log::info(
            "level {:02d}: {} models, {} vertices, {} textures",
            id,
            level.models.size(),
            level.vertices.size(),
            level.images.size()
        );
    }

    if (newer > 0) {
        log::error(
            "{} of {} levels in {} are of a newer format than this game reads: build the game "
            "again (the "
            "launcher's \"Build the game\"; tools/build_port.py)",
            newer,
            ids.size(),
            levels.string()
        );
        return 1;
    }

    if (bad > 0) {
        log::error(
            "{} of {} levels in {} cannot be drawn: read them from the disc again (the launcher's "
            "\"Get the levels ready\"; editor/extract.py port --replace)",
            bad,
            ids.size(),
            levels.string()
        );
        return 1;
    }

    if (std::FILE* f = std::fopen(stamp.string().c_str(), "wb")) {
        const std::string text = std::format(
            "{{\"format\": {}, \"levels\": {}, \"program\": \"{}\"}}\n",
            scene::kManifestFormat,
            ids.size(),
            program_name()
        );
        std::fwrite(text.data(), 1, text.size(), f);
        std::fclose(f);
    } else {
        log::error("cannot write {}", stamp.string());
        return 1;
    }

    log::info("all {} levels can be drawn: {}", ids.size(), stamp.string());
    return 0;
}
#endif

}  // namespace openrac::program
