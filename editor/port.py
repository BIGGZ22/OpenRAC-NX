"""Write extracted levels for the native port's level viewer (port/viewer).

Each level goes to OUT/level_NN/, with the meshes and textures the Godot
export writes (godot.py), in the same files, and two JSON files the viewer
reads in place of the Godot scene:

    manifest.json      format 1: the level, its files, its classes (mesh,
                       moby class scale and name), bounds and counts
    placements.json    ties, shrubs and mobys: class, 4x4 matrix
                       (column-major, game units and axes, Z up) and the
                       fields the game stores per instance
    terrain.glb        one node per terrain fragment
    collision.glb      the collision mesh, coloured by surface type
    ties/tie_<class>.glb, shrubs/shrub_<class>.glb
    mobys/moby_<class>.glb   mesh, skeleton (bind pose) and animations
    sky.glb            the sky's shells, one node each in drawing order,
                       with vertex colours (alpha 0x80 = 1.0, as 0..1)
    sky.png            the sky as a panorama, as for Godot
    textures/*.png     shared by every mesh
    probe-bake.json    matching authored lighting profile, when one exists

A moby placement's matrix is the instance's own (position, rotation, scale);
the class scale in the manifest multiplies in when the class mesh is drawn,
as the game does. Nothing here is ever committed (docs/policy/SOURCING.md).
"""

import json
import math
import struct
from pathlib import Path

from enhanced.bake_inputs import write_bake_inputs
from enhanced.profiles import write_bake_profile
from formats import png, unpack
import games
from gltf import Gltf
from godot import PANORAMA, STORED_W, LevelWriter, chunk_terrain, problem_summary
from level import Level
from lighting import light_bank, light_shrub_instance, light_tie_instance
from mesh import Mesh
from moby_class import MobyClass, moby_classes
from mobys import moby_class_names, moby_instances
from shrubs import shrub_class_normals, shrub_classes, shrub_instances
from sky import Sky, panorama, sky
from terrain import terrain
from occlusion import export_pvs
from ties import tie_classes, tie_instances, tie_slot_normals

# The export's format: the port reads only its own (viewer/level.h kManifestFormat), and its level
# check names the format in the stamp the launcher waits for, so levels exported before a change
# are read from the disc again. 2: the mobys' metal packets and the chrome and glass maps. 3: the
# gadget classes whose bind pose collapses a joint (the Blaster, the Pyrocitor).
FORMAT = 3


def sky_glb(data: Sky, texture_uri) -> bytes:
    """The shells as a GLB: one node per shell, in drawing order; one
    primitive per texture with POSITION, TEXCOORD_0 and COLOR_0 (RGBA 0..1).
    Textured faces get a blended material with the texture at
    texture_uri(index); untextured faces a blended material without one,
    so the vertex colour alone shows."""
    gltf = Gltf()
    untextured = None
    for shell in data.shells:
        primitives = []
        for key, faces in shell.faces.items():
            corners = [v for face in faces for v in face]
            if not corners:
                continue
            if key is None:
                if untextured is None:
                    gltf.doc["materials"].append({"name": "sky_untextured", "doubleSided": True, "alphaMode": "BLEND",
                                                  "pbrMetallicRoughness": {"metallicFactor": 0}})
                    untextured = len(gltf.doc["materials"]) - 1
                material = untextured
            else:
                material = gltf.material(f"sky_{key[1]:04}", texture_uri(key[1]))
                gltf.doc["materials"][material]["alphaMode"] = "BLEND"
            attributes = {"POSITION": gltf.floats([shell.positions[v] for v in corners], bounds=True),
                          "TEXCOORD_0": gltf.floats([shell.uvs[v] for v in corners]),
                          "COLOR_0": gltf.floats([shell.colours[v] for v in corners])}
            primitives.append({"attributes": attributes, "material": material, "mode": 4})
        if primitives:
            gltf.doc["meshes"].append({"name": shell.name, "primitives": primitives})
            gltf.node(shell.name, len(gltf.doc["meshes"]) - 1)
    return gltf.glb()


def finite(fields: dict) -> dict:
    """A moby's fields as JSON can hold them: a float that is not finite (Going Commando stores
    infinity for some rooted distances) becomes the string "inf", "-inf" or "nan"."""
    return {k: (str(v) if isinstance(v, float) and not math.isfinite(v) else v) for k, v in fields.items()}


def placement(p: dict) -> dict:
    """A tie or shrub placement: class and matrix first, then the stored
    fields; matrix_w only where the game stores something other than 0.01."""
    out = {"index": p["index"], "class": p["class_id"], "matrix": p["matrix"]}
    out.update({k: v for k, v in p.items() if k not in ("index", "class_id", "matrix", "stored_w", "ambient")})
    if p["stored_w"] != STORED_W:
        out["matrix_w"] = p["stored_w"]
    return out


class PortLevelWriter(LevelWriter):
    """The Godot writer's meshes and textures, with JSON instead of a scene.

    The terrain, tie, shrub and collision meshes come from LevelWriter
    unchanged (it still builds its scene text in memory; only these files
    are written). Mobys and the sky have writers of their own here: the port
    needs the class table and the shells, not Godot's marker scenes.
    """

    editable_collision = False
    godot_scales = False

    def __init__(self, out: Path, level: Level, normals=None):
        super().__init__(out, level)
        self.dir = out / self.name
        # The executable's (cos, sin) table: with it the terrain is lit as the game lights it.
        self.normals = normals
        self.classes = {"tie": {}, "shrub": {}, "moby": {}}
        self.placements = {"format": FORMAT, "ties": [], "shrubs": [], "mobys": []}
        self.sky = None

    def write(self, lod: int) -> dict:
        level, fmt = self.level, self.fmt
        self.dir.mkdir(parents=True)
        problems = level.problems if fmt.lenient else None
        sky_offset, = unpack("<I", level.index, 0x10)
        self.write_environment(sky(level.block(sky_offset), fmt) if sky_offset else None)
        # The game's lighting at level load needs the executable's normal table, which only
        # Ratchet & Clank's is known for (disc.Version.normal_table): the later games' levels go
        # out unlit, their vertex colours left to the renderer's defaults.
        lights = (light_bank(level.gameplay), self.normals) if self.normals is not None and fmt.lighting else None
        # The terrain block (usually at the core's start, offset 0); the flight world has none.
        tfrags, = unpack("<I", level.index, 0x08)
        has_terrain = not getattr(level, "no_terrain", False)
        self.write_terrain(terrain(level.block(tfrags), lod, lights, problems) + chunk_terrain(level, lod, lights, problems)
                           if has_terrain else [], lod)
        ties = tie_classes(level)
        tie_placements = tie_instances(level.instances, ties, fmt, problems)
        if lights is not None:
            # Each instance's 64 colours as LightTies leaves them at level load, RGBA bytes
            # packed little-endian (0x80 = 1.0); a vertex takes the one of its light slot.
            normals = tie_slot_normals(level)
            for p in tie_placements:
                lit = light_tie_instance(normals[p["class_id"]], p["matrix"], p["ambient"],
                                         p["directional_lights"], lights[0])
                p["colours"] = [r | g << 8 | b << 16 | a << 24 for r, g, b, a in lit]
        self.write_objects("tie", ties, tie_placements)
        shrubs = shrub_classes(level)
        shrub_placements = shrub_instances(level.instances, shrubs, fmt, problems)
        if lights is not None:
            # Each instance's 24 colours as LightShrubs leaves them at level load, one per
            # class normal; a vertex takes the one of its normal (_LIGHT_SLOT).
            normals = shrub_class_normals(level)
            for p in shrub_placements:
                lit = light_shrub_instance(normals[p["class_id"]], p["matrix"], p["colour"],
                                           p["directional_lights"], lights[0])
                p["colours"] = [r | g << 8 | b << 16 | a << 24 for r, g, b, a in lit]
        self.write_objects("shrub", shrubs, shrub_placements)
        # The level's own classes, and the ones the loader adds (the ships, spaceships.py).
        self.write_mobys(moby_instances(level.gameplay, fmt),
                         {**getattr(level, "extra_moby_classes", {}), **moby_classes(level)})
        collision, = unpack("<I", level.index, 0x14)
        if collision:  # the title world has none
            self.write_collision(level.block(collision), [block for _, _, block in level.chunks if block])
        return self.finish()

    def write_objects(self, family: str, classes: dict[int, Mesh], placements: list[dict]) -> None:
        super().write_objects(family, classes, placements)
        self.classes[family] = {str(c): {"mesh": f"{family}s/{family}_{c}.glb"} for c in sorted(classes)}
        self.placements[f"{family}s"] = [placement(p) for p in placements]

    def write_mobys(self, placements: list[dict], classes: dict[int, MobyClass | None]) -> None:
        """One GLB per class with a mesh, and the class table: mesh (or null
        for a class the viewer draws as a box), class scale and name."""
        names = moby_class_names() if self.fmt.class_names else {}
        placed = {p["class_id"] for p in placements}
        models = {}
        for class_id in sorted(set(classes) | placed):
            moby = classes.get(class_id)
            entry = {"mesh": None, "scale": moby.scale if moby is not None else 1.0}
            if moby is not None and moby.mesh is not None:
                self.moby_glb(f"mobys/moby_{class_id}.glb", moby)
                entry["mesh"] = f"mobys/moby_{class_id}.glb"
                models[class_id] = moby
            if class_id in names:
                entry["name"] = names[class_id]
            self.classes["moby"][str(class_id)] = entry
        self.placements["mobys"] = [
            {"index": p["index"], "class": p["class_id"], "matrix": p["matrix"], "rotation": list(p["rotation"]),
             "scale": p["scale"], "colour": p["fields"].get("colour"), "fields": finite(p["fields"])}
            for p in placements]
        animated = [m.skeleton for m in models.values() if m.skeleton is not None]
        self.stats["mobys"] = {"classes": len(set(classes) | placed), "instances": len(placements),
                               "model_classes": len(models),
                               "class_triangles": sum(m.mesh.triangles for m in models.values()),
                               "animated_classes": len(animated),
                               "animations": sum(len(s.animations) for s in animated)}

    def write_environment(self, data: Sky | None) -> None:
        """sky.png as for Godot, sky.glb with the shells, and the background."""
        if data is None:
            return
        width, height = PANORAMA
        (self.dir / "sky.png").write_bytes(png(width, height, 2, panorama(data, width, height)))
        for i, texture in enumerate(data.textures):
            self.textures[f"sky_{i:04}"] = texture
        (self.dir / "sky.glb").write_bytes(sky_glb(data, lambda i: f"textures/sky_{i:04}.png"))
        self.sky = {"mesh": "sky.glb", "panorama": "sky.png", "background": list(data.background)}
        self.stats["sky"] = {"shells": len(data.shells), "triangles": sum(m.triangles for m in data.shells),
                             "textures": len(data.textures), "panorama": [width, height]}

    def finish(self) -> dict:
        """Textures, placements.json and manifest.json."""
        export_pvs(self.level, self.dir)
        (self.dir / "textures").mkdir(parents=True, exist_ok=True)
        for name, texture in sorted(self.textures.items()):
            (self.dir / "textures" / f"{name}.png").write_bytes(texture.png())
        self.stats["textures"] = len(self.textures)
        # LevelWriter keeps corners in Godot's axes, (x, z, -y).
        corners = [(x, 0.0 - z, y) for x, y, z in self.corners]  # 0.0 - z: never -0.0
        bounds = ([min(p[i] for p in corners) for i in range(3)],
                  [max(p[i] for p in corners) for i in range(3)]) if corners else ([0, 0, 0], [0, 0, 0])
        self.stats["bounds"] = [list(bounds[0]), list(bounds[1])]
        if self.fmt.lenient:
            self.stats["problems"] = problem_summary(self.level.problems)
        manifest = {"format": FORMAT, "game": self.fmt.game, "version": self.level.version, "level": self.level.id,
                    "axes": "game units, Z up; matrices column-major",
                    "terrain": "terrain.glb" if (self.dir / "terrain.glb").exists() else None,
                    "collision": "collision.glb" if (self.dir / "collision.glb").exists() else None,
                    "sky": self.sky, "classes": self.classes, "placements": "placements.json",
                    "textures": sorted(f"textures/{name}.png" for name in self.textures),
                    "bounds": self.stats["bounds"], "stats": self.stats}
        bake_inputs = write_bake_inputs(self.level, self.dir)
        write_bake_profile(self.level, self.dir)
        if bake_inputs:
            manifest["bake_inputs"] = bake_inputs
        if self.fmt.lenient:
            # Early access: what the editor and the overrides packs can use, and what not yet.
            manifest["early_access"] = {"playable": False, "packable": False, "lit": False}
        if self.normals is not None and self.fmt.lighting:
            # The 16 directional light sets the level loader copies from the gameplay file
            # (colour A with its back factor in w, direction A, colour B, direction B): what
            # the game lights its mobys with.
            manifest["lights"] = [[list(struct.unpack("<4f", struct.pack("<4I", *v))) for v in s]
                                  for s in light_bank(self.level.gameplay)]
        (self.dir / "placements.json").write_text(json.dumps(self.placements, indent=1, allow_nan=False) + "\n")
        (self.dir / "manifest.json").write_text(json.dumps(manifest, indent=2, allow_nan=False) + "\n")
        return self.stats
