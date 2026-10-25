# Developing OpenRAC

How to work on OpenRAC from a checkout: the launcher, the decompilations, the
PC port and the level editor. If you only want to play, use the launcher from
the [releases page](https://github.com/OpenRAC/OpenRAC/releases/latest)
instead ([README.md](../README.md#play)).

## Run the launcher from source

```sh
git clone https://github.com/OpenRAC/OpenRAC.git && cd OpenRAC/launcher
corepack enable && yarn install && yarn tauri dev
```

The launcher finds the checkout it runs from. Turn on **developer tools** in
its Settings: it then builds the PC port from your tree and runs the
decompilations' builds and checks for you. More in
[launcher/README.md](../launcher/README.md).

## Set up the decompilations

Put images of your own discs in `baserom/` ([baserom/README.md](../baserom/README.md)
lists the supported discs and their checksums), then:

```sh
python3 tools/openrac.py discs     # identify and check your disc images
python3 tools/openrac.py setup     # put each game's files where its build expects them
```

`setup` ends by printing the next command for each game. The compilers are not
free software and are not included: [toolchains/README.md](../toolchains/README.md)
says what each game needs. Then follow the game's own README, for example
[games/rac1](../games/rac1/README.md). Each game and version is its own
project, built from its own directory; [games/README.md](../games/README.md)
lists them all and explains how they relate.

## The PC port

Ratchet & Clank (PAL) is the first game on PC. The port ([port/](../port/README.md))
builds the game's whole decompilation into one program that runs the game's own
`main`. OpenRAC draws it with its own OpenGL renderer and plays its sound
effects, music and speech. Code that is still assembly is logged and skipped.

```sh
cd port && cmake --preset release && cmake --build --preset release && ctest --preset release
```

The design and the order of work are in [docs/port](port/README.md). To help
test and fix it, start with [docs/port/WORKFLOW.md](port/WORKFLOW.md). The
port's code follows its [coding conventions](../port/docs/CODING_CONVENTIONS.md).

## The level editor from the command line

The launcher's **Mods** tab does all of this for you. By hand, for
Ratchet & Clank (PAL):

```sh
# Turn the 19 levels on your disc into Godot projects (about 20 seconds).
python3 editor/extract.py godot baserom/SCES_509.16.iso assets/levels/rac1/pal --split

# Open one in the Godot 4 editor (level_00 is Veldin).
godot --path assets/levels/rac1/pal/level_00 -e

# Pack your edits into game data. Your disc is not changed.
python3 editor/extract.py pack baserom/SCES_509.16.iso assets/levels/rac1/pal assets/packed/rac1/pal
```

The PC port plays the packed levels with `--mods assets/packed/rac1/pal`.
[editor/README.md](../editor/README.md) covers the rest: moby classes, pvars
and each level's gameplay data. To change how things behave, use the mod SDK
([docs/port/MODS.md](port/MODS.md)).

## Repository commands

Run these from the top of the checkout. Each game's own build and check
commands are in its README and in [workflow/](workflow/README.md).

```sh
python3 tools/openrac.py discs              # identify and check the images in baserom/
python3 tools/openrac.py setup [GAME/VER]   # place each game's inputs
python3 tools/openrac.py progress [--fetch] # consolidate progress, refresh the README tables
python3 tools/openrac.py tables             # refresh the generated tables only
python3 tools/sources.py status|sync GAME/VER   # follow the original repositories
python3 tools/xmap.py scan|report|ports FROM TO # the function map across the games
python3 tools/port.py FROM TO [--check]         # functions FROM matched that TO has unchanged
python3 tools/shared.py check|find|sync PATH    # files several games share stay identical
python3 tools/permuter run|base|queue PROFILE FUNC   # decomp-permuter for any game
python3 -m unittest discover -s tools       # the tools' tests
python3 -m unittest discover -s editor      # the editor's tests
```

## Before you send a change

- Read [CONTRIBUTING.md](../CONTRIBUTING.md) and the
  [sourcing policy](policy/SOURCING.md). Your own disc is the evidence.
  Leaked material and SDK source, samples or headers are never used.
- Commits follow Conventional Commits, for example `feat(rac1/pal): ...`.
- Decisions still to be made are in
  [policy/OPEN_QUESTIONS.md](policy/OPEN_QUESTIONS.md).
- OpenRAC's own code is being restructured, package by package.
  [restructure/PROGRESS.md](restructure/PROGRESS.md) says what is done, who
  works on what and what is free to pick up.

[LAYOUT.md](LAYOUT.md) explains how the repository is organised and where a
new file goes.
