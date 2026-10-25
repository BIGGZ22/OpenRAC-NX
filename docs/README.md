# Documentation map

Where to find what. Start with the first table.

## Start here

| Document | For |
|---|---|
| [README.md](../README.md) | What OpenRAC is and how to play |
| [DEVELOPING.md](DEVELOPING.md) | Working from a checkout: the launcher, the port, the level editor, the decompilations |
| [CONTRIBUTING.md](../CONTRIBUTING.md) | Ground rules, commits, pull requests, tests |
| [LAYOUT.md](LAYOUT.md) | How the repository is organised, and where a new file goes |
| [policy/SOURCING.md](policy/SOURCING.md) | What may and may not be used or committed |
| [policy/OPEN_QUESTIONS.md](policy/OPEN_QUESTIONS.md) | Decisions still to be made |
| [TODO.md](TODO.md) | Larger work that is decided and ready to pick up |
| [restructure/PROGRESS.md](restructure/PROGRESS.md) | The restructure of OpenRAC's own code: every package, its owner, how to help |
| [LICENSE.md](../LICENSE.md), [THIRD_PARTY_NOTICES.md](../THIRD_PARTY_NOTICES.md), [CREDITS.md](../CREDITS.md) | Licensing, third-party work, the people behind each project |

## The PC port

| Document | For |
|---|---|
| [docs/port](port/README.md) | The port, OpenGOAL's way: the design ([DESIGN.md](port/DESIGN.md)) and the order of work ([ROADMAP.md](port/ROADMAP.md)) |
| [port/](../port/README.md) | The code: how it fits together, building and testing it |
| [port/docs/CODING_CONVENTIONS.md](../port/docs/CODING_CONVENTIONS.md) | How the port's C++ and C are written, and what CI checks |
| [port/WORKFLOW.md](port/WORKFLOW.md) | From a player's report to a fix and a release |
| [port/BACKFILL.md](port/BACKFILL.md) | Which functions to fill in with C for the port, and the check that says they are right |
| [port/KNOWN_ISSUES.md](port/KNOWN_ISSUES.md) | Bugs that are understood and guarded but not properly fixed yet |
| [port/SETTINGS.md](port/SETTINGS.md), [port/DEBUGGING.md](port/DEBUGGING.md) | The settings file and where it lives; options and variables for debugging |
| [port/RENDERER.md](port/RENDERER.md), [port/VU_PROGRAMS.md](port/VU_PROGRAMS.md) | How the game builds a frame; its vector unit programs |
| [port/MODS.md](port/MODS.md), [port/ANDROID.md](port/ANDROID.md) | The mod SDK; the Android build |
| [port/OPENGOAL_NOTES.md](port/OPENGOAL_NOTES.md) | How OpenGOAL's renderer and platform layer work, and what carries over |

## The launcher and the editor

| Document | For |
|---|---|
| [launcher/README.md](../launcher/README.md) | The launcher: using it, developing it, [connecting a game](../launcher/docs/INTEGRATION.md) |
| [editor/README.md](../editor/README.md) | The Godot level editor: extracting your levels, what it reads, its code |

## The decompilations

Each game version in `games/` documents itself: start with its README.

| Game | Start with |
|---|---|
| Ratchet & Clank, PAL | [games/rac1](../games/rac1/README.md) |
| Ratchet & Clank, NTSC-U | [games/rac1/ntsc](../games/rac1/ntsc/README.md) |
| Going Commando | [games/rac2/ntsc](../games/rac2/ntsc/README.md) |
| Up Your Arsenal | [games/rac3/ntsc](../games/rac3/ntsc/README.md) |
| Deadlocked | [games/rac4/ntsc](../games/rac4/ntsc/README.md) |
| Size Matters | [games/rac5/ntsc](../games/rac5/ntsc/README.md) |
| Secret Agent Clank | [games/rac6/ntsc](../games/rac6/ntsc/README.md) |

Across the games: [games/](../games/README.md) (how the versions relate),
[engine/](engine/README.md) (what is known about the engine),
[SOURCES.md](SOURCES.md) (where each game's code came from) and
[progress/](../progress/README.md) (generated).
