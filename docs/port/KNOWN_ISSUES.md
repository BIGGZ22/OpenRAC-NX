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

**The proper fix (not done): a bigger pool.** Investigated 2026-10-20 (code
reading only). The pool size is data, not a fixed array:

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
    mark) except level 17, which ships 400. So larger values already work in
    retail data.
- **The recommended fix:** a port wrap on the load step that raises `n2` to,
  say, 512 or 768 after the header is in memory and before the loader reads
  it, on both discs. The array, pvars, pool end and later allocations all
  follow. Indices stay within the 15-bit limits.
  - Effort: about half a day to a day.
  - Keep the 0.8.4 guard as the stopgap for a pool that still fills.
- **Check before choosing the number:**
  - the level heap's headroom on the heaviest levels (16, 5 and 3 have the
    most placed mobys: 1569, 1431 and 1029);
  - the moby grid's 1536 shared blocks of 16 ids (game_moby_grid.c; it traps
    when they run out);
  - the 512-entry target list in the scratchpad;
  - any fixed cap in the native renderer or the VU packet buffers on how many
    mobys it draws (not checked).
- **Rejected approaches:** moving the array after load (placed mobys, groups
  and the grid hold its addresses and indices), and a second overflow pool
  (`(m - base) >> 8` indices break outside the array).
- **The level packer:** `editor/gameplay.py` keeps the spare count in the
  header, so it could raise it for mods too.
- **Other levels:** every level has the same capacity, so any level with a
  heavy spawner (sparks, bolt bursts, debris) can fill it. A full pool at a
  CreateMoby whose caller doesn't check crashes the same way. Crash reports
  now name the moby being updated, which helps spot them.
- **Behaviour:** a bigger pool means more effects alive at once than on a
  PS2.

Evidence and scripts: the port census (`OPENRAC_TMP_CENSUS`, a temporary
patch to `port/game/common/frontend.cpp`, not kept) and the fcheck census of
snapshots at SpawnFallingDebrisRing (0x2EBA58), frames 1390 to 1848.
