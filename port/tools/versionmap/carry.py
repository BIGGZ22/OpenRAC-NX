#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""
Carries a port's address lists from the version that runs to another version
of the same game, through the map versionmap.py builds.

  carry.py names    MAP GEN NAME ...          the target's name for each of the source's symbols
  carry.py table    MAP GEN FROM.tsv          a libraries.tsv, row by row (first column a symbol)
  carry.py c        MAP GEN FROM.c [--data-low HEX --data-high HEX]
                                               host C: its func_ names and its address literals

GEN is the target's generated folder (port/build/<preset>/games/<id>/gen):
its functions.c says what the target's decompilation calls the function at
each address, and stubs.c which of two spellings has no C.

Nothing carried is proven by being carried. Every line of output that rests
on a `near` pair (the function changed between the versions) or on a data
address that was not formed by code both versions share is reported on
stderr with its kind; `c` also marks it in the C with a comment.
"""
import argparse
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from versionmap import Map  # noqa: E402

SOURCE_NAME = re.compile(r"\bfunc_(?:L(\d\d)_)?([0-9A-F]{8})(?=__game\b|\b)")
ENTRY = re.compile(r"\{0x([0-9A-F]{8})u, \(openrac_host_fn\)(\w+), \"(\w+)\"\}")


class Target:
    """The target's names by address, per program, from its generated functions.c."""

    def __init__(self, gen: Path):
        self.names: dict[int, dict[int, list[str]]] = {}
        overlay = None
        for line in (gen / "functions.c").read_text().splitlines():
            m = re.match(r"static const openrac_fn_entry k_(exe|overlay_(\d\d))\[\]", line)
            if m:
                overlay = -1 if m.group(1) == "exe" else int(m.group(2))
                continue
            m = ENTRY.search(line)
            if m and overlay is not None:
                self.names.setdefault(overlay, {}).setdefault(int(m.group(1), 16), []).append(m.group(3))
        self.stubs = set(re.findall(r"^\w[\w ]*?\b(\w+)\([^)]*\) \{\n    openrac_guest_missing", (gen / "stubs.c").read_text(), re.M))

    def add_stubs(self, source: Path, places: Path | None) -> None:
        """Also the functions the target's decompilation only has as assembly stubs
        (INCLUDE_ASM lines), which hostgen never sees when nothing calls them by name:
        each by the address in its name and, for a level function, by its places."""
        stub = re.compile(r'INCLUDE_ASM\(\s*(?:"[^"]*"\s*)+,\s*(\w+)\s*\)')
        named = re.compile(r"(?:func|FUN)_(?:L(\d\d)_)?([0-9A-Fa-f]{8})$")
        where: dict[str, list[tuple[int, int]]] = {}
        if places is not None and places.is_file():
            for line in places.read_text().splitlines():
                cols = line.split("\t")
                if len(cols) > 5 and not line.startswith("#"):
                    where[cols[0]] = [(int(p[:2]), int(p[3:], 16)) for p in cols[5].split(",") if p]
        self.asm = set()
        for path in sorted(source.rglob("*.c")):
            for name in stub.findall(path.read_text(errors="replace")):
                self.asm.add(name)
                m = named.match(name)
                if not m:
                    continue
                own = (int(m.group(1)) if m.group(1) else -1, int(m.group(2), 16))
                for overlay, address in {own, *where.get(name, [])}:
                    known = self.names.setdefault(overlay, {}).setdefault(address, [])
                    if name not in known:
                        known.append(name)

    def name(self, address: int, overlay: int = -1) -> str | None:
        """The symbol the target has at ADDRESS: the one with C when two spell it."""
        found = self.names.get(overlay, {}).get(address)
        if not found and overlay >= 0:
            found = self.names.get(-1, {}).get(address)
        if not found:
            return None
        asm = [n for n in found if n in getattr(self, "asm", ())]
        with_c = [n for n in found if cname(n) not in self.stubs and n not in asm]
        # With C: that name. Without: the name its assembly stub has, which a candidate replaces.
        return (with_c or asm or found)[0]


def cname(symbol: str) -> str:
    """hostgen's C name of a game function (program.py)."""
    return symbol if symbol.startswith("func_") else f"game_{symbol}"


class Carrier:
    def __init__(self, map_path: Path, gen: Path, source: Path | None = None, places: Path | None = None):
        self.map, self.target = Map(map_path), Target(gen)
        if source is not None:
            self.target.add_stubs(source, places)
        self.notes: list[str] = []

    def symbol(self, name: str) -> tuple[str | None, str]:
        """(the target's symbol, kind) for one of the source's func_ names."""
        m = SOURCE_NAME.fullmatch(name)
        if not m:
            return None, "not an address name"
        level, address = m.group(1), int(m.group(2), 16)
        program = f"level:{level}" if level else "boot"
        hit = self.map.function(address, program)
        if hit is None:
            return None, "unpaired"
        there = self.target.name(hit[0], int(level) if level else -1)
        if there is None:
            return None, f"no name at {hit[0]:08X}"
        return there, hit[1]


def cmd_names(c: Carrier, args) -> int:
    for name in args.names:
        there, kind = c.symbol(name)
        print(f"{name}\t{there or '-'}\t{kind}")
    return 0


def cmd_table(c: Carrier, args) -> int:
    for line in Path(args.table).read_text().splitlines():
        cols = line.split("\t")
        if line.startswith("#") or not line.strip():
            print(line)
            continue
        there, kind = c.symbol(cols[0])
        if there is None:
            print(f"carry: {cols[0]} ({cols[2] if len(cols) > 2 else ''}): {kind}; row left out", file=sys.stderr)
            continue
        if kind != "same":
            print(f"carry: {cols[0]} -> {there}: {kind}", file=sys.stderr)
        cols[0] = there
        print("\t".join(cols))
    return 0


SOURCE_DATA = re.compile(r"\bD_(?:L(\d\d)_)?([0-9A-F]{8})\b")
LEVEL_MACRO = re.compile(r"OPENRAC_L(?:DATA|CODE)\(\s*(\d+)\s*,\s*$")


def carry_c(c: Carrier, text: str, low: int, high: int, keep_spelling: bool,
            program: str = "boot", defined: set[str] | None = None) -> tuple[str, list[str]]:
    """TEXT with the source's function names, D_ names and address literals as the target's.
    keep_spelling writes a function as the target's decompilation names it, or func_<address>
    where it has no name (the decompilation's dialect, for C hostgen translates: a spelling by
    address meets the function there); otherwise as the C name of the target's symbol (host C,
    compiled as it is)."""
    notes = []

    def function(m):
        there, kind = c.symbol(m.group(0))
        level, address = m.group(1), int(m.group(2), 16)
        hit = c.map.function(address, f"level:{level}" if level else "boot")
        if hit is None:
            notes.append(f"{m.group(0)}: unpaired")
            return m.group(0) + "__NOT_CARRIED"
        if hit[1] != "same":
            notes.append(f"{m.group(0)} -> {hit[0]:08X}: {hit[1]}")
        if keep_spelling:
            by_address = f"func_{'L' + level + '_' if level else ''}{hit[0]:08X}"
            if defined is not None:
                # The functions the C defines take the target's names (the stubs they replace);
                # what it only calls keeps a spelling by address, which hostgen's candidates
                # declare apart from the file they are put in (prep.py, own_declarations).
                return there if m.group(0) in defined and there else by_address
            return there or by_address
        if there is None:
            notes.append(f"{m.group(0)}: {kind}")
            return m.group(0) + "__NOT_CARRIED"
        return cname(there)

    def data_name(m):
        level, address = m.group(1), int(m.group(2), 16)
        program = f"level:{level}" if level else "boot"
        hit = c.map.data(address, program)
        fn = c.map.function(address, program)
        if fn is not None:
            hit = (fn[0], "code " + fn[1])
        if hit is None:
            notes.append(f"{m.group(0)}: no place found")
            return m.group(0) + "__NOT_CARRIED"
        if hit[1] not in ("exact", "code same"):
            notes.append(f"{m.group(0)} -> {hit[0]:08X}: {hit[1]}")
        return f"D_{'L' + level + '_' if level else ''}{hit[0]:08X}"

    def literal(m):
        value = int(m.group(1), 16)
        if not low <= value < high:
            return m.group(0)
        macro = LEVEL_MACRO.search(m.string, max(0, m.start() - 40), m.start())
        where = f"level:{int(macro.group(1)):02d}" if macro else program
        hit = c.map.data(value, where) or (None, "unknown")
        fn = c.map.function(value, where)
        if fn is not None:
            hit = (fn[0], "code " + fn[1])
        if hit[0] is None:
            notes.append(f"0x{value:08X}: no place found, left as it is")
            return m.group(0)
        if hit[1] not in ("exact", "code same"):
            notes.append(f"0x{value:08X} -> 0x{hit[0]:08X}: {hit[1]}" + (f" ({where})" if where != "boot" else ""))
        digits = m.group(1)
        out = f"{hit[0]:0{len(digits)}X}"
        return "0x" + (out if digits.upper() == digits else out.lower()) + m.group(2)

    text = SOURCE_NAME.sub(function, text)
    text = SOURCE_DATA.sub(data_name, text)
    text = re.sub(r"\b0x([0-9A-Fa-f]{6,8})([uU]?)\b", literal, text)
    return text, sorted(set(notes))


def cmd_c(c: Carrier, args) -> int:
    text, notes = carry_c(c, Path(args.file).read_text(), int(args.data_low, 16), int(args.data_high, 16),
                          args.keep_spelling, args.program)
    sys.stdout.write(text)
    for n in notes:
        print(f"carry: {n}", file=sys.stderr)
    return 0


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    sub = ap.add_subparsers(dest="cmd", required=True)
    for name in ("names", "table", "c"):
        p = sub.add_parser(name)
        p.add_argument("map", type=Path)
        p.add_argument("gen", type=Path)
        p.add_argument("--source", type=Path, help="the target decompilation's src: its assembly stubs count as names too")
        p.add_argument("--places", type=Path, help="the target's table of level places (hostgen.json, \"places\")")
        if name == "names":
            p.add_argument("names", nargs="+")
        elif name == "table":
            p.add_argument("table")
        else:
            p.add_argument("file")
            p.add_argument("--data-low", default="00100000")
            p.add_argument("--data-high", default="02000000")
            p.add_argument("--program", default="boot",
                           help="the program a bare address literal is of (boot, level:00, ...); inside "
                                "OPENRAC_LDATA(n, ...) it is level n's")
            p.add_argument("--keep-spelling", action="store_true",
                           help="write functions as the decompilation names them (C that hostgen translates), not as their host C names")
    args = ap.parse_args()
    c = Carrier(args.map, args.gen, args.source, args.places)
    return {"names": cmd_names, "table": cmd_table, "c": cmd_c}[args.cmd](c, args)


if __name__ == "__main__":
    sys.exit(main())
