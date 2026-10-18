"""Moby instances: the level's placed objects (crates, enemies, NPCs, vendors...).

The gameplay file's header word at +0x44 points to a count, 12 bytes of
padding, then one 0x78-byte record per instance. The record layout and
what the level loader does with each field follow ReRAC (ISC):
crates/rc-formats/src/gameplay.rs and docs/plan/moby_render_notes.md.

Placement: the moby's matrix is R = Rz(z) * Ry(y) * Rx(x) (x applied
first) from the Euler angles in radians, times the instance scale, at the
position, all in game units and axes. The class scale (class header +0x24)
multiplies in when the class mesh is drawn, so it belongs to the class,
not the instance.
"""

import math
import re
import struct
from pathlib import Path

from formats import FormatError, unpack

CLASS_NAMES = Path(__file__).parent / "moby_classes.tsv"
POINTER = 0x44
SIZE = 0x78
RECORD = struct.Struct("<7if4i3f3f2if4i3i2i")
FIELDS = ("size", "unknown_4", "spawn_flags", "spawn_id", "unknown_10", "unknown_14", "class_id", "scale",
          "draw_distance", "update_distance", "unused_28", "unused_2c", "x", "y", "z", "rx", "ry", "rz",
          "group", "is_rooted", "rooted_distance", "unknown_54", "pvar_index", "occlusion", "mode_bits",
          "r", "g", "b", "light", "unknown_74")
assert RECORD.size == SIZE and len(FIELDS) == len(RECORD.unpack(bytes(SIZE)))

# The value most instances store (measured on all 19 PAL levels). Only a
# different value becomes node metadata; a packer writes these back for
# the rest. The last four never vary on the disc.
USUAL = {"unknown_4": -1, "spawn_flags": 0, "unknown_10": 0, "unknown_14": 0, "draw_distance": 64,
         "update_distance": 64, "is_rooted": 0, "rooted_distance": 0.0, "occlusion": 1, "mode_bits": 32,
         "light": 0, "unknown_74": -1, "unused_28": 32, "unused_2c": 64, "unknown_54": 1}
ALWAYS = ("spawn_id", "group", "pvar_index")


def moby_instances(gameplay: bytes) -> list[dict]:
    """Each instance: index, class_id, matrix (column-major 4x4), the
    Euler angles and scale it came from, and its other fields."""
    start, = unpack("<I", gameplay, POINTER)
    if start == 0:
        return []
    count, = unpack("<i", gameplay, start)
    if count < 0 or start + 0x10 + count * SIZE > len(gameplay):
        raise FormatError(f"moby instance table at {start:#x} does not fit ({count} records)")
    out = []
    for i in range(count):
        record = dict(zip(FIELDS, RECORD.unpack_from(gameplay, start + 0x10 + i * SIZE)))
        if record["size"] != SIZE:
            raise FormatError(f"moby instance {i}: size {record['size']:#x}, expected {SIZE:#x}")
        fields = {k: record[k] for k in ALWAYS}
        fields["colour"] = [record["r"], record["g"], record["b"]]
        fields.update({k: record[k] for k, usual in USUAL.items() if record[k] != usual})
        out.append({"index": i, "class_id": record["class_id"], "fields": fields,
                    "rotation": (record["rx"], record["ry"], record["rz"]), "scale": record["scale"],
                    "matrix": matrix((record["x"], record["y"], record["z"]),
                                     (record["rx"], record["ry"], record["rz"]), record["scale"])})
    return out


def matrix(position, rotation, scale: float) -> list[float]:
    """Column-major R * scale at position, R = Rz * Ry * Rx."""
    cx, cy, cz = (math.cos(a) for a in rotation)
    sx, sy, sz = (math.sin(a) for a in rotation)
    rows = [[cz * cy, cz * sy * sx - sz * cx, cz * sy * cx + sz * sx],
            [sz * cy, sz * sy * sx + cz * cx, sz * sy * cx - cz * sx],
            [-sy, cy * sx, cy * cx]]
    columns = [[rows[r][c] * scale for r in range(3)] + [0.0] for c in range(3)]
    return [v for column in columns for v in column] + [*position, 1.0]


def moby_class_names() -> dict[int, str]:
    """Class number -> name, from moby_classes.tsv (see its header)."""
    rows = (line.split("\t") for line in CLASS_NAMES.read_text().splitlines() if line and not line.startswith("#"))
    return {int(number): name for number, name in rows}


LEVEL_NAMES = ("Veldin", "Novalis", "Aridia", "Kerwan", "Eudora", "Rilgar", "Blarg Station", "Umbris", "Batalia",
               "Gaspar", "Orxon", "Pokitaru", "Hoven", "Gemlik Base", "Oltanis", "Quartu", "Kalebo III",
               "Drek's Fleet", "Veldin (the return)")


def node_name(class_id: int, index: int, names: dict[int, str]) -> str:
    """A placed moby's node: its class's name and its place in the level's table, "Vendor_0012";
    "Class809_0001" for a class nobody has named."""
    name = names.get(class_id)
    kind = "".join(part.capitalize() for part in re.split(r"[^A-Za-z0-9]+", name) if part) if name else f"Class{class_id}"
    return f"{kind}_{index:04}"


DECOMPILATION = Path(__file__).resolve().parents[1] / "games" / "rac1"
_UPDATE = re.compile(r"UpdateMoby_(\d+):([\d/]+)")
_logic = None


def moby_logic(level: int) -> dict[int, dict]:
    """class -> {"function", "file", "decompiled", "defined_as"} for the classes LEVEL updates: the function
    the game calls for each moby of the class every frame, where it is in the decompilation
    (games/rac1), and whether it is C there yet. From that project's
    config/overlays/names.tsv (the UpdateMoby_<class> roles) and its src/. Empty when the
    decompilation is not in this checkout."""
    global _logic
    if _logic is None:
        _logic = {}
        table = DECOMPILATION / "config" / "overlays" / "names.tsv"
        if table.is_file():
            # A function the decompilation has renamed is defined under its readable name
            # (include/names.h maps each one to its func_ symbol).
            names_h = DECOMPILATION / "include" / "names.h"
            symbol_of = dict(re.findall(r"^#define\s+(\w+)\s+(func_\w+)\s*$", names_h.read_text(), re.M)) \
                if names_h.is_file() else {}
            where, defined_as = {}, {}
            for path in sorted((DECOMPILATION / "src").rglob("*.c")):
                text = path.read_text(errors="replace")
                for name in re.findall(r"^(?:INCLUDE_ASM|ASM_FUNC)\([^)]*\b(func_\w+)\);", text, re.M):
                    where.setdefault(name, (path, False))
                for name in re.findall(r"^(?!extern\b)[A-Za-z_][^;=\n]*?\b([A-Za-z_]\w*)\s*\([^;]*$", text, re.M):
                    symbol = name if re.fullmatch(r"func_(?:L\d\d_)?[0-9A-F]{8}", name) else symbol_of.get(name)
                    if symbol is not None:
                        where[symbol] = (path, True)
                        defined_as[symbol] = name
            for line in table.read_text().splitlines():
                if line.startswith("#") or "\t" not in line:
                    continue
                function, roles = line.split("\t", 1)
                for class_id, levels in _UPDATE.findall(roles):
                    path, is_c = where.get(function, (None, False))
                    for lv in levels.split("/"):
                        _logic.setdefault(int(lv), {})[int(class_id)] = {
                            "function": function, "decompiled": is_c,
                            "defined_as": defined_as.get(function, function),
                            "file": path.relative_to(DECOMPILATION.parents[1]).as_posix() if path else None}
    return _logic.get(level, {})
