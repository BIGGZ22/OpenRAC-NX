# Code shared between the games

How much machine code the five versions have in common, measured from the
discs, and what each project can take from the others today. The tables are
generated: [shared/xmap/README.md](../../shared/xmap/README.md). This page
explains the method, reads the first results (2026-10-04) and says what to do
with them.

## What was measured

[tools/xmap.py](../../tools/xmap.py) reads every program a version has: the
boot executable's `core.text`, `.text` and `net.text`, RAC3's `frontbin.elf`,
and each level's code (19 in RAC1, 27 in RAC2, 51 in RAC3, 47 in Deadlocked).
It cuts each code section into functions the same way for every game, and a
function that repeats in several levels counts once. Each function gets two
fingerprints ([tools/mips.py](../../tools/mips.py)):

- **strict**: only address fields are masked (jump targets, the halves of
  RAM addresses, `$gp` offsets). Two functions share it only when they are the
  same function at another address.
- **shape**: constants and struct offsets are masked too. Two functions that
  differ only in a number share it; they are relatives, not copies.

It also fingerprints the functions as each project itself cuts and lists
them, with which ones the project has matched in C, so "matched there, open
here" can be answered whatever the two projects call a function.

The rules are the ones that built rac1/pal's US to PAL map
([OVERLAYS.md](../../games/rac1/pal/docs/OVERLAYS.md)). The strict figure is a
floor: that map pairs 84% of the US code by identical fingerprint and most of
the rest by similarity between those anchors.

For the resident programs (boot executable and frontend) the tool takes that
step too: between two functions that are unchanged and in the same order in
both versions, it pairs the functions in between when their instructions are
at least 80% alike. Level code is not paired this way yet.

## Results

**How much distinct code there is.** Far less than the totals the projects
count, because level code repeats: RAC2's 27 overlays hold 5.05 MB of
distinct code against the 48.8 MB its progress scope sums.

| Version | Distinct functions | Distinct code |
|---|---:|---:|
| rac1/pal | 4,827 | 3.66 MB |
| rac1/ntsc | 4,827 | 3.65 MB |
| rac2/ntsc | 8,056 | 5.05 MB |
| rac3/ntsc | 14,436 | 7.02 MB |
| rac4/ntsc | 15,235 | 5.39 MB |

**What is shared.**

- *RAC1's two regions*: 75.6% of the bytes are identical functions, and 98%
  of the core. The rest differs in more than addresses, which is why the PAL
  project pairs it by similarity.
- *The engine core carries over; the game does not.* RAC1 and RAC2 share 70%
  of RAC1's `core.text` (81 KB) but 8% of the game code and almost no level
  code. RAC3 and Deadlocked share 69% of RAC3's core (106 KB) and 567 KB in
  all, 8 to 10% of each, the largest overlap between two games. RAC1 shares
  27% of its core with RAC3 and with Deadlocked.
- *In every version*: 319 functions, 38 KB, of which 30 KB is in the core,
  at the addresses of Sony's libraries and the C runtime. That is the code a
  shared library tree would hold first.
- *Level code is each game's own*: under 1% is identical between two games,
  except RAC3 and Deadlocked (6 to 10%).
- *Changed a little*: in the boot executables, another 50 KB of RAC1's
  functions reappear in RAC2 at least 80% alike, 34 KB of RAC2's in RAC3, and
  24 KB of RAC3's in Deadlocked.

**What each project can take from the others** (2026-10-07). 4,750 distinct
functions (3.1 MB) are matched in at least one project. Matched elsewhere,
identical in this version's code, and still open here:

| Version | Functions | Bytes | Compared with what the project has matched |
|---|---:|---:|---|
| rac1/pal | 133 | 56,904 | +2.7% |
| rac1/ntsc | 185 | 36,624 | +1.7% |
| rac2/ntsc | 480 | 75,020 | 2.8 times as much |
| rac3/ntsc | 222 | 28,816 | +11% |
| rac4/ntsc | 324 | 44,000 | 1.7 times as much |

On 2026-10-04 rac1/pal's row was 245 functions and 125,876 bytes, until 114
of them were carried over from Lombyte (below); Lombyte has since matched
more, and 117 of its functions (55,936 bytes) are identical in PAL and open
there. Most of RAC2's come from the two RAC1 projects (rac1/pal alone
offers 327 functions, 55 KB). The lists, one per
pair, are in [shared/xmap/ports/](../../shared/xmap/ports): source name and
address, target program and address, size, and whether it is the same function
or a relative.

## Using a port candidate

A candidate marked `same` is the same machine code in both games, so the C
that produces it in one game produces it in the other **when built the same
way**. That last part is the catch: the projects do not agree on compilers
([docs/toolchains](../toolchains/README.md)). So:

1. Take the C from the project that matched it, with its license and a
   provenance note, as the projects already do
   ([workflow](../workflow/README.md#6-porting-between-versions-and-games)).
   RAC3's code, and rac1/pal's since 2026-10-07, is GPL v3; it can only move
   into an MIT tree with its authors' agreement ([LICENSE.md](../../LICENSE.md)).
2. Rename its symbols to the target's (addresses differ, and each project
   names by its own).
3. Build it with the target project's own tools and pass that project's own
   check. If it does not match there, the two projects compile that code
   differently: that is evidence for the compiler question, worth writing
   down, not a failed port.

A `shape` candidate needs its constants and offsets adjusted; treat it as a
strong draft. A `similar` one changed by a few instructions: a draft to adapt,
with the ratio saying how close.

## Porting by machine

For a `same` candidate, steps 1 to 3 need no judgement, so
[tools/port.py](../../tools/port.py) does them:

```sh
python3 tools/port.py rac1/ntsc rac1/pal           # candidates in build/port/rac1-ntsc--rac1-pal/
python3 tools/port.py rac1/ntsc rac1/pal --check   # ... and the target project's verdict on each
```

It cuts the function and the declarations it needs out of the source
project's file, renames every symbol, and writes a candidate with a credit
line. `--check` hands each candidate to the target project's own check, reads
what the compiler says about names the target's file already uses, and tries
again. What passes is listed for the target project to land its own way;
nothing is written into a game's `src/`.

**How it knows the target's names.** It does not look them up. The two
functions are the same instructions, so the n-th address the source version's
code forms (a call, a `lui` with its `%lo`, a `$gp` offset) is the n-th the
target's code forms ([tools/mips.py](../../tools/mips.py), `references`). A
symbol of the source C has an address in the source version; the references
that reach it give its address in the target; the target names that address
by its own rules. This works for any global, named or not, in any level.

**How it knows the target's declarations.** The projects compile with
different compilers and declare a global differently to get the same access.
The tool reads the access from the target's code instead of translating the
declaration: a global reached only through `$gp` is small data, one whose
every `lui` access has the assembler's own shape (`lui $2` / `lw $2,%lo($2)`,
or through `$at`) is `MACRO_ADDR`, a one- or two-byte global reached with
`lui` is kept out of small data. For rac1/pal these three rules took the pass
rate from 39 of 102 candidates to 102 of 140.

**Measured: Lombyte to rac1/pal, 2026-10-04.** Of 228 functions Lombyte has
matched whose code is identical in PAL and open there:

| Outcome | Functions | Bytes |
|---|---:|---:|
| Landed in rac1/pal | 114 | 80,252 |
| Close, not exact: the two compilers schedule or allocate an instruction or two differently, or the size differs | 37 | 25,408 |
| Uses the source project's inline `sq $0` helper, which rac1/pal has no form for | 17 | 6,416 |
| Never taken: the movie code ([shared/port/](../../shared/port/README.md)) | 56 | 9,896 |
| Other (no listed function at the target address, an ambiguous name) | 4 | 2,916 |

Of the 114, 100 passed rac1/pal's checks exactly as generated. Seven more are
library functions that were first held back and then reviewed
([shared/port/README.md](../../shared/port/README.md)), and seven needed a
prototype that an earlier caller in the same file had guessed put right by
hand. 92 are level functions, each exact under rac1/pal's strict link-time
check and again with every other C function of its file; 22 are in the
executable, confirmed by the full build (1,043 exact, none with a wrong
size). No model wrote or adjusted any of them.

One of the library functions showed why the full build is the proof and the
quick check is not: `sceLseek` passed the masked comparison with one wrong
address and was 3 bytes off in the linked image. The cause was in the tool
(dead padding after a jump hid a reference) and is fixed.

So the claim above holds, with its caveat measured: identical machine code
came from identical C in about three cases out of four once declarations
follow the target's rules, and in the rest the two compilers differ by an
instruction or two. Those near misses are the cheapest matches left in
rac1/pal, and each is a data point on which compiler built the game.

**Measured the other way: rac1/pal to Lombyte, 2026-10-04.** The same tool
with Lombyte's rules (every symbol under a private name with an assembler
label, small data marked `__attribute__((sda))`), checked by Lombyte's own
`check-unit.py`:

| Outcome | Functions | Bytes |
|---|---:|---:|
| Pass as generated | 14 | 11,452 |
| Same size, a few bytes differ | 13 | 10,284 |
| Another size | 16 | 8,888 |
| The place is a second copy of a function Lombyte lists once | 37 | 7,372 |
| In the executable, where this check does not reach yet | 36 | 6,508 |
| Other | 10 | 7,088 |

With the 14 applied, Lombyte's `make overlays` reports all 1,554 overlay
functions matching (1,540 before). They are not applied here, since
`games/rac1/ntsc` is a copy that follows Lombyte's repository; `--check`
leaves them as a patch beside its results
(`games/rac1/ntsc/build/port/rac1-pal--rac1-ntsc/exact.patch`). Since
rac1-decomp moved to the GPL on 2026-10-07, C written there after that date
can go to Lombyte (MIT) only with its authors' agreement.

Fewer pass in this direction, for two reasons worth knowing. Lombyte's
catalogue gives one name to functions that differ only in a constant, so 37
of the functions rac1/pal matched have no place of their own there. And the
PAL C was shaped to PAL's compiler: where it leans on that compiler's
scheduling, Lombyte's compiler orders an instruction or two differently.

**Adding a pair.** A version's side is the `port` entry of its `game.json`
(the keys are listed at the top of `port.py`): where its sources and catalogue
are, its `$gp`, how it names an address and declares a global, how its check
is run, and what it never takes. rac1's two projects are described both as
source and as target. Another game needs its entry, and a decision first if
its rules restrict what may come along
([open question 6](../policy/OPEN_QUESTIONS.md#6-code-shared-between-the-games)).

Byte-identical library code deserves a note: where a function comes from a
prebuilt archive (Sony's libraries, libgcc), every game that linked the same
archive has the same bytes, whatever compiled the game itself. Those ports
depend least on the compiler question.

## Running it

`scan` needs each game's code on disk, which each game's setup produces
(nothing is committed; `build/xmap/` is ignored):

| Version | What it reads | Produced by |
|---|---|---|
| rac1/pal | `games/rac1/pal/baserom/SCES_509.16`, `baserom/overlays/level_*` | `openrac.py setup rac1/pal`, then `python3 tools/overlays.py dump` there |
| rac1/ntsc | `games/rac1/ntsc/config/us/SCUS_971.99`, `config/us/overlays/level_*` | `openrac.py setup rac1/ntsc`, then its `setup.sh` |
| rac2/ntsc | `games/rac2/ntsc/baserom/SCUS_972.68`, `baserom/levels/*_*` | `python3 tools/levels.py dump` there |
| rac3/ntsc | `build/rac3/wrench_out/uya_scus_973_53/` | the Wrench unpack on [rac3's page](../../games/rac3/README.md#getting-started) |
| rac4/ntsc | `games/rac4/ntsc/baserom/SCUS_974.65.elf`, `build/rac4/wrench_out/levels/` | `bash tools/setup_asm.sh` there, and `wrenchbuild unpack ... -o build/rac4/wrench_out -g dl -r us` |

```sh
python3 tools/xmap.py scan                       # all five take about five seconds
python3 tools/xmap.py report                     # shared/xmap/summary.json and README.md
python3 tools/xmap.py ports rac1/ntsc rac1/pal   # one pair's candidates
```

Rerun after a project lands matches or changes how it lists functions, and
commit `shared/xmap/` as `chore(shared)`.

## Limits

- **A floor, not the whole overlap.** Functions that changed a little are
  paired only in the resident programs, only between anchors in the same
  order, and only in runs short enough to compare; level code and reordered
  code are not.
- **Cuts differ.** A function one project cuts in two is not found whole in
  another. Each version is indexed both ways (uniformly, and as its project
  cuts it), which recovers most cases, not all.
- **"Matched" means what each project says.** RAC2 lists only the functions
  it has matched, so nothing is known about the rest of its list. RAC3's 33
  common level functions are not read yet. Deadlocked's matches rest on a
  looser check ([games/rac4](../../games/rac4/README.md#what-matched-means-here)).
- **Small functions prove little.** Fingerprints under 16 bytes are ignored,
  shapes under 48.

## Files, not only functions

The same comparison on the source trees finds 16 files that two games build
with unchanged: GCC's libgcc sources and build helpers in rac1/pal and rac4,
and label macros in rac3 and rac4. [shared/files.json](../../shared/files.json)
lists them, and `python3 tools/shared.py check` keeps their copies identical
([shared/README.md](../../shared/README.md)).

## What comes next

In order ([open question 6](../policy/OPEN_QUESTIONS.md#6-code-shared-between-the-games)):
port the candidates above, each proven in its target (by machine where the
code is the same, starting with the pairs `port.py` does not describe yet);
extend the similarity step to level code; then give the library code that is in every version one
home that every game's build compiles and checks. That last step needs each
game's per-object compile recipe as a shared tool first: a library file
matches only when built exactly as the game built it.

## One source tree for several versions

`tools/versions.py check FROM TO --build CHECKOUT` asks whether FROM's
sources could build TO as well. It takes FROM's compiled objects (from a
full build of its project), places each matched boot function at the
address `xmap.py` pairs it with in TO, and checks that it comes out as TO's
retail bytes: every instruction equal except the fields relocations fill,
and every symbol needing a single value across all the functions placed.
The values found are TO's address map, functions and data alike, written to
`build/versions/FROM--TO.tsv`.

First result, 2026-10-15, rac1/pal to rac1/ntsc: of 1,030 matched boot
functions, 919 pair one to one, and 898 of those reproduce NTSC-U's bytes
exactly with one consistent map of 1,765 symbols (865 of them data). The
rest are 14 real instruction differences, 4 relocation pairings the checker
does not read yet and 3 conflicts in one object. Level programs are not
checked yet. This is the evidence for building PAL and NTSC-U from one tree
with a per-version address map rather than keeping two source trees.
