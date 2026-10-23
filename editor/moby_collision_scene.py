"""Moby class collision in the Godot editor, editable, and packed back into the level's core data.

The export (godot.py's write_marker) gives each class scene, levels/level_NN/mobys/moby_<class>.tscn,
the class's collision (moby_collision.py) as a hidden child of its root:

    Moby_<class>                the class scene's root
      Collision                 hidden; metadata rc1_collision_format = 1; scaled by the class scale,
                                so its children are in the class's model units (packed / 1024)
        Sphere_NN               MeshInstance3D, SphereMesh: a sphere (kind 1)
        JointSphere_NN          SphereMesh on joint rc1_joint, drawn at the bind pose (kind 2)
        Cylinder_NN             CylinderMesh, upright (kind 3)
        Capsule_NN              CapsuleMesh between joints rc1_joints, drawn at the bind pose (kind 4)
        Mesh_XX                 an instance of collision/moby_<class>_XX.glb: the triangles of type byte XX

Each child carries rc1_collision_kind (sphere, joint_sphere, cylinder, capsule, mesh) and
rc1_surface (a mesh's type byte; 0x1f for the primitives, which have none: a primitive hit reports
no surface), a primitive also rc1_index (its place in the blob) and rc1_queries (mask bits: 1 the
queries with flag 0x4, 2 the others). Colours are the level collision's (collision.colour).

What a modder can do, and what the packer makes of it: move, resize or delete a child (a sphere's
or cylinder's place and size, a joint sphere's offset and radius, a capsule's radius; a capsule
follows its joints, set by rc1_joints); change rc1_queries, rc1_joint, rc1_joints, rc1_surface;
edit a Mesh_XX glTF in Blender (a material named rc1_surface_YY gives its faces that type byte);
add a MeshInstance3D with a SphereMesh, CylinderMesh or CapsuleMesh (rc1_joint on a sphere puts
it on that joint; a capsule needs rc1_joints) or a BoxMesh, PlaneMesh, QuadMesh or a .glb instance
(triangles). Deleting every child removes the class's collision.

A class's collision is one blob in the level's core data that every moby of the class uses, so
an edit applies to all of that level's mobys of the class; each level has its own core, so the same
class in another level keeps its own. The packer writes the blob where the disc has it when it fits,
else at the end of the core data (the class header's +0x10 is an offset from the class, which the
loader turns into a pointer, so the blob can be anywhere in the core). Standard library only.
"""

import hashlib
import json
import math
import re
import struct
from pathlib import Path

import collision as col
import collision_scene as cs
from formats import FormatError
import gltf_read
import moby_collision as mc
from util import cross, f32

FORMAT = 1
ALPHA = 0.55
MANIFEST = "collision.json"            # mobys/collision.json: what the export wrote
NODE_NAMES = {mc.SPHERE: "Sphere", mc.JOINT_SPHERE: "JointSphere", mc.CYLINDER: "Cylinder", mc.CAPSULE: "Capsule"}
MESH_NODE = re.compile(r"^Mesh_([0-9a-fA-F]{2})$")
# Godot's defaults, which it leaves out of a saved scene.
DEFAULTS = {"SphereMesh": {"radius": 0.5, "height": 1.0},
            "CylinderMesh": {"top_radius": 0.5, "bottom_radius": 0.5, "height": 2.0},
            "CapsuleMesh": {"radius": 0.5, "height": 2.0}}
SHAPES = {"SphereMesh": mc.SPHERE, "CylinderMesh": mc.CYLINDER, "CapsuleMesh": mc.CAPSULE}
KINDS = {name: kind for kind, name in mc.KIND_NAMES.items()}


# --- the class: its blob and its joints ------------------------------------------------------------

def class_collision(core: bytes, start: int) -> tuple[int, mc.Blob | None]:
    """The offset (+0x10) of the class at START in the core data, and its blob (None without one)."""
    at, = struct.unpack_from("<i", core, start + 0x10)
    return at, (mc.parse(core, start + at) if at else None)


def room(core: bytes, start: int) -> int:
    """The bytes the class's blob may fill where it is: its own, padded to 16."""
    at, = struct.unpack_from("<i", core, start + 0x10)
    return -(-mc.extent(core, start + at) // 16) * 16 if at else 0


def joint_positions(core: bytes, start: int) -> list[tuple[float, float, float]]:
    """Each joint's place in the bind pose, packed units: the translation of the inverse of its
    skeleton matrix (class +0x14), which the posed joint matrices the queries read reduce to when
    the moby stands in its bind pose. Empty for a class without a skeleton."""
    from moby_anim import apply, inverse3
    count = core[start + 8]
    skeleton, = struct.unpack_from("<i", core, start + 0x14)
    if not count or not skeleton:
        return []
    out = []
    for j in range(count):
        rows = [struct.unpack_from("<3f", core, start + skeleton + j * 64 + r * 16) for r in range(4)]
        a = [[rows[c][r] for c in range(3)] for r in range(3)]
        try:
            inverse, _det = inverse3(a)
        except FormatError:
            out.append((0.0, 0.0, 0.0))
            continue
        out.append(tuple(-v for v in apply(inverse, rows[3])))
    return out


def joint_at(joints: list, j: int) -> tuple:
    return joints[j] if 0 <= j < len(joints) else (0.0, 0.0, 0.0)


# --- export ----------------------------------------------------------------------------------------

def number(x: float) -> str:
    from godot import number as n
    return n(x)


def raw(text: str):
    from godot import Raw
    return Raw(text)


def column_major(basis_columns: tuple, origin: tuple) -> list[float]:
    """A column-major 4 x 4 (godot.transform's input) from three basis columns and an origin."""
    return [*basis_columns[0], 0.0, *basis_columns[1], 0.0, *basis_columns[2], 0.0, *origin, 1.0]


def unit(v: tuple) -> tuple:
    n = math.sqrt(sum(c * c for c in v))
    return tuple(c / n for c in v)


def along(axis: tuple) -> tuple:
    """Basis columns whose Y is AXIS (unit): how a Godot cylinder or capsule (along its Y) is turned."""
    other = (1.0, 0.0, 0.0) if abs(axis[0]) < 0.9 else (0.0, 0.0, 1.0)
    x = unit(cross(axis, other))
    z = cross(x, axis)
    return (x, axis, z)


AXES = ((1.0, 0.0, 0.0), (0.0, 1.0, 0.0), (0.0, 0.0, 1.0))
UPRIGHT = ((1.0, 0.0, 0.0), (0.0, 0.0, 1.0), (0.0, -1.0, 0.0))  # Godot's Y up to the game's Z up


def primitive_shape(p: mc.Primitive, joints: list) -> tuple[str, dict, list[float]]:
    """The Godot mesh (type, properties) and the node's column-major transform of a primitive, in
    model units."""
    r = p.radius / mc.PACKED
    if p.kind == mc.SPHERE:
        return "SphereMesh", {"radius": r, "height": 2 * r}, column_major(AXES, tuple(c / mc.PACKED for c in p.centre))
    if p.kind == mc.JOINT_SPHERE:
        at = tuple((a + b) / mc.PACKED for a, b in zip(joint_at(joints, p.joints[0]), p.centre))
        return "SphereMesh", {"radius": r, "height": 2 * r}, column_major(AXES, at)
    if p.kind == mc.CYLINDER:
        h = p.height / mc.PACKED
        base = tuple(c / mc.PACKED for c in p.centre)
        return ("CylinderMesh", {"top_radius": r, "bottom_radius": r, "height": h},
                column_major(UPRIGHT, (base[0], base[1], base[2] + h / 2)))
    a, b = (tuple(c / mc.PACKED for c in joint_at(joints, j)) for j in p.joints[:2])
    d = tuple(y - x for x, y in zip(a, b))
    length = math.sqrt(sum(c * c for c in d))
    basis = along(unit(d)) if length > 1e-6 else UPRIGHT
    middle = tuple((x + y) / 2 for x, y in zip(a, b))
    return "CapsuleMesh", {"radius": r, "height": length + 2 * r}, column_major(basis, middle)


def material(scene, kind: int, cache: dict):
    if kind not in cache:
        red, green, blue = col.colour(kind)
        cache[kind] = scene.subresource("StandardMaterial3D", f"collision_{kind:02x}", transparency=1, shading_mode=0,
                                        cull_mode=2, albedo_color=raw(f"Color({number(red)}, {number(green)}, "
                                                                      f"{number(blue)}, {number(ALPHA)})"))
    return cache[kind]


def node_metadata(p: mc.Primitive, index: int) -> dict:
    meta = {"metadata__rc1_collision_kind": p.name, "metadata__rc1_surface": mc.NO_SURFACE,
            "metadata__rc1_index": index, "metadata__rc1_queries": p.queries}
    if p.kind == mc.JOINT_SPHERE:
        meta["metadata__rc1_joint"] = p.joints[0]
    elif p.kind == mc.CAPSULE:
        meta["metadata__rc1_joints"] = list(p.joints)
    return meta


def scene_nodes(scene, blob: mc.Blob | None, joints: list, class_scale: float, class_id: int,
                level_dir: Path, res: str) -> dict:
    """Adds the Collision node and its children to a class scene (godot.Scene) and writes the mesh
    part's glTF files under LEVEL_DIR/mobys/collision; returns what was written, for level.json."""
    from godot import transform
    description = ("The class's collision, as every moby of this class in this level has it (moby_collision.py). "
                   "Move, resize, delete or add spheres, cylinders and capsules here, or edit a Mesh_XX glTF; "
                   "extract.py pack writes it back.")
    s = class_scale
    scene.node("Collision", ".", "Node3D", visible=False, editor_description=description,
               transform=transform([s, 0, 0, 0, 0, s, 0, 0, 0, 0, s, 0, 0, 0, 0, 1]),
               metadata__rc1_collision_format=FORMAT, metadata__rc1_class_scale=s)
    stats = {"primitives": 0, "triangles": 0, "files": {}}
    if blob is None:
        return stats
    cache: dict = {}
    for i, p in enumerate(blob.primitives):
        if p.kind not in mc.KIND_NAMES:
            continue
        kind, props, m = primitive_shape(p, joints)
        mesh = scene.subresource(kind, f"collision_shape_{i}", material=material(scene, mc.NO_SURFACE, cache),
                                 **{k: v for k, v in props.items()})
        scene.node(f"{NODE_NAMES[p.kind]}_{i:02}", "Collision", "MeshInstance3D", transform=transform(m), mesh=mesh,
                   **node_metadata(p, i))
        stats["primitives"] += 1
    by_kind: dict[int, list] = {}
    for points, kind in mc.triangles(blob):
        # triangles_glb writes its points / 64 (collision.SCALE): packed / 16 lands in model units.
        by_kind.setdefault(kind, []).append(tuple(tuple(c * col.SCALE / mc.PACKED for c in q) for q in points))
    for kind in sorted(by_kind):
        name = f"Mesh_{kind:02x}"
        file = f"mobys/collision/moby_{class_id}_{kind:02x}.glb"
        data = cs.triangles_glb(name, {kind: by_kind[kind]})
        (level_dir / file).parent.mkdir(parents=True, exist_ok=True)
        (level_dir / file).write_bytes(data)
        stats["files"][file] = hashlib.sha1(data).hexdigest()
        scene.node(name, "Collision", instance=scene.resource("PackedScene", f"{res}/{file}", f"collision_{kind:02x}"),
                   metadata__rc1_collision_kind=mc.MESH, metadata__rc1_surface=kind)
        stats["triangles"] += len(by_kind[kind])
    return stats


def write_manifest(level_dir: Path, scenes: dict[int, str], files: dict[str, str]) -> None:
    """mobys/collision.json: each class scene's and collision file's SHA-1 as exported, so an
    untouched project packs without reading the disc's classes."""
    manifest = {"format": FORMAT,
                "scenes": {str(c): hashlib.sha1((level_dir / path).read_bytes()).hexdigest()
                           for c, path in sorted(scenes.items())},
                "files": dict(sorted(files.items()))}
    (level_dir / "mobys").mkdir(parents=True, exist_ok=True)
    (level_dir / "mobys" / MANIFEST).write_text(json.dumps(manifest, indent=1) + "\n")


# --- reading a class scene back --------------------------------------------------------------------

def meta(node: dict, key: str):
    value = node["properties"].get(f"metadata/rc1_{key}")
    if value is None:
        return None
    value = value.strip()
    if value.startswith('"'):
        return value.strip('"')
    if value.startswith("["):
        return [int(float(v)) for v in value.strip("[]").split(",") if v.strip()]
    number = float(value)
    return int(number) if number == int(number) else number


def collision_layer(nodes: list[dict]) -> dict | None:
    return next((n for n in nodes if n["name"] == "Collision" and n["parent"] == "."
                 and "metadata/rc1_collision_format" in n["properties"]), None)


def scene_scale(text: str) -> float:
    """The class scale a class scene's Collision node carries."""
    _ext, _sub, nodes = cs.read_scene(text)
    layer = collision_layer(nodes)
    value = meta(layer, "class_scale") if layer is not None else None
    if value is None:
        raise FormatError("not a class scene with a Collision node (export the level again)")
    return float(value)


def columns(t: tuple) -> list[tuple]:
    """The basis columns of a scene transform (rows, then origin)."""
    return [(t[k], t[3 + k], t[6 + k]) for k in range(3)]


def length(v: tuple) -> float:
    return math.sqrt(sum(c * c for c in v))


def mesh_properties(sub: dict, node: dict) -> tuple[str, dict]:
    ref = re.search(r'SubResource\("([^"]+)"\)', node["properties"].get("mesh", ""))
    if not ref or ref.group(1) not in sub:
        raise FormatError(f"{node['name']}: a MeshInstance3D without a mesh of its own")
    kind, props = sub[ref.group(1)]["type"], sub[ref.group(1)]["properties"]
    values = dict(DEFAULTS.get(kind, {}))
    for key in values:
        if key in props:
            values[key] = float(props[key])
    return kind, values


def node_primitive(node: dict, sub: dict, joints: list) -> mc.Primitive:
    """A primitive child as the scene has it, packed units."""
    t = cs.transform_of(node)
    cols, origin = columns(t), t[9:12]
    shape, values = mesh_properties(sub, node)
    named = meta(node, "collision_kind")
    kind = KINDS.get(named) if named is not None else SHAPES.get(shape)
    if kind is None:
        raise FormatError(f"{node['name']}: rc1_collision_kind {named!r} is none of "
                          f"{', '.join(KIND_NAMES)} or mesh")
    if kind == mc.SPHERE and named is None and meta(node, "joint") is not None:
        kind = mc.JOINT_SPHERE
    wanted = {mc.SPHERE: "SphereMesh", mc.JOINT_SPHERE: "SphereMesh", mc.CYLINDER: "CylinderMesh",
              mc.CAPSULE: "CapsuleMesh"}[kind]
    if shape != wanted:
        raise FormatError(f"{node['name']}: a {mc.KIND_NAMES[kind]} is drawn with a {wanted}, not a {shape}")
    queries = meta(node, "queries")
    p = mc.Primitive(kind, queries=mc.POSED_QUERIES | mc.OTHER_QUERIES if queries is None else int(queries))
    scales = [length(c) for c in cols]
    if kind in (mc.SPHERE, mc.JOINT_SPHERE):
        if max(scales) - min(scales) > 1e-3 * max(scales):
            raise FormatError(f"{node['name']}: a sphere is scaled the same on every axis")
        p.radius = values["radius"] * scales[0] * mc.PACKED
        centre = tuple(c * mc.PACKED for c in origin)
        if kind == mc.JOINT_SPHERE:
            joint = meta(node, "joint")
            if not isinstance(joint, int):
                raise FormatError(f"{node['name']}: a sphere on a joint needs rc1_joint")
            p.joints = (joint,)
            centre = tuple(c - j for c, j in zip(centre, joint_at(joints, joint)))
        p.centre = centre
    elif kind == mc.CYLINDER:
        up = cols[1]
        if math.hypot(up[0], up[1]) > 1e-3 * length(up):
            raise FormatError(f"{node['name']}: the game's cylinders stand upright; it may not be tilted")
        if abs(scales[0] - scales[2]) > 1e-3 * max(scales[0], scales[2]):
            raise FormatError(f"{node['name']}: a cylinder is round: scale its X and Z the same")
        if values["top_radius"] != values["bottom_radius"]:
            raise FormatError(f"{node['name']}: a cylinder has one radius (top_radius = bottom_radius)")
        height = values["height"] * scales[1]
        p.radius = values["top_radius"] * scales[0] * mc.PACKED
        p.height = height * mc.PACKED
        p.centre = (origin[0] * mc.PACKED, origin[1] * mc.PACKED, (origin[2] - height / 2) * mc.PACKED)
    else:
        pair = meta(node, "joints")
        if not isinstance(pair, list) or len(pair) != 2:
            raise FormatError(f"{node['name']}: a capsule needs rc1_joints, the two joints it runs between")
        p.joints = tuple(pair)
        p.radius = values["radius"] * min(scales[0], scales[2]) * mc.PACKED
    p.radius, p.height = f32(p.radius), f32(p.height)
    p.centre = tuple(f32(c) for c in p.centre)
    return p


def close(a: float, b: float) -> bool:
    return abs(a - b) <= 1e-3 + 1e-5 * max(abs(a), abs(b))


def same_primitive(scene: mc.Primitive, disc: mc.Primitive) -> bool:
    """Whether a scene primitive is the disc's one as exported: same kind, joints and mask, and
    its numbers equal up to the text and bind-pose arithmetic of the round trip."""
    if scene.kind != disc.kind or tuple(scene.joints) != tuple(disc.joints) or scene.queries != disc.queries:
        return False
    if not close(scene.radius, disc.radius):
        return False
    if disc.kind == mc.CYLINDER and not close(scene.height, disc.height):
        return False
    return disc.kind == mc.CAPSULE or all(close(a, b) for a, b in zip(scene.centre, disc.centre))


def mesh_triangles(project: Path, node: dict, sub: dict) -> list[tuple[tuple, int]]:
    """A triangle child's triangles, packed units (glTF winding), with their type bytes. For a
    Mesh_XX node still of type XX, a glTF material rc1_surface_YY names its faces' type; a node
    given another rc1_surface gives every face that one."""
    surface = cs.surface_of(node)
    exported = MESH_NODE.match(node["name"])
    t = cs.transform_of(node)
    if node["instance"] is not None:
        path = cs.resolve(project, node["instance"])
        if path.suffix.lower() not in (".glb", ".gltf"):
            raise FormatError(f"{node['name']}: an instance of {path.name}; collision reads .glb and .gltf files")
        if not path.is_file():
            raise FormatError(f"{node['name']}: {node['instance']} is not there")
        found = cs.gltf_triangles(path)
    elif node["type"] == "MeshInstance3D":
        ref = re.search(r'SubResource\("([^"]+)"\)', node["properties"].get("mesh", ""))
        if not ref or ref.group(1) not in sub:
            raise FormatError(f"{node['name']}: a MeshInstance3D without a mesh of its own")
        found = [(p, None) for p in cs.primitive_triangles(sub[ref.group(1)], node)]
    else:
        return []
    forced = surface is not None and not (exported and int(exported.group(1), 16) == surface)
    default = surface if surface is not None else mc.NO_SURFACE
    out = []
    for points, kind in found:
        q = tuple(tuple(round(c * mc.PACKED) for c in cs.apply(t, p)) for p in points)
        if not all(mc.S16[0] <= c <= mc.S16[1] for p in q for c in p):
            raise FormatError(f"{node['name']}: reaches past 32 model units from the class's origin")
        out.append((q, default if forced or kind is None else kind))
    return out


def scene_blob(project: Path, text: str, disc: mc.Blob | None, joints: list) -> tuple[bool, mc.Blob | None, list[str]]:
    """What a class scene holds of its class's collision: (edited, the blob (None: no collision),
    problems). A scene without the Collision node (an older export) is not edited. Primitives and
    triangles as the export wrote them keep the disc's records and order."""
    _ext, sub, nodes = cs.read_scene(text)
    if collision_layer(nodes) is None:
        return False, disc, []
    problems, prims, tris = [], [], []
    for node in nodes:
        if node["parent"] != "Collision":
            continue
        try:
            kind = meta(node, "collision_kind")
            is_mesh = (kind == mc.MESH or node["instance"] is not None or
                       (kind is None and node["type"] == "MeshInstance3D" and
                        mesh_properties(sub, node)[0] not in SHAPES))
            if is_mesh:
                tris += mesh_triangles(project, node, sub)
                continue
            if node["type"] != "MeshInstance3D":
                continue
            p = node_primitive(node, sub, joints)
            index = meta(node, "index")
            original = disc.primitives[index] if (disc is not None and isinstance(index, int)
                                                  and 0 <= index < len(disc.primitives)) else None
            if original is not None and same_primitive(p, original):
                p = original
            elif original is not None:
                p.tag, p.spare = original.tag, original.spare if original.kind == p.kind else b""
            prims.append((index if original is not None else 1 << 30, len(prims), p))
        except (FormatError, ValueError, KeyError, IndexError, struct.error, gltf_read.GltfError) as e:
            problems.append(f"{node['name']}: {e}")
    if problems:
        return False, disc, problems
    prims.sort(key=lambda e: e[:2])
    ordered = [p for *_, p in prims]
    old_tris = sorted((mc.canonical(t), k) for t, k in mc.triangles(disc)) if disc is not None else []
    new_tris = sorted((mc.canonical(t), k) for t, k in tris)
    if disc is not None and old_tris == new_tris and len(ordered) == len(disc.primitives) and all(
            a is b for a, b in zip(ordered, disc.primitives)):
        return False, disc, []
    if disc is None and not ordered and not tris:
        return False, None, []
    if not ordered and not tris:
        return True, None, []
    import copy
    blob = mc.Blob(disc.posed if disc is not None else (0, 0), [copy.copy(p) for p in ordered])
    if disc is not None and old_tris == new_tris:
        blob.vertices, blob.faces = list(disc.vertices), list(disc.faces)
    else:
        mc.with_triangles(blob, tris)
    mc.finish(blob)
    return True, blob, []


# --- the class scene from a command ----------------------------------------------------------------

def replace_in_scene(path: Path, blob: mc.Blob) -> None:
    """Puts BLOB (made by moby_collision.generate) into a class scene's Collision node in place of
    its children, with the triangle part as glTF files beside the others: what pack then reads.
    The old children go, with the resources nothing else in the scene uses."""
    from godot import Scene
    text = path.read_text()
    _ext, _sub, nodes = cs.read_scene(text)
    layer = collision_layer(nodes)
    found = re.fullmatch(r"moby_(\d+)", path.stem)
    if layer is None or not found:
        raise FormatError(f"{path.name}: not a class scene with a Collision node (export the level again)")
    class_id, level_dir = int(found.group(1)), path.parent.parent
    project = next((p for p in path.parents if (p / "project.godot").is_file()), None)
    if project is None:
        raise FormatError(f"{path}: not inside a Godot project")
    blocks = [b for b in re.split(r"\n+(?=\[)", text.strip("\n"))]
    blocks = [b for b in blocks if not (b.startswith("[node ") and 'parent="Collision"' in b.splitlines()[0])]
    while True:  # resources no remaining block refers to
        used = set(re.findall(r'(?:Ext|Sub)Resource\("([^"]+)"\)', "\n".join(blocks)))
        keep = [b for b in blocks if not (re.match(r"\[(ext|sub)_resource ", b) and
                                          re.search(r' id="([^"]+)"', b.splitlines()[0]).group(1) not in used)]
        if len(keep) == len(blocks):
            break
        blocks = keep
    taken = set(re.findall(r' id="([^"]+)"', "\n".join(b.splitlines()[0] for b in blocks)))
    for stale in (level_dir / "mobys" / "collision").glob(f"moby_{class_id}_*.glb"):
        stale.unlink()
    part = Scene()
    scene_nodes(part, blob, [], float(meta(layer, "class_scale") or 1.0), class_id, level_dir,
                "res://" + level_dir.relative_to(project).as_posix())
    prefix = "collision_"
    while any(t.startswith(prefix) for t in taken):
        prefix = "new_" + prefix

    def renamed(b: str) -> str:
        return re.sub(r'(id="|Resource\(")collision_', lambda m: m.group(1) + prefix, b)

    resources = [renamed(r) for r in part.resources]
    subs = [renamed(b) for b in part.subresources]
    children = [renamed(n) for n in part.nodes if 'parent="Collision"' in n.splitlines()[0]]
    first_sub = next((i for i, b in enumerate(blocks) if b.startswith(("[sub_resource", "[node"))), len(blocks))
    first_node = next((i for i, b in enumerate(blocks) if b.startswith("[node")), len(blocks))
    out = blocks[:first_sub] + resources + blocks[first_sub:first_node] + subs + blocks[first_node:] + children
    out[0] = re.sub(r" load_steps=\d+", "", out[0])  # Godot counts them again
    path.write_text("\n\n".join(out) + "\n")


# --- packing ---------------------------------------------------------------------------------------

def unchanged_scenes(level_dir: Path) -> bool:
    """Whether every class scene and collision file is as the export wrote it (mobys/collision.json)."""
    file = level_dir / "mobys" / MANIFEST
    if not file.is_file():
        return True
    manifest = json.loads(file.read_text())
    if manifest.get("format") != FORMAT:
        return True
    folder = level_dir / "mobys"
    for class_id, sha1 in manifest["scenes"].items():
        scene = folder / f"moby_{class_id}.tscn"
        if not scene.is_file() or hashlib.sha1(scene.read_bytes()).hexdigest() != sha1:
            return False
    for path, sha1 in manifest["files"].items():
        if not (level_dir / path).is_file() or hashlib.sha1((level_dir / path).read_bytes()).hexdigest() != sha1:
            return False
    extra = {p.name for p in folder.glob("moby_*.tscn")} - {f"moby_{c}.tscn" for c in manifest["scenes"]}
    return not extra


def class_starts(level) -> dict[int, int]:
    return {struct.unpack_from("<i", e, 4)[0]: struct.unpack_from("<i", e)[0] for e in level.table(0x18, 32)}


def pack_classes(level, project: Path, level_dir: Path) -> tuple[dict[int, bytes | None], list[str], list[str]]:
    """The classes whose collision the level's class scenes changed: class -> its new blob (None:
    the class loses its collision), what changed, what could not be packed."""
    if unchanged_scenes(level_dir):
        return {}, [], []
    starts = class_starts(level)
    edits, changes, problems = {}, [], []
    for path in sorted((level_dir / "mobys").glob("moby_*.tscn"), key=lambda p: int(re.sub(r"\D", "", p.stem) or 0)):
        found = re.fullmatch(r"moby_(\d+)", path.stem)
        if not found:
            continue
        class_id = int(found.group(1))
        start = starts.get(class_id, 0)
        text = path.read_text()
        if start <= 0:
            if collision_layer(cs.read_scene(text)[2]) is not None and re.search(r'parent="Collision"', text):
                problems.append(f"moby class {class_id}: it has no class data in this level's core (a gadget, or a "
                                "class without a model); its collision is not packed")
            continue
        try:
            at, disc = class_collision(level.core, start)
            joints = joint_positions(level.core, start)
            edited, blob, issues = scene_blob(project, text, disc, joints)
            problems += [f"moby class {class_id}: {p}; its collision is left as the disc has it" for p in issues]
            if not edited:
                continue
            if blob is not None:
                mc.validate(blob, level.core[start + 8])
        except FormatError as e:
            problems.append(f"moby class {class_id}: {e}; its collision is left as the disc has it")
            continue
        edits[class_id] = None if blob is None else mc.write(blob)
        whom = "every moby of the class in this level; other levels keep their own"
        if blob is None:
            changes.append(f"moby class {class_id}: collision removed ({whom})")
        else:
            changes.append(f"moby class {class_id}: collision edited, {len(blob.primitives)} primitives and "
                           f"{len(blob.faces)} triangles ({whom})")
    return edits, changes, problems


def core_with_classes(core: bytes, index: bytes, level, edits: dict[int, bytes | None]) -> tuple[bytes, bytes, list[str]]:
    """The core data and index with each edited class's blob: in its place when it fits (the rest
    of that room zeroed); else at the end of the core data (16-byte aligned, as the disc's), its old
    place zeroed, the class header's +0x10 pointing there and the index's core size (+0x8C)
    following. A class whose blob is None gets +0x10 = 0: no collision."""
    out, notes = bytearray(core), []
    starts = class_starts(level)
    for class_id, data in sorted(edits.items()):
        start = starts[class_id]
        at, = struct.unpack_from("<i", out, start + 0x10)
        space = room(bytes(out), start)
        if at:
            out[start + at:start + at + space] = bytes(space)
        if data is None:
            struct.pack_into("<i", out, start + 0x10, 0)
            continue
        padded = data + bytes(-len(data) % 16)
        if at and len(padded) <= space:
            out[start + at:start + at + len(padded)] = padded
            continue
        out += bytes(-len(out) % 0x40)
        moved = len(out)
        out += padded
        struct.pack_into("<i", out, start + 0x10, moved - start)
        notes.append(f"moby class {class_id}: its collision is {len(padded) - space} bytes larger than its place, so "
                     f"moved to the end of the core data")
    if len(out) != len(core):
        index = bytearray(index)
        struct.pack_into("<I", index, 0x8C, len(out))
        index = bytes(index)
    return bytes(out), index, notes
