"""Pack the levels edited in Godot back into the game's level data.

    python3 editor/extract.py pack baserom/SCES_509.16.iso assets/godot assets/packed/rac1

For each level whose scene differs from the disc, writes that level's
gameplay file (the placements) as the game stores it, and a list of the disc
sectors each file replaces:

    <out>/level_NN/gameplay_pal.wad   the level's gameplay file, compressed
    <out>/patches.tsv                 first sector, sector count, file: one line per file
    <out>/changes.txt                 what was changed, in words

The disc image is never written. The native port reads `patches.tsv` (its
`--mods` option) and serves those sectors from the files instead of the disc.

What it packs, for now: mobys that were moved, turned, scaled, given another
class or other values (the `rc1_*` metadata). A moby that was added or
deleted is reported and left out, as are edits to ties, shrubs and terrain:
their tables are a later step.

A moby the scene has as the extractor wrote it keeps its bytes on the disc,
so a scene nobody edited packs to nothing, and an edited level's file differs
from the disc's only in the records that were edited.
"""

import math
import re
import struct
from pathlib import Path

from disc import SECTOR
from formats import FormatError, unpack
from level import decoded
from mobys import ALWAYS, FIELDS, POINTER, RECORD, SIZE, USUAL, matrix, moby_instances
from wadpack import compress

# Godot saves a node's header with more in it than the extractor writes (type, unique_id).
NODE = re.compile(r'^\[node name="([^"]+)"[^\]]* parent="Game/Mobys"')
INSTANCE = re.compile(r'instance=ExtResource\("([^"]+)"\)')
RESOURCE = re.compile(r'^\[ext_resource [^\]]*path="([^"]+)"[^\]]*id="([^"]+)"')
CLASS_SCENE = re.compile(r"/moby_(\d+)\.tscn$")
PROPERTY = re.compile(r"^([A-Za-z_/0-9]+) = (.*)$")
IDENTITY = (1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0)


def f32(x: float) -> bytes:
    """The 32-bit float a number in the scene stands for; the scene writes -0 as 0."""
    return struct.pack("<f", x + 0.0 if x else 0.0)


def scene_mobys(text: str) -> list[dict]:
    """The moby nodes of a level scene: name, class id, the 12 floats of its transform
    (basis rows, then origin) and its rc1_* metadata."""
    classes, nodes, node = {}, [], None
    for line in text.splitlines():
        if line.startswith("["):
            node = None
            resource = RESOURCE.match(line)
            if resource:
                found = CLASS_SCENE.search(resource.group(1))
                if found:
                    classes[resource.group(2)] = int(found.group(1))
                continue
            match = NODE.match(line)
            if match:
                instance = INSTANCE.search(line)
                node = {"name": match.group(1), "class_id": classes.get(instance.group(1)) if instance else None,
                        "transform": IDENTITY, "meta": {}}
                nodes.append(node)
            continue
        if node is None:
            continue
        prop = PROPERTY.match(line)
        if not prop:
            continue
        key, value = prop.groups()
        if key == "transform":
            inner = re.fullmatch(r"Transform3D\((.*)\)", value.strip())
            if not inner:
                raise FormatError(f"{node['name']}: cannot read its transform")
            floats = tuple(float(v) for v in inner.group(1).split(","))
            if len(floats) != 12:
                raise FormatError(f"{node['name']}: a transform has 12 numbers")
            node["transform"] = floats
        elif key.startswith("metadata/rc1_"):
            name, value = key[len("metadata/rc1_"):], value.strip()
            if value.startswith("["):
                node["meta"][name] = [int(float(v)) for v in value.strip("[]").split(",") if v.strip()]
            else:
                number = float(value)
                node["meta"][name] = int(number) if number == int(number) and "." not in value else number
    return nodes


def scene_transform(m: list[float]) -> tuple:
    """A column-major matrix as the scene lists it: basis rows, then the origin."""
    return tuple([m[col * 4 + row] for row in range(3) for col in range(3)] + list(m[12:15]))


def near(x: float, y: float) -> bool:
    """The same 32-bit float, or its neighbour: the extractor writes the shortest text of a
    64-bit value, which for a value midway between two 32-bit floats can read back as the other."""
    a, b = (struct.unpack("<i", f32(v))[0] for v in (x, y))
    return abs(a - b) <= 1


def same(a: tuple, b: tuple) -> bool:
    """Equal as the 32-bit floats the scene's text stands for."""
    return all(near(x, y) for x, y in zip(a, b))


def same_value(a, b) -> bool:
    """Equal as the scene's text stands for them: a float is a 32-bit one."""
    if isinstance(a, float) or isinstance(b, float):
        return near(a, b)
    return a == b


def same_fields(a: dict, b: dict) -> bool:
    return a.keys() == b.keys() and all(same_value(a[k], b[k]) for k in a)


def placement(t: tuple) -> tuple[tuple, tuple, float]:
    """Position, Euler angles (radians, R = Rz * Ry * Rx) and scale of a scene transform."""
    rows = [t[0:3], t[3:6], t[6:9]]
    scales = [math.sqrt(sum(rows[r][c] ** 2 for r in range(3))) for c in range(3)]
    scale = scales[0]
    if scale <= 0 or any(abs(s - scale) > 1e-3 * max(scale, 1.0) for s in scales):
        raise FormatError("a moby has one scale for all three axes")
    r = [[rows[i][j] / scale for j in range(3)] for i in range(3)]
    sy = max(-1.0, min(1.0, -r[2][0]))
    ry = math.asin(sy)
    if abs(sy) < 0.999999:
        rx, rz = math.atan2(r[2][1], r[2][2]), math.atan2(r[1][0], r[0][0])
    else:                                   # looking straight up or down: x and z turn about one axis
        rx, rz = math.atan2(-r[1][2], r[1][1]), 0.0
    return tuple(t[9:12]), (rx, ry, rz), scale


def pack_level(level_id: int, gameplay: bytes, scene: str) -> tuple[bytes, list[str], list[str]]:
    """The gameplay file with the scene's edits, what changed, and what could not be packed."""
    start, = unpack("<I", gameplay, POINTER)
    originals = moby_instances(gameplay)
    out, changes, problems = bytearray(gameplay), [], []
    seen: dict[int, str] = {}
    for node in scene_mobys(scene):
        index = node["meta"].get("index")
        if index is None or not 0 <= index < len(originals):
            problems.append(f"{node['name']}: added in Godot; adding mobys is not packed yet")
            continue
        if index in seen:
            problems.append(f"{node['name']}: a copy of {seen[index]}; adding mobys is not packed yet")
            continue
        seen[index] = node["name"]
        original = originals[index]
        at = start + 0x10 + index * SIZE
        record = dict(zip(FIELDS, RECORD.unpack_from(gameplay, at)))
        edits = []
        if not same(node["transform"], scene_transform(original["matrix"])):
            (x, y, z), (rx, ry, rz), scale = placement(node["transform"])
            record.update(x=x, y=y, z=z, rx=rx, ry=ry, rz=rz, scale=scale)
            edits.append("moved")
        if node["class_id"] is not None and node["class_id"] != original["class_id"]:
            record["class_id"] = node["class_id"]
            edits.append(f"class {original['class_id']} -> {node['class_id']}")
        # rc1_class is the class scene's own; Godot saves it on each instance too.
        meta = {k: v for k, v in node["meta"].items() if k not in ("index", "class")}
        if not same_fields(meta, original["fields"]):
            for key in ALWAYS:
                if key in meta:
                    record[key] = meta[key]
            if "colour" in meta:
                record["r"], record["g"], record["b"] = meta["colour"]
            for key, usual in USUAL.items():
                record[key] = meta.get(key, usual)
            unknown = sorted(set(meta) - set(ALWAYS) - set(USUAL) - {"colour"})
            if unknown:
                problems.append(f"{node['name']}: no such value: rc1_{', rc1_'.join(unknown)}")
            edits.append("values")
        if edits:
            RECORD.pack_into(out, at, *(record[f] for f in FIELDS))
            changes.append(f"{node['name']}: {', '.join(edits)}")
    for index in sorted(set(range(len(originals))) - set(seen)):
        problems.append(f"Moby_{index:04}: deleted in Godot; deleting mobys is not packed yet (it stays in the level)")
    return bytes(out), changes, problems


def pack(disc, survey: dict, project: Path, out: Path) -> tuple[int, int]:
    """Writes OUT from the scenes of PROJECT. Returns (levels packed, mobys changed)."""
    patches, report, packed, total = [], [], 0, 0
    for info in survey["levels"]:
        scene = project / "levels" / f"level_{info['id']:02}" / f"level_{info['id']:02}.tscn"
        if not scene.is_file():
            continue
        place = info["ranges"]["gameplay_pal"]
        stored = disc.sectors(place["lba"], place["sectors"])
        gameplay = decoded(stored)
        edited, changes, problems = pack_level(info["id"], gameplay, scene.read_text())
        if changes or problems:
            report.append(f"level {info['id']:02}")
            report += [f"  {line}" for line in changes] + [f"  not packed: {line}" for line in problems]
        if not changes:
            continue
        stream = compress(edited, stored[:16])
        if decoded(stream) != edited:
            raise FormatError(f"level {info['id']}: the packed file does not read back")
        room = place["sectors"] * SECTOR
        if len(stream) > room:
            raise FormatError(f"level {info['id']}: the packed gameplay file is {len(stream)} bytes, "
                              f"{len(stream) - room} more than its place on the disc")
        name = f"level_{info['id']:02}/gameplay_pal.wad"
        (out / name).parent.mkdir(parents=True, exist_ok=True)
        (out / name).write_bytes(stream + bytes(room - len(stream)))
        patches.append(f"{place['lba']}\t{place['sectors']}\t{name}")
        packed += 1
        total += len(changes)
    out.mkdir(parents=True, exist_ok=True)
    (out / "patches.tsv").write_text("# first sector\tsectors\tfile (beside this list)\n" + "".join(p + "\n" for p in patches))
    (out / "changes.txt").write_text("\n".join(report) + "\n" if report else "No level differs from the disc.\n")
    return packed, total
