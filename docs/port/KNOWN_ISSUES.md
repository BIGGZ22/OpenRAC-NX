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

**The proper fix (not done): a bigger pool.**
- The spawn array is a fixed range the level's data sets up, with other data
  right after it.
- Enlarging it means moving it to free guest memory at level load and
  updating its bounds.
- Every structure sized to it must move too: per-moby variable (pvar) pools,
  side tables, code that turns a moby pointer into a byte or short index, and
  fixed-size lists.
- It changes behaviour slightly: more sparks alive at once than on a PS2.
- An investigation of what it entails is under way (2026-10-20); its findings
  go here.
- Other busy levels may hit the same limit. A full pool anywhere, at a
  CreateMoby whose caller doesn't check, crashes the same way. Crash reports
  now name the moby being updated, which helps spot them.

Evidence and scripts: the port census (`OPENRAC_TMP_CENSUS`, a temporary
patch to `port/game/common/frontend.cpp`, not kept) and the fcheck census of
snapshots at SpawnFallingDebrisRing (0x2EBA58), frames 1390 to 1848.
