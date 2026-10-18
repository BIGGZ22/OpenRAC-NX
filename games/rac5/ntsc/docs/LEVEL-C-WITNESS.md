# Shared level and frontend C witnesses

`func_00000100` is an independently authored 8-byte C body returning `1`.
The same two retail instructions occupy module offset `0x100` in all twenty
numbered USA `LEVEL_*.REL` files. The source has **one definition** in
[`src/levels/shared/func_00000100.c`](../src/levels/shared/func_00000100.c).
Its name describes the measured offset; its game-level purpose is unknown.

`func_shared_zero_buffer` is a second independently authored, 72-byte C body.
It reads two level-local pointer records and calls the imported `memset` to
clear a rounded buffer length. Its module offset differs among the twenty
levels, so the [trial profile](../config/levels/compiler_trial.json) pins each
placement. The two addressed data objects also move independently. The gate
resolves their local HI16/LO16 fixups at each placement and requires their
64-byte data windows to be identical across levels and unique within each
module. This verifies the particular data references used by this function;
it does not establish semantic identity for other masked families.

The narrowly scoped [level compiler trial](../config/levels/compiler_trial.json)
reuses the hash-pinned RAC5 GNU EE tools and `-O2 -G0` flags. It accepts these
two shared functions and one frontend function. Two independent forms serve as
uncounted controls:
an indexed global-array read at `LEVEL_01.REL:0x190` (24 bytes) and a global
store at `FRONTEND.REL:0x2CD8` (12 bytes). Their C remains under
`nonmatching/levels/`. Matching these short forms does not identify the
original compiler or qualify arbitrary level functions.

`tools/level_c_gate.py` recompiles all five sources on a native Linux
filesystem, requires pinned object hashes, links each object at its measured
module offset with the pinned SN linker, and compares every body byte. At all
twenty level placements it checks that offset `0x100` is a direct call target,
the adjacent `0x108` is separately called, no **direct** jump or branch enters
the body interior, and no named or packed relocation touches its eight bytes.
For the 72-byte body, it also checks the imported `memset` relocation, the
four local data fixups, the return boundary and direct control-flow entries.
It links the same C object separately at each measured module address using
the corresponding data-symbol addresses. It then replaces both C bodies in
each complete private SNR2 module and requires
all **20/20 complete module SHA-256 values** to match the pinned disc. The
two control functions also preserve their complete frontend/level 01 files,
and the previously measured relocated-image hashes at those two bases.
`func_00002F80` is an independently authored **476-byte** frontend New Game
initializer at module offset `0x2F80`. The [Archipelago Size Matters
research](https://github.com/Panguu/Archipelago-RaCSM/blob/SM-Main/worlds/rac_size_matters/core/patches/starting_planet.py)
identified that entry and its first three instructions; they match the pinned
USA module. A separate Ghidra boundary pass proposed the complete contiguous
body, and the pinned frontend has four direct callers with no direct jump or
branch from outside into its interior. The source keeps offsets where field
names and structure sizes are not yet proven; no Archipelago code or runtime
patch is imported. The gate checks the compiled symbol's complete 476-byte
extent, the six named imports, local fixups, complete FRONTEND.REL file and the
previously measured relocated-image hash. This qualifies that **one frontend
placement**, not a general frontend compiler profile or gameplay equivalence.

The rest of each module is supplied by the pinned retail input. No game bytes,
compiler binaries or linked outputs are committed.

With your own lawful pinned USA disc and matching local tools, run in WSL:

```sh
python3 tools/level_c_gate.py baserom/SCUS_976.15.iso \
  --boot baserom/SCUS_976.15 \
  --cpp /path/to/cpp --cc1 /path/to/cc1 --as /path/to/as \
  --linker /mnt/c/path/to/ld.exe \
  --output-dir /home/you/rac5-level-c-trial
```

The ignored `build/level-c-gate/report.json` starts as `incomplete` with zero
matches on every run. Only a full pass reports three accepted functions:
**556 unique C body bytes** (8 + 72 shared, 476 frontend) and **2,076
placement bytes** (1,600 across twenty levels, 476 in frontend). These are
scoped matches, not a qualified game-wide percentage. More functions require
their own boundaries, compiler controls and complete-program checks before
their C bytes can be counted.
