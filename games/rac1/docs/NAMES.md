# Names

Every symbol keeps its address name: `func_<ADDR>` in the executable,
`func_LNN_<ADDR>` in level code, `D_<ADDR>` / `D_LNN_<ADDR>` for data.
The build and every tool read the address (and level) out of that name
(DECOMP_PROGRESS.md, "Real function names"), so it is what C declares and
defines, what `INCLUDE_ASM` names and what the linker sees.

Readable names sit on top, as macros:

| File | What it is |
|---|---|
| `config/names.tsv` | the table: one row per named symbol, its name, tier, source and the evidence that put the name on this PAL address |
| `include/names.h` | generated from the table: `#define FadeToBlack func_001F4E08`, for every name not a candidate; included by `common.h` |
| `tools/names.py` | `build` (sources to table), `header` (table to names.h), `apply` (bodies use the names), `check`, and lookup: `python3 tools/names.py func_001F4E08` or `... FadeToBlack` |

C writes the readable name inside function bodies and the address name
everywhere else (declarations, definitions, `__asm__` labels, comments that
cite a symbol). The preprocessor turns one into the other, so the objects
are byte for byte the same either way: the change that introduced this
was verified by hashing every compiler output and `rac1.elf` before and
after. `tools/dossier.py` shows a function's name in its `CONTEXT.md`.

Core/SDK/libc functions (`src/core`) are in the table but not in the
header: their names (`memcpy`, `sceGsSync`, `rand` ...) are the libraries'
own, and a macro for them would shadow the real library spelling.

## Tiers

| Tier | In names.h | Meaning |
|---|---|---|
| recovered | yes | the original identifier: `config/symbol_names.txt`; the NTSC decomp's `symbols.txt` as it stood before its 2026-09 automated naming loop; a symbol Lombyte recovered |
| descriptive | yes | a later name with evidence: the NTSC decomp's 2026-09 names, a Lombyte proposal of high confidence, a ReRAC name marked verified, the PAL memory map (globals) |
| candidate | no | weaker: Lombyte medium confidence, ReRAC suggested or inferred, or a name another symbol already took. Shown in dossiers for workers |

When sources disagree, the best tier wins, then the source order above;
the other names are kept in the `alternatives` column. A name is given to
one symbol only.

## Sources (all RaC1)

- **NTSC decomp**: matching NTSC
  decompilation; `config/symbols.txt`, mangled GCC 2.x names demangled to
  the base name (`Class_method` for members).
- **Lombyte** (github.com/mateuszklysz/Lombyte, MIT): matching US
  decompilation; `config/us/recovered_names.json` and its report.
- **ReRAC** (github.com/re-rac/rerac, ISC): native PC port;
  `tools/ghidra/names/doc_names.csv`, the names its design docs give to
  US boot and level functions after reading them in Ghidra.
- **PAL memory map**: the community spreadsheet "Ratchet & Clank Series
  Addresses" (RaC1 sheet, PS2 PAL column). Only addresses that are the
  exact start of one of our `D_` symbols, with accesses of the described
  size, are used (the list is `MEMORY_MAP` in `tools/names.py`).

## From US addresses to PAL symbols

- **Executable:** function-size alignment of the US executable (Lombyte's
  report) against ours, runs of four or more equal sizes, as
  `tools/lombyte.py` does. Lombyte's report gives ROM offsets, converted
  with its splat segment's `start`/`vram`.
- **Level code:** Lombyte's US overlay catalogue
  (`config/overlays/us/functions.tsv`) fingerprints functions exactly as
  `tools/overlays.py` does. 3,828 of our 4,040 fingerprints occur once in
  each catalogue; with the same size, that is the same function, and its
  US places map to our catalogue name. The same pairs cover the
  executable functions the levels hold (`exe` kind).
- **Cross-checks:** where alignment and fingerprints both place an
  executable function they must agree (they do everywhere; a
  disagreement would drop both). Of the functions `symbol_names.txt`
  already named, the NTSC decomp's and Lombyte's names placed by the alignment agree
  209 of 209.

## Refreshing

```sh
NTSC=... LOMBYTE=... RERAC=... python3 tools/names.py build
python3 tools/names.py header
python3 tools/names.py apply      # new C that still spells address names
python3 tools/names.py check
```

`build` needs full clones (the NTSC decomp's history dates its names). Then rebuild
and check that nothing but source text changed.
