"""The level's collision in the Godot editor, editable, and read back for the packer.

The export (godot.py's LevelWriter) writes the collision as tiles, one glTF per 64 x 64-unit
square of the level, and the hero-only walls as one glTF each:

    levels/level_NN/collision/tile_XX_YY.glb    the triangles whose centre is in that square
    levels/level_NN/collision/hero_NNN.glb      hero group NNN (walls only the player meets)
    levels/level_NN/collision/collision.json    what was written: each file's name, SHA-1, triangles

    Game/Collision            hidden; metadata rc1_collision = 1 (this layout)
      Tile_XX_YY              an instance of tile_XX_YY.glb
    Game/HeroWalls            hidden
      HeroWall_NNN            an instance of hero_NNN.glb

A tile has one primitive per type byte, with a material named `rc1_surface_XX` (the byte in hex:
bits 0-4 the surface id, 5-6 the footstep sound class, bit 7 the query exclusion; COLLISION.md),
coloured as the surface. In Blender the material is the face's type: assign another
`rc1_surface_XX` material to change it. Positions are game units and axes, exactly the grid the
game stores (1/16 unit across, 1/64 up), so an untouched tile reads back as the same triangles.

What a modder can do, and what the packer makes of it:

- delete a tile or a hero wall: its triangles are removed;
- move, turn or scale a tile: its triangles move;
- edit a tile's or hero wall's glTF (Blender), keeping its name: the triangles differ;
- add a node under Game/Collision: an instance of any .glb/.gltf, or a MeshInstance3D with a
  BoxMesh, PlaneMesh or QuadMesh, with `rc1_surface` metadata for its type byte (0x1f, "no special
  surface", without one; a material named rc1_surface_XX in a glTF wins); under Game/HeroWalls, a
  new hero group the same way;
- set `rc1_surface` on a tile node: every face of it takes that type byte;
- edit faces with the editor plugin (addons/openrac_collision, the Collision dock): it leaves the
  tiles as they are and keeps its edits as metadata of Game/Collision, saved with the scene:

    rc1_removed       PackedInt32Array, ten per face: its corners x0 y0 z0 x1 y1 z1 x2 y2 z2 in
                      1/64 game units (the tiles' positions times 64, glTF's winding), its type byte
    rc1_added         the same form: faces added (a repaint or a move is the face removed and added
                      again), on the game's grid
    rc1_hero_removed  ten per face: the hero wall's number (HeroWall_NNN), then the corners

  scene_collision applies them on top of what the nodes hold.

The packer compares the scene's triangles with the disc's (collision.Block): what is gone is
removed from every cell holding it, what is new goes into every cell it touches, and every other
cell keeps its bytes (collision.edit). Standard library only.
"""

import hashlib
import json
import re
import struct
from pathlib import Path

import collision as col
from collision import SCALE, FormatError
from gltf import Gltf
import gltf_read

FORMAT = 1                        # the layout above (Game/Collision's rc1_collision)
TILE = 64 * SCALE                 # a tile's side in model units (64 game units)
ALPHA = 0.55
DEFAULT_SURFACE = 0x1f
MATERIAL = re.compile(r"rc1_surface_([0-9a-fA-F]{2})\b")
PARENTS = ("Game/Collision", "Game/HeroWalls")
EDITS = ("rc1_removed", "rc1_added", "rc1_hero_removed")  # the plugin's metadata on Game/Collision


# --- export --------------------------------------------------------------------------------------

def tile_of(p: tuple) -> tuple[int, int]:
    return (sum(q[0] for q in p) // 3 // TILE, sum(q[1] for q in p) // 3 // TILE)


def surface_material(gltf: Gltf, kind: int) -> int:
    """An unlit, translucent, double-sided material in the surface's colour, named for its type byte."""
    gltf.doc["extensionsUsed"] = ["KHR_materials_unlit"]
    gltf.doc["materials"].append({
        "name": f"rc1_surface_{kind:02x}", "doubleSided": True, "alphaMode": "BLEND",
        "pbrMetallicRoughness": {"baseColorFactor": [*(c ** 2.2 for c in col.colour(kind)), ALPHA],
                                 "metallicFactor": 0},
        "extensions": {"KHR_materials_unlit": {}}})
    return len(gltf.doc["materials"]) - 1


def triangles_glb(name: str, by_kind: dict[int, list[tuple]]) -> bytes:
    """One node with one unindexed primitive per type byte; positions in game units."""
    gltf = Gltf()
    primitives = []
    for kind in sorted(by_kind):
        rows = [tuple(c / SCALE for c in p) for t in by_kind[kind] for p in t]
        primitives.append({"attributes": {"POSITION": gltf.floats(rows, bounds=True)},
                           "material": surface_material(gltf, kind), "mode": 4})
    gltf.doc["meshes"].append({"name": name, "primitives": primitives})
    gltf.node(name, len(gltf.doc["meshes"]) - 1)
    return gltf.glb()


def export(level_dir: Path, block: bytes) -> dict:
    """Writes collision/ for a level's collision block; returns the manifest (collision.json)."""
    model = col.parse(block)
    tiles: dict[tuple[int, int], dict[int, list]] = {}
    for key, kind in sorted(col.block_triangles(model)):
        tiles.setdefault(tile_of(key), {}).setdefault(kind, []).append(key)
    folder = level_dir / "collision"
    folder.mkdir(parents=True, exist_ok=True)
    manifest = {"format": FORMAT, "tiles": {}, "hero_walls": {}}
    for (x, y), by_kind in sorted(tiles.items()):
        name, file = f"Tile_{x:02}_{y:02}", f"collision/tile_{x:02}_{y:02}.glb"
        data = triangles_glb(name, by_kind)
        (level_dir / file).write_bytes(data)
        manifest["tiles"][name] = {"file": file, "sha1": hashlib.sha1(data).hexdigest(),
                                   "triangles": sum(len(t) for t in by_kind.values())}
    for n, group in enumerate(model.heroes or []):
        name, file = f"HeroWall_{n:03}", f"collision/hero_{n:03}.glb"
        data = triangles_glb(name, {DEFAULT_SURFACE: [tuple(group.vertices[i] for i in t) for t in group.triangles]})
        (level_dir / file).write_bytes(data)
        manifest["hero_walls"][name] = {"file": file, "sha1": hashlib.sha1(data).hexdigest(),
                                        "triangles": len(group.triangles)}
    (folder / "collision.json").write_text(json.dumps(manifest, indent=1) + "\n")
    return manifest


def add_collision_nodes(scene, res: str, manifest: dict) -> None:
    """Adds the level's collision to SCENE (a godot.Scene) as nodes, in the layout above
    (moby_collision_scene.add_class_collision_nodes does it for a class's collision, and
    pack.read_scene_nodes reads a level scene's placements back)."""
    description = ("The level's collision, by 64-unit tile. Delete, move or edit tiles (their .glb in Blender; "
                   "a material rc1_surface_XX is the faces' type byte), or add meshes here with rc1_surface "
                   "metadata. Packed into the level's collision by extract.py pack.")
    scene.node("Collision", "Game", "Node3D", visible=False, editor_description=description,
               metadata__rc1_collision=FORMAT)
    for name, entry in manifest["tiles"].items():
        rid = name.lower()
        scene.node(name, "Game/Collision", instance=scene.resource("PackedScene", f"{res}/{entry['file']}", rid))
    if manifest["hero_walls"]:
        scene.node("HeroWalls", "Game", "Node3D", visible=False,
                   editor_description="Walls only the player meets (the hero groups), one node each: delete, edit "
                                      "or add meshes here. At most 255 triangles each.")
        for name, entry in manifest["hero_walls"].items():
            scene.node(name, "Game/HeroWalls",
                       instance=scene.resource("PackedScene", f"{res}/{entry['file']}", name.lower()))


# --- reading the scene ---------------------------------------------------------------------------

HEAD = re.compile(r'^\[(\w+)((?:\s+\w+=(?:"[^"]*"|\w+\([^)]*\)|[\w.-]+))*)\s*\]$')
ATTR = re.compile(r'(\w+)=("([^"]*)"|\w+\("([^"]*)"\)|[\w.-]+)')
PROPERTY = re.compile(r"^([A-Za-z_/0-9]+) = (.*)$")
IDENTITY = (1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0)


def read_scene(text: str) -> tuple[dict, dict, list[dict]]:
    """(ext resources id -> path, sub resources id -> {type, properties}, nodes) of a .tscn; a node
    is {name, type, parent, instance (path), properties}."""
    ext, sub, nodes, current = {}, {}, [], None
    for line in text.splitlines():
        head = HEAD.match(line.strip()) if line.startswith("[") else None
        if head:
            attrs = {m.group(1): m.group(3) if m.group(3) is not None else m.group(4) if m.group(4) is not None
                     else m.group(2) for m in ATTR.finditer(head.group(2))}
            current = None
            if head.group(1) == "ext_resource":
                ext[attrs.get("id")] = attrs.get("path")
            elif head.group(1) == "sub_resource":
                current = sub.setdefault(attrs.get("id"), {"type": attrs.get("type"), "properties": {}})["properties"]
            elif head.group(1) == "node":
                node = {"name": attrs.get("name"), "type": attrs.get("type"), "parent": attrs.get("parent"),
                        "instance": ext.get(attrs["instance"]) if "instance" in attrs else None, "properties": {}}
                nodes.append(node)
                current = node["properties"]
            continue
        prop = PROPERTY.match(line)
        if prop and current is not None:
            current[prop.group(1)] = prop.group(2).strip()
    return ext, sub, nodes


def numbers(value: str) -> list[float]:
    inner = re.search(r"\((.*)\)", value)
    return [float(v) for v in inner.group(1).split(",")] if inner and inner.group(1).strip() else []


def transform_of(node: dict) -> tuple:
    value = node["properties"].get("transform")
    if value is None:
        return IDENTITY
    floats = numbers(value)
    if len(floats) != 12:
        raise FormatError(f"{node['name']}: cannot read its transform")
    return tuple(floats)


def apply(t: tuple, p) -> tuple[float, float, float]:
    """A scene transform (basis rows, then the origin) on a point."""
    return tuple(t[r * 3] * p[0] + t[r * 3 + 1] * p[1] + t[r * 3 + 2] * p[2] + t[9 + r] for r in range(3))


def surface_of(node: dict) -> int | None:
    value = node["properties"].get("metadata/rc1_surface")
    if value is None:
        return None
    kind = int(float(value)) if not value.startswith('"') else int(value.strip('"'), 0)
    if not 0 <= kind <= 0xff:
        raise FormatError(f"{node['name']}: rc1_surface is a type byte, 0 to 255")
    return kind


def resolve(project: Path, path: str) -> Path:
    if not path.startswith("res://"):
        raise FormatError(f"{path}: not a res:// path of the project")
    return project / path[len("res://"):]


def gltf_triangles(path: Path) -> list[tuple[tuple, int | None]]:
    """Every triangle of a glTF file in its own space, with the type byte its material names (or None)."""
    doc = gltf_read.read(path)
    out = []
    for n, gnode in enumerate(doc.list("nodes")):
        if "mesh" not in gnode:
            continue
        world = doc.node_world(n)
        for prim in doc.list("meshes")[gnode["mesh"]]["primitives"]:
            if prim.get("mode", 4) != 4:
                raise FormatError(f"{path.name}: a primitive that is not triangles (mode {prim.get('mode')})")
            points = [gltf_read.point(world, p) for p in doc.accessor(prim["attributes"]["POSITION"])]
            order = [i for (i,) in doc.accessor(prim["indices"])] if "indices" in prim else list(range(len(points)))
            material = doc.list("materials")[prim["material"]].get("name", "") if "material" in prim else ""
            found = MATERIAL.search(material)
            kind = int(found.group(1), 16) if found else None
            out += [((points[order[i]], points[order[i + 1]], points[order[i + 2]]), kind)
                    for i in range(0, len(order) - 2, 3)]
    return out


def primitive_triangles(sub: dict, node: dict) -> list[tuple]:
    """A BoxMesh, PlaneMesh or QuadMesh as triangles in its own space, wound outward (glTF's way)."""
    kind, props = sub["type"], sub["properties"]
    if kind == "BoxMesh":
        sx, sy, sz = numbers(props.get("size", "Vector3(1, 1, 1)"))
        h = (sx / 2, sy / 2, sz / 2)
        out = []
        for axis in range(3):
            for sign in (-1, 1):
                u, v = (axis + 1) % 3, (axis + 2) % 3
                corners = []
                for a, b in ((-1, -1), (1, -1), (1, 1), (-1, 1)):
                    p = [0.0, 0.0, 0.0]
                    p[axis], p[u], p[v] = sign * h[axis], a * h[u], b * h[v]
                    corners.append(tuple(p))
                if sign < 0:
                    corners.reverse()
                out += [(corners[0], corners[1], corners[2]), (corners[0], corners[2], corners[3])]
        return out
    if kind in ("PlaneMesh", "QuadMesh"):
        sx, sy = numbers(props.get("size", "Vector2(2, 2)" if kind == "PlaneMesh" else "Vector2(1, 1)"))
        facing = int(props.get("orientation", "1" if kind == "PlaneMesh" else "2"))
        offset = numbers(props.get("center_offset", "Vector3(0, 0, 0)")) or [0.0, 0.0, 0.0]
        # The plane's two axes and its normal: FACE_X (0), FACE_Y (1), FACE_Z (2).
        u, v = {0: (2, 1), 1: (0, 2), 2: (0, 1)}[facing]
        quad = []
        for a, b in ((-1, -1), (1, -1), (1, 1), (-1, 1)):
            p = list(offset)
            p[u] += a * sx / 2
            p[v] += b * sy / 2
            quad.append(tuple(p))
        # Counter-clockwise seen from the normal's side: FACE_Y's (x, z) axes turn the other way.
        if facing in (0, 1):
            quad.reverse()
        return [(quad[0], quad[1], quad[2]), (quad[0], quad[2], quad[3])]
    raise FormatError(f"{node['name']}: a {kind} is not read as collision; use a BoxMesh, PlaneMesh, QuadMesh "
                      "or a .glb")


def node_triangles(project: Path, node: dict, sub: dict) -> list[tuple[tuple, int]]:
    """A collision node's triangles in game units, with their type bytes."""
    t, forced = transform_of(node), surface_of(node)
    found: list[tuple[tuple, int | None]]
    if node["instance"] is not None:
        path = resolve(project, node["instance"])
        if path.suffix.lower() not in (".glb", ".gltf"):
            raise FormatError(f"{node['name']}: an instance of {path.name}; collision reads .glb and .gltf files")
        if not path.is_file():
            raise FormatError(f"{node['name']}: {node['instance']} is not there")
        found = gltf_triangles(path)
    elif node["type"] == "MeshInstance3D":
        ref = re.search(r'SubResource\("([^"]+)"\)', node["properties"].get("mesh", ""))
        if not ref or ref.group(1) not in sub:
            raise FormatError(f"{node['name']}: a MeshInstance3D without a mesh of its own")
        found = [(p, None) for p in primitive_triangles(sub[ref.group(1)], node)]
    else:
        return []
    return [(tuple(apply(t, q) for q in p), forced if forced is not None else
             kind if kind is not None else DEFAULT_SURFACE) for p, kind in found]


def unchanged(project: Path, level_dir: Path, nodes: list[dict], manifest: dict) -> bool:
    """Whether the scene has the collision as the export wrote it: every node and file as written."""
    written = {**manifest["tiles"], **manifest["hero_walls"]}
    seen = set()
    for node in nodes:
        entry = written.get(node["name"])
        if entry is None or node["instance"] != f"res://{level_dir.relative_to(project).as_posix()}/{entry['file']}":
            return False
        if transform_of(node) != IDENTITY or surface_of(node) is not None:
            return False
        file = level_dir / entry["file"]
        if not file.is_file() or hashlib.sha1(file.read_bytes()).hexdigest() != entry["sha1"]:
            return False
        seen.add(node["name"])
    return seen == set(written)


def layer_edits(layer: dict) -> dict[str, list[tuple[int, ...]]]:
    """The editor plugin's edits on Game/Collision (EDITS): each a list of ten-int records."""
    out = {}
    for key in EDITS:
        value = layer["properties"].get(f"metadata/{key}")
        values = [int(v) for v in numbers(value)] if value else []
        if len(values) % 10:
            raise FormatError(f"Game/Collision: {key} holds {len(values)} numbers, not ten per face")
        out[key] = [tuple(values[i:i + 10]) for i in range(0, len(values), 10)]
    return out


def corners(record: tuple[int, ...]) -> tuple:
    return (record[0:3], record[3:6], record[6:9])


def apply_edits(result: dict, edits: dict) -> None:
    """The plugin's edits on what the nodes hold (RESULT, scene_collision's): removed faces leave,
    added ones are put on the grid and join; a removed face no node has any more is a problem."""
    stale = 0
    for record in edits["rc1_removed"]:
        key = (col.canonical(corners(record)), record[9])
        if key in result["triangles"]:
            result["triangles"].discard(key)
        else:
            stale += 1
    for record in edits["rc1_added"]:
        if not 0 <= record[9] <= 0xff:
            result["problems"].append(f"collision: an added face has type byte {record[9]}, not 0 to 255")
            continue
        q = tuple(col.quantise(tuple(c / SCALE for c in p)) for p in corners(record))
        if col.degenerate(q):
            result["dropped"] += 1
            continue
        result["triangles"].add((col.canonical(q), record[9]))
    for record in edits["rc1_hero_removed"]:
        name = f"HeroWall_{record[0]:03}"
        key = col.canonical(corners(record[1:]))
        tris = result["heroes"].get(name)
        if tris is None or key not in map(col.canonical, tris):
            stale += 1
            continue
        result["heroes"][name] = [t for t in tris if col.canonical(t) != key]
    if stale:
        result["problems"].append(f"collision: {stale} faces the collision dock removed are no longer in the "
                                  "tiles (edited elsewhere since); nothing to remove there")


def scene_collision(project: Path, level_dir: Path, text: str) -> dict | None:
    """What the scene holds of the collision: None for a project made before this layout, or one
    whose collision is as exported; else {"triangles": {(key, kind)}, "heroes": {name: [tri]},
    "added_heroes": [[tri]], "problems": [...]}, in model units."""
    manifest_file = level_dir / "collision" / "collision.json"
    if not manifest_file.is_file():
        return None
    manifest = json.loads(manifest_file.read_text())
    if manifest.get("format") != FORMAT:
        return None
    _ext, sub, nodes = read_scene(text)
    layer = next((n for n in nodes if n["name"] == "Collision" and n["parent"] == "Game"), None)
    if layer is None or "metadata/rc1_collision" not in layer["properties"]:
        return None
    ours = [n for n in nodes if n["parent"] in PARENTS]
    edits = layer_edits(layer)
    if not any(edits.values()) and unchanged(project, level_dir, ours, manifest):
        return None
    result = {"triangles": set(), "heroes": {}, "added_heroes": [], "problems": [], "dropped": 0}
    for node in ours:
        try:
            found = node_triangles(project, node, sub)
        except (FormatError, gltf_read.GltfError, ValueError, KeyError, IndexError, struct.error) as e:
            result["problems"].append(f"{node['name']}: {e}; left as it was" if node["name"] in
                                      {**manifest["tiles"], **manifest["hero_walls"]} else f"{node['name']}: {e}")
            if node["name"] in manifest["tiles"] or node["name"] in manifest["hero_walls"]:
                result.setdefault("keep", set()).add(node["name"])
            continue
        triangles, hero = [], node["parent"] == "Game/HeroWalls"
        for p, kind in found:
            if not all(abs(c) < 1e6 for q in p for c in q):
                result["problems"].append(f"{node['name']}: a vertex is not a usable number")
                break
            # Hero groups keep 1/64 on every axis and are stored as they are; the cells' grid is
            # coarser across, and a triangle flattened by it is left out.
            q = tuple(tuple(round(c * SCALE) for c in x) if hero else col.quantise(x) for x in p)
            if not hero and col.degenerate(q):
                result["dropped"] += 1
                continue
            triangles.append((q, kind))
        if hero:
            tris = [q for q, _ in triangles]
            if node["name"] in manifest["hero_walls"]:
                result["heroes"][node["name"]] = tris
            elif tris:
                result["added_heroes"].append(tris)
        else:
            result["triangles"].update((col.canonical(q), kind) for q, kind in triangles)
    apply_edits(result, edits)
    result["manifest"] = manifest
    return result


# --- packing -------------------------------------------------------------------------------------

def pack_block(block: bytes, scene: dict) -> tuple[bytes | None, list[str], list[str]]:
    """The collision block with the scene's edits (None when they change nothing), what changed,
    and what could not be packed."""
    model = col.parse(block)
    changes, problems = [], list(scene["problems"])
    if scene["dropped"]:
        changes.append(f"collision: {scene['dropped']} triangles too thin to keep on the game's grid left out")
    original = col.block_triangles(model)
    keep = scene.get("keep", set())
    if keep:
        # A tile that could not be read keeps its triangles: they count as still there.
        for key, kind in original:
            if tile_name(key) in keep:
                scene["triangles"].add((key, kind))
    removed = original - scene["triangles"]
    added = [(key, kind) for key, kind in sorted(scene["triangles"] - original)]
    edited = False
    if removed or added:
        rewritten, new, pieces = col.edit(model, removed, added)
        changes.append(f"collision: {len(removed)} triangles removed, {len(added)} added"
                       + (f" (as {pieces} once split to fit the cells)" if pieces != len(added) else "")
                       + f"; {rewritten} cells rewritten, {new} new")
        edited = True
    if model.heroes is not None:
        heroes = []
        for n, group in enumerate(model.heroes):
            name = f"HeroWall_{n:03}"
            if name in keep:
                heroes.append(group)
                continue
            if name not in scene["heroes"]:
                changes.append(f"collision: hero wall {n} deleted")
                continue
            tris = scene["heroes"][name]
            stored = [tuple(group.vertices[i] for i in t) for t in group.triangles]
            if sorted(map(col.canonical, tris)) == sorted(map(col.canonical, stored)):
                heroes.append(group)
                continue
            if not tris:
                changes.append(f"collision: hero wall {n} emptied, so deleted")
                continue
            heroes += [col.hero_group(tris[i:i + 255]) for i in range(0, len(tris), 255)]
            changes.append(f"collision: hero wall {n} edited")
        for tris in scene["added_heroes"]:
            heroes += [col.hero_group(tris[i:i + 255]) for i in range(0, len(tris), 255)]
            changes.append(f"collision: a hero wall added ({len(tris)} triangles)")
        if len(heroes) != len(model.heroes) or any(a is not b for a, b in zip(heroes, model.heroes)):
            model.heroes = heroes
            edited = True
    elif scene["added_heroes"]:
        problems.append("collision: this level has no hero group section; hero walls not added")
    if not edited:
        return None, changes, problems
    col.validate(model)
    return col.write(model), changes, problems


def tile_name(key: tuple) -> str:
    x, y = tile_of(key)
    return f"Tile_{x:02}_{y:02}"


# --- the level data: core and index with the new block -------------------------------------------

def core_with(core: bytes, index: bytes, at: int, room: int, block: bytes) -> tuple[bytes, bytes, list[str]]:
    """The core data and core index with BLOCK as the level's collision. It is written where the
    disc's was when it fits (the rest of that room zeroed); a larger one goes to the end of the core
    data (0x40-aligned), its old place zeroed, and the index's collision offset (+0x14) and core size
    (+0x8C) follow. The
    loader places what comes after the core from its decompressed size, so a larger core moves that
    up by as much."""
    if len(block) <= room:
        out = bytearray(core)
        out[at:at + room] = block + bytes(room - len(block))
        return bytes(out), index, []
    out = bytearray(core)
    # The old place is no longer read: zeros, so that the stream, which has to fit the disc's room,
    # carries almost nothing for it.
    out[at:at + room] = bytes(room)
    out += bytes(-len(core) % 0x40)
    moved = len(out)
    out += block
    new_index = bytearray(index)
    struct.pack_into("<I", new_index, 0x14, moved)
    struct.pack_into("<I", new_index, 0x8C, len(out))
    return bytes(out), bytes(new_index), [
        f"collision: {len(block) - room} bytes larger than its place, so moved to the end of the core data "
        f"(the level's memory after the core moves up {len(out) - len(core)} bytes)"]


def level_data_patches(data: bytes, core: bytes, index: bytes, original_index: bytes, compress) -> list[tuple[int, bytes]]:
    """The level data's changed byte runs, (offset in the data, bytes) whole sectors, for a new core
    (compressed into the room of the disc's core stream, padded with zeros) and index. DATA is the
    level data as on the disc (its sectors)."""
    from disc import SECTOR
    index_at, index_size = struct.unpack_from("<ii", data, 2 * 8)
    core_at, core_size = struct.unpack_from("<ii", data, 10 * 8)
    stream = compress(core, data[core_at:core_at + 16])
    if len(stream) > core_size:
        raise FormatError(f"the core data compresses to {len(stream)} bytes, {len(stream) - core_size} more than "
                          "its place in the level data")
    new = bytearray(data)
    new[core_at:core_at + core_size] = stream + bytes(core_size - len(stream))
    runs = [(core_at, core_at + core_size)]
    if index != original_index:
        new[index_at:index_at + index_size] = index
        runs.insert(0, (index_at + 0x14, index_at + 0x90))
    out = []
    for start, end in runs:
        first, last = start // SECTOR, -(-end // SECTOR)
        out.append((first * SECTOR, bytes(new[first * SECTOR:last * SECTOR])))
    return out
