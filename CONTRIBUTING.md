# Contributing to OpenRAC

Thanks for helping. Most work happens in the native PC port (`port/`) and the
launcher (`launcher/`); the decompilations the port is built from live in
`games/`, one project per game version, each with its own docs.

## Before you start

1. Read the [sourcing policy](docs/policy/SOURCING.md). It is short, and it is
   the one rule nobody may bend: your own disc is the evidence. Sony SDK
   source, samples and headers, and leaked or NDA material, are never used.
2. Set up a checkout: [docs/DEVELOPING.md](docs/DEVELOPING.md).
3. See where things go: [docs/LAYOUT.md](docs/LAYOUT.md).

## Ground rules

- **Never commit game data**: disc images, executables, extracted data,
  generated assembly, build output or toolchains. Check `git status` before
  every commit.
- **Regenerate generated files with their tools**, never by hand: `progress/`
  and the tables in the READMEs (`python3 tools/openrac.py tables`), and each
  game's own generated files.
- **A shared file changes everywhere at once.** `shared/files.json` lists the
  files several games build with; `python3 tools/shared.py check` fails when
  their copies differ.
- **Credit what you reuse**: the project, the file and its license, in
  `THIRD_PARTY_NOTICES.md` and at the code.
- **Respect removal requests.** Refer to the third-party NTSC decompilation of
  Ratchet & Clank only as "the NTSC decomp", never by its author's name,
  handle or address, and never bring back material a project removed at
  someone's request.
- **Leave other people's work alone**: their files, their uncommitted
  changes, and pushed history. Do not install a game's own git hooks:
  `.git/hooks` serves all of OpenRAC.
- **Open questions are decided together.** If your change depends on one
  ([docs/policy/OPEN_QUESTIONS.md](docs/policy/OPEN_QUESTIONS.md)), raise it.
- **Decompilation work follows its game's rules.** Work from the game's
  directory and follow its README: each game proves a match its own way, and
  its checks have the last word.

## Commits

[Conventional Commits](https://www.conventionalcommits.org/):

```
type(scope): short summary

What changed and why, wrapped at 72 columns, and what you verified.
```

- **Types**: `feat`, `fix`, `docs`, `refactor`, `perf`, `test`, `build`,
  `ci`, `chore` (regenerated files, imports), `style` (formatting only),
  `revert`.
- **Scopes**: a part (`port`, `launcher`, `editor`, `tools`, `docs`, `policy`,
  `progress`, `sources`) or a game version (`rac1/pal`, `rac1/ntsc`, `rac2`
  to `rac6`). Narrower is fine (`rac1/pal/overlays`).
- **One logical change per commit**, each leaving the build and tests
  working. Large regenerated output goes in its own commit, except rac1/pal's
  `progress/report.json`, which CI checks with the source.

```
fix(port): never skip a frame that uploads a picture
feat(launcher): slide between pages and games on desktop
feat(rac1/pal): func_L05_002D48B8 exact match
```

Commits made with an AI assistant end with a `Co-Authored-By:` trailer naming
it, for now ([open question 2](docs/policy/OPEN_QUESTIONS.md#2-credit-lines-for-ai-assistants-in-commits)).

## Pull requests

- One topic per pull request, titled like a commit summary.
- Say what you verified and how: the commands and their results.
- Never include generated or disc-derived files.

## Tests

```sh
cd port && cmake --preset release && cmake --build --preset release && ctest --preset release
cd launcher && yarn verify && cargo test -p openrac-launcher-core && yarn test:android
python3 -m unittest discover -s tools      # the repository's tools
python3 -m unittest discover -s editor     # the level editor
python3 tools/cpp_style.py [--fix] [PATH]  # the C++ layout (port/docs/CODING_CONVENTIONS.md)
```

Each game has its own checks; its README names them.

## Working with an AI assistant

The rules above apply the same way. On top of them, an assistant:

- pushes, opens pull requests or publishes only when asked;
- asks before automated game runs (scripted play, screenshots, sweeps): they
  open windows on your screen and take long to review;
- reports what it checked as it is, failures included;
- follows a game's own agent docs where there are some (rac1/pal's
  [docs/AGENT_WORKFLOW.md](games/rac1/docs/AGENT_WORKFLOW.md), run from
  `games/rac1`; the `AGENTS.md` of rac2, rac5 and rac6).
