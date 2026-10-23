# openrac_common

The helpers OpenRAC's Python tools share: finding the repository, reading a game's manifest and the
tab-separated tables, hashing a file, placing files, running a program and reading a disc image.
Before it, each of these was written several times over in `tools/`, `port/tools/`, `sdk/tools/`
and `editor/` (five readers of `game.json`, nine of `libraries.tsv`, four of a disc image), each
copy with its own small differences. Each helper here is written from the best existing copy, keeps
the quirks every caller relies on, and names its sources in its module docstring.

It is not a framework and not a place for one tool's logic: a helper belongs here when at least two
tools need it. It holds no game knowledge beyond the layout of `games/<game>/game.json` and the
format of a PS2 disc, and no game data.

The tools in `tools/` use it since P1-05, `port/tools/versionmap` and the probe tools since P2-10; `editor/` and `sdk/tools/`
move onto it in their own packages ([PROGRESS.md](../../docs/restructure/PROGRESS.md)). Who uses
what today:

| Module | Used by |
|---|---|
| `paths` | every tool in `tools/` that knows the repository: `openrac`, `extractor`, `sources`, `shared`, `xmap`, `versions`, `port`, `build_port`, `build_android`, `bundle_standalone`, `fcheck_gate`; `port/tools/versionmap` (`versionmap`, `carry_functions`, `hostnames`); the probe tools' `seed_regions` and `moby_seeds` |
| `games` | `openrac`, `extractor`, `sources`, `xmap`; `fcheck_gate` (`source_checkout`); `versionmap` |
| `tables` | `fcheck_gate`, `port`, `xmap`, `versions`; `carry_functions` |
| `digest` | `openrac`, `extractor`, `shared`, `build_port`, `build_android`, `fcheck_gate`; the probe tools' `bake_all` and `seed_regions` |
| `files` | `openrac` (`place_link`), `extractor`, `bundle_standalone` |
| `proc` | every `git` call in `tools/`; `build_android`'s and `bundle_standalone`'s commands |
| `iso9660` | `openrac`, `extractor` |

Two places in `tools/` keep their own code on purpose. `fcheck_level_build.py` runs copied alone
inside rac1-decomp's container, from rac1-decomp's root, where neither OpenRAC nor this package is.
`openrac.py`'s `checksums` hashes a disc image with several algorithms and CRC32 in one pass, which
`digest` (one algorithm a read) would make three reads of 4 GiB.

## Interface

| Module | What it gives |
|---|---|
| `paths` | `repo_root(start=None)`: the top of OpenRAC, the nearest directory above START holding `games/` and `AGENTS.md` (a checkout) or `STANDALONE` (the launcher's bundled copy) |
| `games` | `load(game)`, `load_all()`, `versions()`, `games_dir()`, `manifest_path(game)`, `version_dir(game, name)`: `games/<game>/game.json`; `source_checkout(game_id, version_name)`, `source_variable(game_id, version_name)`: where a version's sister project is checked out |
| `tables` | `read_tsv(path, columns=None, strip=False, required=False)`, `header(path)`, `is_data(line)`, `split_row(line)`: the `.tsv` tables and their comment and header conventions |
| `digest` | `sha1(path)`, `sha256(path)`, `file_digest(path, algorithm)`: hex digests read in 1 MiB blocks |
| `files` | `link_or_copy(source, dest, keep_metadata=True)`, `copy_tree(source, dest, keep)` |
| `proc` | `run(cmd, cwd, env, echo, check, capture)`, `git(repo, *args, check, binary)`: no shell, ever |
| `iso9660` | `boot_serial(image)`, `parse_boot2(text)`, `files(image)`, `read(image, path)`, `find(f, path)`, `walk`, `records`, `root_directory`, `IsoError` |

`repo_root` accepts two kinds of top directory, and both are meant: a checkout, marked by
`AGENTS.md`, and the copy of the tools the launcher carries for players
(`tools/bundle_standalone.py`), which has no `AGENTS.md` and is marked by the `STANDALONE` note that
script writes. The launcher recognises that copy by the same file (`launcher/core/src/detect.rs`), and
the copy's tools find their root through `repo_root`. The copy carries this package beside
`openrac.py` and `extractor.py`, which import it.

### Where the sister checkouts are

A version synced from a matching decomp names that project under `source` in its `game.json`,
`checkout` included: the path of its local checkout, `~` for the home folder. `games.source_checkout`
is the one rule every tool follows to find it, decided by the maintainers (P2-30):

1. `$OPENRAC_SOURCE_<GAME>_<VERSION>` (`games.source_variable`; `OPENRAC_SOURCE_RAC1_PAL` for rac1's
   PAL tree), when it is set and not empty;
2. else `game.json`'s `checkout`, with `~` expanded;
3. `None` for a version with no `source`, whatever the environment says.

`game.json` records where the projects' own checkouts are; the variable is for a machine that keeps
one somewhere else. Nothing checks that the folder exists: each tool says what is missing in its own
words. `sources.py` (import, sync, status) and `openrac.py` (`progress --fetch` and the reports'
dates) use it; `fcheck_gate.py` uses it for rac1-decomp, behind its own `OPENRAC_DECOMP`, which
still wins when set.

Every function takes a `pathlib.Path` or a string for a path. The functions that read the repository
(`games.*`) take an optional `root`, so a test can point them at a synthetic tree.

### Using it

A script in `tools/` imports it directly, since Python puts the script's folder on the path:

```python
from openrac_common import games, iso9660
```

A script anywhere else runs one line first, the bootstrap (`paths.BOOTSTRAP`), and then imports it
the same way:

```python
import sys
from pathlib import Path

sys.path.insert(1, str(next(p for p in Path(__file__).resolve().parents if (p / "tools/openrac_common").is_dir()) / "tools"))
from openrac_common import paths  # noqa: E402
```

The rule, decided in P2-10:

- **One line, the same everywhere.** Every script outside `tools/` that imports the package writes
  the line above verbatim, and nothing else to find it; `tools/test_openrac_common.py` checks each
  one against `paths.BOOTSTRAP`. A script that only imports its own folder's modules writes no
  `sys.path` line at all: Python and `unittest discover` already put that folder on the path.
- **Found by the folder that holds it.** The line walks up to the nearest folder holding
  `tools/openrac_common`, not to `AGENTS.md`: the launcher's bundled copy (`tools/bundle_standalone.py`)
  carries the package and the probe tools, but no `AGENTS.md`. Once the package is imported,
  `paths.repo_root(__file__)` finds the top of either kind of root, by `AGENTS.md` or `STANDALONE`.
- **Second on the path.** `tools/` goes after the script's own folder, so a module of the script's
  keeps its name when `tools/` holds one of the same (`editor/port.py` beside `tools/port.py`), and
  before the installed packages, so a module of `tools/` (`tools/mips.py`, which
  `port/tools/versionmap` imports) is not shadowed by an installed one.

`port/tools/versionmap` and `port/enhanced/tools/probes` use it (P2-10); `editor/` and `sdk/tools/` move onto it with their own
packages.

## Depends on

The Python standard library alone, Python 3.10 and newer (CI runs 3.12). It imports none of the
tools that use it.

## State and threads

None. Every function reads what it is given and returns; nothing is cached between calls, so a
function may be called from any thread.

## Errors

Malformed input raises: `iso9660.IsoError` (a `ValueError`; its `detail` is the text the extractor
shows a player, `""` for an image that is not ISO 9660 at all), `FileNotFoundError` for a manifest,
a required table or a file not on the disc, `subprocess.CalledProcessError` from `proc` when asked
to check. Where an existing reader answered "nothing" instead of raising (`iso9660.find` and
`boot_serial` on a non-disc, `tables.read_tsv` on a missing table), the helper does the same. Nothing
is logged and nothing is printed, except the command `proc.run(echo=True)` shows.

## Configuration

None: no `OPENRAC_*` variable, no settings file. A tool that reads one passes the result in.

## Tests

`tools/test_openrac_common.py`, run with the rest of the tools' tests:

```sh
python3 -m unittest discover -s tools
```

Synthetic data only: the disc images are built by the test (a small ISO 9660 with nested
directories and a root directory longer than one sector), never read from a disc. Two kinds of test:

- each helper's own behaviour, with expected values from the format (ECMA-119) or from `hashlib`;
- characterisation: the existing copy each helper was written from runs on the same input, and both
  must agree. The copies in `tools/` are gone (P1-05), and versionmap's and the probe tools' (P2-10), and their
  comparisons with them; those left are `tools/openrac.py`'s `checksums` and `fcheck_gate.py`'s
  `fcheck_commit` (which use the package's results), `port/tools/hostgen/{hostgen,no_c}.py`,
  `sdk/tools/gen_binding.py` and `editor/disc.py`. The tables are also read from this checkout's own `port/game/*/libraries.tsv`
  and compared. When a copy is removed by the package that moves its callers, its comparison goes
  with it.
- the bootstrap: `paths.BOOTSTRAP` run from a script in a synthetic checkout and in a synthetic
  bundled copy (no `AGENTS.md`), and every script outside `tools/` that imports the package
  checked for the line.

The tools that use the package have their own tests beside it (`test_openrac.py`,
`test_extractor.py`, `test_sources.py`, `test_build_android.py`, `test_bundle_standalone.py`, ...);
`test_bundle_standalone.py` also runs the bundled copy's tools from the copy.

## Further reading

- [docs/LAYOUT.md](../../docs/LAYOUT.md): where a tool goes.
- [docs/restructure/PROGRESS.md](../../docs/restructure/PROGRESS.md): which tools have moved onto it.
- ECMA-119 (ISO 9660), the format `iso9660` reads.
