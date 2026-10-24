// A mod for OpenRAC's native port, in C++ (sdk/README.md, docs/port/MODS.md). openrac/mod.hpp is
// the C++ layer over the C interface; openrac/games/rac1.hpp names Ratchet & Clank's hook points.
//
// How it works: OPENRAC_MOD_CPP at the end of the file writes the two entry points the port looks
// for. The port calls the load function once, before the game runs; there the mod checks the game
// it runs on, reads its settings, and takes what it needs: hook points (a replacement for a game
// function) and callbacks (code that runs after each game tick). After load, the game's thread runs
// them; nothing here uses another thread.
#include "openrac/mod.hpp"

#include "openrac/games/rac1.hpp"

namespace {

// Loads the mod: refuses a game it is not for, reads a setting, takes one hook point and one
// per-tick callback. Throws openrac::Error to refuse the load; the message is shown to the player.
// Called once by the port at start, on the game's thread.
void load(openrac::Mod& mod) {
    // Refuse here rather than misbehave later: a mod written for one game reads its memory and
    // takes its hook points, which another game does not have.
    if (mod.game() != "rac1") {
        throw openrac::Error("this mod is for Ratchet & Clank");
    }

    const bool extra = mod.setting<bool>("enabled_extra");

    // A hook: the lambda gets the way on (the game's function, or a mod's loaded earlier) first,
    // then the hook point's arguments. An exception thrown here is logged, and the original runs.
    mod.hook<openrac::rac1::Hero_SetPlanarVelocity>([](auto original, float angle) {
        // ... before the game's own ...
        original(angle);
        // ... after it ...
    });

    // Once a game tick: read or change game memory through mod.memory().
    mod.on_frame([] {});

    mod.info(extra ? "loaded, with the extra" : "loaded");
}

}  // namespace

// The id and version are the manifest's: the port checks they agree.
OPENRAC_MOD_CPP("my-mod", "0.1.0", load)
