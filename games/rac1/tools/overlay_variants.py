#!/usr/bin/env python3
"""
Variants (config/overlays/variants.tsv, docs/OVERLAYS.md "Variants"): level
functions with the same instructions as another function except for a
constant, a float or a struct offset.

  python3 tools/overlay_variants.py stubs    # INCLUDE_ASM stubs for new variants
  bash tools/docker/run.sh python tools/overlay_variants.py clone [name | name=parent ...]

`stubs` adds a stub for every catalogued shared or level function that
src/overlays/ doesn't have yet, in address order: after the function
before it in its level, in the directory of its kind (shared/ or
lNN_<planet>/, tools/levels.py).
A function that a neighbour branches into goes in that neighbour's file.

`clone` writes a variant's C from its parent's, with no model: the
parent's definition with the variant's name, the symbols its assembly
names in place of the parent's, and the numbers that differ between the
two functions' instructions replaced in the C. Each candidate goes through
tools/try_func.py (the strict overlay check); an EXACT one replaces the
stub. Variants whose parent isn't matched, or whose difference has no
literal in the C (a struct field), are left for a worker. name=parent
clones from a parent variants.tsv doesn't list, such as a relative at
100% in config/overlays/families.tsv.
"""
from __future__ import annotations
import itertools
import json
import re
import struct
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "tools"))
import claims  # noqa: E402
import levels  # noqa: E402

SRC = ROOT / "src/overlays"
ASM = ROOT / "asm/overlays"
CATALOGUE = ROOT / "config/overlays/functions.tsv"
VARIANTS = ROOT / "config/overlays/variants.tsv"
DUMP = ROOT / "baserom/overlays"
TRY = ROOT / "build-sn/try"
HEADER = ("/* Functions of {where} added after the first split; stubs added by tools/overlay_variants.py, "
          "replaced by C as functions are matched. */\n#include \"common.h\"\n#include \"include_asm.h\"\n\n")
STUB = re.compile(r'^\s*(?:INCLUDE_ASM|LINKER_REMNANT)\([^)]*\b(func_L\d\d_[0-9A-F]{8})\);')
DEF = re.compile(r"^(?!extern\b)[A-Za-z_].*?\b(func_L\d\d_[0-9A-F]{8})\s*\(")
SYMBOL = re.compile(r"\b(?:func_|D_|jtbl_)(?:L\d\d_)?[0-9A-F]{8}\b")
NUMBER = re.compile(r"(?<![\w.])(0[xX][0-9A-Fa-f]+|\d+\.\d*(?:[eE][-+]?\d+)?[fF]?|\d+)(?![\w.])")
MAX_TRIES = 12


def rows(path: Path) -> list[list[str]]:
    return [l.split("\t") for l in path.read_text().splitlines() if l and not l.startswith("#")]


def spans(path: Path) -> dict[str, tuple[int, int, bool]]:
    """name -> (first line, last line, is_c) of each overlay function in PATH."""
    lines = path.read_text().splitlines()
    out, i = {}, 0
    while i < len(lines):
        m = STUB.match(lines[i])
        if m:
            out[m.group(1)] = (i, i, False)
        else:
            m = DEF.match(lines[i])
            if m and not lines[i].rstrip().endswith(";"):
                depth, seen, j = 0, False, i
                while j < len(lines):
                    depth += lines[j].count("{") - lines[j].count("}")
                    seen = seen or "{" in lines[j]
                    if seen and depth == 0:
                        break
                    j += 1
                out[m.group(1)] = (i, j, True)
                i = j
        i += 1
    return out


def index() -> dict[str, tuple[Path, int, int, bool]]:
    return {n: (p, *s) for p in sorted(SRC.glob("*/*.c")) for n, s in spans(p).items()}


def branches_to(name: str, target: str) -> bool:
    """NAME's assembly branches (not calls) into TARGET: the two must share a file."""
    path = ASM / f"{name}.s"
    return path.exists() and any(target in l and " jal " not in l for l in path.read_text().splitlines())


def stubs() -> None:
    have = index()
    added = 0
    new = sorted(name for name, kind, *_ in rows(CATALOGUE) if kind != "exe" and name not in have)
    kinds = {r[0]: r[1] for r in rows(CATALOGUE)}
    for name in new:
        line = f'INCLUDE_ASM("asm/overlays", {name});'
        home = "shared" if kinds[name] == "shared" else levels.dirname(int(name[6:8]))
        level = sorted(n for n in have if n[:8] == name[:8])
        before = [n for n in level if n < name]
        after = [n for n in level if n > name]
        # Address order within the level is link order. A function another
        # one branches into has to stay in that one's file; otherwise keep
        # to the directory of its kind.
        if before and branches_to(before[-1], name):
            path, at = have[before[-1]][0], have[before[-1]][2] + 1
        elif after and branches_to(name, after[0]):
            path, at = have[after[0]][0], have[after[0]][1]
        else:
            near = [n for n in before if have[n][0].parent.name == home]
            if near:
                path, at = have[near[-1]][0], have[near[-1]][2] + 1
            else:
                path, at = SRC / home / "variants.c", None
        if at is None:
            where = "common level code" if home == "shared" else f"level {name[6:8]}"
            lines = (path.read_text() if path.exists() else HEADER.format(where=where)).splitlines()
            lines.append(line)
        else:
            lines = path.read_text().splitlines()
            lines.insert(at, line)
        took = claims.lock("stubs")
        try:
            path.write_text("\n".join(lines) + "\n")
        finally:
            if took:
                claims.unlock("stubs")
        have = index()
        added += 1
    print(f"{added} stubs added")


def function_words(name: str, places: dict) -> tuple[int, ...]:
    level, addr, size = places[name]
    text = next(r for r in json.loads((DUMP / f"level_{level:02d}/manifest.json").read_text())["records"]
                if r["name"] == "text")
    data = (DUMP / f"level_{level:02d}/text.bin").read_bytes()[addr - text["address"]:][:size]
    return struct.unpack(f"<{len(data) // 4}I", data)


def asm_symbols(name: str) -> list[str]:
    path = ASM / f"{name}.s"
    if not path.exists():
        return []
    return [s for l in path.read_text().splitlines() if "/*" in l
            for s in SYMBOL.findall(l.split("*/", 1)[-1])]


def number_changes(a: tuple, b: tuple) -> list[tuple[float, float]] | None:
    """(old, new) values of the immediates that differ, each as the numbers
    C could spell them with; None when the functions differ in more than
    immediates."""
    out = []
    for x, y in zip(a, b):
        if x == y or x >> 26 in (2, 3):
            continue
        if x >> 16 != y >> 16:
            return None
        i, j = x & 0xFFFF, y & 0xFFFF
        if x >> 26 == 0x0F:
            fo, fn = (struct.unpack("<f", struct.pack("<I", v << 16))[0] for v in (i, j))
            out.append((fo, fn))
            out.append((i << 16, j << 16))
            continue
        out.append((i, j))
        si, sj = (v - 0x10000 if v & 0x8000 else v for v in (i, j))
        if (si, sj) != (i, j):
            out.append((si, sj))
            out.append((-si, -sj))
    return out


def value(token: str) -> float:
    if token[:2].lower() == "0x":
        return int(token, 16)
    return float(token.rstrip("fF")) if "." in token else int(token)


def spell(new: float, like: str) -> str:
    if like[:2].lower() == "0x":
        return f"0x{int(new):X}"
    if "." in like:
        return repr(float(new)) + ("f" if like[-1] in "fF" else "")
    return str(int(new))


def candidates(body: str, changes: list[tuple[float, float]]):
    """BODY with the changed numbers replaced: every literal that equals an
    old value first, then smaller sets of them."""
    if not changes:
        yield body
        return
    new_for = {}
    for old, new in changes:
        if old >= 0:
            new_for.setdefault(old, new)
    hits = [(m.start(1), m.end(1), spell(new_for[value(m.group(1))], m.group(1)))
            for m in NUMBER.finditer(body) if value(m.group(1)) in new_for
            and (new_for[value(m.group(1))] >= 0)]
    if not hits:
        return
    sets = [tuple(range(len(hits)))]
    for k in range(1, len(hits)):
        sets += itertools.combinations(range(len(hits)), k)
    for chosen in sets[:MAX_TRIES]:
        out, at = [], 0
        for i in chosen:
            s, e, text = hits[i]
            out += [body[at:s], text]
            at = e
        yield "".join(out) + body[at:]


def clone(only: list[str]) -> None:
    have = index()
    cat = rows(CATALOGUE)
    places = {r[0]: (int(r[5].split(",")[0][:2]), int(r[5].split(",")[0][3:], 16), int(r[2])) for r in cat}
    exact, tried = [], 0
    names_h = ROOT / "include/names.h"
    readable = dict(re.findall(r"^#define\s+(\w+)\s+((?:func_|D_)\w+)\s*$", names_h.read_text(), flags=re.M)) \
        if names_h.exists() else {}
    # NAME=PARENT clones from a parent variants.tsv doesn't list (a close
    # relative in config/overlays/families.tsv, say); plain names keep theirs.
    pairs = dict(a.split("=", 1) for a in only if "=" in a)
    only = [a.split("=", 1)[0] for a in only]
    listed = [(n, pairs.pop(n, p), k, s) for n, p, k, s in rows(VARIANTS)]
    for name, par, kind, size in listed + [(n, p, "", "") for n, p in pairs.items()]:
        if only and name not in only:
            continue
        if name not in have or have[name][3] or par not in have or not have[par][3]:
            continue
        if not claims.claim("clone", name):         # another agent is on it (tools/claims.py)
            continue
        ppath, first, last, _ = have[par]
        text = ppath.read_text()
        body = "\n".join(text.splitlines()[first:last + 1])
        changes = number_changes(function_words(par, places), function_words(name, places))
        old_syms, new_syms = asm_symbols(par), asm_symbols(name)
        if changes is None or len(old_syms) != len(new_syms):
            print(f"{name}: differs from {par} in more than numbers; skipped")
            claims.release("clone", name)
            continue
        rename = {o: n for o, n in zip(old_syms, new_syms) if o != n}
        rename[par] = name
        # Bodies may use include/names.h's readable names: back to the
        # address names first, so a symbol that differs gets renamed.
        body = re.sub(r"\b[A-Za-z_]\w*\b", lambda m: readable.get(m.group(0), m.group(0)), body)
        body = re.sub("|".join(rf"\b{o}\b" for o in rename), lambda m: rename[m.group(0)], body)
        # Declarations the body needs, from the parent's file, unless the
        # variant's own file (it may be another one) already declares the name.
        vtext = have[name][0].read_text()
        back = {n: o for o, n in rename.items()}
        externs = []
        for sym in dict.fromkeys(re.findall(r"\b(?:func_|D_|jtbl_)\w+", body)):
            if sym == name or re.search(rf"^(?!INCLUDE_ASM)(?:extern\b[^;\n]*|[A-Za-z_][^;\n]*)\b{sym}\b", vtext, flags=re.M):
                continue
            old_sym = back.get(sym, sym)
            for decl in re.findall(rf"^extern[^;\n]*\b{old_sym}\b[^;\n]*;", text, flags=re.M)[:1]:
                externs.append(re.sub(rf"\b{old_sym}\b", sym, decl))
        work = TRY / name
        work.mkdir(parents=True, exist_ok=True)
        verdict = "no literal for the changed number"
        for k, cand in enumerate(candidates(body, changes)):
            source = "\n".join(externs + [cand]) + "\n"
            path = work / f"clone{k}.c"
            path.write_text(source)
            run = subprocess.run([sys.executable, "tools/try_func.py", name, str(path.relative_to(ROOT)),
                                  "--no-budget"], cwd=ROOT, capture_output=True, text=True)
            tried += 1
            verdict = (run.stdout.strip().splitlines() or ["?"])[-1].split(":", 1)[-1].strip()
            if verdict.startswith("EXACT"):
                took = claims.lock("clone")
                try:
                    have = index()
                    vpath, vfirst, vlast, _ = have[name]
                    lines = vpath.read_text().splitlines()
                    lines[vfirst:vlast + 1] = source.rstrip("\n").splitlines()
                    vpath.write_text("\n".join(lines) + "\n")
                finally:
                    if took:
                        claims.unlock("clone")
                have = index()
                exact.append(name)
                break
        else:
            claims.release("clone", name)
        print(f"{name} (from {par}): {verdict}")
    print(f"{len(exact)} cloned exactly, {tried} try_func runs")


if __name__ == "__main__":
    if sys.argv[1:2] == ["stubs"]:
        stubs()
    elif sys.argv[1:2] == ["clone"]:
        clone(sys.argv[2:])
    else:
        sys.exit(__doc__)
