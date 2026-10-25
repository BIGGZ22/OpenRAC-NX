# How the repository is organised

```
OpenRAC/
├── README.md, CONTRIBUTING.md
├── LICENSE.md, THIRD_PARTY_NOTICES.md, CREDITS.md
├── games/                  one directory per game, one per version inside it
│   ├── rac1/               Ratchet & Clank (2002)
│   │   ├── game.json       discs, checksums, what is decompiled, where it came from
│   │   ├── pal/            SCES_509.16, from rac1-decomp
│   │   └── ntsc/           SCUS_971.99, from Lombyte
│   ├── rac2/ntsc/          Going Commando, SCUS_972.68 v1.01, from rac2-decomp
│   ├── rac3/ntsc/          Up Your Arsenal, SCUS_973.53, from rac3-uya-decomp
│   ├── rac4/ntsc/          Deadlocked, SCUS_974.65, from rac-deadlocked-decomp
│   ├── rac5/ntsc/          Size Matters, SCUS_976.15, from rac5-sm-decomp
│   └── rac6/ntsc/          Secret Agent Clank, SCUS_976.23, from rac-sac-decomp
├── editor/                 Godot level editor and extractor (shared)
├── launcher/               desktop app to set up, build and play every game (Tauri)
├── port/                   the native PC port: runtime, hostgen, renderer, platform, the games' programs
├── tools/                  repository-wide tools: openrac.py, extractor.py, sources.py
│   └── openrac_common/     the helpers the Python tools share (paths, game.json, tables, discs)
├── docs/                   knowledge and rules that span the games
│   ├── assets/             the logo and the pictures README.md shows
│   ├── decomp/             the decompilations across games: workflows, compilers, rules
│   ├── policy/             sourcing policy, open questions
│   ├── port/               the native port: design, studies, order of work
│   ├── restructure/        the restructure's progress: packages, owners, how to help
│   └── engine/             what is known about the engine, across games
├── progress/               consolidated progress (generated)
├── shared/                 what the games have in common: the function map, the shared-files list
├── baserom/                your own disc images (ignored, except its README)
└── toolchains/             compilers you supply (ignored, except its README)
```

## The rules that keep it organised

**Each game version is its own project.** It lives in
`games/<game>/<version>/`, or in the `dir` its `game.json` entry names. It has
its own README, license, build, tools and docs. Work from inside that
directory: its paths are relative to it, and its build never reaches into
another game. Its own docs decide how a match is proven there. rac1 is being
merged into one tree at `games/rac1/` (the PAL tree), with NTSC-U built from
it ([games/rac1/VERSIONS.md](../games/rac1/VERSIONS.md)).

**Only shared things live at the top level**: things that serve more than one
game, or the repository itself. That is the editor, the launcher,
`tools/openrac.py`, the extractor, the policies and the cross-game references.
A game may use top-level parts (rac1/pal uses the editor's readers). The top
level never depends on one game's internals, apart from the per-game
knowledge the editor decodes.

**Generated and personal files stay out of git.** Disc images, extracted
data, assembly, build output, the generated Godot projects and toolchains are
ignored, by the top-level `.gitignore` and by each game's own. `progress/`
and the tables in `README.md` are generated but committed, because they are
what readers see.

## Where a new file goes

| You are adding | Put it in |
|---|---|
| The native port's code | `port/` ([its README](../port/README.md)): a game's own part in `port/game/<game>/`, following [its coding conventions](../port/docs/CODING_CONVENTIONS.md) |
| A study or decision for the native port | `docs/port/` ([its README](port/README.md)) |
| Launcher code, or a game's actions in the launcher | `launcher/` (a game's actions: `launcher/actions.json`) |
| Editor code | `editor/`; a helper two or more of its modules need in `editor/util.py` (one that raises `FormatError` in `formats.py`), with a test, until P2-10 moves the editor onto `tools/openrac_common` |
| A tool that serves several games or the repository | `tools/` (standard library Python where possible), with tests, using [`tools/openrac_common`](../tools/openrac_common/README.md) for what tools share |
| A helper two or more Python tools need | `tools/openrac_common/`, with a test beside the others in `tools/test_openrac_common.py` |
| A decision or rule for everyone | `docs/policy/`, and `CONTRIBUTING.md` if it changes how people work |
| Restructure work: claiming a package, reporting it | its row in [docs/restructure/PROGRESS.md](restructure/PROGRESS.md) |
| Decompiled code, a symbol map, a game's build change | `games/<game>/<version>/`, following that project's layout |
| A note about one game | that game's `docs/` |
| Knowledge that holds across games (a format, a subsystem) | `docs/engine/`, linking the per-game evidence |
| Matching or compiler knowledge across games | `docs/decomp/` |
| A file a second game needs unchanged (a library source, a build helper) | a copy in that game, listed with the original in `shared/files.json` ([shared/](../shared/README.md)) |
| A new game version | `games/<game>/<version>/`, plus its entry in `games/<game>/game.json` (the editor reads a disc's identity from it: boot executable, size, SHA-1, region, disc version) |

## Names

- Games are `rac1`, `rac2`, `rac3`, `rac4`, `rac5` and `rac6` in release order. `rac4` is
  Ratchet: Deadlocked (Ratchet: Gladiator in Europe); `rac5` is the PlayStation 2
  port of Ratchet & Clank: Size Matters; `rac6` is the PlayStation 2 port of
  Ratchet & Clank: Secret Agent Clank.
- Versions are named by region: `pal` (Europe), `ntsc` (North America,
  NTSC-U). A Japanese release would be `ntsc-j`. The serial and disc version
  are in `game.json`.
- Commit scopes use the same names: `rac1/pal`, `rac1/ntsc`, `rac2`, `rac3`,
  `rac4`, `rac5`, `rac6`, and `editor`, `launcher`, `port`, `tools`, `docs`, `progress` for the top level
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
PC directly (`#when PC_PORT`), so it has one copy of the game. OpenRAC's
`games/` holds C that still builds the PS2 executable byte for byte, and most
of it follows projects of their own (rac1-decomp, Lombyte). So the PC's
changes to the game live beside it, in `port/game/<game>-<version>/`
(`hand/`, `host/`, `libraries.tsv`, `split_places.tsv`), and hostgen applies
them. Each version needs its own copy of those changes.

Where it goes from here:

- `port/` stays: it is the engine, like OpenGOAL's `game/`.
- `port/game/<game>-<version>/` shrinks over time:
  - A change the game's C needs on the PC (an argument the PS2 passed in a
    register, a call through an address) goes into `games/` as C that builds
    the same PS2 bytes, and is offered to the project that tree follows.
    Changes kept only in OpenRAC's copy are listed in that tree's
    `OPENRAC_PATCHES.md`, so a sync keeps them.
  - Code that can only be PC code goes into `games/` under
    `#ifdef OPENRAC_PC` once that project accepts it. Until then it stays in
    `port/game/`.
- What is built from a disc has one root per game and one folder per
  version. The levels the port draws are in `assets/port/<game>/<version>`
  in a checkout and in `<data>/port/<version>` in an installed launcher,
  where the game also looks when `--levels` is not given. No version gets a
  sibling folder named after it next to these.

## Where this is heading

The layout keeps each project buildable as it was. Later steps the group has
discussed, in [OPEN_QUESTIONS.md](policy/OPEN_QUESTIONS.md): one RAC1 tree
that builds both regions, a shared tree for code that is identical across
games, and one build container for every game.
