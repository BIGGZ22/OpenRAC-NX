# Modding models and textures

Ratchet & Clank (PAL) in OpenRAC's native port can draw your own textures
and models in place of the game's: upscaled textures, a remodelled Ratchet,
a repainted vendor. You export what you want to change from your own disc's
levels, edit it in Blender or any image editor, import it back, and the
result is an **overrides pack** the game loads on top of its levels. Packs
from other people go in the same way, as a .zip or a folder.

Nothing here changes your disc image or the extracted levels, and nothing
you export may be shared: it is your disc's data. A pack holds only what you
changed; share it only with people who own the game.

## In the launcher

Everything below is on the **Mods** page, under **Models and textures**:

1. **Export**: choose a level and what to export (characters and objects,
   textures, scenery, or everything), then Export. The folder opens when it
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
   game page's Play plays the levels as they are on the disc.

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

The levels must be the ones the export was made from: after "Get the levels
ready" runs again with a newer OpenRAC, export again and redo the edit.

## From a shell

```sh
python3 editor/assets.py export assets/port/rac1/pal build/my-export --level 0 --what mobys [--class 0]
python3 editor/assets.py import build/my-export --levels assets/port/rac1/pal --out build/my-pack --name "My pack" --author me
python3 editor/overrides.py check build/my-pack          # what a pack holds, and what is wrong with it
python3 editor/overrides.py add build/my-pack --into build/overrides
python3 editor/overrides.py list --into build/overrides
python3 editor/overrides.py stack --into build/overrides --out build/mods [--packed assets/packed/rac1]
port/build/release/openrac-rac1-pal --data <data> --window --levels assets/port/rac1/pal --mods build/mods
python3 editor/overrides.py zip my-pack my-pack.zip --into build/overrides
```

`--what` is `mobys`, `textures`, `scenery` (ties and shrubs) or `all`. Inside
this repository, output goes under `assets/` or `build/` only.

## The pack format

A pack is a folder, or a .zip of one (the manifest at the zip's root or in
its one top folder):

```
openrac-mod.json                  {"format": 1, "game": "rac1", "version": "pal",
                                   "name": "...", "author": "...", "description": "...",
                                   "levels_format": 2, "replaces": {"level_00": {"textures": [...], "models": [...]}}}
level_NN/textures/<name>.png      a level texture, by its name in that level
level_NN/mobys/moby_<class>.glb   a class's model, as the port reads it (game axes, model units)
level_NN/ties/tie_<class>.glb, level_NN/shrubs/shrub_<class>.glb
level_NN/placements.json          ties and shrubs placed again (the level packer's)
level_NN/gameplay_pal.wad, patches.tsv   a packed level (the level packer's)
README*, LICENSE*, *.txt, *.md    for people; other files are ignored
```

The files mirror the extracted levels (`editor/extract.py port`): the port
reads each level's files from the stacked mods folder (`--mods`,
`<mods>/level_NN/`) where they are there and from the levels otherwise
([port/viewer/level.cpp](../port/viewer/level.cpp)), and the disc sectors
`patches.tsv` lists from the files beside it. A model GLB names textures
as `../textures/<name>.png`: the pack's own where it carries them, the
level's otherwise. Joints are `joint_NNN` nodes, numbered as the game's.

The game's set-up folder holds the packs that were added
(`overrides/packs/<id>/`), their order and state (`overrides/overrides.json`,
`{"format": 1, "packs": [{"id": ..., "enabled": true}]}`), exports
(`exports/`) and the stacked folder the game reads (`mods/`, made again
before each Play with your mods).

## Limits

- Sky textures the sky's sprites use, the game's 2D pictures (the HUD,
  menus, fonts) and the terrain's geometry cannot be replaced yet;
  terrain, tie and shrub textures can.
- A replacement model draws in the port only; the game's collision, bone
  points (where Ratchet holds things) and bounds stay the original's.
- Low-detail models are not exported; the port draws the high-detail one.
