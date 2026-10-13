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
class or other values (the `rc1_*` metadata), added (a copy made in Godot, or
a class scene of the level dropped in) or deleted. Edits to ties, shrubs and
terrain are a later step, as are classes the level does not have.

A moby the scene has as the extractor wrote it keeps its bytes on the disc,
so a scene nobody edited packs to nothing, and a level whose mobys were only
edited differs from the disc's in those records alone. Adding and deleting
write the table again, and with it what names mobys by their place in it
(gameplay.py).
"""

import math
import re
import struct
from pathlib import Path

from disc import SECTOR
from formats import FormatError, unpack
from level import decoded
import gameplay as gp
from mobys import (ALWAYS, FIELDS, POINTER, RECORD, SIZE, USUAL, matrix, moby_class_names,
                   moby_instances, node_name)
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


def apply(node: dict, record: dict, original: dict | None, whole: bool, problems: list[str]) -> list[str]:
    """Writes a scene node's placement, class and values into RECORD (a moby's fields). ORIGINAL is
    the moby the node was extracted from, when RECORD is still that moby's: what the scene has
    as the extractor wrote it is then left alone. WHOLE says the node lists every value that is
    not the usual one (a node the extractor wrote, or Godot's copy of one), so a value it does
    not list is the usual one. Returns what changed, in words."""
    edits = []
    if original is None or not same(node["transform"], scene_transform(original["matrix"])):
        (x, y, z), (rx, ry, rz), scale = placement(node["transform"])
        record.update(x=x, y=y, z=z, rx=rx, ry=ry, rz=rz, scale=scale)
        edits.append("moved")
    if node["class_id"] is not None and node["class_id"] != record["class_id"]:
        edits.append(f"class {record['class_id']} -> {node['class_id']}")
        record["class_id"] = node["class_id"]
    # rc1_class is the class scene's own; Godot saves it on each instance too.
    meta = {k: v for k, v in node["meta"].items() if k not in ("index", "class")}
    if original is None or not same_fields(meta, original["fields"]):
        for key in ALWAYS:
            if key in meta:
                record[key] = meta[key]
        if "colour" in meta:
            record["r"], record["g"], record["b"] = meta["colour"]
        for key, usual in USUAL.items():
            if whole or key in meta:
                record[key] = meta.get(key, usual)
        unknown = sorted(set(meta) - set(ALWAYS) - set(USUAL) - {"colour"})
        if unknown:
            problems.append(f"{node['name']}: no such value: rc1_{', rc1_'.join(unknown)}")
        edits.append("values")
    return edits


def pack_level(level_id: int, gameplay: bytes, scene: str) -> tuple[bytes, list[str], list[str]]:
    """The gameplay file with the scene's edits, what changed, and what could not be packed.

    A moby the scene has once keeps its place in the table. A node with the index of another
    (Godot's copy of a moby) or with none (a class scene dropped into the level) is a new moby,
    after the table's own: it takes after the moby it copies, or the level's first moby of its
    class, with a spawn number of its own and a copy of that moby's pvar block (and of the
    fixups, links and shared records that name it), and joins its group. A moby with no node
    left is deleted: the table closes up, and the groups and the pvars' moby links, which name
    mobys by their place in the table, follow."""
    originals = moby_instances(gameplay)
    raw = gp.records(gameplay)
    nodes = scene_mobys(scene)
    changes, problems = [], []

    def index_of(node: dict) -> int | None:
        index = node["meta"].get("index")
        return index if isinstance(index, int) and 0 <= index < len(originals) else None

    # The node that is each moby: the one still named after its place when Godot copied it.
    owner: dict[int, dict] = {}
    for node in nodes:
        index = index_of(node)
        if index is None:
            continue
        named = node["name"].endswith(f"_{index:04}")
        if index not in owner or (named and not owner[index]["name"].endswith(f"_{index:04}")):
            owner[index] = node

    kept: dict[int, bytes] = {}
    added: list[tuple[dict, int | None]] = []          # the new moby's fields, the moby it takes after
    for node in nodes:
        index = index_of(node)
        try:
            if index is not None and owner[index] is node:
                record = dict(zip(FIELDS, RECORD.unpack(raw[index])))
                edits = apply(node, record, originals[index], True, problems)
                kept[index] = RECORD.pack(*(record[f] for f in FIELDS)) if edits else raw[index]
                if edits:
                    changes.append(f"{node['name']}: {', '.join(edits)}")
                continue
            model = index
            if model is None and node["class_id"] is not None:
                model = next((m["index"] for m in originals if m["class_id"] == node["class_id"]), None)
            if model is None and node["class_id"] is None:
                problems.append(f"{node['name']}: added in Godot with no class scene; not packed")
                continue
            if model is None:
                record = dict.fromkeys(FIELDS, 0)
                record.update(USUAL, size=SIZE, class_id=node["class_id"], scale=1.0, group=-1, pvar_index=-1,
                              r=128, g=128, b=128)
                problems.append(f"{node['name']}: the level has no moby of class {node['class_id']} to take "
                                "after; added with the usual values and no pvars")
            else:
                record = dict(zip(FIELDS, RECORD.unpack(raw[model])))
            source_pvar = record["pvar_index"] if model is not None else -1
            # A copy lists its values as the moby it copies does; a class scene dropped in lists none.
            apply(node, record, None, index is not None, problems)
            record["pvar_index"] = source_pvar          # the copy gets a block of its own below
            added.append((record, model))
            changes.append(f"{node['name']}: added" + (f", after moby {model}" if model is not None else ""))
        except FormatError as e:
            problems.append(f"{node['name']}: {e}; not packed")
            if index is not None and owner[index] is node:
                kept[index] = raw[index]
    deleted = sorted(set(range(len(originals))) - set(kept))
    names = moby_class_names()
    for index in deleted:
        changes.append(f"{node_name(originals[index]['class_id'], index, names)}: deleted")

    if not added and not deleted:
        out = bytearray(gameplay)
        start, = unpack("<I", gameplay, POINTER)
        for index, record in kept.items():
            out[start + 0x10 + index * SIZE:start + 0x10 + (index + 1) * SIZE] = record
        return bytes(out), changes, problems

    # The table again: the mobys that stay, in their order, then the new ones.
    place = {old: new for new, old in enumerate(sorted(kept))}
    table = [kept[old] for old in sorted(kept)]
    group_lists = gp.groups(gameplay)
    group_lists = [None if g is None else [place[m] for m in g if m in place] for g in group_lists]
    links, pointers = gp.fixups(gameplay, gp.LINKS), gp.fixups(gameplay, gp.POINTERS)
    blob, shared_records = gp.shared(gameplay)
    used = [m["fields"]["pvar_index"] for m in originals] + [p for p, _ in links + pointers]
    pvars = gp.pvar_table(gameplay, gp.pvar_count(gameplay, used))
    data_start, data_end = gp.extent(gameplay, gp.DATA)
    data = bytearray(gameplay[data_start:data_end]) if data_start > 0 else bytearray()
    spawn = max((m["fields"]["spawn_id"] for m in originals), default=-1)
    for record, model in added:
        spawn += 1
        record["spawn_id"] = spawn
        source = record["pvar_index"]
        if 0 <= source < len(pvars):
            at, size = pvars[source]
            block = bytes(data[at:at + size])
            data += bytes(-len(data) % 16)
            new = len(pvars)
            pvars.append((len(data), size))
            data += block
            links += [(new, where) for pvar, where in links if pvar == source]
            pointers += [(new, where) for pvar, where in pointers if pvar == source]
            shared_records += [(new, where, into) for pvar, where, into in shared_records if pvar == source]
            record["pvar_index"] = new
        else:
            record["pvar_index"] = -1
        group = record["group"]
        if 0 <= group < len(group_lists):
            group_lists[group] = (group_lists[group] or []) + [len(table)]
        table.append(RECORD.pack(*(record[f] for f in FIELDS)))
    # A link is a moby's place in the table, inside a pvar block: it follows the moby.
    for pvar, where in links:
        at = pvars[pvar][0] + where
        if at + 4 > len(data):
            continue
        moby, = struct.unpack_from("<i", data, at)
        if 0 <= moby < len(originals):
            struct.pack_into("<i", data, at, place.get(moby, -1))

    sections = {gp.MOBYS: gp.write_records(gameplay, table)}
    for pointer, new in ((gp.GROUPS, gp.write_groups(group_lists) if group_lists else b""),
                         (gp.TABLE, b"".join(struct.pack("<2i", *entry) for entry in pvars)),
                         (gp.DATA, bytes(data)),
                         (gp.LINKS, gp.write_fixups(links)),
                         (gp.POINTERS, gp.write_fixups(pointers)),
                         (gp.SHARED, gp.write_shared(blob, shared_records))):
        start = gp.offset(gameplay, pointer)
        if start > 0 and new and gameplay[start:start + len(new)] != new:
            sections[pointer] = new
    return gp.replace(gameplay, sections), changes, problems


def pack(disc, survey: dict, project: Path, out: Path) -> tuple[int, int]:
    """Writes OUT from the scenes of PROJECT. Returns (levels packed, mobys changed)."""
    patches, report, packed, total = [], [], 0, 0
    for info in survey["levels"]:
        name = f"level_{info['id']:02}"
        # one project with every level, or a project per level (`godot --split`)
        scene = next((s for s in (project / "levels" / name / f"{name}.tscn",
                                  project / name / "levels" / name / f"{name}.tscn") if s.is_file()), None)
        if scene is None:
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
        name = f"{name}/gameplay_pal.wad"
        (out / name).parent.mkdir(parents=True, exist_ok=True)
        (out / name).write_bytes(stream + bytes(room - len(stream)))
        patches.append(f"{place['lba']}\t{place['sectors']}\t{name}")
        packed += 1
        total += len(changes)
    out.mkdir(parents=True, exist_ok=True)
    (out / "patches.tsv").write_text("# first sector\tsectors\tfile (beside this list)\n" + "".join(p + "\n" for p in patches))
    (out / "changes.txt").write_text("\n".join(report) + "\n" if report else "No level differs from the disc.\n")
    return packed, total
