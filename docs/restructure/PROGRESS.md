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
| P0-03 | CMake test helpers (`openrac_add_test`, `openrac_add_c_test`); the copy-pasted test stanzas converted | `port/cmake/Tests.cmake` (new), `port/CMakeLists.txt`, section 19 of `port/docs/CODING_CONVENTIONS.md` | | done | maintainer | [b7ebf0ba](https://github.com/OpenRAC/OpenRAC/commit/b7ebf0ba), [a3a6e9be](https://github.com/OpenRAC/OpenRAC/commit/a3a6e9be), [2569af5d](https://github.com/OpenRAC/OpenRAC/commit/2569af5d), and this page |
| P0-04 | Retire the ReRAC conversion's dead build gate and the stale WIP pages | `port/cmake/Assets.cmake`, `port/cmake/Window.cmake`, the WIP pages (deleted), `docs/TODO.md`, `THIRD_PARTY_NOTICES.md` | | done | maintainer | [070ece9c](https://github.com/OpenRAC/OpenRAC/commit/070ece9c), [11ded3b4](https://github.com/OpenRAC/OpenRAC/commit/11ded3b4), [4406bced](https://github.com/OpenRAC/OpenRAC/commit/4406bced), and this page |
| P0-05 | Shared Python helpers package, with tests (no callers migrated yet) | `tools/openrac_common/` (new), `tools/test_openrac_common.py` (new), `docs/LAYOUT.md` | | done | maintainer | [a3c7e671](https://github.com/OpenRAC/OpenRAC/commit/a3c7e671), and this page |
| P0-06 | Master's CI green again (the launcher's Windows test, the Linux build of the probe baker, the macOS copy without its baker), and batch 1's decisions: the target `openrac_renderer_vulkan`, the pipeline cache saved without a race (a deliberate fix of behaviour), `repo_root` keeping the `STANDALONE` copy, the headless preset building and in CI | `launcher/core/src/`, `tools/build_port.py`, `port/common/joining_thread.h` (new), `port/enhanced/lighting/bake.cpp`, `port/renderer/vulkan/`, `port/cmake/Window.cmake`, `port/CMakeLists.txt`, `port/game/common/main.cpp`, `.github/workflows/checks.yml`, `tools/openrac_common/` | | done | maintainer | [513484d3](https://github.com/OpenRAC/OpenRAC/commit/513484d3), [d9340289](https://github.com/OpenRAC/OpenRAC/commit/d9340289), [856225c1](https://github.com/OpenRAC/OpenRAC/commit/856225c1), [9c0a779f](https://github.com/OpenRAC/OpenRAC/commit/9c0a779f), [33f778ca](https://github.com/OpenRAC/OpenRAC/commit/33f778ca), [05f5b41c](https://github.com/OpenRAC/OpenRAC/commit/05f5b41c), [50702a3e](https://github.com/OpenRAC/OpenRAC/commit/50702a3e), [a3c8f3c0](https://github.com/OpenRAC/OpenRAC/commit/a3c8f3c0), [d09d4c01](https://github.com/OpenRAC/OpenRAC/commit/d09d4c01), [f4b4c2d4](https://github.com/OpenRAC/OpenRAC/commit/f4b4c2d4), and this page |

## Phase 1: independent extractions, away from the busy files

Group A is the port; group B is everything else and shares no file with group A.

| Id | Package | Paths | Depends on | Status | Owner | Commits |
|---|---|---|---|---|---|---|
| P1-01 | `common/` gets `json` (from `viewer/`) and `ps2_float` (from `assets/`) | `port/viewer/json.*`, `port/assets/ps2_float.*`, `port/common/` (and its new README), include lines, `port/CMakeLists.txt`, `port/cmake/Window.cmake` (one link), `port/tools/layering/`, hostgen's test, the docs that named the old paths | P0-03 | done | maintainer | [0caebafa](https://github.com/OpenRAC/OpenRAC/commit/0caebafa), [bcad97eb](https://github.com/OpenRAC/OpenRAC/commit/bcad97eb), [9783822d](https://github.com/OpenRAC/OpenRAC/commit/9783822d), [8e3af9b9](https://github.com/OpenRAC/OpenRAC/commit/8e3af9b9), and this page |
| P1-02 | Split `renderer/vulkan/vkgl.cpp` (4,878 lines) into translation units, a pure move | `port/renderer/vulkan/`, `port/cmake/Window.cmake`, a fix in `port/tools/conventions/`, the docs that named `vkgl.cpp` | | done | maintainer | [9dcfcf8e](https://github.com/OpenRAC/OpenRAC/commit/9dcfcf8e), [92ea4ce0](https://github.com/OpenRAC/OpenRAC/commit/92ea4ce0), [a56c1469](https://github.com/OpenRAC/OpenRAC/commit/a56c1469), [6ae3353c](https://github.com/OpenRAC/OpenRAC/commit/6ae3353c), [6bf93545](https://github.com/OpenRAC/OpenRAC/commit/6bf93545), [e1cee59b](https://github.com/OpenRAC/OpenRAC/commit/e1cee59b), and this page |
| P1-03 | `common/env.h`: one table of `OPENRAC_*` variables, tested against DEBUGGING.md; `getenv` migrated in runtime, platform, renderer | `port/common/env.*` (and its README), `port/tests/env_test.cpp`, `port/CMakeLists.txt`, `port/runtime/`, `port/platform/`, `port/renderer/` (not `vulkan/`), `docs/port/DEBUGGING.md`, one line of `port/tools/layering/rules.tsv` | P0-03, P1-01 | done | maintainer | [037586a0](https://github.com/OpenRAC/OpenRAC/commit/037586a0), [4b3836cb](https://github.com/OpenRAC/OpenRAC/commit/4b3836cb), [ce11de59](https://github.com/OpenRAC/OpenRAC/commit/ce11de59), [6d44bba5](https://github.com/OpenRAC/OpenRAC/commit/6d44bba5), [f6f72093](https://github.com/OpenRAC/OpenRAC/commit/f6f72093), [1111492b](https://github.com/OpenRAC/OpenRAC/commit/1111492b), [b21d1a1f](https://github.com/OpenRAC/OpenRAC/commit/b21d1a1f), and this page |
| P1-04 | rac2/rac4 host glue: one shared WAD and trig implementation, per-game addresses kept per game | `port/game/rac2-ntsc/host/`, `port/game/rac4-ntsc/host/`, `port/game/common/lib/` (new files), the docs that name them | | done | maintainer | [91729357](https://github.com/OpenRAC/OpenRAC/commit/91729357), [bcecca63](https://github.com/OpenRAC/OpenRAC/commit/bcecca63), and this page |
| P1-05 | `tools/*.py` use `tools/openrac_common` | `tools/*.py`, `tools/openrac_common/` (docs and one fix) | P0-05 | done | maintainer | [01efb629](https://github.com/OpenRAC/OpenRAC/commit/01efb629), [e82e5fe4](https://github.com/OpenRAC/OpenRAC/commit/e82e5fe4), [d15bb423](https://github.com/OpenRAC/OpenRAC/commit/d15bb423), [59ad1c31](https://github.com/OpenRAC/OpenRAC/commit/59ad1c31), [a0a90da4](https://github.com/OpenRAC/OpenRAC/commit/a0a90da4), [e2566c52](https://github.com/OpenRAC/OpenRAC/commit/e2566c52), [148ba42c](https://github.com/OpenRAC/OpenRAC/commit/148ba42c), [30ef8159](https://github.com/OpenRAC/OpenRAC/commit/30ef8159), [e00aa248](https://github.com/OpenRAC/OpenRAC/commit/e00aa248), [89503beb](https://github.com/OpenRAC/OpenRAC/commit/89503beb), [548dff41](https://github.com/OpenRAC/OpenRAC/commit/548dff41), and this page |
| P1-06 | Editor: one copy of each helper; disc identity from `game.json`; tests for `cparse.py`, `spaceships.py` | `editor/*.py`, `editor/README.md`, `docs/LAYOUT.md` | | done | maintainer | [62faec21](https://github.com/OpenRAC/OpenRAC/commit/62faec21), [243ecbd5](https://github.com/OpenRAC/OpenRAC/commit/243ecbd5), [255455d6](https://github.com/OpenRAC/OpenRAC/commit/255455d6), [e96d19b6](https://github.com/OpenRAC/OpenRAC/commit/e96d19b6), and this page |
| P1-07 | Launcher: logic out of `src-tauri` into the core, with tests | `launcher/src-tauri/src/lib.rs`, `launcher/core/src/`, `launcher/docs/ARCHITECTURE.md` | | done | maintainer | [6139a70e](https://github.com/OpenRAC/OpenRAC/commit/6139a70e), [6f09f481](https://github.com/OpenRAC/OpenRAC/commit/6f09f481), [d96264c1](https://github.com/OpenRAC/OpenRAC/commit/d96264c1), [5401de6e](https://github.com/OpenRAC/OpenRAC/commit/5401de6e), [e2a8f754](https://github.com/OpenRAC/OpenRAC/commit/e2a8f754), and this page |
| P1-08 | Launcher frontend: shared helpers, dead exports removed, STYLE.md fixes | `launcher/src/lib/`, the components and pages that repeat them | | done | maintainer | [f93c3b99](https://github.com/OpenRAC/OpenRAC/commit/f93c3b99), [4a71a80d](https://github.com/OpenRAC/OpenRAC/commit/4a71a80d), [cd046b10](https://github.com/OpenRAC/OpenRAC/commit/cd046b10), [1517a0df](https://github.com/OpenRAC/OpenRAC/commit/1517a0df), [2f8f4feb](https://github.com/OpenRAC/OpenRAC/commit/2f8f4feb), [c0595d9b](https://github.com/OpenRAC/OpenRAC/commit/c0595d9b), [82b090c2](https://github.com/OpenRAC/OpenRAC/commit/82b090c2), [50eb200c](https://github.com/OpenRAC/OpenRAC/commit/50eb200c), and this page |
| P1-09 | One manifest test-vector file for the three manifest validators; one `fetch_sdk.py` | `sdk/tests/manifests/` (new), the validators' tests, `sdk/template*/tools/fetch_sdk.py`, `sdk/tools/package_sdk.py` | | done | maintainer | [abbadfab](https://github.com/OpenRAC/OpenRAC/commit/abbadfab), [19ff24fe](https://github.com/OpenRAC/OpenRAC/commit/19ff24fe), [31cceee5](https://github.com/OpenRAC/OpenRAC/commit/31cceee5), [f53a71a1](https://github.com/OpenRAC/OpenRAC/commit/f53a71a1), and this page |

## Phase 2: the host program's structure

These touch the busiest files (`frontend.cpp`, `main.cpp`, `debug_overlay.cpp`, `game/rac1/`), so
they run one at a time per file. The rac1 ones wait for the next rac1 source sync. Each split is two
commits: a pure move, then the interface changes. The packages are grouped in three lanes that never
touch the same files: the frontend and scene (P2-13 to P2-18 and P2-01, P2-02, P2-04, P2-09, P2-14),
common/, the C contracts, `main.cpp` and the debug overlay (P2-05, P2-08, P2-19 to P2-27), and work
outside the port (P2-10 to P2-12, P2-28 to P2-35). P2-03, P2-06, P2-07 and P2-36 to P2-39 come after
the lanes, one at a time.

| Id | Package | Depends on | Status | Owner | Commits |
|---|---|---|---|---|---|
| P2-01 | The frontend's pure helpers (frame box, aspect, frame size, pacing, camera blend) moved to `frontend/geometry` and `frontend/pacing`, with characterisation tests | P2-13, P2-14 | todo | | |
| P2-02 | The `scene` module (`openrac_scene`) from `viewer/`, with forwarding headers left in `viewer/` until P2-37 | P2-14 | todo | | |
| P2-03 | `host::GameProfile` for rac1: the game state moves into `game/rac1/`, read through the profile | P2-02, P2-18, P2-27, the level loader, the rac1 sync | todo | | |
| P2-04 | Split `frontend.cpp` 1/5: shared state into an internal header; window and desktop integration | P2-01, P2-09 | todo | | |
| P2-05 | Split `main.cpp` into `program/`: options, vsync, levels, sound; the two old warnings fixed | P2-25 | todo | | |
| P2-06 | Raw GL confined to `renderer/`: `renderer/ui/` | P2-37 | todo | | |
| P2-07 | `renderer` stops including `enhanced/`: a post-process hook, GTAO registers with it | P2-06 | todo | | |
| P2-08 | `common/result.h` (`openrac::Result<T>`) and the C-boundary wrapper | | done | maintainer | [4101ec2d](https://github.com/OpenRAC/OpenRAC/commit/4101ec2d), [c22502bc](https://github.com/OpenRAC/OpenRAC/commit/c22502bc), and this page |
| P2-09 | `frontend.cpp`'s getenv reads onto `common/env.h`; `OPENRAC_DEVELOPER` as decided | | done | maintainer | [2526426f](https://github.com/OpenRAC/OpenRAC/commit/2526426f), [9439dd11](https://github.com/OpenRAC/OpenRAC/commit/9439dd11), [8ac32546](https://github.com/OpenRAC/OpenRAC/commit/8ac32546), and this page |
| P2-10 | `port/tools` and the probe tools use `openrac_common`, through one bootstrap; versionmap's `mips` clash | | todo | | |
| P2-11 | `editor/catalogue.py` moves to `sdk/tools/` | P2-32 | todo | | |
| P2-12 | Launcher TypeScript types generated with ts-rs (proposal, question 5); `mock.ts` typed on them | P2-34 | todo | | |
| P2-13 | Source folders for the host program's split (`program/`, `frontend/`, `debug/` globbed) | | done | maintainer | [293f4f43](https://github.com/OpenRAC/OpenRAC/commit/293f4f43), and this page |
| P2-14 | The test stanzas of `Window.cmake`, `ProbeBake.cmake`, `Assets.cmake` onto the helpers | | done | maintainer | [df05bed6](https://github.com/OpenRAC/OpenRAC/commit/df05bed6), [4f37b97e](https://github.com/OpenRAC/OpenRAC/commit/4f37b97e), [bcebacaf](https://github.com/OpenRAC/OpenRAC/commit/bcebacaf), and this page |
| P2-15 | Split `frontend.cpp` 2/5: input | P2-04 | todo | | |
| P2-16 | Split `frontend.cpp` 3/5: world submission | P2-15 | todo | | |
| P2-17 | Split `frontend.cpp` 4/5: interpolation | P2-16 | todo | | |
| P2-18 | Split `frontend.cpp` 5/5: `frame()` in named steps | P2-17 | todo | | |
| P2-19 | A C entry point to the `OPENRAC_*` table (new contract); `game/common/lib` reads through it | P2-08 | done | maintainer | [800ab1b6](https://github.com/OpenRAC/OpenRAC/commit/800ab1b6), [87cd74ff](https://github.com/OpenRAC/OpenRAC/commit/87cd74ff), [3f97186f](https://github.com/OpenRAC/OpenRAC/commit/3f97186f), and this page |
| P2-20 | getenv in `renderer/vulkan`, Android, the probe baker and `mod_scripts.cpp` onto `common/env.h` | | todo | | |
| P2-21 | `stats::probe` and `log.cpp` read through the env table | P2-19 | todo | | |
| P2-22 | getenv in `main.cpp`, `debug_overlay.cpp` and `debug_menu.cpp` onto `common/env.h`; `OPENRAC_DEVELOPER` as decided | | done | maintainer | [8d6ef873](https://github.com/OpenRAC/OpenRAC/commit/8d6ef873), [1011d3d3](https://github.com/OpenRAC/OpenRAC/commit/1011d3d3), [31769667](https://github.com/OpenRAC/OpenRAC/commit/31769667), [e2921877](https://github.com/OpenRAC/OpenRAC/commit/e2921877), and this page |
| P2-23 | The C-boundary wrapper on the entry points of mods, scripts, fixes and the window | P2-08, P2-20 | todo | | |
| P2-24 | The getenv rule in `check_conventions.py`; CODING_CONVENTIONS.md brought up to date | P2-19, P2-21, P2-22 | todo | | |
| P2-25 | `program/game_host.cpp`: the host side of `game_host.h`, moved out of `main.cpp` and wrapped | P2-08, P2-13, P2-22 | todo | | |
| P2-26 | Split `debug_overlay.cpp` 1/2: internal header and inspection panels | P2-13, P2-22 | todo | | |
| P2-27 | Split `debug_overlay.cpp` 2/2: drawer, menu bar, controls | P2-26 | todo | | |
| P2-28 | CI: the touched-line format check fails; the layering check runs | | in progress | lane 3 | 46050a4d: the layering check runs in CI ("Port layering"). `--strict-touched` waits for the maintainers (see Follow-ups): master's recent merges fail it |
| P2-29 | Launcher desktop entries take their titles from game.json | | done | maintainer | [82f85fb8](https://github.com/OpenRAC/OpenRAC/commit/82f85fb8), [f9decb76](https://github.com/OpenRAC/OpenRAC/commit/f9decb76), and this page |
| P2-30 | Sister checkouts from game.json, with the env override | | done | maintainer | [2360dec8](https://github.com/OpenRAC/OpenRAC/commit/2360dec8), [0506f468](https://github.com/OpenRAC/OpenRAC/commit/0506f468), and this page |
| P2-31 | Intel Macs ship without the probe baker | | done | maintainer | [94c6a682](https://github.com/OpenRAC/OpenRAC/commit/94c6a682), [c9d14c2b](https://github.com/OpenRAC/OpenRAC/commit/c9d14c2b), [eac32239](https://github.com/OpenRAC/OpenRAC/commit/eac32239), [c0380ec7](https://github.com/OpenRAC/OpenRAC/commit/c0380ec7), and this page |
| P2-32 | The editor uses `openrac_common`; `scene_nodes` renamed; one `unit()` | P2-10 | todo | | |
| P2-33 | Launcher: Discord glue moves into the core | | todo | | |
| P2-34 | Launcher: first-start install move into the core; `run()` split | P2-33 | todo | | |
| P2-35 | hostgen `main` and no_c `classify` in named steps | P2-10 | todo | | |
| P2-36 | The frontend's rac1 addresses and game-id branches move into the profile | P2-03 | todo | | |
| P2-37 | `game/common` includes `scene/` directly; `openrac::scene`; forwarders removed | P2-02, P2-05, P2-18, P2-27 | todo | | |
| P2-38 | rac1 host C reads through the C entry point | P2-19, the level loader, the rac1 sync | todo | | |
| P2-39 | `scene` stops including `enhanced/`: lighting and shadow hooks | P2-07 | todo | | |

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
| P0-01 | `check_conventions.py` gains a `getenv` rule when `common/env.h` lands. | not in P1-03 (`port/tools/conventions/` is outside its paths): a rule on new C++ files can land now, the one on every file once P2-09 has moved the last reads |
| P0-02 | CI does not run the layering check. A step beside "Port conventions" in checks.yml running `layering.py` and its tests would hold every push to it. | a later CI package |
| P0-02 | A package that moves or removes an include listed in `port/tools/layering/rules.tsv` removes or updates its row in the same commit series (the check warns until it does): P1-01 (json, ps2_float), P2-02 (the scene files), P2-03 (the game state), P2-07 (GTAO). | P1-01, P2-02, P2-03, P2-07 |
| P0-02 | `viewer/level.h`, `level_renderer.h` and `level_renderer.cpp` include `enhanced/` (6 includes), allowed while they sit in a program's directory. Moved to `scene/`, they become scene -> enhanced: list them in rules.tsv as exceptions until P2-07 inverts them. | P2-02 |
| P0-02 | Moving the game state into `game/rac1` must not leave `game/common` including `game/rac1` (a higher layer): the host reads it through `host::GameProfile` only. | P2-03 |
| P0-02 | `platform/window.cpp` opens the Vulkan backend (`renderer/vulkan/vkgl.h`), platform -> renderer. No package removes it: the renderer could open its backend on the window platform hands it. | the plan |
| P0-02 | The check reads `#include "<dir>/..."` only: an include without a directory found through a target's own include directories, and the CMake link graph (section 19), are checked by review. | a later package |
| P0-04 | The `headless` preset does not build on master: `port/game/common/main.cpp` calls `openrac_sound_stop_readers` in `std::atexit` outside `#ifdef OPENRAC_FRONTEND`, the only place `openrac/sound_engine.h` is included. CI builds no headless preset, so nothing catches it. The atexit belongs inside the `#ifdef`, and a CI job could build the preset. | done in P0-06 ([a3c8f3c0](https://github.com/OpenRAC/OpenRAC/commit/a3c8f3c0), [d09d4c01](https://github.com/OpenRAC/OpenRAC/commit/d09d4c01)): the header is included for both builds instead, since a headless run starts track readers too; checks.yml builds and tests the preset |
| P0-05 | `tools/bundle_standalone.py` copies only `openrac.py` and `extractor.py` into the launcher's bundle. When they move onto `openrac_common`, the bundle carries `tools/openrac_common/` too (and its test checks it); `paths.repo_root` already accepts the bundle's `STANDALONE` marker. | done in P1-05 ([e82e5fe4](https://github.com/OpenRAC/OpenRAC/commit/e82e5fe4)): the test also runs the copy's tools from the copy |
| P0-05 | A directory record cut short at the end of a directory raises `struct.error` in `iso9660`, as it does in the extractor's `walk` today, so a damaged image crashes the extractor instead of giving error 4020. Turning it into an `IsoError` changes what a player sees: a commit of its own after the extractor has moved. | done in P1-05 ([89503beb](https://github.com/OpenRAC/OpenRAC/commit/89503beb)) |
| P0-05 | `tools/fcheck_gate.py:171,374` default `OPENRAC_ROOT` to `~/Projects/RAC/OpenRAC` and `tools/fcheck_level_build.py:44` takes `Path.cwd()` as the root. `paths.repo_root()` is the candidate, but the default names where the port is built, which a worktree may not be: decide it when they move. | done in P1-05 ([d15bb423](https://github.com/OpenRAC/OpenRAC/commit/d15bb423)): `OPENRAC_ROOT` defaults to the gate's own checkout (in a worktree, the worktree's build; the variable still overrides it). `fcheck_level_build.py`'s `Path.cwd()` is rac1-decomp's root, not OpenRAC's (the gate runs it copied alone in rac1-decomp's container), so it stays, renamed `DECOMP` with the reason |
| P0-05 | `editor/disc.py` and `editor/wads.py` keep stricter checks than `iso9660` (root directory only, 2048-byte logical blocks, no subdirectory or multi-extent file) and a `BOOT2` regex that needs `cdrom0:\`. Moving the editor onto `iso9660` keeps those checks on the editor's side. | P2-10 (P1-06 left the editor's readers as they are: the editor does not import `openrac_common` yet) |
| P0-05 | `tools/test_openrac_common.py` imports the existing copies from `port/tools/hostgen`, `port/tools/versionmap`, `sdk/tools`, `editor` and `port/enhanced/tools/probes` to compare against them. A package that deletes a copy deletes its comparison in the same commit. | done for `tools/` in P1-05; P1-06 deleted no copy it compares against (it compares `editor/disc.py`'s reader, which stays); P2-10 |
| P1-02 | `vkgl_gl.cpp` is 1,667 lines: the GL functions and the table that names them. Splitting it further needs its entry functions declared in `vkgl_internal.h` for the table; it fits the state's move into objects. | P3-03 |
| P1-02 | The backend's source files keep the `using namespace openrac::gl` that `vkgl.cpp` had, which section 9.6 allows only for the module's own namespace (the header uses using-declarations). | P3-03 |
| P1-02 | A pure move kept the bodies as they were: the header's types (`Context`, `State`, `Frame`, `Program`, ...) have no member comments (7.5), the files' own functions no doc comments (7.2), and clang-tidy has not been run on the moved code. | P3-03 |
| P1-02 | Section 9.1 of the conventions names the module's target `openrac_renderer_vulkan`; CMake's is `openrac_vkgl`. One of the two changes. | done in P0-06 ([05f5b41c](https://github.com/OpenRAC/OpenRAC/commit/05f5b41c)): the target was renamed |
| P1-02 | Small helpers the draw path calls (`ring`, `barrier`, `end_rendering`, `resolve`, ...) are now in other translation units, which a build without LTO does not inline across (`fail`, `check` and `frame` are inline in the header; `standalone` builds with LTO). Not measured: the frame-time comparison is the maintainers' regression run. | the maintainers |
| P1-02 | DEBUGGING.md does not list `OPENRAC_GPU_DRIVEN_OFF` and `OPENRAC_GPU_MATERIALS_OFF` (RAC1_GPU_DRIVEN.md does). | done in P1-03 ([037586a0](https://github.com/OpenRAC/OpenRAC/commit/037586a0)), with the 28 others the page lacked |
| P1-02 | Tests run in parallel (`ctest -j8`) can leave the shared Vulkan pipeline cache (`vulkan_pipelines_*.bin` in the user's cache folder) bad: every process saves through the same `<file>.tmp` before renaming it. `vulkan_uniform` then fails (wrong colours), in that run and in later runs that load the file, until it is replaced; with a fresh cache or `OPENRAC_VULKAN_PIPELINE_CACHE=0` it never fails. Measured the same before and after P1-02 (4 of 8 parallel rounds each, none of 12 with the cache off). A temporary file per process, or the tests running with a cache of their own, would fix it: a change of behaviour, not a refactor's. | fixed in P0-06 ([50702a3e](https://github.com/OpenRAC/OpenRAC/commit/50702a3e)): a temporary file per save, renamed into place, and a damaged file dropped on reading |
| P0-06 | The pipeline cache changed behaviour on purpose: each save goes through a temporary file of its own and a renamed file, and the file carries an envelope (length, FNV-1a hash), so a damaged one is dropped instead of reaching the driver. Caches saved before it are dropped once. | the record |
| P0-06 | A save killed between writing and renaming leaves its `<cache>.<hex>.tmp` beside the cache, and nothing removes it. `remove_other_pipeline_caches` could remove such files once they are old, without touching a save in flight. | P3-03 |
| P0-06 | On Windows a rename over a file another program holds open is refused, so a save made while another program reads the cache is lost (logged; the next program saves it). Not measured: CI's Windows job builds and runs only the runtime's tests, so `pipeline_cache_file` has not run on Windows. | P3-03, a later CI package |
| P0-06 | `game/common/lib/sound_engine.cpp` joins its track readers by hand (`reader.thread.join()`); `common/joining_thread.h` gives them an owner that joins. | P3-01 |
| P0-06 | Embree's static library takes one ISA with AppleClang, so an Intel Mac (SSE2, AVX, AVX2) still cannot build the probe baker through `build_port.py`. The launcher builds its macOS game on Apple silicon only, so CI does not see it. | the maintainers |
| P0-06 | `port/game/common/main.cpp:777` and `:784` warn (`-Wsign-compare`, `-Wshadow`) in every game's build; they predate the restructure. | P2-05 |
| P0-06 | Launcher tests that plan Play read the user's data folder through `saves::data_home()`; `plans_fill_placeholders_and_report_blockers` now runs in an empty one (`saves::with_data_home`), and any other test that reads the folder should too. | done in P1-07 ([5401de6e](https://github.com/OpenRAC/OpenRAC/commit/5401de6e)): one more test read it (`a_restore_from_outside_the_card_is_refused`), found by making `data_home()` panic in tests outside `with_data_home`; every core test now stays out of the user's folder |
| P0-03 | The test stanzas outside `port/CMakeLists.txt` are still written out: about 40 in `cmake/Window.cmake` (several run one program under more than one name: register the first through the helper, add the others with `add_test` on its target), 2 in `cmake/ProbeBake.cmake`, `sound_engine` in `cmake/Games.cmake`. Each moves to the helpers when its file is restructured (conventions, section 24.1). | P2-02 and P2-06 (`Window.cmake`), P3-01 (`sound_engine`), the rest when touched |
| P0-03 | `openrac_add_module_tests` (`cmake/Assets.cmake`) makes the same `<dir>_<name>_test` / `<dir>_<name>` pair as `openrac_add_test`, plus SKIP_RETURN_CODE 77; its loop body could call the helper, leaving one way to register a test. | the next package in `cmake/Assets.cmake` |
| P0-03 | `mod_loader` and `mod_scripts` stay written out: their mods are MODULE libraries passed in through `$<TARGET_FILE>` definitions. A second test of that shape would justify a helper for it. | the record |
| P1-04 | No ctest pins `wad_routine.c` and `vu0_sine.c`: a C test on synthetic game memory (as `graph_dma_test` does for `graph.c`) needs a line in `port/CMakeLists.txt`, outside the package. Their equivalence with the old host files was shown by a scratch driver (the commits say what it ran). | P3-01 (characterisation tests for `game/common/lib`) |
| P1-05 | `tools/fcheck_gate.py` still defaults `OPENRAC_DECOMP` to `~/Projects/RAC/rac1-decomp`, and `sources.py`/`openrac.py` take a sister checkout from `game.json`'s `checkout` (or `OPENRAC_SOURCE_*`). One rule for where sister checkouts are (beside the checkout, or a setting) would serve both; it changes where tools look, so it is not a refactor. | the maintainers |
| P1-05 | `tools/versions.py`'s `source_dir` keeps its own default for a version's folder (it reads xmap's entry, which carries no game id); `games.version_dir` could take a game id and a version entry instead of the manifest. | P2-10 |
| P1-05 | `openrac_common/README.md` gives scripts outside `tools/` an import line that finds a checkout by `AGENTS.md`; the launcher's bundled copy carries `editor/` and has no `AGENTS.md`, so the editor's move onto the package needs a marker the copy has too (for example, walking up to the folder holding `tools/openrac_common`). | P2-10 |
| P1-05 | The Mac's `/usr/bin/python3` is 3.9 and fails the tools' tests on master already (the tools need 3.10); the packages' verify lines say `python3`, so run them with `/opt/homebrew/bin/python3` or a 3.10+ on the PATH. A version check at the top of `openrac.py` and `extractor.py` would turn the players' traceback into a sentence. | the plan, a later tools package |
| P1-05 | `tools/android_keystore.py` and `updater_manifest.py` are imported as `tools.X` by their tests, so they cannot import `openrac_common` as the others do; they need nothing from it today. | none, noted |
| P1-06 | The editor keeps its shared helpers in `editor/util.py` (`sha256`, `read_json`, `f32`, `cross`, `manifest`, `disc_identity`) rather than importing `tools/openrac_common`: the launcher's bundled copy runs the editor where the package's import line finds no checkout. `digest.sha256` and `games.load` serve the same purpose; move the editor onto them with the bootstrap P2-10 settles, and update `openrac_common/digest.py`'s docstring, which still names `editor/assets.py:135` and `overrides.py:396` as copies. | P2-10 |
| P1-06 | `editor/cparse.py`'s `parse_type` raises `ParseError`, which the module never defines: a declaration without a type ends in a `NameError`. Defining `class ParseError(ValueError)` changes the error a caller sees, so it is a fix of its own (`test_cparse.py` accepts any exception today). Its nested anonymous unions also stay in `structs` under their placeholder name (`<anon@N>`, the pop after the body uses the advanced index); the catalogue's output would show whether anything depends on it. | an editor fix |
| P1-06 | Three different functions are named `scene_nodes` (`collision_scene.py`, `moby_collision_scene.py`: writing nodes into a scene; `pack.py`: reading them back), and `unit` is written twice (`moby_collision_scene.py`, `sky.py`: the same result for 3-vectors, written differently). Not unified: the brief asked for identical copies. Clearer names, and one `unit` in `util.py`, would be a small editor package. | P2-10 |
| P1-06 | The editor's Python files carry no licence line, the editor being under its own MIT `LICENSE` (copyright holder named there); the conventions page's "licence lines" for Python name `port/tools/`'s GPL header. New editor files follow the editor (none). Which line, if any, editor files get is the maintainers' call. | the maintainers |
| P1-04 | rac1's WAD decompressor (`rac1/host/hand.c`) makes the same checks in its own form and traces other fields; it could call `openrac_wad_routine` once rac1's host is restructured. `OPENRAC_TRACE_WAD` is read in both places. | P2-03, P2-09 |
| P1-04 | The rest of rac2's and rac4's host C still repeats itself: `sound.c` (0.76 similar; `rac4_io_test` compiles rac4's), `entry.c`, `movie.c`, `dma.c`, and rac2's `print.c` against `rac1/host/hand.c`. | P3-02 |
| P1-04 | Going Commando's sine reads -pi from the pair at `0xC98`; Deadlocked's negates pi. If that pair holds exactly -pi, the two are the same and the table's `minus_pi` field can go. Not checked: it needs the game's packet. | the maintainers |
| P1-07 | The launcher's and the games' Linux desktop entries are written twice: by the launcher (`launcher/core/src/desktop.rs`) and by the port itself (`port/game/common/frontend.cpp`, the window and desktop integration). One writer, or one shared description of the entries, would keep them from disagreeing. | P2-04 |
| P1-07 | `desktop::game_title` hard-codes the games' menu names, against ARCHITECTURE.md rule 1 (the launcher hard-codes nothing about a game but its colours); the catalogue's titles (`game.json`) could name them. It changes the entries' text, so it is not a refactor. | done in P2-29 ([82f85fb8](https://github.com/OpenRAC/OpenRAC/commit/82f85fb8)): the entries take the port's name, the game's title and its disc's video standard from game.json |
| P1-07 | `launcher/README.md`'s "Linux desktop" row lists the launcher's two `.desktop` files only; the launcher also writes two entries and an icon per game of the checkout (`desktop::write_games`). Outside this package's paths. | done in P2-29 ([f9decb76](https://github.com/OpenRAC/OpenRAC/commit/f9decb76)) |
| P1-07 | `run_action` still holds the Discord glue (the playing status, the planet and bolts from the play job's log, idle when it ends) and `run` the install folder's first-start move (`active/` renamed into `~/OpenRAC`); both are logic for the core, with tests, when `run` is split. | P2-12 |
| P1-08 | The three `openFolder`s are not copies: Saves opens the card's folder (`api.openSavesFolder`) and toasts the raw error, "Could not open folder: …"; the mods panel opens a mod's folder (`api.openOutput`) and Mods a project folder (`api.openGameFolder` or `api.openPath`), both through `guardShown`, which hides paths. Moving Saves onto `guardShown` would keep paths out of its toast too, but changes its text, so it is not a refactor. | the launcher |
| P1-08 | `lib/reports.ts`'s `duration` is not the job lists' duration: it writes a play session in words ("1 hour 14 minutes"), from seconds. Kept apart; a name such as `playTime` would tell it from `jobDuration`. | the launcher |
| P1-08 | `lib/api.ts` has 17 exported types no other module names, and `lib/app.svelte.ts` exports `syncProgressWeb` for itself only; left to P2-12, which generates the types. Ten other types used only in their own module stay exported because they name a parameter or result of an exported function (`LevelName`, `PlayerState`, `Theme`, ...). | P2-12 |
| P1-08 | Still open from the audit's launcher section: about 46 `rgb()`/`rgba()` literals in 12 components (Header has 17: shades and shadows on the bar) that STYLE.md's "use the tokens" also covers, which `stylesheets.test.ts` could then check; 16 exports used only by tests (`plugins.ts`'s `settingNumber` overlaps `settingValues.ts`'s `parseTyped`); the per-panel `load`/`remove` written in four panels. | a later launcher package |
| P1-08 | `cargo test -p openrac-launcher-core` fails one test on the Mac when `python3` on the PATH is `/usr/bin/python3` (3.9): `an_installed_launchers_copy_offers_the_editor` runs `tools/bundle_standalone.py`, which needs 3.10 (P1-05's note above). With `/opt/homebrew/bin` first on the PATH all 204 pass. The test could skip, as it does when there is no Python, on a Python older than the tools need. | the launcher |
| P1-09 | The three checks of a mod's manifest disagree on 43 of the 120 shared manifests and on one of the 16 ids ([sdk/tests/manifests/README.md](../../sdk/tests/manifests/README.md) outlines them; each case says why). The SDK is the strictest about form, the launcher takes leading zeros and does not check the ids in `requires` and `conflicts`, the port reads every number as a double, and Python's `$` and `\d` let a final newline and non-ASCII digits through. Whether to bring the three together, and to which rule, changes behaviour: not settled by the package. | the maintainers |
| P1-09 | MODS.md, section 3, says the launcher and the port apply its rules, but not that the three checks are held to `sdk/tests/manifests/cases.json`; a sentence there would send someone changing a rule to the file. Outside this package's paths. | the next mods docs change |
| P1-09 | `port/tests/mod_manifest_test.cpp` now includes `viewer/json.h` (to read the shared cases), beside `mod_manifest.h`'s own include of it; P1-01's move of `json` to `common/` changes that include line too. | done in P1-01 ([0caebafa](https://github.com/OpenRAC/OpenRAC/commit/0caebafa)): it includes `common/json.h`, and its alias `namespace json = viewer::json` went |
| P1-01 | `openrac_json` folded into `openrac_common` (the reader needs only the standard library, so the link graph stays acyclic and the module is one target, section 19). `openrac_ps2float` now links `openrac_runtime` alone, yet its block still sits in `port/CMakeLists.txt`'s assets section, where it was for `openrac_assets_core`; moving it under the runtime's would reorder `ctest -N`, so it was left for a package that restructures that file. | the next `port/CMakeLists.txt` package (P1-03 left it: its check allows one new test in `ctest -N` and no other change of order) |
| P1-01 | `renderer/world/vu_float.h` no longer includes `assets/`, but `openrac_renderer_world` still links `openrac_assets_core`: `renderer/world/profile.h` includes `assets/version.h` (renderer -> assets is allowed by the layers). | none, noted |
| P1-01 | `common/ps2_float.h` declares `u32`, `u64`, `s32`, `s64` and `f32` in `openrac::ps2`, as `assets/bytes.h` declares them in `openrac::assets`, since `common/` cannot include `assets/`. If a second `common/` header needs them, one `common/` header of fixed-width aliases, which `bytes.h` then uses, would replace both. | the next package that needs them in `common/` |
| P1-01 | `json.cpp`'s `Parser` class and `ps2_float.cpp`'s helpers have no doc comments (section 7.2); the move documented the public interfaces only. | when the files are next changed |
| P1-01 | The 15 OpenGL tests of the release preset (`viewer`, `renderer_direct`, `renderer_shader`, `imgui_gl`, `touch_overlay`, `world_gl_smoke`, their `_es` forms, `enhanced_rendering_compat`) skip in an agent's session on the Mac: there is no GL context (the Vulkan ones run). `viewer` holds the JSON reader's only direct checks, so in such a session the reader is tested through the mods' tests alone. The JSON checks as a `json` test of `common/` would run everywhere. | a later tests package |
| P1-01 | `check_conventions.py --touched` skips the changed-lines check when `git-clang-format` is not on the PATH (Xcode does not ship it); the package checked each changed file with `xcrun clang-format` instead. | a later tools package |
| P1-03 | The table holds all 113 variables the port reads: 72 read with `getenv` by name, 12 through `android/gpu_driver.cpp`'s and `enhanced/viewer/probe_lighting.cpp`'s own `getenv` helpers, and 31 `stats::probe` switches (two in more than one group). The audit's count of 72 was the first group alone. | the record |
| P1-03 | `OPENRAC_DEVELOPER` is read two ways: `debug_menu.cpp` (and DEBUGGING.md) take any value but the empty string and `0`; `frontend.cpp` and `debug_overlay.cpp` take a first character other than `0`, so `00` or `01` is off there. The table's row follows `debug_menu.cpp` and the page (`NotEmptyOrZero`); moving the other two onto it turns such values on for them, a change of behaviour to decide or to note. | P2-09 |
| P1-03 | `env::integer` reads as `strtoll`. The reads P2-09 moves use `atoi`, `atol`, `atoll` and `strtoull` (`OPENRAC_DUMP_RAM`, `OPENRAC_FX_DUMP`, `OPENRAC_SHOT_FROM` compared with an unsigned frame number): they agree on every value in `int`'s range, but `strtoull` wraps a negative value to a huge one and `atoi` past `int` is undefined, so each comparison needs checking as `OPENRAC_DUMP_TEXTURE`'s was (its commit says how). | P2-09 |
| P1-03 | Nine variables are read by C (`game/common/lib/*.c`, `game/rac1/host/*.c`: `OPENRAC_TRACE_CD`, `_TRACE_SIF`, `_TRACE_WAD`, `_SOUND_LOG`, `_COLL_TRACE`, `_HIT_LOG`, `_UI_LOG`, `_UI_SKIP`, `_DIRECT`), which cannot include `env.h` (section 14.3). Reading them through the table needs a C entry point in a contract (a change of the contract, not a refactor) or they stay with `getenv`; the conventions page's "`getenv` only in `common/env.cpp`" would then name the exception. | P2-09, the maintainers |
| P1-03 | `stats::probe` (`common/stats.cpp`) still calls `getenv` for its 31 switches and carries a description of its own beside the table's. It could take an `env::Var` and the table's description, reading the first value through `env::flag`; the overlay's switching stays the probe's. | P2-09 |
| P1-03 | CODING_CONVENTIONS.md section 1.3 lists `common/env.h` as arriving with P1-03, and sections 3.14 and 16 say "once it lands": true for `runtime`, `platform` and `renderer`, not yet for the rest. The page changes through a pull request a maintainer approves, so P1-03 left it. | the maintainers |
| P1-03 | DEBUGGING.md's "Read in" column names where a variable takes effect, which env.h does not record; the test checks names only. Kept that way: the table says what a variable is, the page where to look. | none, noted |
| P1-03 | `platform/pump.cpp` read `OPENRAC_NO_PUMP_FIBER` with `SDL_getenv`, now the C library's `getenv` through `env.h`. On Windows SDL3 starts its environment from the process's, so they agree unless something sets the variable while the program runs (nothing does). Windows-only: CI's Windows job compiles it, no test runs it. | the record |
| PR #42 | `port/game/rac1-pal/host/item_pose.c` came in without the comments section 6.2 asks for on its branches, loops and early returns, without 6.5's tags on its numbers (the class table's 0x25 records of 0x4C bytes, the `copy_*` offsets), and with doc comments that name neither parameters nor results (7.2). When the class is not in the table, its lookup leaves `rec` one record past the end; whether retail does the same is not checked. | its next change |
| PR #40 | `rac1-pal/host/crt0.c` and `tlbtrap.c` describe their one function in the file comment only; section 7.2 wants a doc comment on the function. No ctest covers the new kernel answers, `spr_dma.c` or crt0's .bss loop. A C test on synthetic game memory, as `graph_dma_test` does for `graph.c`, would pin them. | rac1's port tests |
| PR #41 | hostgen translates an empty asm statement's output only when a bare digit constraint ties it to an input. A `+` output, a `%[name]` tie or a constraint such as `"0r"` is reported and stays a barrier, and none of them has a test. | hostgen |
| PR #37 | `vu_times` (rac1's `game_fastfunc.c`, PAL and NTSC-U) gives zero for a zero exponent as the vector unit does. An overflow or an exponent-255 operand still gives the host's inf or NaN where the console saturates. | the next ps2float change |
| P2-13 | Section 19 of the conventions asks for no new globs; `cmake/Games.cmake` now globs `game/common/program/`, `frontend/` and `debug/` into the game programs, as the plan's phase 2 asks so that the splits never edit it. They are a program's source folders, not a module's library target; the page could say so beside the library replacements' glob, or each folder gets a target with listed sources once it is a module. | P2-24 |
| P2-13 | A rebuild after reconfiguring never prints `ninja: no work to do`: `openrac_build_info` runs at every build, and its commit gains `-dirty` once `port/` has changes, which relinks every program. The proof that a CMake change keeps the build is `ninja -C build/<preset> -t commands` before and after, which must be identical. | the briefs of CMake packages |
| P2-13 | `mod_loader` was killed (`Subprocess killed`, 0.39 s) once in a full `ctest --preset release` run, and passed alone three times and in a second full run; seen once, not investigated. | the maintainers |
| P2-08 | The wrapper is `c_boundary::call(where, failure, body, reporter)` in `common/c_boundary.h` (a void overload has no `failure`); the reporter defaults to the log. The game side passes `openrac_game_crash`, which does not return, so the failure value is reached on the host side only. Wrapping `openrac_game_crash` itself, P2-25 keeps the log as its reporter, so a throw inside it does not recurse. | P2-23, P2-25 |
| P2-08 | CODING_CONVENTIONS.md still says Result and the wrapper are arriving: section 1.3's two rows, 13.3's "arriving with P2-08" and "until it lands", and 13.4's written-out `try` (now `c_boundary::call`). Section 14.3 says P2-08 gathers `game_host.cpp`; that is P2-25. Section 2's clang-tidy line needs `--extra-arg=-isysroot$(xcrun --show-sdk-path)` with Homebrew's LLVM on macOS, or it finds no standard header. | P2-24 |
| P2-08 | `port/README.md`'s "What is here" row for `common/` names `joining_thread.h` among the small helpers and not `result.h` or `c_boundary.h`; the file is outside P2-08's paths. | P2-24 |
| P2-08 | clang-tidy reports `bugprone-implicit-widening-of-multiplication-result` at `common/log.h:63`, a finding of the existing code. | the next change to `log.h` |
| P2-09 | `frontend.cpp` still reads the frame numbers `OPENRAC_DUMP_RAM`, `OPENRAC_FX_DUMP` and `OPENRAC_SHOT_FROM` as `strtoull` (its `env_frame`), not through `env::integer`: the two differ only for a negative value past -2^63, which `strtoull` wraps to a frame number below 2^63 and `strtoll` holds to -2^63 (never). Moving them onto `env::integer` is that change of behaviour, to decide. | the maintainers |
| P2-09 | `OPENRAC_FRAME_COST` and `OPENRAC_MOBY_LOG`'s period were read with `atoi`, undefined past `int`'s range (Apple's libc and glibc truncate, MSVC clamps); they are now held to `INT_MAX` on every platform ([2526426f](https://github.com/OpenRAC/OpenRAC/commit/2526426f)). | the record |
| P2-09 | `probe_lighting.cpp` parses `OPENRAC_PROBE_RAYS` (`Kind::Integer`) and `OPENRAC_PROBE_SKY_INTENSITY` (`Kind::Number`) strictly (`from_chars`, `stof`: the whole value or an error) from `env::value`, since `env::integer` and `env::number` accept a partial number. A strict typed read in `env.h` would serve both. | the next change to `common/env.h` |
| P2-09 | `port/tools/layering/rules.tsv` rows 96 to 101 name `frontend.cpp`'s `viewer/` includes at lines 50 to 55; P2-09's four new includes moved them to 54 to 59, so `layering.py` warns (it passes, 0 to fix). The file is outside P2-09's paths: each row's line goes up by 4. | the next package that edits `rules.tsv` (P2-02, P2-04) |
| P2-09 | `debug_overlay.cpp:290` still reads `OPENRAC_DEVELOPER` with `getenv` and takes a first character other than `0`; under the maintainer's decision it becomes `env::flag(env::Var::Developer)`, a `fix` commit of its own as [8ac32546](https://github.com/OpenRAC/OpenRAC/commit/8ac32546) was. | P2-22 |
| P2-09 | `port/common/README.md`'s Configuration paragraph says `game/`, `renderer/vulkan/`, `android/` and `enhanced/` still call `getenv` ("P2-09"): `frontend.cpp` and `enhanced/viewer/probe_lighting.cpp` now read through `env.h`, and what is left is P2-20's and P2-22's. Outside P2-09's paths. | P2-20, P2-22 (the last to land) |
| P2-09 | The `mod_loader` failure P2-13 saw recurred once here (passed on rerun and in two full runs): `tests/mod_loader_test.cpp:314` names its temporary folder from an unseeded `std::rand()`, the same name in every run, so two `ctest` runs at once (the parallel lanes' checkouts) share and delete it. | the maintainers |
| P2-30 | Every `source.checkout` in `games/*/game.json` names `~/Projects/<project>`, while the maintainer's checkouts are in `~/Projects/RAC/` (AGENTS.md: the sibling `../rac1-decomp`). Since [0506f468](https://github.com/OpenRAC/OpenRAC/commit/0506f468), `fcheck_gate.py` on that machine needs `OPENRAC_DECOMP` or `OPENRAC_SOURCE_RAC1_PAL` set, as `sources.py` and `openrac.py progress --fetch` already needed `OPENRAC_SOURCE_*`. The manifests are under `games/`: changing the paths (or making them relative to the checkout, as AGENTS.md's sibling is) is the maintainer's call. P1-05's follow-up on one rule for sister checkouts is done by P2-30. | the maintainer |
| P2-30 | `docs/workflow/BACKFILL.md:230` still gives `OPENRAC_DECOMP`'s default as `~/Projects/RAC/rac1-decomp`; it is now `OPENRAC_SOURCE_RAC1_PAL`, else `games/rac1/game.json`'s checkout (`fcheck_gate.py`'s docstring and `tools/openrac_common/README.md` say so). Outside P2-30's paths. `grep 'Projects/'` finds no other sister path written into `tools/`, `port/tools/` or `sdk/tools/`. | the next docs package |
| P2-22 | `port/tools/layering/rules.tsv` rows 94, 102 and 103 name `debug_overlay.cpp:42` and `main.cpp:74` and `:75`; P2-22's new includes moved them to 45, 75 and 76, so `layering.py` warns (it passes, 0 to fix). The file is outside P2-22's paths. | the next package that edits `rules.tsv` (P2-02, P2-04) |
| P2-22 | `port/common/README.md`'s Configuration paragraph still says `game/` calls `getenv`: `main.cpp`, `debug_overlay.cpp` and `debug_menu.cpp` now read through `env.h` as `frontend.cpp` does; what `game/` has left is C (`game/common/lib`, `game/rac1/host`, P2-19 and P2-38) and `mod_scripts.cpp` (P2-20). Outside P2-22's paths. | P2-20 (the last to land) |
| P2-22 | `env_string` (a variable's value as an `std::optional<std::string>`, for reads that hand it to `sscanf`, `strtoull` or a C function wanting a NUL) is now written out in three files: `frontend.cpp`, `debug_overlay.cpp` and `main.cpp`. A `text(Var)` in `env.h`, or a promise there that `value`'s view ends in a NUL, would replace all three. | the next change to `common/env.h` |
| P2-22 | `debug_overlay.cpp` reads `OPENRAC_SHOT_FROM` with `strtoull` as `frontend.cpp` does, so P2-09's follow-up on the frame numbers and `env::integer` covers this read as well. | the maintainers |
| P2-22 | `OPENRAC_DEVELOPER` is now read alike by all three of its readers (`frontend.cpp`, `debug_overlay.cpp`, `debug_menu.cpp`: `env::flag`, `NotEmptyOrZero`), which closes P1-03's follow-up on it. DEBUGGING.md's row names only `debug_menu.cpp` as its reader; the other two could be named. A wording change in lane 1's file. | lane 1 (DEBUGGING.md) |
| P2-22 | On the Mac, `check_conventions.py --touched` prints `git-clang-format not found, skipped` for its changed-line layout check: Xcode ships `clang-format` but not `git-clang-format`. P2-22 checked its changed lines with `clang-format --lines` on each hunk instead. The tool could fall back to that, or the page could say to install `git-clang-format`. | P2-28 |
| P2-22 | `tools/cpp_style.py` lists `main.cpp`, `debug_overlay.cpp` and `debug_menu.cpp` as needing formatting, as on master: P2-22 laid out only its changed lines. `main.cpp`'s system includes are in two unsorted blocks, which `clang-format` sorts and merges as soon as a line inside them changes. | P2-05, the layout pass on `game/common` |
| P2-14 | Eight test programs of `cmake/Window.cmake` stay written out, because `openrac_add_test` has no option for them: `viewer_test`, whose tests pass the program a level directory, and the enhanced renderer's `probe_gpu_test`, `early_lighting_test`, `hero_shadow_test`, `sun_shadow_test`, `gtao_test`, `gtao_horizon_test` and `enhanced_rendering_test`, none of whose tests is named after the program (`hero_shadow_cpu`, `gtao_vulkan`, `enhanced_rendering_compat`), and some have no test at all without Vulkan. Two options in `cmake/Tests.cmake` would cover them: `ARGS` for the program's arguments, and `NAME` (or no default test) for a program whose tests are all named after where they run. Tests.cmake is shared with lane 2. | the next package in `cmake/Tests.cmake`, then P2-02 or P2-06 (`Window.cmake`) |
| P2-31 | `port/enhanced/lighting/README.md` says the pinned-Embree baker "is what the launcher's installers carry"; Intel Macs ship without it (docs/port/PORTABILITY.md, section 7). One sentence there would say so. Outside P2-31's paths. | the next package in `port/enhanced/` |
| P2-31 | Under Rosetta, an x86-64 build of Python or of the launcher on Apple silicon counts as an Intel Mac (`platform.machine()`, the launcher's target), so it leaves the baker out and says so. The launcher's macOS builds are Apple-silicon builds, so no shipped copy does this; detecting translation (`sysctl.proc_translated`) would cover a hand-made one. | the maintainers, if it matters |
| P2-31 | `library::tests::an_installed_launchers_copy_offers_the_editor` runs `python3` from PATH to call `tools/bundle_standalone.py`; where that is macOS's Python 3.9 (`/usr/bin/python3`) the script fails on `openrac_common`'s `Path \| str` annotations and the test fails, on master too. The test could skip a Python older than 3.10, as it skips a missing one, or `openrac_common` could add `from __future__ import annotations`. | lane 3 (launcher core or `tools/openrac_common`) |
| P2-19 | The new contract `openrac/host_env.h` is not in CODING_CONVENTIONS.md section 14.1's table. Proposed row, after `ps2float.h`'s: "`host_env.h` \| `port/runtime/include/openrac/` \| the `OPENRAC_*` variables of `common/env.h`'s table, for C (`openrac_env_string`)". Section 14.3's list of contract files gains "`host_env.cpp` for `host_env.h`". Section 16 (and 3.14, checklist 22): "`getenv` appears only in `common/env.cpp`; C++ reads the variables through `common/env.h`, C through `openrac/host_env.h`, whose raw value each C site tests as its old read did. A variable read from C is a row of the table like any other." | P2-24 |
| P2-19 | The four reads of `game/common/lib` agree with their rows of the table: `OPENRAC_TRACE_CD`, `_TRACE_WAD`, `_SOUND_LOG` were tested against NULL (`Truth::Set`), `OPENRAC_TRACE_SIF` as set and not empty (`Truth::NotEmpty`). No disagreement to record. A name the table lacks is logged once as an error and reads as unset (not an abort in a debug build, so debug and release builds behave alike). | the record |
| P2-19 | rac1's host C (`ui2d.c`, `hand.c`, `collision.c`, `boot.c`) still calls `getenv` for `OPENRAC_COLL_TRACE`, `_HIT_LOG`, `_UI_LOG`, `_UI_SKIP`, `_DIRECT`, and in `hand.c` `OPENRAC_SOUND_LOG` and `OPENRAC_TRACE_WAD`: `openrac_env_string` gives the raw value they need (`_UI_SKIP` is text), each keeping its test; check each test against its row then. | P2-38 |
| P2-19 | `port/README.md`'s "What is here" row for `runtime/` could link the new `runtime/README.md` (section 7.7's form). Lane 1's file. | lane 1 (`port/README.md`) |
| P2-19 | `ctest -N`'s raw output differs between a configured-only build tree (each test followed by "Could not find executable" and its paths) and a built one, and names the tree's path, so the brief's literal `diff` of before and after shows noise; P2-19 compared the `Test #N: name` lines without their numbers (release 135 to 136, headless 91 to 92, `host_env` the one added). A package recipe could record `ctest -N` after building, or compare names. | the plan |
| P2-19 | `layering.py` warns that nine rows of `rules.tsv` (`debug_overlay.cpp`, `frontend.cpp`, `main.cpp` includes of `viewer/`) name line numbers those includes have moved from, on master too; the rows need their lines updated. | the owner of `port/tools/layering/rules.tsv` |
| P2-28 | `--strict-touched` is not in checks.yml yet: with it, the pushes of master's recent merges would have failed. Since `ab8c2e94` (PR #42), `port/game/rac1/host/collision.c:36, 82, 114, 144, 547, 625, 927` (from `824276fd`, PR #37); since `8a47b77e` (PR #41), also `port/tests/exe_moby_grid_test.c:87` (`661f3133`); and P1-01's own push would have failed on `port/tests/viewer_test.cpp:207` (`0caebafa`). Measured with clang-format 21.1.8 (PyPI), the version CI pins. Turning it on holds every pull request to it, including ones from contributors who do not run clang-format: decide whether to switch, and whether a formatting commit of those lines comes first (the lines are in files other lanes own). | the maintainers |
| P2-28 | Two docs still say CI does not run the layering check: `port/tools/layering/README.md` ("It is not wired into CMake or CI yet") and the tools table of `port/docs/CODING_CONVENTIONS.md` (the `layering.py` row, "not in CI yet"). Since 46050a4d the checks job runs it ("Port layering"). | lane 2 (layering), lane 1 (the conventions page) |
