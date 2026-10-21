/* A mod for OpenRAC's native port (sdk/README.md, docs/port/MODS.md). */
#include <string.h>

#include "openrac/openrac_mod.h"
#include "openrac/games/rac1.h"

/* The id and version are the manifest's: the port checks they agree. */
OPENRAC_MOD_DEFINE("my-mod", "0.1.0")

static const openrac_api* s_api;
static openrac_mod* s_mod;

static void on_frame(void* user) {
    (void)user;
    /* Once a game tick: read or change game memory through s_api->memory. */
}

OPENRAC_MOD_EXPORT int openrac_mod_load(const openrac_api* api, openrac_mod* mod) {
    s_api = api;
    s_mod = mod;
    if (strcmp(api->game, "rac1") != 0) {
        api->fail(mod, "this mod is for Ratchet & Clank");
        return 1;
    }
    api->on_frame(mod, on_frame, NULL);
    api->log(mod, OPENRAC_LOG_INFO, api->setting_bool(mod, "enabled_extra") ? "loaded, with the extra" : "loaded");
    return 0;
}
