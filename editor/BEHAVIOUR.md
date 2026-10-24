# Moby behaviour

How a moby's behaviour is made in Ratchet & Clank (PAL), and where modders change it. Behaviour is
modded through the mod SDK ([docs/port/MODS.md](../docs/port/MODS.md): native mods and Luau
scripts on the port's hook points), never in the level editor: Godot edits levels and their data
(placements, pvars, volumes, paths, cameras, collision), and nothing in a Godot project runs game
logic. The catalogue below is experimental: its names follow the decompilation and will change in
the C++ refactor.

## What a moby's behaviour is

The game (RAC1, PAL) keeps 0x100-byte moby records in a table. Per frame, for each
active moby (state below 0xFD, not flagged "no update"), the moby loop does three
things (ReRAC's docs/plan/moby_update_catalogue.md, section 1):
advance the animation, call `moby->update(moby)`, rebuild the matrix. **The only argument
is the moby.** Everything else is in its state:

| Part | Where | Example |
|---|---|---|
| header: position, rotation, scale, state byte (0x20), flags, class, animation (seq, frame) | `struct Moby`, [moby.h](../games/rac1/include/moby.h) | `moby->state = 1` |
| private variables (pvars) | `moby->pvars` (+0x78), a per-class struct from the level's gameplay file | `pv->rider`, `data->velocity` |
| mode / state bits | `moby->state` is the class's own state machine (a `switch` on it); `flags` has engine bits | `moby->flags \|= 0x41` |
| animation | `moby->seq`, `set_animation(moby, seq, frame, blend)` | `func_00213DE0` |
| collision and hits | `coll_sphere`, `coll_line`, `get_hit(moby, mask, keep)` returns a hit message | crate breaking |
| spawning and deleting | `create_moby(class)`, `delete_moby(moby)` | debris, bolts |
| sound | `play_class_sound(n, ?, moby)`, handles released with `sound_release` | |
| messages to other mobys | `coll_sphere_mobys(r, pos, mask, owner, burst)` delivers a hit message to every moby in range; group iteration `group_first/next` | explosions |
| globals | frame counter, dt (1/50 PAL), hero record, level number | `D_0015EE6C` = dt |

Update functions are shared: **645 distinct update functions serve 1,167 class roles**
(bolts 13-16 share one; crates 500/501/502/505/511 share one). Per-class differences
sit in the class number and the pvars. All 645 have C in the port's build
(the catalogue counts them).

## The catalogue for mods

[sdk/tools/catalogue.py](../sdk/tools/catalogue.py), one of the SDK's tools that reads the
decompilation through this folder's `cparse.py` and `mobys.py`, writes, from the source tree
alone, what a native mod ([docs/port/MODS.md](../docs/port/MODS.md)) needs to change a class's
behaviour:
[sdk/catalogue/rac1/BEHAVIOUR.md](../sdk/catalogue/rac1/BEHAVIOUR.md) to read,
`classes.json` (the stable table the Luau binding is made from) and `behaviour.json` /
`behaviour.tsv` beside it for tools. For each class: its update function
(symbol, readable name, levels, where the port gets its C), the classes sharing it and the
moby's class field to filter on (`s16 oclass`, +0xA6), its pvar structs with field offsets,
types and comments, the game functions the update calls (hook-after candidates), and its
hook status: the hook point, or the `hook` row of `libraries.tsv` that would make one and
the levels that row covers. It ships in the mod SDK's archive.

```sh
python3 sdk/tools/catalogue.py write        # regenerate sdk/catalogue/rac1/
python3 sdk/tools/catalogue.py check        # exit 1 when it is stale
python3 sdk/tools/catalogue.py show 500     # one class, as JSON
```

## The helpers update functions call

[engine_helpers.tsv](engine_helpers.tsv) lists the engine helpers and what each does (about 60,
counted over the update functions' C; the catalogue marks them so its hook-after candidates
leave them out). The most used, by number of update
functions that call them: `qcopy` (283), `scale_ticks` (185), `DeleteMoby` (180),
`FastVecAdd` (159), `FastAddRots` (142), `FastVecSub` (124), `VecDistance2` (115),
`Atan2` (104), `FastDecTimer` (104+53), `random_float_between` (102),
`PlayClassSound` (101), `vec_set_length` (100), `FastSin` (89), `FastVecDist` (88),
`set_animation` (73), `FastVecLength` (72), `is_point_inside_clip_volume` (70),
`random_integer_below` (67), `FastCos` (65), `frame_scale` (65), `FastVecScale` (63),
`AddDrawCallback` (62), `get_hit` (55), `int_to_float` (52), `FastAbsF` (51),
`sound_release` (51), `joint_position` (50), `random_angle` (49), `rand_range` (48),
`FastDiffRots` (47). About two thirds are plain maths (vectors, angles, timers,
random numbers); the rest are services (collision,
sound, hits, spawning, drawing) that only the running game can answer.
