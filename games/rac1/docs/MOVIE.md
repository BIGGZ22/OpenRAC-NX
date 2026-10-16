# The movie code

The game's movie player (`src/game/movie/`, text `0x23B670`-`0x23E7xx`, plus a few libmpeg
helpers in `src/core/`) is Sony's EE "mpeg streaming" sample (ezmpeg) as built into the game.
The game needs it, so it has to be decompiled. What decides whether that is allowed is where
the C comes from, not the code itself.

## History

On 2026-09-17 the movie code was matched by compiling the sample's own source, taken from
leaked source releases. That is Sony's NDA code, and on 2026-09-30 (`85ecd8b`) all of it went
back to assembly, to be redone **from the retail assembly alone**.

## Rules

1. **Source of the C: the retail assembly only.** The retail binary (our disassembly, Ghidra,
   m2c) and the SDK library *binaries* the toolchain ships. No ezmpeg source, header or sample,
   and no text recalled from them. If you know the sample, write the function from the
   assembly as if you did not, and say so rather than copy from memory.
2. **Names.** Function names come only from `config/symbol_names.txt` (the `recovered` tier
   of `config/names.tsv`, from the NTSC decomp's symbols). Names from the `candidate` and
   `descriptive` tiers and from the alternatives column are not used for movie code: write
   "no recovered name" in the comment instead.
3. **Structures and fields.** Plain C structs with `pad` arrays, a field per offset the
   assembly touches. Name a field by its offset (`f48`, `x30`) or by what the assembly itself
   shows (`count`, `data`), not by what the sample's struct called it.
4. **Comments.** Say what the assembly does ("ring of 0x138C0-byte slots, oldest entry"),
   nothing taken from the sample's documentation.
5. **Provenance row.** Every movie function needs a row in `config/movie_provenance.tsv`
   (`asm only` plus the date) before `tools/apply_candidate.py` will put it into `src/`.
   Writing the row is your statement that rule 1 holds.
6. **Separate commits.** Movie functions go into commits of their own, so that all of them
   can be removed again with one revert (as in `85ecd8b`).
7. **Hardware.** DMA/IPU register addresses come from the public EE hardware documents.

## Checks that back this up

- `tools/integrate.py` refuses a candidate that mentions `ezmpeg`, `ezcontainer` or the
  sample's `UncAddr` helper, in code or comments (`tools/provenance.py`).
- `tools/apply_candidate.py` repeats that check and refuses a function in `src/game/movie/`
  that has no row in `config/movie_provenance.tsv`.
- `include/ezmpeg.h` does not exist and must not come back.

## Open question

The function names (`viBuf*`, `voBuf*`, `audioDec*`) come from the NTSC decomp's symbols and
could mirror the sample's own structure. Whether that is acceptable is for the maintainers to
decide; if it is not, rename the movie functions to neutral `func_<ADDR>` names and drop the
name comments.
