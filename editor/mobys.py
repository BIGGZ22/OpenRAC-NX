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
import games

CLASS_NAMES = Path(__file__).parent / "moby_classes.tsv"
# Ratchet & Clank's record (games.RAC1_MOBY), which the level packer (gameplay.py, pack.py) writes.
POINTER = games.RAC1.mobys
SIZE = games.RAC1_MOBY.size
RECORD = struct.Struct(games.RAC1_MOBY.layout)
FIELDS = games.RAC1_MOBY.fields
# The value most instances store (measured on all 19 PAL levels). Only a
# different value becomes node metadata; a packer writes these back for
# the rest. The last four never vary on the disc.
USUAL = games.RAC1_MOBY.usual
ALWAYS = ("spawn_id", "group", "pvar_index")


def moby_instances(gameplay: bytes, fmt: games.Format = games.RAC1) -> list[dict]:
    """Each instance: index, class_id, matrix (column-major 4x4), the
    Euler angles and scale it came from, and its other fields. The record is the game's
    (fmt.moby: 0x78 bytes in Ratchet & Clank, 0x88 in Going Commando and Up Your Arsenal, 0x70
    in Deadlocked), its table at the gameplay header word fmt.mobys."""
    layout = fmt.moby
    record_format, size = struct.Struct(layout.layout), layout.size
    start, = unpack("<I", gameplay, fmt.mobys)
    if start == 0:
        return []
    count, = unpack("<i", gameplay, start)
    if count < 0 or start + 0x10 + count * size > len(gameplay):
        raise FormatError(f"moby instance table at {start:#x} does not fit ({count} records)")
    out = []
    for i in range(count):
        record = dict(zip(layout.fields, record_format.unpack_from(gameplay, start + 0x10 + i * size)))
        if record["size"] != size:
            raise FormatError(f"moby instance {i}: size {record['size']:#x}, expected {size:#x}")
        fields = {k: record[k] for k in ALWAYS}
        fields["colour"] = [record["r"], record["g"], record["b"]]
        fields.update({k: record[k] for k, usual in layout.usual.items() if record[k] != usual})
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
_SYMBOL = re.compile(r"func_(?:L\d\d_)?[0-9A-F]{8}")
_logic = None


def update_roles(table: str) -> dict[str, dict[int, list[int]]]:
    """function -> class -> the levels whose class table gives the class that update function,
    from the text of config/overlays/names.tsv (`func_...<TAB>UpdateMoby_500:00/01,...`)."""
    out: dict[str, dict[int, list[int]]] = {}
    for line in table.splitlines():
        if line.startswith("#") or "\t" not in line:
            continue
        function, roles = line.split("\t", 1)
        for class_id, levels in _UPDATE.findall(roles):
            out.setdefault(function, {})[int(class_id)] = [int(lv) for lv in levels.split("/")]
    return out


def names_h_symbols(path: Path) -> dict[str, str]:
    """include/names.h: readable name -> func_ symbol."""
    return dict(re.findall(r"^#define\s+(\w+)\s+(func_\w+)\s*$", path.read_text(), re.M)) if path.is_file() else {}


def c_definitions(sources, symbol_of: dict[str, str]) -> tuple[dict, dict]:
    """Where the C of each function is: (symbol -> (path, is_c), symbol -> the name it is defined
    under) over SOURCES, (path, text) pairs. A function is C where a file defines it under its
    func_ symbol, its readable name (SYMBOL_OF, from include/names.h: readable -> symbol) or a name
    of the file's own that `__asm__("func_...")` or a `#define` gives that symbol; assembly where only
    INCLUDE_ASM/ASM_FUNC has it."""
    where, defined_as = {}, {}
    for path, text in sources:
        for name in re.findall(r"^(?:INCLUDE_ASM|ASM_FUNC)\([^)]*\b(func_\w+)\);", text, re.M):
            where.setdefault(name, (path, False))
        aliases = dict(re.findall(r'\b([A-Za-z_]\w*)\s*\([^;{}()]*\)\s*__asm__\s*\(\s*"(func_\w+)"\s*\)', text))
        aliases.update(re.findall(r"^#define\s+(\w+)\s+(func_\w+)\s*$", text, re.M))
        # A definition: a line starting with its type, with no ';' before the line's end (a
        # prototype has one). Lazy, so that one definition does not swallow the ones after it.
        for name in re.findall(r"^(?!extern\b)[A-Za-z_][^;=\n]*?\b([A-Za-z_]\w*)\s*\([^;]*?$", text, re.M):
            symbol = name if _SYMBOL.fullmatch(name) else symbol_of.get(name) or aliases.get(name)
            if symbol is not None:
                where[symbol] = (path, True)
                defined_as[symbol] = name
    return where, defined_as


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
            symbol_of = names_h_symbols(DECOMPILATION / "include" / "names.h")
            sources = ((p, p.read_text(errors="replace")) for p in sorted((DECOMPILATION / "src").rglob("*.c")))
            where, defined_as = c_definitions(sources, symbol_of)
            for function, classes in update_roles(table.read_text()).items():
                path, is_c = where.get(function, (None, False))
                for class_id, levels in classes.items():
                    for lv in levels:
                        _logic.setdefault(lv, {})[class_id] = {
                            "function": function, "decompiled": is_c,
                            "defined_as": defined_as.get(function, function),
                            "file": path.relative_to(DECOMPILATION.parents[1]).as_posix() if path else None}
    return _logic.get(level, {})
