# Contributing to OpenRAC

Thanks for helping. OpenRAC holds a decompilation project for each version of
each game, plus a few things they share. This page covers what applies
everywhere; each game's own docs cover how work is done there. AI agents also
read [AGENTS.md](AGENTS.md), which builds on this page.

## Before you start

1. Read the [sourcing policy](docs/policy/SOURCING.md). It is short, and it is
   the one rule nobody may bend: your own disc is the evidence; Sony SDK
   source, samples and headers, and leaked or NDA material, are never used.
2. See how the repository is organised: [docs/LAYOUT.md](docs/LAYOUT.md).
3. Pick a game in [games/](games/README.md), then set up your discs and
   toolchains ([baserom/README.md](baserom/README.md),
   [toolchains/README.md](toolchains/README.md)):

   ```sh
   python3 tools/openrac.py discs     # identify and check your images in baserom/
   python3 tools/openrac.py setup     # put each game's inputs where its build expects them
   ```

4. Work from that game version's directory, following its README and its
   workflow ([docs/workflow](docs/workflow/README.md) compares them).

## Ground rules

- **Never commit** disc images, executables, extracted data, generated
  assembly, build output or toolchains. The `.gitignore` files cover the
  usual places; check `git status` before every commit anyway.
- **Each game proves matches its own way**, and its docs are the authority:
  a byte-identical rebuild, a strict per-function check, or both. Do not
  land a function the game's checks reject.
- **Keep a change inside one game version** unless it is shared work. A
  game's build must not reach into another game's directory.
- **Generated files are regenerated, never edited by hand**: `progress/`, the
  tables in `README.md`, `baserom/README.md` and `games/README.md`
  (`python3 tools/openrac.py progress`), and each game's own generated files
  (for example rac1/pal's `progress/report.json`, `nonmatching/README.md`
  and `include/names.h`).
- **A shared file changes in every game at once.** `shared/files.json` lists
  the files several games build with; `python3 tools/shared.py check` fails
  when their copies differ ([shared/README.md](shared/README.md)).
- **Credit what you reuse**: the project, the file or function, and its
  license, in the game's `THIRD_PARTY_NOTICES.md` and at the code.
- **Open questions are decided together.** If your change depends on one
  ([docs/policy/OPEN_QUESTIONS.md](docs/policy/OPEN_QUESTIONS.md)), raise it
  rather than settling it in passing.

## Commits

OpenRAC uses [Conventional Commits](https://www.conventionalcommits.org/).
Inside OpenRAC this replaces the commit styles of the original projects
(Lombyte's `docs/commit-messages.md`, rac2's `AGENTS.md` and `.gitmessage`).

```
type(scope): summary in the imperative or as a short description

Body: what changed and why, wrapped at 72 columns. Say what you verified
(the build, the checks, the tests) and anything left undone.

Optional-Footer: value
```

**Types**

| Type | Use it for |
|---|---|
| `feat` | New capability: matched functions, a new tool or command, a new editor feature |
| `fix` | A bug fix: in code, tools, a build, or a wrong statement in docs that people rely on |
| `docs` | Documentation only |
| `refactor` | Restructuring that changes no behaviour or output (moving files, renaming) |
| `perf` | Faster or smaller, same output |
| `test` | Adding or fixing tests only |
| `build` | Build system, containers, toolchain wiring, dependencies |
| `ci` | Continuous integration configuration |
| `chore` | Maintenance with no change to code behaviour: regenerated reports, staged near misses, imports and syncs of the sister projects |
| `style` | Formatting only |
| `revert` | Reverting an earlier commit (name it in the body) |

**Scopes** name what the commit touches: a game version (`rac1/pal`,
`rac1/ntsc`, `rac2`, `rac3`, `rac4`, `rac5`), or a shared part (`editor`,
`launcher`, `port`, `tools`, `docs`, `progress`, `policy`, `sources`,
`setup`, `legal`, `import`, `games`).
A scope may be narrower when that helps (`rac1/pal/overlays`). Leave it out
only for a change to the repository as a whole.

**Granularity**

- One logical change per commit, so it can be reviewed, reverted or
  bisected on its own. A batch of matched functions in one game is one
  change; a matched function and an unrelated tool fix are two.
- Every commit leaves what it touches working: the game still builds and
  matches, the tests still pass.
- Keep regenerated output in its own commit when it is large (a progress
  report after a batch of matches: `chore(progress): ...`), unless the game's
  checks require it with the source: rac1/pal's `progress/report.json` goes in
  the same commit as the functions it counts, because CI checks it against
  `src/` on every commit.
- Mark a change that breaks something others rely on with `!` after the
  scope and a `BREAKING CHANGE:` footer.

**Examples**

```
feat(rac1/pal): func_L05_002D48B8 exact match
fix(rac2): stop pointing to RAC1's moby layout in place of the removed one
docs(toolchains): compare the compilers each game was built with
refactor(editor): move the level editor to the top level
build(rac1/pal): mount all of OpenRAC in the build container
chore(progress): consolidate the progress of all four games
chore(import): rac1/ntsc from Lombyte at 1a2b3c4
```

**Credit lines for AI assistants.** Commits made with an AI assistant end
with a `Co-Authored-By:` trailer naming it, as OpenRAC's commits have so far.
Whether to keep that rule is [open question 2](docs/policy/OPEN_QUESTIONS.md#2-credit-lines-for-ai-assistants-in-commits).

## Pull requests

- One topic per pull request, titled like a commit summary.
- Say what you verified and how (commands and their results), and follow the
  game's own checklist (for example rac1/ntsc's and rac3's pull request
  templates under `games/*/*/.github/`).
- Never include generated or disc-derived files.

## Tests

```sh
python3 -m unittest discover -s tools     # tools/openrac.py
python3 -m unittest discover -s editor    # the level editor (synthetic data, no disc)
```

The C++ under `port/` and `sdk/` has a layout check (the rules are in
[port/docs/CODING_CONVENTIONS.md](port/docs/CODING_CONVENTIONS.md), section 5):

```sh
python3 tools/cpp_style.py [PATH...]          # list the files that would change
python3 tools/cpp_style.py --fix [PATH...]    # rewrite them: clang-format, then the blank lines around control blocks
python3 tools/cpp_style.py --comments [PATH...]   # control blocks with no comment above them (report only)
```

CI runs it as a report for now; it does not fail the build.

Each game has its own: rac2's `python -m unittest discover -s tests`,
rac1/ntsc's `python3 scripts/test_public_tools.py` (Linux), rac1/pal's build
audit and `tools/overlay_file_check.py`, rac3's `tools/pr_check.py`.

## Bringing in work from a project's own repository

While the projects are still active in their original repositories, their
new commits are applied with `tools/sources.py sync` and committed as
`chore(import)`, after the same review against the sourcing policy
([docs/SOURCES.md](docs/SOURCES.md#bringing-in-later-work)).
