/* SPDX-License-Identifier: MIT
 * Copyright (c) 2026 the OpenRAC contributors
 *
 * An example mod that uses each part of the interface (sdk/README.md): it refuses a game it is not
 * for, reads a bool and an int setting, counts the game's ticks with on_frame, and takes a hook
 * point, counting the calls and going on to what it replaced. Every `every` ticks it logs both
 * counts. It changes nothing in the game.
 *
 * How it works: OPENRAC_MOD_DEFINE writes openrac_mod_query; openrac_mod_load keeps the table and
 * the handle in file-level variables (the port calls the mod's functions on the game's thread, one
 * at a time, so no lock is needed), takes the hook point and the callback, and logs a line;
 * openrac_mod_unload logs the totals at exit. Written in C, so the whole mod is plain C99 against
 * openrac_mod.h and games/rac1.h.
 */
#include <stdio.h>
#include <string.h>

#include "openrac/games/rac1.h"
#include "openrac/openrac_mod.h"

OPENRAC_MOD_DEFINE("frame-counter", "1.0.0")

/* The port's table and this mod's handle, kept at load. Read on the game's thread only. */
static const openrac_api* s_api;
static openrac_mod* s_mod;

/* The settings, read at load: log a line every `s_every` ticks, at info level when `s_verbose`
 * (else debug). */
static int64_t s_every;
static int s_verbose;

/* The counts: the game's ticks so far (on_frame) and the pad reports seen (the hook). Written on
 * the game's thread only. */
static uint64_t s_ticks;
static uint64_t s_reports;

/* What Pad_ProcessReport goes on to: the game's own, or a mod's loaded before this one. Set by the
 * port at load. */
static openrac_rac1_Pad_ProcessReport s_pad_original;

/* The ticks between two lines when the manifest's `every` is not a positive number: about ten
 * seconds of play (the game ticks 50 times a second on PAL discs and 60 on NTSC-U). */
static const int64_t kDefaultEvery = 600;

/* The hook: the same signature as the hook point (games/rac1.h). Counts the report, then goes on
 * to the original so the game sees it as before. */
static void pad_process_report(openrac_gaddr pad, openrac_gaddr report, int32_t size) {
    ++s_reports;
    s_pad_original(pad, report, size);
}

/* The callback of on_frame: counts the tick and, every `s_every` ticks, logs the counts and the
 * level that is loaded. `user` is unused. */
static void on_frame(void* user) {
    char line[128];

    (void)user;

    /* Most ticks only count. */
    if (++s_ticks % (uint64_t)s_every != 0) {
        return;
    }

    snprintf(
        line,
        sizeof line,
        "%llu ticks, %llu pad reports, level %d",
        (unsigned long long)s_ticks,
        (unsigned long long)s_reports,
        s_api->loaded_level()
    );
    s_api->log(s_mod, s_verbose ? OPENRAC_LOG_INFO : OPENRAC_LOG_DEBUG, line);
}

/* The entry point the port calls once at start: refuses a game it is not for, reads the settings,
 * takes the hook point and the callback. Returns 0 to load, 1 to refuse (after saying why). */
OPENRAC_MOD_EXPORT int openrac_mod_load(const openrac_api* api, openrac_mod* mod) {
    char line[96];

    s_api = api;
    s_mod = mod;

    /* The hook point and the memory it reads are Ratchet & Clank's: refuse any other game. */
    if (strcmp(api->game, "rac1") != 0) {
        api->fail(mod, "frame-counter is for Ratchet & Clank (rac1)");
        return 1;
    }

    s_every = api->setting_int(mod, "every");

    /* A zero or negative setting would divide by zero in on_frame: fall back to the default. */
    if (s_every <= 0) {
        s_every = kDefaultEvery;
    }

    s_verbose = api->setting_bool(mod, "verbose");

    /* A port without this hook point (an older one, or another disc) says so: refuse. */
    if (OPENRAC_RAC1_HOOK(api, mod, Pad_ProcessReport, pad_process_report, &s_pad_original)
        != OPENRAC_HOOK_OK) {
        api->fail(mod, "this OpenRAC has no Pad_ProcessReport hook point");
        return 1;
    }

    api->on_frame(mod, on_frame, NULL);

    snprintf(
        line,
        sizeof line,
        "counting on %s/%s, a line every %lld ticks",
        api->game,
        api->version,
        (long long)s_every
    );
    api->log(mod, OPENRAC_LOG_INFO, line);

    return 0;
}

/* The entry point the port calls at exit: logs the total of ticks. Optional in the interface. */
OPENRAC_MOD_EXPORT void openrac_mod_unload(void) {
    char line[96];

    /* A load that was refused before it kept the table has nothing to log through. */
    if (s_api == NULL) {
        return;
    }

    snprintf(line, sizeof line, "%llu ticks in all", (unsigned long long)s_ticks);
    s_api->log(s_mod, OPENRAC_LOG_INFO, line);
}
