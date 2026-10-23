# Known issues

Problems found and understood but not properly fixed yet: what happens, why,
what the port does about it for now, and what the real fix needs. Remove an
entry when its proper fix lands.

## Ratchet & Clank: the moby pool fills up (Gaspar's Pilot's Helmet)

**Seen:** a player crash on 0.8.3 (PAL, Windows), 2026-10-20: Gaspar
(level 9), game mode 2, hero at 342.17 313.78 41.02, "read at game address
0x43706BB3". The Pilot's Helmet (moby class 1290) was picked up and its
cutscene started.

**Why:**
- Gaspar's falling rocks throw lava sparks (class 417) when they land in
  the lava, about 5 a frame for a while. Each spark lives about 115 frames.
- Bursts of 100 to 150 sparks fill the level's 255-slot spawn pool
  (`[D_0016001C, D_00160020)` on PAL).
- If the helmet's cutscene starts at that moment, StartCutscene →
  UpdateWorldObjectAnimation (PAL `func_00204FC0`) calls CreateMoby
  (`func_0020D348`) for the scene's actor. CreateMoby returns 0, and the
  game reads `mob->parts->used` through the null moby.

**Not a port bug:** the original game does the same. A census with fcheck's
retail interpreter on Gaspar shows retail's pool at 254 of 255, with spark
bursts of up to 136. The port's own run peaked at 255 of 255. On a console
the null read lands in low kernel memory, which is readable, so the game
most likely carries on there unnoticed. In the port it ends at an unmapped
address and crashes.

**What the port does now (0.8.4):** when the scene can't create an actor
because the pool is full, it skips that actor and logs "scene actor
skipped". The scene plays without that copy. This is a documented divergence
from the console.

**The bigger pool (2026-10-22):** `port/game/rac1/host/moby_pool.c`, on both
discs. The pool size is data, not a fixed array:

- **How the pool is set up.**
  - Each level's gameplay file gives its spare count at the moby section
    (header +0x44, second word, `n2`). The level loader `LoadLevelGameplay`
    (PAL `func_L00_002422D8`, src/overlays/shared/loaders_00240398.c; US
    load_level_gameplay.c) sizes everything from it.
  - The array is `(placed + n2) << 8`. Its pvars are `n2 << 7`, 0x80 per
    spare slot. The pool end is `base + (placed + n2 - 1) * 0x100`.
  - Everything after the array is bump-allocated from that point, so it moves
    with `n2`. Nothing sits at a fixed address.
  - Every level has 256 spare (255 usable, since the last slot takes the free
    mark) except level 17, which ships 400.
- **What the port does.** A wrap of the loader (libraries.tsv `wrap`; the
  mods' hook point sits under it) marks the header the loader names. The
  loader decompresses that header itself, so the raise happens in
  WadDecompress (`func_0020C468`, rac1/host/hand.c) as that decompression
  ends: `n2` becomes at least 768 (767 usable). A mod's run of the original
  loader sees the bigger pool too. The 0.8.4 guard stays, for a pool that
  still fills.
- **The limits checked** (code reading, and the loaders' allocations replayed
  against both discs):
  - **The level heap** runs up to 0x1FF8000 (the console's malloc arena and
    stack sit above it) with no check; in the port memory ends at 0x2000000.
    768 slots cost 192 KiB more than 256. Worst case (every placed moby kept,
    the largest ship and text), Kalebo III has 829 KiB to spare, Kerwan
    287 KiB, Gemlik 278 KiB, Veldin (final) 357 KiB and the other levels more,
    but Rilgar (5), Blarg Station (6) and Quartu (15) end within 0 to 65 KiB
    of the top. **Those three keep the disc's 255.**
  - **The moby grid** (1536 blocks of 16 ids, game_moby_grid.c): only mobys
    with collision enter it, and lava sparks (class 417) have none. Retail
    levels use about 31 to 276 blocks, and blocks are per occupied cell, so
    spawned colliders near the player cost little. Not raised. The per-cell
    count is the tighter limit: a signed byte to the collision queries, so a
    cell with 128 or more colliding mobys drops out of collision, and it wraps
    at 256 (Kalebo III's densest cell holds about 117). Open, in docs/TODO.md.
  - **The target list** (0x70000800, 512 entries, copied to the pointer
    table D_001C7C80, 512 words with its NULL): build_resident_visibility_list
    (`func_0020E6B8`) bounds neither; past 511 it writes into the next table
    and the scratchpad's group flags. Sparks don't count (no mode 0x1000), but
    placed mobys with it reach 803 on Rilgar and spawned enemies add to them.
    **The port's copy of the builder (moby_pool.c) stops the list at 511** and
    logs "moby target list full" once; the rest still run.
  - **The renderer** draws from vectors; the only fixed cap is the port's
    4096-moby guard in render_native.c's mark_drawn_mobys. The game's own VU1
    packet buffers belong to the console renderer, which the port stubs.
  - **Indices** stay far below 15 bits (at most about 2,300 slots).
- **Rejected approaches:** moving the array after load (placed mobys, groups
  and the grid hold its addresses and indices), and a second overflow pool
  (`(m - base) >> 8` indices break outside the array).
- **The level packer:** `editor/gameplay.py` keeps the spare count in the
  header, so it could raise it for mods too.
- **Left:** Rilgar, Blarg Station and Quartu still have 255 slots, so a heavy
  spawner there can still fill the pool and reach the guard. A full pool at a
  CreateMoby whose caller doesn't check crashes the same way. Crash reports
  name the moby being updated, which helps spot them. docs/TODO.md has the
  rest.
- **Behaviour:** more effects alive at once than on a PS2
  (differences.json, `bigger-moby-pool`).

Evidence and scripts: the port census (`OPENRAC_TMP_CENSUS`, a temporary
patch to `port/game/common/frontend.cpp`, not kept) and the fcheck census of
snapshots at SpawnFallingDebrisRing (0x2EBA58), frames 1390 to 1848.
