# Level code overlays

Each level carries its own build of the game program, which replaces the
executable's `main` segment (literals, bss, data, three vtables, text) when
the level loads (docs/ASSETS.md, "Code overlays"). The executable's game
code is a subset of every level's program; all distinct game code is about
3.5 MB. This page is the plan for decompiling it, and the contract between
the tools involved.

## Layers

| | Tracked | Made by |
|---|---|---|
| `baserom/overlays/level_NN/{lit,bss,data,vtbl,camvtbl,sndvtbl,text}.bin` and `manifest.json` (record addresses) | no, game data | `tools/overlays.py dump` |
| `config/overlays/functions.tsv`: every distinct function, its name, kind and places | yes | `tools/overlays.py catalogue` |
| `asm/overlays/<name>.s`: one file per distinct non-exe function, from its canonical level | no, generated | `tools/overlay_asm.py` |
| `src/overlays/...`: C and `INCLUDE_ASM` stubs for overlay functions | yes | generated stubs, then matching |
| `config/overlays/us_map.tsv`: every US function (executable and levels) and its PAL counterpart | yes | `tools/overlays.py us-map` |
| `config/overlays/rerac_notes.tsv`: ReRAC's names and notes on our functions | yes | `tools/overlays.py rerac-notes` |

## Levels

A level is known by its index everywhere: `baserom/overlays/level_NN/`,
the `NN` in `func_LNN_*` and `D_LNN_*`, and the report category
`level_NN`. Only the source directory and the report's label carry the
planet, from `tools/levels.py`:

| NN | Directory | Planet |
|---|---|---|
| 00 | `l00_veldin1` | Veldin |
| 01 | `l01_novalis` | Novalis |
| 02 | `l02_aridia` | Aridia |
| 03 | `l03_kerwan` | Kerwan |
| 04 | `l04_eudora` | Eudora |
| 05 | `l05_rilgar` | Rilgar |
| 06 | `l06_blarg` | Blarg Station (Nebula G34) |
| 07 | `l07_umbris` | Umbris |
| 08 | `l08_batalia` | Batalia |
| 09 | `l09_gaspar` | Gaspar |
| 10 | `l10_orxon` | Orxon |
| 11 | `l11_pokitaru` | Pokitaru |
| 12 | `l12_hoven` | Hoven |
| 13 | `l13_gemlik` | Gemlik Base |
| 14 | `l14_oltanis` | Oltanis |
| 15 | `l15_quartu` | Quartu |
| 16 | `l16_kalebo3` | Kalebo III |
| 17 | `l17_fleet` | Drek's Fleet |
| 18 | `l18_veldin2` | Veldin (return) |

Evidence for the order:

- The save's unlocked-planet flags are one byte per level in this order
  (offset 0x00 Veldin 1 through 0x12 Veldin 2; community memory map,
  "Ratchet & Clank Series Addresses", RaC1 sheet).
- Each level's own debug strings agree: level 02 has the Surfer Agent
  and shark paths (Skid McMarx and the sand sharks of Aridia), 03 the
  train and Helga (Kerwan), 05 the Race Girl and the Bouncer (Rilgar's
  hoverbike race), 13 a boss timer and tractor beam (the Qwark fight at
  Gemlik Base).

## Names

The catalogue deduplicates functions by comparing instructions with their
address fields masked: jump targets, the `lui` of a RAM address, `$gp`
offsets, and immediates on a register that holds or derives from such a
`lui` (see `identity()` in `tools/overlays.py`). Constants, float halves
and struct offsets are compared, so two functions that differ in a number
are two functions (see [Variants](#variants)). Two copies that only call
different functions are one function: the call target is a relocation.

- **exe**: the same code as an executable game function. It keeps that
  name (`func_XXXXXXXX`) and its C lives in `src/game/` as now.
- **shared**: in two or more levels, not in the executable.
- **level**: in one level only.

A shared or level function is named `func_LNN_XXXXXXXX`: its address
(`XXXXXXXX`) in the lowest-numbered level that has it (`NN`), its
*canonical level*. `places` lists every level and address where it occurs;
one level can hold the same code at several addresses (tiny stubs).

Data referenced from overlay code is named by address in the canonical
level: `D_LNN_XXXXXXXX` for addresses in the replaced `main` range
(0x15F000 and up), and the executable's own `D_XXXXXXXX` / `func_XXXXXXXX`
names below it, since the core segment (`core_text`, `core_data`, SDK) stays
resident and is shared by every level.

## Caveats

- Masking hides constants that go through the masked fields, so two
  functions that differ only in such a constant share a fingerprint. The
  per-level rebuild (below) is what finally proves a C body right for every
  place.
- Function boundaries come from calls, returns and tail calls followed by a
  frame opener, plus the executable's functions found in each level.

## Plan

1. **Assembly** (`tools/overlay_asm.py`): write `asm/overlays/<name>.s` for
   each shared and level function, disassembled from its canonical level
   in the executable's `asm/nonmatchings` style, so `INCLUDE_ASM` and the
   assembler take it unchanged.

   `python3 tools/overlays.py dump`, then `python3 tools/overlay_asm.py`
   (spimdisasm in the build container, two passes per level; about 50
   minutes). `tools/setup_asm.sh` runs it last when `baserom/overlays/`
   exists. Each level is disassembled whole, split exactly at the
   catalogue's places, and all 3,307 functions are written:
   - **Jump tables** come from the level's `data` record (not `lit`) and
     are emitted after the function as `dlabel jtbl_LNN_...` in `.rodata`.
   - **Resident names**: the executable's core symbols are given to
     spimdisasm, and addresses still raw after a first pass are named
     `D_XXXXXXXX` (resident) or `D_LNN_XXXXXXXX` (level data) for a second.
     Addresses outside main RAM (the scratchpad, 0x70000000) stay numbers.
   - **Branches written as `.word`** (retail's encoding, the instruction in
     a comment): a branch out of the function, unless its target is the
     next function in the same file; and every backward branch, because
     GNU as's R5900 short-loop fix miscounts after a forward branch and
     pads loops retail's assembler did not (func_L00_002422D8), and no
     option turns it off.
2. **Sources and matching**: `src/overlays/shared/` for shared functions
   and `src/overlays/lNN_<planet>/` for each level's own (see Levels), as
   `INCLUDE_ASM` stubs,
   in link order (see Layout). A file runs until the executable unit its
   functions follow changes, or about 32 KB; it is named after that unit
   and its first function (`hud_00235960.c`). A function that branches
   into the next one stays in its file. The grouping is provisional
   until the original file boundaries are known. The executable build
   stays untouched.

   `tools/try_func.py` takes `func_LNN_*` names. It checks a candidate
   more strictly than an executable function: it links the compiled file
   so the function sits at its canonical address, defines every other
   symbol at its address in that level (from the name, or the catalogue
   for an executable function's name) with `_gp` = 0x166D00, and compares
   the result with the level's bytes. `EXACT` there means every
   relocation reaches the right place.
3. **Audit and progress**: the same check over every C function in
   `src/overlays/`. In `progress/report.json` the shared code is one unit
   ("Shared level code", category `common`) and each level is one
   unit named after it ("Level 04: Eudora", categories `levels` and
   `level_NN`), so each distinct function counts once. decomp.dev lays
   its treemap out in report order with no border around a group: one
   unit per level makes each level one box, and the shared files sit
   side by side. The executable's units and categories
   stay as they are; the report's totals cover everything, so its
   percentage is that of the whole game's code.
4. **Per-level rebuild** (later): link each level's program from the same
   sources and compare it with the level's records, as the executable build
   does. Needs the link order and data layout per level.

## Layout

Measured on the dump (2026-09-27):

- **One link order.** The executable and every level keep their common
  functions in the same order. The executable's units reappear in each
  level as contiguous runs (52 of 53 in level 0, counting functions of 32
  bytes or more that occur once), with level functions between their
  functions: a level program is the same objects linked with more of each
  kept, as a dead-stripping link would. Some executable units span
  several original files (`vendor` has 14 functions spread over 270K of
  level 0).
- **Where level code sits.** 1.42 MB of shared and level code lies inside
  an executable unit's span, 1.80 MB between units (1.17 MB between
  `help` and `hud`, 464K between `vendor` and `movie/movie`).
- **Records.** `lit` starts at 0x15F000 in every level; the other records
  are packed after it at each level's own sizes. `$gp` is 0x166D00
  everywhere, so a level link fixes it rather than deriving it.
- **Shared code** differs between levels only in relocated fields (20
  functions compared across all their places).
- **Jump tables** live in the level's `data` record; its pointers into
  functions are nearly all table entries. Vtable pointers land on the
  catalogue's function starts (3,983 of 3,987).

## Roles

Each level's program dispatches through three tables in its records:

| Record | Entry | Ends at |
|---|---|---|
| `vtbl` | 12 bytes: `{oClass, update function, pointer to a 6-word table}`, one per moby class the level has | oClass -1; the 6-word tables follow it |
| `camvtbl` | 20 bytes: `{camera id, init, activate, update, exit}` | id -1 |
| `sndvtbl` | 8 bytes: `{id, function}` | id -1 |

`python3 tools/overlays.py names` reads them from the dump and writes
`config/overlays/names.tsv`: every catalogued function a table points at,
with its roles (`UpdateMoby_<oClass>`, `InitCamera_<id>`, ...,
`SoundFunc_<id>`) and the levels that use it that way. On the dump of
2026-09-28 that is 690 functions, 610 with a single role: 475 level and
210 shared functions (1.03 MB, a third of all level code) and 5
executable ones. A function with many roles is a generic one (an empty
`Exit`, a class that reuses another's update). An update function gets
the moby in `$a0`. `tools/dossier.py` puts the role in `CONTEXT.md` in
words, with the class's name from `editor/moby_classes.tsv` (OpenRAC's top level)
("update function of moby class 726 (novalis_elevator) on levels 01, 03,
13").

The names stay `func_LNN_XXXXXXXX` everywhere else: a role is a hint for
the worker, and the oClass numbers are RaC1's own.

Four table pointers are not a catalogued function start: 0x10 into the
24-byte func_L00_002EDB58 (InitCamera_7 in levels 0, 1 and 8) and 8 into
the 16-byte func_L08_002DB438 (UpdateMoby_324 in level 8). Each is two
small functions the split joined; the next catalogue run can use the
table pointers as split points.

## Joined functions (`config/overlays/joined.tsv`)

The catalogue ends a function wherever the next address is a function start. Two cases make
one real C function show up as several entries, and `tools/overlay_check.py` (and so
`try_func.py`) handles both by adding the sizes of the pieces listed after the owner:

- A function whose first half branches into the second (the level 18 pairs).
- A function whose last `jr` has its delay slot catalogued as a separate 4-byte entry, a shared
  fragment such as `func_001EC030` (kind `exe`). Retail has one function of size + 4; a C
  function compiles to that size, so the entry reads `owner<TAB>fragment` (16 of these:
  see the file). Write the C under the owner's name and leave the fragment alone.

The pieces must follow the owner in the catalogue; the check compares the C against the bytes of
all of them together, and the progress report counts each piece as finished once the owner is EXACT.

A function that ends in a return of its own which other code also reaches has `func_001E9768`
(the executable's empty function, `jr $31; nop`) as its last piece. The catalogue sizes that entry
4 bytes, from a place where another entry starts in its delay slot, so the check adds the nop:
`func_L16_002E4BD8` is 40 bytes in the catalogue and 48 in retail and in C.

An entry of single words that follows a finished function, each on an 8-byte boundary with a nop
or fill after it, and that nothing branches to, is a different thing: what the linker left of
functions it stripped. Those are listed in `config/overlays/linker_remnants.txt` and marked
`LINKER_REMNANT` in the source (docs/ASM_CLASSIFICATION.md, "Level code"); `tools/overlay_remnants.py`
tells the two apart from the bytes.

## Relatives

`python3 tools/overlays.py families` lists, for each shared and level
function, its most similar other function within 15% of its size
(`config/overlays/families.tsv`, masked instructions compared by
alignment). 428 functions (1.2 MB) have one at 75% or more, nearly all in
another level: level code is often the same source built with small
changes. Match one, then start its relative from that C. Matching order:
shared code in all 19 levels first, then one function per family, then
the rest.

## Variants

137 catalogued functions (23 KB) are *variants*: the same instructions as
another function except for a constant, a float or a struct offset (a moby
class of `0x23D` against `0x23E`, 95.0 against 58.5). The catalogue lists
each with its parent in `config/overlays/variants.tsv`; the parent is the
executable's function of that shape, or the first one in the catalogue.
A variant's stub sits right after its parent in the parent's file, and
variants of executable functions are in `exe_variants.c`.

```
python3 tools/overlay_variants.py stubs     # after regenerating the catalogue
python3 tools/overlay_asm.py --fix-branches # stubs moved: recheck branches between files
bash tools/docker/run.sh python tools/overlay_variants.py clone
```

`clone` matches variants without a model. It takes the parent's C, renames
the function and the symbols its assembly names differently, replaces the
numbers that differ between the two functions' instructions, and keeps
the result when the strict check says EXACT. Run it after each wave: every
newly matched parent can bring its variants along. It leaves alone a
variant whose difference has no literal in the C (a struct field) or
whose parent is in the executable.

## US map

`python3 tools/overlays.py us-map` pairs every function of the US build
(SCUS-97199) with its PAL counterpart, from the code alone, and writes
`config/overlays/us_map.tsv`: program (`boot` for the executable,
`level_NN`), US address and size, our name (the catalogue's, `func_X`
for the executable's; `-` if none), method and similarity. It reads the
US disc as ReRAC extracts it (`$RERAC/extracted`, default
`~/Projects/rerac`: `boot/SCUS_971.99` and `levels/NN/overlay.bin`,
whose seven records `formats.overlay_sections()` reads as it does ours),
plus the dump, the catalogue and `progress/report.json`. It never writes
there. The table holds addresses and names only.

1. **Cutting the US code.** Each US text (the executable's `core.text`
   and `.text`, each level's `text` record) is split by `split()` and at
   the PAL functions found in it: each PAL function's masked instructions
   are looked for near where the previous one was found, since both
   builds link their code in one order and the distance only drifts. A
   function under 64 bytes must sit exactly there or at a `split()`
   start. `split()` starts inside a found function are dropped, so US
   functions are cut as their counterparts are. Padding (zero words, the
   executable's `0xCDCDCDCD` fill) is not a function.
2. **fingerprint**: `identity()` fingerprints aligned in address order
   (difflib) with the same program's PAL functions: the report's for the
   executable, the catalogue's places in level NN for a level. A function
   left over, of 16 bytes or more, whose fingerprint is one function
   anywhere in the PAL build takes that name.
3. **aligned**: the functions left between two anchors, paired in order
   to maximise their total similarity, each pair at least 0.5 alike.
   Similarity is difflib's ratio over `mask()`ed instructions, so 1.00
   means only constants differ (a PAL timing or screen constant).
4. **similar**: what is left against any unpaired PAL function of the
   program, at least 0.8 alike and within 15% of its size. Nothing needs
   it on the current data.

On 2026-10-02 (rerac `a7efec0`):

| Program | Functions mapped | Bytes mapped |
|---|---|---|
| boot | 1,667 of 1,689 | 458,024 of 461,288 (99.3%) |
| 19 levels | 46,319 of 46,639 | 21,364,832 of 21,433,136 (99.7%) |

45,797 pairs are fingerprints and 2,189 aligned. About 17 functions per
level (3.6 KB) and 22 in the executable have no counterpart: US code the
PAL build replaced or dropped, and dead fragments such as a lone
`addiu $sp` after a return.

Checks: the levels' dispatch tables (see Roles) name each function in
both builds by its class, camera or sound id. For all 3,669 ids found in
both, the US function's mapped name is the PAL function's catalogue name
(3,430 fingerprint pairs, 239 aligned). The 4 US pointers that are not a
function start are the ones Roles lists for PAL (InitCamera_7 in levels
0, 1 and 8, UpdateMoby_324 in level 8). In the executable it agrees
with `tools/lombyte.py`'s size alignment on 969 Lombyte functions,
corrects 2 that alignment put one function off (`IsNaN`,
`JumpToRfuStatus`) and pairs 216 it could not.

`tools/lombyte.py` pairs through the map (docs/SIBLING_DECOMPS.md).
Rerun `us-map` after the catalogue changes, then `rerac-notes`.

## ReRAC notes

`python3 tools/overlays.py rerac-notes` writes
`config/overlays/rerac_notes.tsv`: every function entry of ReRAC's
`tools/ghidra/names/doc_names.csv` (ISC, "Copyright (c) 2026 ReRAC
contributors") whose US address is a function start with a counterpart
in `us_map.tsv`, with our name, ReRAC's name, its confidence, its note
(trimmed to 160 characters), its source doc and the US place. The header
records ReRAC's commit. On `a7efec0`: 795 entries on 712 of our
functions; 2 more have no PAL counterpart. Where `config/names.tsv`
already has a ReRAC name for the function (471, through Lombyte's
catalogue), the two agree; 241 functions get one only through the map.
Several identical stubs ReRAC names apart share one of our names, as the
catalogue gives every copy of the same code one name.

`tools/dossier.py` adds up to two of them to a function's `CONTEXT.md`
as "ReRAC (ISC) calls it `PathPlatformUpdate` (suggested, its
docs/plan/moby_update_catalogue.md:143): ...". They describe what the
code does; they are ReRAC's names, not recovered ones.

## Status

2026-09-28: 11 level functions matched (2,284 bytes of common level
code), from two waves of 24 Sonnet workers. The near-misses' notes are
in `build-sn/try/func_L00_*/`. Found along the way, and fixed:

- the catalogue dropped a final jump's delay-slot `nop` from 302 sizes;
- the assembler pads backward branches in stubs (now `.word`s);
- calls to a function in the same file, or to one of several identical
  copies of a helper, resolved to the wrong address in the check;
- jump tables in compiled level code were refused;
- spimdisasm left some level data unnamed, and `CONTEXT.md` gave `$gp`
  globals at 0x15F000 and up executable names.

Open: some near-misses needed per-file flags (`-G8 -mno-split-addresses`
for func_L00_00235FF8), and retail keeps a redundant `andi` in
func_L00_00286128 that our compiler drops: the level code may have been
built with other flags, which isn't mapped yet.

2026-09-28 flag sweep: the best candidate of each of the seven level
near-misses left (func_L00_00233B08, 002352D0, 00235CA0, 00236DE8,
0023B610, 0023BAB8, 0023D750) was rerun with `-G8`,
`-mno-split-addresses` and both. None matched or improved:

- `-mno-split-addresses` made four worse (00233B08 8 → 43 bytes off,
  002352D0 and 0023BAB8 change size) and left the rest as they were.
- `-G8` doesn't compile help_00232560.c, whose declarations rely on
  `-G2` placement, and changed nothing in hud_00235960.c.

So these residuals are not a file-wide flag. func_L00_00235FF8's flags
stay a per-function exception until a second function needs them.
