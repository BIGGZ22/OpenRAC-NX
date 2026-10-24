# Documentation map

Every document in OpenRAC, by where it lives and what it is for. Start with
the first table; go to a game's documents when you work on that game.

## Start here

| Document | For |
|---|---|
| [README.md](../README.md) | What OpenRAC is and how to play |
| [DEVELOPING.md](DEVELOPING.md) | Working from a checkout: the launcher from source, setting up the decompilations, the port, the level editor by hand |
| [CONTRIBUTING.md](../CONTRIBUTING.md) | Ground rules, the commit convention, pull requests, tests |
| [AGENTS.md](../AGENTS.md) and [CLAUDE.md](../CLAUDE.md) | Rules for AI agents |
| [LAYOUT.md](LAYOUT.md) | How the repository is organised, and where a new file goes |
| [policy/SOURCING.md](policy/SOURCING.md) | What may and may not be used or committed |
| [policy/OPEN_QUESTIONS.md](policy/OPEN_QUESTIONS.md) | Decisions the group still has to make |
| [TODO.md](TODO.md) | Larger work that is scoped and decided but not started, ready to pick up |
| [port/KNOWN_ISSUES.md](port/KNOWN_ISSUES.md) | Bugs that are understood and guarded but not properly fixed yet |
| [SOURCES.md](SOURCES.md) | Where each game's code came from, what changed on import, how to sync |
| [baserom/README.md](../baserom/README.md), [toolchains/README.md](../toolchains/README.md) | Setting up your discs and compilers; the supported discs and their checksums |

## Across the games

| Document | For |
|---|---|
| [engine/](engine/README.md) | What is known about Insomniac's engine, and in which game it was measured |
| [engine/SHARED_CODE.md](engine/SHARED_CODE.md), [shared/](../shared/README.md) | How much code the versions share, the functions each project could port from another, and the files several games build with |
| [toolchains/](toolchains/README.md) | Which compilers built what, and what each project uses to match |
| [workflow/](workflow/README.md) | How matching works in each game, side by side, and porting between games |
| [games/](../games/README.md) | The games and versions, with a page per game |
| [progress/](../progress/README.md) | Consolidated progress (generated) |
| [editor/README.md](../editor/README.md) | The Godot level editor: extracting your levels, what it reads, its code |
| [editor/GDSCRIPT_CONVENTIONS.md](../editor/GDSCRIPT_CONVENTIONS.md) | Style for the editor's GDScript |
| [port/README.md](port/README.md) | The native port, OpenGOAL's way: no emulation; what is decided and open ([DESIGN.md](port/DESIGN.md)), the order of work ([ROADMAP.md](port/ROADMAP.md)) |
| [restructure/PROGRESS.md](restructure/PROGRESS.md) | The restructure of OpenRAC's own code: every package, its status and owner, and how to help |
| [port/docs/CODING_CONVENTIONS.md](../port/docs/CODING_CONVENTIONS.md) | The port's C++ and C conventions (decided): layout, documentation, naming, ownership, errors, the C boundary, layering, tests; what CI checks |
| [port/WORKFLOW.md](port/WORKFLOW.md) | Helping test and fix the port: from a player's report to a release, and the guides for each step |
| [port/DECOMP_STATUS.md](port/DECOMP_STATUS.md), [port/PORTABILITY.md](port/PORTABILITY.md) | How much of Ratchet & Clank (PAL) is C a PC could build; what the matching C assumes about the console |
| [port/RENDERER.md](port/RENDERER.md) | How the game builds a frame, everything it draws, the VU programs, the graphics chip's conventions |
| [port/RAC1_PAL_SURVEY.md](port/RAC1_PAL_SURVEY.md) | Ratchet & Clank's frame loop, display list, microprograms, menu path, disc, memory card and library surface, from the decompilation |
| [port/VU_PROGRAMS.md](port/VU_PROGRAMS.md) | The games' vector unit programs by name: where the executables list them, which ones the games share |
| [port/OPENGOAL_NOTES.md](port/OPENGOAL_NOTES.md) | How OpenGOAL's renderer and platform layer work, and what carries over |
| [launcher/README.md](../launcher/README.md) | The desktop launcher: developing it, and [connecting a game](../launcher/docs/INTEGRATION.md) to it |
| [LICENSE.md](../LICENSE.md), [THIRD_PARTY_NOTICES.md](../THIRD_PARTY_NOTICES.md), [CREDITS.md](../CREDITS.md) | Licensing by directory, third-party work, the people behind each project |

## Ratchet & Clank, PAL ([games/rac1](../games/rac1))

| Document | For |
|---|---|
| [README.md](../games/rac1/README.md), [CONTRIBUTING.md](../games/rac1/CONTRIBUTING.md), [LEGAL.md](../games/rac1/LEGAL.md) | Setup and build, contributing, what the project contains |
| [docs/WORKFLOW.md](../games/rac1/docs/WORKFLOW.md) | From an `INCLUDE_ASM` stub to matched C, step by step |
| [docs/LEVERS.md](../games/rac1/docs/LEVERS.md), [docs/DECOMP_PROGRESS.md](../games/rac1/docs/DECOMP_PROGRESS.md) | Matching levers; the knowledge base of solved questions and dead ends |
| [docs/TOOLCHAIN.md](../games/rac1/docs/TOOLCHAIN.md) | What builds the image and why (SN ProDG, flags, pipeline) |
| [docs/OVERLAYS.md](../games/rac1/docs/OVERLAYS.md) | The 19 level programs, variants, the US to PAL map |
| [docs/ASSETS.md](../games/rac1/docs/ASSETS.md) | Disc, WAD and level data formats, with evidence; what the editor reads |
| [docs/NAMES.md](../games/rac1/docs/NAMES.md), [docs/NONMATCHING.md](../games/rac1/docs/NONMATCHING.md), [docs/ASM_CLASSIFICATION.md](../games/rac1/docs/ASM_CLASSIFICATION.md) | Symbol names, near-miss staging, handwritten assembly |
| [docs/AGENT_WORKFLOW.md](../games/rac1/docs/AGENT_WORKFLOW.md), [docs/QUEUE.md](../games/rac1/docs/QUEUE.md), [docs/WORKER.md](../games/rac1/docs/WORKER.md), [docs/LONG_FUNCTIONS.md](../games/rac1/docs/LONG_FUNCTIONS.md), [docs/LLM_DECOMP_INSTRUCTIONS.md](../games/rac1/docs/LLM_DECOMP_INSTRUCTIONS.md) | AI agent waves, worker protocols, claims (the last in Czech) |
| [docs/PERMUTER.md](../games/rac1/docs/PERMUTER.md), [docs/CONTAINERS.md](../games/rac1/docs/CONTAINERS.md) | decomp-permuter; the build and Ghidra containers |
| [docs/SIBLING_DECOMPS.md](../games/rac1/docs/SIBLING_DECOMPS.md) | The other projects and how to port from them |
| [docs/DATAMINED_REVERSE_ENGINEERING.md](../games/rac1/docs/DATAMINED_REVERSE_ENGINEERING.md) | A generated Ghidra datamining report |
| [notes/](../games/rac1/notes) | Dated investigation logs and per-function write-ups (60 files) |
| [nonmatching/README.md](../games/rac1/nonmatching/README.md) | Index of staged near misses (generated) |

## Ratchet & Clank, NTSC-U ([games/rac1/ntsc](../games/rac1/ntsc))

| Document | For |
|---|---|
| [README.md](../games/rac1/ntsc/README.md), [CONTRIBUTING.md](../games/rac1/ntsc/CONTRIBUTING.md) | The project, its seven-step contribution loop |
| [docs/building.md](../games/rac1/ntsc/docs/building.md) | Automatic and manual builds, environment variables, targets |
| [docs/decompilation-tips.md](../games/rac1/ntsc/docs/decompilation-tips.md), [docs/progress-metrics.md](../games/rac1/ntsc/docs/progress-metrics.md) | Acceptance discipline and matching tips; what C_EXACT and C_FUZZY measure |
| [docs/patched-toolchain.md](../games/rac1/ntsc/docs/patched-toolchain.md), [patches/README.md](../games/rac1/ntsc/patches/README.md) | The patched EE-GCC: lineage, rules, hashes |
| [docs/overlays.md](../games/rac1/ntsc/docs/overlays.md) | Level programs in the US build |
| [docs/engine-source-layout.md](../games/rac1/ntsc/docs/engine-source-layout.md), [docs/moby-selector-scratch-path.md](../games/rac1/ntsc/docs/moby-selector-scratch-path.md), [docs/recovered-names.md](../games/rac1/ntsc/docs/recovered-names.md) | Engine source layout (from prerelease builds), a moby data path, the names catalog |
| [docs/commit-messages.md](../games/rac1/ntsc/docs/commit-messages.md) | What a good commit summary and body say (OpenRAC's convention applies) |
| [config/README.md](../games/rac1/ntsc/config/README.md), [src/README.md](../games/rac1/ntsc/src/README.md) | Directory notes |

## Ratchet & Clank: Going Commando ([games/rac2/ntsc](../games/rac2/ntsc))

| Document | For |
|---|---|
| [README.md](../games/rac2/ntsc/README.md), [docs/BUILDING.md](../games/rac2/ntsc/docs/BUILDING.md), [CONTRIBUTING.md](../games/rac2/ntsc/CONTRIBUTING.md), [AGENTS.md](../games/rac2/ntsc/AGENTS.md) | The project, first steps, contribution rules, agent rules |
| [docs/BUILDING.md](../games/rac2/ntsc/docs/BUILDING.md), [docs/LEVELS.md](../games/rac2/ntsc/docs/LEVELS.md), [docs/COMPILER-NOTES.md](../games/rac2/ntsc/docs/COMPILER-NOTES.md) | Building (Docker on any host), the level programs and the catalogue, the compiler |
| [toolchain/README.md](../games/rac2/ntsc/toolchain/README.md), [tools/compiler/README.md](../games/rac2/ntsc/tools/compiler/README.md) | The tools to supply; the compilers rebuilt from source |
| [docs/COMPILER-NOTES.md](../games/rac2/ntsc/docs/COMPILER-NOTES.md) | The C compiler profile and its patches |
| [docs/LEVEL-ARCHIVE-FORMAT.md](../games/rac2/ntsc/docs/LEVEL-ARCHIVE-FORMAT.md), [docs/MOBY-DISPATCH-TABLES.md](../games/rac2/ntsc/docs/MOBY-DISPATCH-TABLES.md), [docs/VU-MICROPROGRAMS.md](../games/rac2/ntsc/docs/VU-MICROPROGRAMS.md) | Level archives, class dispatch tables, VU microprograms |
| [docs/COMMUNITY-ENGINE-REFERENCE.md](../games/rac2/ntsc/docs/COMMUNITY-ENGINE-REFERENCE.md) | A reverse-engineering reference ("reference, never evidence") |
| [docs/RAC1-TO-RAC2.md](../games/rac2/ntsc/docs/RAC1-TO-RAC2.md), [docs/SECOND-C-LOT.md](../games/rac2/ntsc/docs/SECOND-C-LOT.md) | What carries over from RAC1, and the bodies reused from it |
| [docs/LEVEL-NATIVE-C.md](../games/rac2/ntsc/docs/LEVEL-NATIVE-C.md), [docs/LEVEL-INTEGRATION-PLAN.md](../games/rac2/ntsc/docs/LEVEL-INTEGRATION-PLAN.md), [docs/C-NATIVE-EXPERIMENT-REGISTER.md](../games/rac2/ntsc/docs/C-NATIVE-EXPERIMENT-REGISTER.md) | C in the level overlays, the plan, the register of trials |
| [docs/PROTOTYPE-BUILDS.md](../games/rac2/ntsc/docs/PROTOTYPE-BUILDS.md), [docs/AUG6-RETAIL-ANCHORS.md](../games/rac2/ntsc/docs/AUG6-RETAIL-ANCHORS.md), [docs/ENGINE-SYMBOL-NAMES.md](../games/rac2/ntsc/docs/ENGINE-SYMBOL-NAMES.md), [docs/ASSERT-MESSAGE-NAMES.md](../games/rac2/ntsc/docs/ASSERT-MESSAGE-NAMES.md) | Prerelease builds and the names drawn from them; names from retail assert messages |
| `docs/*-C-LOT.md`, `docs/*-NATIVE-LOT.md`, [docs/INTEGRATION-FIRST-LOT.md](../games/rac2/ntsc/docs/INTEGRATION-FIRST-LOT.md) | The matching log, one file per lot (28 files) |

## Ratchet & Clank: Up Your Arsenal ([games/rac3/ntsc](../games/rac3/ntsc))

| Document | For |
|---|---|
| [README.md](../games/rac3/ntsc/README.md), [CONTRIBUTING.md](../games/rac3/ntsc/CONTRIBUTING.md) | The project, contribution rules |
| [docs/wiki/Home.md](../games/rac3/ntsc/docs/wiki/Home.md) | The wiki's index: [Setup](../games/rac3/ntsc/docs/wiki/Setup.md), [Toolchain and build](../games/rac3/ntsc/docs/wiki/Toolchain-and-Build.md), [Workflow](../games/rac3/ntsc/docs/wiki/Workflow.md), [Matching patterns](../games/rac3/ntsc/docs/wiki/Matching-Patterns.md), [Pull requests](../games/rac3/ntsc/docs/wiki/Pull-Requests.md), [Tools](../games/rac3/ntsc/docs/wiki/Tools.md), [Cross-repository resources](../games/rac3/ntsc/docs/wiki/Cross-Repository-Resources.md) |
| [docs/compiler_matrix_findings.md](../games/rac3/ntsc/docs/compiler_matrix_findings.md) | 15 compilers against 8 flag sets: what built the game |
| [docs/source_files.md](../games/rac3/ntsc/docs/source_files.md), [docs/trailing_padding.md](../games/rac3/ntsc/docs/trailing_padding.md) | How the source files are laid out; trailing padding |
| [docs/common_level_c.md](../games/rac3/ntsc/docs/common_level_c.md), [docs/shared_code_findings.md](../games/rac3/ntsc/docs/shared_code_findings.md) | C for code common to the levels; code shared between levels |
| [docs/full_match_roadmap.md](../games/rac3/ntsc/docs/full_match_roadmap.md), [docs/permuter.md](../games/rac3/ntsc/docs/permuter.md), [docs/permuter_todo.md](../games/rac3/ntsc/docs/permuter_todo.md) | The plan to 100%; decomp-permuter and its worklist |

## Ratchet: Deadlocked ([games/rac4/ntsc](../games/rac4/ntsc))

| Document | For |
|---|---|
| [README.md](../games/rac4/ntsc/README.md), [CONTRIBUTING.md](../games/rac4/ntsc/CONTRIBUTING.md) | The project, setup, adding a function, matching tips, naming, commits |
| [LEGAL.md](../games/rac4/ntsc/LEGAL.md) | What may be committed, allowed sources, the naming rule |
| [docs/RESEARCH.md](../games/rac4/ntsc/docs/RESEARCH.md) | The packed executable, its 17 sections, the compiler and assembler, libgcc and libm, open questions |
| [docs/OVERLAYS.md](../games/rac4/ntsc/docs/OVERLAYS.md) | The 47 level overlays: obtaining, splitting, counting and disassembling them |
| [docs/CREDITS.md](../games/rac4/ntsc/docs/CREDITS.md), [THIRD_PARTY_NOTICES.md](../games/rac4/ntsc/THIRD_PARTY_NOTICES.md) | Projects it drew on; GCC and newlib notices |

## Ratchet & Clank: Size Matters ([games/rac5/ntsc](../games/rac5/ntsc))

| Document | For |
|---|---|
| [README.md](../games/rac5/ntsc/README.md), [CONTRIBUTING.md](../games/rac5/ntsc/CONTRIBUTING.md), [AGENTS.md](../games/rac5/ntsc/AGENTS.md) | The project, what is established, how to contribute and check work |
| [LEGAL.md](../games/rac5/ntsc/LEGAL.md) | What may be committed, allowed sources |
| [docs/ROADMAP.md](../games/rac5/ntsc/docs/ROADMAP.md) | The gates still open |
| [docs/REL-FORMAT.md](../games/rac5/ntsc/docs/REL-FORMAT.md), [docs/BOOT-SYMBOLS.md](../games/rac5/ntsc/docs/BOOT-SYMBOLS.md) | The SNR2 level modules; the boot's named symbols |
| [docs/BOOT-TRIAL-GATE.md](../games/rac5/ntsc/docs/BOOT-TRIAL-GATE.md), [docs/BOOT-MATCH-SCOPE.md](../games/rac5/ntsc/docs/BOOT-MATCH-SCOPE.md) | How a function is accepted, and what a boot match covers |

## Ratchet & Clank: Secret Agent Clank ([games/rac6/ntsc](../games/rac6/ntsc))

| Document | For |
|---|---|
| [README.md](../games/rac6/ntsc/README.md), [CONTRIBUTING.md](../games/rac6/ntsc/CONTRIBUTING.md), [AGENTS.md](../games/rac6/ntsc/AGENTS.md) | The project, what is established, how to contribute and check work |
| [LEGAL.md](../games/rac6/ntsc/LEGAL.md) | What may be committed, allowed sources |
| [PROMPT.md](../games/rac6/ntsc/PROMPT.md) | The starter prompt and wave protocol |
| [docs/STATE.md](../games/rac6/ntsc/docs/STATE.md) | Where the project stands, open gates |
| [docs/METHOD.md](../games/rac6/ntsc/docs/METHOD.md), [docs/GATES.md](../games/rac6/ntsc/docs/GATES.md) | Wave protocol, body gate, placement gate, complete-program gate |
