// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

#include "options_menu.h"

#include <cstdio>
#include <cstring>
#include <string>

#include "common/log.h"
#include "openrac/game_host.h"
#include "openrac/guest.h"
#include "settings.h"

namespace openrac::frontend::options_menu {

namespace {

/*
 * Ratchet & Clank (PAL): the front end's Video options page.
 *
 * A page is data in the boot program (debug_menu.cpp describes it: seq[14], the frame objects'
 * animations; parent; kind; focus; widget[14]). The Video page's list is the game's row list
 * (update func_00221E60, draw func_00222070): rows of 0x14 bytes {label text, a byte's address,
 * the text shown while the byte is set, the text shown while it is clear, flags}, ended by a row
 * without a label. Cross on a row flips its byte (a row with flag 1 is the game's own video mode,
 * which it handles itself).
 *
 * The port's rows are more rows of that list, with texts of the port's own (ids the game's table
 * does not have: the text lookup is wrapped, host/options.c) and a byte each, which update() reads
 * as "Cross was pressed here": the setting moves to its next value and the byte is cleared. Both
 * of a row's value texts are the same id, whose string is the setting's value now.
 *
 * The page's frame for one row is too low for four, so the page borrows the Sound options page's
 * frame animations for its list and its hint (seq 0xD2 and 0xD7: three sliders' worth) and that
 * page's hint widget ("Toggle", "Exit").
 */
namespace pal {

constexpr gaddr kVideoPage = 0x001D4E38u;
constexpr gaddr kVideoList = 0x001D4F40u;
constexpr gaddr kVideoRows = 0x001D4F18u;   // the game's one row and its end
constexpr gaddr kSoundHint = 0x001CE720u;   // the Sound page's hint widget
constexpr std::int32_t kSoundListSeq = 0xD2;
constexpr std::int32_t kSoundHintSeq = 0xD7;
constexpr gaddr kRowBytes = 0x14;

// The port's text ids: past the game's last (its table's ids are under 0x7000).
constexpr int kTextFirst = 0x7E00;
enum Row { kAspect, kFrameRate, kFullscreen, kRows };
constexpr const char* kLabels[kRows] = {"Picture", "Frame rate", "Fullscreen"};
constexpr gaddr kTextBytes = 48;

struct Native {
    gaddr rows = 0;    // the game's row, the port's, the end
    gaddr flags = 0;   // a byte per row of the port's
    gaddr labels = 0;  // kRows strings of kTextBytes
    gaddr values = 0;  // kRows strings of kTextBytes
};
Native n;

void put_text(gaddr at, const std::string& text) {
    char* p = static_cast<char*>(G(at));
    std::snprintf(p, kTextBytes, "%s", text.c_str());
}

std::string value_of(int row) {
    const settings::Settings& s = settings::get();
    switch (row) {
        case kAspect:
            return settings::name(s.aspect);
        case kFrameRate:
            return settings::name(s.frame_rate);
        case kFullscreen:
            return s.fullscreen ? "On" : "Off";
        default:
            return {};
    }
}

void next_value(int row) {
    settings::Settings& s = settings::get();
    switch (row) {
        case kAspect:
            s.aspect = static_cast<settings::Aspect>((static_cast<int>(s.aspect) + 1) % static_cast<int>(settings::Aspect::Count));
            break;
        case kFrameRate:
            s.frame_rate = static_cast<settings::FrameRate>((static_cast<int>(s.frame_rate) + 1)
                                                            % static_cast<int>(settings::FrameRate::Count));
            break;
        case kFullscreen:
            s.fullscreen = !s.fullscreen;
            break;
        default:
            break;
    }
}

void build() {
    if (n.rows != 0) {
        return;
    }
    n.rows = openrac_guest_static(kRowBytes * (kRows + 2));
    n.flags = openrac_guest_static(8);
    n.labels = openrac_guest_static(kTextBytes * kRows);
    n.values = openrac_guest_static(kTextBytes * kRows);
    std::memset(G(n.rows), 0, kRowBytes * (kRows + 2));
    std::memset(G(n.flags), 0, 8);
    for (int i = 0; i < kRows; ++i) {
        const gaddr row = n.rows + kRowBytes * static_cast<gaddr>(i + 1);
        GREF(std::int32_t, row + 0x00) = kTextFirst + i;
        GREF(std::uint32_t, row + 0x04) = n.flags + static_cast<gaddr>(i);
        GREF(std::int32_t, row + 0x08) = kTextFirst + kRows + i;
        GREF(std::int32_t, row + 0x0C) = kTextFirst + kRows + i;
        put_text(n.labels + kTextBytes * static_cast<gaddr>(i), kLabels[i]);
    }
}

bool update() {
    build();
    // The game's own row first, as the boot program has it now (it is loaded again after a level).
    std::memcpy(G(n.rows), G(kVideoRows), kRowBytes);
    if (GREF(std::uint32_t, kVideoList + 0x34) == kVideoRows) {
        GREF(std::uint32_t, kVideoList + 0x34) = n.rows;
        GREF(std::int32_t, kVideoPage + 4 * 3) = kSoundListSeq;
        GREF(std::int32_t, kVideoPage + 4 * 4) = kSoundHintSeq;
        GREF(std::uint32_t, kVideoPage + 0x44 + 4 * 4) = kSoundHint;
    }
    bool changed = false;
    for (int i = 0; i < kRows; ++i) {
        std::uint8_t& pressed = GREF(std::uint8_t, n.flags + static_cast<gaddr>(i));
        if (pressed != 0) {
            pressed = 0;
            next_value(i);
            changed = true;
        }
        put_text(n.values + kTextBytes * static_cast<gaddr>(i), value_of(i));
    }
    if (changed) {
        settings::save();
    }
    return changed;
}

std::uint32_t text(int id) {
    if (n.rows == 0 || id < kTextFirst || id >= kTextFirst + 2 * kRows) {
        return 0;
    }
    const int i = id - kTextFirst;
    return i < kRows ? n.labels + kTextBytes * static_cast<gaddr>(i)
                     : n.values + kTextBytes * static_cast<gaddr>(i - kRows);
}

}  // namespace pal

bool is_pal(const char* game_id) {
    return game_id != nullptr && std::strcmp(game_id, "rac1-pal") == 0;
}

bool g_pal = false;

}  // namespace

bool update(const char* game_id, int loaded_overlay) {
    g_pal = is_pal(game_id);
    if (!g_pal || loaded_overlay != OPENRAC_OVERLAY_EXE) {
        return false;
    }
    return pal::update();
}

std::uint32_t text(int id) {
    return g_pal ? pal::text(id) : 0;
}

}  // namespace openrac::frontend::options_menu
