"""Write extracted levels as a Godot 4 project that the editor opens and edits.

Each level is a scene, levels/level_NN/level_NN.tscn:

    Level_NN              rc1/level.gd: fly camera when the scene is run
      WorldEnvironment    the sky, baked into sky.png, and flat ambient light
      Sun                 a directional light for the editor and the fly camera
      Game                game axes (Z up) rotated into Godot's (Y up)
        Terrain           terrain.glb, one node per fragment (editable children)
        Collision         collision.glb, hidden: the baked collision mesh by surface type
        Ties/Tie_NNNN     instances of ties/tie_<class>.glb
        Shrubs/Shrub_NNNN instances of shrubs/shrub_<class>.glb
        Mobys/<Class>_NNNN   instances of mobys/moby_<class>.tscn: the class's
                          mesh (or a box) and a label naming the class

Placements are node transforms in game units; fields the game stores
per instance are node metadata (rc1_*), so a packer can write them back.

Going Commando's, Up Your Arsenal's and Deadlocked's levels (wads.py) are written the same way,
with the same rc1_* metadata names, plus rc1_game on the level node. Their scenes are for looking
and making texture and model packs: the level packer reads Ratchet & Clank's only, which the
level node says (rc1_packable, and its description in the Scene dock).
"""

import colorsys
from dataclasses import replace
import json
import shutil
import struct
from pathlib import Path

from collision import collision_mesh, decode
import collision_scene
from formats import png, unpack
import gamedata
import games
from gltf import Gltf
from level import Level
from mesh import Mesh
from moby_class import GREY, METAL_CHROME, METAL_GLASS, UNTEXTURED, MobyClass, moby_classes
from mobys import LEVEL_NAMES, moby_logic, node_name, moby_class_names, moby_instances
from shrubs import shrub_classes, shrub_instances
from sky import panorama, sky
from terrain import terrain
from ties import tie_classes, tie_instances

SCRIPTS = Path(__file__).parent / "rc1"
GAME_TO_GODOT = [1, 0, 0, 0, 0, 0, -1, 0, 0, 1, 0, 0, 0, 0, 0, 1]  # (x, y, z) -> (x, z, -y)
# Placements store W = 0.01 (0.0 on some); only other values become metadata.
STORED_W = struct.unpack("<f", struct.pack("<f", 0.01))[0]
COLLISION_ALPHA = 0.55
PANORAMA = (2048, 1024)  # About one texel per pixel around the horizon.

PROJECT = """config_version=5

[application]

config/name="{name}"
run/main_scene="{main}"

[importer_defaults]

scene={{
"meshes/create_shadow_meshes": false,
"meshes/ensure_tangents": false,
"meshes/force_disable_compression": true,
"meshes/generate_lods": false
}}
texture={{
"compress/mode": 0,
"detect_3d/compress_to": 0,
"mipmaps/generate": true
}}
"""


class Raw(str):
    """A value already in Godot's text syntax, such as ExtResource("id")."""


def number(x: float) -> str:
    """The shortest text that reads back as the same float32, as Godot writes it."""
    target = struct.pack("<f", x)
    for digits in range(1, 10):
        shortest = float(f"{x:.{digits}g}")
        if struct.pack("<f", shortest) == target:
            break
    text = repr(shortest).removesuffix(".0")
    return "0" if text == "-0" else text


def transform(m: list[float]) -> Raw:
    """Transform3D lists basis rows, then the origin; m is column-major."""
    values = [m[col * 4 + row] for row in range(3) for col in range(3)] + m[12:15]
    return Raw(f"Transform3D({', '.join(map(number, values))})")


def apply(m: list[float], p) -> tuple[float, float, float]:
    return tuple(sum(m[k * 4 + i] * p[k] for k in range(3)) + m[12 + i] for i in range(3))


def value(v) -> str:
    if isinstance(v, Raw):
        return v
    if isinstance(v, bool):
        return "true" if v else "false"
    if isinstance(v, float):
        return number(v)
    if isinstance(v, list):
        return f"[{', '.join(map(value, v))}]"
    return json.dumps(v) if isinstance(v, str) else str(v)


class Scene:
    """A text scene (.tscn) built node by node."""

    def __init__(self):
        self.resources, self.subresources, self.nodes, self.editable = [], [], [], []

    @staticmethod
    def block(head: str, properties: dict) -> str:
        return "\n".join([head] + [f"{k.replace('__', '/')} = {value(v)}" for k, v in properties.items()])

    def resource(self, kind: str, path: str, rid: str) -> Raw:
        self.resources.append(f'[ext_resource type="{kind}" path="{path}" id="{rid}"]')
        return Raw(f'ExtResource("{rid}")')

    def subresource(self, kind: str, rid: str, **properties) -> Raw:
        self.subresources.append(self.block(f'[sub_resource type="{kind}" id="{rid}"]', properties))
        return Raw(f'SubResource("{rid}")')

    def node(self, name: str, parent: str | None = None, kind: str | None = None,
             instance: str | None = None, index: int | None = None, **properties) -> None:
        head = f'[node name="{name}"'
        head += f' type="{kind}"' if kind else ""
        head += f' parent="{parent}"' if parent is not None else ""
        head += f' index="{index}"' if index is not None else ""
        head += f" instance={instance}" if instance else ""
        self.nodes.append(self.block(head + "]", properties))

    def text(self) -> str:
        parts = ["[gd_scene format=3]", "\n".join(self.resources), *self.subresources, *self.nodes]
        parts += ["\n".join(f'[editable path="{path}"]' for path in self.editable)] if self.editable else []
        return "\n\n".join(p for p in parts if p) + "\n"


class LevelWriter:
    """Textures, meshes and the scene for one level."""

    # The collision as tiles to edit (collision_scene.py) where the level packs back; the port's
    # writer keeps the one collision.glb its viewer draws.
    editable_collision = True

    def __init__(self, project: Path, level: Level):
        self.level = level
        self.name = f"level_{level.id:02}"
        self.dir = project / "levels" / self.name
        self.res = f"res://levels/{self.name}"
        self.textures = {}
        self.scene = Scene()
        self.corners = []
        self.fmt = games.of(level)
        self.moby_extra = {}  # moby index -> its pvar metadata (gamedata.py)
        self.stats = {"schema": 3, "level": level.id,
                      "overlay": {"entry_point": level.overlay["entry_point"],
                                  "sections": [{k: s[k] for k in ("address", "bytes", "type")}
                                               for s in level.overlay["sections"]]},
                      "mesh_instances": 0, "meshes": 0, "triangles": 0}
        self.editable_collision = self.editable_collision and self.fmt.packable
        if self.fmt.game != "rac1":  # Ratchet & Clank's level.json stays as it was
            self.stats["game"] = self.fmt.game

    def material(self, gltf: Gltf, key, depth: int) -> int:
        # The metal packets' maps are named metal_chrome / metal_glass: the port's renderer draws
        # those faces as the game's shine pass (sphere-mapped, blended over the moby).
        name = ("moby_untextured" if key == UNTEXTURED else "metal_chrome" if key == METAL_CHROME
                else "metal_glass" if key == METAL_GLASS else f"{key[0]}_{key[1]:04}")
        if key == UNTEXTURED:
            texture = GREY
        elif not self.fmt.lenient or key in self.level.textures:
            texture = self.level.texture(*key)
        else:
            # A later game's face whose texture the level does not hold (a chrome map Going
            # Commando keeps elsewhere, for one): grey, as the game's untextured faces, and noted.
            texture = GREY
            if f"texture {name}: not in the level, drawn grey" not in self.level.problems:
                self.level.problems.append(f"texture {name}: not in the level, drawn grey")
        self.textures[name] = texture
        return gltf.material(name, f"{'../' * depth}textures/{name}.png", cutout=texture.cutout)

    def glb(self, path: str, meshes: list[Mesh], depth: int) -> str:
        """Write meshes, one node each, to path; return its res:// path."""
        gltf = Gltf()
        for mesh in meshes:
            materials = {key: self.material(gltf, key, depth) for key in mesh.faces}
            gltf.node(mesh.name, gltf.mesh(mesh, materials))
        target = self.dir / path
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes(gltf.glb())
        return f"{self.res}/{path}"

    def moby_glb(self, path: str, moby: MobyClass) -> str:
        """A moby class's mesh and, for an animated class, its skeleton (joint
        nodes in their bind pose, with a child node for joints scaled after
        the chain) and one animation per sequence. A looping sequence's name
        ends in _loop, which Godot's importer turns into a looping animation.
        """
        bones = moby.skeleton
        if bones is None:
            return self.glb(path, [moby.mesh] + ([moby.metal] if moby.metal is not None else []), 1)
        count = len(bones.joints)
        scaled = [j for j, joint in enumerate(bones.joints) if joint.post_scale]
        # Every node is a skin joint, so that Godot makes each one a bone;
        # vertices of a post-scaled joint use its child node.
        slot = {j: count + i for i, j in enumerate(scaled)}
        skins = [tuple((slot.get(j, j), w) for j, w in pairs) for pairs in moby.mesh.skins]
        gltf = Gltf()
        mesh = gltf.mesh(replace(moby.mesh, skins=skins), {key: self.material(gltf, key, 1) for key in moby.mesh.faces})
        first = len(gltf.doc["nodes"])
        post = {j: first + slot[j] for j in scaled}
        for j, joint in enumerate(bones.joints):
            children = [first + c for c, other in enumerate(bones.joints) if other.parent == j]
            children += [post[j]] if j in post else []
            gltf.node(f"joint_{j:03}", root=joint.parent is None, translation=list(joint.translation),
                      rotation=list(joint.rotation), scale=list(joint.scale), **({"children": children} if children else {}))
        for j in post:
            gltf.node(f"joint_{j:03}_post", root=False)
        skin = gltf.skin(list(range(first, first + count + len(scaled))),
                         [joint.inverse_bind for joint in bones.joints] + [bones.joints[j].inverse_bind for j in scaled])
        gltf.node(moby.mesh.name, mesh, skin=skin)
        if moby.metal is not None:
            # The metal packets' mesh, skinned to the same joints (its own node: the main mesh's
            # primitives stay first).
            metal_skins = [tuple((slot.get(j, j), w) for j, w in pairs) for pairs in moby.metal.skins]
            metal = gltf.mesh(replace(moby.metal, skins=metal_skins),
                              {key: self.material(gltf, key, 1) for key in moby.metal.faces})
            gltf.node(moby.metal.name, metal, skin=skin)
        for animation in bones.animations:
            gltf.animation(animation.name + ("_loop" if animation.loop else ""), animation.times,
                           [(post[j], "scale", values) if kind == "post_scale" else (first + j, kind, values)
                            for (j, kind), values in animation.tracks.items()])
        target = self.dir / path
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes(gltf.glb())
        return f"{self.res}/{path}"

    def place(self, mesh: Mesh, matrix: list[float] | None = None) -> None:
        """Count a placed mesh, and keep its corners in Godot space for check.gd."""
        low, high = mesh.bounds()
        for p in ((x, y, z) for x in (low[0], high[0]) for y in (low[1], high[1]) for z in (low[2], high[2])):
            x, y, z = apply(matrix, p) if matrix else p
            self.corners.append((x, z, -y))
        self.stats["mesh_instances"] += 1
        self.stats["triangles"] += mesh.triangles

    def write(self, lod: int) -> dict:
        level, fmt = self.level, self.fmt
        self.dir.mkdir(parents=True)
        script = self.scene.resource("Script", "res://rc1/level.gd", "level")
        if fmt.packable:
            self.scene.node(f"Level_{level.id:02}", kind="Node3D", script=script, metadata__rc1_level=level.id)
        else:
            # A later game's level: shown and exported, not packed back yet (pack.py).
            self.scene.node(f"Level_{level.id:02}", kind="Node3D", script=script, metadata__rc1_level=level.id,
                            metadata__rc1_game=fmt.game, metadata__rc1_packable=False,
                            editor_description=f"{fmt.title}, level {level.id} (early access).\n"
                            "Static scenery, mobys and textures to look at and to make texture and model "
                            "packs from. Edits here are not packed back into the game yet: the level "
                            "packer reads Ratchet & Clank's levels only.")
        problems = level.problems if fmt.lenient else None
        sky_offset, = unpack("<I", level.index, 0x10)
        self.write_environment(sky(level.block(sky_offset), fmt) if sky_offset else None)
        self.scene.node("Game", ".", "Node3D", transform=transform(GAME_TO_GODOT))
        self.write_terrain(terrain(level.block(unpack("<I", level.index, 0x08)[0]), lod, problems=problems)
                           + chunk_terrain(level, lod, None, problems), lod)
        ties = tie_classes(level)
        self.write_objects("tie", ties, tie_instances(level.instances, ties, fmt, problems))
        shrubs = shrub_classes(level)
        self.write_objects("shrub", shrubs, shrub_instances(level.instances, shrubs, fmt, problems))
        if fmt.packable:  # the gameplay data, packed back by pack.py (gamedata.py)
            self.moby_extra = gamedata.moby_metadata(level.gameplay)
        self.write_mobys(moby_instances(level.gameplay, fmt), moby_classes(level))
        if fmt.packable:
            self.stats["gameplay"] = gamedata.write_scene(self.scene, self.dir, self.res, level.gameplay)
        collision, = unpack("<I", level.index, 0x14)
        if collision:  # Deadlocked's multiplayer menu has none
            self.write_collision(level.block(collision), [block for _, _, block in level.chunks if block])
        return self.finish()

    def write_collision(self, block: bytes, more: list[bytes] = ()) -> None:
        """collision.glb and a hidden Game/Collision; it is not counted with the textured meshes.
        MORE: a later game's chunks' collision blocks, put in the same mesh."""
        data = decode(block)
        for extra in more:
            chunk = decode(extra)
            data.cells += chunk.cells
            data.faces += chunk.faces
            data.triangles += chunk.triangles
            data.hero_groups += chunk.hero_groups
        self.stats["collision"] = {"cells": data.cells, "faces": data.faces, "triangles": len(data.triangles),
                                   "surfaces": {str(k): v for k, v in data.surfaces().items()},
                                   "hero_groups": data.hero_groups}
        if not data.triangles:
            return
        if self.editable_collision and not more:
            # Ratchet & Clank's levels: tiles and hero walls the packer reads back (collision_scene.py).
            manifest = collision_scene.export(self.dir, block)
            self.stats["collision"]["hero_triangles"] = sum(h["triangles"] for h in manifest["hero_walls"].values())
            collision_scene.scene_nodes(self.scene, self.res, manifest)
            return
        gltf = Gltf()
        gltf.node("Collision", gltf.coloured(collision_mesh(data), gltf.overlay("collision", COLLISION_ALPHA)))
        (self.dir / "collision.glb").write_bytes(gltf.glb())
        self.scene.node("Collision", "Game", instance=self.scene.resource("PackedScene", f"{self.res}/collision.glb", "collision"),
                        visible=False)
        # Godot's glTF import ignores vertex colours, so the mesh gets the material here.
        material = self.scene.subresource("StandardMaterial3D", "collision_material", transparency=1, shading_mode=0,
                                          cull_mode=2, vertex_color_use_as_albedo=True,
                                          albedo_color=Raw(f"Color(1, 1, 1, {number(COLLISION_ALPHA)})"))
        self.scene.node("Collision", "Game/Collision", index=0, surface_material_override__0=material)
        self.scene.editable.append("Game/Collision")

    def write_terrain(self, fragments: list[Mesh], lod: int) -> None:
        path = self.glb("terrain.glb", fragments, 0)
        self.scene.node("Terrain", "Game", instance=self.scene.resource("PackedScene", path, "terrain"))
        self.scene.editable.append("Game/Terrain")
        for mesh in fragments:
            self.place(mesh)
        self.stats["meshes"] += len(fragments)
        self.stats["terrain"] = {"lod": lod, "fragments": len(fragments),
                                 "triangles": sum(m.triangles for m in fragments)}

    def write_objects(self, family: str, classes: dict[int, Mesh], placements: list[dict]) -> None:
        """One GLB per class, and one instance of it per placement."""
        group, title = f"{family.capitalize()}s", family.capitalize()
        scenes = {class_id: self.scene.resource("PackedScene", self.glb(f"{family}s/{family}_{class_id}.glb", [mesh], 1),
                                                f"{family}_{class_id}")
                  for class_id, mesh in classes.items()}
        self.scene.node(group, "Game", "Node3D")
        for p in placements:
            fields = {f"metadata__rc1_{k}": v for k, v in p.items()
                      if k not in ("class_id", "matrix", "stored_w", "ambient", "colours")}
            if p["stored_w"] != STORED_W:
                fields["metadata__rc1_matrix_w"] = p["stored_w"]
            self.scene.node(f"{title}_{p['index']:04}", f"Game/{group}", instance=scenes[p["class_id"]],
                            transform=transform(p["matrix"]), **fields)
            self.place(classes[p["class_id"]], p["matrix"])
        self.stats["meshes"] += len({p["class_id"] for p in placements})
        self.stats[f"{family}s"] = {"classes": len(classes), "instances": len(placements),
                                    "class_triangles": sum(m.triangles for m in classes.values())}

    def write_mobys(self, placements: list[dict], classes: dict[int, MobyClass | None]) -> None:
        """A scene per moby class the level loads, mobys/moby_<class>.tscn,
        and an instance of it per placed moby.

        A class scene shows the class's high-LOD mesh (mobys/moby_<class>.glb,
        with its skeleton and animations) at the class scale, or a box for a
        class without one, and a label.
        Moby meshes are counted on their own, not in the level geometry
        totals and bounds.

        rotation_order XYZ makes the Inspector show the game's own Euler
        angles (R = Rz * Ry * Rx); the transform itself is exact either way.
        """
        # The class names and update functions are Ratchet & Clank's (moby_classes.tsv, games/rac1):
        # another game's class numbers mean other classes.
        names = moby_class_names() if self.fmt.class_names else {}
        logic = moby_logic(self.level.id) if self.fmt.class_names else {}
        placed = {p["class_id"] for p in placements}
        scenes, models = {}, {}
        for class_id in sorted(set(classes) | placed):
            moby, model = classes.get(class_id), None
            if moby is not None and moby.mesh is not None:
                models[class_id] = moby
                model = (self.moby_glb(f"mobys/moby_{class_id}.glb", moby), moby.scale,
                         moby.mesh.bounds()[1][2] * moby.scale)
            path = f"mobys/moby_{class_id}.tscn"
            write_marker(self.dir / path, class_id, f"{class_id} {names[class_id]}" if class_id in names else str(class_id),
                         model, names.get(class_id), logic.get(class_id))
            if class_id in placed:
                scenes[class_id] = self.scene.resource("PackedScene", f"{self.res}/{path}", f"moby_{class_id}")
        self.scene.node("Mobys", "Game", "Node3D")
        for p in placements:
            fields = {f"metadata__rc1_{k}": v for k, v in {"index": p["index"], **p["fields"]}.items()}
            fields.update(self.moby_extra.get(p["index"], {}))
            self.scene.node(node_name(p["class_id"], p["index"], names), "Game/Mobys", instance=scenes[p["class_id"]],
                            rotation_order=0, transform=transform(p["matrix"]), **fields)
        shown = [models[p["class_id"]].mesh for p in placements if p["class_id"] in models]
        animated = [m.skeleton for m in models.values() if m.skeleton is not None]
        self.stats["mobys"] = {"classes": len(set(classes) | placed),
                               "named_classes": sum(c in names for c in set(classes) | placed),
                               "instances": len(placements), "model_classes": len(models),
                               "class_triangles": sum(m.mesh.triangles for m in models.values()),
                               "textures": len({key for m in models.values() for key in m.mesh.faces}),
                               "mesh_instances": len(shown), "triangles": sum(m.triangles for m in shown),
                               "animated_classes": len(animated),
                               "bones": sum(len(s.joints) + sum(j.post_scale for j in s.joints) for s in animated),
                               "animations": sum(len(s.animations) for s in animated)}

    def write_environment(self, data) -> None:
        """The sky as a panorama (sky.png), flat ambient light and a sun.

        The game centres its sky on the camera and never moves it, so a
        panorama shows exactly what the shells do, at infinity.
        """
        environment = {"ambient_light_source": 2, "ambient_light_color": Raw("Color(0.85, 0.9, 1, 1)"),
                       "ambient_light_energy": 0.7}
        if data is None:
            environment.update(background_mode=1, background_color=Raw("Color(0, 0, 0, 1)"))
        else:
            width, height = PANORAMA
            (self.dir / "sky.png").write_bytes(png(width, height, 2, panorama(data, width, height)))
            texture = self.scene.resource("Texture2D", f"{self.res}/sky.png", "sky_png")
            material = self.scene.subresource("PanoramaSkyMaterial", "sky_material", panorama=texture)
            environment.update(background_mode=2, sky=self.scene.subresource("Sky", "sky", sky_material=material))
            self.stats["sky"] = {"shells": len(data.shells), "triangles": sum(m.triangles for m in data.shells),
                                 "textures": len(data.textures), "panorama": [width, height]}
        self.scene.node("WorldEnvironment", ".", "WorldEnvironment",
                        environment=self.scene.subresource("Environment", "environment", **environment))
        self.scene.node("Sun", ".", "DirectionalLight3D", rotation=Raw("Vector3(-0.959931, -0.610865, 0)"),
                        light_energy=0.8)

    def finish(self) -> dict:
        """Textures, the scene and level.json."""
        (self.dir / "textures").mkdir(parents=True, exist_ok=True)
        for name, texture in sorted(self.textures.items()):
            (self.dir / "textures" / f"{name}.png").write_bytes(texture.png())
        self.stats["textures"] = len(self.textures)
        self.stats["bounds"] = [[min(p[i] for p in self.corners) for i in range(3)],
                                [max(p[i] for p in self.corners) for i in range(3)]] if self.corners else [[0, 0, 0], [0, 0, 0]]
        if self.fmt.lenient:
            self.stats["problems"] = problem_summary(self.level.problems)
        (self.dir / f"{self.name}.tscn").write_text(self.scene.text())
        (self.dir / "level.json").write_text(json.dumps(self.stats, indent=2) + "\n")
        return self.stats


def chunk_terrain(level, lod: int, lights, problems) -> list[Mesh]:
    """The terrain of a later game's streamed chunks (level.chunks), its fragments named
    Terrain_cN_NNN; nothing for Ratchet & Clank's levels, which have none."""
    meshes = []
    for chunk, tfrags, _ in getattr(level, "chunks", ()):
        if tfrags is None:
            continue
        for mesh in terrain(tfrags, lod, lights, problems):
            mesh.name = f"Terrain_c{chunk}_{mesh.name.removeprefix('Terrain_')}"
            meshes.append(mesh)
    return meshes


def problem_summary(problems: list[str]) -> list[str]:
    """What a later game's level left out, one line per class or fragment: the instances of a
    class that was not extracted are counted on its line, not listed one by one."""
    lines, counts = [], {}
    for line in problems:
        if " instance " in line and "left out with its class" in line:
            key = (line.split(" instance ")[0], line.rsplit(" ", 1)[1])
            counts[key] = counts.get(key, 0) + 1
        else:
            lines.append(line)
    lines += [f"{family} class {class_id}: {count} instance(s) left out with it" for (family, class_id), count in sorted(counts.items())]
    return lines


def write_marker(path: Path, class_id: int, label: str, model: tuple[str, float, float] | None = None,
                 name: str | None = None, logic: dict | None = None) -> None:
    """A moby class's scene: its model, or else a box coloured by class
    number, and a label that faces the camera and fades out beyond 60 units.

    model is (GLB res:// path, class scale, top of the scaled mesh). The
    GLB is in model units, so the Model node carries the class scale.
    """
    scene = Scene()
    # What the class is and what runs it, where the Inspector and a hover in the scene tree show
    # them: the function the game calls for each of its mobys every frame, in the decompilation.
    about, facts = [f"Moby class {class_id}" + (f": {name}" if name else "")], {}
    if name:
        facts["metadata__rc1_class_name"] = name
    if logic:
        facts["metadata__rc1_update"] = logic["function"]
        about.append(f"Updated every frame by {logic['function']}")
        if logic["file"]:
            facts["metadata__rc1_source"] = logic["file"]
            about.append(("C: " if logic["decompiled"] else "Still assembly, in ") + logic["file"])
    elif name is not None or model is not None:
        about.append("No update function in this level")
    scene.node(f"Moby_{class_id}", kind="Node3D", metadata__rc1_class=class_id,
               editor_description="\n".join(about), **facts)
    if model is None:
        r, g, b = colorsys.hsv_to_rgb(class_id * 0.618034 % 1.0, 0.65, 0.95)
        colour = scene.subresource("StandardMaterial3D", "colour",
                                   albedo_color=Raw(f"Color({number(r)}, {number(g)}, {number(b)}, 1)"))
        box = scene.subresource("BoxMesh", "box", material=colour, size=Raw("Vector3(0.5, 0.5, 0.5)"))
        scene.node("Marker", ".", "MeshInstance3D",
                   transform=Raw("Transform3D(1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0.25)"), mesh=box)
        height = 0.9
    else:
        glb, scale, top = model
        scene.node("Model", ".", instance=scene.resource("PackedScene", glb, "model"),
                   transform=transform([scale, 0, 0, 0, 0, scale, 0, 0, 0, 0, scale, 0, 0, 0, 0, 1]))
        height = max(top, 0.0) + 0.4
    scene.node("Label", ".", "Label3D", transform=transform([1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, height, 1]),
               visibility_range_end=60.0, billboard=1, pixel_size=0.006, text=label, font_size=48)
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(scene.text())


def write_project(project: Path, levels: list[int], region: str = "PAL", game: str = "rac1") -> None:
    """project.godot and the rc1 scripts; the main scene is the first level. A project of one
    level is named after it, and the disc's region (disc.Version.region); GAME names the game
    (games.FORMATS) for the others'."""
    project.mkdir(parents=True, exist_ok=True)
    main = f"res://levels/level_{levels[0]:02}/level_{levels[0]:02}.tscn"
    if game == "rac1":
        name = (f"Ratchet & Clank: {LEVEL_NAMES[levels[0]]} ({region})" if len(levels) == 1
                else f"Ratchet & Clank levels ({region})")
    else:
        fmt = games.FORMATS[game]
        name = (f"{fmt.title}: {fmt.level_name(levels[0])} ({region}, early access)" if len(levels) == 1
                else f"{fmt.title} levels ({region}, early access)")
    (project / "project.godot").write_text(PROJECT.format(main=main, name=name))
    shutil.copytree(SCRIPTS, project / "rc1")
