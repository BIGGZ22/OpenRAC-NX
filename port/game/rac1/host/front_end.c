/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (c) 2026 the OpenRAC contributors
 *
 * Ratchet & Clank: what the window's own menus and drawing need of the game (game_host.h,
 * openrac_front_end): the developer menu's level start, the front end's page menu that the
 * Planets entry and the port's Video rows go into (port/game/common/debug_menu.cpp,
 * options_menu.cpp), and the words the window reads. boot.c installs it. One table for every disc: the addresses are
 * games/rac1's, each version's through its rac1_version.h.
 */
#include "hero_attachments.h"
#include "rac1.h"

static const char* const level_names[19] = {
    "Veldin", "Novalis",     "Aridia",       "Kerwan", "Eudora", "Rilgar",      "Blarg Station",
    "Umbris", "Batalia",     "Gaspar",       "Orxon",  "Pokitaru", "Hoven",     "Gemlik Base",
    "Oltanis", "Quartu",     "Kalebo III",   "Drek's Fleet", "Veldin (final)",
};

/* The planet names in the game's text, by level (the final Veldin is Veldin again). */
static const int16_t planet_texts[19] = {20190, 20173, 20174, 20175, 20176, 20177, 20159, 20179, 20180, 20181,
                                         20182, 20183, 20184, 20166, 20186, 20187, 20188, 20170, 20190};

static void menu_sound(int sound, gaddr moby) {
    func_0022ED80(0, sound, moby);
}

/* HeroAttachState is resident and uses the same slot layout in PAL and NTSC (level_hero_attach.c). */
static unsigned hero_shadow_casters(gaddr* addresses, unsigned capacity) {
    return openrac_rac1_hero_attachments(
        openrac_guest_base, GUEST_RAM_SIZE, RAC1_ADDR(0x0013F450u), addresses, capacity
    );
}

const openrac_front_end rac1_front_end = {
    .levels = 19,
    .level_names = level_names,
    .start_level = rac1_start_level,
    .menu_sound = menu_sound,
    .page_menu = RAC1_ADDR(0x001D5F70u),
    .options_list = RAC1_ADDR(0x001D4BE8u),
    .options_items = RAC1_ADDR(0x001D4B90u),
    .video_item = RAC1_ADDR(0x001D4B9Cu), /* PAL's list's second item */
    .language_page = RAC1_ADDR(0x001D4CC0u),
    .language_title = RAC1_ADDR(0x001D4D48u),
    .language_list = RAC1_ADDR(0x001D4DE8u),
    .planets_text = 20216,
    .planet_texts = planet_texts,
    .video_page = RAC1_ADDR(0x001D4E38u),
    .video_list = RAC1_ADDR(0x001D4F40u),
    .video_rows = RAC1_ADDR(0x001D4F18u),
    .sound_hint = RAC1_ADDR(0x001CE720u),
    .sound_list_seq = 0xD2,
    .sound_hint_seq = 0xD7,
    .pad = RAC1_ADDR(0x0013CA40u),
    .video_mode = RAC1_ADDR(0x0016044Cu),
    .game_mode = RAC1_ADDR(0x0015F6E8u),
    .light_bank = RAC1_ADDR(0x0019BEC0u),
    .level = RAC1_ADDR(0x0015EE84u),
    .dialog = RAC1_ADDR(0x00193400u),
    .title_exit = RAC1_ADDR(0x0015F690u),
    .ship_state = RAC1_ADDR(0x0013E150u),
    .hero_shadow_casters = hero_shadow_casters,
    .language = RAC1_ADDR(0x0015EE88u),
};
