#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""abicheck: calls through a code address whose type is not the type of the function there.

    python3 port/tools/hostgen/abicheck.py port/build/release/games/rac1-ntsc/gen [--all]

hostgen writes a direct call as a call to the callee's definition (game_protos.h), so the host
compiler checks it. A call through a code address (GFN: a folded name's place, a level's copy,
another level's function) is a cast to the type the call site gives, and the function the
address finds was compiled with its own. On the EE, System V x86-64 and AArch64 integers and
floats travel in registers of their own, each kind in order, so `f(float, void *, void *)`
reaching `f(void *, void *, float)` works there; on Windows x64 an argument's position decides
its register whatever its type, and the callee reads its pointers from registers nobody set
(rac1/ntsc's Rilgar, 0.8.0: a write at 0x3FBE0000). A float result read where an integer is
returned, or the other way round, is wrong on every host.

This reads what hostgen wrote: the functions at each code address (functions.c), their
definitions (game_protos.h), and every GFN in the generated C whose address is known when it is
written (OPENRAC_LCODE, OPENRAC_CODE, a literal, openrac_folded_call's places,
openrac_call_in_copy's callee). For each, the call's parameter at every position both have must
be of the definition's kind (integer or pointer, float, double), and a result the call reads
must be of the kind the definition returns. It exits 1 when one is not. Calls through an address
the program computes (tables of update functions) are not known here and are not checked; the
assembly's translation calls those through each function's thunk where position decides
(guest.h, OPENRAC_REGS_THUNKS).
"""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

INT_WORDS = {"int", "unsigned", "signed", "char", "short", "long", "gaddr", "uint8_t", "uint16_t", "uint32_t",
             "uint64_t", "int8_t", "int16_t", "int32_t", "int64_t", "openrac_u64", "openrac_s64", "size_t",
             "_Bool", "s8", "u8", "s16", "u16", "s32", "u32", "s64", "u64", "openrac_ti", "openrac_uti",
             "__int128"}

PROTO = re.compile(r"^(?P<ret>[\w\s\*]+?)\s*\b(?P<name>\w+)\((?P<params>[^()]*)\);\s*$")
ENTRY = re.compile(r'\{0x([0-9A-Fa-f]+)u, \(openrac_host_fn\)(\w+), "\w+"\}')
TABLE = re.compile(r"static const openrac_fn_entry k_(exe|overlay_(\d+))\[\]")
FN_TYPE = re.compile(r"^(?P<ret>.+?)\s*\(\*\)\((?P<params>.*)\)$", re.S)
DEFINITION = re.compile(r"^(?:static\s+)?[\w\s\*]+?\b(\w+)\([^;]*\)\s*$")
HEX = r"(?:\(gaddr\))?\(?0x([0-9A-Fa-f]+)u?\)?"


def kind(t: str) -> str:
    """The register kind of a parameter or result type as the host passes it: i, f, d, v (void),
    r (a record by value), ? (unknown)."""
    t = re.sub(r"\b(const|volatile|restrict|__restrict)\b", " ", t).strip()
    if not t or t == "void":
        return "v"
    if "*" in t:
        return "i"
    if t == "float" or t == "f32":
        return "f"
    if t in ("double", "f64", "long double"):
        return "d"
    if t.startswith(("struct ", "union ")):
        return "r"
    if all(w in INT_WORDS for w in t.split()):
        return "i"
    return "?"


def split_params(text: str) -> list[str]:
    text = text.strip()
    if text in ("", "void"):
        return []
    out, depth, cur = [], 0, ""
    for ch in text:
        if ch == "," and depth == 0:
            out.append(cur)
            cur = ""
            continue
        depth += ch in "(["
        depth -= ch in ")]"
        cur += ch
    out.append(cur)
    params = []
    for p in out:
        p = p.strip()
        if p == "...":
            params.append("...")
            continue
        # Drop a parameter's name (game_protos.h: `gaddr a0`).
        m = re.match(r"^(.*?[\s\*])(\w+)$", p)
        if m and m.group(2) not in INT_WORDS and m.group(2) not in ("float", "double", "void"):
            p = m.group(1)
        params.append(p.strip())
    return params


def read_protos(path: Path) -> dict[str, tuple[str, list[str]]]:
    protos = {}
    for line in path.read_text().splitlines():
        m = PROTO.match(line)
        if m and not line.startswith(("#", "typedef", "/*")):
            protos[m.group("name")] = (m.group("ret").strip(), split_params(m.group("params")))
    return protos


def read_tables(path: Path) -> tuple[dict[tuple[int, int], list[str]], dict[str, set[int]]]:
    """(overlay, address) -> the functions there; and each function's overlays."""
    at: dict[tuple[int, int], list[str]] = {}
    where: dict[str, set[int]] = {}
    overlay = None
    for line in path.read_text().splitlines():
        t = TABLE.search(line)
        if t:
            overlay = -1 if t.group(1) == "exe" else int(t.group(2))
            continue
        if line.startswith("}"):
            overlay = None
            continue
        e = ENTRY.search(line)
        if e and overlay is not None:
            addr = int(e.group(1), 16)
            at.setdefault((overlay, addr), [])
            if e.group(2) not in at[(overlay, addr)]:
                at[(overlay, addr)].append(e.group(2))
            where.setdefault(e.group(2), set()).add(overlay)
    return at, where


def _balanced(text: str, start: int) -> int:
    """The index just past the parenthesis closing the one at text[start]."""
    depth = 0
    i = start
    n = len(text)
    while i < n:
        ch = text[i]
        if ch == "(":
            depth += 1
        elif ch == ")":
            depth -= 1
            if depth == 0:
                return i + 1
        elif ch == '"':
            i += 1
            while i < n and text[i] != '"':
                i += 2 if text[i] == "\\" else 1
        i += 1
    return -1


def _split_top(text: str) -> list[str]:
    out, depth, cur = [], 0, ""
    for ch in text:
        if ch == "," and depth == 0:
            out.append(cur.strip())
            cur = ""
            continue
        depth += ch in "([{"
        depth -= ch in ")]}"
        cur += ch
    out.append(cur.strip())
    return out


def targets(expr: str, own: set[int]) -> list[tuple[int, int]] | None:
    """The places a GFN's address can be, or None when the program computes it."""
    expr = expr.strip()
    while expr.startswith("(") and _balanced(expr, 0) == len(expr):
        expr = expr[1:-1].strip()
    m = re.fullmatch(r"OPENRAC_LCODE\((\d+),\s*" + HEX + r"\)", expr)
    if m:
        return [(int(m.group(1)), int(m.group(2), 16))]
    m = re.fullmatch(r"OPENRAC_CODE\(" + HEX + r"\)", expr)
    if m:
        return [(-1, int(m.group(1), 16))]
    m = re.fullmatch(HEX, expr)
    if m:
        addr = int(m.group(1), 16)
        return [(o, addr) for o in sorted(own or {-1})]
    m = re.match(r"openrac_folded_call\(\(const uint32_t\[\]\)\{([^}]*)\}", expr)
    if m:
        out = []
        for v in m.group(1).split(","):
            v = int(v.strip().rstrip("uU"), 16)
            out.append(((v >> 24) - 1, v & 0xFFFFFF))
        return out
    m = re.fullmatch(r"openrac_call_in_copy\((.*)\)", expr, re.S)
    if m:
        a = _split_top(m.group(1))
        if len(a) == 5 and a[0].lstrip("-").isdigit():
            return [(int(a[0]), int(a[4].rstrip("uU"), 16))]
    return None


def compare(call_ret: str, call_params: list[str], def_ret: str, def_params: list[str],
            used: bool) -> list[str]:
    """What differs between a call's type and the definition it reaches, as the host passes them."""
    what = []
    cp = [kind(p) for p in call_params if p != "..."]
    dp = [kind(p) for p in def_params if p != "..."]
    for k in range(min(len(cp), len(dp))):
        if cp[k] != dp[k] and "?" not in (cp[k], dp[k]):
            what.append(f"argument {k + 1} is {'an integer' if cp[k] == 'i' else cp[k]} for "
                        f"{'an integer' if dp[k] == 'i' else dp[k]}")
            break
    rc, dc = kind(call_ret), kind(def_ret)
    # A result the callee does not return is whatever the register held, on the EE as on any host
    # (hostgen's report); a result of the other kind is in the other register file.
    if used and rc in ("i", "f", "d") and dc in ("i", "f", "d") and rc != dc:
        what.append(f"reads a {call_ret} result, returns {def_ret}")
    return what


def check(gen: Path, include_all: bool = False) -> tuple[list[str], dict]:
    protos = read_protos(gen / "game_protos.h")
    at, where = read_tables(gen / "functions.c")
    problems: list[str] = []
    stats = {"calls": 0, "known": 0, "unknown": 0}
    units = gen / "units.txt"
    if units.exists():
        # What the build compiles (a file hostgen no longer writes may still be there).
        paths = [gen / u.strip() for u in units.read_text().splitlines() if u.strip()]
        paths += [gen / n for n in ("stubs.c", "libraries.c") if (gen / n).exists()]
    else:
        paths = sorted(gen.rglob("*.c"))
    for path in paths:
        rel = path.relative_to(gen)
        if rel.parts[0] == "prep" or rel.name in ("functions.c",) or not path.exists():
            continue
        text = path.read_text(errors="replace")
        if "GFN(" not in text:
            continue
        # Each definition's start, for the overlays a literal address is in.
        starts = []
        for m in re.finditer(r"^(?:static\s+)?[A-Za-z_][\w\s\*]*?\b(\w+)\([^;{]*\)\s*\n\{", text, re.M):
            starts.append((m.start(), m.group(1)))
        for m in re.finditer(r"\bGFN\(", text):
            if text[max(0, m.start() - 8):m.start()].endswith("define "):
                continue
            end = _balanced(text, m.end() - 1)
            if end < 0:
                continue
            inside = text[m.end():end - 1]
            parts = _split_top(inside)
            if len(parts) != 2:
                continue
            ftype, expr = parts
            ft = FN_TYPE.match(ftype)
            if not ft:
                continue
            params = split_params(ft.group("params"))
            if params.count("openrac_u64") == 8:
                continue  # the assembly's registers as they are (thunked where position decides)
            stats["calls"] += 1
            caller = None
            for s, name in starts:
                if s < m.start():
                    caller = name
                else:
                    break
            places = targets(expr, where.get(caller or "", set()))
            if places is None:
                stats["unknown"] += 1
                continue
            stats["known"] += 1
            # A result the call uses: anything but a statement of its own.
            close = _balanced(text, end) if text[end:end + 1] == "(" else -1
            before = text[:m.start()].rstrip()
            used = not (before.endswith((";", "{", "}", ":")) and close > 0
                        and text[close:].lstrip().startswith(";"))
            line = text.count("\n", 0, m.start()) + 1
            for place in places:
                # The runtime keeps the first function the table has at an address (guest.cpp).
                for fn in at.get(place, [])[:None if include_all else 1]:
                    if fn not in protos:
                        continue
                    dret, dparams = protos[fn]
                    what = compare(ft.group("ret"), params, dret, dparams, used)
                    if what:
                        problems.append(f"{rel}:{line}: {caller or '?'} calls {fn} at "
                                        f"{'exe' if place[0] < 0 else f'L{place[0]:02d}'}:{place[1]:#010x} as "
                                        f"{ftype}: {'; '.join(what)}")
                if not include_all:
                    break
    return problems, stats


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("gen", type=Path, help="a game's generated directory (port/build/<preset>/games/<id>/gen)")
    ap.add_argument("--all", action="store_true",
                    help="check every place a folded call can reach, not only the first")
    args = ap.parse_args(argv)
    if not (args.gen / "functions.c").exists():
        print(f"abicheck: no functions.c in {args.gen} (not generated yet)", file=sys.stderr)
        return 2
    problems, stats = check(args.gen, args.all)
    for p in problems:
        print(p)
    print(f"abicheck: {stats['calls']} calls through a code address, {stats['known']} to a known one, "
          f"{len(problems)} whose type is not the definition's", file=sys.stderr)
    return 1 if problems else 0


if __name__ == "__main__":
    sys.exit(main())
