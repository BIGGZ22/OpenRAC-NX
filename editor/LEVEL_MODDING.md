# Modding levels: placements, gameplay data and collision

Ratchet & Clank's levels open in Godot 4 (tested with 4.7.2), one project per
level. You move, add and delete objects, change their values, reshape volumes,
paths and cameras, and edit the collision, then pack the level back and play it
in OpenRAC's native port. Your disc image is never changed, and nothing you
extract may be shared: it is your disc's data. Behaviour (what an enemy or a
crate does) is not edited here: that is a mod made with the mod SDK
([docs/port/MODS.md](../docs/port/MODS.md)), whose behaviour catalogue
(`sdk/catalogue/rac1/`) names each class's update function and pvar fields.
Models and textures have a guide of their own ([MODDING.md](MODDING.md)).

## The steps

On the launcher's **Mods** page, for the disc you set up:

1. **Extract the levels**: the 19 levels as Godot projects (about 20 seconds).
   It never overwrites a project; to get a newer extractor's scenes, see
   [Projects from an older extractor](#projects-from-an-older-extractor).
2. **Open a level in Godot**: choose the level. Edit (below) and save the scene
   (Ctrl+S).
3. **Pack your edited levels**: what differs from the disc becomes the
   game's level data, in the packed folder. `changes.txt` there says, level by
   level, what was packed and what could not be, and why.
4. **Play with your edits**: the game reads your packed levels in place of the
   disc's.

From a shell, the same steps:

```sh
python3 editor/extract.py godot baserom/SCES_509.16.iso assets/levels/rac1/pal --split
godot --path assets/levels/rac1/pal/level_03 -e
python3 editor/extract.py pack baserom/SCES_509.16.iso assets/levels/rac1/pal assets/packed/rac1/pal
# then the port with --mods assets/packed/rac1/pal (the launcher's Play with your edits)
```

The NTSC-U disc works the same way (`SCUS_971.99.iso`, `rac1/ntsc`). A packed
folder serves the disc it was packed from.

## What you can edit

Everything is under the scene's `Game` node, in the game's units and axes (Z
up). Values the game stores are the Inspector's **Metadata** (`rc1_*`).

| Node | Edit | Details |
|---|---|---|
| `Mobys/<Class>_NNNN` | move, turn, scale; copy (Ctrl+D) or drop a class scene to add; delete; `rc1_*` values | [README: Pack your edits](README.md#pack-your-edits) |
| a moby's `rc1_pv_<field>`, `rc1_pvar` | its pvars: named fields where the decompilation knows the class's struct, else 32-bit words | [README: Gameplay data](README.md#gameplay-data) |
| a moby's `rc1_link_<offset>` | another moby it refers to, by node name | the same |
| `MobyGroups` | which mobys form each group | the same |
| `Volumes/...` | cuboids, spheres, cylinders, pills: move, turn, scale, copy | the same |
| `Paths/...`, `GrindPaths/...` | Godot paths: move, add or remove points | the same |
| `Cameras/...`, `Sounds/...`, `LevelSettings` | cameras, sound emitters, fog, death height, the ship | the same |
| `Ties/...`, `Shrubs/...` | scenery: move, turn, scale, add, delete | drawn by the port; their collision is the level's (below) |
| `Collision/Tile_XX_YY` | the collision, by 64-unit square: delete, move, edit in Blender, set `rc1_surface` | [Collision](#collision) |
| `HeroWalls/HeroWall_NNN` | walls only the player meets | the same |
| a class scene's `Collision` (`mobys/moby_<class>.tscn`) | a moby class's own collision: spheres, cylinders, capsules, triangles | [Object collision](#object-collision-a-crates-shape) |

## Collision

Show `Game/Collision` (its eye icon in the Scene dock) to see it over the
terrain, coloured by surface. Each tile is a `.glb` file in the level's
`collision/` folder with one material per **type byte**, named
`rc1_surface_XX` (hexadecimal): bits 0-4 are the surface (0x1f: nothing
special; 0x00: water; the README's table has what is known of the others),
bits 5-6 the footstep sound class, bit 7 keeps the face out of some of the
game's tests.

- **Remove collision**: delete a tile, or delete faces from its `.glb` in
  Blender.
- **Change a surface**: in Blender, give faces another `rc1_surface_XX`
  material (add one with that name if the tile has none); or set the tile
  node's `rc1_surface` metadata (an integer) to give every face of it one byte.
- **Add collision**: add a `MeshInstance3D` under `Game/Collision` with a
  `BoxMesh`, `PlaneMesh` or `QuadMesh` (set a `PlaneMesh`'s Orientation to
  Face Z for a floor), or drop in any `.glb`/`.gltf`; place it, and give it an
  integer `rc1_surface` metadata entry (31 without one). A glTF's own
  `rc1_surface_XX` materials win over it.
- **Move collision with scenery**: the collision of ties and shrubs is baked
  into the tiles. A tie you move is walked on where it was until you move
  that part of the collision too (edit the tile in Blender), and a new tie is
  walked through until you add collision for it.
- **Hero walls**: the same, under `Game/HeroWalls`; at most 255 triangles per
  wall (more become several walls).

In Blender: File > Import > glTF, edit, then File > Export > glTF Binary to the
same file, with **Materials: Export** on and **Apply Modifiers** as you like.
Keep the file in the level's `collision/` folder under its own name; Godot
reimports it when the editor gets focus.

Faces are one-sided in the game: a face is solid from the side its corners
turn counter-clockwise from (Blender's normals). Positions snap to the game's
grid, 1/16 unit across and 1/64 up; a sliver that becomes a line there is
left out, and a large face is cut in pieces small enough for the game's
4-unit cells. The packer refuses, for that level's collision only, what the
game cannot hold: more than 255 vertices in one 4-unit cell (simplify the
mesh there), anything outside 0 to 1024 units on an axis.

A level whose collision grew past its place on the disc gets it moved to the
end of the level's core data, which takes more of the level's memory. That
has not been tried in the running game yet: keep additions modest, and report
how it goes.

## Object collision: a crate's shape

A crate, a door or an enemy collides with its class's own collision, not the
level's. Open the class scene (`mobys/moby_<class>.tscn`, the scene each
moby of that class is an instance of; a bolt crate is class 500) and show
its hidden `Collision` node: spheres, upright cylinders, capsules between two
joints, and `Mesh_XX` triangles (the glTF files in `mobys/collision/`, a
material per type byte as the tiles have). Move or scale a sphere, set a
cylinder's height and radius, delete a child, or add a `MeshInstance3D` with a
`SphereMesh`, `CylinderMesh` or `CapsuleMesh` (a capsule names its two joints
in `rc1_joints`); edit the triangles in Blender as you would a tile. Save the
class scene and pack. The details are in the
[README](README.md#object-collision-moby-classes).

- **One class, every moby of it, this level only.** The edit applies to all
  the level's mobys of that class; the same class in another level is a copy
  of its own (each level has its own core data), so edit that one too. One
  moby cannot get collision of its own from the level: give it another class
  the level has, or turn its collision off in behaviour code (a mod made with
  the SDK).
- **Custom models.** A model you replaced keeps the old collision. Make some
  from your model and put it in the class scene:
  `python3 editor/moby_collision.py generate my_model.glb --scene <the class scene>`
  (`--mode box`, `sphere` or `cylinder` for a simpler shape). Then pack.
- **Limits.** The class's vertices stay within 32 model units of its origin
  and its triangles use at most 256 of them; the packer refuses the class
  otherwise and says why in `changes.txt`. A class's collision that grew past
  its place moves to the end of the core data, which, as for the level's
  collision, has not been tried in the running game yet.

## Projects from an older extractor

The extractor never overwrites a project, so a project made before the
editable collision (no `collision/` folder in the level) keeps the old
display-only `Collision` node: its other edits still pack, its collision is
left as the disc has it. To get the new scene and keep your edits: move the
project aside, extract the level again, and redo the edits in the new scene
(Godot saves scenes as text, so a diff of your old scene against a fresh
export of the same extractor shows each edit).

## Limits and what is next

- A project from before the class collision has no `Collision` node in its
  class scenes: extract the level again to edit a class's collision.
- Terrain meshes and classes the level does not load are not packed.
- Cameras and sound emitters cannot be added or deleted yet; volumes and
  paths can be added but not deleted (other data refers to them by number).
- The later games (Going Commando, Up Your Arsenal, Deadlocked) open for
  looking and texture and model packs only.
