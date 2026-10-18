# Mobys

A moby is any game object: Ratchet's model, enemies, bolts, crates,
gadgets, projectiles, platforms, effects, invisible level logic. Field
tables: [data-structures.md](data-structures.md#moby-includemobyh) (use
`include/moby.h`, not `MobyInstance`/`PVARS.md` offsets). Slot reuse
delay, update order and the pre/post-pass flag tests are observable
in-game and must be kept.

## Identity

- `pclass` -> `struct MobyClass`, the class record (`D_001B3580[slot]`):
  model, collision, default scale/flags, `seqs[]` (+0x48), `callbacks`
  (+0x2C).
- `oclass` (s16, 0xA6): RaC1's class number, as in debug strings ("Moby
  28(Footsoldier)") and `UpdateMoby_<oClass>` roles.
- `class_slot` (u8, 0x22): per-level registration slot.
  `D_001B3E40[oClass]` -> slot (0xFF = not loaded); `D_001B3C80[slot]` ->
  oClass.
- Per-class pvars layouts in `include/moby_pvars.h` come from outside
  tooling and are unverified; check each against the asm.

## Memory: the moby array and pvars

(`src/game/mobyfunc.c:141-200`)

- One array of 0x100-byte slots. `CreateMoby` searches `D_0016001C` ..
  `D_00160020`; `D_00160018` is the slot-index base (`InitMobyInstance`
  stores `idx << 16` at 0xA8, `idx` at 0xAC). Slots below `D_0016001C`
  are handled differently by `DeleteMoby`; likely level-placed instances
  (inferred).
- Each slot owns **0x80 bytes of pvars** at `D_00160028 + idx * 0x80`,
  zeroed by `CreateMoby`: the class's private state. Catalogue:
  `docs/PVARS.md` (offset caveat above). Level code casts pvars to private
  structs, e.g. `KillFlags`/`VoiceSlot` in
  `src/overlays/shared/vendor_00298BB8.c`.

## Lifecycle

### Registering a class (level load)

`register_moby_class` (`func_00203E78`, `src/game/loaders.c:973`; takes
the class data, two more arguments and the oClass):

1. Next slot `D_00160000`: fills `D_001B3E40` (oClass -> slot),
   `D_001B3C80` (slot -> oClass), `D_001B3580` (slot -> data).
2. `func_00213BB8` (`assign_moby_class_data`, `src/game/mobyutil.c:175`)
   looks oClass up in the `{key, a, b}` table `D_001E8F00` (key -1 ends):
   `a` (update function) -> `D_001B3900[slot]`, `b` (6-word table) ->
   `MobyClass.callbacks`.
3. Bumps the slot count. With non-null data: keeps its +0x2C word in
   `D_001B6500[slot]` and calls `MobyClassRelocate` (`func_00203B70`).

### Spawning

`CreateMoby` (`func_0020D348(oClass)`, `src/game/mobyfunc.c:188`):

- Takes the first slot with `state >= 0xFE` **and**
  `D_0015F6F0 >= spawn_frame` (`D_0015F6F0` = frame clock).
- 0xFE = free; 0xFF = free tail (taking it writes 0xFF into the next
  slot, moving the mark up).
- Calls `InitMobyInstance`, sets and zeroes `pvars`, decrements
  `D_0015FFFC` if nonzero. On failure prints "... Time: %d, oClass: %d"
  and returns 0.

`InitMobyInstance` (`func_0020D440(moby, oClass)`, `:295`) zeroes the
0x100 bytes, then sets:

- `alpha = 0x80`, `class_slot = D_001B3E40[oClass]`, `oclass = oClass`.
- 0xFF ("none"): `group`, `hit_slot`, `unk71`, `unk72`, `unk7C`,
  `sound_slot`.
- `occlusion = 0x7F80`; the 64-bit word at 0x38 = `0x0040404000000000`;
  bytes 0xA0..0xA3 = 0x7F, 0x7F, 0x80, 0x80.
- `update = D_001B3900[slot]`; 0 sets flag 0x2.
- With a class header: `pclass`, `unk72` (class +0x0E),
  `flags |= class.flags`, `collision`, `scale`, `anim_time` = 1.0, 0x5C =
  1.0; glow sets flag 0x10; shadow (class +0x0F) sets `shadow_range =
  0x18` and flag 0x400; class +0x06 sets `unk73 = 0x18`; then the sequence
  rules after `update_moby_animation_state` (`func_0020D6D0`).
- Without: `pclass = 0`, `flags |= 5`, `collision = 0`.

The 0x38 store puts 0x40 bytes at 0x3D-0x3F, so 0x38 may hold more than a
`u64 spawn_frame` (inferred; `DeleteMoby` overwrites all 8 bytes).

Spawners then fill per-use fields. Example: `func_00226720`
(`spawn_hand_gadget_moby`, `src/game/pause.c:8919`) returns 0 if
`D_001B3E40[oClass] == 0xFF`; else creates the moby, sets `update_dist =
draw_dist = 0xFF`, `state = 0`, `visible = 1`, runs the post pass
`func_0020ED48` once, tints it (`func_0020E340(o, 0x202020, ...)`), and
sets byte 0x73 = 0x18 if class +0x06 is set. Beams use 0x7F distances.

### Deleting and respawning

- `DeleteMoby` (`func_0020D678`, `:383`): `state = 0xFE` (0xFD below
  `D_0016001C`), `spawn_frame = D_0015F6F0 + 2`,
  `func_0020EA70(m, 0x80807F7F)`. The slot cannot be reused this frame or
  the next.
- `delete_moby` (`func_002267C0`): `DeleteMoby`, then `spawn_frame =
  D_0015F6F0` (reusable at once).
- Respawning classes (e.g. 1885) build on `spawn_frame` and slot states.

### States

`state` (0x20) below 0x80 is the class's own state machine. Reserved:

| state | meaning |
|---|---|
| 0x00..0x7F | alive; meaning is per class (update functions `switch` on it) |
| >= 0x80 | skipped by the update loop (signed `lb` + `bltz`) |
| 0xFD | deleted, slot below the dynamic pool |
| 0xFE | free / dead; reusable once `D_0015F6F0 >= spawn_frame` |
| 0xFF | free tail mark of the pool |

## The per-frame update loop

`MobyUpdateLoop` = `func_00213C78` (`asm/nonmatchings/text/func_00213C78.s`,
0x94 bytes, not matched yet):

```c
/* Reconstruction of the asm, not matched C. */
for (m = func_0020E6B8(); m != NULL; m = m->next) {   /* head also saved in a $gp global */
    if ((s8)m->state < 0) continue;                    /* state >= 0x80 */
    if (!(m->flags & 0x40)) func_0020E3D0(m);          /* pre pass */
    if (m->update) m->update(m);                       /* the class's update, moby in $a0 */
    if (!(m->flags & 0x4)) func_0020ED48(m);           /* post pass */
}
```

- `func_0020E6B8` (handwritten asm) builds the list: walks the array to
  the 0xFF tail, skips flag 0x2, tests distance to `D_00187040 + 0x140`
  against `update_dist` (0xFF = always), and links survivors through
  `next` via per-class-slot and per-group buckets in scratchpad
  (0x70000000). Inferred from the asm; ordering rules not yet written as
  C.
- `func_0020E3D0` (pre) / `func_0020ED48` (post): handwritten
  animation/matrix passes. Both read 0x50-0x53; post also reads `unk71`
  and 0xA8. Exact jobs inferred.
- Also called from pause code (`src/overlays/shared/pause_00277208.c:436`,
  `src/game/pause.c`): some menus keep mobys ticking.
- Changes a moby makes to another are seen by later mobys in the same
  frame.

### Distances and visibility

- `update_dist` (0x30): update radius, 0xFF = always.
- `draw_dist` (0x32, s16) and `visible` (0x31) drive drawing (`DrawMobys`
  `func_0020E2B0`, `DrawMobyList` `func_0020E180`). Per-class distances:
  `InitMobyClassDists` / `StashMobyClassDists` / `RestoreMobyClassDists`
  (`func_0020E040`/`068`/`098`).
- `occlusion` (0x36) default 0x7F80; `shadow_range` (0x7F) in units of
  1024.

### Save id, mission and the collected/killed tables

- `save_id` (u16, 0xB2) indexes the level's `collected[]`/`killed[]`
  tables (per-level BSS) and save bits `D_0014C190[level][id >> 5]`.
  Collect-once classes test it at init, e.g.
  `D_L01_001BB9C0.collected[(short)moby->save_id] != 0`
  (`src/overlays/shared/vendor_002B90A8.c:516`). `killed[]` stores "spawn
  slot + 2" (`src/overlays/shared/vendor_00298BB8.c`).
- `mission` (0xB0) indexes the level's `D_0014C050` row (0xFF = not
  spawned). Level code writes `collected[save_id] = mission + 2`
  (`src/overlays/l01_novalis/vendor_002FABE8.c:849`). Shared helpers
  (candidate names): `SetMissionDone` (`func_L00_002512D8`),
  `SetDeathBits` (`func_L00_002584A8`).
- `parent` (0xB8): spawner; its byte 0xB1 is a per-level id.
- `command` (0xBC): command byte other mobys write.
- `group` (0x21): index into `D_Lxx_001ABCC0` (0xFF = none), e.g. class
  871's mine groups.

### Animation fields

| Field | Off | |
|---|---|---|
| `frame`, `prev_frame` | 0x50, 0x51 | current frame / frame in `prev_seq` (blend source) |
| `seq`, `prev_seq` | 0x52, 0x53 | sequence id into `pclass->seqs[]`; 0xFF = none (frame data then points at `D_001AAF40`) |
| `anim_time`, `anim_speed` | 0x54, 0x58 | `InitMobyInstance` sets 1.0, or `anim_speed = 0` for a still one-frame class |
| `cur_frame_data`, `prev_frame_data` | 0x68, 0x6C | refreshed from the class's sequence table (`func_0020D6D0`) |
| `anim_flags`, `anim_trigger_count` | 0x70, 0x7E | |
| `manips` | 0x64 | joint manipulators |

`seqs[i]` -> `AnimSeq` (`include/records.h`, u8 `nframes` at +0x10);
`MobyClass.seq_count` (+0x0C) bounds `seq`. Entry points (names.tsv,
mixed tiers): `set_moby_animation` (`func_00213D28`),
`blend_moby_animation[_ex]` (`func_00213DE0`/`func_00213F28`),
`MobyAnimProc` (`func_00212578`), level helpers `MobyAnimAdvance`
(`func_L00_002514B8`) and `MobyAnimEval` (`func_L00_00252018`).

## Binding classes to update functions

- Each level's `vtbl` record (`docs/OVERLAYS.md`, "Roles"): 12-byte
  entries `{oClass, update function, 6-word table pointer}`, ended by
  oClass -1, tables after it. Registration copies them to
  `D_001B3900[slot]` / `MobyClass.callbacks`; `InitMobyInstance` sets
  `moby->update`. Called with the moby in `$a0`.
- `func_00213BB8` reads the table as `D_001E8F00`; manifests put each
  level's `vtbl` elsewhere (0x1EA680 level 0, 0x20BE00 level 1). Copy vs
  relocation: unverified.
- `python3 tools/overlays.py names` decodes all tables into
  `config/overlays/names.tsv` (roles `UpdateMoby_<oClass>:<levels>`,
  levels slash-separated). Many roles = likely a generic function.
- `tools/extract/moby_classes.tsv`: editor-label class names from outside
  tooling, unverified.

### The most widespread classes

1167 classes have an update function somewhere; 916 in one level only;
**79 in all 19 levels**. Below: the 25 lowest-numbered of the 79 (all in
`src/overlays/shared/`). Descriptions condensed from the source comment.
"Shared" = classes using the same function.

| oClass | Update function | Shared | File (`src/overlays/shared/`) | Description (from the source comment) |
|---|---|---|---|---|
| 11 | `func_L00_002A6A38` | 1 | `vendor_002A5138.c` | pulsing light-beam moby: spawns a child, fades it with distance to the player, draws it and may trigger the player's grab |
| 13, 14, 15, 16 | `func_L00_002A7078` (BoltUpdate) | 4 | `vendor_002A5138.c` | bolt: lands where dropped, hovers and spins, bounces, flies to the hero when near (or sucked in), leaves a trail, collected on arrival |
| 71 | `func_L00_002A9768` | 1 | `vendor_002A5138.c` | normal wrench swings and its thrown/returning collision sweep |
| 74 | `func_L00_002AB910` | 1 | `vendor_002AB910.c` | mine: falls and bounces, sticks to what it hits, homes on a nearby target and explodes |
| 112 | `func_L00_002ADD78` | 2 (112, 1192) | `vendor_002AB910.c` | decaying moby: spins down its position fields and fades out |
| 121 | `func_L00_002AEDD0` | 1 | `vendor_002AB910.c` | thrown bomb: flies and spins, bounces once, explodes on a hit or timeout, blast ring grows for 15 frames |
| 122 | `func_L00_002B08A0` | 1 | `vendor_002AB910.c` | falling spark: emits a burst particle, moves, fades, dies out of bounds |
| 149 | `func_L00_002B0D30` | 11 (149, 348, 349, 354-359, 363, 364) | `vendor_002AB910.c` | |
| 153 | `func_L00_002B1688` | 1 | `vendor_002AB910.c` | |
| 157 | `func_L00_002B33E8` | 1 | `vendor_002B33E8.c` | Devastator: tracks the aimed target, draws the reticle, fires, keeps usage statistics, moves the muzzle light |
| 163 | `func_L00_002B4340` | 1 | `vendor_002B33E8.c` | arms a pickup-style reaction when the player state allows it |
| 166 | `func_L00_002B4748` | 1 | `vendor_002B33E8.c` | |
| 167 | `func_L00_002B5488` | 1 | `vendor_002B33E8.c` | multi-state timed object: random delays pace the stages, which adjust step/scale in its pvars and call the shared callback |
| 168 | `func_L00_002B5C98` | 1 | `vendor_002B33E8.c` | Blaster: aims (reticle in first person, auto-aim otherwise), fires a shot with sparks, statistics, muzzle light |
| 172 | `func_L00_002B7430` | 1 | `vendor_002B33E8.c` | visibomb missile: drift, homing toward the target, arming countdown |
| 175 | `func_L00_002B8208` | 1 | `vendor_002B33E8.c` | |
| 176 | `func_L00_002B8AE8` | 1 | `vendor_002B33E8.c` (asm) | |
| 177 | `func_L00_002B9AD8` | 1 | `vendor_002B33E8.c` | Tesla claw: tracks up to two targets while fire is held, extends the claw tip, firing states, statistics, light |
| 179 | `func_L00_002BC708` | 1 | `vendor_002BA7C8.c` | follows its parent's flag and spins |
| 180 | `func_L00_002BC860` | 1 | `vendor_002BA7C8.c` | scatters ten fading sparkle quads around the hero |
| 185 | `func_L00_002BDAF0` | 1 | `vendor_002BA7C8.c` | Morph-o-Ray: aims it, transforms what it hits over time, keeps its glow light and sound going while held |
| 186 | `func_L00_002C12B0` | 1 | `vendor_002C12B0.c` (asm) | (no comment; see "M186" below) |

The remaining 54 all-level classes: 188 190 192 203 204 208 209 213 214
222 223 225 226 229 230 270 305 348 349 354 355 356 357 358 359 428 433
454 457 479 500 501 511 562 585 607 618 639 775 804 806 849 1006 1007
1138 1143 1192 1251 1289 1290 1438 1447 1449 1900. `func_L00_002C6720`
serves 11 (204 213 214 222 223 225 226 1006 1438 1447 1449). Regenerate:
per oClass, union the levels of all `UpdateMoby_<oClass>` roles in
`config/overlays/names.tsv`.

## Classes named by retail debug strings

Verbatim from `strings baserom/overlays/level_NN/data.bin`. In every case
the update-function levels (names.tsv) match the levels holding the
string.

| oClass | What the string says | String in levels | Update function (levels) |
|---|---|---|---|
| 28 | `*Error: Moby 28(Footsoldier) with no boundPath / flyPath / introPath / hatch index` | 14, 15 | `func_L14_002B2A28` (14, 15) |
| 557 | `*Error: Moby 557(hoverboat) with no target.` | 14 | `func_L14_002E9038` (14) |
| 610 | `*Error: Moby 610 (wind-control) with no cuboid index (wind pushing hero area)` (also fxAct / fxArea cuboids) | 14 | `func_L14_002EC3E8` (14) |
| 908, 921 | `Enemy Fighter (moby908) with no flyPath.`; same for `moby921` | 14 | `func_L14_002FD578`, `func_L14_002FE038` (14) |
| 923, 1401 | `MineBot (moby923) with no activation cuboid.`; `MineBot (moby1401) ...` (flyPath, exitPath) | 14; 16 | `func_L14_002FF728` (14); `func_L16_002E4C08` (16) |
| 1417 | `Flying Car (moby1417) with no flyPath.` | 14 | `func_L14_00308368` (14) |
| 1059 | `Illegal Bounding path` / `Illegal Hero Detect path` / `Illegal guard point cuboid` / `Illegal water moby for moby 1059 instance #%d.` | 07 | `func_L07_003109D0` (07) |
| 871 | `Moby 871 could not find mines for group %d` (and mine-count vs pVar checks) | 07 | `func_L07_0030C3E0` (07) |
| 1142 | `ERROR! Moby 1142 Equilizer Ammo Spawner has no trigger cuboid!` [sic] | 07 | `func_L07_0031E6B0` (07) |
| 774 | `WARNING! - Moby 774 wasn't expecting a M774_ShipChallengeDone call. ...` | 01, 08 | `func_L01_003004F0` (01, 08) |
| 947 | `Moby947 maya instance %d HAS NO PATH!!!` | 10 | `func_L10_002DA0F0` (10) |
| 1067 | `moby class 1067 REQUIRES a link to dataProcessor class 1073!!` | 10 | `func_L10_002DB688` (10); 1073: `func_L10_002DE018` (10) |
| 1181 | `could not link next / prev on moby type 1181 #%d` (a linked list of instances) | 09 | `func_L09_00305710` (09) |
| 1885 | `Moby 1885 UID:%d could not find a moby to respawn at time %d!` (a respawner) | 09 | `func_L09_0030BF88` (09, 13) |
| 427 | `moby 427, instance %d requires both path fields to run!!! DELETING MOBY FROM LEVEL!!` | 04 | `func_L04_002C5BD0` (04) |
| 1120 | `Moby 1120 %d has no segment switch!` | 04 | `func_L04_002E2DF0` (04) |
| 1290 | `Moby 1290 %d has no segment switch!`, `Moby 1290 %d needs a cut scene!` | all 19 | `func_L00_002E1850` (all 19) |
| 899 | `Bad path data on moby class 899, maya instance %d` | 03 | `func_L03_002DC648` (03) |
| 111 | `no path for moby 111 #%d: killing` | 13 | `func_L13_002C56B0` (13) |
| 6 | `No trigger for moby 6 instance %d. Deleting` | 13 | `func_L13_002B2020` (13) |
| 794 | `M794 %d trigger %d = moby %d (oClass %d)` (an effect/trigger moby) | 10 | `func_L10_002CB030` (10) |
| 671 | `M671 texture w count = 0.0, correcting to 1.0`, `... greater than max texture tile count (8)` (texture tiling) | 08 | `func_L08_002F8560` (08) |
| 186 | `Hey, M186 init should be earlier!` (an init-order check) | all 19 | `func_L00_002C12B0` (all 19) |

- "UID" = per-level instance id; "maya instance" = level-editor instance
  number. Neither is the slot index.
- "Mystery 774" is in the literal pool (`lit.bin`) of levels 01 and 08, not in `data.bin`; `M774_ShipChallengeDone` is in both levels' strings.
  No name for 774 is established.
- Without an oClass in the string: "Hovertank" (level 14; bomb/bound/
  exit/intro paths), "Kamikaze" and "Bigship moby" (level 09).
- Generic init-code strings: `Moby Class %d Instance %d has no pVars!  May
  be in the art file.` (10), `cleaned up moby:%d (oclass:%d)` (00),
  `internal moby sound mismatch on moby %d` (00).

More retail strings per level: [datamining.md](datamining.md#5-per-level-strings-and-npc-names).

## Tips for decompiling and porting moby code

- **Start from the role** in `config/overlays/names.tsv`. Don't
  specialise a function shared by many classes.
- **Include `moby.h`, `hero.h`, `pad.h`, `vector.h`**, not private copies
  (see commits d19de34f, d8808743). pvars still need a local struct for
  the 0x80 bytes, cast from `moby->pvars` (u8 *).
- **Get signedness/width right.** `state` is read signed by the loop
  (`lb`, `bltz`) but compared `>= 0xFE` elsewhere; `save_id` is u16 used
  as `(short)`; `draw_dist` s16, `update_dist` u8. A wrong type changes
  `lb`/`lbu`/`lh`/`lhu` and, in a port, behaviour at 0x80/0x8000.
- **Use the conventions:** delete with `DeleteMoby`, not by poking
  `state`; test `D_001B3E40[oClass] != 0xFF` before spawning.
- **Order and timing are behaviour.** Out-of-range mobys don't tick
  (timers freeze). Level frame counts often go through
  `scale_game_frames()` (`func_001F98C0`); keep the scaling.
- **Handwritten asm stays asm** (`func_0020E6B8`, `func_0020E3D0`,
  `func_0020ED48`, `func_0020EA70`, ...; `config/handwritten_asm.txt`;
  VU0 macro ops, scratchpad). A port needs exact C reimplementations
  (list order, float results). Post-matching rewrites keep them marked
  as asm.
- **Grep the level's `data.bin` strings** for the oClass before naming a
  class or pvar.
- **Check callees against their asm** (argument registers read,
  `$v0`/`$f0` set). Update functions take exactly one argument.
