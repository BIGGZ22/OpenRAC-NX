// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// openrac-<game>: one of the games, native. Every game's program is this
// file, linked with that game's translated C and its description
// (openrac_game, generated from port/game/<id>/hostgen.json).
//
//   openrac-<id> --data <install>/active/<game>/data [--cards DIR] [--frames N] [--stop-on-missing]
//                [--level N] [--skip-movies] [--mods DIR] [--settings FILE]
//
// The data folder is what the extractor made from the player's disc
// (tools/extractor.py; the launcher's "Set up from your disc"). The program
// does what the console's loader and the game's start-up code did, then runs
// the game's own main, translated from its decompilation:
//
//   1. game memory, laid out as the console's (port/runtime);
//   2. the executable's data copied to its addresses, from the player's copy
//      of it (its code bytes come along and are never run);
//   3. the game's functions registered by code address, with the level
//      program that is loaded deciding which ones a level address means;
//   4. the game's main.
//
// A function that has no C yet (still assembly in the decompilation, or a
// library not replaced yet) is logged and returns, and the game carries on;
// --stop-on-missing stops at the first one instead and says which, for checks.

#include <chrono>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <filesystem>
#include <memory>
#include <optional>
#include <span>
#include <algorithm>
#include <format>
#include <string>
#include <thread>
#include <vector>

#include "common/log.h"
#include "openrac/elf.h"
#include "openrac/game_host.h"
#include "openrac/guest.h"
#include "openrac/memory.h"

#ifdef OPENRAC_FRONTEND
#include "frontend.h"
#include "settings.h"
#include "openrac/sound_engine.h"
#include "platform/audio.h"
#include "platform/pump.h"
#include "renderer/texture.h"
#include "viewer/moby_pose.h"
#endif
#ifdef OPENRAC_MOVIES
#include "media/movie.h"
#include "platform/audio.h"
#endif

extern "C" {
const char* openrac_game_disc_image = nullptr;
const char* openrac_game_card_dir = nullptr;
const char* openrac_game_mods_dir = nullptr;
int openrac_game_language = 1;
int openrac_game_no_card = 0;
int openrac_game_start_level = -1;
void (*openrac_game_on_frame)(unsigned frame) = nullptr;
int (*openrac_game_bolts)(void) = nullptr;
}

namespace {

namespace fs = std::filesystem;
using namespace openrac;

struct Options {
    fs::path data;
    fs::path cards;
    long frames = -1;
    bool keep_going = true;
    bool window = false;
    bool skip_movies = false;  // development probes; not movie-playback validation
    fs::path levels;  // the extracted levels the window draws (level_00, ...)
    fs::path mods;  // levels edited in Godot and packed (editor/pack.py), read in place of the disc's
    fs::path settings;  // the player's settings (settings.h); in OpenRAC's data folder unless given
};

Options g_options;
long g_frame = 0;
gaddr g_chain = 0;  // the display list sent this frame
bool g_vsync_since_kick = true;  // a vertical blank was waited for since the last frame was sent
bool g_window = false;
std::chrono::steady_clock::time_point g_next_frame;

std::string program_name() {
    return std::string("openrac-") + openrac_game.id;
}

[[noreturn]] void usage(const std::string& why) {
    log::error("{}", why);
    log::error(
        "usage: {} --data <install>/active/{}/data [--cards DIR] [--frames N] [--stop-on-missing] [--level N] [--skip-movies] [--mods DIR] [--settings FILE]",
        program_name(),
        openrac_game.game
    );
    std::exit(64);
}

// OpenRAC's folder in the player's data home, as the launcher finds it
// (launcher/core/src/saves.rs, get_memcard_dir).
fs::path data_home() {
#if defined(__APPLE__)
    const char* home = std::getenv("HOME");
    fs::path base = fs::path(home != nullptr ? home : ".") / "Library/Application Support/OpenRAC";
#else
    fs::path base;
    if (const char* xdg = std::getenv("XDG_DATA_HOME"); xdg != nullptr && xdg[0] != '\0') {
        base = fs::path(xdg) / "openrac";
    } else {
        const char* home = std::getenv("HOME");
        base = fs::path(home != nullptr ? home : ".") / ".local/share/openrac";
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
        } else if (a == "--help" || a == "-h") {
            usage(program_name() + ": " + openrac_game.title + ", native");
        } else {
            usage("unknown option " + a);
        }
    }
    if (o.data.empty()) {
        if (const char* env = std::getenv("OPENRAC_DATA")) {
            o.data = env;
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
    return o;
}

void finish() {
    openrac_guest_report_missing();
    log::info("{} frames", g_frame);
}

}  // namespace

// ---- What the library replacements call (openrac/game_host.h) ----

extern "C" {

// What the game's renderers drew since the last frame (openrac_game_draw), and whether it reports.
static unsigned g_draws = 0;
static bool g_reports_draws = false;

void openrac_game_draw(unsigned layers) {
    g_draws |= layers;
    g_reports_draws = true;
#ifdef OPENRAC_FRONTEND
    // The world's terrain: the camera it is drawn with is the frame's world camera.
    if (g_window && (layers & OPENRAC_DRAW_TERRAIN) != 0) {
        const auto* ram = runtime::Memory::get().base();
        frontend::world_drawn(std::span<const std::uint8_t>(ram, 32u * 1024 * 1024));
    }
#endif
}

// OPENRAC_WRITE=FRAME:ADDRESS:VALUE[:FRAMES],...: a 32-bit word written into the game's memory at
// each vertical blank from FRAME for FRAMES blanks (default 1), hexadecimal address and value; the
// old interpreter's --write, for getting somewhere directly (0x15EE84, the level being played, set
// around a new game's transition loads that level).
static void apply_writes() {
    static const char* script = std::getenv("OPENRAC_WRITE");
    if (script == nullptr) {
        return;
    }
    for (const char* at = script; at && *at;) {
        unsigned long frame = 0, address = 0, value = 0, length = 1;
        if (std::sscanf(at, "%lu:%lx:%lx:%lu", &frame, &address, &value, &length) >= 3
            && g_frame >= frame && g_frame < frame + length) {
            const std::uint32_t word = static_cast<std::uint32_t>(value);
            std::memcpy(G(static_cast<gaddr>(address)), &word, 4);
        }
        at = std::strchr(at, ',');
        at = at ? at + 1 : nullptr;
    }
}

int openrac_game_vsync(void) {
    g_vsync_since_kick = true;
    apply_writes();
    // With a window, the renderer draws the frame from the game's memory; without one, a frame is
    // only paced, at the game's frame rate.
#ifdef OPENRAC_FRONTEND
    if (g_window) {
        // The tick about to be drawn was due at the time the last one set (frames between ticks
        // count from it).
        const auto period = std::chrono::microseconds(
            1000000 / (openrac_game.frame_rate > 0 ? openrac_game.frame_rate : 60));
        const auto now = std::chrono::steady_clock::now();
        frontend::set_tick(g_next_frame.time_since_epoch().count() == 0 || now > g_next_frame + period
                               ? now : g_next_frame, period);
        const auto* ram = runtime::Memory::get().base();
        const std::uint32_t draws = g_reports_draws ? g_draws : ~0u;
        g_draws = 0;
        if (!frontend::frame(std::span<const std::uint8_t>(ram, 32u * 1024 * 1024), g_chain, draws)) {
            std::exit(0);
        }
    }
#endif
    g_frame++;
    // The level being played, once per change: the launcher reads this line for Discord's presence.
    static int shown_level = OPENRAC_OVERLAY_EXE;
    static int shown_bolts = -1;
    static long bolts_frame = 0;
    if (const int level = openrac_game_loaded_overlay(); level != shown_level) {
        shown_level = level;
        shown_bolts = -1;
        if (level != OPENRAC_OVERLAY_EXE) {
            log::info("now playing level {}", level);
        }
    }
    // Its bolts too, when they change, at most every 600 frames: Discord takes few updates.
    if (shown_level != OPENRAC_OVERLAY_EXE && openrac_game_bolts != nullptr &&
        (shown_bolts < 0 || g_frame >= bolts_frame + 600)) {
        if (const int bolts = openrac_game_bolts(); bolts >= 0 && bolts != shown_bolts) {
            shown_bolts = bolts;
            bolts_frame = g_frame;
            log::info("bolts {}", bolts);
        }
    }
    if (openrac_game_on_frame != nullptr) {
        openrac_game_on_frame(static_cast<unsigned>(g_frame));
    }
    if (g_options.frames >= 0 && g_frame >= g_options.frames) {
        std::exit(0);  // finish() runs at exit
    }
    using namespace std::chrono;
    const auto now = steady_clock::now();
    // OPENRAC_FPS: the frames per second over each 300 frames, in the log. OPENRAC_UNCAPPED: no wait
    // for the next frame's time (the game then runs as fast as the machine allows: a measure, not a
    // way to play, since the game's tick is its frame).
    static const bool fps = std::getenv("OPENRAC_FPS") != nullptr;
    static const bool uncapped = std::getenv("OPENRAC_UNCAPPED") != nullptr;
    static steady_clock::time_point fps_from = now;
    // OPENRAC_WATCH=ADDRESS:COUNT[:i],...: COUNT floats (or words, with i) from each ADDRESS, each
    // frame, in the log.
    static const char* watch = std::getenv("OPENRAC_WATCH");
    if (watch != nullptr) {
        std::string line;
        for (const char* at = watch; at && *at;) {
            unsigned long address = 0, count = 1;
            char kind = 'f';
            std::sscanf(at, "%lx:%lu:%c", &address, &count, &kind);
            for (unsigned long i = 0; i < count && i < 32; ++i) {
                std::uint32_t w;
                std::memcpy(&w, G(static_cast<gaddr>(address + 4 * i)), 4);
                float f;
                std::memcpy(&f, &w, 4);
                line += kind == 'i' ? std::format(" {}", static_cast<std::int32_t>(w)) : std::format(" {:.4f}", f);
            }
            line += " |";
            at = std::strchr(at, ',');
            at = at ? at + 1 : nullptr;
        }
        log::info("frame {} watch{}", g_frame, line);
    }
    // OPENRAC_HITCH: each frame that took more than twice the frame time, with how long, in the log.
    static const bool hitch = std::getenv("OPENRAC_HITCH") != nullptr;
    static steady_clock::time_point last = now;
    if (hitch) {
        const double ms = duration<double, std::milli>(now - last).count();
        if (ms > 1.25 * 1000.0 / (openrac_game.frame_rate > 0 ? openrac_game.frame_rate : 60)) {
            log::info("frame {}: {:.1f} ms", g_frame, ms);
        }
    }
    last = now;
    if (fps && g_frame % 300 == 0) {
        const double seconds = duration<double>(now - fps_from).count();
        if (seconds > 0.0) {
            log::info("frame {}: {:.1f} frames per second", g_frame, 300.0 / seconds);
        }
        fps_from = now;
    }
    if (uncapped) {
        return static_cast<int>(g_frame & 1);
    }
    if (g_next_frame.time_since_epoch().count() == 0 || now > g_next_frame + milliseconds(100)) {
        g_next_frame = now;
    }
    const auto period = microseconds(1000000 / (openrac_game.frame_rate > 0 ? openrac_game.frame_rate : 60));
    g_next_frame += period;
#ifdef OPENRAC_FRONTEND
    // Until the next tick is due, frames at the display's rate (each swap waits for a refresh),
    // the scene moved on from the last tick's to this one's by when each is shown.
    if (g_window && frontend::wants_between()) {
        for (int drawn; (drawn = frontend::between(g_next_frame)) != 0;) {
            if (drawn < 0) {
                std::exit(0);
            }
        }
    }
#endif
    std::this_thread::sleep_until(g_next_frame);
    return static_cast<int>(g_frame & 1);
}

void openrac_game_dma_send(gaddr channel, gaddr tag) {
    g_chain = tag;
    // A frame sent to VIF1 with no vertical blank waited for since the last one: on the console
    // the loop is paced by the DMA and the interrupts (the title loop never calls sceGsSyncV), so
    // the frame ends here, with the vertical blank's handlers, as if the hardware had done it.
    if (channel == 0x10009000u) {
        if (!g_vsync_since_kick) {
            openrac_game_vsync();
            openrac_game_run_vsync_handlers();
        }
        g_vsync_since_kick = false;
    }
    log::debug("frame {}: DMA chain at {:#010x} to channel {:#010x}", g_frame, tag, channel);
}

void openrac_game_set_display(const openrac_game_display* d) {
    static openrac_game_display last{};
    if (std::memcmp(&last, d, sizeof(last)) != 0) {
        last = *d;
        log::info("display: {}x{} at page {} (width {}), format {:#x}, at {},{}", d->width, d->height,
                  d->frame_base, d->frame_width, d->psm, d->x, d->y);
    }
}

void openrac_game_set_video_mode(int interlace, int mode, int field_mode) {
    log::info("video mode {:#x} (interlace {}, field mode {})", mode, interlace, field_mode);
}

void openrac_game_load_image(const openrac_game_image* image) {
    log::debug(
        "texture upload {}x{} format {:#x} to block {}",
        image->width,
        image->height,
        image->psm,
        image->base
    );
#ifdef OPENRAC_FRONTEND
    if (g_window && image->width > 0 && image->height > 0) {
        const std::size_t bytes = renderer::image_bytes(
            static_cast<std::uint8_t>(image->psm), image->width, image->height
        );
        frontend::upload_image(
            static_cast<std::uint32_t>(image->base),
            static_cast<std::uint32_t>(image->width_units),
            static_cast<std::uint8_t>(image->psm),
            static_cast<std::uint32_t>(image->x),
            static_cast<std::uint32_t>(image->y),
            static_cast<std::uint32_t>(image->width),
            static_cast<std::uint32_t>(image->height),
            std::span<const std::uint8_t>(static_cast<const std::uint8_t*>(G(image->pixels)), bytes)
        );
    }
#endif
}

void openrac_game_store_image(const openrac_game_image* image) {
    if (image->width <= 0 || image->height <= 0) {
        return;
    }
    const std::size_t bytes =
        static_cast<std::size_t>(image->width) * static_cast<std::size_t>(image->height) * 4;
    auto* out = static_cast<std::uint8_t*>(G(image->pixels));
#ifdef OPENRAC_FRONTEND
    if (g_window && frontend::store_image(static_cast<std::uint32_t>(image->base),
                                          static_cast<std::uint32_t>(image->width_units),
                                          static_cast<std::uint8_t>(image->psm), image->x, image->y,
                                          image->width, image->height, std::span<std::uint8_t>(out, bytes))) {
        return;
    }
#endif
    // Nothing to answer with: black, as an empty frame buffer reads.
    if (image->psm == 0) {
        std::memset(out, 0, bytes);
    }
}

/*
 * The movie player, after ReRAC's movie mode (crates/rc-game/src/movie_player.rs,
 * crates/rc-engine/src/movie_render.rs; ISC License, Copyright (c) 2026 ReRAC contributors):
 * the audio is the clock. Each vertical blank hands the device one blank's worth of the movie's
 * 48 kHz audio, and the picture shown is the one whose time holds the sample being heard (what has
 * been handed over less what the device still holds), so picture and sound cannot drift apart.
 * When the video ends (or is skipped) the last picture stays up for the console's FadeToBlack(4).
 */
int openrac_game_play_movie(uint32_t lsn, uint32_t bytes, int channel, int start_skips) {
    if (g_options.skip_movies) {
        log::info("movie at sector {}: skipped by --skip-movies (development probe)", lsn);
        return 1;  // the same result as the movie player's Start-skip path
    }
#if defined(OPENRAC_FRONTEND) && defined(OPENRAC_MOVIES)
    if (!g_window || openrac_game_disc_image == nullptr || bytes == 0) {
        return 0;
    }
    std::vector<std::uint8_t> file(bytes);
    {
        std::FILE* disc = std::fopen(openrac_game_disc_image, "rb");
        if (disc == nullptr) {
            return 0;
        }
#if defined(_WIN32)
        // Windows' C library has no fseeko; _fseeki64 is its 64-bit seek.
        const bool sought = _fseeki64(disc, static_cast<long long>(lsn) * 2048, SEEK_SET) == 0;
#else
        const bool sought = fseeko(disc, static_cast<off_t>(lsn) * 2048, SEEK_SET) == 0;
#endif
        const bool read = sought && std::fread(file.data(), 1, file.size(), disc) == file.size();
        std::fclose(disc);
        if (!read) {
            log::warn("movie at sector {}: the disc image is too short", lsn);
            return 0;
        }
    }
    std::optional<media::MoviePlayer> player;
    try {
        player.emplace(media::Movie::open(file, static_cast<std::uint8_t>(channel)));
    } catch (const std::exception& e) {
        log::warn("movie at sector {}: {}", lsn, e.what());
        return 0;
    }
    const media::VideoSequence& sequence = player->movie().sequence();
    log::info(
        "movie at sector {}: {}x{}, {}/{} fps, audio channel {}", lsn, sequence.width, sequence.height,
        sequence.fps_num, sequence.fps_den, player->movie().audio_channel() ? int(*player->movie().audio_channel()) : -1
    );
    std::string error;
    auto output = platform::AudioOutput::open(error);
    if (!output) {
        log::warn("movie: no sound ({})", error);
    }
    const int rate = openrac_game.frame_rate > 0 ? openrac_game.frame_rate : 60;
    const std::size_t per_blank = static_cast<std::size_t>(platform::kAudioRate / rate);
    std::vector<std::int16_t> samples(per_blank * 2);
    std::vector<std::uint8_t> rgba;
    std::shared_ptr<const media::VideoFrame> shown;
    std::uint16_t previous = 0xFFFF;
    int skipped = 0;
    auto pace = [&] {
        using namespace std::chrono;
        const auto now = steady_clock::now();
        if (g_next_frame.time_since_epoch().count() == 0 || now > g_next_frame + milliseconds(100)) {
            g_next_frame = now;
        }
        g_next_frame += microseconds(1000000 / rate);
        std::this_thread::sleep_until(g_next_frame);
        g_frame++;
    };
    for (std::uint64_t blank = 0;; ++blank) {
        if (output) {
            const std::size_t got = player->read_audio(samples);
            if (got > 0) {
                output->queue(std::span<const std::int16_t>(samples.data(), got * 2));
            }
        }
        const std::uint64_t handed = (blank + 1) * per_blank;
        const std::uint64_t held = output ? static_cast<std::uint64_t>(output->queued_frames()) : 0;
        const double heard = static_cast<double>(handed > held ? handed - held : 0) / platform::kAudioRate;
        auto frame = player->frame_at(heard);
        if (frame) {
            shown = frame;
        }
        if (player->video_finished()) {
            break;
        }
        if (shown) {
            media::frame_to_rgba(*shown, rgba);
            if (!frontend::show_picture(rgba.data(), int(shown->width), int(shown->height), 0.0f)) {
                std::exit(0);
            }
        }
        // Start skips when pressed during the movie: what is held as it starts is not a press.
        std::uint16_t buttons = 0xFFFF;
        std::uint8_t analog[4];
        frontend::pad(0, &buttons, analog);
        if (blank == 0) {
            previous = buttons;
        }
        const bool start_now = (buttons & 0x0008) == 0;
        const bool start_before = (previous & 0x0008) == 0;
        previous = buttons;
        if (start_skips && start_now && !start_before) {
            log::info("movie at sector {}: Start pressed (pad {:#06x})", lsn, buttons);
            skipped = 1;
            break;
        }
        pace();
    }
    // FadeToBlack(4) over the last picture, then its closing black blank.
    for (int step = 0; step <= 4 && shown; ++step) {
        frontend::show_picture(rgba.data(), int(shown->width), int(shown->height), step >= 4 ? 1.0f : float(step + 1) / 4.0f);
        pace();
    }
    if (output) {
        output->pause(true);
    }
    log::info("movie at sector {}: {}", lsn, skipped ? "skipped" : "played");
    return skipped;
#else
    (void)lsn;
    (void)bytes;
    (void)channel;
    (void)start_skips;
    return 0;
#endif
}

void openrac_game_mobys_drawn(gaddr first, int count) {
#ifdef OPENRAC_FRONTEND
    if (g_window && count > 0) {
        const auto* ram = runtime::Memory::get().base();
        frontend::mobys_drawn(std::span<const std::uint8_t>(ram, 32u * 1024 * 1024), first, count);
    }
#else
    (void)first;
    (void)count;
#endif
}

void openrac_game_effect_quad(const openrac_game_quad* quad) {
#ifdef OPENRAC_FRONTEND
    if (g_window) {
        const auto* ram = runtime::Memory::get().base();
        frontend::effect_quad(std::span<const std::uint8_t>(ram, 32u * 1024 * 1024), *quad);
    }
#else
    (void)quad;
#endif
}

int openrac_game_ui_quad(const openrac_game_ui_rect* quad) {
#ifdef OPENRAC_FRONTEND
    if (g_window) {
        return frontend::ui_quad(*quad);
    }
#else
    (void)quad;
#endif
    return -1;
}

void openrac_game_particle(const uint8_t* record, uint32_t pixels, uint32_t clut, int log2_side) {
#ifdef OPENRAC_FRONTEND
    if (g_window) {
        const auto* ram = runtime::Memory::get().base();
        frontend::particle(std::span<const std::uint8_t>(ram, 32u * 1024 * 1024), record, pixels, clut, log2_side);
    }
#else
    (void)record;
    (void)pixels;
    (void)clut;
    (void)log2_side;
#endif
}

void openrac_game_sky_sprite(const uint8_t* record, uint64_t tex0) {
#ifdef OPENRAC_FRONTEND
    if (g_window) {
        frontend::sky_sprite(record, tex0);
    }
#else
    (void)record;
    (void)tex0;
#endif
}

void openrac_game_moby_chain(gaddr moby, gaddr marks) {
#ifdef OPENRAC_FRONTEND
    const auto* ram = runtime::Memory::get().base();
    const auto pose = viewer::moby_pose_matrices(std::span<const std::uint8_t>(ram, 32u * 1024 * 1024), moby);
    const auto* mark = static_cast<const std::uint8_t*>(G(marks));
    const std::size_t count = std::min<std::size_t>(mark[0x7F], pose.size());
    for (std::size_t j = 0; j < count; ++j) {
        if (mark[j] != 0) {
            std::memcpy(G(0x70000000u + static_cast<gaddr>(j) * 0x40), pose[j].data(), 0x40);
        }
    }
#else
    (void)moby;
    (void)marks;
#endif
}

gaddr openrac_game_text(int id) {
#ifdef OPENRAC_FRONTEND
    if (g_window) {
        return frontend::text(id);
    }
#endif
    (void)id;
    return 0;
}

int openrac_game_pad(int port, uint16_t* buttons, uint8_t analog[4]) {
#ifdef OPENRAC_FRONTEND
    if (g_window && frontend::pad(port, buttons, analog)) {
        return 1;
    }
#endif
    (void)port;
    *buttons = 0xFFFF;  // nothing pressed (active low)
    std::memset(analog, 0x80, 4);
    return 0;
}

}  // extern "C"

// A level's program as the extractor wrote it (decompiler_out/<game>/levels/NN/overlay.bin): the
// pieces the game's ParseBin copies in, each a header (load address, size, a word, entry) and its
// bytes, while the entry stays the same. For the runtime's level-to-level relocation.
static void load_level_program(int level) {
    char name[8];
    std::snprintf(name, sizeof(name), "%02d", level);
    const fs::path path =
        g_options.data / "decompiler_out" / openrac_game.game / "levels" / name / "overlay.bin";
    std::vector<std::uint8_t> bytes;
    if (!runtime::read_file(path, &bytes)) {
        log::warn("level {}'s program is not in {}: its shared functions are not relocated",
                  level, path.string());
        return;
    }
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

#if defined(_WIN32)
// winmm's timer resolution, without windows.h (its macros clash with the port's names).
extern "C" __declspec(dllimport) unsigned int __stdcall timeBeginPeriod(unsigned int);
#pragma comment(lib, "winmm.lib")
#endif

int main(int argc, char** argv) {
#if defined(_WIN32)
    // Windows sleeps in steps of 15.6 ms unless a program asks for 1 ms: the wait for the next
    // 20 ms tick would overshoot by up to a step, and the frames stagger.
    timeBeginPeriod(1);
#endif
    g_options = parse(argc, argv);
    const fs::path iso = g_options.data / "iso_data" / openrac_game.game;
    const fs::path exe = iso / openrac_game.serial;
    const fs::path disc = iso / "disc.iso";
    if (!fs::exists(exe) || !fs::exists(disc)) {
        log::fatalf(
            "{} has no {} and disc.iso: set the game up from your disc first "
            "(python3 tools/extractor.py <image> --game {} --proj-path {})",
            iso.string(),
            openrac_game.serial,
            openrac_game.game,
            g_options.data.string()
        );
    }
    log::set_file((g_options.data / (program_name() + ".log")).string().c_str());
    log::info("{}: {}", program_name(), openrac_game.title);

    runtime::Memory::create();
    runtime::install_crash_handler();

    std::vector<std::uint8_t> file;
    runtime::ElfImage image;
    std::string reason;
    if (!runtime::read_file(exe, &file) || !runtime::read_elf(file, &image, &reason)
        || !runtime::load_elf(file, image, &reason)) {
        log::fatalf("{}: {}", exe.string(), reason.empty() ? "cannot be read" : reason);
    }
    log::info(
        "{}: {} segments loaded, entry {:#x}",
        openrac_game.serial,
        image.segments.size(),
        image.entry
    );

    static std::string disc_path = disc.string();
    std::error_code made;
    fs::create_directories(g_options.cards, made);
    static std::string cards_path = g_options.cards.string();
    openrac_game_disc_image = disc_path.c_str();
    openrac_game_card_dir = cards_path.c_str();
    static std::string mods_path = g_options.mods.string();
    if (!mods_path.empty()) {
        openrac_game_mods_dir = mods_path.c_str();
        log::info("edited levels from {}", mods_path);
    }

#ifdef OPENRAC_FRONTEND
    if (g_options.window) {
        std::string why;
        settings::load(g_options.settings);
        fs::path levels =g_options.levels.empty() ? g_options.data / "port" : g_options.levels;
        if (!frontend::open(openrac_game.id, levels, why)) {
            log::fatalf("no window: {}", why);
        }
        g_window = true;
        // The game's sounds (openrac/sound_engine.h), rendered on the output's thread.
        static std::unique_ptr<platform::AudioOutput> sound_output =
            platform::AudioOutput::open(why, [](std::span<std::int16_t> samples) {
                openrac_sound_render(samples.data(), samples.size() / 2);
            });
        if (!sound_output) {
            log::warn("no sound: {}", why);
        }
    }
#endif

    openrac_game_register_functions();
    openrac_guest_set_overlay_source(openrac_game_loaded_overlay);
    openrac_guest_set_level_programs(load_level_program);
    openrac_guest_stop_on_missing(g_options.keep_going ? 0 : 1);
    std::atexit(finish);

    // What the console's start-up code did before main: the stack and heap are
    // the runtime's, .bss was zeroed by the loader.
    int ran = 0;
#ifdef OPENRAC_FRONTEND
    if (g_window) {
        // Beside the window's event pump, so the game goes on while the system holds the pump (a
        // window being dragged or sized on Windows: platform/pump.h).
        ran = platform::run_pumped([](void*) { return openrac_game_run(); }, nullptr);
    } else {
        ran = openrac_game_run();
    }
#else
    ran = openrac_game_run();
#endif
    if (!ran) {
        log::error(
            "{}'s main is not known yet: port/game/{}/hostgen.json has no \"entry\"",
            openrac_game.title,
            openrac_game.id
        );
        return 2;
    }
    return 0;
}
