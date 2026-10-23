# The restructure: progress

OpenRAC's own code is being restructured: the port condensed into modules with one way to do
each thing, the Python tools and the editor sharing their helpers, the launcher's logic moved into
its tested core. It lays the foundation for converting the port's host code entirely to C++. The
games' decompiled C under `games/` is not part of it and is never edited by it: it is synced from
the decompilation projects and stays byte-identical.

The work is cut into packages: each one small enough to review, with its paths, its checks and
what it waits for. This page is where anyone, people or agents, sees what is done, what is being
worked on and by whom, and what is free to pick up. Each package updates its own row in the same
commit series as its work.

The rules every package follows are [port/docs/CODING_CONVENTIONS.md](../../port/docs/CODING_CONVENTIONS.md)
(decided), [AGENTS.md](../../AGENTS.md) and [CONTRIBUTING.md](../../CONTRIBUTING.md).

**Status:** `todo` (free to claim once what it depends on is done), `in progress` (claimed: see the
owner), `done` (landed on master and verified).

---

## Phase 0: guardrails and the conventions

Independent of each other.

| Id | Package | Paths | Depends on | Status | Owner | Commits |
|---|---|---|---|---|---|---|
| P0-01 | The port's coding conventions, decided, and the checks that enforce them; this page | `port/docs/`, `port/.clang-tidy`, `port/tools/conventions/`, `.github/workflows/checks.yml` | | done | maintainer | [f40d8bd8](https://github.com/OpenRAC/OpenRAC/commit/f40d8bd8), [419d8986](https://github.com/OpenRAC/OpenRAC/commit/419d8986), [3a925aad](https://github.com/OpenRAC/OpenRAC/commit/3a925aad), [23482f3c](https://github.com/OpenRAC/OpenRAC/commit/23482f3c), and this page |
| P0-02 | Layering checker: includes against the allowed module edges, today's violations as known exceptions, new ones fail | `port/tools/layering/` (new) | | done | maintainer | [08656987](https://github.com/OpenRAC/OpenRAC/commit/08656987), [d2e768a8](https://github.com/OpenRAC/OpenRAC/commit/d2e768a8), and this page |
| P0-03 | CMake test helpers (`openrac_add_test`, `openrac_add_c_test`); the copy-pasted test stanzas converted | `port/cmake/Tests.cmake` (new), `port/CMakeLists.txt` | | in progress | maintainer | |
| P0-04 | Retire the ReRAC conversion's dead build gate and the stale WIP pages | `port/cmake/Assets.cmake`, `port/cmake/Window.cmake`, the WIP pages (deleted), `docs/TODO.md`, `THIRD_PARTY_NOTICES.md` | | done | maintainer | [070ece9c](https://github.com/OpenRAC/OpenRAC/commit/070ece9c), [11ded3b4](https://github.com/OpenRAC/OpenRAC/commit/11ded3b4), [4406bced](https://github.com/OpenRAC/OpenRAC/commit/4406bced), and this page |
| P0-05 | Shared Python helpers package, with tests (no callers migrated yet) | `tools/openrac_common/` (new), `tools/test_openrac_common.py` (new), `docs/LAYOUT.md` | | done | maintainer | [a3c7e671](https://github.com/OpenRAC/OpenRAC/commit/a3c7e671), and this page |

## Phase 1: independent extractions, away from the busy files

Group A is the port; group B is everything else and shares no file with group A.

| Id | Package | Paths | Depends on | Status | Owner | Commits |
|---|---|---|---|---|---|---|
| P1-01 | `common/` gets `json` (from `viewer/`) and `ps2_float` (from `assets/`) | `port/viewer/json.*`, `port/assets/ps2_float.*`, `port/common/`, include lines | P0-03 | todo | | |
| P1-02 | Split `renderer/vulkan/vkgl.cpp` (4,878 lines) into translation units, a pure move | `port/renderer/vulkan/`, `port/cmake/Window.cmake` | | todo | | |
| P1-03 | `common/env.h`: one table of `OPENRAC_*` variables, tested against DEBUGGING.md; `getenv` migrated in runtime, platform, renderer | `port/common/env.*`, `port/runtime/`, `port/platform/`, `port/renderer/`, a test | P0-03, P1-01 | todo | | |
| P1-04 | rac2/rac4 host glue: one shared WAD and trig implementation, per-game addresses kept per game | `port/game/rac2-ntsc/host/`, `port/game/rac4-ntsc/host/`, `port/game/common/lib/` (new files) | | todo | | |
| P1-05 | `tools/*.py` use `tools/openrac_common` | `tools/*.py` | P0-05 | todo | | |
| P1-06 | Editor: one copy of each helper; disc identity from `game.json`; tests for `cparse.py`, `spaceships.py` | `editor/*.py` | | todo | | |
| P1-07 | Launcher: logic out of `src-tauri` into the core, with tests | `launcher/src-tauri/src/lib.rs`, `launcher/core/src/`, `launcher/docs/ARCHITECTURE.md` | | todo | | |
| P1-08 | Launcher frontend: shared helpers, dead exports removed, STYLE.md fixes | `launcher/src/lib/`, the components and pages that repeat them | | todo | | |
| P1-09 | One manifest test-vector file for the three manifest validators; one `fetch_sdk.py` | `sdk/tests/manifests.json` (new), the validators' tests, `sdk/template*/tools/fetch_sdk.py` | | todo | | |

## Phase 2: the host program's structure

These touch the busiest files (`frontend.cpp`, `main.cpp`, `debug_overlay.cpp`, `game/rac1/`), so
they run one at a time per file, after the level loader and moby pool work has landed, and the rac1
ones after the next rac1 source sync. Each split is two commits: a pure move, then the interface
changes.

| Id | Package | Depends on | Status | Owner | Commits |
|---|---|---|---|---|---|
| P2-01 | Characterisation tests for the frontend's pure helpers (frame box, aspect, frame size, pacing, camera blend), moved to `frontend/geometry` and `frontend/pacing` | P0-03 | todo | | |
| P2-02 | The `scene` module (`openrac_scene`) from `viewer/`: level, level renderer, moby pose and shadow, PVS, screenshot; `viewer/` becomes the program | P1-01 | todo | | |
| P2-03 | `host::GameProfile` for rac1: the game state and the frontend's addresses and game-id branches into `game/rac1/profile.cpp`, tables per disc | P2-02, the rac1 sync, the moby pool work | todo | | |
| P2-04 | Split `frontend.cpp`: window and desktop integration, pacing, interpolation, world submission, input, the frame in named steps | P2-01, P2-03 | todo | | |
| P2-05 | Split `debug_overlay.cpp` by panel, and `main.cpp` into options, vsync hook, program start | P2-04 | todo | | |
| P2-06 | Raw GL confined to `renderer/`: `renderer/ui/` for the ImGui backend, touch overlay, FPS counter, debug menu drawing | P2-05 | todo | | |
| P2-07 | `renderer` stops including `enhanced/`: renderer and scene own the hook interfaces, enhanced registers with them | P2-02, P2-06 | todo | | |
| P2-08 | `common/result.h` (`openrac::Result<T>`) and the C-boundary wrapper, applied to `game_host`, `mods`, `mod_scripts` entry points | P0-01 | todo | | |
| P2-09 | The remaining `getenv` (game/common, renderer/vulkan) onto `common/env.h` | P1-02, P1-03, P2-05 | todo | | |
| P2-10 | Python packages: `port/tools` and `editor` use `openrac_common`; the `sys.path.insert` calls go; the longest functions split | P1-05, P1-06 | todo | | |
| P2-11 | `editor/catalogue.py` moves to `sdk/tools/` | P1-06 | todo | | |
| P2-12 | Launcher: TypeScript types generated from Rust; `mock.ts` built on them; `src-tauri`'s `run` split | P1-07, a maintainers' pick (question 5) | todo | | |

## Phase 3: conversion and deeper abstractions

Partly gated on the maintainers' decisions below.

| Id | Package | Depends on | Status | Owner | Commits |
|---|---|---|---|---|---|
| P3-01 | `game/common/lib/*.c` (kernel, memory card, CD, graph, pad, sound, libc, VU0) to C++ behind the unchanged `game_lib.h`, with characterisation tests first | P2-08 | todo | | |
| P3-02 | The game-agnostic host: `openrac_host` and one `GameProfile` per version; `game/<id>/` reduces to tables and game C | P2-03, P2-04 | todo | | |
| P3-03 | The Vulkan backend's and the GL loader's state into context objects | P1-02 | todo | | |
| P3-04 | The renderer's interface: GL 4.1 as the decided interface with Vulkan carrying it out, or a narrower one; and which world renderer stays | question 1 and 2 | todo | | |
| P3-05 | One source of truth for the level formats' readers (Python in `editor/` or C++ in `port/assets/`) | question 3 | todo | | |
| P3-06 | rac1 PAL and NTSC `hand/` C as one copy with per-disc tables, or its move into `games/` as backfill | the PAL campaign, the rac1 sync | todo | | |

---

## How to help

### Claiming a package

1. Pick a `todo` row whose "Depends on" packages are all `done`. Phase 2 and 3 rows also wait for
   the work in flight below.
2. Claim it before you start: a commit (or pull request) that sets its status to `in progress` and
   its owner to your name or handle. Two people never work on one package; if the owner has been
   quiet for a week, ask on the pull request or issue before taking it over.
3. Work on a branch named `restructure/<id>`, in your own checkout or worktree.

### The rules

- Refactor only: behaviour does not change. Where coverage is thin, add characterisation tests
  first, in their own commit.
- Follow [CODING_CONVENTIONS.md](../../port/docs/CODING_CONVENTIONS.md) in everything you write or
  touch, and its migration rules (section 24): one module at a time, never a sweep.
- Never touch `games/`. Never hand-edit a generated file. Never change a C contract (`guest.h`,
  `game_host.h`, `game_lib.h`, `openrac_mod.h`) as part of a refactor.
- No new abstraction without two users; no big-bang renames or directory shuffles; no rewrite of
  verified behaviour.
- Touch only your package's paths. If you find you must touch another, say so on the pull request
  before you do.
- Commits follow [CONTRIBUTING.md](../../CONTRIBUTING.md#commits): Conventional Commits, one
  logical change each, every commit leaving the build and the tests working.

### Verification

Run what your paths need, and say what you ran:

```sh
# games/ untouched (every package)
git diff --quiet origin/master -- games/ && test -z "$(git status --porcelain -- games/)"
# the conventions (every package)
python3 port/tools/conventions/check_conventions.py --since origin/master --touched
# the layering (every package that adds, moves or removes a port include)
python3 port/tools/layering/layering.py
# the port
cd port && cmake --preset release && cmake --build --preset release && ctest --preset release
cd port && cmake --preset headless && cmake --build --preset headless && ctest --preset headless
python3 -m unittest discover -s port/tools/hostgen
# Python
python3 -m unittest discover -s tools
python3 -m unittest discover -s editor
python3 -m unittest discover -s sdk/tools
# the launcher
cd launcher && yarn verify && cargo test -p openrac-launcher-core
```

A package that touches CMake records `ctest -N` before and after (same tests, same count). A
package that touches the game's side of the port shows hostgen's generated C unchanged (the hashes
of `port/build/release/games/*/gen/*.c` before and after). The gameplay regression comparison
([REGRESSION.md](../port/REGRESSION.md)) opens windows and is run by the maintainers; list the
command in your report rather than running it.

### Reporting

When the package has landed, update its row in the same commit series: status `done`, the commits
(linked), and anything you found that belongs to another package under "Follow-ups" below. The
pull request or report says what changed, the commands you ran and their results, failures
included, and what was left undone.

---

## Work in flight that packages wait for

| Work | Touches | Waits for it |
|---|---|---|
| The level loader and moby pool | `port/CMakeLists.txt`, rac1's `libraries.tsv`, `port/game/rac1/host/`, `port/game/rac1/differences.json`, its tests | The moby pool landed in [c712c679](https://github.com/OpenRAC/OpenRAC/commit/c712c679): P0-03, P1-01 and P1-03 branch from it or later. The level loader's further work still holds P2-03, P2-04, P2-05, P3-02, P3-06 |
| The next rac1 source sync (symbol renames) | `games/rac1/`, and in its wake rac1's tables, `hand/`, `host/`, the script binding | P2-03, P3-02, P3-06 |
| The PAL campaign (backfill, fixes) | `port/game/rac1*`, now and then `frontend.cpp` and `main.cpp` | the phase 2 splits of `game/common` need a short announced freeze each |
| Launcher releases | `launcher/src-tauri/src/lib.rs`, `api.ts`, `mock.ts` | P1-07 rebases on the day it lands; P2-12 after a release |

## Questions for the maintainers

| # | Question | State |
|---|---|---|
| 1 | The renderer's interface: keep GL 4.1 as the interface with Vulkan carrying it out, or plan a narrower one | open |
| 2 | One world renderer: the scene's (`LevelScene`), `renderer/world`, or one feeding the other | open |
| 3 | Asset readers: Python in `editor/` or C++ in `port/assets/` as the source of truth | open (DESIGN.md, open decision 6) |
| 4 | The conventions page | decided 2026-10-22: [CODING_CONVENTIONS.md](../../port/docs/CODING_CONVENTIONS.md), with `Result<T>` and the C-boundary rule |
| 5 | TypeScript types for the launcher: ts-rs, specta, or none | open |
| 6 | The C++ floor | decided 2026-10-22: C++20; C++23 waits for every target toolchain |
| 7 | May refactor packages run the hidden-window regression comparison without asking each time | open |

## Follow-ups

Found while doing a package, for another package or the maintainers.

| Found by | Follow-up | For |
|---|---|---|
| P0-01 | The plan lists `memory.h` among the C contracts; it is a C++ header of the runtime (`namespace openrac::runtime`), as is `elf.h`. The conventions page lists the C contracts as they are. | the plan |
| P0-01 | CI reports clang-format's changes on changed lines without failing. Make it fail (`--strict-touched` in checks.yml) once phase 1 has landed. | after phase 1 |
| P0-01 | clang-tidy is not run in CI (it needs a configured build). A job that runs it on the C++ files a pull request changes would hold new code to it. | a later CI package |
| P0-01 | The module READMEs predate section 7.7's template; each gains the missing sections when its module is restructured. | each module's package |
| P0-01 | `check_conventions.py` gains a `getenv` rule when `common/env.h` lands. | P1-03 |
| P0-02 | CI does not run the layering check. A step beside "Port conventions" in checks.yml running `layering.py` and its tests would hold every push to it. | a later CI package |
| P0-02 | A package that moves or removes an include listed in `port/tools/layering/rules.tsv` removes or updates its row in the same commit series (the check warns until it does): P1-01 (json, ps2_float), P2-02 (the scene files), P2-03 (the game state), P2-07 (GTAO). | P1-01, P2-02, P2-03, P2-07 |
| P0-02 | `viewer/level.h`, `level_renderer.h` and `level_renderer.cpp` include `enhanced/` (6 includes), allowed while they sit in a program's directory. Moved to `scene/`, they become scene -> enhanced: list them in rules.tsv as exceptions until P2-07 inverts them. | P2-02 |
| P0-02 | Moving the game state into `game/rac1` must not leave `game/common` including `game/rac1` (a higher layer): the host reads it through `host::GameProfile` only. | P2-03 |
| P0-02 | `platform/window.cpp` opens the Vulkan backend (`renderer/vulkan/vkgl.h`), platform -> renderer. No package removes it: the renderer could open its backend on the window platform hands it. | the plan |
| P0-02 | The check reads `#include "<dir>/..."` only: an include without a directory found through a target's own include directories, and the CMake link graph (section 19), are checked by review. | a later package |
| P0-04 | The `headless` preset does not build on master: `port/game/common/main.cpp` calls `openrac_sound_stop_readers` in `std::atexit` outside `#ifdef OPENRAC_FRONTEND`, the only place `openrac/sound_engine.h` is included. CI builds no headless preset, so nothing catches it. The atexit belongs inside the `#ifdef`, and a CI job could build the preset. | the `main.cpp` owner, P2-05 |
| P0-05 | `tools/bundle_standalone.py` copies only `openrac.py` and `extractor.py` into the launcher's bundle. When they move onto `openrac_common`, the bundle carries `tools/openrac_common/` too (and its test checks it); `paths.repo_root` already accepts the bundle's `STANDALONE` marker. | P1-05 |
| P0-05 | A directory record cut short at the end of a directory raises `struct.error` in `iso9660`, as it does in the extractor's `walk` today, so a damaged image crashes the extractor instead of giving error 4020. Turning it into an `IsoError` changes what a player sees: a commit of its own after the extractor has moved. | P1-05 |
| P0-05 | `tools/fcheck_gate.py:171,374` default `OPENRAC_ROOT` to `~/Projects/RAC/OpenRAC` and `tools/fcheck_level_build.py:44` takes `Path.cwd()` as the root. `paths.repo_root()` is the candidate, but the default names where the port is built, which a worktree may not be: decide it when they move. | P1-05 |
| P0-05 | `editor/disc.py` and `editor/wads.py` keep stricter checks than `iso9660` (root directory only, 2048-byte logical blocks, no subdirectory or multi-extent file) and a `BOOT2` regex that needs `cdrom0:\`. Moving the editor onto `iso9660` keeps those checks on the editor's side. | P1-06, P2-10 |
| P0-05 | `tools/test_openrac_common.py` imports the existing copies from `port/tools/hostgen`, `port/tools/versionmap`, `sdk/tools`, `editor` and `port/enhanced/tools/probes` to compare against them. A package that deletes a copy deletes its comparison in the same commit. | P1-05, P1-06, P2-10 |
