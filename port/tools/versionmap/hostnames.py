#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""
Keeps a version's table of names and addresses (host/rac1_version.h) in step
with the host C its game's versions share (port/game/rac1/host, README.md).

  hostnames.py check VERSION_DIR [--map MAP --gen GEN]   what the table lacks, what it has unused,
                                                         and, with the map, where it disagrees
  hostnames.py write VERSION_DIR --map MAP --gen GEN      the table's generated part again: the
                                                         entries kept, new ones from the map

VERSION_DIR is port/game/<id> (its hostgen.json names the shared C,
"host_shared"). The shared C names functions as games/rac1 does
(func_001F8B6C, func_L00_002A8A20, func_0022C9A8__game) and writes addresses
as RAC1_ADDR(0x...u) and RAC1_LADDR(NN, 0x...u); `#if !RAC1_OMIT(name)`
leaves a function out where the table says RAC1_OMIT_name. MAP is the map
versionmap.py builds from games/rac1's version to this one, GEN this
version's generated folder (port/build/<preset>/games/<id>/gen).

Without a map, `check` needs nothing of a disc: it is the test the port's
ctest runs (no entry missing, none unused). The map only adds the comparison
with what the two programs' code says. An entry the map does not agree with
is reported, never changed: the tables' hand-checked entries win, and the
reason belongs in a comment at the entry.
"""
import argparse
import json
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

MARK = "/* ---- generated below ---- */"
NAME = re.compile(r"\b(func_(?:L(\d\d)_)?([0-9A-F]{8})(?:__game)?)\b")
ADDR = re.compile(r"\bRAC1_ADDR\(\s*(0x[0-9A-F]{8}u)\s*\)")
LADDR = re.compile(r"\bRAC1_LADDR\(\s*(\d\d)\s*,\s*(0x[0-9A-F]{8}u)\s*\)")
LITERAL = re.compile(r"\b0x00([0-9A-Fa-f]{6})[uU]?\b")
OMIT_IF = re.compile(r"^\s*#\s*if\s+!\s*RAC1_OMIT\(\s*(\w+)\s*\)\s*$")
DEFINE = re.compile(r"^#define (\w+) (.+?)\s*(?:/\*.*\*/)?$")


def strip(text: str) -> str:
    """TEXT without comments and string literals (lines kept)."""
    out, i, n = [], 0, len(text)
    while i < n:
        if text.startswith("/*", i):
            j = text.find("*/", i + 2)
            j = n if j < 0 else j + 2
            out.append(re.sub(r"[^\n]", " ", text[i:j]))
            i = j
        elif text.startswith("//", i):
            j = text.find("\n", i)
            j = n if j < 0 else j
            i = j
        elif text[i] == '"':
            j = i + 1
            while j < n and text[j] != '"':
                j += 2 if text[j] == "\\" else 1
            out.append('""')
            i = j + 1
        else:
            out.append(text[i])
            i += 1
    return "".join(out)


def is_key(name: str) -> bool:
    """A name or an address of the shared C (not RAC1_OMIT_..., RAC1_TYPE_..., the header guard)."""
    return NAME.fullmatch(name) is not None or name.startswith("RAC1_ADDR_")


class Table:
    """A version's rac1_version.h: the hand-written part above MARK, the generated entries below."""

    def __init__(self, path: Path):
        self.path = path
        text = path.read_text()
        self.head, _, tail = text.partition(MARK)
        self.has_mark = bool(_)
        self.hand = self._defines(self.head)
        self.generated = self._defines(tail) if self.has_mark else {}
        # Entries written by hand above MARK (with the reason, where the map cannot pair them) count
        # as the generated ones do; `write` leaves them where they are.
        self.entries = {**self.generated, **{k: v for k, v in self.hand.items() if is_key(k)}}

    @staticmethod
    def _defines(text: str) -> dict[str, str]:
        found = {}
        for line in text.splitlines():
            m = DEFINE.match(line.strip())
            if m and "(" not in m.group(1) and not m.group(1).startswith("OPENRAC_"):
                found[m.group(1)] = m.group(2).strip()
        return found

    def omitted(self) -> set[str]:
        return {k[len("RAC1_OMIT_"):] for k, v in self.hand.items() if k.startswith("RAC1_OMIT_") and v == "1"}

    def write(self, entries: dict[str, str]) -> None:
        entries = {k: v for k, v in entries.items() if k not in self.hand}
        names = sorted(k for k in entries if not k.startswith("RAC1_"))
        addrs = sorted(k for k in entries if k.startswith("RAC1_ADDR_"))
        out = [self.head.rstrip("\n"), MARK, "",
               "/* Functions: games/rac1's name, this version's. */"]
        out += [f"#define {k} {entries[k]}" for k in names]
        out += ["", "/* Addresses: games/rac1's, this version's (RAC1_LADDR: in level NN's program). */",
                "#define RAC1_ADDR(a) RAC1_ADDR_##a", "#define RAC1_LADDR(n, a) RAC1_ADDR_L##n##_##a"]
        out += [f"#define {k} {entries[k]}" for k in addrs]
        out += ["", "#endif /* OPENRAC_RAC1_VERSION_H */", ""]
        self.path.write_text("\n".join(out))


def shared_dirs(version: Path) -> list[Path]:
    cfg = json.loads((version / "hostgen.json").read_text())
    return [(version / d).resolve() for d in cfg.get("host_shared", [])]


def used(sources: list[Path], omitted: set[str]) -> tuple[dict[str, set[str]], list[str]]:
    """The keys the shared C uses in this version (key -> files), and literals left unwrapped."""
    keys: dict[str, set[str]] = {}
    loose = []
    for path in sources:
        lines = strip(path.read_text()).splitlines()
        skip = [False]
        for no, line in enumerate(lines, 1):
            m = OMIT_IF.match(line)
            if m:
                skip.append(skip[-1] or m.group(1) in omitted)
                continue
            if re.match(r"^\s*#\s*if", line):
                skip.append(skip[-1])
                continue
            if re.match(r"^\s*#\s*endif", line):
                skip.pop()
                continue
            if skip[-1] or re.match(r"^\s*#\s*include", line):
                continue
            for m in NAME.finditer(line):
                keys.setdefault(m.group(1), set()).add(path.name)
            for m in ADDR.finditer(line):
                keys.setdefault(f"RAC1_ADDR_{m.group(1)}", set()).add(path.name)
            for m in LADDR.finditer(line):
                keys.setdefault(f"RAC1_ADDR_L{m.group(1)}_{m.group(2)}", set()).add(path.name)
            bare = LADDR.sub("", ADDR.sub("", line))
            for m in LITERAL.finditer(bare):
                if 0x100000 <= int(m.group(1), 16) < 0x2000000:
                    loose.append(f"{path.name}:{no}: {m.group(0)} outside RAC1_ADDR")
    return keys, loose


def expected(key: str, carrier) -> tuple[str | None, str]:
    """(value, kind) the map gives for KEY, or (None, why)."""
    from carry import cname
    m = re.fullmatch(r"RAC1_ADDR_(?:L(\d\d)_)?0x([0-9A-F]{8})u", key)
    if m:
        program = f"level:{m.group(1)}" if m.group(1) else "boot"
        address = int(m.group(2), 16)
        hit = carrier.map.data(address, program)
        fn = carrier.map.function(address, program)
        if fn is not None and (hit is None or hit[1] != "exact"):
            hit = (fn[0], "code " + fn[1])
        if hit is None:
            return None, "no place found"
        return f"0x{hit[0]:08X}u", hit[1]
    suffix = "__game" if key.endswith("__game") else ""
    there, kind = carrier.symbol(key[: len(key) - len(suffix)])
    if there is None:
        return None, kind
    return cname(there) + suffix, kind


def carrier_of(args):
    if not args.map:
        return None
    from carry import Carrier
    version = Path(args.version)
    cfg = json.loads((version / "hostgen.json").read_text())
    root = Path(__file__).resolve().parents[3]
    source = root / cfg["source"]
    places = source / cfg["places"] if cfg.get("places") else None
    splits = version / cfg["split_places"] if cfg.get("split_places") else None
    return Carrier(Path(args.map), Path(args.gen), source / "src" if source.exists() else None, places, splits)


def gather(args):
    version = Path(args.version)
    table = Table(version / "host" / "rac1_version.h")
    sources = sorted(p for d in shared_dirs(version) for p in d.glob("*.[ch]"))
    keys, loose = used(sources, table.omitted())
    return version, table, keys, loose


def cmd_check(args) -> int:
    version, table, keys, loose = gather(args)
    problems = 0
    if not table.has_mark:
        # The version's own names are games/rac1's (RAC1_ADDR is the address itself).
        if "RAC1_ADDR" not in table.path.read_text():
            print(f"{table.path}: no RAC1_ADDR")
            return 1
        for line in loose:
            print(f"loose: {line}")
        print(f"{version.name}: games/rac1's own names; {len(keys)} used, {len(loose)} loose literals")
        return 1 if loose else 0
    for key in sorted(set(keys) - set(table.entries)):
        print(f"missing: {key} (used in {', '.join(sorted(keys[key]))})")
        problems += 1
    for key in sorted(set(table.entries) - set(keys)):
        print(f"unused: {key}")
        problems += 1
    for line in loose:
        print(f"loose: {line}")
        problems += 1
    carrier = carrier_of(args)
    disagree = 0
    if carrier is not None:
        for key in sorted(set(keys) & set(table.entries)):
            value, kind = expected(key, carrier)
            have = table.entries[key]
            if value is None:
                print(f"map: {key} = {have}: the map has nothing ({kind})")
                disagree += 1
            elif value != have:
                print(f"map: {key} = {have}, the map says {value} ({kind})")
                disagree += 1
            elif kind not in ("same", "exact", "code same") and args.verbose:
                print(f"map: {key} = {have} ({kind})")
    print(f"{version.name}: {len(keys)} names and addresses used, {len(table.entries)} entries, "
          f"{problems} problems" + (f", {disagree} not as the map says" if carrier is not None else ""))
    return 1 if problems else 0


def cmd_write(args) -> int:
    version, table, keys, loose = gather(args)
    if not table.has_mark:
        print(f"{table.path}: no generated part ({MARK!r}); nothing to write")
        return 1
    carrier = carrier_of(args)
    entries = {k: v for k, v in table.entries.items() if k in keys}
    for key in sorted(set(keys) - set(entries)):
        value, kind = expected(key, carrier)
        if value is None:
            print(f"left out: {key}: {kind} (write it by hand)")
            continue
        entries[key] = value
        print(f"new: {key} = {value} ({kind})")
    for key in sorted(set(table.entries) - set(keys)):
        print(f"dropped: {key} = {table.entries[key]}" + (" (by hand: remove it there)" if key in table.hand else ""))
    for line in loose:
        print(f"loose: {line}")
    table.write(entries)
    return 0


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    sub = ap.add_subparsers(dest="cmd", required=True)
    for name in ("check", "write"):
        p = sub.add_parser(name)
        p.add_argument("version", help="port/game/<id>")
        p.add_argument("--map", help="versionmap.py's map from games/rac1's version to this one")
        p.add_argument("--gen", help="this version's generated folder (port/build/<preset>/games/<id>/gen)")
        p.add_argument("--verbose", action="store_true", help="also list entries that rest on near pairs")
    args = ap.parse_args()
    if args.cmd == "write" and not (args.map and args.gen):
        ap.error("write needs --map and --gen")
    if args.map and not args.gen:
        ap.error("--map needs --gen")
    return cmd_check(args) if args.cmd == "check" else cmd_write(args)


if __name__ == "__main__":
    sys.exit(main())
