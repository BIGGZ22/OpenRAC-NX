"""The level's gameplay data in the Godot scene, and packed back: pvars, links between objects,
moby groups, volumes, paths, grind paths, cameras, sound instances and the level settings.

RAC1 moby behaviour is native code (the decompilation's C), but the gameplay file says what each
placed object starts with. The sections read and written here (header words of the decompressed
gameplay file; docs/engine/formats/LEVEL.md section 8):

    +0x00  level settings   0x50 bytes: background and fog colours (three s32 each), fog near
                            and far distance and intensity, death height, ship position and
                            rotation about Z (f32), ship path, ship camera cuboids (s32), pad[2]
    +0x08  cameras          count, pad[3]; 0x20 records {s32 class, f32 position[3],
                            f32 rotation[3], s32 pvar}
    +0x0C  sound instances  count, pad[3]; 0x90 records {s16 class, s16 m-class, u32 a run-time
                            pointer (0 on the disc), s32 pvar, f32 range, then a shape body}
    +0x60..+0x6C  cuboids, spheres, cylinders, pills: count, pad[3]; 0x80-byte shape bodies
                            {f32 matrix[4][4] (rows: the local axes' images, then the centre;
                            W 0, 0, 0, 0.01), f32 inverse[3][4] (the 3 x 3 inverse, W 0),
                            f32 euler[3] (R = Rz * Ry * Rx), u32 0}
    +0x70  paths            count, data offset, data size, pad; count s32 offsets into the data;
                            data at a multiple of 16: per path {s32 count, pad[3]} and count
                            {f32 x, y, z, w} points
    +0x74  grind paths      the same, with count 0x20 records {f32 bounding sphere[4], s32 flag,
                            pad[3]} before the offsets
    +0x48  moby groups, +0x4C..+0x5C the pvars (gameplay.py)

Pvars are the per-instance class variables: one block per moby, camera or sound that has one
(pvar index), each a multiple of 16 bytes. Three lists make words of a block special: the moby
links (+0x50, the word is a moby's place in the table, which the loader makes a run-time moby
index), the pointer fixups (+0x5C, an offset in the block the loader makes an address) and the
shared-data records (+0x4C, the field gets an address in the shared blob). The layouts the
decompilation knows (games/rac1/include/moby_pvars.h, `struct update<class>`, adapted there from
Wrench, GPL-3.0-or-later; the camera header from port/assets/world/cameras.h, after ReRAC, ISC)
give fields names and types; the rest of a block is shown as its 32-bit words.

The formats follow the port's readers (port/assets/world/gameplay.h, cameras.h and
port/assets/disc/volumes.h, adapted from ReRAC, https://github.com/re-rac/rerac, ISC License,
Copyright (c) 2026 ReRAC contributors) and Wrench's documentation of the RAC1 gameplay file;
the record layouts were measured on all 19 PAL levels (editor/README.md, "Gameplay data").

The scene side (godot.py calls `moby_metadata` and `write_scene`; pack.py calls `pack_level`):

    Game/LevelSettings            metadata: the settings; child Ship, the ship's place
    Game/MobyGroups               metadata rc1_group_<n>: each group's members, by node name
    Game/Cameras/Camera_NNNN      gameplay/camera.tscn: position and rotation; class, pvars
    Game/Sounds/Sound_NNNN        gameplay/sound.tscn: the shape body's matrix; class, range, pvars
    Game/Volumes/Cuboids/Cuboid_NNNN (Spheres, Cylinders, Pills): gameplay/<kind>.tscn, the unit
                                  shape through the record's matrix, translucent
    Game/Paths/Path_NNNN          Path3D: the points in its Curve3D, the fourth floats as rc1_w
    Game/GrindPaths/GrindPath_NNNN Path3D: as paths, with rc1_bounding_sphere and rc1_flag

and on each moby, camera and sound with a pvar block: rc1_pvar (the block's 32-bit words: a
number with a decimal point or an exponent is a float, any other an integer), rc1_pv_<field>
(a typed field of a known layout) and rc1_link_<offset> (the moby a link names, by node name).

Packing compares what the scene has with what the extractor wrote, as 32-bit floats: what was
not edited keeps its bytes, and a scene nobody edited packs to nothing. A missing node or key is
"not edited". Standard library only.
"""

import json
import math
import re
import struct
from pathlib import Path

import gameplay as gp
from formats import FormatError
from mobys import DECOMPILATION, matrix as euler_matrix, moby_class_names, moby_instances, node_name
from util import f32

SETTINGS, CAMERAS, SOUNDS = 0x00, 0x08, 0x0C
PATHS, GRIND_PATHS = 0x70, 0x74
# The four shape sections: kind, header word, node group, node prefix, mesh.
SHAPES = (("cuboid", 0x60, "Cuboids", "Cuboid"), ("sphere", 0x64, "Spheres", "Sphere"),
          ("cylinder", 0x68, "Cylinders", "Cylinder"), ("pill", 0x6C, "Pills", "Pill"))
SHAPE_SIZE, CAMERA_SIZE, SOUND_SIZE, GRIND_SIZE = 0x80, 0x20, 0x90, 0x20
SETTINGS_SIZE = 0x50
CAMERA = struct.Struct("<i3f3fi")
SOUND_HEAD = struct.Struct("<hhIif")
SETTINGS_RECORD = struct.Struct("<3i3i4ff3ff3i2i")
SETTINGS_FIELDS = ("background", "fog_colour", "fog_near", "fog_far", "fog_near_intensity", "fog_far_intensity",
                   "death_height", "ship_path", "ship_camera_cuboids", "pad")
# The usual W column of a shape's matrix: what a new volume gets.
SHAPE_W = (0.0, 0.0, 0.0, struct.unpack("<f", struct.pack("<f", 0.01))[0])
PATH_W = -1.0       # the fourth float of nearly every path point on the disc
# The nodes this module writes under Game (rc1/check.gd leaves them out of the level geometry).
GROUPS = ("LevelSettings", "MobyGroups", "Cameras", "Sounds", "Volumes", "Paths", "GrindPaths")
VOLUME_ALPHA = 0.25
COLOURS = {"cuboid": (0.2, 0.8, 1.0), "sphere": (1.0, 0.75, 0.2), "cylinder": (0.4, 1.0, 0.4),
           "pill": (1.0, 0.4, 0.9), "camera": (1.0, 1.0, 0.3), "sound": (0.6, 0.5, 1.0), "ship": (1.0, 0.3, 0.2)}

# A scalar field: its struct format and its range.
SCALARS = {"f32": "f", "s32": "i", "u32": "I", "s16": "h", "u16": "H", "s8": "b", "u8": "B"}
# Every camera block starts with this header (port/assets/world/cameras.h, after ReRAC).
CAMERA_HEADER = (("w00", 0x00, "f32", 0), ("sphere", 0x08, "s32", 0), ("cuboid", 0x0C, "s32", 0),
                 ("cylinder", 0x10, "s32", 0), ("path", 0x14, "s32", 0), ("f18", 0x18, "f32", 0),
                 ("priority", 0x1C, "u8", 0), ("blend", 0x1D, "u8", 0), ("b1e", 0x1E, "u8", 0),
                 ("activation", 0x1F, "u8", 0))


def owns(key: str) -> bool:
    """A moby node's metadata key (without rc1_) that this module packs, not pack.py."""
    return key == "pvar" or key.startswith(("pv_", "link_"))


# --- bytes -----------------------------------------------------------------------------------------------------

def start(g: bytes, pointer: int) -> int:
    return gp.offset(g, pointer) if len(g) >= 0x94 else 0


def table(g: bytes, pointer: int, size: int) -> list[bytes]:
    """A `count, pad[3]` section's records; none when it is absent."""
    at = start(g, pointer)
    if at <= 0:
        return []
    count, = struct.unpack_from("<i", g, at)
    if count < 0 or at + 0x10 + count * size > len(g):
        raise FormatError(f"section +{pointer:#x}: {count} records of {size:#x} bytes do not fit")
    return [bytes(g[at + 0x10 + i * size:at + 0x10 + (i + 1) * size]) for i in range(count)]


def write_table(g: bytes, pointer: int, records: list[bytes]) -> bytes:
    at = start(g, pointer)
    return struct.pack("<i", len(records)) + bytes(g[at + 4:at + 0x10]) + b"".join(records)


def splines(data: bytes, offsets: list[int]) -> list[list[tuple]]:
    out = []
    for i, at in enumerate(offsets):
        if at < 0 or at + 0x10 > len(data):
            raise FormatError(f"path {i}: offset {at:#x} outside its data")
        count, = struct.unpack_from("<i", data, at)
        if count < 0 or at + 0x10 + 16 * count > len(data):
            raise FormatError(f"path {i}: {count} points do not fit")
        out.append([struct.unpack_from("<4f", data, at + 0x10 + 16 * k) for k in range(count)])
    return out


def write_splines(paths: list[list[tuple]]) -> tuple[list[int], bytes]:
    offsets, data = [], bytearray()
    for points in paths:
        offsets.append(len(data))
        data += struct.pack("<i12x", len(points)) + b"".join(struct.pack("<4f", *p) for p in points)
    return offsets, bytes(data)


def read_paths(g: bytes) -> list[list[tuple]]:
    """Each path's points (x, y, z, w)."""
    at = start(g, PATHS)
    if at <= 0:
        return []
    count, data_at, size, _ = struct.unpack_from("<4i", g, at)
    offsets = list(struct.unpack_from(f"<{count}i", g, at + 0x10))
    return splines(bytes(g[at + data_at:at + data_at + size]), offsets)


def write_paths(g: bytes, paths: list[list[tuple]]) -> bytes:
    offsets, data = write_splines(paths)
    head = 0x10 + 4 * len(paths)
    head += -head % 16
    at = start(g, PATHS)
    pad = bytes(g[at + 12:at + 16]) if at > 0 else bytes(4)
    out = struct.pack("<3i", len(paths), head, len(data)) + pad + struct.pack(f"<{len(offsets)}i", *offsets)
    return out + bytes(head - len(out)) + data


def read_grind_paths(g: bytes) -> list[dict]:
    """Each grind path: its record (bounding sphere, the four words after it) and its points."""
    at = start(g, GRIND_PATHS)
    if at <= 0:
        return []
    count, data_at, size, _ = struct.unpack_from("<4i", g, at)
    records = [struct.unpack_from("<4f4i", g, at + 0x10 + GRIND_SIZE * i) for i in range(count)]
    offsets = list(struct.unpack_from(f"<{count}i", g, at + 0x10 + GRIND_SIZE * count))
    points = splines(bytes(g[at + data_at:at + data_at + size]), offsets)
    return [{"sphere": r[:4], "words": r[4:], "points": p} for r, p in zip(records, points)]


def write_grind_paths(g: bytes, paths: list[dict]) -> bytes:
    offsets, data = write_splines([p["points"] for p in paths])
    head = 0x10 + (GRIND_SIZE + 4) * len(paths)
    head += -head % 16
    at = start(g, GRIND_PATHS)
    pad = bytes(g[at + 12:at + 16]) if at > 0 else bytes(4)
    out = struct.pack("<3i", len(paths), head, len(data)) + pad
    out += b"".join(struct.pack("<4f4i", *p["sphere"], *p["words"]) for p in paths)
    out += struct.pack(f"<{len(offsets)}i", *offsets)
    return out + bytes(head - len(out)) + data


def read_settings(g: bytes) -> dict | None:
    at = start(g, SETTINGS)
    if at <= 0 or at + SETTINGS_SIZE > len(g):
        return None
    v = SETTINGS_RECORD.unpack_from(g, at)
    return {"background": list(v[0:3]), "fog_colour": list(v[3:6]), "fog_near": v[6], "fog_far": v[7],
            "fog_near_intensity": v[8], "fog_far_intensity": v[9], "death_height": v[10],
            "ship_position": list(v[11:14]), "ship_yaw": v[14], "ship_path": v[15],
            "ship_camera_cuboids": list(v[16:18]), "pad": list(v[18:20])}


def write_settings(s: dict) -> bytes:
    return SETTINGS_RECORD.pack(*s["background"], *s["fog_colour"], s["fog_near"], s["fog_far"],
                                s["fog_near_intensity"], s["fog_far_intensity"], s["death_height"],
                                *s["ship_position"], s["ship_yaw"], s["ship_path"], *s["ship_camera_cuboids"],
                                *s["pad"])


# --- shapes ----------------------------------------------------------------------------------------------------

def shape_matrix(body: bytes) -> list[float]:
    """A shape body's matrix as a column-major 4 x 4 for the scene: the local axes' images, then
    the centre (the record's rows are the scene's columns)."""
    m = struct.unpack_from("<16f", body, 0)
    return [*m[0:3], 0.0, *m[4:7], 0.0, *m[8:11], 0.0, *m[12:15], 1.0]


def inverse3(cols: list[list[float]]) -> list[list[float]]:
    """The inverse of the 3 x 3 whose rows are COLS (row-vector form: world = l * M + centre)."""
    (a, b, c), (d, e, f), (g, h, i) = cols
    det = a * (e * i - f * h) - b * (d * i - f * g) + c * (d * h - e * g)
    if not math.isfinite(det) or abs(det) < 1e-12:
        raise FormatError("a volume needs a size on all three axes")
    return [[(e * i - f * h) / det, (c * h - b * i) / det, (b * f - c * e) / det],
            [(f * g - d * i) / det, (a * i - c * g) / det, (c * d - a * f) / det],
            [(d * h - e * g) / det, (b * g - a * h) / det, (a * e - b * d) / det]]


def euler_of(cols: list[list[float]]) -> tuple[float, float, float]:
    """The Euler angles (R = Rz * Ry * Rx) of the rotation in the axes COLS (scale removed)."""
    unit = []
    for col in cols:
        length = math.sqrt(sum(v * v for v in col))
        unit.append([v / length for v in col])
    r = [[unit[j][i] for j in range(3)] for i in range(3)]      # r[i][j]: column j is axis j
    sy = max(-1.0, min(1.0, -r[2][0]))
    ry = math.asin(sy)
    if abs(sy) < 0.999999:
        return math.atan2(r[2][1], r[2][2]) + 0.0, ry + 0.0, math.atan2(r[1][0], r[0][0]) + 0.0
    return math.atan2(-r[1][2], r[1][1]) + 0.0, ry + 0.0, 0.0  # straight up or down: x and z share an axis


def shape_body(t: tuple, base: bytes | None) -> bytes:
    """A shape body from a scene transform (basis rows, then the origin): the matrix, its
    inverse and Euler angles computed again; the W words and the last word from BASE (the body it
    replaces), or the usual ones."""
    cols = [[t[0 + j], t[3 + j], t[6 + j]] for j in range(3)]
    values = [*cols[0], *cols[1], *cols[2], *t[9:12]]
    if not all(math.isfinite(v) for v in values):
        raise FormatError("a transform that is not a number")
    old = struct.unpack_from("<32f", base, 0) if base else None
    w = (old[3], old[7], old[11], old[15]) if old else SHAPE_W
    inv = inverse3(cols)
    inv_w = (old[19], old[23], old[27]) if old else (0.0, 0.0, 0.0)
    tail = base[0x7C:0x80] if base else bytes(4)
    out = struct.pack("<16f", *cols[0], w[0], *cols[1], w[1], *cols[2], w[2], *t[9:12], w[3])
    out += struct.pack("<12f", *inv[0], inv_w[0], *inv[1], inv_w[1], *inv[2], inv_w[2])
    return out + struct.pack("<3f", *euler_of(cols)) + tail


# --- pvars -----------------------------------------------------------------------------------------------------

class Pvars:
    """A gameplay file's pvar blocks and the lists that point into them."""

    def __init__(self, g: bytes):
        self.g = g
        self.links, self.pointers = gp.fixups(g, gp.LINKS), gp.fixups(g, gp.POINTERS)
        _, self.shared = gp.shared(g)
        owners = [m[0x58 // 4] for m in (struct.unpack("<30i", r) for r in gp.records(g))]
        owners += [CAMERA.unpack(r)[7] for r in table(g, CAMERAS, CAMERA_SIZE)]
        owners += [SOUND_HEAD.unpack_from(r)[3] for r in table(g, SOUNDS, SOUND_SIZE)]
        self.table = gp.pvar_table(g, gp.pvar_count(g, owners + [p for p, _ in self.links + self.pointers]))
        self.data_at = gp.offset(g, gp.DATA)

    def block(self, index: int) -> bytes | None:
        if not 0 <= index < len(self.table) or self.data_at <= 0:
            return None
        at, size = self.table[index]
        if at < 0 or size <= 0 or self.data_at + at + size > len(self.g):
            return None
        return bytes(self.g[self.data_at + at:self.data_at + at + size])

    def link_offsets(self, index: int) -> list[int]:
        return sorted({at for pvar, at in self.links if pvar == index})

    def locked(self, index: int) -> set[int]:
        """The words of block INDEX the loader rewrites (pointers and shared-data addresses)."""
        return {at for pvar, at in self.pointers if pvar == index} | {at for pvar, at, _ in self.shared if pvar == index}


_layouts = None


def class_layouts() -> dict[int, list[tuple]]:
    """class -> the typed fields of its pvars (name, offset, scalar type, array length or 0), from
    the decompilation's moby_pvars.h: `struct update<class>`. A struct whose offsets as parsed differ
    from those its comments give is left out (its words are still shown). Empty without
    games/rac1 in the checkout."""
    global _layouts
    if _layouts is not None:
        return _layouts
    _layouts = {}
    header = DECOMPILATION / "include" / "moby_pvars.h"
    if not header.is_file():
        return _layouts
    from cparse import Types
    types = Types()
    for name in ("common.h", "structs.h"):
        if (DECOMPILATION / "include" / name).is_file():
            types.read((DECOMPILATION / "include" / name).read_text(errors="replace"))
    # cparse reads `short int` as two names; the header writes it so.
    text = re.sub(r"\b(short|long)\s+int\b", r"\1", header.read_text(errors="replace"))
    types.read(text)
    for class_id, body in re.findall(r"^struct update(\d+) \{[^\n]*\n(.*?)^\};", text, re.M | re.S):
        s = types.structs.get(f"update{class_id}")
        if s is None or not s.exact:
            continue
        stated = {name: int(at, 16) for at, name in re.findall(r"/\* 0x([0-9a-fA-F]+) \*/[^;]*?\b(\w+)\s*(?:\[[^\]]*\])?;", body)}
        if any(stated.get(f.name, f.offset) != f.offset for f in s.fields):
            continue
        _layouts[int(class_id)] = flatten(types, s, "", 0, 0)
    return _layouts


def flatten(types, s, prefix: str, base: int, depth: int) -> list[tuple]:
    """The named scalar fields of struct S (and of the structs in it), as (name, offset, type, n)."""
    out = []
    for f in s.fields:
        if re.match(r"(unknown|pad|unk)", f.name) or f.ctype.ptr:
            continue
        name, t = f"{prefix}{f.name}", f.ctype
        if t.base in SCALARS and len(t.dims) <= 1 and (not t.dims or t.dims[0] <= 16):
            out.append((name, base + f.offset, t.base, t.dims[0] if t.dims else 0))
        elif t.base in types.structs and depth < 3:
            inner = types.structs[t.base]
            if not inner.exact:
                continue
            count = t.dims[0] if len(t.dims) == 1 and t.dims[0] <= 8 else (1 if not t.dims else 0)
            for k in range(count):
                tag = f"{name}{k}_" if t.dims else f"{name}_"
                out += flatten(types, inner, tag, base + f.offset + k * inner.size, depth + 1)
    return out


def word_value(word: bytes):
    """A 32-bit word as the scene shows it: a float when it reads as an ordinary one, else an
    integer (signed)."""
    i, = struct.unpack("<i", word)
    x, = struct.unpack("<f", word)
    exponent = (i >> 23) & 0xFF
    if i and 0 < exponent < 255 and 1e-6 <= abs(x) <= 1e9:
        return x
    return i


def read_field(block: bytes, at: int, kind: str, count: int):
    fmt = SCALARS[kind]
    size = struct.calcsize(fmt)
    values = [struct.unpack_from("<" + fmt, block, at + k * size)[0] for k in range(max(count, 1))]
    return values if count else values[0]


def field_size(kind: str, count: int) -> int:
    return struct.calcsize(SCALARS[kind]) * max(count, 1)


def pvar_meta(pvars: Pvars, index: int, layout, names: list[str]) -> dict:
    """The metadata (keys without rc1_) of pvar block INDEX: its words, the LAYOUT's fields that
    no fixup touches, and its moby links by node name (NAMES: the mobys' node names)."""
    block = pvars.block(index)
    if block is None:
        return {}
    meta = {"pvar": [word_value(block[at:at + 4]) for at in range(0, len(block) - 3, 4)]}
    links = pvars.link_offsets(index)
    special = set(links) | pvars.locked(index)
    for name, at, kind, count in layout or ():
        size = field_size(kind, count)
        if at + size > len(block) or any(at <= w + 3 and w < at + size for w in special):
            continue
        value = read_field(block, at, kind, count)
        if kind == "f32" and not all(math.isfinite(v) for v in (value if count else [value])):
            continue
        meta[f"pv_{name}"] = value
    for at in links:
        if at + 4 <= len(block):
            target, = struct.unpack_from("<i", block, at)
            meta[f"link_{at:x}"] = names[target] if 0 <= target < len(names) else str(target)
    return meta


def same_value(a, b) -> bool:
    """Equal as the scene's text stands for them: floats as 32-bit floats, lists item by item, an
    int and a float never."""
    from pack import near
    if isinstance(a, list) or isinstance(b, list):
        return isinstance(a, list) and isinstance(b, list) and len(a) == len(b) and all(map(same_value, a, b))
    if isinstance(a, bool) or isinstance(b, bool):
        return a == b
    if isinstance(a, float) and isinstance(b, float):
        return near(a, b) if math.isfinite(a) and math.isfinite(b) else struct.pack("<f", a) == struct.pack("<f", b)
    if isinstance(a, (int, float)) and isinstance(b, (int, float)):
        return type(a) is type(b) and a == b
    return a == b


def same_typed(a, b, kind: str) -> bool:
    """Equal as values of a field of KIND: a float field compares as 32-bit floats whatever the
    scene's text, an integer field as integers."""
    from pack import near
    if isinstance(a, list) or isinstance(b, list):
        return isinstance(a, list) and isinstance(b, list) and len(a) == len(b) and all(same_typed(x, y, kind) for x, y in zip(a, b))
    if not isinstance(a, (int, float)) or isinstance(a, bool):
        return False
    return near(float(a), float(b)) if kind == "f32" else a == b


def pack_word(value) -> bytes:
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise FormatError(f"{value!r} is not a number")
    if isinstance(value, float):
        if not math.isfinite(value):
            raise FormatError("not a finite number")
        return struct.pack("<f", value)
    if not -(1 << 31) <= value < (1 << 32):
        raise FormatError(f"{value} does not fit 32 bits")
    return struct.pack("<I", value & 0xFFFFFFFF)


def pack_field(value, kind: str, count: int) -> bytes:
    values = value if count else [value]
    if not isinstance(values, list) or len(values) != max(count, 1):
        raise FormatError(f"needs {max(count, 1)} values")
    out = b""
    for v in values:
        if isinstance(v, bool) or not isinstance(v, (int, float)):
            raise FormatError(f"{v!r} is not a number")
        if kind == "f32":
            if not math.isfinite(v):
                raise FormatError("not a finite number")
            out += struct.pack("<f", v)
            continue
        if v != int(v):
            raise FormatError(f"{v} is not an integer")
        try:
            out += struct.pack("<" + SCALARS[kind], int(v))
        except struct.error:
            raise FormatError(f"{int(v)} does not fit a {kind}") from None
    return out


def edit_block(name: str, block: bytearray, base: dict, meta: dict, layout, links: list[int], locked: set[int],
               numbering: dict[str, int], problems: list[str]) -> bool:
    """Writes into BLOCK what META (a node's keys) changed from BASE (what the extractor wrote for
    it); True when something was written. Words first, then typed fields, then links."""
    edited, special = False, set(links) | locked
    if "pvar" in meta and "pvar" in base and not same_value(meta["pvar"], base["pvar"]):
        words = meta["pvar"]
        if not isinstance(words, list) or len(words) != len(base["pvar"]):
            problems.append(f"{name}: rc1_pvar needs {len(base['pvar'])} words (the block's size); not packed")
        else:
            for k, (now, was) in enumerate(zip(words, base["pvar"])):
                if same_value(now, was):
                    continue
                if 4 * k in special:
                    problems.append(f"{name}: rc1_pvar word {k} (+{4 * k:#x}) is one the level loader rewrites "
                                    f"({'a moby link: edit rc1_link_' + format(4 * k, 'x') if 4 * k in links else 'a pointer'}); not packed")
                    continue
                try:
                    block[4 * k:4 * k + 4] = pack_word(now)
                    edited = True
                except FormatError as e:
                    problems.append(f"{name}: rc1_pvar word {k}: {e}; not packed")
    fields = {f"pv_{n}": (at, kind, count) for n, at, kind, count in layout or ()}
    for key in sorted(k for k in meta if k.startswith("pv_")):
        if key not in fields or key not in base:
            problems.append(f"{name}: no such value: rc1_{key}")
            continue
        at, kind, count = fields[key]
        if same_typed(meta[key], base[key], kind):
            continue
        try:
            block[at:at + field_size(kind, count)] = pack_field(meta[key], kind, count)
            edited = True
        except FormatError as e:
            problems.append(f"{name}: rc1_{key}: {e}; not packed")
    for key in sorted(k for k in meta if k.startswith("link_")):
        if key not in base:
            problems.append(f"{name}: rc1_{key} names no moby link of its pvars; not packed")
            continue
        now = meta[key]
        if now == base[key]:
            continue
        at = int(key[len("link_"):], 16)
        if not isinstance(now, str):
            problems.append(f"{name}: rc1_{key} must name a moby node; not packed")
        elif re.fullmatch(r"-\d+", now.strip()):
            block[at:at + 4] = struct.pack("<i", int(now))
            edited = True
        elif now in numbering:
            block[at:at + 4] = struct.pack("<i", numbering[now])
            edited = True
        else:
            problems.append(f"{name}: rc1_{key} names {now!r}, which is no moby of the level (-1 for none); not packed")
    return edited


# --- the scene: writing ----------------------------------------------------------------------------------------

def text(v) -> str:
    """A value in Godot's text syntax; a float always reads back as a float."""
    from godot import number
    if isinstance(v, bool):
        return "true" if v else "false"
    if isinstance(v, float):
        t = number(v)
        return t if any(c in t for c in ".en") else t + ".0"
    if isinstance(v, list):
        return f"[{', '.join(text(x) for x in v)}]"
    if isinstance(v, str):
        return json.dumps(v)
    return str(v)


def raw(meta: dict) -> dict:
    """Metadata as Scene.node keyword arguments, each value already Godot text."""
    from godot import Raw
    return {f"metadata__rc1_{k}": Raw(text(v)) for k, v in meta.items()}


def moby_names(g: bytes) -> list[str]:
    names = moby_class_names()
    return [node_name(m["class_id"], m["index"], names) for m in moby_instances(g)]


def moby_metadata(g: bytes) -> dict[int, dict]:
    """moby index -> the Scene.node keyword arguments of its pvars (godot.py's write_mobys)."""
    pvars, names, layouts = Pvars(g), moby_names(g), class_layouts()
    out = {}
    for m in moby_instances(g):
        meta = pvar_meta(pvars, m["fields"]["pvar_index"], layouts.get(m["class_id"]), names)
        if meta:
            out[m["index"]] = raw(meta)
    return out


def write_class_scene(path: Path, kind: str) -> None:
    """A small scene for one kind of gameplay object: a translucent unit shape (or a marker box)."""
    from godot import Raw, Scene, number
    scene = Scene()
    r, g, b = COLOURS[kind]
    material = scene.subresource("StandardMaterial3D", "material", transparency=1, shading_mode=0, cull_mode=2,
                                 albedo_color=Raw(f"Color({number(r)}, {number(g)}, {number(b)}, {number(VOLUME_ALPHA)})"))
    # The game's unit shapes: |x|, |y|, |z| <= 1; |l| < 1; the cylinder and the pill along local z.
    along_z = Raw("Transform3D(1, 0, 0, 0, 0, -1, 0, 1, 0, 0, 0, 0)")
    meshes = {"cuboid": ("BoxMesh", {"size": Raw("Vector3(2, 2, 2)")}, None),
              "sphere": ("SphereMesh", {"radius": 1.0, "height": 2.0}, None),
              "cylinder": ("CylinderMesh", {"top_radius": 1.0, "bottom_radius": 1.0, "height": 2.0}, along_z),
              "pill": ("CapsuleMesh", {"radius": 1.0, "height": 4.0}, along_z),
              "camera": ("BoxMesh", {"size": Raw("Vector3(0.6, 0.6, 1)")}, None),
              "sound": ("SphereMesh", {"radius": 1.0, "height": 2.0}, None),
              "ship": ("BoxMesh", {"size": Raw("Vector3(3, 1.5, 0.8)")}, None)}
    mesh_type, props, placed = meshes[kind]
    mesh = scene.subresource(mesh_type, "mesh", material=material, **props)
    scene.node(kind.capitalize(), kind="Node3D")
    scene.node("Shape", ".", "MeshInstance3D", mesh=mesh, **({"transform": placed} if placed else {}))
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(scene.text())


def curve(scene, rid: str, points: list[tuple]):
    """A Curve3D of POINTS (no handles: the game's paths are polylines)."""
    from godot import Raw, number
    flat = ", ".join(f"0, 0, 0, 0, 0, 0, {number(x)}, {number(y)}, {number(z)}" for x, y, z, _ in points)
    tilts = ", ".join("0" for _ in points)
    return scene.subresource("Curve3D", rid, _data=Raw(f'{{\n"points": PackedVector3Array({flat}),\n'
                                                       f'"tilts": PackedFloat32Array({tilts})\n}}'),
                             point_count=len(points))


def write_scene(scene, directory: Path, res: str, g: bytes) -> dict:
    """The gameplay nodes of a level scene (under Game, after the mobys) and their class scenes in
    DIRECTORY/gameplay; returns what was written, for level.json."""
    from godot import Raw, transform
    pvars, names = Pvars(g), moby_names(g)
    scenes = {}
    for kind in ("cuboid", "sphere", "cylinder", "pill", "camera", "sound", "ship"):
        write_class_scene(directory / "gameplay" / f"{kind}.tscn", kind)
        scenes[kind] = scene.resource("PackedScene", f"{res}/gameplay/{kind}.tscn", f"gameplay_{kind}")
    stats = {}

    settings = read_settings(g)
    if settings is not None:
        meta = {k: settings[k] for k in SETTINGS_FIELDS if k != "pad" or any(settings["pad"])}
        scene.node("LevelSettings", "Game", "Node3D", **raw(meta))
        ship = euler_matrix(settings["ship_position"], (0.0, 0.0, settings["ship_yaw"]), 1.0)
        scene.node("Ship", "Game/LevelSettings", instance=scenes["ship"], transform=transform(ship))

    groups = gp.groups(g) if start(g, gp.GROUPS) > 0 else []
    scene.node("MobyGroups", "Game", "Node", **raw({f"group_{i}": [names[m] if m < len(names) else str(m) for m in members]
                                                     for i, members in enumerate(groups) if members}))
    stats["groups"] = len(groups)

    scene.node("Cameras", "Game", "Node3D")
    cameras = table(g, CAMERAS, CAMERA_SIZE)
    for i, record in enumerate(cameras):
        class_id, *place, pvar = CAMERA.unpack(record)
        meta = {"index": i, "class": class_id, "pvar_index": pvar, **pvar_meta(pvars, pvar, CAMERA_HEADER, names)}
        scene.node(f"Camera_{i:04}", "Game/Cameras", instance=scenes["camera"], rotation_order=0,
                   transform=transform(euler_matrix(place[:3], place[3:], 1.0)), **raw(meta))
    stats["cameras"] = len(cameras)

    scene.node("Sounds", "Game", "Node3D", visible=False)
    sounds = table(g, SOUNDS, SOUND_SIZE)
    for i, record in enumerate(sounds):
        class_id, m_class, function, pvar, reach = SOUND_HEAD.unpack_from(record)
        meta = {"index": i, "class": class_id, "m_class": m_class, "range": reach, "pvar_index": pvar}
        meta.update({"function": function} if function else {})
        meta.update(pvar_meta(pvars, pvar, None, names))
        scene.node(f"Sound_{i:04}", "Game/Sounds", instance=scenes["sound"],
                   transform=transform(shape_matrix(record[0x10:])), **raw(meta))
    stats["sounds"] = len(sounds)

    scene.node("Volumes", "Game", "Node3D", visible=False)
    for kind, pointer, group, prefix in SHAPES:
        scene.node(group, "Game/Volumes", "Node3D")
        bodies = table(g, pointer, SHAPE_SIZE)
        for i, body in enumerate(bodies):
            scene.node(f"{prefix}_{i:04}", f"Game/Volumes/{group}", instance=scenes[kind],
                       transform=transform(shape_matrix(body)), **raw({"index": i}))
        stats[f"{kind}s"] = len(bodies)

    scene.node("Paths", "Game", "Node3D")
    paths = read_paths(g)
    for i, points in enumerate(paths):
        scene.node(f"Path_{i:04}", "Game/Paths", "Path3D", curve=curve(scene, f"path_{i:04}", points),
                   **raw({"index": i, "w": [float(p[3]) for p in points]}))
    stats["paths"] = len(paths)

    scene.node("GrindPaths", "Game", "Node3D")
    grinds = read_grind_paths(g)
    for i, path in enumerate(grinds):
        meta = {"index": i, "w": [float(p[3]) for p in path["points"]], "bounding_sphere": [float(v) for v in path["sphere"]],
                "flag": path["words"][0]}
        meta.update({"pad": list(path["words"][1:])} if any(path["words"][1:]) else {})
        scene.node(f"GrindPath_{i:04}", "Game/GrindPaths", "Path3D", curve=curve(scene, f"grind_{i:04}", path["points"]),
                   **raw(meta))
    stats["grind_paths"] = len(grinds)
    layouts = class_layouts()
    stats["pvars"] = len(pvars.table)
    stats["typed_pvar_classes"] = sorted(c for c in layouts if any(m["class_id"] == c for m in moby_instances(g)))
    return stats


# --- the scene: reading ----------------------------------------------------------------------------------------

class Call:
    """A constructor in a scene's text: Transform3D(...), SubResource("id"), PackedVector3Array(...)."""

    def __init__(self, name: str, args: list):
        self.name, self.args = name, args

    def __repr__(self):
        return f"{self.name}({self.args})"


VALUE_TOKEN = re.compile(r"""\s*(?:(?P<str>&?"(?:\\.|[^"\\])*")
    |(?P<num>[-+]?(?:inf|nan)(?![\w])|[-+]?(?:\d+\.?\d*|\.\d+)(?:[eE][-+]?\d+)?)
    |(?P<id>[A-Za-z_]\w*)|(?P<p>[\[\]{}(),:]))""", re.X)


def parse_value(source: str):
    """A value in Godot's text syntax: numbers (int or float by their text), strings, bools,
    null, arrays, dictionaries and constructors (Call)."""
    tokens, at = [], 0
    while at < len(source):
        if source[at:].strip() == "":
            break
        m = VALUE_TOKEN.match(source, at)
        if not m:
            raise FormatError(f"cannot read the value {source[:60]!r}")
        tokens.append((m.lastgroup, m.group(m.lastgroup)))
        at = m.end()
    pos = 0

    def take():
        nonlocal pos
        pos += 1
        return tokens[pos - 1]

    def expect(p):
        kind, t = take()
        if t != p:
            raise FormatError(f"expected {p!r} in {source[:60]!r}")

    def items(close):
        out = []
        while tokens[pos][1] != close:
            out.append(value())
            if tokens[pos][1] == ",":
                take()
        take()
        return out

    def value():
        kind, t = take()
        if kind == "str":
            return json.loads(t.lstrip("&"))
        if kind == "num":
            return float(t) if any(c in t for c in ".eEn") or "inf" in t else int(t)
        if kind == "p" and t == "[":
            return items("]")
        if kind == "p" and t == "{":
            out = {}
            while tokens[pos][1] != "}":
                key = value()
                expect(":")
                out[key if not isinstance(key, list) else str(key)] = value()
                if tokens[pos][1] == ",":
                    take()
            take()
            return out
        if kind == "id":
            if t in ("true", "false"):
                return t == "true"
            if t == "null":
                return None
            if pos < len(tokens) and tokens[pos][1] == "[":          # Array[int]([...])
                take()
                while take()[1] != "]":
                    pass
            if pos < len(tokens) and tokens[pos][1] == "(":
                take()
                args = items(")")
                return args[0] if t in ("Array", "Dictionary") and len(args) == 1 else Call(t, args)
            return Call(t, [])
        raise FormatError(f"cannot read the value {source[:60]!r}")

    try:
        result = value()
    except IndexError:
        raise FormatError(f"cannot read the value {source[:60]!r}") from None
    return result


HEADER = re.compile(r'(\w+)=("(?:\\.|[^"\\])*"|\w+\("(?:\\.|[^"\\])*"\)|[^\s\]]+)')
PROPERTY = re.compile(r"^([A-Za-z_][\w/]*) = (.*)$")


def depth(line: str) -> int:
    """How many brackets LINE opens and does not close, outside strings."""
    level, quoted, escaped = 0, False, False
    for c in line:
        if quoted:
            escaped = not escaped and c == "\\"
            quoted = escaped or c != '"'
            continue
        if c == '"':
            quoted = True
        elif c in "([{":
            level += 1
        elif c in ")]}":
            level -= 1
    return level


def parse_scene(source: str) -> tuple[dict, dict]:
    """A .tscn's nodes, by path ("Game/Cameras/Camera_0000"; the root is "."), each {"name",
    "parent", "type", "instance", "props": {key: value text}}, and its sub_resources by id."""
    nodes, subresources, current = {}, {}, None
    lines = source.splitlines()
    i = 0
    while i < len(lines):
        line = lines[i]
        i += 1
        if line.startswith("["):
            head = dict(HEADER.findall(line))
            current = None
            kind = line[1:].split(" ", 1)[0].rstrip("]")
            if kind == "node":
                name = json.loads(head.get("name", '""'))
                parent = json.loads(head["parent"]) if "parent" in head else None
                path = "." if parent is None else (name if parent == "." else f"{parent}/{name}")
                current = {"name": name, "parent": parent, "path": path, "props": {},
                           "type": json.loads(head["type"]) if "type" in head else None, "instance": head.get("instance")}
                nodes[path] = current
            elif kind == "sub_resource":
                current = {"type": json.loads(head.get("type", '""')), "props": {}}
                subresources[json.loads(head.get("id", '""'))] = current
            continue
        prop = PROPERTY.match(line)
        if current is None or not prop:
            continue
        key, value = prop.groups()
        level = depth(value)
        while level > 0 and i < len(lines):
            value += "\n" + lines[i]
            level += depth(lines[i])
            i += 1
        current["props"][key] = value
    return nodes, subresources


def meta_of(node: dict) -> dict:
    """A node's rc1_* metadata, keys without the prefix; a value that cannot be read is skipped
    (and named by the caller's checks)."""
    out = {}
    for key, value in node["props"].items():
        if key.startswith("metadata/rc1_"):
            try:
                out[key[len("metadata/rc1_"):]] = parse_value(value)
            except FormatError:
                out[key[len("metadata/rc1_"):]] = Call("unreadable", [value])
    return out


def transform_of(node: dict) -> tuple:
    """A node's transform as the scene lists it (basis rows, then the origin)."""
    from pack import IDENTITY
    if "transform" not in node["props"]:
        return IDENTITY
    t = parse_value(node["props"]["transform"])
    if not isinstance(t, Call) or t.name != "Transform3D" or len(t.args) != 12:
        raise FormatError(f"{node['name']}: cannot read its transform")
    return tuple(float(v) for v in t.args)


def placed(nodes: dict, node: dict) -> tuple:
    """A node's transform in game space: its own after those of its parents below Game (a group
    node that was moved moves what is in it)."""
    t, parent = transform_of(node), node["parent"]
    while parent and parent.startswith("Game/") and parent in nodes:
        t = compose(transform_of(nodes[parent]), t)
        parent = nodes[parent]["parent"]
    return t


def compose(a: tuple, b: tuple) -> tuple:
    """Transform A then B's parent: A * B, both as the scene lists them."""
    ra = [a[0:3], a[3:6], a[6:9]]
    rb = [b[0:3], b[3:6], b[6:9]]
    basis = [sum(ra[r][k] * rb[k][c] for k in range(3)) for r in range(3) for c in range(3)]
    origin = [sum(ra[r][k] * b[9 + k] for k in range(3)) + a[9 + r] for r in range(3)]
    return tuple(basis + origin)


def point_in(t: tuple, p) -> tuple:
    return tuple(t[3 * r] * p[0] + t[3 * r + 1] * p[1] + t[3 * r + 2] * p[2] + t[9 + r] for r in range(3))


def children(nodes: dict, parent: str) -> list[dict]:
    return [n for n in nodes.values() if n["parent"] == parent]


def owners(found: list[dict], count: int, prefix: str) -> tuple[dict[int, dict], list[dict]]:
    """The node that is each record (by rc1_index; the one still named after its place when Godot
    copied it), and the nodes that are none (copies, or class scenes dropped in)."""
    owner, extra = {}, []
    for node in found:
        index = meta_of(node).get("index")
        if not isinstance(index, int) or not 0 <= index < count:
            extra.append(node)
            continue
        named = node["name"] == f"{prefix}_{index:04}"
        if index not in owner or (named and owner[index]["name"] != f"{prefix}_{index:04}"):
            if index in owner:
                extra.append(owner[index])
            owner[index] = node
        else:
            extra.append(node)
    return owner, extra


def curve_points(nodes: dict, node: dict, subresources: dict) -> list[tuple] | None:
    """The points of a Path3D node's Curve3D, in game space (the node's own transform applied)."""
    value = node["props"].get("curve")
    if value is None:
        return None
    ref = parse_value(value)
    if not isinstance(ref, Call) or ref.name != "SubResource" or ref.args[0] not in subresources:
        raise FormatError(f"{node['name']}: its curve is not in the scene")
    props = subresources[ref.args[0]]["props"]
    data = parse_value(props["_data"]) if "_data" in props else {}
    points = data.get("points") if isinstance(data, dict) else None
    flat = [float(v) for v in points.args] if isinstance(points, Call) else []
    if len(flat) % 9:
        raise FormatError(f"{node['name']}: cannot read its curve")
    t = placed(nodes, node)
    return [point_in(t, flat[k + 6:k + 9]) for k in range(0, len(flat), 9)]


# --- the scene: packing ----------------------------------------------------------------------------------------

def pack_level(original: bytes, edited: bytes, source: str, numbering: dict[str, int],
               changes: list[str], problems: list[str]) -> tuple[bytes, list[str], list[str]]:
    """EDITED (the gameplay file pack.py made from ORIGINAL, the disc's) with the scene's edits to
    the gameplay data. NUMBERING: each moby node's name -> its place in EDITED's table. Adds to
    CHANGES and PROBLEMS and returns the three."""
    nodes, subresources = parse_scene(source)
    out = bytearray(edited)
    names = moby_names(original)
    before, after = Pvars(original), Pvars(edited)
    layouts = class_layouts()
    sections: dict[int, bytes] = {}

    def pvar_edits(node: dict, meta: dict, base_index: int, target_index: int, layout) -> bool:
        if not any(owns(k) for k in meta):
            return False
        base = pvar_meta(before, base_index, layout, names)
        block = after.block(target_index)
        if not base or block is None:
            problems.append(f"{node['name']}: has rc1_pvar values but no pvar block; not packed")
            return False
        work = bytearray(block)
        if not edit_block(node["name"], work, base, meta, layout, after.link_offsets(target_index),
                          after.locked(target_index), numbering, problems):
            return False
        at = after.data_at + after.table[target_index][0]
        out[at:at + len(work)] = work
        return True

    # Mobys: pvars and links.
    originals = moby_instances(original)
    now = [struct.unpack("<30i", r) for r in gp.records(edited)]
    for node in children(nodes, "Game/Mobys"):
        meta = meta_of(node)
        if not any(owns(k) for k in meta):
            continue
        index, target = meta.get("index"), numbering.get(node["name"])
        if not isinstance(index, int) or not 0 <= index < len(originals) or target is None or target >= len(now):
            problems.append(f"{node['name']}: pvars of a moby that is not one of the level's; not packed")
            continue
        if pvar_edits(node, meta, originals[index]["fields"]["pvar_index"], now[target][0x58 // 4],
                      layouts.get(originals[index]["class_id"])):
            changes.append(f"{node['name']}: pvars")

    pack_groups(original, out, nodes, numbering, sections, changes, problems)
    pack_cameras(original, out, nodes, pvar_edits, changes, problems)
    pack_sounds(original, out, nodes, pvar_edits, changes, problems)
    for kind, pointer, group, prefix in SHAPES:
        pack_shapes(original, nodes, pointer, group, prefix, sections, changes, problems)
    pack_paths(original, nodes, subresources, sections, changes, problems)
    pack_settings(original, out, nodes, changes, problems)
    if sections:
        return gp.replace(bytes(out), sections), changes, problems
    return bytes(out), changes, problems


def pack_groups(original: bytes, out: bytearray, nodes: dict, numbering: dict[str, int], sections: dict,
                changes: list[str], problems: list[str]) -> None:
    """The moby groups, in a scene that has Game/MobyGroups (an older one's groups stay as pack.py
    leaves them): a name added to or removed from a group's list there sets that moby's group (its
    record's +0x48, rc1_group), and then each group lists the mobys whose record names it, those it
    had in their order first."""
    node = nodes.get("Game/MobyGroups")
    table_at = start(bytes(out), gp.MOBYS)
    if node is None or start(bytes(out), gp.GROUPS) <= 0 or table_at <= 0:
        return
    lists = gp.groups(bytes(out))
    count, = struct.unpack_from("<i", out, table_at)
    group_at = [table_at + 0x10 + i * gp.SIZE + 0x48 for i in range(count)]
    groups = [struct.unpack_from("<i", out, at)[0] for at in group_at]
    names, meta, moved = moby_names(original), meta_of(node), {}
    for i, members in enumerate(gp.groups(original)):
        key, base = f"group_{i}", [names[m] for m in members or []]
        listed = meta.get(key, [])
        if not isinstance(listed, list) or not all(isinstance(n, str) for n in listed):
            problems.append(f"MobyGroups: rc1_{key} must list moby node names; not packed")
            continue
        if listed == base:
            continue
        for name in listed:
            if name in numbering:
                moved[numbering[name]] = i
            else:
                problems.append(f"MobyGroups: rc1_{key} names {name!r}, which is no moby of the level; not packed")
        for name in base:
            if name not in listed and name in numbering and numbering[name] not in moved:
                moved[numbering[name]] = -1
    for key in meta:
        if not re.fullmatch(r"group_\d+", key) or int(key[6:]) >= len(lists):
            problems.append(f"MobyGroups: no such group: rc1_{key}")
    for m, group in sorted(moved.items()):
        if groups[m] != group:
            groups[m] = group
            struct.pack_into("<i", out, group_at[m], group)
            changes.append(f"moby {m}: group {group}" + (" (from Game/MobyGroups)" if group >= 0 else ", none"))
    wanted: list[list[int] | None] = []
    for i, members in enumerate(lists):
        kept = [m for m in members or [] if m < count and groups[m] == i]
        wanted.append((kept + [m for m in range(count) if groups[m] == i and m not in kept]) or None)
    for m in range(count):
        if groups[m] >= len(lists):
            problems.append(f"moby {m}: group {groups[m]} does not exist (the level has {len(lists)}); in none")
    if wanted != [g or None for g in lists]:
        sections[gp.GROUPS] = gp.write_groups(wanted)
        changes.append("moby groups: the lists follow the mobys' groups")


def pack_cameras(original: bytes, out: bytearray, nodes: dict, pvar_edits, changes: list[str], problems: list[str]) -> None:
    from pack import placement, same, scene_transform
    found = children(nodes, "Game/Cameras")
    if "Game/Cameras" not in nodes:
        return
    cameras = table(original, CAMERAS, CAMERA_SIZE)
    owner, extra = owners(found, len(cameras), "Camera")
    for node in extra:
        problems.append(f"{node['name']}: adding cameras is not supported (other data names them by place); not packed")
    for i in sorted(set(range(len(cameras))) - set(owner)):
        problems.append(f"Camera_{i:04}: deleting cameras is not supported (other data names them by place); kept")
    at = start(original, CAMERAS) + 0x10
    count = len(Pvars(original).table)
    for i, node in owner.items():
        class_id, *place, pvar = CAMERA.unpack(cameras[i])
        meta, edits = meta_of(node), []
        try:
            t = placed(nodes, node)
            if not same(t, scene_transform(euler_matrix(place[:3], place[3:], 1.0))):
                position, rotation, _ = placement(t)
                place = [*position, *rotation]
                edits.append("moved")
        except FormatError as e:
            problems.append(f"{node['name']}: {e}; not packed")
        for key, value in (("class", class_id), ("pvar_index", pvar)):
            new = meta.get(key, value)
            if new == value:
                continue
            if not isinstance(new, int) or (key == "pvar_index" and not -1 <= new < count) or not -(1 << 31) <= new < (1 << 31):
                problems.append(f"{node['name']}: rc1_{key} = {new!r} is out of range; not packed")
                continue
            if key == "class":
                class_id = new
            else:
                pvar = new
            edits.append(key)
        for key in sorted(set(meta) - {"index", "class", "pvar_index"}):
            if not owns(key):
                problems.append(f"{node['name']}: no such value: rc1_{key}")
        if edits:
            out[at + i * CAMERA_SIZE:at + (i + 1) * CAMERA_SIZE] = CAMERA.pack(class_id, *place, pvar)
        if pvar_edits(node, meta, CAMERA.unpack(cameras[i])[7], pvar, CAMERA_HEADER):
            edits.append("pvars")
        if edits:
            changes.append(f"{node['name']}: {', '.join(edits)}")


def pack_sounds(original: bytes, out: bytearray, nodes: dict, pvar_edits, changes: list[str], problems: list[str]) -> None:
    from pack import same, scene_transform
    if "Game/Sounds" not in nodes:
        return
    sounds = table(original, SOUNDS, SOUND_SIZE)
    owner, extra = owners(children(nodes, "Game/Sounds"), len(sounds), "Sound")
    for node in extra:
        problems.append(f"{node['name']}: adding sound instances is not supported; not packed")
    for i in sorted(set(range(len(sounds))) - set(owner)):
        problems.append(f"Sound_{i:04}: deleting sound instances is not supported; kept")
    at = start(original, SOUNDS) + 0x10
    count = len(Pvars(original).table)
    for i, node in owner.items():
        head = list(SOUND_HEAD.unpack_from(sounds[i]))
        body, meta, edits = sounds[i][0x10:], meta_of(node), []
        try:
            t = placed(nodes, node)
            if not same(t, scene_transform(shape_matrix(body))):
                body = shape_body(t, body)
                edits.append("moved")
        except FormatError as e:
            problems.append(f"{node['name']}: {e}; not packed")
        for slot, key, low, high in ((0, "class", -0x8000, 0x8000), (1, "m_class", -0x8000, 0x8000),
                                     (2, "function", 0, 1 << 32), (3, "pvar_index", -1, count), (4, "range", None, None)):
            if key not in meta:
                continue
            new, old = meta[key], head[slot]
            if key == "range":
                if not isinstance(new, (int, float)) or isinstance(new, bool) or not math.isfinite(new):
                    problems.append(f"{node['name']}: rc1_range must be a number; not packed")
                elif not same_typed(new, old, "f32"):
                    head[slot] = float(new)
                    edits.append("range")
                continue
            if new == old:
                continue
            if not isinstance(new, int) or isinstance(new, bool) or not low <= new < high:
                problems.append(f"{node['name']}: rc1_{key} = {new!r} is out of range; not packed")
                continue
            head[slot] = new
            edits.append(key)
        for key in sorted(set(meta) - {"index", "class", "m_class", "function", "pvar_index", "range"}):
            if not owns(key):
                problems.append(f"{node['name']}: no such value: rc1_{key}")
        if edits:
            out[at + i * SOUND_SIZE:at + (i + 1) * SOUND_SIZE] = SOUND_HEAD.pack(*head) + body
        if pvar_edits(node, meta, SOUND_HEAD.unpack_from(sounds[i])[3], head[3], None):
            edits.append("pvars")
        if edits:
            changes.append(f"{node['name']}: {', '.join(edits)}")


def pack_shapes(original: bytes, nodes: dict, pointer: int, group: str, prefix: str, sections: dict,
                changes: list[str], problems: list[str]) -> None:
    """A shape section: records edited in place, nodes without one appended (other data names
    volumes by their place, so none is deleted or moved)."""
    from pack import same, scene_transform
    parent = f"Game/Volumes/{group}"
    if parent not in nodes:
        return
    bodies = table(original, pointer, SHAPE_SIZE)
    owner, extra = owners(children(nodes, parent), len(bodies), prefix)
    for i in sorted(set(range(len(bodies))) - set(owner)):
        problems.append(f"{prefix}_{i:04}: deleting volumes is not supported (cameras, pvars and the camera grid "
                        "name them by place); kept")
    result, edited = list(bodies), False
    for i, node in sorted(owner.items()):
        try:
            t = placed(nodes, node)
            if not same(t, scene_transform(shape_matrix(bodies[i]))):
                result[i] = shape_body(t, bodies[i])
                changes.append(f"{node['name']}: moved")
                edited = True
        except FormatError as e:
            problems.append(f"{node['name']}: {e}; not packed")
    for node in extra:
        index = meta_of(node).get("index")
        model = bodies[index] if isinstance(index, int) and 0 <= index < len(bodies) else None
        try:
            t = placed(nodes, node)
            # A copy left where its volume is keeps that volume's bytes.
            same_place = model is not None and same(t, scene_transform(shape_matrix(model)))
            result.append(model if same_place else shape_body(t, model))
            changes.append(f"{node['name']}: added as {prefix}_{len(result) - 1:04}")
            edited = True
        except FormatError as e:
            problems.append(f"{node['name']}: {e}; not packed")
    if edited:
        sections[pointer] = write_table(original, pointer, result)


def path_edits(nodes: dict, node: dict, meta: dict, subresources: dict, points: list[tuple] | None,
               problems: list[str]) -> list[tuple] | None:
    """A path's points as the node has them, or None when they are the disc's (POINTS) or cannot
    be packed. A point's fourth float is rc1_w's (the disc's -1 for a point added in Godot)."""
    from pack import near
    try:
        now = curve_points(nodes, node, subresources)
    except FormatError as e:
        problems.append(f"{e}; not packed")
        return None
    if now is None:
        return None
    w = meta.get("w", [p[3] for p in points] if points is not None and len(points) == len(now) else [])
    if not isinstance(w, list) or not all(isinstance(v, (int, float)) and not isinstance(v, bool) for v in w):
        problems.append(f"{node['name']}: rc1_w must list numbers; not packed")
        return None
    w = [float(v) for v in w[:len(now)]] + [PATH_W] * max(0, len(now) - len(w))
    result = [(*p, v) for p, v in zip(now, w)]
    if not all(math.isfinite(v) for p in result for v in p):
        problems.append(f"{node['name']}: a point that is not a number; not packed")
        return None
    if points is not None and len(points) == len(result) and all(
            near(a, b) for p, q in zip(result, points) for a, b in zip(p, q)):
        return None
    if not result:
        problems.append(f"{node['name']}: a path needs a point; not packed")
        return None
    return result


def bounding_sphere(points: list[tuple], sphere: tuple | None = None) -> tuple:
    """A sphere around POINTS: SPHERE (the path's own, which the disc's builder made by a rule
    not known here) when it holds them all, else grown just enough for each point outside it;
    with none, the centre of their box and the farthest point's distance."""
    if sphere is None or sphere[3] <= 0:
        low = [min(p[a] for p in points) for a in range(3)]
        high = [max(p[a] for p in points) for a in range(3)]
        centre, radius = [(low[a] + high[a]) / 2 for a in range(3)], 0.0
    else:
        centre, radius = list(sphere[:3]), sphere[3]
    for p in points:
        d = math.dist(centre, p[:3])
        if d > radius:
            grow = (d - radius) / 2
            centre = [c + (p[a] - c) * grow / d for a, c in enumerate(centre)]
            radius += grow
    from pack import near
    radius = max([radius] + [math.dist(centre, p[:3]) for p in points])
    if sphere is not None and all(near(a, b) for a, b in zip((*centre, radius), sphere)):
        return tuple(sphere)
    return (*centre, f32(radius) * (1 + 2 ** -20))


def pack_paths(original: bytes, nodes: dict, subresources: dict, sections: dict, changes: list[str], problems: list[str]) -> None:
    """Paths and grind paths: edited in place or appended, never deleted (cameras, pvars and the
    level settings name paths by their place)."""
    from pack import near
    if "Game/Paths" in nodes:
        paths = read_paths(original)
        owner, extra = owners(children(nodes, "Game/Paths"), len(paths), "Path")
        for i in sorted(set(range(len(paths))) - set(owner)):
            problems.append(f"Path_{i:04}: deleting paths is not supported (other data names them by place); kept")
        result, edited = list(paths), False
        for i, node in sorted(owner.items()):
            points = path_edits(nodes, node, meta_of(node), subresources, paths[i], problems)
            if points is not None:
                result[i] = points
                changes.append(f"{node['name']}: points")
                edited = True
        for node in extra:
            points = path_edits(nodes, node, meta_of(node), subresources, None, problems)
            if points is not None:
                result.append(points)
                changes.append(f"{node['name']}: added as Path_{len(result) - 1:04}")
                edited = True
        if edited:
            sections[PATHS] = write_paths(original, result)
    if "Game/GrindPaths" in nodes and start(original, GRIND_PATHS) > 0:
        grinds = read_grind_paths(original)
        owner, extra = owners(children(nodes, "Game/GrindPaths"), len(grinds), "GrindPath")
        for i in sorted(set(range(len(grinds))) - set(owner)):
            problems.append(f"GrindPath_{i:04}: deleting grind paths is not supported (other data names them by place); kept")
        result, edited = [dict(g) for g in grinds], False
        for node, i in [(n, i) for i, n in sorted(owner.items())] + [(n, None) for n in extra]:
            meta = meta_of(node)
            base = grinds[i] if i is not None else {"sphere": (0.0, 0.0, 0.0, 0.0), "words": (0, 0, 0, 0), "points": None}
            points = path_edits(nodes, node, meta, subresources, base["points"], problems)
            record = {"sphere": base["sphere"], "words": base["words"], "points": points or base["points"]}
            edits = ["points"] if points is not None else []
            sphere = meta.get("bounding_sphere")
            if sphere is not None and (not isinstance(sphere, list) or len(sphere) != 4
                                       or not all(isinstance(v, (int, float)) and math.isfinite(v) for v in sphere)):
                problems.append(f"{node['name']}: rc1_bounding_sphere needs four numbers; not packed")
            elif sphere is not None and not all(near(float(a), b) for a, b in zip(sphere, base["sphere"])):
                record["sphere"] = tuple(float(v) for v in sphere)
                edits.append("bounding sphere")
            elif points is not None:
                record["sphere"] = bounding_sphere(points, base["sphere"] if i is not None else None)
            words = list(base["words"])
            for slot, key in ((0, "flag"), (1, "pad")):
                value = meta.get(key, words[0] if slot == 0 else words[1:])
                if slot == 1 and value == list(words[1:]):
                    continue
                if slot == 0 and isinstance(value, int) and not isinstance(value, bool) and -(1 << 31) <= value < (1 << 31):
                    if value != words[0]:
                        words[0] = value
                        edits.append("flag")
                elif slot == 1 and isinstance(value, list) and len(value) == 3 and all(isinstance(v, int) for v in value):
                    words[1:] = value
                    edits.append("pad")
                else:
                    problems.append(f"{node['name']}: rc1_{key} = {value!r} cannot be packed")
            record["words"] = tuple(words)
            if record["points"] is None:
                problems.append(f"{node['name']}: added without points; not packed")
                continue
            if i is None:
                result.append(record)
                changes.append(f"{node['name']}: added as GrindPath_{len(result) - 1:04}")
                edited = True
            elif edits:
                result[i] = record
                changes.append(f"{node['name']}: {', '.join(edits)}")
                edited = True
        if edited:
            sections[GRIND_PATHS] = write_grind_paths(original, result)


def pack_settings(original: bytes, out: bytearray, nodes: dict, changes: list[str], problems: list[str]) -> None:
    from pack import near, same, scene_transform
    node = nodes.get("Game/LevelSettings")
    settings = read_settings(original)
    if node is None or settings is None:
        return
    meta, new, edits = meta_of(node), dict(settings), []
    counts = {"ship_path": len(read_paths(original)), "ship_camera_cuboids": len(table(original, 0x60, SHAPE_SIZE))}
    for key in SETTINGS_FIELDS:
        if key not in meta:
            continue
        value, old = meta[key], settings[key]
        if isinstance(old, float):
            if isinstance(value, bool) or not isinstance(value, (int, float)) or not math.isfinite(value):
                problems.append(f"LevelSettings: rc1_{key} must be a number; not packed")
            elif not near(float(value), old):
                new[key] = float(value)
                edits.append(key)
            continue
        if value == old:
            continue
        values = value if isinstance(value, list) else [value]
        if (not isinstance(value, type(old)) or len(values) != len(old if isinstance(old, list) else [old])
                or not all(isinstance(v, int) and not isinstance(v, bool) and -(1 << 31) <= v < (1 << 31) for v in values)):
            problems.append(f"LevelSettings: rc1_{key} = {value!r} cannot be packed")
            continue
        if key in counts and not all(-1 <= v < counts[key] for v in values):
            problems.append(f"LevelSettings: rc1_{key} = {value!r} names none of the level's "
                            f"{'paths' if key == 'ship_path' else 'cuboids'}; not packed")
            continue
        new[key] = value
        edits.append(key)
    for key in sorted(set(meta) - set(SETTINGS_FIELDS)):
        problems.append(f"LevelSettings: no such value: rc1_{key}")
    ship = nodes.get("Game/LevelSettings/Ship")
    if ship is not None:
        try:
            t = placed(nodes, ship)
            if not same(t, scene_transform(euler_matrix(settings["ship_position"], (0.0, 0.0, settings["ship_yaw"]), 1.0))):
                if not all(math.isfinite(v) for v in t):
                    raise FormatError("a transform that is not a number")
                new["ship_position"] = list(t[9:12])
                new["ship_yaw"] = math.atan2(t[3], t[0])    # rotation about Z only: the first axis's turn
                edits.append("ship moved")
        except FormatError as e:
            problems.append(f"Ship: {e}; not packed")
    if edits:
        at = start(original, SETTINGS)
        out[at:at + SETTINGS_SIZE] = write_settings(new)
        changes.append(f"level settings: {', '.join(edits)}")
