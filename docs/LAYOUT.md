# How the repository is organised

```
OpenRAC/
├── README.md, CONTRIBUTING.md, AGENTS.md, CLAUDE.md
├── LICENSE.md, THIRD_PARTY_NOTICES.md, CREDITS.md
├── games/                  one directory per game, one per version inside it
│   ├── rac1/               Ratchet & Clank (2002)
│   │   ├── game.json       discs, checksums, what is decompiled, where it came from
│   │   ├── pal/            SCES_509.16, from rac1-decomp
│   │   └── ntsc/           SCUS_971.99, from Lombyte
│   ├── rac2/ntsc/          Going Commando, SCUS_972.68 v1.01, from rac2-decomp
│   ├── rac3/ntsc/          Up Your Arsenal, SCUS_973.53, from rac3-uya-decomp
│   ├── rac4/ntsc/          Deadlocked, SCUS_974.65, from rac-deadlocked-decomp
│   └── rac5/ntsc/          Size Matters, SCUS_976.15, from rac5-sm-decomp
├── editor/                 Godot level editor and extractor (shared)
├── launcher/               desktop app to set up, build and play every game (Tauri)
├── port/                   the native PC port: runtime, hostgen, renderer, platform, the games' programs
├── tools/                  repository-wide tools: openrac.py, extractor.py, sources.py
│   └── openrac_common/     the helpers the Python tools share (paths, game.json, tables, discs)
├── docs/                   knowledge and rules that span the games
│   ├── policy/             sourcing policy, open questions
│   ├── port/               the native port: design, studies, order of work
│   ├── restructure/        the restructure's progress: packages, owners, how to help
│   ├── engine/             what is known about the engine, across games
│   ├── toolchains/         which compilers built what
│   └── workflow/           how matching works in each game
├── progress/               consolidated progress (generated)
├── shared/                 what the games have in common: the function map, the shared-files list
├── baserom/                your own disc images (ignored, except its README)
└── toolchains/             compilers you supply (ignored, except its README)
```

## The rules that keep it organised

**A game version is a self-contained project.** `games/<game>/<version>/` (or the `dir` its `game.json` entry names: rac1 is being merged into one tree at `games/rac1/`, the PAL tree, with NTSC-U built from it; see games/rac1/VERSIONS.md)
has its own README, license, build, tools and docs, and you work on it from
inside that directory: its paths are relative to it, and its build never
reaches into another game. Its own docs are the authority on how a match is
proven there.

**Shared things live at the top level, and only shared things.** A tool,
document or component goes to the top level when it serves more than one
game, or the repository itself: the editor, the launcher, `tools/openrac.py`,
the extractor, the policies, the cross-game references. A game may use
top-level components (rac1/pal uses the editor's readers); the top level
never depends on one game's internals, apart from the per-game knowledge the
editor decodes.

**Generated and personal files stay out of git.** Disc images, extracted
data, assembly, build output, the generated Godot project and toolchains
are ignored, at the top level and in each game's own `.gitignore`.
`progress/` and the tables in `README.md` are generated but committed: they
are what readers see.

## Where a new file goes

| You are adding | Put it in |
|---|---|
| Decompiled code, a symbol map, a game's build change | `games/<game>/<version>/`, following that project's layout |
| A note about one game | that game's `docs/` |
| Knowledge that holds across games (a format, a subsystem, a compiler) | `docs/engine/`, `docs/toolchains/` or `docs/workflow/`, linking the per-game evidence |
| A tool that serves several games or the repository | `tools/` (standard library Python where possible), with tests, using [`tools/openrac_common`](../tools/openrac_common/README.md) for what tools share |
| A helper two or more Python tools need | `tools/openrac_common/`, with a test beside the others in `tools/test_openrac_common.py` |
| A file a second game needs unchanged (a library source, a build helper) | a copy in that game, listed with the original in `shared/files.json` ([shared/](../shared/README.md)) |
| Editor code | `editor/`; a helper two or more of its modules need in `editor/util.py` (one that raises `FormatError` in `formats.py`), with a test, until P2-10 moves the editor onto `tools/openrac_common` |
| A study or decision for the native port | `docs/port/` ([its README](port/README.md)) |
| The native port's code | `port/` ([its README](../port/README.md)): a game's own part in `port/game/<game>/`, following [its coding conventions](../port/docs/CODING_CONVENTIONS.md) |
| Restructure work: claiming a package, reporting it | its row in [docs/restructure/PROGRESS.md](restructure/PROGRESS.md) |
| Launcher code, or a game's actions in the launcher | `launcher/` (a game's actions: `launcher/actions.json`) |
| A decision or rule for everyone | `docs/policy/`, and `AGENTS.md` and `CONTRIBUTING.md` if it changes how people work |
| A new game version | `games/<game>/<version>/`, plus its entry in `games/<game>/game.json` (the editor reads a disc's identity from it: boot executable, size, SHA-1, region, disc version) |

## Names

- Games are `rac1`, `rac2`, `rac3`, `rac4` and `rac5` in release order. `rac4` is
  Ratchet: Deadlocked (Ratchet: Gladiator in Europe); `rac5` is the PlayStation 2
  port of Ratchet & Clank: Size Matters.
- Versions are named by region: `pal` (Europe), `ntsc` (North America,
  NTSC-U). A Japanese release would be `ntsc-j`. The serial and disc version
  are in `game.json`.
- Commit scopes use the same names: `rac1/pal`, `rac1/ntsc`, `rac2`, `rac3`,
  `rac4`, `rac5`, and `editor`, `launcher`, `port`, `tools`, `docs`, `progress` for the top level
  ([CONTRIBUTING.md](../CONTRIBUTING.md#commits)).

## The decompilations, the port, and what is built from your disc

The arrangement follows OpenGOAL's (`jak-project`):

| OpenGOAL | OpenRAC | |
|---|---|---|
| `goal_src/<game>` | `games/<game>/<version>` | the decompiled game code |
| `game/` (kernel, graphics, sound, overlord) | `port/` (runtime, renderer, audio, platform) | the PC runtime that hosts it |
| `goalc/` | `port/tools/hostgen` | what makes the game code a PC program |
| `out/<game>/` | `assets/port/<game>/<version>`, `build/` | what is built from your disc |

One difference shapes the rest. OpenGOAL edits its decompiled source for the
PC directly (`#when PC_PORT`), so there is one copy of the game. `games/`
holds C that still builds the PS2 executable byte for byte, most of it
followed from projects of its own (rac1-decomp, Lombyte), so the PC's
changes to the game live beside it, in `port/game/<game>-<version>/`
(`hand/`, `host/`, `libraries.tsv`, `split_places.tsv`), applied by hostgen.
Each version needs its own copy of those changes.

Where it goes from here:

- `port/` stays: it is the engine, OpenGOAL's `game/`.
- `port/game/<game>-<version>/` shrinks over time. A change the game's own C
  needs on the PC (an argument the PS2 passed in a register, a call through
  an address) goes into `games/` as C that builds the same PS2 bytes, and is
  offered to the project the tree follows; changes kept only in OpenRAC's
  copy are listed in that tree's `OPENRAC_PATCHES.md` so a sync keeps them.
  What can only be PC code goes under `#ifdef OPENRAC_PC` in `games/` once
  that project takes it; until then it stays in `port/game/`.
- What is built from a disc has one root per game and a folder per
  version: the levels the port draws are `assets/port/<game>/<version>` in a
  checkout and `<data>/port/<version>` in an installed launcher, where the
  game also looks without `--levels`. No version gets a sibling folder of
  its own name beside them.

## Where this is heading

The layout keeps each project buildable as it was. Later steps the group has
discussed, in [OPEN_QUESTIONS.md](policy/OPEN_QUESTIONS.md): one RAC1 tree
that builds both regions, a shared tree for code that is identical across
games, and one build container for every game.
