# Level editor

The editor turns the levels on your own disc of Ratchet & Clank (PAL,
`SCES_509.16` v2.00, or NTSC-U, `SCUS_971.99` v1.00) into a Godot 4 project.
Godot is the map editor: open a level, move, add or delete objects, and save. Tools to pack edited scenes back into game
data come later; the scenes already keep what they will need.

It sits at the top of OpenRAC because it serves all four games. It reads
Ratchet & Clank's PAL and NTSC-U discs in full, and Going Commando's, Up
Your Arsenal's and Deadlocked's NTSC-U discs in early access: their levels,
textures and models, to look at and to make texture and model packs from
([The later games](#the-later-games-early-access)).

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
and SHA-1, which it reads from the version's entry in
[games/rac1/game.json](../games/rac1/game.json), as it does for the later games'
discs from theirs).

## The two discs

Both discs keep their levels the same way, so every reader serves both;
what differs is one table per version in [disc.py](disc.py) (`VERSIONS`;
the executable's name, size and SHA-1 come from `games/rac1/game.json`):

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
python3 editor/extract.py godot baserom/SCES_509.16.iso assets/levels/rac1/pal --split
godot --path assets/levels/rac1/pal/level_00 -e
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
breaking, a vendor's menu. It is C in the decompilation, and the native port
is built from that C. Behaviour is not edited in Godot: the level editor edits
levels and their data, and nothing in a project runs game logic. A class's
behaviour is changed by a mod through the mod SDK, on the port's hook points
([docs/port/MODS.md](../docs/port/MODS.md)); the SDK's behaviour catalogue
([BEHAVIOUR.md](BEHAVIOUR.md), `sdk/catalogue/rac1/`) names each class's update
function, its hook point and its pvar fields. The table comes from the
decompilation's `config/overlays/names.tsv` (its `UpdateMoby_<class>` roles),
per level, so it is only written when `games/rac1` is in the checkout.

## Pack your edits

The modder's guide to the whole round trip (extract, edit in Godot, pack,
play from the launcher) is [LEVEL_MODDING.md](LEVEL_MODDING.md).

```sh
python3 editor/extract.py pack baserom/SCES_509.16.iso assets/levels/rac1/pal assets/packed/rac1/pal
python3 editor/extract.py pack baserom/SCUS_971.99.iso assets/levels/rac1/ntsc assets/packed/rac1/ntsc
```

Turns what was edited in Godot back into the game's level data (the folder
of per-level projects, or a single project with every level), from the disc
the project was extracted from. For each level whose scene differs from the
disc it writes that level's gameplay file as the game stores it, compressed,
and the list of disc sectors it replaces:

```
assets/packed/rac1/pal/
  level_NN/gameplay_<version>.wad   one per level whose mobys were edited, the size of its place on the disc
  level_NN/placements.json    one per level whose ties or shrubs were edited, for the native port
  level_NN/core_<version>.bin  one per level whose collision (the level's or a moby class's) was edited: the level data's sectors of its core data
  level_NN/index_<version>.bin with it, when the core data grew (a collision moved to its end): the core index's sector
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
`--mods assets/packed/rac1/pal` and serves those sectors from the files
([port/game/common/lib/cdvd.c](../port/game/common/lib/cdvd.c)); the
launcher's Mods page does all three steps (edit, pack, play).

What is packed today: **mobys that were moved, turned, scaled, given another
class, given other values** (the `rc1_*` metadata in the Inspector), **added
or deleted**, and **ties and shrubs** the same way (moved, turned, scaled
each axis its own, another class of the level, other values, added, deleted),
**the gameplay data**: pvars, links between objects, moby groups, volumes,
paths, grind paths, cameras, sound instances and the level settings
([Gameplay data](#gameplay-data)), **the collision**: tiles and hero walls
deleted, moved, edited or added ([Collision](#collision)), and **moby class
collision**: a class's spheres, cylinders, capsules and triangles, for every
moby of the class in that level ([Object collision](#object-collision-moby-classes)). A moby keeps one scale for all three axes.
Not yet: terrain, and anything of a class the level does not load (the
port draws a denser or reshaped terrain from an overrides pack instead:
[MODDING.md](MODDING.md#editing-the-terrain)).

Ties and shrubs are packed for the native port only ([scenery.py](scenery.py)):
the port draws them from the extractor's files, so the pack writes the
level's `placements.json` with the lists that changed, each tie and shrub lit
again where it stands, and `--mods` reads it in place of the level's own.
The gameplay file's own tie and shrub tables stay the disc's, and the level's
collision is baked from the ties and shrubs: a tie that was moved is drawn in
its new place and still walked on in its old one until its collision is moved
too (the tiles, below), and a new one is walked through until collision is
added for it.

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
colours, materials, skeleton, inverse bind matrices, weights, animations), its
terrain (`--what terrain`: one node per fragment, any vertex count coming
back) and its textures as PNG, each with a manifest; reads the edited files back as an
overrides pack ([overrides.py](overrides.py)) that the port loads with
`--mods`. Either disc's levels (`assets/port/rac1/ntsc` the same way), and a
pack made from one disc's applies to the other's where it matches. [MODDING.md](MODDING.md) is the guide: editing in Blender,
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
project.godot                enables the Collision dock ([editor_plugins])
rc1/level.gd                 fly camera when a level is run
rc1/check.gd                 headless check against level.json
addons/openrac_collision/    the Collision dock, an editor plugin (Collision, below)
levels/level_NN/
  level_NN.tscn              the level
  level.json                 what was extracted: counts, bounds, code overlay
  terrain.glb                one node per terrain fragment
  collision/tile_XX_YY.glb   the collision of a 64-unit square, a material per type byte
  collision/hero_NNN.glb     a hero wall (a hero group)
  collision/collision.json   what was written: each file's name, SHA-1 and triangles
  ties/tie_<class>.glb       one mesh per tie class
  shrubs/shrub_<class>.glb   one mesh per shrub class
  mobys/moby_<class>.tscn    a moby class: its mesh (or a box), a label and its collision (hidden)
  mobys/moby_<class>.glb     a moby class's mesh, skeleton and animations
  mobys/collision/moby_<class>_XX.glb   a class's collision triangles of type byte XX
  mobys/collision.json       the class scenes and collision files as written (SHA-1)
  gameplay/<kind>.tscn       a volume's unit shape (cuboid, sphere, cylinder, pill), or a
                             camera's, sound's or the ship's marker
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
    Collision           the collision, by 64-unit tile, hidden until you show it (Collision, below)
    HeroWalls           the walls only the player meets, hidden too
    Ties/Tie_NNNN       one node per placed tie
    Shrubs/Shrub_NNNN   one node per placed shrub
    Mobys/Vendor_0012   one node per placed moby (crates, enemies, NPCs...), named
                        after its class and its place in the level's table
    LevelSettings       the gameplay data (Gameplay data, below): fog, death height,
    MobyGroups, Cameras,  ship; groups; cameras; sounds; volumes; paths
    Sounds, Volumes,
    Paths, GrindPaths
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

## Gameplay data

RAC1 moby behaviour is C in the port, but each level's gameplay file says
what every object starts with, and those values are in the scene, editable,
and packed back like the mobys ([gamedata.py](gamedata.py); the formats are in
[LEVEL.md](../docs/engine/formats/LEVEL.md#8-the-gameplay-file)):

| Node | What | Packed |
|---|---|---|
| `Game/LevelSettings` | `rc1_background`, `rc1_fog_colour` (r, g, b), `rc1_fog_near`, `rc1_fog_far` and their `_intensity`, `rc1_death_height`, `rc1_ship_path`, `rc1_ship_camera_cuboids` (first, last) | values |
| `Game/LevelSettings/Ship` | where the player's ship lands; turn it about Z only | moved |
| `Game/MobyGroups` | `rc1_group_<n>`: each group's members, by node name | members |
| `Game/Cameras/Camera_NNNN` | position and rotation (the game's Euler angles), `rc1_class`, pvars | edited in place |
| `Game/Sounds/Sound_NNNN` | the record's matrix, `rc1_class`, `rc1_m_class`, `rc1_range`, pvars (hidden) | edited in place |
| `Game/Volumes/Cuboids/Cuboid_NNNN`, `Spheres`, `Cylinders`, `Pills` | the unit shape through the record's matrix, translucent (hidden until you show `Volumes`) | moved, turned, scaled, added |
| `Game/Paths/Path_NNNN` | a `Path3D`: the points in its curve, `rc1_w` each point's fourth float | points moved, added or removed; paths added |
| `Game/GrindPaths/GrindPath_NNNN` | as paths, with `rc1_bounding_sphere` (x, y, z, r) and `rc1_flag` | the same |

On a moby, camera or sound with a pvar block (its class's private
variables, `rc1_pvar_index`):

- `rc1_pvar`: the block as 32-bit words. A number with a decimal point or an
  exponent packs as a float, any other as an integer; the list keeps its
  length (blocks are not resized).
- `rc1_pv_<field>`: the fields the decompilation names, typed (the classes'
  `struct update<class>` in games/rac1's
  [moby_pvars.h](../games/rac1/include/moby_pvars.h), 19 classes, and the
  header every camera block starts with: `rc1_pv_sphere`, `_cuboid`,
  `_cylinder`, `_path` name the volumes and path the camera's region test
  uses, `rc1_pv_activation` when it switches on). A field and the word under
  it are the same bytes: edit either; when both change, the field wins.
- `rc1_link_<offset>`: a moby link in the block (the gameplay file's +0x50
  list), as the name of the moby node it names, or `"-1"` for none. Name
  another moby node to point it there; a link follows its moby when mobys
  are added or deleted.
- The words the level loader rewrites (moby links, pointers into the block,
  addresses of shared data) are not packed from `rc1_pvar`: links through
  `rc1_link_*`, the others not at all.

A copy of a moby (Ctrl+D) gets a copy of its pvar block, and its own
`rc1_pv*` values pack into the copy. Moby groups: removing a name from
`rc1_group_<n>` or adding one moves that moby (its `rc1_group`), and each
group then lists the mobys whose `rc1_group` names it, so setting a moby's
`rc1_group` moves it between the lists too (in a project exported with
`MobyGroups`; an older one packs its groups as before).

Volumes, paths and grind paths are named by their place in their table by
other data (camera blocks, moby pvars, the camera collision grid, the level
settings), so they are never deleted or renumbered: a node deleted in Godot
is packed as it was, with a note in `changes.txt`. A new one (a copy, or
`gameplay/<kind>.tscn` dropped under its group, or a new `Path3D` under
`Game/Paths`) goes after the table's own; when the section no longer fits
where it was it moves to the end of the file. Cameras and sound instances
are edited in place only. An edited volume's inverse matrix and Euler angles
are computed again; an edited grind path keeps its bounding sphere when it
still holds every point, and grows it just enough otherwise (the disc's own
rule is not known). Curve handles and tilts are not the game's: paths are
polylines, so they are ignored, and a point added in Godot gets the fourth
float -1, which nearly every path point on the disc has (on grind paths it
varies per point; its meaning is not established).

"Triggers" in RAC1 are mostly these volumes: cameras switch by the cuboid,
sphere or cylinder of their block's header, many moby classes keep a cuboid
or path index in their pvars (shown as words, or as a typed field where the
layout is known), the level settings name the ship's camera cuboids, and
the camera collision grid (not exported) lists volumes per 16-unit cell.
Environment transitions (fog zones) have matrices of their own and are not
exported yet.

As with mobys, what the scene has as the extractor wrote it keeps its bytes:
an untouched project packs to nothing, and Godot's own saving is not an
edit. Values that cannot be packed (not a number, out of the field's range,
a link to no moby, an index past its table) are listed as `not packed:` in
`changes.txt` and leave the bytes as they were.

Checked on 2026-10-21 on the PAL disc: every section above, read and written
again unchanged, gives its bytes back in all 19 levels; each level exported
and packed untouched gives the disc's gameplay file; and one edit of each
kind (a pvar word, a typed field, a link, a group, a camera, a sound's range,
a cuboid moved and one added, a path point and its fourth float, a grind
path, the death height, the ship) on levels 3 and 16 changed only that
record's bytes (or, for an added cuboid, the table moved to the end of the
file).

## Collision

`Game/Collision` is the level's baked collision: the triangles the game tests
Ratchet, bolts and shots against, with ties and shrubs already included.
`Game/HeroWalls` holds the walls only the player meets (fences, invisible
walls: the block's hero groups). Both are hidden by default (`visible` is off)
so they do not hide the terrain. In the editor, click the eye icon in the Scene
dock; in a running level press C. They are translucent and unshaded, with a
colour per surface, so they read over the terrain. Triangles are double-sided
here, but the game treats them as one-sided: a triangle faces the side its
corners turn counter-clockwise from, as in any glTF file.

The colour is the surface id, the low five bits of each face's type byte
(bits 5-6 choose a footstep sound class and bit 7 excludes the face from some
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

### Editing it

**The Collision dock** (`addons/openrac_collision`, an editor plugin every
export enables; [LEVEL_MODDING.md](LEVEL_MODDING.md#collision) is the
modder's guide) edits the collision face by face: Show collision draws it over
the level with a legend of its surfaces; Pick faces selects faces in the 3D
view (click, Shift-click, a box, connected faces of one type byte, the faces
baked inside a selected tie or shrub); the selection is repainted (surface id,
sound class, exclusion bit), deleted, moved by an offset, or faces are added
(a quad, a box, the triangles of a selected mesh, simplified to fit the
cells). Each edit is undoable and saved with the scene. The dock leaves the
tiles alone and keeps its edits as metadata of `Game/Collision`, which
`collision_scene.scene_collision` applies on top of the nodes' triangles:

| Metadata | Ten ints per face |
|---|---|
| `rc1_removed` | the corners x0 y0 z0 x1 y1 z1 x2 y2 z2 in 1/64 game units (the tiles' positions times 64, glTF's winding), then the type byte |
| `rc1_added` | the same, on the game's grid; a repaint or a move is a face removed and added again |
| `rc1_hero_removed` | the hero wall's number (`HeroWall_NNN`), then the corners |

A face is named by its corners and type byte, whichever corner comes first.
A removed face no node holds any more (a tile edited in Blender since) is
reported in `changes.txt`, not guessed at. The dock's logic is in plain
scripts (`level_faces.gd`, `class_collision.gd`), which
[test_collision_dock.gd](test_collision_dock.gd) runs headless on a real level.

The collision also comes as tiles, one per 64 x 64-unit square of the level
(`Game/Collision/Tile_XX_YY`, an instance of `collision/tile_XX_YY.glb`), and
one node per hero wall (`Game/HeroWalls/HeroWall_NNN`, `collision/hero_NNN.glb`).
A tile has a material per type byte named `rc1_surface_XX` (the byte in
hexadecimal: `rc1_surface_1f`, `rc1_surface_0c`). What you can do:

| In Godot or Blender | What the packer does |
|---|---|
| Delete a tile or a hero wall | its triangles leave the level |
| Move, turn or scale a tile | its triangles move |
| Edit a tile's `.glb` in Blender and export it to the same file (keep the materials; give a face another `rc1_surface_XX` material to change its type) | the triangles that differ are taken out and put in |
| Set `rc1_surface` (an integer, the type byte) on a tile | every face of it takes that byte |
| Add a node under `Game/Collision`: a `MeshInstance3D` with a `BoxMesh`, `PlaneMesh` or `QuadMesh`, or an instance of any `.glb`/`.gltf`, with `rc1_surface` metadata (0x1f, 31, without it; a material named `rc1_surface_XX` in a glTF wins) | its triangles are added, in the node's place |
| The same under `Game/HeroWalls` | a new hero wall (255 triangles each; more become several) |

How it is packed ([collision_scene.py](collision_scene.py), [collision.py](collision.py)):

- The packer compares the scene's triangles with the disc's. A scene with
  the collision as the export wrote it (re-saved by Godot or not) packs
  nothing, and read back from the tiles it is exactly the disc's triangles
  (checked on all 19 levels of both discs).
- The game keeps its collision in a grid of 4-unit cells, each with its own
  copy of every face that touches it ([COLLISION.md](../docs/engine/formats/COLLISION.md)).
  A removed triangle leaves every cell that has it (a quad that loses one
  half keeps the other); an added one goes into every cell it touches; every
  other cell keeps its bytes, and the block keeps the disc's layout, so an
  untouched block is written back byte for byte.
- Positions snap to the game's grid: 1/16 unit across, 1/64 up. A triangle
  that becomes a line on it is left out (`changes.txt` counts them). A large
  triangle is halved until each cell it touches can hold its corners (a
  vertex is at most 32 units from its cell's centre).
- Checked before anything is written, or the level's collision is left as
  the disc has it and `changes.txt` says why: at most 255 vertices and 255
  quads in a cell (simplify the mesh there), a cell's size, everything inside
  0-1024 units on each axis (the game answers no query outside), hero walls
  of at most 255 triangles.
- The block goes back into the level's core data, in its own place when it
  fits; a larger one moves to the end of the core data (the core index says
  where), and the level's memory after the core moves up by the block's size.
  **That has not been tried in the running game yet**: keep growth small, and
  tell us how it goes.
- The core data is compressed again (about ten seconds a level) into the room
  it has on the disc, and its sectors go in the packed folder.

What the decoded collision misses: the cells' quads are split into triangles
here, and a face shared by cells appears once, so the tiles show the level's
surfaces, not the cells.

### Object collision (moby classes)

Crates, doors, enemies and the other mobys collide through their **class**:
a class in the level's core can carry a collision blob
([MOBY.md, section 8](../docs/engine/formats/MOBY.md#8-collision-bangles-gif-usage)) of
**primitives** (spheres, spheres on a joint, upright cylinders, capsules
between two joints) and **triangles** with type bytes, which the game's
queries test against each moby of the class where it stands, turned and
scaled with it. A bolt crate, for one, is a sphere and a 12-triangle box.
Each class scene (`mobys/moby_<class>.tscn`) has it as a hidden `Collision`
child of its root, in the class's model units (the node carries the class
scale), coloured like the level's surfaces:

```
Moby_<class>
  Collision             hidden; rc1_collision_format = 1
    Sphere_NN           a SphereMesh: a sphere
    JointSphere_NN      a SphereMesh on joint rc1_joint, drawn in the bind pose
    Cylinder_NN         a CylinderMesh, upright (on a tilted moby the game keeps it upright too)
    Capsule_NN          a CapsuleMesh between joints rc1_joints, drawn in the bind pose
    Mesh_XX             collision/moby_<class>_XX.glb: the triangles of type byte XX
```

Every child has `rc1_collision_kind` (`sphere`, `joint_sphere`, `cylinder`,
`capsule`, `mesh`) and `rc1_surface` (a mesh's type byte; 31 on the
primitives, which have no surface: a hit on one reports none). A primitive
also has `rc1_index`, its place in the blob, and `rc1_queries`: bit 0 for the
queries that pass flag 0x4 (the player's movement among them), bit 1 for the others
(shots, bolts, explosions; the ground probe tests no primitives). Triangles are tested by every
query. Joint primitives follow their joints in the running game; the editor
shows them in the bind pose. Open the class scene and click the `Collision`
eye to see it (the Collision dock shows it on a selected moby, and edits its
parts' sizes, positions, queries and type bytes, or makes it from the class's
model: the same rules as below).

| In Godot or Blender | What the packer does |
|---|---|
| Move or scale a sphere or cylinder (the cylinder's height and radius), or a joint sphere (its offset from the joint) | the primitive moves or resizes |
| Scale a capsule | its radius; it runs between `rc1_joints`, which you change instead of moving it |
| Change `rc1_queries`, `rc1_joint` or `rc1_joints` | the primitive takes them (joints must be the class's) |
| Delete a child | the primitive or those triangles go; with every child gone the class has no collision |
| Add a `MeshInstance3D` with a `SphereMesh` (with `rc1_joint` on a joint), `CylinderMesh` or `CapsuleMesh` (with `rc1_joints`) | a new primitive |
| Add a `BoxMesh`, `PlaneMesh`, `QuadMesh` or a `.glb`/`.gltf` instance, or edit a `Mesh_XX` glTF in Blender (a material `rc1_surface_YY` gives faces that type byte) | the triangles change |

**What a class edit affects.** The collision belongs to the class, so an edit
is **every moby of that class in that level**. Each level has its own core,
so the same class in another level keeps its own collision: edit it there too.
One moby alone cannot get other collision from the level data: nothing in a
moby's record turns it on or off. What works is giving that moby another class
the level has (which changes its model too), or behaviour code
([docs/port/MODS.md](../docs/port/MODS.md)): the game sets a moby's own
`collision` pointer (moby +0x94, copied from the class) to 0 to make it
intangible and sets bits of `collision_disable` (+0x98: bit *i* turns off
primitive *i*, the first 32) on objects being carried, and a mod can do the same.

How it is packed ([moby_collision_scene.py](moby_collision_scene.py),
[moby_collision.py](moby_collision.py)):

- What is as the export wrote it keeps the disc's bytes: an untouched project
  packs nothing (checked on every class of both discs: 2,972 PAL and 2,968
  NTSC-U class scenes read back as the disc's), and an edited class keeps the
  records of the primitives you left alone.
- Checked first, or that class is left as the disc has it and `changes.txt`
  says why: kinds, radii and heights above 0, joints within the class's
  skeleton, vertices within 32 model units of the class's origin (16 bits in
  1/1024), faces reaching only the first 256 vertices (byte indices).
- The blob goes back where the class has it when it fits (its place is padded
  to 16 bytes), else to the end of the core data: the class header's
  collision offset is relative to the class and the loader adds the class's
  address, so it can point anywhere in the core. Like a moved level collision
  block, **that has not been tried in the running game yet**.
- The port reads it as the game does: the core data, served from the packed
  sectors by `--mods`, is in the game's memory, and the collision kernels read
  the class blob there.

**Custom models.** A model you replace with an overrides pack
([MODDING.md](MODDING.md)) keeps the class's collision, so a bigger crate is
still hit as the old one. Make collision for it from its mesh:

```sh
python3 editor/moby_collision.py generate my_crate.glb --scene assets/levels/rac1/pal/level_03/levels/level_03/mobys/moby_500.tscn
python3 editor/moby_collision.py generate my_crate.glb --mode box --surface 0x7f --scene ...
```

`--mode mesh` (the default) takes the triangles themselves, merged until at
most 256 vertices are left; `box` the box around them; `sphere` and
`cylinder` one primitive around them. The model is read as `assets.py`
exports models (glTF's Y up, at the drawn size: the class scale is taken
from the class scene); `--axes game` reads one in the game's axes, `--scale`
gives the size. The class scene's `Collision` children are replaced, and
`extract.py pack` packs it. `--out FILE` writes the blob instead, for tools.
A class the level does not have cannot be added yet, so neither can its
collision.

## Checks

```sh
python3 -m unittest discover -s editor                         # synthetic data only
godot --headless --path assets/godot --import
godot --headless --path assets/godot --script res://rc1/check.gd
godot --headless --editor --quit --path assets/godot           # loads the Collision dock: no errors
OPENRAC_GODOT=godot OPENRAC_DOCK_DISC=baserom/SCES_509.16.iso OPENRAC_DOCK_PROJECT=assets/levels/rac1/pal/level_00 \
  python3 -m unittest discover -s editor -p test_collision_dock.py   # the dock's edits, packed
```

The last runs the dock's logic headless on a copy of an exported, imported
level (picking by ray, selecting, repainting, deleting, moving, adding, a
mesh made into collision, a class's collision made from its model) and checks
the packed block is the disc's with exactly those edits, every other cell byte
for byte.

The Godot check loads every level scene and compares its meshes, triangles,
textures, bounds, sky panorama and moby count with what the extractor wrote.
The collision layer is not counted with the textured meshes; its triangles
are compared with `level.json` on their own, and it must be hidden.
It also loads every moby class scene and compares their models, triangles,
textures, skeletons, bones and animations, and the meshes the placed mobys
show, and each class scene's collision (classes, primitives, triangles; it
must be hidden).

## Coverage

Extracted:

- terrain (tfrags);
- ties and shrubs, with their placements;
- moby placements, and each moby class's high-detail mesh, skeleton and
  animations;
- the sky;
- the collision and the hero walls, as hidden layers, editable;
- the gameplay data: pvars, moby links and groups, volumes, paths, grind
  paths, cameras, sound instances and level settings;
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

Not yet extracted: low-detail moby meshes, audio, video
and each level's code overlay. [ASSETS.md](../games/rac1/docs/ASSETS.md) describes
the formats and the evidence for them.

## The later games (early access)

Going Commando (`SCUS_972.68` v1.01), Up Your Arsenal (`SCUS_973.53`) and
Deadlocked (`SCUS_974.65`), NTSC-U. Their levels are the first game's
format with a few differences, read by the same readers through a table of
what each game changes ([games.py](games.py); the formats, and what was
checked on each disc, are in
[docs/engine/formats/LATER_GAMES.md](../docs/engine/formats/LATER_GAMES.md)).
None of these games is playable in OpenRAC yet: this is for modders to get
ready, looking at the levels and making texture and model packs that will
apply once they are.

```sh
python3 editor/extract.py survey baserom/SCUS_972.68.iso                  # the levels on the disc, as JSON
python3 editor/extract.py godot baserom/SCUS_972.68.iso assets/levels/rac2/ntsc --split
godot --path assets/levels/rac2/ntsc/level_01 -e                         # Oozla
python3 editor/extract.py port baserom/SCUS_973.53.iso assets/port/rac3/ntsc   # for texture and model packs
python3 editor/assets.py export assets/port/rac3/ntsc build/uya-export --level 2 --what textures
python3 editor/assets.py import build/uya-export --levels assets/port/rac3/ntsc --out build/uya-pack
```

The disc is told apart by the executable `SYSTEM.CNF` boots, as for the
first game. A level is numbered as the game numbers it (its level header's
number), so the folders have gaps: Going Commando 0–20, 22–26 and 30 (the
Insomniac Museum); Up Your Arsenal 1–55 but for 15, 25, 37 and 38;
Deadlocked 0–74 with its multiplayer maps and split-screen copies. The
launcher's Mods page lists them by name.

| What | Going Commando | Up Your Arsenal | Deadlocked |
|---|---|---|---|
| Levels | 27 | 51 | 47 |
| Terrain, with the chunks large levels stream | yes | yes | yes |
| Ties and shrubs, with placements | yes | yes | yes |
| Moby class meshes, with placements | yes | yes | yes |
| Moby skeletons | yes | yes | yes |
| Moby animations | yes, but some classes | yes, but some classes | not yet |
| Textures | yes | yes | yes (Deadlocked's swizzled pixels are put back in order) |
| Chrome map for metal parts | not found: grey | not found: grey | not found: grey |
| Sky | yes | yes | yes |
| Collision layer | yes | yes | yes |
| Lighting baked as the game does it | not yet | not yet | not yet |
| Moby class names and update functions | not yet | not yet | not yet |
| Deadlocked's mission instances and classes | | | not yet |
| Packing edits back into the game | not yet | not yet | not yet |
| Playing | not yet | not yet | not yet |

What does not decode yet is left out rather than stopping the level, and
each level says what: `level.json`'s `problems` in a Godot project,
`stats.problems` in the port layout's `manifest.json` (a class without its
animations, a terrain fragment, a texture drawn grey).

In Godot a later game's level opens like the first game's, with the same
nodes and `rc1_*` metadata. Its level node carries `rc1_game` and
`rc1_packable = false`, and its description in the Scene dock says that the
level is for looking and making packs: you can move things, but the packer
reads Ratchet & Clank's levels only and refuses another game's project
(`extract.py pack` stops with a message), so nothing you change in the scene
reaches the game yet. Moby nodes are named `Class<N>_NNNN`: the class names
are the first game's only.

Texture and model packs work as for the first game ([MODDING.md](MODDING.md)):
an export made from a later game's levels says its game, its import makes a
pack for that game (`"game": "rac2"`, `"rac3"` or `"rac4"`, `"version":
"ntsc"`), and `overrides.py` stacks a pack only for levels of its own game.
Such a pack holds textures and models; a level edit in it is ignored and
said so. The launcher's Mods page has all of it: **Read the levels**, then
**Models and textures**, and the level editor; its Pack and Play steps say
they are not there yet for these games.

## Code

The GDScript files follow [`GDSCRIPT_CONVENTIONS.md`](GDSCRIPT_CONVENTIONS.md).

The editor's files carry no licence lines, as the maintainers decided: the editor's MIT [LICENSE](LICENSE) covers every one of them.

| File | Contents |
|---|---|
| `extract.py` | Command line and safe output |
| `disc.py` | ISO files, the two discs' versions (what identifies each from `games/rac1/game.json`), sector table of contents, level headers |
| `wads.py` | Going Commando's, Up Your Arsenal's and Deadlocked's discs (identified from their `game.json`): table of contents, level table, level headers |
| `games.py` | What the four games' level formats share and what each changes, for the shared readers; the versions each game's levels are read from (`game.json`) |
| `level.py` | A level's sections, core blocks and textures (each game's) |
| `formats.py` | `FormatError` and `check`, bounded reads, WAD decompression, code overlays, textures |
| `util.py` | The helpers several modules share, one copy each: SHA-256 of bytes, reading JSON with the caller's own error, 32-bit floats, the cross product, the disc identity in `games/<game>/game.json`. What the repository's [`openrac_common`](../tools/openrac_common/README.md) already does (a file's digest, reading a manifest, the ISO 9660 records) the editor takes from there |
| `terrain.py`, `ties.py`, `shrubs.py`, `sky.py` | Geometry and placements |
| `collision.py` | The collision block: decoded for display, and read, edited and written back byte-exact |
| `mobys.py`, `moby_class.py`, `moby_anim.py` | Moby placements, class meshes, skeletons and animations |
| `mesh.py`, `gltf.py` | The mesh type and the GLB writer |
| `gltf_read.py`, `pngio.py` | Reading glTF and PNG files back (a modder's edits) |
| `assets.py`, `overrides.py` | Export for modding and re-import; overrides packs ([MODDING.md](MODDING.md)) |
| `godot.py` | Project, scenes and level.json |
| `collision_scene.py` | The collision as tiles and hero walls in the scene, read back and packed into the level's core data |
| `moby_collision.py` | Moby class collision: read, checked and written back byte-exact; generated from a mesh (also a command) |
| `moby_collision_scene.py` | A class's collision in its class scene, read back and packed into the level's core data |
| `gamedata.py` | The gameplay data (pvars, links, groups, volumes, paths, cameras, sounds, level settings) in the scene and packed back |
| `pack.py`, `gameplay.py`, `scenery.py`, `wadpack.py` | Packing edited scenes back into the game's level data |
| `addons/openrac_collision/` | The Collision dock (GDScript, editor only): `plugin.gd` the plugin, `dock.gd` and `help.gd` its panel, `level_faces.gd` the level's faces and their edits, `class_collision.gd` a class scene's collision, `surfaces.gd` the type bytes |

The decoders accept only the layouts found on these discs and raise
`FormatError` on anything else. Nothing is skipped silently.

## Credits

- **[Wrench](https://github.com/chaoticgd/wrench)** by chaoticgd and
  contributors (GPL-3.0-or-later). Most of what the extractor knows about
  terrain, ties, shrubs, the sky, textures and placements comes from
  reading its source. [ASSETS.md](../games/rac1/docs/ASSETS.md#sources-and-credits)
  lists which files each format came from. What the later games change
  (their table of contents, level headers and level data, the instance
  records, the tie and moby headers, Deadlocked's swizzle, skeletons and
  sky, the streamed chunks) comes from reading it too, and their level
  names are its `data/underlay/*_levels.asset`
  ([LATER_GAMES.md](../docs/engine/formats/LATER_GAMES.md)).
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
