/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (c) 2026 the OpenRAC contributors
 *
 * The port's rows in the game's options (port/game/common/options_menu.cpp): their texts; and the
 * game's own options a console forgets at power off, which the port remembers between runs. */
#include "rac1.h"

/* The text of an id (func_001FE540, matched C in the decompilation: the language's table, or the
 * empty string for an id it does not have). The port's own ids, which the table never has, are
 * answered with the port's strings (libraries.tsv, port = wrap). */
RAC1_TYPE_func_001FE540_result func_001FE540(int id) {
    const gaddr own = openrac_game_text(id);
    return own != 0 ? (RAC1_TYPE_func_001FE540_result)own : func_001FE540__game(id);
}

/* The game's options a console forgets at power off, which the port remembers between runs in its
 * settings (game_host.h, openrac_game_option): the language (D_0015EE88, which InitOnce sets from
 * the console's language, sceScfGetLanguage) and the Sound page (D_0015EEE8: the playback mode,
 * the music volume and the sound volume, 0 to 0x400). A loaded save carries all four too; the
 * player's stay (memcard_RestoreGame below), as a new game already keeps the Sound page
 * (NewGameInit). */
#define LANGUAGE RAC1_ADDR(0x0015EE88u)
#define SOUND RAC1_ADDR(0x0015EEE8u)
/* The Sound page's three words from SOUND on (above), the first of them the playback mode, a flag
 * the page toggles (any value is one of its two), and the loudest volume (seen in game code:
 * initialize_gameplay_sound_system, the Sound page's update). */
#define SOUND_WORDS 3
#define SOUND_WORD(i) (SOUND + 4u * (i))
#define SOUND_PLAYBACK 0u
#define LOUDEST 0x400

static const char* const sound_keys[SOUND_WORDS] =
    {"game_playback", "game_music_volume", "game_sound_volume"};

/* Whether the Sound page has the remembered values: from the sound's start on. */
static int sound_put;

/* The languages the game has, by its own index in LANGUAGE. */
#define GAME_LANGUAGES 6

/* The console's language for the game's (InitOnce's table read back: 1 English, 2 French,
 * 4 German, 3 Spanish, 5 Italian); 0 for one the console has none for. */
static int console_language(int game) {
    static const int codes[GAME_LANGUAGES] = {1, 0, 2, 4, 3, 5};
    return game >= 0 && game < GAME_LANGUAGES ? codes[game] : 0;
}

/* initialize_gameplay_sound_system: at the game's start it gives the mixer the Sound page's
 * values, so the remembered ones go in first. */
void func_0022DBE8(void) {
    /* Once, at the game's start: from then on the page holds what the player sets. */
    if (!sound_put) {
        sound_put = 1;
        for (unsigned i = 0; i < SOUND_WORDS; ++i) {
            const int value = openrac_game_option(sound_keys[i], -1);

            /* None remembered (-1), or a volume past the loudest in a hand-edited file: the
             * game's own value stays. */
            if (value >= 0 && (i == SOUND_PLAYBACK || value <= LOUDEST)) {
                GREF(int, SOUND_WORD(i)) = value;
            }
        }
    }
    func_0022DBE8__game();
}

/* memcard_RestoreGame: a save's state, the options with it; the language and the Sound page stay
 * as they were (the caller gives the mixer the Sound page's values right after). */
void func_00209CE8(RAC1_TYPE_func_00209CE8_save save) {
    const int language = GREF(int, LANGUAGE);
    int sound[SOUND_WORDS];
    for (unsigned i = 0; i < SOUND_WORDS; ++i) {
        sound[i] = GREF(int, SOUND_WORD(i));
    }
    func_00209CE8__game(save);
    GREF(int, LANGUAGE) = language;
    for (unsigned i = 0; i < SOUND_WORDS; ++i) {
        GREF(int, SOUND_WORD(i)) = sound[i];
    }
}

/* Each frame: the options as the player left them, for the next run. The language once InitOnce
 * has set it (it reads what the port remembered), the Sound page once it has the remembered
 * values. */
void rac1_options_frame(void) {
    static int language_set;
    const int language = console_language(GREF(int, LANGUAGE));
    /* Before InitOnce the word holds no choice yet: it is the start's once the two agree. */
    if (!language_set) {
        language_set = language == openrac_game_language;
    }
    /* A language the console has no code for is not remembered. */
    if (language_set && language > 0) {
        openrac_game_set_option("game_language", language);
    }
    /* Before the sound's start the page holds the game's defaults, not the player's. */
    if (sound_put) {
        for (unsigned i = 0; i < SOUND_WORDS; ++i) {
            openrac_game_set_option(sound_keys[i], GREF(int, SOUND_WORD(i)));
        }
    }
}
