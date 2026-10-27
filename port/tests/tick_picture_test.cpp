// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Tests frontend::TickPictures (game/common/frontend/tick_picture.h): how each tick's frames start.
// The sequences are the ticks Pokitaru's vendor opening and Kerwan's take-off play, as the frame
// log (OPENRAC_FRAME_LOG) showed them.

#include <cstddef>
#include <cstdint>

#include "game/common/frontend/tick_picture.h"
#include "tests/check.h"

namespace {

using openrac::frontend::kMenuSnapshotBytes;
using openrac::frontend::kMostMenuGapTicks;
using openrac::frontend::TickFacts;
using openrac::frontend::TickPicture;
using openrac::frontend::TickPictures;

// Every world layer, and the moby layer alone (the galactic map).
constexpr std::uint32_t kAllLayers = 0x1Fu;
constexpr std::uint32_t kMobysOnly = 0x10u;
// A play tick's 2D list: the HUD after the world.
constexpr std::size_t kPlayBytes = 162944;
constexpr std::size_t kPlayBeforeWorld = 2176;
// The fade's clears and darkening sprites, all before the world.
constexpr std::size_t kFadeBytes = 1440;

// A level's tick with a camera, a picture already drawn, and the given layers and list.
TickFacts level_tick(std::uint32_t draws, std::size_t packets, std::size_t before_world) {
    TickFacts tick;
    tick.draws = draws;
    tick.has_camera = true;
    tick.in_level = true;
    tick.packets = packets;
    tick.before_world = before_world;
    tick.has_picture = true;
    return tick;
}

// The vendor opening: four ticks without a world darken the picture shown, then the fly-in's first
// world tick has no list. None of them starts from a cleared frame (the cyan flash).
void test_vendor_opening_keeps_the_picture() {
    TickPictures pictures;
    CHECK(
        pictures.choose(level_tick(kAllLayers, kPlayBytes, kPlayBeforeWorld)) == TickPicture::Draw
    );

    for (int tick = 0; tick < 4; ++tick) {
        CHECK(pictures.choose(level_tick(0, kFadeBytes, kFadeBytes)) == TickPicture::Overdraw);
    }

    CHECK(pictures.choose(level_tick(kAllLayers, 0, 0)) == TickPicture::Repeat);
    CHECK(
        pictures.choose(level_tick(kAllLayers, kPlayBytes, kPlayBeforeWorld)) == TickPicture::Draw
    );
}

// The take-off: the fade's black ticks without a world, then a world whose only 2D is the fade's
// clears before it. That tick stands for the last; the next, with its list, is drawn.
void test_take_off_world_without_its_list_stands() {
    TickPictures pictures;
    CHECK(pictures.choose(level_tick(0, kFadeBytes, kFadeBytes)) == TickPicture::Overdraw);
    CHECK(pictures.choose(level_tick(0, kFadeBytes, kFadeBytes)) == TickPicture::Overdraw);
    CHECK(pictures.choose(level_tick(kAllLayers, kFadeBytes, kFadeBytes)) == TickPicture::Repeat);
    CHECK(pictures.choose(level_tick(kAllLayers, 38384, 1344)) == TickPicture::Draw);

    // In play, a world tick with its whole list before the world (the flight's) is drawn.
    CHECK(pictures.choose(level_tick(0x11u, 9856, 9856)) == TickPicture::Draw);
    CHECK(pictures.choose(level_tick(kMobysOnly, 44176, kPlayBeforeWorld)) == TickPicture::Draw);
}

// After the pause menu closes: no world under the page menu's list stands for at most
// kMostMenuGapTicks ticks, and a page menu's list is never drawn over the last picture.
void test_menu_gap_stands_a_few_ticks() {
    TickPictures pictures;
    CHECK(
        pictures.choose(level_tick(kAllLayers, kPlayBytes, kPlayBeforeWorld)) == TickPicture::Draw
    );

    for (int tick = 0; tick < kMostMenuGapTicks; ++tick) {
        CHECK(
            pictures.choose(level_tick(0, kMenuSnapshotBytes + 1, kMenuSnapshotBytes + 1))
            == TickPicture::Repeat
        );
    }

    CHECK(
        pictures.choose(level_tick(0, kMenuSnapshotBytes + 1, kMenuSnapshotBytes + 1))
        == TickPicture::Draw
    );
}

// The boot program, a tick without a camera, and the very first picture start from a cleared frame.
void test_outside_a_level_ticks_are_drawn() {
    TickPictures pictures;
    TickFacts boot = level_tick(0, kFadeBytes, kFadeBytes);
    boot.in_level = false;
    CHECK(pictures.choose(boot) == TickPicture::Draw);

    TickFacts loading = level_tick(0, kFadeBytes, kFadeBytes);
    loading.has_camera = false;
    CHECK(pictures.choose(loading) == TickPicture::Draw);

    TickFacts first = level_tick(kAllLayers, 0, 0);
    first.has_picture = false;
    CHECK(pictures.choose(first) == TickPicture::Draw);
}

// A save loaded in play: the loading picture's ticks run in the boot program, and the next level's
// ticks keep the last level's camera in the game's memory. Its ticks without a world (a scene's
// first) start from a cleared frame, not over the loading picture, until it draws its world.
void test_another_level_forgets_the_picture() {
    TickPictures pictures;
    TickFacts play = level_tick(kAllLayers, kPlayBytes, kPlayBeforeWorld);
    play.level = 4;
    CHECK(pictures.choose(play) == TickPicture::Draw);

    TickFacts loading = level_tick(0, kFadeBytes, kFadeBytes);
    loading.in_level = false;
    loading.level = -1;
    CHECK(pictures.choose(loading) == TickPicture::Draw);

    TickFacts scene = level_tick(0, kFadeBytes, kFadeBytes);
    scene.level = 3;
    CHECK(pictures.choose(scene) == TickPicture::Draw);
    CHECK(pictures.choose(scene) == TickPicture::Draw);

    TickFacts world = level_tick(kAllLayers, kPlayBytes, kPlayBeforeWorld);
    world.level = 3;
    CHECK(pictures.choose(world) == TickPicture::Draw);
    CHECK(pictures.choose(scene) == TickPicture::Overdraw);
}

// A save loaded from the in-game Load Game page: its story card's ticks (no world, a short list)
// come while the old level and its camera are still in memory. The picture shown is the page
// menu's, not a world, so they start from a cleared frame (as from the title), not over the menu.
void test_story_card_after_a_page_menu_is_drawn() {
    TickPictures pictures;
    TickFacts play = level_tick(kAllLayers, kPlayBytes, kPlayBeforeWorld);
    play.level = 4;
    CHECK(pictures.choose(play) == TickPicture::Draw);

    TickFacts menu = level_tick(0, kMenuSnapshotBytes + 1, kMenuSnapshotBytes + 1);
    menu.level = 4;
    for (int tick = 0; tick < kMostMenuGapTicks; ++tick) {
        CHECK(pictures.choose(menu) == TickPicture::Repeat);
    }
    CHECK(pictures.choose(menu) == TickPicture::Draw);

    TickFacts card = level_tick(0, kFadeBytes, kFadeBytes);
    card.level = 4;
    CHECK(pictures.choose(card) == TickPicture::Draw);
    CHECK(pictures.choose(card) == TickPicture::Draw);

    // Once the level's world is drawn again, its ticks without one go over it as before.
    CHECK(pictures.choose(play) == TickPicture::Draw);
    CHECK(pictures.choose(card) == TickPicture::Overdraw);
}

}  // namespace

int main() {
    test_vendor_opening_keeps_the_picture();
    test_take_off_world_without_its_list_stands();
    test_menu_gap_stands_a_few_ticks();
    test_outside_a_level_ticks_are_drawn();
    test_another_level_forgets_the_picture();
    test_story_card_after_a_page_menu_is_drawn();
    return openrac::test::result();
}
