// A mod for OpenRAC's native port, in C++ (sdk/README.md, docs/port/MODS.md). openrac/mod.hpp
// is the C++ layer over the C interface; openrac/games/rac1.hpp names Ratchet & Clank's hook points.
#include "openrac/mod.hpp"

#include "openrac/games/rac1.hpp"

namespace {

// Taken at load: refuse there (throw openrac::Error) rather than misbehave later.
void load(openrac::Mod& mod) {
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
