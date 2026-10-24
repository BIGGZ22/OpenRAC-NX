# Versions

This repository covers every release of the game we have. Each one is a version with its own
disc, its own byte-identical build from assembly, its own function inventory and its own progress
report, and they share one C source.

| Key | Report id | Release | Disc image (`baserom/`, yours, never committed) | Built and counted by |
| --- | --- | --- | --- | --- |
| `pal` | `SCES_509.16` | PAL (Europe) v2.00, the **primary** | `SCES_509.16.iso` (and `SCES_509.16`, the executable) | this repository (`tools/build_sn.sh`, `tools/gen_progress_report.py`), unchanged |
| `pal_v1` | `SCES_509.16_v1` | PAL (Europe) v1.00 | `SCES_509.16_v1.00.iso` | this repository (below) |
| `jp` | `SCPS_150.37` | NTSC-J v1.02 | `SCPS_150.37.iso` | this repository (below) |
| `us` | `SCUS_971.99` | NTSC-U v1.00 | `SCUS_971.99.iso` | this repository (below); the count ByteMatch shows is **the NTSC decomp**'s own, Lombyte's project (see the end) |

Both PAL discs have the same serial and boot file name (`SCES_509.16`); their builds differ, so
v1.00's report id is `SCES_509.16_v1`. ByteMatch (bytematch.dev/OpenRAC/rac1-decomp) reads one CI
artifact per version, `<id>_report`.

`tools/versions.py` is the table of versions. A new release (another region, a later disc)
slots in with one more `Version` row there: its key, report id, disc image name, boot file and the
executable's SHA-1. Every tool below takes `--version KEY`.

## The programs: dumps and their byte-identical build

    python3 tools/versions.py dump --version jp      # baserom/jp/SCPS_150.37, baserom/jp/overlays/level_NN/
    python3 tools/versions.py config --version jp    # config/versions/jp/programs.tsv (hashes, addresses, sizes)
    python3 tools/versions.py check                  # every dump against its committed programs.tsv
    python3 tools/version_build.py --version jp -P 16  # 20 programs from their assembly: MATCH each

A version's programs are its boot executable and the 19 level programs (the code overlays each
level loads over the executable's `main` segment, docs/OVERLAYS.md), read straight from the disc
image (ISO 9660 for the executable, the sector table of contents at sector 1500 for the levels;
all four discs lay it out the same way). `config/versions/<K>/programs.tsv` keeps each program's
SHA-1 as the disc stores it (a level's after WAD decompression), its entry point and each
section's address, size and SHA-1; `boot_layout.json` the executable's ELF and section headers.

`tools/version_build.py` writes each program as assembly, one `.s` per function as the version's
function table cuts it (`asm/versions/<K>/`, never committed), assembles it with the toolchain's
binutils 2.10-ee (`ee-as`, a Linux program: native on Linux, through `tools/docker/run.sh` on a
Mac), links every section at its address (`ee-ld`), puts the executable back together from its
layout and each level from its records, and compares the SHA-1 with the disc's. All 20 programs of
each of `pal`, `pal_v1` and `jp` (2026-10-21) and `us` (2026-10-23) give back the disc's bytes:

| Version | Executable SHA-1 | Programs that match |
| --- | --- | --- |
| `pal` | `79956931bd62fafd8d20fa2eae796dbaf2e15e83` | 20 of 20 |
| `pal_v1` | `553a7f6c2c7a004d384d77b7139accbc52242149` | 20 of 20 |
| `us` | `72fd1de379dafd8f243dceeb3be50980f3fd3cd5` | 20 of 20 |
| `jp` | `0df83877d53e19d69ec82038e2aaa227f643178f` | 20 of 20 |

The primary's own build (the C linked into the executable, `tools/build_sn.sh`; level C checked
function by function, `tools/overlay_check.py`) and its report are as they were.

## Counting the functions (`config/versions/<K>/functions.tsv`)

    python3 tools/version_map.py --version jp

Each version's code is cut and counted with the primary's rules (`tools/overlays.py`: `split()`,
the catalogue's identity of masked instructions, the US map's pairing, docs/OVERLAYS.md): each
code section is cut by `split()` and at the primary's functions found in it, so its functions are
cut as their counterparts are; each is paired with the primary's by identity, then in order between
those anchors by similarity; then the level functions are counted once each, whatever the number
of their places, exactly as the primary's catalogue counts them. Each row: its name (the
counterpart's, or `func_<TAG>_...` for code only this version has: `E1` for PAL v1.00, `J` for
NTSC-J, `U` for NTSC-U), its kind (`exe`, `shared`, `level`), size, fingerprint, counterpart,
how it was paired, and its places. `config/versions/pal/functions.tsv` is the primary's in the
same form.

| Version | Executable | Shared level code | Level-specific code | Total |
| --- | ---: | ---: | ---: | ---: |
| `pal` | 1,692 (465,644 bytes) | 1,964 (1,196,676) | 1,468 (2,051,248) | 5,124 functions (report) |
| `pal_v1` | 1,698 (460,224) | 1,931 (1,186,188) | 1,468 (2,047,336) | 5,097 |
| `jp` | 1,702 (467,616) | 1,939 (1,187,792) | 1,468 (2,050,936) | 5,109 |
| `us` | 1,701 (461,232) | 1,933 (1,186,540) | 1,468 (2,047,132) | 5,102 |

## Checking the C against a version (`config/versions/<K>/map.tsv`)

    python3 tools/version_check.py --version jp [--func NAME --diff]

The C is compiled once, by the primary build, each file with its own flags
(`config/file_cflags.txt`): a file builds to the same object for every version. Each C function is
placed where the version has its counterpart, and every relocation in it is resolved as the
primary's checkers resolve it (`tools/try_func.py`, `tools/overlay_check.py`, their %hi/%lo pairing
and their rule for identical copies of a function), giving the primary's address of what it refers
to; that address is translated to the version's, and the result is compared with the version's
bytes, byte for byte. References into the object's own `.rodata` stay masked, as `try_func.py`
masks them in the executable. The translation table comes from the code both versions share: each
pair of counterparts places a function, and each pair with the same instructions pairs every call
target, `$gp` offset and %hi/%lo address in it. `--version pal --self-test` runs the primary
against itself: every C function of `src/` comes out EXACT.

Statuses: `match` (EXACT), `asm` (the primary's original assembly, the same bytes here),
`joined` (a later piece of a joined function that matched), `differs` (the code here is not the
same: bytes the C builds differ outside relocated fields), `refs` (the same instructions, but they
refer to another function or datum here: code that differs in what it calls), `absent` (no
counterpart here), `nomatch` (the check could not place the C: a tool problem), `copy` (see below).

## Shared and version-specific C

Every function has one C source for every version: its file in `src/`. Only where a version's
bytes differ does it get its own definition:

    src/versions/<K>/<path of the shared file without .c>/<function>.c

holding that function alone, under the counterpart's name. It is built inside its own
translation unit, as retail built it: `tools/version_src.py build --version K` compiles the shared
file with the version's definitions put in place of the shared ones, through the same pipeline and
with the same flags as the shared file, so a version can never build a function with options or a
context its object did not have (docs/BUILD_FIDELITY.md applies unchanged).

    bash tools/docker/run.sh python tools/version_src.py try --version jp func_X cand.c   # one candidate
    bash tools/docker/run.sh python tools/version_clone.py --version jp                   # no model

`tools/version_clone.py` writes version-specific C without a model. It compiles and relocates the
shared C for the version as the check does and explains every word that differs, when the sizes
agree, by two kinds of edit: an immediate that is not relocated (a 50 Hz count against a 60 Hz
one, a field offset) takes the new value in every literal that spells the old one (the method
`tools/overlay_variants.py clone` uses for level variants); a relocated field that reaches another
address here (a call to a function this version changed or added, a table that moved) renames the
symbol it names, in that definition only, to the one at the version's address, with the replaced
symbol's declaration. Code and data only a version has are named `func_<TAG>_...` and
`D_<TAG>_...` (`E1`, `J`) and resolve to the version's own address. A candidate is kept only when
it is EXACT. It runs one function per worker, each worker with its own Wine prefix
(`RAC1_WINEPREFIXES`), and `--shard I/N` splits the list across machines.

On 2026-10-21 it gave 76 definitions for PAL v1.00 and 94 for NTSC-J. What is left
(`differs` and `refs` in `config/versions/<K>/map.tsv`) changed size between the releases or
differs in more than numbers and names: 181 functions of PAL v1.00 and 64 of NTSC-J, listed there
for later work.

## Keeping the versions in step

    python3 tools/version_sync.py check    # CI, and part of tools/gen_progress_report.py --check
    python3 tools/version_sync.py run      # before each push (dumps and toolchain needed)

`check` needs no disc: it reads the committed tables. It fails when a version's map or report is
out of date with `src/` (a C function or a version-specific definition without its row, a row whose
source is gone), when a version-specific definition is a copy (the shared C builds that version's
bytes too: status `copy`), when the same code (one name, one fingerprint) is finished in one
version and not in another whose check did not find it different, or when the check could not
place shared C that a version shares (`nomatch`).

**Before each push**, after the primary's build and report (`bash tools/docker/run.sh bash
tools/build_sn.sh`, `... python tools/gen_progress_report.py --no-build`), run `python3
tools/version_sync.py run` (inside the container on a Mac): it checks the dumps, builds every
version's own definitions, checks every C function against every version, carries a version's
own definition to the other versions with the same code (`carry`, kept only where EXACT), and
rewrites the maps and the reports (`tools/version_report.py`). A new match in `src/` therefore
reaches every version where the code is the same with no further step; commit what `run` changes
together with the C.

## The NTSC decomp

NTSC-U is a version like PAL v1.00 and NTSC-J: the shared C, its own definitions only where its
bytes differ (`src/versions/us/`), its build from assembly, its map and its report
(`progress/us/report.json`, our count). Its other decompilation is **the NTSC decomp**, Lombyte's
project by Mateusz Kłysz (https://github.com/mateuszklysz/Lombyte, GPL-3.0 since its a1379f57),
which builds and counts itself with its own compilers and tools. ByteMatch's `SCUS_971.99` figure
is the NTSC decomp's own count: CI generates its report from its tree and uploads it as
`SCUS_971.99_report`. We take its names (docs/NAMES.md), and some of its structs and headers,
credited in THIRD_PARTY_NOTICES.md.

It is not part of this repository. It is pinned to one commit in
`config/versions/us/ntsc_decomp.txt` (`url commit`) and read from a cache outside the checkout,
`~/.cache/rac1-decomp/ntsc/<commit>/` (`RAC1_NTSC_DECOMP` moves the cache), a shallow fetch of
that one commit; `RAC1_NTSC_DECOMP_DIR` names a checkout of your own instead:

    python3 tools/ntsc_source.py path       # its checkout at the pin, fetched the first time (a few seconds)
    python3 tools/ntsc_source.py follow     # follow it: bump the pin to its HEAD and re-import (below)
    tools/build_ntsc.sh                     # its setup.sh, make elf and make overlays: PASS each
    python3 tools/version_sync.py ntsc      # config/versions/us/carry.tsv

**Following it.** `python3 tools/ntsc_source.py follow [COMMIT]` (default: its HEAD) fetches the
new commit, writes the pin, runs `tools/build_ntsc.sh` (skip with `--no-build` on a host without
its toolchain), `tools/version_sync.py ntsc`, and commits the pin with `carry.tsv` ("chore(ntsc):
follow the NTSC decomp to ..."); then `tools/names.py ntsc` and `rename`, committed on their own
("chore(names): the NTSC decomp's names at ..."). It ends by listing the functions the NTSC
decomp newly matched that are still open here.

`tools/build_ntsc.sh` runs the NTSC decomp's own entry points on Linux (its `setup.sh --elf`,
`make elf`, `make overlays`) in that checkout, taking the executable from `baserom/us/SCUS_971.99`
and the disc image from `baserom/SCUS_971.99.iso`; its header lists the settings for a host without
system Wine. At 6abcf077 it ends in `PASS: reconstructed boot ELF matches retail` and
`PASS: all 3002 overlay functions in C match retail` (with all 6,630 level data objects in C).

Nothing is committed into the NTSC decomp from here and nothing is pushed to it.
`tools/version_sync.py ntsc` reads its report (generated in its checkout) and our check of the C
against the NTSC-U bytes (`config/versions/us/map.tsv`), and lists in `config/versions/us/carry.tsv`
the functions with the same code matched on one side only: "to the NTSC decomp" (our C builds the
NTSC-U bytes, it has not matched them: a pull request on its repository) and "from the NTSC
decomp" (matched there, not here).

## Totals (2026-10-23)

| Version | Functions finished | Code finished |
| --- | --- | --- |
| `SCES_509.16` (PAL v2.00) | 5,089 of 5,120 | 98.48% (3,656,816 of 3,713,416 bytes) |
| `SCES_509.16_v1` (PAL v1.00) | 4,848 of 5,092 | 76.90% (2,840,248 of 3,693,592 bytes) |
| `SCPS_150.37` (NTSC-J v1.02) | 4,983 of 5,104 | 88.13% (3,266,172 of 3,706,188 bytes) |
| `SCUS_971.99` (NTSC-U v1.00), our count (`progress/us/report.json`) | 4,756 of 5,097 | 74.21% (2,741,936 of 3,694,748 bytes) |
| `SCUS_971.99` (NTSC-U v1.00), the NTSC decomp's own count (on ByteMatch) | 4,221 of 4,244 | 98.93% (3,510,004 of 3,547,932 bytes) |

The NTSC decomp groups level copies and pieces differently, hence its 4,244 functions against our
5,097, and counts its level data objects in C as well (not shown here).
The code share of PAL v1.00, NTSC-J and our NTSC-U trails their function share: the functions still to do
are the long ones whose size changed between releases (the menus and text code; NTSC-J's text and
font handling for Japanese). `progress/<K>/report.json` and ByteMatch have the current numbers.
