# Levels

## The programs

The disc holds the boot executable (`SCUS_972.68`) and 27 levels, each with its own program.
Loading a level copies its program over the boot's memory from `0x1A7C80`: every level
carries its own copy of much of the engine, at addresses of its own, alongside its own code.
Everything below `0x1A7C80` (the boot's resident code and data) is the same in every program,
and all of them share `$gp = 0x1AEFF0`.

`tools/levels.py dump` reads them with its own ISO9660 and archive reader:

- A level's archive is `G/LEVELn.WAD`; the level's number is the archive's own word at
  `+0x08`, which is the file's number except for `G/LEVEL21.WAD`, level 30 (Insomniac Museum).
- The archive header's word pair at `+0x10` is the level data (sector offset, sector count).
- The level data opens with twelve (offset, size) slots; slot 0 is the program: a list of
  sections, each a 16-byte record (destination, size, type, entry point) followed by its
  bytes, a NOBITS section's included. Every section of one program repeats the entry
  point, and the list ends where it changes.
- Seven sections per level: `.lit` (always at `0x1A7C80`), `.bss`, `.data`, the three
  vtables (`vtbl`, `camvtbl`, `sndvtbl`) and `.text`.

The dump is checked against the boot's pinned hash, every level's entry point and the raw
hash of every function extent of the earlier catalogues (106,337 extents, all equal).

## The catalogue

`config/boot/functions.tsv` lists the boot's 3,390 functions (name, address, size).
`config/levels/functions.tsv` lists every distinct function of the levels once, with all its
places:

    name  kind  size  fingerprint  levels  places (level:address, ...)

- **kind**: `boot` (the boot has it too, and its name is the boot's), `shared` (several
  levels) or `level` (one).
- **name**: the boot's name, or `func_LNN_XXXXXXXX`, its address in the lowest-numbered
  level that has it.

Two places are one function when their instructions are equal apart from what linking
changes: jump targets and address halves are masked, except those pointing below
`0x1A7C80`, which every program shares; constants, stack slots and field offsets on
pointer arguments are compared. Two places in one program are always two functions (the
k-th copy of identical code in each level, by address, is one function), the same
instructions with different padding are two functions, and a level function takes a boot
function's name only when the pairing is unambiguous or recorded in
`config/levels/boot_pairs.tsv`.

The 27 programs hold 106,337 placements (47.4 MB); they are 7,765 distinct functions
(5.3 MB), 2,055 of them copies of boot functions. With the boot, the game is about 9,100
distinct functions, 5.5 MB of code.

An inferred function extent can start inside the preceding function's epilogue.
`tools/levels.py` corrects three measured Aranos entries before fingerprinting:
`00354DE8` to `00354DF8`, `003D54A8` to `003D54B0`, and `0040EA98` to `0040EAA0`.
Retail data and virtual-table pointers reference the corrected entries; the original
end addresses stay unchanged, and the skipped prefixes remain assembly in the build.
Correcting the second entry also groups it with Siberius's `003EBC90`, so the two
places share one definition. These corrections apply to level 00 only; regenerate
the catalogue with `tools/levels.py catalogue`, rather than editing its rows.

## The C

The layout is rac1-decomp's: every function of both catalogues appears once in `src/`, in
address order, as its C or as an `INCLUDE_ASM("...", name);` marker (`include/include_asm.h`;
here the marker is empty, since the build takes a function without C from the disassembly).

- `src/core/<ADDR>.c`: the boot's `core.text` (SDK, libc), one file per original object, as the
  linker's `0xCDCDCDCD` fill between objects shows.
- `src/game/<ADDR>.c`: the boot's `.text`.
- `src/levels/shared/LNN_<ADDR>.c` and `src/levels/NN_name/LNN_<ADDR>.c`: level functions in
  several levels and in one, in their canonical level's order.

A file is compiled one way (`config/cflags.txt`), so a file ends where the next C needs another
compiler profile. Names carried from rac1-decomp with no rac2 counterpart yet keep an `r1_`
prefix: they state no address, and the build reads their values.

A function's name says where it goes; `tools/build.py` links it at every place the
catalogue gives it. What it refers to is named by what it is: other functions by their
catalogue names, the boot's resident data `D_XXXXXXXX`, a level's data `D_LNN_XXXXXXXX`
(its address in level NN), the k-th external a function uses with no address of its own
`ref_LNN_XXXXXXXX_kk`. The value each symbol has in each program is read from the retail
bytes under the relocations of the placed functions; uses must agree, and a name that
states an address must have it (a boot address inside a level's memory is not checked
there: the level has its own copy).
