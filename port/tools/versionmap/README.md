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
