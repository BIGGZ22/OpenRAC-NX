# Level editor

The editor turns the levels on your own disc of Ratchet & Clank (PAL,
`SCES_509.16` v2.00, or NTSC-U, `SCUS_971.99` v1.00) into a Godot 4 project.
Godot is the map editor: open a level, move, add or delete objects, and save. Tools to pack edited scenes back into game
data come later; the scenes already keep what they will need.

It sits at the top of OpenRAC because it is meant to serve all four games.
Today it reads Ratchet & Clank's PAL and NTSC-U discs; the formats of the
later games are close relatives (see [docs/engine](../docs/engine/README.md)).

Never commit or share what it produces (see [docs/policy/SOURCING.md](../docs/policy/SOURCING.md)).
It only writes to new directories under OpenRAC's ignored `assets/` or `build/`.

## Before you start

You need:

- Python 3.10 or newer (standard library only).
- Godot 4, tested with 4.7.2. On macOS its command is
  `/Applications/Godot.app/Contents/MacOS/Godot`; below it is `godot`.
- An ISO image of your own disc, placed at the top of OpenRAC as
  `baserom/SCES_509.16.iso` (PAL) or `baserom/SCUS_971.99.iso` (NTSC-U)
  ([baserom/README.md](../baserom/README.md)). `baserom/` is ignored by git.
  The commands below name the PAL image; every one of them reads the NTSC-U
  image the same way.

The RAC1 PAL build only needs the executable from that image
([games/rac1/README.md](../games/rac1/README.md)).
The extractor reads the whole image, and it stops unless the executable
inside is PAL v2.00's or NTSC-U v1.00's (its name in `SYSTEM.CNF`, its size
and SHA-1).

## The two discs

Both discs keep their levels the same way, so every reader serves both;
what differs is one table per version in [disc.py](disc.py) (`VERSIONS`):

| | PAL v2.00 | NTSC-U v1.00 |
|---|---|---|
| Executable | `SCES_509.16` | `SCUS_971.99` |
| Table of contents | sector 1500, 0x2960 bytes, levels at 0x28c8 | the same |
| Gameplay copy the game loads (each level header has both) | `gameplay_pal` | `gameplay_ntsc` |
| (cos, sin) table the lighting reads | 0x165600 | 0x165500 (the same values) |

The NTSC-U side follows ReRAC's disc layout (`docs/formats/disc_layout.md`)
and the NTSC decomp's `load_level_gameplay`, which loads the NTSC copy
unless the game runs in PAL mode. The two discs' levels are the same
levels with small differences of their builds: on 2026-10-16 an export of
each had the same tie, shrub and moby placements and terrain in every
level, while NTSC-U's moby class 1138 (loaded in every level) has three
animations fewer, most levels have one moby texture fewer, a few collision
cells and surfaces differ, and level 17 loads four moby classes fewer
(Drek's and Qwark's among them).

## Extract

```sh
python3 editor/extract.py godot baserom/SCES_509.16.iso assets/levels/rac1 --split
godot --path assets/levels/rac1/level_00 -e
```

`--split` writes a Godot project per level, `level_00` to `level_18`, each
named after its planet. The second command opens Veldin in the editor. A
project holds one level, so Godot imports it in about five seconds the
first time it is opened, and only that level is loaded. This is what the
launcher's Mods page uses.

Without `--split` the 19 levels go into one project (`assets/godot` in
earlier versions of this page): the other levels are then in the FileSystem
dock under `levels/`, and Godot's first import takes about a minute and a
half.

All 19 levels take about 20 seconds and 800 MB to extract, one level per
CPU core at a time. Options:

- `--level N` (repeatable) exports only some levels.
- `--jobs N` limits how many levels are exported at once.
- `--terrain-lod 2` uses the coarsest terrain.

## Regenerate

Nothing the extractor writes is kept in git, so a fresh checkout has none
of it: run the command above to make it. The same disc always gives
byte-identical files. To rebuild a project, delete its directory and run
the command again; the extractor never overwrites an existing directory.
Godot rebuilds its import cache (`.godot/`) the next time it opens the
project.

Regenerating replaces the project, and with it any edit that has not been
packed (below). To keep an edited level across a regeneration:

1. Copy its scene, `levels/level_NN/level_NN.tscn`, somewhere safe.
2. Regenerate.
3. Copy the scene back. The meshes and textures it refers to come out
   identical, so its paths still resolve.

That only works when the extractor hasn't changed since the scene was
made. A newer extractor can write scenes with other nodes (the sky's
WorldEnvironment, for instance), so after updating it, move your changes
into the new scene instead. Godot saves a scene as text, so a diff
against a fresh export shows each edit.

## What a moby is, and what runs it

A placed moby's node is named after its class: `Vendor_0012`, `Bolt_0003`,
`BlargHeavyInterceptor_0040` (568 classes have a name, from
[moby_classes.tsv](moby_classes.tsv); the rest show as `Class809_0001`).
Select one and the Inspector shows its values as `rc1_*` metadata (spawn id,
group, colour, draw distance...), which the packer writes back.

Its class scene (`levels/level_NN/mobys/moby_<class>.tscn`) says what the
class is and what runs it, in its tooltip in the Scene dock and as metadata:

| Metadata | |
|---|---|
| `rc1_class`, `rc1_class_name` | the class number and name |
| `rc1_update` | the function the game calls for each moby of the class, every frame |
| `rc1_source` | where that function is in the decompilation ([games/rac1](../games/rac1)), and the tooltip says whether it is C there yet |

That function is the moby's behaviour: an enemy's attack pattern, a crate
breaking, a vendor's menu. It is ordinary C in the decompilation, and the
native port is built from that C, so **changing a moby's logic is editing
that function and building the port again** (the launcher's Build the game).
There is no scripting layer on top of it yet: no way to change behaviour
from Godot or without a compiler. The table comes from the decompilation's
`config/overlays/names.tsv` (its `UpdateMoby_<class>` roles), per level, so
it is only written when `games/rac1` is in the checkout.

## Moby behaviour as GDScript (prototype)

```sh
python3 editor/behaviour.py export <exported level project> --level 0 --classes 357,749,500
```

Writes a GDScript per moby class into `levels/level_NN/mobys/behaviour/` and attaches it to
the class scene: the class, its update function and source, its pvars as typed exported
variables with their offsets, and the update function translated from the decompiled C (what
could not be translated is a `pass  # not translated` line saying why). The scripts run against
the `RcEngine` stub in `rc1/` for reading and preview; the port still runs the C. See
[BEHAVIOUR.md](BEHAVIOUR.md) for what was found and the routes to getting scripts into the game.

## Pack your edits

```sh
python3 editor/extract.py pack baserom/SCES_509.16.iso assets/levels/rac1 assets/packed/rac1
python3 editor/extract.py pack baserom/SCUS_971.99.iso assets/levels/rac1-ntsc assets/packed/rac1-ntsc
```

Turns what was edited in Godot back into the game's level data (the folder
of per-level projects, or a single project with every level), from the disc
the project was extracted from. For each level whose scene differs from the
disc it writes that level's gameplay file as the game stores it, compressed,
and the list of disc sectors it replaces:

```
assets/packed/rac1/
  level_NN/gameplay_<version>.wad   one per level whose mobys were edited, the size of its place on the disc
  level_NN/placements.json    one per level whose ties or shrubs were edited, for the native port
  patches.tsv                 first sector, sector count, file
  changes.txt                 what changed, in words
  packed.json                 the disc it was packed from
```

Each level header has a gameplay copy per version, and each disc's game
loads its own (`gameplay_pal` or `gameplay_ntsc`, the table above): the
packer reads and replaces that one, so the sectors in `patches.tsv` are the
disc's own. A packed folder serves the disc it was packed from only; the
Mods page stacks it for that disc and leaves it out for the other
([overrides.py](overrides.py)).

The disc image is never written. The native port reads the folder with
`--mods assets/packed/rac1` and serves those sectors from the files
([port/game/common/lib/cdvd.c](../port/game/common/lib/cdvd.c)); the
launcher's Mods page does all three steps (edit, pack, play).

What is packed today: **mobys that were moved, turned, scaled, given another
class, given other values** (the `rc1_*` metadata in the Inspector), **added
or deleted**, and **ties and shrubs** the same way (moved, turned, scaled
each axis its own, another class of the level, other values, added, deleted).
A moby keeps one scale for all three axes. Not yet: terrain, and anything of
a class the level does not load.

Ties and shrubs are packed for the native port only ([scenery.py](scenery.py)):
the port draws them from the extractor's files, so the pack writes the
level's `placements.json` with the lists that changed, each tie and shrub lit
again where it stands, and `--mods` reads it in place of the level's own.
The gameplay file's own tie and shrub tables and the level's collision stay
the disc's: a tie that was moved is drawn in its new place and still walked
on in its old one, and a new one is walked through.

An added moby is a copy made in Godot (Ctrl+D) or one of the level's class
scenes dropped under `Game/Mobys`. It takes after the moby it copies (or the
level's first moby of its class): the same values, a spawn number of its own,
a copy of that moby's pvars, and a place in its group. A deleted moby leaves
the table, and the groups and pvar links that name mobys by their place in
it follow ([gameplay.py](gameplay.py)). A level the game gives more mobys
than it has room for is not caught yet: the room is what the level's own
"spare" count and memory allow.

How it stays exact ([pack.py](pack.py), [wadpack.py](wadpack.py)):

- A moby the scene has as the extractor wrote it keeps its bytes from the
  disc. A project nobody edited packs to nothing, and an edited level's file
  differs from the disc's only in the records that were edited. Godot's own
  way of saving a scene (other float rounding, `unique_id`, `rc1_class` on
  every instance) is not an edit.
- The compressor writes streams the way the retail ones are laid out,
  measured on all 19 gameplay files: 0x2000-byte blocks that no packet
  crosses, each ending with a skip and unused room after it, and no literal
  run after a literal run. Its output is smaller than retail's for every
  level, so a packed level always fits where the original is.

Checked on 2026-10-12: all 19 gameplay files round-trip through the
compressor; a scene re-saved by Godot 4.7.2 with one moby moved packs to a
file five bytes from the disc's, all in that moby's record; the port ran
level 0 for 400 frames from it. Whether the moved moby shows where it was
put has not been looked at in the running game yet. Checked on 2026-10-16
on the NTSC-U disc: level 0 extracted from it, one bolt moved in the scene,
packed to `level_00/gameplay_ntsc.wad` at the NTSC copy's sectors, which
reads back as the disc's copy but for two bytes of that bolt's record; all
19 NTSC gameplay copies round-trip through the compressor too.

## Models and textures for modders

```sh
python3 editor/assets.py export assets/port/rac1/pal build/my-export --level 0 --what mobys
python3 editor/assets.py import build/my-export --levels assets/port/rac1/pal --out build/my-pack
```

Exports a level's moby, tie and shrub models as glTF (mesh, UVs, vertex
colours, materials, skeleton, inverse bind matrices, weights, animations) and
its textures as PNG, each with a manifest; reads the edited files back as an
overrides pack ([overrides.py](overrides.py)) that the port loads with
`--mods`. [MODDING.md](MODDING.md) is the guide: editing in Blender,
upscaling, re-importing, the pack format. The launcher's Mods page does it
all.

## Other commands

```sh
python3 editor/extract.py survey baserom/SCES_509.16.iso      # disc layout, as JSON
python3 editor/extract.py raw baserom/SCES_509.16.iso build/level-00 --level 0
python3 editor/extract.py port baserom/SCES_509.16.iso build/port-levels --level 0
```

`port` writes each level for the native port's level viewer
([port/viewer](../port/viewer/README.md)): the same meshes and textures as
the Godot export, the sky as a mesh, and the placements as JSON. Its
manifests say which disc they came from (`"version": "pal"` or `"ntsc"`);
the format is the same for both. Each version's export has a folder of its
own under one root: `assets/port/rac1/pal` and `assets/port/rac1/ntsc` in a
checkout, `<data>/port/pal` and `<data>/port/ntsc` in an installed launcher
(the game reads `<data>/port/<version>` without `--levels`). An export made
before that layout, straight in `assets/port/rac1` or `<data>/port`, moves
into `pal/` the next time the PAL levels are extracted with `--replace`.

```sh
python3 editor/extract.py port baserom/SCUS_971.99.iso assets/port/rac1/ntsc   # NTSC-U, every level
```

`raw` writes one level's sections as stored on the disc and decompressed,
with a manifest of their sizes and hashes.

## The project

```
project.godot
rc1/level.gd                 fly camera when a level is run
rc1/check.gd                 headless check against level.json
levels/level_NN/
  level_NN.tscn              the level
  level.json                 what was extracted: counts, bounds, code overlay
  terrain.glb                one node per terrain fragment
  collision.glb              the collision mesh, coloured by surface type
  ties/tie_<class>.glb       one mesh per tie class
  shrubs/shrub_<class>.glb   one mesh per shrub class
  mobys/moby_<class>.tscn    a moby class: its mesh (or a box) and a label
  mobys/moby_<class>.glb     a moby class's mesh, skeleton and animations
  sky.png                    the sky as a panorama, used by the WorldEnvironment
  textures/*.png             shared by the level's meshes
```

Each level scene looks like this:

```
Level_NN
  WorldEnvironment      the sky (sky.png) and flat ambient light
  Sun                   a directional light, so shapes read in the editor
  Game                  turns the game's Z-up axes into Godot's Y-up
    Terrain             fragments Terrain_000... (editable children)
    Collision           the collision layer, hidden until you show it
    Ties/Tie_NNNN       one node per placed tie
    Shrubs/Shrub_NNNN   one node per placed shrub
    Mobys/Vendor_0012   one node per placed moby (crates, enemies, NPCs...), named
                        after its class and its place in the level's table
```

To edit a level:

- Move, rotate or scale objects as usual. Transforms under `Game` are in
  game units and axes.
- To add an object, drag a class `.glb` from the FileSystem dock onto
  `Game/Ties` or `Game/Shrubs`. The file is the object's class.
- Per-object game fields are in the Inspector's Metadata section:
  - `rc1_index`: the object's record in the original level (new objects
    have none);
  - `rc1_draw_distance`, `rc1_directional_lights`;
  - ties only: `rc1_occlusion_index`, `rc1_uid`;
  - shrubs only: `rc1_colour`, raw integers;
  - `rc1_matrix_w`: only on objects that store 0.0 instead of the usual 0.01.
- Mobys show as their class's mesh in its bind pose, with a label giving
  the class number and name (`11 vendor`) that fades out beyond 60 units.
  Classes without a mesh (spawn points, triggers, particle spawners...)
  show as a coloured box. Each class scene, `mobys/moby_<class>.tscn`,
  has one for every class the level loads, placed or not; drag one onto
  `Game/Mobys` to add a moby. Its `Model` node carries the class scale.
  A moby's rotation in the Inspector is the game's own Euler angles
  (rotation order XYZ).
- Animated classes have a `Skeleton3D` in the bind pose and an
  `AnimationPlayer` with one animation per sequence: `seq_NN` by slot, and
  Ratchet's `ratchet_seq_NNN`. Sequences that loop in the game loop here.
  To preview one, double-click `mobys/moby_<class>.glb` in the FileSystem
  dock: Godot's import settings window plays each animation. In a scene,
  enable Editable Children on the `Model` node to reach the
  `AnimationPlayer`.
- A moby's metadata:
  - always `rc1_index`, `rc1_spawn_id`, `rc1_group`, `rc1_pvar_index` and
    `rc1_colour` (the ambient colour, 128 = 1.0);
  - the other fields only when they differ from what most mobys store:
    `rc1_spawn_flags` (0), `rc1_draw_distance` and `rc1_update_distance`
    (64), `rc1_mode_bits` (32), `rc1_occlusion` (1), `rc1_light` (0),
    `rc1_unknown_74` (-1), and a few unknown fields
    ([ASSETS.md](../games/rac1/docs/ASSETS.md#mobys)).
- Textures are ordinary PNGs. Editing one changes every mesh that uses it.

Run a level (F6) to fly around it:

| Key | Action |
|---|---|
| Right mouse | Look |
| WASD | Move |
| Q / E | Down / up |
| Shift | Faster |
| Mouse wheel | Change speed |
| F | Frame the terrain |
| C | Show or hide the collision layer |

## Collision layer

`Game/Collision` is the level's baked collision mesh: the triangles the game
tests Ratchet, bolts and shots against, with ties and shrubs already
included. It is hidden by default (`visible` is off) so it does not hide the
terrain. In the editor, click its eye icon in the Scene dock; in a running
level press C. It is one translucent, unshaded mesh with a colour per
surface, so it reads over the terrain. Triangles are double-sided here, but
the game treats them as one-sided.

The colour is the surface id, the low five bits of each face's type byte
(the other bits choose footstep sounds and exclude the face from some
queries). The game has no table of surface types, so the names are
inferences from how its code tests them, and most ids are only colours:

| Id | Colour | What is known |
|---|---|---|
| 31 (0x1f) | grey `#8c8c8c` | no special surface; most of every level |
| 0 | blue `#3373ff` | water surface (the ground probe looks for the floor below it) |
| 8, 9, 10, 12 | orange `#f27130`, `#30f2aa`, `#e230f2`, blue `#3091f2` | common; the game's wall and ledge checks test for these ids |
| 11 (0x0b) | `#caf230` | the ground snap is skipped (some special floor) |
| 13 (0x0d) | `#f23058` | liquid-like (level 12 only) |
| 1-7, 14 | other hues | used in many levels; meaning unknown |

The other ids are coloured by hue from the id. Each level's `level.json`
lists its surfaces and how many triangles use each.

The mesh is decoded from the level's collision block with every cell's
faces merged, so a face that straddles several cells appears once.
Vertices are quantised (1/16 unit across, 1/64 up), which leaves tiny
gaps and some doubled faces. Hero-only walls and fences (invisible walls
that only block the player) are in the block too, but they are not
extracted yet.

## Checks

```sh
python3 -m unittest discover -s editor                         # synthetic data only
godot --headless --path assets/godot --import
godot --headless --path assets/godot --script res://rc1/check.gd
```

The Godot check loads every level scene and compares its meshes, triangles,
textures, bounds, sky panorama and moby count with what the extractor wrote.
The collision layer is not counted with the textured meshes; its triangles
are compared with `level.json` on their own, and it must be hidden.
It also loads every moby class scene and compares their models, triangles,
textures, skeletons, bones and animations, and the meshes the placed mobys
show.

## Coverage

Extracted:

- terrain (tfrags);
- ties and shrubs, with their placements;
- moby placements, and each moby class's high-detail mesh, skeleton and
  animations;
- the sky;
- the collision mesh, as a hidden layer;
- the textures they use.

Approximations:

- Normals are flat, and Godot lights the scene. The game's own lighting,
  baked into vertex colours, is not decoded yet. Mobys use their stored
  vertex normals; the game lights them at run time.
- Alpha-tested textures become cutouts.
- Moby faces the game draws untextured get a flat grey texture, which is
  what the game binds for them. Some classes are made only of such faces
  (rings, force fields, barriers); the game may draw those translucent,
  glowing or not at all. Glow parts are lit like the rest, and the chrome
  and glass passes and the low-detail meshes are not extracted.
- Moby animations run at the game's 60 Hz tick, as ReRAC infers it. Godot
  interpolates rotation keys spherically where the game lerps, and a joint
  channel that stays within 1e-5 of the bind pose (1e-4 for rotations) is
  left out of an animation. The two helper classes 1 and 2, which animate
  joints of another skeleton, stay static. The gadget classes (the wrench
  and weapons in Ratchet's hand) are not extracted.
- The sky is baked into a 2048×1024 panorama. The game centres its sky
  shells on the camera and never moves them, so a panorama loses nothing.
  Their blending follows the PS2's usual texture and alpha modes, which is
  inferred rather than traced in the game's code. Sprites the game adds
  to the sky at run time are not included.

Not yet extracted: low-detail moby meshes, hero collision, audio, video
and each level's code overlay. [ASSETS.md](../games/rac1/docs/ASSETS.md) describes
the formats and the evidence for them.

## Code

The GDScript files follow [`GDSCRIPT_CONVENTIONS.md`](GDSCRIPT_CONVENTIONS.md).

| File | Contents |
|---|---|
| `extract.py` | Command line and safe output |
| `disc.py` | ISO files, the two discs' versions, sector table of contents, level headers |
| `level.py` | A level's sections, core blocks and textures |
| `formats.py` | Bounded reads, WAD decompression, code overlays, textures |
| `terrain.py`, `ties.py`, `shrubs.py`, `sky.py`, `collision.py` | Geometry and placements |
| `mobys.py`, `moby_class.py`, `moby_anim.py` | Moby placements, class meshes, skeletons and animations |
| `mesh.py`, `gltf.py` | The mesh type and the GLB writer |
| `gltf_read.py`, `pngio.py` | Reading glTF and PNG files back (a modder's edits) |
| `assets.py`, `overrides.py` | Export for modding and re-import; overrides packs ([MODDING.md](MODDING.md)) |
| `godot.py` | Project, scenes and level.json |

The decoders accept only the layouts found on these discs and raise
`FormatError` on anything else. Nothing is skipped silently.

## Credits

- **[Wrench](https://github.com/chaoticgd/wrench)** by chaoticgd and
  contributors (GPL-3.0-or-later). Most of what the extractor knows about
  terrain, ties, shrubs, the sky, textures and placements comes from
  reading its source. [ASSETS.md](../games/rac1/docs/ASSETS.md#sources-and-credits)
  lists which files each format came from.
- **[Replanetizer](https://github.com/RatchetModding/Replanetizer)** by
  RatchetModding contributors, consulted for what fields mean.
- **[ReRAC](https://github.com/re-rac/rerac)** (ISC): the moby instance
  record and what each field does in the game's level loader, the moby
  class format (packets, vertex cache, skinning slots, normals, the
  untextured faces, skeletons and animation sequences), and the collision
  block and the meaning of its surface bytes
  ([ASSETS.md](../games/rac1/docs/ASSETS.md#collision)).
- **[Lombyte](https://github.com/lombyte-project/Lombyte)** (MIT): the moby
  class names on the labels (`moby_classes.tsv`, see its header).
- **[OpenGOAL's jak-project](https://github.com/open-goal/jak-project)**,
  whose extractor was the model for extracting from your own disc.
- **[Godot Engine](https://godotengine.org)** and the Khronos Group's
  **[glTF 2.0](https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html)**,
  which the output is built for.

No code from these projects is included; the extractor is newly written.
