/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (c) 2026 the OpenRAC contributors
 *
 * An example mod that uses each part of the interface (sdk/README.md): it refuses a game it is not
 * for, reads a bool and an int setting, counts the game's ticks with on_frame, and takes a hook
 * point, counting the calls and going on to what it replaced. Every `every` ticks it logs both
 * counts. It changes nothing in the game.
 */
#include <stdio.h>
#include <string.h>

#include "openrac/openrac_mod.h"
#include "openrac/games/rac1.h"

OPENRAC_MOD_DEFINE("frame-counter", "1.0.0")

static const openrac_api* s_api;
static openrac_mod* s_mod;
static int64_t s_every;
static int s_verbose;
static uint64_t s_ticks;
static uint64_t s_reports;
/* What Pad_ProcessReport goes on to: the game's own, or a mod's loaded before this one. */
static openrac_rac1_Pad_ProcessReport s_pad_original;

/* The hook: the same signature as the hook point (games/rac1.h). */
static void pad_process_report(openrac_gaddr pad, openrac_gaddr report, int32_t size) {
    ++s_reports;
    s_pad_original(pad, report, size);
}

static void on_frame(void* user) {
    (void)user;
    if (++s_ticks % (uint64_t)s_every != 0) {
        return;
    }
    char line[128];
    snprintf(line, sizeof line, "%llu ticks, %llu pad reports, level %d", (unsigned long long)s_ticks,
             (unsigned long long)s_reports, s_api->loaded_level());
    s_api->log(s_mod, s_verbose ? OPENRAC_LOG_INFO : OPENRAC_LOG_DEBUG, line);
}

OPENRAC_MOD_EXPORT int openrac_mod_load(const openrac_api* api, openrac_mod* mod) {
    s_api = api;
    s_mod = mod;
    if (strcmp(api->game, "rac1") != 0) {
        api->fail(mod, "frame-counter is for Ratchet & Clank (rac1)");
        return 1;
    }
    s_every = api->setting_int(mod, "every");
    if (s_every <= 0) {
        s_every = 600;
    }
    s_verbose = api->setting_bool(mod, "verbose");
    if (OPENRAC_RAC1_HOOK(api, mod, Pad_ProcessReport, pad_process_report, &s_pad_original) != OPENRAC_HOOK_OK) {
        api->fail(mod, "this OpenRAC has no Pad_ProcessReport hook point");
        return 1;
    }
    api->on_frame(mod, on_frame, NULL);
    char line[96];
    snprintf(line, sizeof line, "counting on %s/%s, a line every %lld ticks", api->game, api->version,
             (long long)s_every);
    api->log(mod, OPENRAC_LOG_INFO, line);
    return 0;
}

OPENRAC_MOD_EXPORT void openrac_mod_unload(void) {
    if (s_api != NULL) {
        char line[96];
        snprintf(line, sizeof line, "%llu ticks in all", (unsigned long long)s_ticks);
        s_api->log(s_mod, OPENRAC_LOG_INFO, line);
    }
}
