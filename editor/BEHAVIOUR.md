# Moby behaviour in GDScript: what was found

A study of letting the modding community write moby behaviour (enemy AI, crate
logic, vendors) as GDScript, with a working prototype in
[behaviour.py](behaviour.py). Numbers come from `behaviour.py census` and
`behaviour.py corpus` run on this checkout.

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
sit in the class number and the pvars. 554 of the 645 are C in the checkout.

## The catalogue for native mods

[catalogue.py](catalogue.py) writes, from the source tree alone, what a native mod
([docs/port/MODS.md](../docs/port/MODS.md)) needs to change a class's behaviour:
[sdk/catalogue/rac1/BEHAVIOUR.md](../sdk/catalogue/rac1/BEHAVIOUR.md) to read, and
`behaviour.json` / `behaviour.tsv` beside it for tools. For each class: its update function
(symbol, readable name, levels, where the port gets its C), the classes sharing it and the
moby's class field to filter on (`s16 oclass`, +0xA6), its pvar structs with field offsets,
types and comments, the game functions the update calls (hook-after candidates), and its
hook status: the hook point, or the `hook` row of `libraries.tsv` that would make one and
the levels that row covers. It ships in the mod SDK's archive.

```sh
python3 editor/catalogue.py write        # regenerate sdk/catalogue/rac1/
python3 editor/catalogue.py check        # exit 1 when it is stale
python3 editor/catalogue.py show 500     # one class, as JSON
```

## The API a script needs

[behaviour_api.tsv](behaviour_api.tsv) lists the helpers and what each does (about 60,
counted over the 554 C update functions). The most used, by number of update
functions that call them: `qcopy` (283), `scale_ticks` (185), `DeleteMoby` (180),
`FastVecAdd` (159), `FastAddRots` (142), `FastVecSub` (124), `VecDistance2` (115),
`Atan2` (104), `FastDecTimer` (104+53), `random_float_between` (102),
`PlayClassSound` (101), `vec_set_length` (100), `FastSin` (89), `FastVecDist` (88),
`set_animation` (73), `FastVecLength` (72), `is_point_inside_clip_volume` (70),
`random_integer_below` (67), `FastCos` (65), `frame_scale` (65), `FastVecScale` (63),
`AddDrawCallback` (62), `get_hit` (55), `int_to_float` (52), `FastAbsF` (51),
`sound_release` (51), `joint_position` (50), `random_angle` (49), `rand_range` (48),
`FastDiffRots` (47). About two thirds are plain maths (vectors, angles, timers,
random numbers) that a script can have in full; the rest are services (collision,
sound, hits, spawning, drawing) that only the running game can answer.

## Three routes

**a. Translate the C to GDScript mechanically.** Done for three classes
(`behaviour.py export`). Over all 554 C update functions the tool translates
**91.2% of 46,105 statements** (`behaviour.py corpus`); 76 functions completely and
242 more at 90% or above. What it cannot: raw `*(T *)(p + off)` memory access (6.2% of
statements; 382 of 554 functions use it somewhere), pointer arithmetic (1.0%),
`goto`/`do` (0.2%), switch fall-through. "Translated" means read and written out,
not proven equal: the check is that the scripts parse and run 5 frames in Godot
against the stub. A modder could change anything in the script, but **it does not
get back into the game**: the port runs the C. It is a reading and previewing aid,
and a source for route b. What breaks: the preview has no level, hero, collision or
sound, so a script runs until it needs one (stubs return neutral values), and the
many functions with raw access are only partly there. Size: the translator is
done in prototype form; widening it to the raw-access functions (reading the byte
offsets as pvars struct fields, which the pvars headers have for about two dozen classes) is
about 2 agent-weeks; a GDScript preview of an enemy that moves against the level
collision is another 2-3 (the editor already has the collision mesh).

**b. A hand-written GDScript API (`RcMoby`) the port calls into.** What the
prototype already is on the Godot side: `RcMoby`, `RcMobyData` (the header, generated
from moby.h), `RcEngine` (the helpers), exported pvars. To make it run in the game,
the script must become something the port executes. The port keeps the game in
guest memory with functions registered by code address
(`openrac_guest_register`, [guest.h](../port/runtime/include/openrac/guest.h)) and
the class-to-update table is in guest memory too (`0x197b00[slot]` is copied to
`moby+0x74` when a moby is made), so replacing one class's behaviour needs no change
to the C: register a native function at a fresh code address and write it into the
table after the level's classes load. Two ways to get the native function:
  1. **GDScript subset to C**, compiled when the port is built (it needs Clang
     anyway). Modders need a compiler; errors show up late. Medium effort: a GDScript
     parser and a C emitter whose helpers are `guest.h` accessors; the prototype's
     translator is the same shape in the other direction.
  2. **GDScript subset to bytecode**, interpreted in the port by a small VM whose
     pvar access reads guest memory at `moby+0x78+offset` and whose helpers call the
     port's own versions of the C helpers. Packed into the level WAD with the
     existing `--mods` mechanism, no compiler on the modder's machine, can be
     hot-reloaded. About 3-4 agent-weeks for the VM and the packer, plus one for
     the helpers' guest-memory shims.
What breaks: scripts depend on the helper semantics matching (float
behaviour, call order, the engine's use of `moby->state` by other code such as
hit handlers). Vanilla behaviour stays C; only a replaced class goes through the
script, so a bad script breaks one class, not the game.

**c. Keep the behaviour in C, expose data.** The editor can already show
`rc1_update` and `rc1_source`. What is missing is exposing per-class parameters.
Candidates are pvars (typed structs exist for about two dozen classes in
[moby_pvars.h](../games/rac1/include/moby_pvars.h); the translated scripts
declare them for the three prototype classes), tables and constants in the C
(speeds, ranges, timers: the crate's `0x1F9` class numbers, the toad's
distances). A modder changes numbers in Godot, the packer writes them into the
gameplay file's pvars (it already copies pvars) or into a patch table the port
reads. This works with no scripting and no new engine risk, but a modder cannot
change *logic*, only values. Size: pvars per instance in the exporter and packer
about 1 agent-week; constants-as-data needs the C edited to read a table, about
1 week for the first dozen classes, then linear.

## Recommendation

Do (c) for pvars first, because it is the one piece all routes need and it is useful
alone: export each moby's pvars as typed, named properties (the script prototypes
already declare the fields with offsets; the instance values come from the gameplay
file), and pack edits back. Then (b2) as the route for real logic, using the
translator (a) as the on-ramp: "translate this class to a script" gives a modder
a working starting point, and the VM runs the edited script in the game. Do not
make GDScript-to-C (b1) the main path because it needs a compiler on every
modder's machine.

Smallest useful version: one class, e.g. the crate, replaced by its script in the
running port, say the fading effect `func_L00_002ADD78` (a timer, a rotation and a
fade, 19 lines of C, classes 112 and 1192): (1) pvars exported per instance, (2) the bytecode VM running a script
with `state`, a handful of pvars and 8-10 helpers (`delete_moby`, `dec_timer`,
`add_rots`, vector maths, `set_animation`, `play_class_sound`), (3) the
table patch. That takes one small class end to end; roughly 4 agents for 2 weeks in
parallel: VM, packer and loader, helper shims, translator widening.

## The prototype

`behaviour.py export <project> --level 0 --classes 357,749,500` writes
`levels/level_00/mobys/behaviour/moby_<class>.gd` and attaches it to the class
scene. Scripts extend `RcMoby` (rc1/rc_moby.gd), whose `rc_preview` export runs
`rc_update` at 60 Hz in the editor. `rc1/behaviour_check.gd` loads every script and
runs five frames:

```sh
godot --headless --path <project> --script res://rc1/behaviour_check.gd
```

| Class | Update function | Statements translated |
|---|---|---|
| 357 crate_fragment | `func_L00_002B0D30` | 32 of 33 (pointer arithmetic on the collision normal) |
| 749 horny_toad | `func_L00_002D5AC0` | 149 of 149 |
| 500 bolt_crate | `func_L00_002D06C8` | 203 of 203 |

The horny toad and crate are 100% by statement because their C is written with typed
structs; 12 and 15 of their statements call engine helpers that are only
stubs here (`RcEngine.unmapped`), so the preview does not play them.
