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
   (Ctrl+S). The **Collision** dock, on the right, edits the collision.
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
| the Collision dock | the level's collision, face by face: select, repaint, delete, move, add, make from a mesh | [Collision](#collision) |
| `Collision/Tile_XX_YY`, `HeroWalls/HeroWall_NNN` | the same collision as tiles: delete, move, edit in Blender, set `rc1_surface` | [Advanced: the tiles in Blender](#advanced-the-tiles-in-blender) |
| a class scene's `Collision` (`mobys/moby_<class>.tscn`) | a moby class's own collision: spheres, cylinders, capsules, triangles (the dock edits them too) | [Object collision](#object-collision-a-crates-shape) |

## Collision

Collision is what Ratchet stands on and bumps into, and what shots and bolts
hit. A level has two kinds: the **level's collision**, one mesh for the whole
level with the terrain, ties and shrubs baked in (`Game/Collision`, and
`Game/HeroWalls`, the walls only the player meets), and each moby class's
**object collision** ([below](#object-collision-a-crates-shape)). Both are
edited in the **Collision dock**, which every exported project has (an editor
plugin, enabled in the project; it is on the right, next to the Inspector).
Its **How to edit collision** button opens the same instructions as this
section.

### See it

Turn on **Show collision**: every face is drawn over the level in its
surface's colour, and every moby shows its class's collision. The legend lists
the surfaces the level has and what is known of each (the
[README's table](README.md#collision)). A face's **type byte** has three
parts: the surface id (bits 0-4, the colour: 31 is nothing special, 0 water),
the footstep sound class (bits 5-6, 0 to 3) and a bit that keeps the face out
of some of the game's queries (bit 7). Hero walls are drawn yellow; they have
no type byte.

### Select

Turn on **Pick faces in the 3D view**, then click a face. Shift-click adds a
face or takes it out; drag a box to select every face whose middle is inside
it; Alt-click takes the faces that touch it with the same type byte
(Shift-Alt-click adds them, **Connected, same surface** adds those of the
whole selection). The dock says what is selected: how many faces, their
surface ids, sound classes and how many are excluded. With Pick faces off, the
editor selects nodes as usual: selecting a tie or a shrub selects the
collision baked inside its bounds, and selecting a moby shows its class's
collision.

### Edit

| Button | What it does |
|---|---|
| **Repaint the selected faces** | gives them the surface id, sound class and exclusion chosen above it |
| **Delete the selected faces** | the game no longer meets them (hero wall faces: delete only) |
| **Move the selected faces by this** | moves them by the X, Y, Z offset in game units (Z up) |
| **Add a floor quad / a box here** | adds one where you last clicked, of the size given, with the chosen type byte |
| **Make collision from the selected mesh** | adds the triangles of the selected `MeshInstance3D` (or the meshes under a node): collision for your own models, simplified to fit the game's cells |
| **Check the limits** | estimates the cells over the game's limit before you pack |

Every edit is one step of the scene's undo history (Ctrl+Z) and is saved with
the scene (Ctrl+S). Faces are solid from the side their corners turn
counter-clockwise from; an added box faces out.

- **Moving scenery.** The collision of ties and shrubs is baked into the
  level's: a tie you move is walked on where it was. Select the tie (its baked
  faces get selected), turn Pick faces on, and move the faces by the offset
  you moved the tie; a new tie needs collision added (Make collision from the
  selected mesh).
- **What is kept.** The dock does not change the tiles: its edits are
  metadata of `Game/Collision` (`rc1_removed`, `rc1_added`,
  `rc1_hero_removed`), which the packer applies on top of them. The faces you
  did not touch keep the disc's bytes, and a level nobody edited packs to
  nothing.

### Limits

The game keeps the collision in a grid of 4-unit cells. The packer refuses,
for that level's collision only, what the game cannot hold, and says why in
`changes.txt`: more than 255 corners in one cell (simplify the mesh there;
**Check the limits** estimates it), anything outside 0 to 1024 units on an
axis. Positions snap to the game's grid, 1/16 unit across and 1/64 up; a
sliver that becomes a line there is left out, and a large face is cut in
pieces small enough for the cells.

A level whose collision grew past its place on the disc gets it moved to the
end of the level's core data, which takes more of the level's memory. That
has not been tried in the running game yet: keep additions modest, and report
how it goes.

### Advanced: the tiles in Blender

The collision is also on disk, as tiles: `Game/Collision/Tile_XX_YY` is an
instance of `collision/tile_XX_YY.glb`, the triangles of a 64-unit square,
with one material per type byte named `rc1_surface_XX` (hexadecimal). You can
delete or move a tile node, set an integer `rc1_surface` metadata entry on it
(every face of it takes that byte), add a `MeshInstance3D` with a `BoxMesh`,
`PlaneMesh` or `QuadMesh` (31 without `rc1_surface`), or drop in any
`.glb`/`.gltf`. To edit a tile's triangles in Blender: File > Import > glTF,
edit (give faces another `rc1_surface_XX` material to change their type), then
File > Export > glTF Binary to the same file, with **Materials: Export** on.
Keep the file in the level's `collision/` folder under its own name; Godot
reimports it when the editor gets focus. The dock's edits apply on top of
what the tiles hold, so edit a tile in Blender before you edit its faces in
the dock (a face the dock removed that the tile no longer has is reported, not
guessed at). Hero walls are the same under `Game/HeroWalls`, at most 255
triangles per wall (more become several walls).

## Object collision: a crate's shape

A crate, a door or an enemy collides with its class's own collision, not the
level's: spheres, upright cylinders, capsules between two joints, and
triangles. Select a moby: the dock shows its class's collision in the view and
lists its parts. Press **Open its class scene to edit** (a bolt crate is
class 500, `mobys/moby_500.tscn`). In the class scene:

- select a part under `Collision` (Scene dock) to change its radius, height,
  position, queries (1: the player's movement and the other flag-0x4 queries,
  2: shots, bolts and the rest) or type byte in the dock, then **Apply to the
  selected part**; or move it with the gizmo, or delete it;
- **Make class collision from its model** replaces the class's collision with
  one made from the class's model: `mesh` its triangles, merged until at most
  256 corners are left (written as a glTF file in `mobys/collision/`), `box`
  the box around it, `sphere` or `cylinder` one shape around it.

Save the class scene and pack. Undo works for each step. You can also add a
`MeshInstance3D` with a `SphereMesh`, `CylinderMesh` or `CapsuleMesh` (a
capsule names its two joints in `rc1_joints`) by hand, or edit the `Mesh_XX`
triangles in Blender as you would a tile. The details are in the
[README](README.md#object-collision-moby-classes).

- **One class, every moby of it, this level only.** The edit applies to all
  the level's mobys of that class; the same class in another level is a copy
  of its own (each level has its own core data), so edit that one too. One
  moby cannot get collision of its own from the level: give it another class
  the level has, or turn its collision off in behaviour code (a mod made with
  the SDK).
- **Custom models.** A model you replaced keeps the old collision. Make some
  from your model: in its class scene, Make class collision from its model; or
  from a shell, `python3 editor/moby_collision.py generate my_model.glb --scene <the class scene>`
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

A project made before the Collision dock gets it the next time you extract the
levels into the same folder (the launcher's **Extract the levels**): the dock is
added and enabled, and the scenes, with your edits, are left as they are. In a
project without the editable collision the dock says so and offers only the
object collision.

## Limits and what is next

- A project from before the class collision has no `Collision` node in its
  class scenes: extract the level again to edit a class's collision.
- Terrain meshes and classes the level does not load are not packed.
- Cameras and sound emitters cannot be added or deleted yet; volumes and
  paths can be added but not deleted (other data refers to them by number).
- The later games (Going Commando, Up Your Arsenal, Deadlocked) open for
  looking and texture and model packs only.
