# Getting started

From nothing to a mod a player installs by pasting a link. Pick a track.

## Script mod (no compiler)

1. **Get the SDK.** Download `openrac-mod-sdk-<version>.zip` from
   [OpenRAC's releases](https://github.com/OpenRAC/OpenRAC/releases) (the ones
   named "Mod SDK") and unpack it. Script mods need SDK 1.1 or later.
2. **Copy the template.** `template-script/` is a whole repository: copy it
   to a new folder and make it a git repository (`git init`).
3. **Name your mod.** In `openrac-mod.json`, set `id` (lower case and
   hyphens, such as `big-jump`), `name`, `author`, `description` and
   `version` ([the manifest](manifest.md)).
4. **Write it.** `main.luau` runs once when the game starts; it takes its
   hooks and callbacks there:

   ```lua
   openrac.on_frame(function()
       if openrac.fields.bolts > 1000 then
           openrac.log("rich!")
       end
   end)
   ```

   [Script mods](scripts.md) has the rest. Point your editor's Luau language
   server at the SDK's `types/rac1.d.luau` for autocomplete (the template's
   `.vscode/settings.json` does it for VS Code with luau-lsp).
5. **Try it.** In the launcher's **Mods** tab, **Add a mod**, and choose your
   folder. Switch it on and press Play. Your `openrac.log` lines are in the
   game's log (**Logs** on the game's page).
6. **Release it.** Push the repository to GitHub (public), then a tag with
   the manifest's version:

   ```sh
   git tag v0.1.0
   git push origin v0.1.0
   ```

   The template's workflow packages the mod and publishes the release
   ([packaging and releases](packaging-and-releases.md)).
7. **Share the link.** Players paste `https://github.com/<you>/<repo>` in the
   Mods tab's **Add from a link** ([install from a link](install-from-link.md)).

## Native mod (C or C++)

1. **Get the SDK** as above, and the tools: CMake, Ninja, and a compiler
   (Xcode's command-line tools on macOS, `build-essential` on Linux, Visual
   Studio with "Desktop development with C++" on Windows).
2. **Copy the template.** `template/` is a whole repository for a C++ mod.
3. **Fetch the SDK it pins, and build:**

   ```sh
   python3 tools/fetch_sdk.py      # the SDK sdk.lock names, into .sdk/ (commit sdk.lock after the first time)
   cmake -S . -B build -G Ninja
   cmake --build build
   ```

4. **Write it** in `src/mod.cpp` ([in C++](native-cpp.md), or [in C](native-c.md)):

   ```cpp
   #include "openrac/mod.hpp"
   #include "openrac/games/rac1.hpp"

   static void load(openrac::Mod& mod) {
       mod.hook<openrac::rac1::Hero_SetPlanarVelocity>([](auto original, float angle) {
           original(angle);   // the game's own, or the mod loaded before yours
       });
   }
   OPENRAC_MOD_CPP("my-mod", "0.1.0", load)
   ```

5. **Try it.** **Add a mod** in the launcher with the folder
   `build/package/<target>`, switch it on, Play.
6. **Release it** with a tag, as for a script mod: the workflow builds it on
   Windows, Linux and macOS and publishes one archive per platform with their
   checksums. Players never need a compiler.

## Next

- [Hooks](hooks.md): what you can change, and how mods share a hook.
- [Settings](settings.md): give players options.
- [Troubleshooting](troubleshooting.md): when the mod does not load.
