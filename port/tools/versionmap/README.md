# versionmap

Two versions of one game are mostly the same code at other addresses. What
the port of one version knows by address (which functions are libraries,
which the port answers itself, where the level is kept) can be carried to
the other instead of being found again. These two tools do that; rac1-ntsc
was started from rac1-pal with them
([port/game/rac1-ntsc](../../game/rac1-ntsc/README.md)).

```sh
# The map: every function and data address of FROM with its place in TO.
python3 port/tools/versionmap/versionmap.py build rac1/pal rac1/ntsc \
    --out build/versionmap/rac1-pal--rac1-ntsc.json \
    --entries port/game/rac1-pal/libraries.tsv --entries port/game/rac1-pal/host/hand.c
python3 port/tools/versionmap/versionmap.py lookup build/versionmap/rac1-pal--rac1-ntsc.json 0012DB18

# Carrying: a list of names, a libraries.tsv, C.
python3 port/tools/versionmap/carry.py table MAP GEN port/game/rac1-pal/libraries.tsv
python3 port/tools/versionmap/carry.py c MAP GEN port/game/rac1-pal/host/vector.c
python3 port/tools/versionmap/carry.py c MAP GEN --keep-spelling --program level:00 FILE.c
```

`versionmap.py` reads the code your own discs give after each game's setup,
the files [tools/xmap.py](../../../tools/xmap.py) scans
(`games/<game>/game.json`, "xmap"); `OPENRAC_CODE_ROOT` names the checkout
that holds them when this one does not. The map it writes holds addresses
and kinds, never code, and stays in `build/` (ignored).

| Kind | A function of FROM is paired with one of TO because |
|---|---|
| `same` | the two are the same instructions; only address fields differ. C for one is C for the other |
| `near` | they sit between the same unchanged neighbours, but differ: the function changed between the versions. C for one is a draft for the other |
| `called` | code both versions share calls them at the same instruction |
| `found` | TO has FROM's instructions at that address in exactly one place (an entry inside a function; asked for with `--entries`) |

| Kind | A data address of FROM is in TO |
|---|---|
| `exact` | where the same instruction of a `same` function forms it |
| `between` | by the move of its neighbours on both sides, which agree |
| `guess` | by the move of the neighbour below it, when the one above moved differently |

`carry.py` writes FROM's `func_` names, `D_` names and address literals as
TO's. GEN is TO's generated folder (`port/build/<preset>/games/<id>/gen`):
its `functions.c` says what TO's decompilation calls each address;
`--source` and `--places` add the functions that decompilation only has as
assembly stubs. Everything that does not rest on `same` or `exact` is
listed on stderr. An address literal is a guess by nature (a size or a
colour in the same range reads as an address): read what it changed.

Nothing carried is proven by being carried.

## Carrying functions

`carry_functions.py` does the above for every function the target still has
only as assembly: it pairs each with the source's through the map (back,
`--back-map`, TO to FROM), takes the C the source's port runs for a `same`
pair (matched C, the port's hand C, or the candidate its functional table
names), preprocesses it in its own file, cuts it down to the function and the
declarations, types and static helpers it uses (its own types and helpers
renamed apart, with a suffix from the target's address), carries it with
`carry_c`, and checks it with Clang where hostgen will put it. `--write` puts
it in the target's `hand/functional/<name>.c` with a row in `hand/hand.tsv`,
and drops the rows that no longer replace a stub (and their files).

```sh
python3 port/tools/versionmap/carry_functions.py --target port/game/rac1-ntsc/hostgen.json \
    --source port/game/rac1-pal/hostgen.json \
    --to-map build/versionmap/rac1-pal--rac1-ntsc.json --back-map build/versionmap/rac1-ntsc--rac1-pal.json \
    --gen port/build/release/games/rac1-ntsc/gen --source-gen port/build/release/games/rac1-pal/gen \
    --out build/carry/rac1-ntsc [--write]
```

What it does not carry it lists in `--out`: `near.json` (the function changed
between the versions; the source's C is a draft), `write.json` (no
counterpart, or one with no C: write it from the assembly), `skipped.json`
(remnants, handwritten routines, names with no function or no stub behind
them, C that does not translate, each with the reason) and `covered.json`
(names hostgen reports without C whose address has C under another name).
Its tests: `python3 -m unittest port/tools/versionmap/test_carry_functions.py`
(they need Clang).

## The shared host C's tables

rac1's host C is one copy for both discs ([port/game/rac1](../../game/rac1/README.md)),
written with games/rac1's names and addresses; each other version has a
table of its own names and addresses for it (`host/rac1_version.h`).
`hostnames.py` keeps that table in step:

```sh
python3 port/tools/versionmap/hostnames.py check port/game/rac1-ntsc      # no disc needed
python3 port/tools/versionmap/hostnames.py check port/game/rac1-ntsc \
    --map build/versionmap/rac1-pal--rac1-ntsc.json --gen port/build/release/games/rac1-ntsc/gen
python3 port/tools/versionmap/hostnames.py write port/game/rac1-ntsc --map ... --gen ...
```

`check` lists what the table lacks, what it has that nothing uses, and
address literals left outside `RAC1_ADDR`; with the map, every entry the map
does not agree with (an entry is never changed for it: the reason goes in a
comment at the entry, above the generated part). `write` drops unused
entries and adds new ones from the map. Its tests: `python3 -m unittest
discover -s port/tools/versionmap`.
