#!/usr/bin/env python3
"""Moby class collision: the blob a moby class header points to at +0x10, as a model to read,
edit, check and write back byte for byte, and collision made from a mesh for a custom model.

    moby_collision.py generate MODEL.glb (--out BLOB.bin | --scene CLASS.tscn) [--mode mesh|box|sphere|cylinder]
                      [--surface 0x1f] [--scale S] [--axes gltf|game]

The blob is what the game's own query kernels read (the segment, sphere and capsule queries and
coll_sphere_mobys, docs/engine/systems/COLLISION_QUERIES.md); the layout is the one the port's
port/game/rac1/host/collision.c reads from the game's memory (adapted from ReRAC, ISC License,
Copyright (c) 2026 ReRAC contributors), measured here on every class blob of the PAL and NTSC-U
discs (docs/engine/formats/MOBY.md, section 8):

    0x00  u16  joints posed for the queries with flag 0x4 (primitive mask bit 0)
    0x02  u16  joints posed for the other queries (mask bit 1)
    0x04  s32  primitive bytes (0x20 each)
    0x08  s32  face bytes (4 each)
    0x0C  s32  vertex bytes (8 each)
    0x10       the primitives, then the vertices, then the faces

A primitive (0x20 bytes): s8 kind, u8 4 (on every one of both discs; not read by the kernels),
u16 mask (bit 0 the flag-0x4 queries, bit 1 the others, bit 15 on the last primitive), then by
kind: 1 a sphere (centre at +0x10, radius at +0x1C); 2 a sphere on a joint (s32 joint at +4,
radius at +0xC, offset from the joint at +0x10); 3 an upright cylinder (height at +4, centre of
its base at +0x10, radius at +0x1C); 4 a capsule between two joints (s16 joints at +4 and +6,
radius at +0xC). A vertex is s16 x, y, z and a zero; a face three u8 vertex indices and a type
byte (as the level collision's: COLLISION.md). Everything is in the class's packed units: the
game takes a point p of the blob to the world at the moby's position + its rotation of
p x the moby's scale / 1024, the moby's scale being the class scale times the instance's, so
the blob shares the units of the class's mesh (model units x 1024).

Standard library only.
"""

import argparse
from dataclasses import dataclass, field
import math
import struct
import sys
from pathlib import Path

from formats import FormatError, check
from util import f32

HEADER, PRIMITIVE, VERTEX, FACE = 0x10, 0x20, 8, 4
PACKED = 1024                      # packed units per model unit
SPHERE, JOINT_SPHERE, CYLINDER, CAPSULE = 1, 2, 3, 4
KIND_NAMES = {SPHERE: "sphere", JOINT_SPHERE: "joint_sphere", CYLINDER: "cylinder", CAPSULE: "capsule"}
MESH = "mesh"                      # the triangle part's kind name in the editor
POSED_QUERIES, OTHER_QUERIES, END = 0x1, 0x2, 0x8000
TAG = 4                            # byte 1 of every primitive on both discs
NO_SURFACE = 0x1f                  # what a primitive hit reports (CollType returns -1 for it)
MAX_INDEXED = 256                  # the vertices the faces' u8 indices reach
MAX_VERTICES = 2048                # the kernels transform at most this many (ReRAC's port of them)
MAX_POSED = 0x7f                   # the kernels pose at most this many joints
S16 = (-0x8000, 0x7fff)
# The record bytes each kind leaves unused (zero on both discs), kept as stored.
UNUSED = {SPHERE: ((4, 16),), JOINT_SPHERE: ((8, 12), (28, 32)), CYLINDER: ((8, 16),),
          CAPSULE: ((8, 12), (16, 32))}
UNKNOWN_UNUSED = ((4, 32),)


@dataclass
class Primitive:
    """One primitive, in packed units. CENTRE: a sphere's or the base of a cylinder; for a sphere on
    a joint its offset from the joint. JOINTS: one for a sphere on a joint, two for a capsule.
    QUERIES: mask bits 0-14. SPARE: the bytes the kind does not use, as stored."""
    kind: int
    radius: float = 0.0
    centre: tuple[float, float, float] = (0.0, 0.0, 0.0)
    height: float = 0.0
    joints: tuple[int, ...] = ()
    queries: int = POSED_QUERIES | OTHER_QUERIES
    end: bool = False
    tag: int = TAG
    spare: bytes = b""

    @property
    def name(self) -> str:
        return KIND_NAMES.get(self.kind, f"kind_{self.kind}")


@dataclass
class Blob:
    """A class's collision: POSED the joints the two kinds of query pose (header +0 and +2),
    primitives, vertices (x, y, z, w; w is 0 on the disc) and faces (a, b, c, type byte), in the
    game's winding (its normal is (v2 - v0) x (v1 - v0), as the level collision's)."""
    posed: tuple[int, int] = (0, 0)
    primitives: list[Primitive] = field(default_factory=list)
    vertices: list[tuple[int, int, int, int]] = field(default_factory=list)
    faces: list[tuple[int, int, int, int]] = field(default_factory=list)


def unused(kind: int) -> tuple:
    return UNUSED.get(kind, UNKNOWN_UNUSED)


def parse_primitive(record: bytes) -> Primitive:
    kind, tag, mask = struct.unpack_from("<bBH", record)
    p = Primitive(kind, queries=mask & 0x7fff, end=bool(mask & END), tag=tag,
                  spare=b"".join(record[a:b] for a, b in unused(kind)))
    f = struct.unpack_from("<8f", record)
    if kind == SPHERE:
        p.centre, p.radius = f[4:7], f[7]
    elif kind == JOINT_SPHERE:
        p.joints, p.radius, p.centre = struct.unpack_from("<i", record, 4), f[3], f[4:7]
    elif kind == CYLINDER:
        p.height, p.centre, p.radius = f[1], f[4:7], f[7]
    elif kind == CAPSULE:
        p.joints, p.radius = struct.unpack_from("<2h", record, 4), f[3]
    return p


def encode_primitive(p: Primitive) -> bytes:
    out = bytearray(PRIMITIVE)
    struct.pack_into("<bBH", out, 0, p.kind, p.tag, (p.queries & 0x7fff) | (END if p.end else 0))
    if p.kind == SPHERE:
        struct.pack_into("<4f", out, 0x10, *p.centre, p.radius)
    elif p.kind == JOINT_SPHERE:
        struct.pack_into("<i", out, 4, *p.joints)
        struct.pack_into("<f3f", out, 0xc, p.radius, *p.centre)
    elif p.kind == CYLINDER:
        struct.pack_into("<f", out, 4, p.height)
        struct.pack_into("<4f", out, 0x10, *p.centre, p.radius)
    elif p.kind == CAPSULE:
        struct.pack_into("<2h", out, 4, *p.joints)
        struct.pack_into("<f", out, 0xc, p.radius)
    ranges = unused(p.kind)
    if len(p.spare) == sum(b - a for a, b in ranges):
        at = 0
        for a, b in ranges:
            out[a:b] = p.spare[at:at + b - a]
            at += b - a
    return bytes(out)


def extent(data: bytes, at: int = 0) -> int:
    """The bytes of the blob at AT, from its header (it has no size of its own)."""
    check(at + HEADER <= len(data), "moby collision: the header is past the data")
    _j0, _j1, prims, faces, verts = struct.unpack_from("<HHiii", data, at)
    check(prims >= 0 and faces >= 0 and verts >= 0 and prims % PRIMITIVE == 0 and faces % FACE == 0
           and verts % VERTEX == 0, "moby collision: a part's size is not a whole number of records")
    size = HEADER + prims + faces + verts
    check(at + size <= len(data), "moby collision: its parts run past the data")
    return size


def parse(data: bytes, at: int = 0) -> Blob:
    """The model of the blob at AT; write(parse(b)) == b for every class blob of both discs."""
    extent(data, at)
    j0, j1, prims, faces, verts = struct.unpack_from("<HHiii", data, at)
    base = at + HEADER
    blob = Blob((j0, j1))
    blob.primitives = [parse_primitive(data[base + i:base + i + PRIMITIVE]) for i in range(0, prims, PRIMITIVE)]
    base += prims
    blob.vertices = [struct.unpack_from("<4h", data, base + i) for i in range(0, verts, VERTEX)]
    base += verts
    blob.faces = [tuple(data[base + i:base + i + FACE]) for i in range(0, faces, FACE)]
    return blob


def write(blob: Blob) -> bytes:
    """The blob's bytes (no padding; it is 16-byte aligned in its class and padded to 16 there)."""
    out = bytearray(struct.pack("<HHiii", *blob.posed, PRIMITIVE * len(blob.primitives), FACE * len(blob.faces),
                                VERTEX * len(blob.vertices)))
    for p in blob.primitives:
        out += encode_primitive(p)
    for v in blob.vertices:
        out += struct.pack("<4h", *v)
    for f in blob.faces:
        out += bytes(f)
    return bytes(out)


# --- checks ----------------------------------------------------------------------------------------

def joints_used(blob: Blob, queries: int) -> int:
    """How many joints the primitives in a query set (QUERIES: a mask bit) need posed: the highest
    joint they name, plus one."""
    return max((j + 1 for p in blob.primitives if p.queries & queries and p.kind in (JOINT_SPHERE, CAPSULE)
                for j in p.joints), default=0)


def derived_posed(blob: Blob) -> tuple[int, int]:
    """The fewest joints each kind of query must pose (joints_used per set). The disc's headers
    pose these or more: 1,027 of the PAL disc's 1,064 blobs exactly these, the others more (class
    186 poses six joints for primitives that name none)."""
    return joints_used(blob, POSED_QUERIES), joints_used(blob, OTHER_QUERIES)


def finish(blob: Blob) -> Blob:
    """An edited blob made consistent the way the disc's are: the end bit on the last primitive
    only, and posed joint counts that cover the primitives (the header's own when they already do)."""
    for i, p in enumerate(blob.primitives):
        p.end = i == len(blob.primitives) - 1
    blob.posed = tuple(max(have, need) for have, need in zip(blob.posed, derived_posed(blob)))
    return blob


def finite(*values: float) -> bool:
    return all(math.isfinite(v) for v in values)


def validate(blob: Blob, joint_count: int | None = None) -> None:
    """Raises FormatError for what the game cannot use: kinds, the end bit, radii, joints outside the
    class's skeleton (JOINT_COUNT: header byte 8) or not posed, vertices past the faces' u8 indices
    or the s16 range, faces naming a missing vertex."""
    n = len(blob.primitives)
    for i, p in enumerate(blob.primitives):
        what = f"moby collision: primitive {i} ({p.name})"
        check(p.kind in KIND_NAMES, f"{what}: kind {p.kind} is not one the game has (1 to 4)")
        check(p.end == (i == n - 1), f"{what}: only the last primitive carries the end bit")
        check(0 <= p.queries <= 0x7fff, f"{what}: its query mask is 15 bits")
        check(0 <= p.tag <= 0xff, f"{what}: byte 1 is a byte")
        check(finite(p.radius, p.height, *p.centre), f"{what}: a value is not a finite number")
        check(p.radius > 0, f"{what}: its radius must be above 0")
        if p.kind == CYLINDER:
            check(p.height > 0, f"{what}: its height must be above 0")
        wanted = {JOINT_SPHERE: 1, CAPSULE: 2}.get(p.kind, 0)
        check(len(p.joints) == wanted, f"{what}: names {len(p.joints)} joints, {wanted} wanted")
        for j in p.joints:
            limit = min(joint_count, MAX_POSED) if joint_count is not None else MAX_POSED
            check(0 <= j < limit, f"{what}: joint {j} is not one of the class's {limit} joints")
            for bit, count in ((POSED_QUERIES, blob.posed[0]), (OTHER_QUERIES, blob.posed[1])):
                check(not p.queries & bit or j < count,
                       f"{what}: joint {j} is past the {count} joints the header poses for its queries")
    for count in blob.posed:
        check(0 <= count <= MAX_POSED and (joint_count is None or count <= max(joint_count, 0)),
               f"moby collision: the header poses {count} joints, more than the class has")
    check(len(blob.vertices) <= MAX_VERTICES,
           f"moby collision: {len(blob.vertices)} vertices, {MAX_VERTICES} at most (the faces reach the first "
           f"{MAX_INDEXED})")
    for v in blob.vertices:
        check(len(v) == 4 and all(S16[0] <= c <= S16[1] for c in v),
               "moby collision: a vertex is outside the 16-bit range (32 model units from the class's origin)")
    for f in blob.faces:
        check(len(f) == 4 and len(set(f[:3])) == 3 and max(f[:3]) < min(len(blob.vertices), MAX_INDEXED)
               and min(f) >= 0 and f[3] <= 0xff,
               "moby collision: a face names a missing vertex (or one past the 256 its byte indices reach), "
               "repeats one, or has no type byte")


# --- triangles -------------------------------------------------------------------------------------

def canonical(t: tuple) -> tuple:
    turn = min(range(3), key=t.__getitem__)
    return t[turn:] + t[:turn]


def triangles(blob: Blob) -> list[tuple[tuple, int]]:
    """The faces as (three points in packed units, in glTF's winding), type byte."""
    out = []
    for a, b, c, kind in blob.faces:
        out.append(((blob.vertices[a][:3], blob.vertices[c][:3], blob.vertices[b][:3]), kind))
    return out


def with_triangles(blob: Blob, tris: list[tuple[tuple, int]]) -> Blob:
    """BLOB with its triangle part replaced by TRIS (points in packed units, glTF winding)."""
    vertices, where, faces = [], {}, []
    for (p0, p1, p2), kind in tris:
        corners = []
        for p in (p0, p2, p1):
            key = tuple(int(c) for c in p)
            if key not in where:
                where[key] = len(vertices)
                vertices.append((*key, 0))
            corners.append(where[key])
        if len(set(corners)) == 3:
            faces.append((*corners, kind))
    blob.vertices, blob.faces = vertices, faces
    return blob


# --- generation from a mesh ------------------------------------------------------------------------

def bounds(points: list[tuple]) -> tuple[tuple, tuple]:
    return tuple(min(p[k] for p in points) for k in range(3)), tuple(max(p[k] for p in points) for k in range(3))


def packed(p, scale: float = 1.0) -> tuple[int, int, int]:
    q = tuple(round(c * PACKED / scale) for c in p)
    check(all(S16[0] <= c <= S16[1] for c in q),
           "moby collision: the mesh reaches past 32 model units from the class's origin, which the "
           "vertices' 16 bits cannot hold; make it smaller or use --mode sphere / cylinder")
    return q


def cluster(tris: list[tuple], limit: int = MAX_INDEXED) -> list[tuple]:
    """TRIS (packed points) with their vertices merged on a grid until at most LIMIT are left:
    the coarsest step is reached by doubling, and triangles that collapse are dropped."""
    step = 1
    while True:
        def snap(p):
            return tuple(round(c / step) * step for c in p)
        out, seen, points = [], set(), set()
        for t in tris:
            s = tuple(snap(p) for p in t)
            if len(set(s)) < 3 or canonical(s) in seen:
                continue
            seen.add(canonical(s))
            out.append(s)
            points.update(s)
        if len(points) <= limit:
            return out
        step *= 2


def generate(tris: list[tuple], mode: str = MESH, surface: int = NO_SURFACE, scale: float = 1.0,
             queries: int = POSED_QUERIES | OTHER_QUERIES) -> Blob:
    """Collision for a mesh. TRIS: triangles (three points, glTF winding) in the class's model units
    times SCALE (an override model is at the drawn size: SCALE is the class scale), game axes.
    MODE: "mesh" the triangles themselves, their vertices merged until at most 256 are left;
    "box" the box around them as triangles; "sphere" one sphere around them; "cylinder" one
    upright cylinder around them. SURFACE: the type byte of the faces. The result passes validate()."""
    points = [p for t in tris for p in t]
    check(bool(points), "moby collision: the mesh has no triangles")
    check(finite(*(c for p in points for c in p)), "moby collision: the mesh has a point that is not a number")
    check(0 <= surface <= 0xff, "moby collision: the surface is a type byte, 0 to 255")
    blob = Blob()
    if mode in (MESH, "box"):
        if mode == "box":
            lo, hi = bounds(points)
            corner = [tuple((lo, hi)[(i >> k) & 1][k] for k in range(3)) for i in range(8)]
            quads = ((0, 2, 3, 1), (4, 5, 7, 6), (0, 1, 5, 4), (2, 6, 7, 3), (0, 4, 6, 2), (1, 3, 7, 5))
            tris = [t for a, b, c, d in quads for t in ((corner[a], corner[b], corner[c]), (corner[a], corner[c], corner[d]))]
        merged = cluster([tuple(packed(p, scale) for p in t) for t in tris])
        check(bool(merged), "moby collision: the mesh is too thin to keep any triangle")
        with_triangles(blob, [(t, surface) for t in merged])
    elif mode in ("sphere", "cylinder"):
        lo, hi = bounds(points)
        if mode == "sphere":
            centre = tuple((a + b) / 2 for a, b in zip(lo, hi))
            radius = max(math.dist(centre, p) for p in points)
            blob.primitives = [Primitive(SPHERE, radius * PACKED / scale, tuple(c * PACKED / scale for c in centre))]
        else:
            centre = ((lo[0] + hi[0]) / 2, (lo[1] + hi[1]) / 2, lo[2])
            radius = max(math.hypot(p[0] - centre[0], p[1] - centre[1]) for p in points)
            blob.primitives = [Primitive(CYLINDER, radius * PACKED / scale, tuple(c * PACKED / scale for c in centre),
                                         height=(hi[2] - lo[2]) * PACKED / scale)]
        p = blob.primitives[0]
        p.radius = max(f32(p.radius), 1.0)
        p.height = max(f32(p.height), 1.0) if mode == "cylinder" else 0.0
        p.centre = tuple(f32(c) for c in p.centre)
        p.queries = queries
    else:
        raise FormatError(f"moby collision: no mode {mode!r} (mesh, box, sphere, cylinder)")
    finish(blob)
    validate(blob)
    return blob


def gltf_points(path: Path) -> list[tuple]:
    """Every triangle of a glTF file in its own space."""
    import collision_scene
    return [t for t, _kind in collision_scene.gltf_triangles(path)]


def gltf_to_game(p) -> tuple:
    """glTF's axes (Y up) to the game's (Z up), as the asset export turns them."""
    return (p[0], -p[2], p[1])


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    commands = parser.add_subparsers(dest="command", required=True)
    gen = commands.add_parser("generate", help="class collision from a mesh (a custom model)")
    gen.add_argument("model", type=Path, help="a .glb or .gltf: the model the collision is for")
    where = gen.add_mutually_exclusive_group(required=True)
    where.add_argument("--out", type=Path, help="write the blob here")
    where.add_argument("--scene", type=Path, help="a class scene (levels/level_NN/mobys/moby_<class>.tscn) whose "
                                                  "Collision node gets it, for extract.py pack")
    gen.add_argument("--mode", default=MESH, choices=(MESH, "box", "sphere", "cylinder"))
    gen.add_argument("--surface", default=f"{NO_SURFACE:#x}", help="type byte of the faces (default 0x1f: none)")
    gen.add_argument("--scale", type=float, help="the class scale the model is drawn at (default: the class "
                                                 "scene's, else 1)")
    gen.add_argument("--axes", default="gltf", choices=("gltf", "game"),
                     help="gltf: Y up, as assets.py exports models (default); game: Z up, as the class scenes")
    args = parser.parse_args(argv)
    try:
        tris = gltf_points(args.model)
        if args.axes == "gltf":
            tris = [tuple(gltf_to_game(p) for p in t) for t in tris]
        scale = args.scale
        if scale is None and args.scene is not None:
            import moby_collision_scene
            scale = moby_collision_scene.scene_scale(args.scene.read_text())
        blob = generate(tris, args.mode, int(args.surface, 0), scale or 1.0)
        if args.out is not None:
            args.out.write_bytes(write(blob))
        else:
            import moby_collision_scene
            moby_collision_scene.replace_in_scene(args.scene, blob)
    except (FormatError, OSError, ValueError) as e:
        print(f"moby_collision: {e}", file=sys.stderr)
        return 1
    print(f"{len(blob.primitives)} primitives, {len(blob.vertices)} vertices, {len(blob.faces)} faces"
          + (f" -> {args.out}" if args.out else f" -> {args.scene}"))
    return 0


if __name__ == "__main__":
    sys.exit(main())
