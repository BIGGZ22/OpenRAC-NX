# Camera system: map and refactor plan

The camera code, the state it owns, and the plan for turning its raw byte
offsets into named fields without changing one byte of the build. The
layouts are in [`include/camera.h`](../../include/camera.h); this page is the
map and the conversion table. Conventions: [conventions.md](conventions.md);
the byte-exact rule: [README](README.md#the-one-rule).

Status (2026-10-18, branch `refactor/camera-pilot`): the header, the names
and the declaration flips are in (`gCamera`, `gDebugFreeCam`,
`gCameraTypes`, `gCameraDefs` in camera.c; `gCameraLvl` and the level
tables in the level files); `Camera_UpdateUnderwaterFlag`,
`Camera_UpdateFade`, `UpdateCamera` and `Camera_FindTypeIndex` are
converted. The check for every edit is
`bash tools/docker/run.sh python tools/file_check.py <file>` (15 s for
camera.c, about 75 s for a level file). The other functions
are per-function work in `build-sn/refactor/camera/tasks.json`, with the
worker rules in `build-sn/refactor/WORKER_BRIEF.md`.

## 1. Where the code is

| File | What | C functions |
|---|---|---|
| `src/game/camera.c` | the executable's camera unit (`camera.cpp`, text 0x1EC038-0x1EDFF8) | 22 |
| `src/overlays/shared/camera_001EB508.c` | the level copy, shared by all 19 levels (canonical addresses in level 00) | 15 (+7 stubs) |
| `src/overlays/shared/camera_001ED6D8.c` | `Camera_InitSlots`, in a file of its own for `-fno-force-mem` (BUILD_FIDELITY.md, "Flags") | 1 |
| `src/overlays/shared/camera_001FC308.c` | `RegisterUniqueCallback` (canonical in level 02) | 1 |
| `include/camera.h` | the layouts below, offsets checked at compile time | |

The executable's `main` only runs before the first level loads; each level
carries its own build of the game program ([architecture.md](architecture.md#2-level-overlays)).
So there are two copies of the camera code and of its state:

- the executable's, at `D_00187040` and friends (`src/game/camera.c`);
- the level build's, at `D_L00_00166D80` and friends in level 00 (other
  levels pack it elsewhere: level 01 at `D_L01_00167180`, level 02 at
  `D_L02_00167300` ...). Shared level code names the level-00 symbol and
  the build places it per level.

Some functions exist only once and both copies call them by their
executable name (the catalogue's `exe` kind: `func_001EC270`,
`func_001EC2B8`, `func_001ED818` ... in level code mean the level's copy of
that function).

Level code reaches the small data 0x40 below the executable:
`D_0015F08C` (exe) is `D_L00_0015F04C` (level), `D_0015F090` is
`D_L00_0015F050`, `D_0015F09C` is `D_L00_0015F05C`, `D_0015F53C` is
`D_L00_0015F4FC`, `D_0015F6E8` is `D_L00_0015F6A8`. The bss block moves by
0x202C0 (`D_00187040` / `D_L00_00166D80`, `D_0018C418` / `D_L00_0016C158`).

### A fold to know about

`func_001EC270` (`Camera_runSetupToNewCam`, calls a type's `activate`
hook) and `func_001EC780` (`Camera_Exit`, calls its `exit` hook) differ only
in the hook offset (+0x08 / +0x10), so the overlay catalogue fingerprints
them as one function: `config/overlays/functions.tsv` gives
`func_001EC270` two places per level (level 00: 0x1EB848 and 0x1EBD58). The
first call in `UpdateAllCamerasLvl` is written `Camera_runSetupToNewCam(best)`
but its `jal` goes to 0x1EBD58, the `exit` wrapper, like the executable's
`Camera_Exit(cur)`. The C is byte-exact either way; read the level copy's
calls through those two places, not through the name. See the
"catalogue false folds" note in the project memory.

## 2. Memory map

Executable addresses; the level-00 symbol in the third column. Names are
`config/names_local.tsv` rows (suffix `Lvl` = the level build's copy).

| Address | Level 00 | Name | Type (`camera.h`) | Size | What |
|---|---|---|---|---|---|
| `D_00187040` | `D_L00_00166D80` | `gCamera` / `gCameraLvl` | `CameraSystem` | 0x3A0 | the camera block; transition.c clears 0x3A0 bytes on a level load |
| `D_00187510` | `D_L00_00167250` | `gCameraSlots` / `gCameraSlotsLvl` | `Camera[48]` | 0x1E00 | the camera slots |
| `D_00189310` | (`D_L00_00169050`) | `gCameraBackup` | `Camera` | 0xA0 | `BackupCurrentCam` copies the current camera here |
| `D_001893B0` | `D_L00_001690F0` | `gCamPostUpdFuncs` / `...Lvl` | `void (*[16])(void)` | 0x40 | post-update queue (`RegisterUniqueCallback` caps it at 16) |
| (`D_001894D0`) | `D_L00_00169210` | `gCamCurStateLvl` | `u8[0x280]` | 0x280 | state buffer of the current camera (`Camera.state_buf`) |
| `D_00189750` | `D_L00_00169490` | `gCamPrevState` / `...Lvl` | `u8[0x280]` | 0x280 | state buffer of the previous camera |
| `D_001899D0` | | `gCamBackupState` | `u8[0x280]` | 0x280 | state buffer of `gCameraBackup` |
| `D_00189C50` | `D_L00_00169990` | `gCameraSlotUsed` / `...Lvl` | `s32[48]` | 0xC0 | nonzero = slot in use |
| `D_0018C418` | `D_L00_0016C158` | `gDebugFreeCam` / `...Lvl` | `DebugFreeCam` | | debug free camera; `.active` (+0x14, `D_0018C42C`) stops the camera code writing the view |
| `D_001E8F80` | `D_L00_001EAC00` | `gCameraTypes` / `...Lvl` | `CameraType[]` | 0x14 each | the camvtbl rows; id -1 ends it |
| `D_0015F08C` | `D_L00_0015F04C` | `gCamPostUpdCount` / `...Lvl` | `s32` | | entries in the queue |
| `D_0015F090` | `D_L00_0015F050` | `gCameraDefs` / `...Lvl` | `CameraDef *` | | the level's camera table |
| | `D_L00_0015F054` | `gCameraDefCountLvl` | `s32` | | its rows |
| `D_0015F09C` | `D_L00_0015F05C` | `gCamSphereMask` / `...Lvl` | `s32` | | collision mask of the camera's sphere probes (`MoveCameraToward` passes it to `coll_sphere`) |
| `D_0015F0A0` | `D_L00_0015F060` | `gCamSphereMaskBase` / `...Lvl` | `s32` | | the same before bit 0x80 |
| `D_0015F098` | `D_L00_0015F058` | `gCamLineMask` / `...Lvl` | `s32` | | collision mask of the camera's line probes (vendor_002E1660.c) |
| `D_0015F53C` | `D_L00_0015F4FC` | `gFade` / `gFadeLvl` | `f32` | | fade level: 1.0 on a level load, cut by `Camera_UpdateFade` and by cutscenes |

`D_001894D0` (the executable's current-state buffer) and `D_L00_00169050`
(the level's backup camera) are not referenced as symbols; code reaches
the first as `gCamPrevState - 0x280` and through `Camera.state_buf`.

Sub-blocks of `gCamera` that code reaches through their own symbols (keep
the symbol where the conversion changes code, section 6):

| Symbol | Level 00 | Field |
|---|---|---|
| `D_00187180` | `D_L00_00166EC0` | `gCamera.pos` (z: `D_L00_00166EC8`) |
| `D_00187198` | `D_L00_00166ED8` | `gCamera.rot.f[2]`, the yaw (lights.c) |
| `D_001871A0` | `D_L00_00166EE0` | `gCamera.shake` |
| `D_001871C0` | `D_L00_00166F00` | `gCamera.current` |
| (`D_001871C4`) | `D_L00_00166F04` | `gCamera.previous` |
| `D_001871D0` | `D_L00_00166F10` | `gCamera.follow` |
| `D_001872B0` | `D_L00_00166FF0` | `gCamera.blend` (`D_001872B2`: `.kind`) |
| `D_00187390` | `D_L00_001670D0` | `gCamera.matrix[0]` (`D_001873A0`: `[1]`, `D_001873B0`: `[2]`) |
| `D_001873D4` | `D_L00_00167114` | `gCamera.underwater` |
| `D_0018C42C` | `D_L00_0016C16C` | `gDebugFreeCam.active` |

## 3. Layouts

Field names and comments are in `include/camera.h`; this is the overview.
Offsets are checked by `CAMERA_AT`/`CAMERA_SIZE` lines in the header: a
wrong one stops the compile.

### Camera (0xA0, one slot)

| Off | Type | Field | Evidence |
|---|---|---|---|
| 0x00 | `VecQuad[3]` | `basis` | rotation rows; `UpdateCamera` copies them to `gCamera.matrix` |
| 0x30 | `Vec4f` | `pos` | position; `UpdateAllCameras` copies x/y/z to `prev_pos` |
| 0x40 | `u8[0x24]` | `type_data` | per-type state the hooks keep |
| 0x64 | `CameraVec3` | `prev_pos` | |
| 0x70 | `void *` | `state_buf` | the type's 0x280-byte state (`gCamCurState` while current, `gCamPrevState` after; vendor_002F7700.c reads it as its orbit record) |
| 0x74 | `s32` | `activation` | the rule in `Camera_ActivationCheckPriority`: 0 by priority, 1/2 by priority once `handoff_state` is set, 4 while the hero is in `CameraInfo.clip_volume`, 7 by the hero's mode (+0x2284) and `rule7_index`; from `CameraInfo` +0x1F |
| 0x78 | `f32` | `blend_time` | the switch's blend rate (eased) or frames (orbit); <= 0 takes the default; -1 after init |
| 0x7C | `u8` | `priority` | 0 never activates; a higher one takes over; from `CameraInfo` +0x1C |
| 0x7D | `u8` | `handoff_state` | |
| 0x7E | `s16` | `transition_state` | nonzero: any eligible camera may take over (`Camera_ReleaseCurrent` sets 1); the switch reads 2 (blend), 3 and 5 (hand-off), 4 (blocked) |
| 0x80, 0x82 | `s16` | `unk80`, `unk82` | -1 after init |
| 0x84 | `s16` | `def_index` | row of `gCameraDefs` |
| 0x86 | `s16` | `id` | from `CameraDef.id`; 0 is the default behind-the-hero camera (`FindCameraById(0)`, the hero collision moby), 6 turns shake and the underwater test off |
| 0x89 | `u8` | `hit_wall` | `MoveCameraToward` hit something |
| 0x8A | `s16` | `snap` | `MoveCameraToward` jumps to the target once |
| 0x8C | `s16` | `type` | row of `gCameraTypes` (`Camera_FindTypeIndex(id)`) |
| 0x8E | `s16` | `hard_cut` | 1 when it became current without a blend (the switch's blocked and fallback cases, `CameraResetBehindHero`); no reader in the camera files |

### CameraType (0x14, one camvtbl row)

`{id, check, activate, update, exit}`. `check(cam, current)` returns -1
(never), 1 (always) or anything else (the activation rule decides);
`activate` runs when the camera becomes current; `update` each frame while
it is current; `exit` is what `Camera_Exit` calls, and `UpdateAllCameras`
calls it on the current camera first thing each frame. `tools/overlays.py`
names the roles `Init/Activate/Update/ExitCamera_<id>`; `check` is its
"init".

### CameraDef (0x20) and CameraInfo

`gCameraDefs[i]`: `id` (+0x0C, u16), `unk10`, `info` (+0x1C). `info` points
to the camera's parameters: `slot` (+0x04, set by `Camera_InitSlots`),
`clip_volume` (+0x0C), `priority` (+0x1C), `blend_kind` (+0x1D: 1 eased
blend, 5 orbit blend, 3 and 6 hand-off), `activation` (+0x1F),
`rule7_index` (+0x24). vendor_002F7700.c calls the same rows
`CameraPresetRow`/`CameraPreset` (+0x20..+0x34 there: paths and a target).

### CameraSystem (0x3A0, gCamera)

| Off | Field | Type | What |
|---|---|---|---|
| 0x040 | `view_matrix` | `f32[4][4]` | the renderer's view matrix (draw.c) |
| 0x140 | `pos` | `Vec4` | eye position |
| 0x150 | `rot` | `Vec4` | Euler angles of `matrix` (`MatrixToEuler`), z the yaw |
| 0x160 | `shake` | `CameraShake[2]` | {amplitude, offset, frames, frames_max}; [0] moves along `matrix[2]`, [1] along `matrix[0]` |
| 0x180 | `current` | `Camera *` | |
| 0x184 | `previous` | `Camera *` | `Camera_StartBlend` blends from it |
| 0x190 | `follow` | `CameraFollow` | what `CameraPreUpdate` tracks of the hero (below) |
| 0x270 | `blend` | `CamBlend` | the switch blend (below) |
| 0x350 | `matrix` | `Vec4[3]` | view rotation rows |
| 0x380 | `unk380` | `s32[5]` | zeroed by `Camera_InitSlots` |
| 0x394 | `underwater` | `s32` | the eye is under water (draw.c reads it) |
| 0x398 | `frames` | `s32` | frames since the last switch (`UpdateCamera` counts, the switch zeroes) |

`CameraFollow` (+0x190): the hero position with a sprung z (`pos`,
`z_vel`), a sprung up vector (`up`, `up_target`, `up_target_prev`,
`up_vel[4]`, minus the hero's `gravity_dir`), last frame's hero position
and this frame's movement split along and across up (`hero_prev`,
`hero_delta`, `hero_delta_side`, `hero_delta_up`, `hero_dist`,
`side_dist`, `up_dist`), five frames of hero yaw (`yaw_hist`), the
ground-type bits for `gCamLineMask` (`coll_bits`, +0xC0), the hero
collision moby of camera id 0 (`hero_coll_moby`, +0xC4), `fade_rate`
(+0xC8), and the moby the hero stands on with its height change
(`ground_moby`, `ground_z`, `ground_dz`).

`CamBlend` (+0x270): `phase` (0 none, 1 start, 2 restart, 3 running),
`kind` (the running blend: 0 lerp, otherwise orbit; 2 orbits around the
hero), `next_kind` (what the switch asked for), then two halves:

- `lerp` (+0x10, `CamBlendLerp`): `rot_t`, `rot_rate`, `rot_rate_cfg`,
  `pos_t`, `pos_rate`, `pos_rate_cfg`, `from_rot`, `from_pos`, `pos`, `rot`.
  `Camera_StepBlend` eases position and slerps rotation from `from_*` to the
  target camera, advancing `*_t` by `*_rate` times D_0015EE60 (frame scale).
- `orbit` (+0x70, `CamBlendOrbit`): `yaw`, `pitch`, `dist`, `frames_left`,
  `inv_frames`, `frames`, the axes `fwd` and `up`, `rot`, `pos`,
  `saved_rot`. `Camera_UpdateTowardTarget` steps yaw/pitch/distance
  toward the target and rebuilds the basis.

## 4. The functions

Executable (`src/game/camera.c`), in address order. Callers are from the
retail assembly (`jal`); "level" lists the level copy.

| Address | Name | What | Callers | State |
|---|---|---|---|---|
| 0x1EC038 | `BackupCurrentCam` | copies the current camera and its state buffer to `gCameraBackup` / `gCamBackupState` | `Camera_SwitchActive`, `SwitchCamera`, `CameraResetBehindHero` | current, backup |
| 0x1EC098 | `ExecuteCamPostUpdFuncs` | runs and empties the post-update queue | `UpdateAllCameras` (level: `Camera_RunQueuedCallbacks`) | queue |
| 0x1EC120 | `Cam_InterpValues` | damped spring step of one value, clamped | `CameraPreUpdate` and 16 camera-type functions | |
| 0x1EC210 | `Camera_handleCollWithHero` | spawns or deletes `follow.hero_coll_moby` by camera id | `UpdateAllCameras`, `UpdateAllCamerasLvl` | follow |
| 0x1EC270 | `Camera_runSetupToNewCam` | calls the type's `activate` | `Camera_SwitchActive`, `SwitchCamera`, `CameraResetBehindHero` (and the fold, section 1) | types |
| 0x1EC2B8 | `Camera_SwitchActive` | picks the blend from the old camera's `transition_state` and the new one's `blend_kind`, installs the new camera | `UpdateAllCameras` (level: `SwitchCamera`) | current, previous, blend, state buffers |
| 0x1EC5B8 | `Camera_ActivationCheckPriority` | whether a camera should take over (type `check`, then the activation rule) | `UpdateAllCameras`, `UpdateAllCamerasLvl` | slots, defs, hero |
| 0x1EC780 | `Camera_Exit` | calls the type's `exit` | `UpdateAllCameras` | types |
| 0x1EC7C8 | `UpdateAllCameras` | `exit` hook, scan of the 48 slots, switch, `update` hook, `prev_pos`, post-update queue | `UpdateCamera` (level: `UpdateAllCamerasLvl`) | slots |
| 0x1EC8D8 | `Camera_Pos2Polar3d` | yaw, pitch and distance of a point in a frame | the capture functions and `Camera_UpdateTowardTarget` | |
| 0x1ECAB8 | `Camera_CaptureOrientation` | hero moby axes into `blend.orbit` and its angles | `Camera_StartBlend` (level: `Camera_CaptureHeroRelativeView`) | blend.orbit |
| 0x1ECB98 | `Camera_stagePendingTransform` | lerp pos/rot into orbit pos/saved_rot (kind 0 only) | `Camera_StartBlend`, `Camera_CaptureTransform` | blend |
| 0x1ECC10 | `Camera_commitPendingTransform` | orbit pos/saved_rot back into lerp (kind set only) | the same | blend |
| 0x1ECC48 | `Camera_StartBlend` | captures the blend source from the previous camera and starts the blend (phase 3) | `UpdateCamera` (level: `Camera_CaptureTransform`) | blend |
| 0x1ECEA0 | `Camera_StepBlend` | one lerp step; 1 when done | `Camera_UpdateBlend` | blend.lerp, pos, matrix |
| 0x1ED080 | `Camera_UpdateTowardTarget` | one orbit step; 1 when done | `Camera_UpdateBlend` | blend.orbit, pos, matrix |
| 0x1ED658 | `Camera_UpdateBlend` | steps the running blend, ends it | `UpdateCamera`, level frame | blend |
| 0x1ED708 | `Camera_UpdateShake` | one shake channel into `gCamera.pos` | `UpdateCamera`, level frame | shake, pos |
| 0x1ED818 | `CameraPreUpdate` | `gCamera.follow` from the hero | `UpdateCamera`, level frame, `CameraResetBehindHero` | follow |
| 0x1EDB98 | `Camera_UpdateUnderwaterFlag` | `gCamera.underwater` by a probe through the eye | `UpdateCamera`, level frame | underwater |
| 0x1EDCE8 | `Camera_PickCollisionMasks` | the probe masks from the hero's state and ground | `UpdateCamera` (level: `Camera_PickCollisionMasksLvl`) | masks, follow.coll_bits |
| 0x1EDE08 | `Camera_UpdateFade` | `gFade -= follow.fade_rate` down to 0 | `UpdateCamera`, level frame | gFade |
| 0x1EDE50 | `UpdateCamera` | the per-frame camera update | `SpaceFlightUpdate` (level: `Camera_UpdateFrame`) | all |

Level copy (`src/overlays/shared/camera_*.c`, level-00 addresses):

| Address | Name | What | Callers |
|---|---|---|---|
| 0x1EB508 | `Camera_RunQueuedCallbacks` | level `ExecuteCamPostUpdFuncs` | `UpdateAllCamerasLvl` |
| 0x1EB578 | `FindCameraById` | the slot with that `id`, or 0 | `CameraResetBehindHero`, camera scripts (`CameraScript*`, `Camera_ActivateOnMoby`) |
| 0x1EB6A8 | `Camera_SmoothAngle` | `Cam_InterpValues` for angles (wraps through `FastSubRots`/`FastAddRots`) | 12 camera-type functions |
| 0x1EB7C8 | `Camera_ReleaseCurrent` | current camera: `transition_state` 1, `handoff_state` 0 | `CameraScript2` |
| 0x1EB890 | `SwitchCamera` | level `Camera_SwitchActive` | `UpdateAllCamerasLvl`, `CameraScript`, `Camera_ActivateOnMoby` |
| 0x1EBDA0 | `UpdateAllCamerasLvl` | level `UpdateAllCameras` | the level frame, `DebugFreeCameraUpdate` |
| 0x1EC090 | `Camera_CaptureHeroRelativeView` | level `Camera_CaptureOrientation` | `Camera_CaptureTransform` |
| 0x1EC220 | `Camera_CaptureTransform` | level `Camera_StartBlend` | the level frame |
| 0x1ED2C0 | `Camera_PickCollisionMasksLvl` | level `Camera_PickCollisionMasks` (joined with 0x1ED380/0x1ED3D4) | the level frame |
| 0x1ED428 | `Camera_UpdateFrame` | level `UpdateCamera` (joined with 0x1ED460/0x1ED464) | `LoadLevelGameplay`, `LevelFrameUpdate` |
| 0x1ED5B0 | `Camera_FindTypeIndex` | row of `gCameraTypesLvl` for an id | `Camera_InitSlots` |
| 0x1ED600 | `CameraResetBehindHero` | camera id 0 becomes current, hard cut, follow reset | `Camera_InitSlots`, `Hero_ResetToPosition`, `PauseMenu_LoadSnapshot`, `Camera_TransitionUpdate` ... |
| 0x1ED6D8 | `Camera_InitSlots` | the slots from `gCameraDefsLvl`, the block from the hero | `LoadLevelGameplay` |
| 0x1ED900 | `Camera_Polar3dToPos` | inverse of `Camera_Pos2Polar3d` | 5 camera-type functions |
| 0x1ED9B0 | `Camera_UpdatePolarFromPos` | yaw (unwrapped), pitch, distance of a vector | 3 camera-type functions |
| 0x1EDA28 | `MoveCameraToward` | moves a position toward the target in steps, stopping on `coll_sphere` hits | `CameraType5Update`, `CameraType7Update` |
| 0x1FC308 (L02) | `RegisterUniqueCallback` | adds a function to the post-update queue once (16 at most) | `func_L02_002FBB48` |

### One frame

`UpdateCamera` (level: `Camera_UpdateFrame`): `frames++`;
`Camera_UpdateFade`; `Camera_PickCollisionMasks`; `CameraPreUpdate`;
`UpdateAllCameras` (`exit` hook, scan, `Camera_SwitchActive` on a change
(which sets `blend.phase` 1 or 2 for blended switches), the `update` hook,
`prev_pos`, the post-update queue); `Camera_StartBlend(previous)` when the
phase is 1 or 2; then either `Camera_UpdateBlend(current)` (phase 3) or,
unless the debug free camera is active, the current camera's position and
basis straight into `gCamera.pos` / `matrix`; Euler angles into
`gCamera.rot`; both shake channels; the underwater test; the environment
(fog) update; with mirrored levels, `matrix[1]` rebuilt as a cross product.

## 5. Names

- Functions keep their names (most are recovered or descriptive already).
  New ones follow `Camera_<Verb><Object>`; a level copy of an executable
  function that has no name of its own takes the executable name plus `Lvl`
  (`UpdateAllCamerasLvl`). Existing level names (`SwitchCamera`,
  `Camera_CaptureTransform` ...) stay.
- Globals: `g` + CamelCase, the level copy with `Lvl`. Sub-block symbols
  (section 2, second table) get no names of their own: they become fields
  of `gCamera`.
- Types: `Camera`, `CameraType`, `CameraDef`, `CameraInfo`, `CameraShake`,
  `CameraFollow`, `CamBlend`, `CamBlendLerp`, `CamBlendOrbit`,
  `CameraSystem`, `DebugFreeCam` (`include/camera.h`). Fields
  `snake_case`; `unkXXX` for accessed-but-unknown, `padXXX` for gaps.
- Locals: `cam` for a `Camera *`, `cs` (or the existing one-letter name)
  for a `CameraSystem *`, `b` for a `CamBlend *`, `lerp` / `orbit` for its
  halves, `f` for a `CameraFollow *`, `st` for a `DebugFreeCam *`, `type`
  for a `CameraType *`, `def` / `info` for the level tables. Parameters get
  the role's name (`arg0` -> `cam`, `to`, `from`). A rename never changes a
  local's type, order of declaration or scope (section 6).
- New names go in `config/names_local.tsv` with evidence, then
  `tools/names.py local`, `header`, `apply-defs`, `apply`.

## 6. Conversion rules (what keeps the bytes)

Measured on the three converted functions; each rule is what the build
showed.

1. **Same access path.** A local pointer stays a local pointer, a direct
   global stays direct. `char *c = gCamera; *(int *)(c + 0x398)` becomes
   `CameraSystem *c = &gCamera; c->frames`, not `gCamera.frames`.
2. **A sub-block reached through its own symbol or pointer keeps a pointer
   to it.** `*(short *)D_001872B0 = 0` written as `gCamera.blend.phase = 0`
   shares `gCamera`'s base register with the rest of `UpdateCamera` (10
   bytes differ); `CamBlend *b = &gCamera.blend; b->phase = 0;` matches.
3. **Arithmetic that leaves the sub-block stays arithmetic.** `v - 0x20`
   from a `CameraShake *v` is `&gCamera.pos`, but writing that changes
   which register holds what; keep `(char *)v - 0x20` with a comment
   naming the field.
4. **Keep the cast of the value, not the address** (conventions.md):
   `x = (s32)cam->field`, never `*(s32 *)&cam->field`.
5. **A declaration flip is per file.** `extern char D_00187040[];` and
   `extern CameraSystem D_00187040;` cannot both be in one file; every
   use changes in the same edit. That is done for `D_00187040`,
   `D_0018C418`, `D_001E8F80`, `D_0015F090`, `D_L00_00166D80` and the
   level tables. The flips left in the tasks (`D_00189310`, `D_00187510`,
   `Elem` in camera_001ED6D8.c) each have one user.
6. **Existing aliases may stay** (`D_001872B0_h`, `D_0015EEB4_m`, the
   `_EB890` set): when replacing one changes bytes, keep it and say why
   in a comment; never add a new one (conventions.md, section 2).
7. Comments: each function's comment says what it does in field names
   (`gCamera.underwater`, not `D_00187040+0x394`); provenance notes
   ("Adapted from Lombyte (GPL-3.0) ...") stay word for word.

## 7. Known oddities

- The fold of `func_001EC270`/`func_001EC780` in level code (section 1).
- `Camera_PickDrawModes` was the old name of `Camera_PickCollisionMasks`:
  its words are collision masks (`MoveCameraToward` passes the sphere mask
  to `coll_sphere`, vendor_002E1660.c the line mask to its line probe).
- `RegisterUniqueCallback` is shared code but its symbols are canonical in
  level 02 (`D_L02_0015F04C`, `D_L02_00169670`), so the `Lvl` names above
  do not cover it.
- Names from the PAL memory map at 0x15F000 and up describe the running
  game, which is a level build; the executable's own symbol there is a
  different variable. `gGameMode` is `D_0015F6A8` in names.tsv, but the
  camera code tests the executable's `D_0015F6E8 == 5` where the level
  copy tests `D_L00_0015F6A8 == 5` (the level small data sit 0x40 lower).
  Worth checking for every memory-map name in that range before relying on
  it in executable code.

## 8. Later: other readers

Outside the four camera files (phase 2, not in the pilot): `gCamera` and
its sub-blocks are declared in 18 executable files and 22 level files
(mostly the eye position, `D_00187180` / `D_L00_00166EC0`, 65 uses), the
current camera in 43 files (the shared declaration preamble), and private
views of the same memory in draw.c (`CameraBlock`, `SceneG`), pause.c
(`S`, `PreviewCamera`), draw_001F3A78.c (`View_1F3E20`, `Cam_1F3E20`) and
vendor_002E1660.c (`Moby_2E8210`). The camera-type functions (`Camera3_Update`,
`CameraType5Update`, ...) read `Camera.type_data` and the state buffer;
their layouts are the next header section.

## 9. Next

What the pilot cost, measured: the designer (reads the code, writes
`camera.h`, the plan and the task briefs) used about 354k tokens; the three
Haiku workers that converted the files used about 100k, 214k and 248k. Every
step was checked by the full build (every function exact, image equal to
retail) and `overlay_file_check.py`.

Lessons:

- Haiku does well on one or two bounded tasks per run (one function, one
  table, one file with its checklist). It stalls on long per-file chains
  where each edit depends on the last build; split those into separate runs.
- The design is where the effort goes. With the header, the naming rules
  and the access-path rules (section 6) written down first, the workers had
  little left to decide.
- The sub-block pointer rule (section 6, rule 2) is the one that cost bytes
  every time it was skipped: code that reached a sub-block through its own
  symbol keeps a pointer to it, and a pointer is not replaced by
  `gCamera.field`.
- A rename pass over a tree that other people are naming collides with
  their names (here `Camera_StepFade`, `Camera_UpdateModeFlags`,
  `Camera_SelectActive` and `CameraScript_ResetObject` against the
  `Camera_UpdateFade`, `Camera_PickCollisionMasksLvl`, `UpdateAllCamerasLvl`
  and `Camera_ReleaseCurrent` used here); rebase before a wave and keep
  `names_local.tsv` as the one source.

Candidate next systems, each with the same method (design first, then
workers on bounded tasks, full build after every task):

1. **Hero** (the state block, the per-state functions, the pad/velocity
   fields; include/hero.h is the start): the largest reader population after the camera.
2. **Mobys** (`Moby` and the pvar layouts per class): most numerous, and
   `Moby` already has a header to extend.
3. **HUD** (the draw lists, counters, the menu records).
4. **Weapons / gadgets** (the gadget table, ammo, the per-gadget update
   functions).

For each: map the symbols and users first (as section 1-3 did for the
camera), write the header and the plan, convert the three or four densest
files as the pilot, then fan out.
