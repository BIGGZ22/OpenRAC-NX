# Modding models and textures

Ratchet & Clank in OpenRAC's native port can draw your own textures and
models in place of the game's: upscaled textures, a remodelled Ratchet, a
repainted vendor, denser or reshaped terrain. You export what you want to change from your own disc's
levels, edit it in Blender or any image editor, import it back, and the
result is an **overrides pack** the game loads on top of its levels. Packs
from other people go in the same way, as a .zip or a folder.

It works the same with any disc the game can be set up from (today the
European and the North American one): the launcher uses the levels of the
disc you set up, and a pack made from either disc's levels works with the
other's too (below, [Packs and discs](#packs-and-discs)).

Going Commando, Up Your Arsenal and Deadlocked work the same way in early
access: export, edit and import their textures and models into packs of
their own (`"game": "rac2"`, `"rac3"`, `"rac4"`), which wait in Your mods
until those games are playable. A pack is used only with its own game's
levels ([README.md](README.md#the-later-games-early-access)).

Moving objects, editing their values, volumes, paths, cameras and the
collision is level editing, in Godot: [LEVEL_MODDING.md](LEVEL_MODDING.md).

Nothing here changes your disc image or the extracted levels, and nothing
you export may be shared: it is your disc's data. A pack holds only what you
changed; share it only with people who own the game.

## In the launcher

Everything below is on the **Mods** page, under **Models and textures**:

1. **Export**: choose a level and what to export (characters and objects,
   textures, scenery, terrain, or everything), then Export. The folder opens when it
   is ready: `exports/level_NN-<what>` in the game's set-up folder. Exporting
   again writes a new folder (`-2`, `-3`...), never over your edits.
2. Edit the files (below).
3. **Import your edits**: choose the export folder. What changed becomes a
   pack named after the folder, switched on. Importing the same folder
   again updates that pack. Anything that cannot be used is listed with
   what to do; the rest is still imported.
4. **Packs**: switch packs on and off, change their order (a pack lower in
   the list wins where two replace the same file), remove one, or add one
   with **Add a .zip** or **Add a folder**, or by dropping it on the page.
5. **Play with your mods** (the first panel's Play) puts your packed levels
   and the packs that are on together and starts the game with them. The
   game page's Play plays the levels as they are on the disc. If a pack has
   anything that does not fit your disc's levels, the activity log lists
   what was left out.

## Editing textures

Textures are `level_NN/textures/*.png`. Edit or upscale them and save them
as PNG under the same name, in the same place.

- **Any size, the same shape.** A 128 x 64 texture may become 512 x 256 or
  1024 x 512, not 512 x 512: the game's UVs stretch a texture over its whole
  area. A texture of another shape is refused, with sizes that fit. At most
  8192 on a side.
- **Alpha** is kept as you paint it. The game's moby textures that are
  partly transparent are drawn cut out; an upscaler that blurs a fully
  opaque texture's edges into transparency makes it cut out too.
- **One picture, every place.** Many textures appear in several levels
  under other names (Ratchet's are in all of them). An edited texture
  replaces every texture with the same original picture, in every level.
- Not saved interlaced (most editors never do).

## Editing models in Blender

Blender 4 or 5 (tested with 5.2). Each model is a folder,
`level_NN/mobys/moby_<class>_<name>/` (ties and shrubs, the scenery, under
`ties/` and `shrubs/`), with a `.gltf`, its `.bin` and `asset.json`.

1. **File > Import > glTF 2.0**, and pick the `.gltf`. A moby comes in at
   the size the game draws it, standing up, with its armature (bones
   `joint_000`, `joint_001`...) and its animations as actions, for
   reference.
2. Edit the mesh, the UVs, the weights and the materials' pictures. Keep the
   armature: its bones are the game's, and they keep moving the way the game
   moves them.
3. **File > Export > glTF 2.0** into the same folder: "glTF Binary (.glb)"
   or "glTF Separate", any name (the newest file in the folder is read). The
   defaults are fine; animations need not be exported.

What the import checks and does:

- **Bones by name.** Every vertex must be weighted to the game's bones
  (`joint_NNN`). A weight on a bone the game does not have is refused, and so
  is a vertex without any weight: weight-paint it, or parent the mesh with
  Ctrl+P > With Automatic Weights. A mesh parented to a single bone follows
  that bone.
- **Four bones per vertex** at most: the four strongest are kept and their
  weights normalised (a note says how many vertices that touched).
- **UVs** are required (a mesh with no UV map is refused).
- **Pictures.** A material whose picture is one of the level's textures
  (same file name) uses that texture; if you painted it, it counts as an
  edit of that texture (above). A new picture travels with the model.
- **Vertex colours and light slots** (the game's lighting multipliers, and
  the glowing parts) are taken from the original's nearest vertex when the
  exported file has none: Blender drops them unless asked.
- **Where it goes.** A moby model replaces its class in every level where
  the class has the same skeleton (Ratchet in all 19). To keep it to some
  levels, set `"levels": [0, 3]` in its `asset.json`.
- Animations are not imported: the port animates the game's skeleton from
  the game's own data. The game's sheared joints (two in Ratchet's hands)
  cannot be held by Blender's bones; the import undoes what Blender's
  importer did to the vertices they move.

## Editing the terrain

The terrain (the ground, the walls, the cliffs: the level's static
geometry that is neither a tie nor a shrub) is `level_NN/terrain/`, one
`terrain.gltf` with its `.bin` and `asset.json`. Open it in Blender the same
way (File > Import > glTF 2.0); each of the game's terrain fragments is an
object, `Terrain_000`, `Terrain_001`... Subdivide it, sculpt it, reshape it,
add detail, and export it into the same folder as for a model.

- **Any number of vertices and triangles.** The port draws the terrain as
  one mesh, whatever its size.
- **UVs and pictures** as for models: UVs are required, a material's picture
  that is one of the level's textures (same file name) uses that texture, a
  new picture travels with the terrain, and a material without a picture is
  refused (Blender gives new faces one unless they take their neighbours').
- **Lighting is the vertex colours.** The game lights its terrain once, at
  level load, into a colour per vertex, and the port draws the terrain with
  those colours alone. The export holds them at half strength (0.5 is the
  game's normal brightness, 1.0 twice it; glTF's colours stop at 1.0), and
  the import doubles them again. A vertex without a colour (Blender drops
  them unless the export's Vertex Color option keeps them, and some edits
  make vertices without any) takes the colour of the original terrain at
  the nearest point, blended across the face it lies on, from the fragment
  of the same name where the object keeps it: a subdivided face keeps its
  shading, and a raised hill takes the colour of the ground below it.
  Paint the colours to light it otherwise.
- **One level.** The terrain replaces its own level's only.

The game's collision is not the terrain: Ratchet still walks on the
original's ground, so keep the surfaces he stands on where they were
(editing the collision is level editing: [LEVEL_MODDING.md](LEVEL_MODDING.md)).

The levels must be the ones the export was made from: after "Get the levels
ready" runs again with a newer OpenRAC, or after setting the game up from
another disc, export again and redo the edit. The pack you made keeps
working either way.

## Seeing your textures in the level editor

The level editor's Godot projects show the textures the extractor wrote. To
edit a level while seeing it as your packs make it look, put the mods
together (**Put the mods together**), then **Show your textures in the editor**
(the Mods page), or from a shell:

```sh
python3 editor/overrides.py preview <data>/mods/pal --projects assets/levels/rac1/pal
python3 editor/overrides.py preview --projects assets/levels/rac1/pal --undo   # the editor's own again
```

Each level texture a pack replaces goes into the projects at the pack's size
(Godot imports it the next time its window has focus; the texture
coordinates are the same, so a 4x picture covers the same faces at four times
the detail). The extractor's own picture waits in `textures/.originals/`,
which Godot does not import, until **Hide your textures in the editor** puts it
back. A pack's pictures that replace no level texture (a new model's own)
have no place in the projects and are left out. The levels open lit as the
game lights them at level load, so what you see is close to the game.

## Packs and discs

The two discs hold the same levels, built a little differently: many
textures have other numbers on one than on the other, a few pictures differ
slightly (Ratchet's own skin among them), and a few classes are missing from
one level or another. A pack records what each of its textures and models
replaces (the original picture's hash, and which models use it; a model's
class, its skeleton and the pictures it draws with), and when the game is
played from the other disc's levels, the launcher puts each one where it
belongs there:

- a **texture** wherever those levels have the same original picture, in
  every level; where that disc has its own version of the picture, where
  the same models use it, if it is the same size;
- a **model** in each level whose class has the same skeleton (a model
  without one: the same vertices), drawing with that disc's own textures;
- the **terrain** in its level where that disc's terrain has the same
  vertices (both discs' levels have the same terrain), drawing with that
  disc's textures; its lighting stays the one it was made with;
- **level edits** (a packed level's gameplay file, ties and shrubs placed
  again) belong to the disc they were packed from and are left out on the
  other.

Whatever has no match is left out and listed (`stack.json` in the stacked
folder, and the activity log). Packs made before packs recorded this keep
working on the disc they were made from; on the other, a texture goes in
where its name is the same picture there, and a model where its skeleton
fits.

## From a shell

```sh
python3 editor/assets.py export assets/port/rac1/pal build/my-export --level 0 --what mobys [--class 0]
python3 editor/assets.py import build/my-export --levels assets/port/rac1/pal --out build/my-pack --name "My pack" --author me
python3 editor/overrides.py check build/my-pack          # what a pack holds, and what is wrong with it
python3 editor/overrides.py add build/my-pack --into build/overrides
python3 editor/overrides.py list --into build/overrides
python3 editor/overrides.py stack --into build/overrides --out build/mods/pal --levels assets/port/rac1/pal [--packed assets/packed/rac1/pal]
port/build/release/openrac-rac1-pal --data <data> --window --levels assets/port/rac1/pal --mods build/mods/pal
python3 editor/overrides.py zip my-pack my-pack.zip --into build/overrides
```

Each disc's levels are `assets/port/rac1/<version>` (`pal`, `ntsc`) in a
checkout and `<data>/port/<version>` in an installed launcher; the commands
are the same for either. `stack --levels` names the levels the game will
play, and its `--out` is that disc's folder: `stack --into build/overrides
--out build/mods/ntsc --levels assets/port/rac1/ntsc` puts the same packs
together for the other disc. `--what` is `mobys`, `textures`, `scenery`
(ties and shrubs), `terrain` or `all`. Inside this repository, output goes under
`assets/` or `build/` only.

## The pack format

A pack is a folder, or a .zip of one (the manifest at the zip's root or in
its one top folder):

```
openrac-mod.json                  {"format": 1, "game": "rac1", "version": "pal",
                                   "name": "...", "author": "...", "description": "...",
                                   "levels_format": 3, "replaces": {"level_00": {"textures": [...], "models": [...]}},
                                   "originals": {"level_00/textures/moby_0000.png": {...}, ...}}
level_NN/textures/<name>.png      a level texture, by its name in that level
level_NN/mobys/moby_<class>.glb   a class's model, as the port reads it (game axes, model units)
level_NN/ties/tie_<class>.glb, level_NN/shrubs/shrub_<class>.glb
level_NN/terrain.glb              the level's terrain, as the port reads it
level_NN/placements.json          ties and shrubs placed again (the level packer's)
level_NN/gameplay_<version>.wad, patches.tsv   a packed level (the level packer's)
README*, LICENSE*, *.txt, *.md    for people; other files are ignored
```

`"version"` is the disc whose levels the files are named after (`pal` or
`ntsc`); a pack is for both. `"originals"` says, for each texture, the
original picture's hash, its size and the models that use it
(`{"sha256", "size", "used_by": [[kind, class, place in the model's
textures, how many]]}`, or `{"new": true}` for a model's own picture), and
for each model its class, a hash of the original's vertex positions and the
pictures it and its class draw with (`{"kind", "class", "vertices",
"textures": {name: hash}, "class_textures": [...]}`; the terrain's the same,
with `"kind": "terrain"` and `"class": null`, and `used_by` names it as
`["terrain", 0, ...]`). The hashes and sizes are all a pack holds of the
originals.

The files mirror the extracted levels (`editor/extract.py port`): the port
reads each level's files from the stacked mods folder (`--mods`,
`<mods>/level_NN/`) where they are there and from the levels otherwise
([port/viewer/level.cpp](../port/viewer/level.cpp)), and the disc sectors
`patches.tsv` lists from the files beside it. A model GLB names textures
as `../textures/<name>.png` (the terrain as `textures/<name>.png`): the
pack's own where it carries them, the level's otherwise. Joints are
`joint_NNN` nodes, numbered as the game's. A terrain GLB has one node per
mesh, no skin, and `COLOR_0` in the game's scale (0x80 = 1.0, up to 2.0).

The game's set-up folder holds the packs that were added
(`overrides/packs/<id>/`), their order and state (`overrides/overrides.json`,
`{"format": 1, "packs": [{"id": ..., "enabled": true}]}`) and exports
(`exports/`), shared by the discs, and the stacked folder the game reads,
one per disc (`mods/<version>/`, made again before each Play with your
mods, with `stack.json` saying what it holds and what was left out).

## Limits

- Sky textures the sky's sprites use and the game's 2D pictures (the HUD,
  menus, fonts) cannot be replaced yet; terrain, tie and shrub textures
  can, and the terrain's geometry.
- A replacement model or terrain draws in the port only; the game's
  collision, bone points (where Ratchet holds things) and bounds stay the
  original's. The game's own terrain data (what it culls and lights at
  level load) is not changed either: the port draws the replacement whole.
- A replacement terrain keeps the lighting it was made with: when the
  game's lights change (a level edit), export it again or paint its
  colours.
- Low-detail models are not exported; the port draws the high-detail one.
