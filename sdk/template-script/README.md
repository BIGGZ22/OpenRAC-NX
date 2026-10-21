# My script mod

A script mod for OpenRAC's port of Ratchet & Clank: Luau files the game runs in
a sandbox. No compiler is needed, and it runs on every system the game does.
This folder is a repository template: copy it into a repository of its own (or
a folder of a repository of several mods, as
[rac1-mods](https://github.com/OpenRAC/rac1-mods)), then set the id, name,
author and version in `openrac-mod.json`, and write `main.luau`.

**Writing it.** `openrac` is all a script sees of the game: the log, its
settings, the hook points by name (`openrac.hook`, `before`, `after`), a
callback each tick and on each pad report, and the game state the port exposes
([docs/port/MODS.md](https://github.com/OpenRAC/OpenRAC/blob/master/docs/port/MODS.md),
section 11). For an editor's autocomplete, `python3 tools/fetch_sdk.py` puts
the SDK in `.sdk/`, whose `types/rac1.d.luau` declares all of it; with
[luau-lsp](https://github.com/JohnnyMorganz/luau-lsp) in VS Code,
`.vscode/settings.json` already points at it (change the version there with
`sdk.lock`'s). The game stops a script that runs too long or raises an error,
and says so in its log and on the mod's row in the launcher.

**Trying it.** In the OpenRAC launcher, **Mods**, then **Add a mod**, and
choose this folder; switch it on and Play. The game's log (Logs, under the
game's saves) has your lines after the mod's id. After a change, add the
folder again (it replaces the installed copy, keeping its settings).

**Releasing.** Push the repository to GitHub. Players can install it from its
address straight away (**Add from a link** in the Mods tab: the repository's
own files are the mod) and get updates as you push. To give them versions, set
the version in `openrac-mod.json` and push a tag `v0.1.0`: the workflow in
`.github/workflows/build.yml` checks the mod, packages it as
`my-script-mod-0.1.0.zip` with `SHA256SUMS`, and publishes them on that
release, which the launcher then installs from and offers as updates.
`python3 .sdk/openrac-mod-sdk-*/tools/package_mod.py . -o dist` makes the same
archive by hand.
