# Making mods for OpenRAC

OpenRAC plays Ratchet & Clank natively on a PC. A **mod** changes how the game
behaves: a new way to strafe, swapped buttons, a tweak to an enemy. Players
install mods from the OpenRAC launcher's **Mods** tab, by pasting the address
of your repository, and switch them on and off there.

There are two kinds of behaviour mod:

| | Script mod | Native mod |
|---|---|---|
| Written in | [Luau](https://luau.org) (a typed Lua) | C or C++ |
| You need | a text editor | a C/C++ compiler and CMake |
| Runs on | every platform, Android included | the platforms you build it for (the template's workflow builds Windows, Linux and macOS) |
| Safety | sandboxed: it sees only what OpenRAC gives it | runs as code on the player's computer |
| Since | interface 1.1 (SDK 1.1) | interface 1.0 |

Start with a script mod unless you need the speed or the reach of native code.
Mods of textures, models and levels are made with the level editor instead
([editor/MODDING.md](https://github.com/OpenRAC/OpenRAC/blob/master/editor/MODDING.md)).

## Guides

1. [Getting started](getting-started.md): from the template to a mod players can install.
2. [The manifest](manifest.md): `openrac-mod.json`, field by field.
3. [Settings](settings.md): options players change in the launcher.
4. [Hooks](hooks.md): how a mod changes what the game does, and the hook ids that never change.
5. Writing the code:
   - [Script mods (Luau)](scripts.md)
   - [Native mods in C](native-c.md)
   - [Native mods in C++](native-cpp.md)
6. [Packaging and releases](packaging-and-releases.md): the archive, the workflow, tags.
7. [Install from a link](install-from-link.md): what the launcher does with your repository.
8. [Where mods live](where-mods-live.md): the folders and files on a player's computer.
9. [Troubleshooting](troubleshooting.md): a mod that does not load, purist mode, crash reports.
10. [Views of the game](views.md): extra views for a VR headset or another display, with OpenGL or Vulkan (native mods, interface 1.4).

## Reference

Generated from the SDK's headers, tables and type files by OpenRAC's
[sdk/tools/gen_reference.py](https://github.com/OpenRAC/OpenRAC/blob/master/sdk/tools/gen_reference.py);
never edited by hand:

- [C interface](reference/c-api.md) (`include/openrac/openrac_mod.h`)
- [C++ layer](reference/cpp-api.md) (`include/openrac/mod.hpp`)
- [Luau API](reference/luau-api.md) (`types/rac1.d.luau`)
- [Ratchet & Clank's hook points and fields](reference/hooks-rac1.md)

## What is stable

Stable, and kept working by every OpenRAC of the same interface major version:
the C interface, the manifest, packaging, install from a link, the repository
templates, hook **ids**, and the `openrac` script API.

**Experimental**: anything that names the game's internals: the behaviour
catalogue (moby classes, their update functions and pvars), the fields exposed
to scripts, and the game functions and addresses a native mod looks up itself.
They follow the decompilation and may change. A hook **id** stays, even when
the function behind it is renamed ([hooks](hooks.md#stable-ids)).

How OpenRAC is built inside will get guides of its own after the port's move to
C++. The design notes are in
[docs/port/MODS.md](https://github.com/OpenRAC/OpenRAC/blob/master/docs/port/MODS.md)
(`docs/MODS.md` in the SDK archive, beside these pages).
