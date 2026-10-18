#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""
Carries the C one version of a game runs to every function another version
still has only as assembly, mechanically, through the version map.

  carry_functions.py --target port/game/rac1-ntsc/hostgen.json --source port/game/rac1-pal/hostgen.json \\
      --to-map build/versionmap/rac1-pal--rac1-ntsc.json --back-map build/versionmap/rac1-ntsc--rac1-pal.json \\
      --gen port/build/release/games/rac1-ntsc/gen --source-gen port/build/release/games/rac1-pal/gen \\
      --out DIR [--write]

For each function of the target with no C (an assembly stub of its source
that nothing replaces, or a name hostgen reports without C), it finds the
source version's counterpart through the map (`same`, `near`, or none):

- `same`: the source's C for it (the matched C, the port's hand C, or the
  candidate its functional table names, in the order the source's port reads
  them) is preprocessed in the source's own file, cut down to the function and
  the declarations, types and file-scope helpers it needs, its types and
  helpers renamed apart (it lands at the end of a target file with types of
  its own), and carried to the target's names and addresses with carry.py.
  The result is checked with Clang where hostgen will put it. With --write it
  goes to the target's hand/functional/<name>.c with a row in hand/hand.tsv;
- `near`: nothing is written; it is listed (near.json) with the source's C;
- none: it is listed for writing from the assembly (write.json).

Anything that cannot be carried is listed with the reason (skipped.json):
pieces and linker remnants, handwritten routines with no C, names with no
function behind them, files hostgen leaves out, C that does not translate.
--write also drops the hand.tsv rows that no longer replace anything (the
target's decompilation has its own C since), and their files.

Nothing carried is proven by being carried.
"""
import argparse
import json
import os
import re
import subprocess
import sys
from collections import defaultdict
from concurrent.futures import ThreadPoolExecutor
from dataclasses import dataclass, field
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]
sys.path.insert(0, str(HERE))
sys.path.insert(0, str(ROOT / "port" / "tools" / "hostgen"))
from carry import Carrier, carry_c  # noqa: E402

# ---------------------------------------------------------------------------------------------
# Programs and names

ENTRY = re.compile(r"\{0x([0-9A-F]{8})u, \(openrac_host_fn\)(\w+), \"(\w+)\"\}")
TABLE = re.compile(r"static const openrac_fn_entry k_(exe|overlay_(\d\d))\[\]")
MISSING = re.compile(r'openrac_guest_missing\("(\w+)')
ADDRESS_NAME = re.compile(r"^(?:func|FUN|D)_(?:L(\d\d)_)?([0-9A-Fa-f]{8})(?:_\w*)?$")
STUB = re.compile(r'^[ \t]*(INCLUDE_ASM|ASM_FUNC|LINKER_REMNANT)\(\s*((?:"[^"]*"\s*)+),\s*(\w+)\s*\)', re.M)
DEFINITION = re.compile(r'^[A-Za-z_][\w \t\*]*?[ \t\*](\w+)\s*\([^;{}]*\)\s*\{', re.M)


def program_key(overlay: int) -> str:
    return "boot" if overlay < 0 else f"level:{overlay:02d}"


def overlay_of(program: str) -> int:
    return -1 if program == "boot" else int(program.split(":")[1])


class Gen:
    """A version's generated folder: who is where (functions.c) and who has no C (stubs.c)."""

    def __init__(self, gen: Path):
        self.at: dict[tuple[int, int], list[str]] = defaultdict(list)
        self.where: dict[str, list[tuple[int, int]]] = defaultdict(list)
        overlay = None
        for line in (gen / "functions.c").read_text().splitlines():
            m = TABLE.match(line)
            if m:
                overlay = -1 if m.group(1) == "exe" else int(m.group(2))
                continue
            m = ENTRY.search(line)
            if m and overlay is not None:
                key = (overlay, int(m.group(1), 16))
                if m.group(3) not in self.at[key]:
                    self.at[key].append(m.group(3))
                self.where[m.group(3)].append(key)
        self.missing = list(dict.fromkeys(MISSING.findall((gen / "stubs.c").read_text())))
        self.no_c = set(self.missing)
        report = gen / "report.json"
        self.report = json.loads(report.read_text()) if report.is_file() else {}

    def with_c(self, overlay: int, address: int) -> list[str]:
        """The names at a place that have C; a level place hostgen does not list falls back to the
        executable's (as the port's own lookup does)."""
        names = self.at.get((overlay, address))
        if names is None and overlay >= 0:
            names = self.at.get((-1, address), [])
        return [n for n in names or [] if n not in self.no_c]


def read_tsv(path: Path) -> list[list[str]]:
    if not path.is_file():
        return []
    return [line.split("\t") for line in path.read_text().splitlines() if line.strip() and not line.startswith("#")]


def read_places(path: Path) -> dict[str, tuple[str, int, list[tuple[int, int]]]]:
    """A catalogue (name, kind, size, fingerprint, levels, places): name -> (kind, size, places)."""
    out = {}
    for cols in read_tsv(path):
        if len(cols) > 5:
            out[cols[0]] = (cols[1], int(cols[2]) if cols[2].isdigit() else 0,
                            [(int(p[:2]), int(p[3:], 16)) for p in cols[5].split(",") if p])
    return out


def read_symbols(paths: list[Path]) -> dict[str, int]:
    out = {}
    for path in paths:
        if path.is_file():
            for line in path.read_text().splitlines():
                m = re.match(r"\s*(\w+)\s*=\s*0x([0-9A-Fa-f]+)", line)
                if m:
                    out.setdefault(m.group(1), int(m.group(2), 16))
    return out


def stubs_of(source: Path, dirs: list[str]) -> dict[str, list[tuple[str, str, str]]]:
    """name -> [(file relative to the decompilation, macro, assembly path)] of every assembly stub."""
    out: dict[str, list[tuple[str, str, str]]] = defaultdict(list)
    for d in dirs:
        for path in sorted((source / d).rglob("*.c")):
            text = path.read_text(errors="replace")
            for m in STUB.finditer(text):
                folder = "".join(re.findall(r'"([^"]*)"', m.group(2)))
                out[m.group(3)].append((path.relative_to(source).as_posix(), m.group(1), folder))
    return out


def definitions_of(source: Path, dirs: list[str], skip: list[str] = ()) -> dict[str, str]:
    """name -> file of every function a decompilation defines in C (its first definition)."""
    out: dict[str, str] = {}
    for d in dirs:
        for path in sorted((source / d).rglob("*.c")):
            rel = path.relative_to(source).as_posix()
            if any(rel == s or rel.startswith(s.rstrip("/") + "/") for s in skip):
                continue
            for name in DEFINITION.findall(path.read_text(errors="replace")):
                out.setdefault(name, rel)
    return out


# ---------------------------------------------------------------------------------------------
# C, cut into its top-level items

KEYWORDS = set("""auto break case char const continue default do double else enum extern float for goto if
inline int long register restrict return short signed sizeof static struct switch typedef union unsigned
void volatile while _Bool asm __asm __asm__ __attribute __attribute__ __inline __inline__ __typeof __typeof__
typeof __extension__ __restrict __restrict__ __const __const__ __volatile __volatile__ __signed __signed__
__label__ __alignof__ __builtin_va_list __builtin_va_arg __builtin_va_start __builtin_va_end __builtin_offsetof""".split())

IDENT = re.compile(r"[A-Za-z_]\w*")


@dataclass
class Item:
    text: str
    kind: str                      # pp, asm, func, decl, empty
    index: int = 0
    names: set = field(default_factory=set)       # what it declares or defines
    refs: set = field(default_factory=set)        # the identifiers it uses
    static: bool = False
    typedef: bool = False
    extern: bool = False
    region: str = "file"           # file, or cand (the candidate's own text)
    func_name: str = ""


def _skip_quoted(text: str, i: int) -> int:
    q = text[i]
    i += 1
    while i < len(text) and text[i] != q:
        i += 2 if text[i] == "\\" else 1
    return i + 1


def code_only(text: str) -> str:
    """TEXT with string and character literals and comments blanked (same length)."""
    out, i = [], 0
    while i < len(text):
        c = text[i]
        if c in "\"'":
            j = _skip_quoted(text, i)
            out.append(c + " " * max(0, j - i - 2) + (c if j - i >= 2 else ""))
            i = j
        elif text.startswith("/*", i):
            j = text.find("*/", i + 2)
            j = len(text) if j < 0 else j + 2
            out.append(re.sub(r"[^\n]", " ", text[i:j]))
            i = j
        elif text.startswith("//", i):
            j = text.find("\n", i)
            j = len(text) if j < 0 else j
            out.append(" " * (j - i))
            i = j
        else:
            out.append(c)
            i += 1
    return "".join(out)


def split_items(text: str) -> list[Item]:
    """The top-level items of preprocessed C, in order: directives, file-scope asm, function
    definitions and declarations (each up to its `;`)."""
    code = code_only(text)
    items, start, depth, i = [], 0, 0, 0
    body_of_function = False

    def emit(end: int, kind: str):
        nonlocal start
        piece = text[start:end]
        if piece.strip():
            items.append(Item(piece, kind))
        start = end

    while i < len(code):
        c = code[i]
        if depth == 0 and c == "#" and code[start:i].strip() == "":
            j = code.find("\n", i)
            j = len(code) if j < 0 else j + 1
            emit(i, "decl") if code[start:i].strip() else None
            start = i
            emit(j, "pp")
            i = j
            continue
        if c in "([":
            depth += 1
        elif c in ")]":
            depth -= 1
        elif c == "{":
            if depth == 0:
                head = code[start:i]
                bare = re.sub(r"__attribute__\s*\(\(.*?\)\)|__asm__\s*\([^)]*\)", "", head, flags=re.S).rstrip()
                body_of_function = bare.endswith(")") and not _top_level_has(head, "=")
            depth += 1
        elif c == "}":
            depth -= 1
            if depth == 0 and body_of_function:
                body_of_function = False
                emit(i + 1, "func")
        elif c == ";" and depth == 0:
            piece = code[start:i]
            emit(i + 1, "asm" if re.match(r"\s*(?:__asm__|asm|__asm)\b", piece) else "decl")
        i += 1
    if text[start:].strip():
        items.append(Item(text[start:], "decl"))
    for n, item in enumerate(items):
        item.index = n
        describe(item)
    return items


def _top_level_has(code: str, ch: str) -> bool:
    depth = 0
    for c in code:
        if c in "([{":
            depth += 1
        elif c in ")]}":
            depth -= 1
        elif c == ch and depth == 0:
            return True
    return False


def _blank_groups(code: str) -> str:
    """Attributes and asm labels blanked (same length): their parentheses are not declarators."""
    out = list(code)
    for m in re.finditer(r"\b(?:__attribute__|__attribute|__asm__|__asm|asm)\s*\(", code):
        depth, j = 0, m.end() - 1
        while j < len(code):
            if code[j] == "(":
                depth += 1
            elif code[j] == ")":
                depth -= 1
                if depth == 0:
                    break
            j += 1
        for k in range(m.start(), min(j + 1, len(code))):
            if out[k] != "\n":
                out[k] = " "
    return "".join(out)


def _without_braces(code: str) -> str:
    """Every top-level {...} emptied, and what each held returned apart."""
    out, inner, depth, buf = [], [], 0, []
    for c in code:
        if c == "{":
            if depth == 0:
                out.append("{")
                buf = []
            else:
                buf.append(c)
            depth += 1
        elif c == "}":
            depth -= 1
            if depth == 0:
                out.append("}")
                inner.append("".join(buf))
            else:
                buf.append(c)
        elif depth:
            buf.append(c)
        else:
            out.append(c)
    return "".join(out), inner


def _split_top(code: str, sep: str = ",") -> list[str]:
    parts, depth, cur = [], 0, []
    for c in code:
        if c in "([{":
            depth += 1
        elif c in ")]}":
            depth -= 1
        if c == sep and depth == 0:
            parts.append("".join(cur))
            cur = []
        else:
            cur.append(c)
    parts.append("".join(cur))
    return parts


def declarator_name(decl: str) -> str | None:
    """The name a declarator declares: `(*name)` for a pointer to a function, else the last
    identifier before its first parenthesis or bracket."""
    decl = decl.split("=")[0] if not _top_level_has(decl, "(") else _split_top(decl, "=")[0]
    m = re.search(r"\(\s*\*+\s*(?:(?:const|volatile|__restrict)\s+)*([A-Za-z_]\w*)", decl)
    first = re.search(r"[(\[]", decl)
    if m and (first is None or m.start() <= first.start()):
        return m.group(1)
    head = decl[:first.start()] if first else decl
    names = [n for n in IDENT.findall(head) if n not in KEYWORDS]
    return names[-1] if names else None


def describe(item: Item) -> None:
    code = code_only(item.text)
    item.refs = {n for n in IDENT.findall(code) if n not in KEYWORDS}
    lead = code.lstrip()
    if item.kind in ("pp", "asm"):
        item.refs = set() if item.kind == "asm" else item.refs
        return
    if not lead.strip(" ;\n\t"):
        item.kind = "empty"
        return
    lead_tokens = IDENT.findall(re.sub(r"__extension__", "", lead[:200]))
    item.static = "static" in lead_tokens[:4]
    item.typedef = "typedef" in lead_tokens[:3]
    item.extern = "extern" in lead_tokens[:3]
    blank = _blank_groups(code)
    # Tags and enumerators it defines.
    for m in re.finditer(r"\b(struct|union|enum)\s+([A-Za-z_]\w*)\s*\{", blank):
        item.names.add(m.group(2))
    for m in re.finditer(r"\benum\b[^{;]*\{([^{}]*)\}", blank):
        for part in m.group(1).split(","):
            n = IDENT.match(part.strip())
            if n:
                item.names.add(n.group(0))
    if item.kind == "func":
        head = blank[:blank.index("{")] if "{" in blank else blank
        item.func_name = declarator_name(head) or ""
        item.names.add(item.func_name)
        return
    flat, _inner = _without_braces(blank)
    m = re.match(r"\s*(?:struct|union|enum)\s+([A-Za-z_]\w*)\s*;\s*$", flat)
    if m:                                          # a tag declared ahead of its definition
        item.names.add(m.group(1))
        return
    if re.fullmatch(r"\s*(?:typedef\s+)?(?:struct|union|enum)\s*(?:[A-Za-z_]\w*)?\s*\{\}\s*;\s*", flat):
        return                                     # a tag's definition alone: no declarator
    flat = flat.rstrip().rstrip(";")
    for n, part in enumerate(_split_top(flat)):
        if n == 0:
            part = re.sub(r"\b(?:struct|union|enum)\s+[A-Za-z_]\w*\s*\{\}", " ", part)
            part = re.sub(r"\b(?:struct|union|enum)\s*\{\}", " ", part)
        name = declarator_name(part)
        if name:
            item.names.add(name)


def is_prototype(item: Item) -> bool:
    """A declaration of functions only (no data, not a pointer to a function)."""
    if item.kind != "decl" or not item.names:
        return False
    flat, _ = _without_braces(_blank_groups(code_only(item.text)))
    return all(re.search(r"\b" + re.escape(n) + r"\s*\(", flat) for n in item.names
               if n not in _tags_and_enumerators(item))


def as_prototype(item: Item) -> str:
    """A function definition's header, as a declaration."""
    code = code_only(item.text)
    blank = _blank_groups(code)
    head = item.text[:blank.index("{")].rstrip()
    return head + ";\n"


def as_extern(item: Item) -> str:
    """A definition of data with external linkage, as a declaration (no initialiser)."""
    text = item.text.strip()
    code = code_only(text)
    depth, start = 0, 0
    pieces = []
    for i, c in enumerate(code):
        if c in "([{":
            depth += 1
        elif c in ")]}":
            depth -= 1
        elif c == "," and depth == 0:
            pieces.append((start, i))
            start = i + 1
    pieces.append((start, len(code)))
    out = []
    for a, b in pieces:
        seg_code, seg = code[a:b], text[a:b]
        eq = None
        depth = 0
        for i, c in enumerate(seg_code):
            if c in "([{":
                depth += 1
            elif c in ")]}":
                depth -= 1
            elif c == "=" and depth == 0:
                eq = i
                break
        if eq is not None:
            seg = seg[:eq].rstrip()
        out.append(seg.rstrip().rstrip(";"))
    text = ",".join(out).strip()
    return ("extern " + text if not re.match(r"extern\b", text) else text) + ";\n"


# ---------------------------------------------------------------------------------------------
# Cutting one function out of its file

MARK = "Carried by port/tools/versionmap/carry_functions.py"
GP_LABEL = re.compile(r'(__asm__\s*\(\s*"[A-Za-z_]\w*?)__gp(")')
SECTION = re.compile(r'__attribute__\s*\(\(\s*section\s*\(\s*"\.s?data"\s*\)\s*\)\)')


def extract(text: str, target: str, cand_marker: str | None = None) -> tuple[str, list[str], list[str]]:
    """The function TARGET of preprocessed TEXT with what it needs: the declarations, types and
    static helpers it uses, transitively, in the file's order. Other functions it uses become
    declarations, data defined here becomes extern. Where the function sits in a candidate's own
    text (after CAND_MARKER's declaration), the candidate's declarations of a name win over the
    file's. Returns (C, the names it defines that are only its own: types, tags, enumerators,
    statics, names with an assembler name of their own; the function's names) or raises LookupError."""
    items = split_items(text)
    if cand_marker:
        seen = False
        for it in items:
            if cand_marker in it.names:
                seen = True
                it.kind = "empty"
            elif seen:
                it.region = "cand"
    # A definition can carry another name, with TARGET as its assembler name.
    label = re.compile(r'__asm__\s*\(\s*"' + re.escape(target) + r'"\s*\)')
    aliases = {target} | {n for it in items if it.kind in ("decl", "func") and label.search(it.text) for n in it.names}
    defs = [it for it in items if it.kind == "func" and it.func_name in aliases]
    if not defs:
        raise LookupError(f"no definition of {target}")
    top = defs[-1]
    providers: dict[str, list[Item]] = defaultdict(list)
    for it in items:
        if it.kind in ("decl", "func"):
            for n in it.names:
                providers[n].append(it)
    need: dict[int, str] = {top.index: "def"}
    todo = [top]
    # The declarations of the function itself, under any of its names, go with it.
    for it in items:
        if it.kind == "decl" and it.names & aliases and it.names <= aliases | _tags_and_enumerators(it) \
                and (top.region != "cand" or it.region == "cand"):
            need[it.index] = "def"
            todo.append(it)
    while todo:
        it = todo.pop()
        refs = it.refs if it.kind != "func" or need[it.index] == "def" else _header_refs(it)
        for name in refs:
            if name in aliases:
                continue
            cands = [p for p in providers.get(name, ()) if p is not top]
            if top.region == "cand" and any(p.region == "cand" for p in cands):
                own = [p for p in cands if p.region == "cand"]
                # A type the candidate does not define itself still comes from the file.
                cands = own
            for p in cands:
                how = "def" if p.kind != "func" or p.static else "proto"
                if p.kind == "func" and not p.static and need.get(p.index) is None:
                    # A function defined in this file that it calls: its declaration, unless the
                    # file already declares it ahead of the function.
                    if any(q.kind == "decl" and q.index < top.index for q in cands):
                        continue
                if need.get(p.index) == "def" or need.get(p.index) == how:
                    continue
                need[p.index] = how
                todo.append(p)
    out, own = [], set()
    # Functions defined after it that it calls: their declarations go ahead of it.
    prototypes_after = [as_prototype(it) for it in items
                        if it.kind == "func" and it.index > top.index and need.get(it.index) == "proto"]
    for it in items:
        how = need.get(it.index)
        if how is None or it.kind in ("pp", "asm", "empty"):
            continue
        if it is top:
            out.extend(prototypes_after)
            prototypes_after = []
            out.append(it.text.strip() + "\n")
            continue
        if it.kind == "func":
            if how == "def":
                out.append(it.text.strip() + "\n")
                own.add(it.func_name)
            elif it.index < top.index:
                out.append(as_prototype(it))
            continue
        if re.match(r"\s*(?:struct|union|enum)\s+[A-Za-z_]\w*\s*;\s*$", code_only(it.text)):
            own |= it.names                      # a tag declared ahead of its definition
            out.append(it.text.strip() + "\n")
        elif it.typedef:
            own |= it.names
            out.append(it.text.strip() + "\n")
        elif it.static:
            own |= it.names
            out.append(it.text.strip() + "\n")
        elif it.extern or is_prototype(it) or not _declares_data(it):
            own |= _tags_and_enumerators(it)
            out.append(it.text.strip() + "\n")
        else:
            own |= _tags_and_enumerators(it)
            out.append(as_extern(it) if _tags_and_enumerators(it) == set() else it.text.strip() + "\n")
    # A name declared with an assembler name of its own is only this C's spelling of that symbol:
    # it is renamed apart too, as another file put in the same unit may spell it the same way.
    for it in items:
        if need.get(it.index) and it.kind == "decl" and not it.typedef and re.search(r'__asm__\s*\(\s*"', it.text):
            own |= it.names - _tags_and_enumerators(it)
    return "".join(out), sorted(own - aliases), sorted(aliases)


def _header_refs(it: Item) -> set:
    code = code_only(it.text)
    head = code[:_blank_groups(code).index("{")]
    return {n for n in IDENT.findall(head) if n not in KEYWORDS}


def _tags_and_enumerators(it: Item) -> set:
    blank = _blank_groups(code_only(it.text))
    out = {m.group(2) for m in re.finditer(r"\b(struct|union|enum)\s+([A-Za-z_]\w*)\s*\{", blank)}
    for m in re.finditer(r"\benum\b[^{;]*\{([^{}]*)\}", blank):
        out |= {n.group(0) for part in m.group(1).split(",") if (n := IDENT.match(part.strip()))}
    return out


def _declares_data(it: Item) -> bool:
    flat, _ = _without_braces(_blank_groups(code_only(it.text)))
    flat = re.sub(r"\b(?:struct|union|enum)\s*(?:[A-Za-z_]\w*)?\s*\{\}", " ", flat).strip().rstrip(";")
    return bool(flat.strip()) and any(declarator_name(p) for p in _split_top(flat))


def records_by_value(body: str, own: list[str], aliases: list[str]) -> list[str]:
    """The functions BODY declares or defines that take or return one of its own records (a struct
    or union it renames apart) by value. Their callers elsewhere, or the callee's own C, have a
    record type of their own for it, and the port's translation passes it as that type: the two
    are not one type to the host compiler."""
    items = split_items(body)
    own = set(own)
    records = set()
    for it in items:
        if it.kind != "decl":
            continue
        blank = _blank_groups(code_only(it.text))
        records |= {m.group(2) for m in re.finditer(r"\b(struct|union)\s+([A-Za-z_]\w*)\s*\{", blank)} & own
        if it.typedef and re.match(r"\s*typedef\s+(?:struct|union)\b", blank):
            flat, _ = _without_braces(blank)
            for part in _split_top(flat.rstrip().rstrip(";")):
                name = declarator_name(part)
                if name in own and "*" not in part.split("}")[-1]:
                    records.add(name)
    out = []
    for it in items:
        if not (it.kind == "func" or is_prototype(it)):
            continue
        code = _blank_groups(code_only(it.text))
        head = code[:code.index("{")] if it.kind == "func" else code
        for name in it.names - _tags_and_enumerators(it):
            m = re.search(r"\b" + re.escape(name) + r"\s*\(", head)
            if not m:
                continue
            ret = head[:m.start()]
            depth, j = 0, m.end() - 1
            while j < len(head):
                depth += {"(": 1, ")": -1}.get(head[j], 0)
                if depth == 0:
                    break
                j += 1
            params = _split_top(head[m.end():j])
            for part in [ret.split(";")[-1]] + params:
                words = set(IDENT.findall(part))
                if words & records and "*" not in part and "[" not in part:
                    out.append(f"{name} ({', '.join(sorted(words & records))})")
    return sorted(set(out))


def rename(text: str, names: list[str], suffix: str) -> str:
    """Every whole-word NAME outside strings and comments as NAME+SUFFIX."""
    if not names:
        return text
    code = code_only(text)
    want = set(names)
    out, last = [], 0
    for m in IDENT.finditer(code):
        if m.group(0) in want:
            out.append(text[last:m.end()])
            out.append(suffix)
            last = m.end()
    out.append(text[last:])
    return "".join(out)


# ---------------------------------------------------------------------------------------------
# The two versions


@dataclass
class Version:
    cfg_path: Path
    cfg: dict
    source: Path
    gen: Gen

    @property
    def dir(self) -> Path:
        return self.cfg_path.parent

    def hand_rows(self) -> list[list[str]]:
        return read_tsv(self.dir / self.cfg["hand"]) if self.cfg.get("hand") else []


def load_version(cfg_path: Path, gen: Path) -> Version:
    cfg = json.loads(cfg_path.read_text())
    return Version(cfg_path, cfg, ROOT / cfg["source"], Gen(gen))


def clang() -> str:
    return os.environ.get("OPENRAC_CLANG") or "clang"


def preprocess(text: str, where: Path, source: Version) -> str:
    """TEXT, a file of the source decompilation at WHERE, through Clang's preprocessor as hostgen
    reads it, but without hostgen's own header (the target's unit includes it again)."""
    flags = ["--target=mipsel-linux-gnu", "-std=gnu89", "-E", "-P", "-nostdinc", "-fsigned-char", "-w",
             "-DM2CTX", "-DOPENRAC_HOSTGEN", "-DINCLUDE_ASM(FOLDER,NAME)=", "-DINCLUDE_RODATA(FOLDER,NAME)="]
    flags += ["-iquote", str(where.parent)]
    for inc in source.cfg["includes"]:
        flags += ["-I", str(source.source / inc)]
    flags += ["-I", str(ROOT / "port/tools/hostgen/include")]
    flags += [f"-D{d}" for d in source.cfg.get("defines", [])]
    r = subprocess.run([clang(), *flags, "-x", "c", "-"], input=text, capture_output=True, text=True)
    if r.returncode:
        raise LookupError("the preprocessor: " + (r.stderr.strip().splitlines() or ["failed"])[0])
    out = GP_LABEL.sub(r"\1\2", r.stdout)
    return SECTION.sub("", out)


class Carry:
    def __init__(self, args):
        self.args = args
        self.tgt = load_version(args.target, args.gen)
        self.src = load_version(args.source, args.source_gen)
        t, s = self.tgt, self.src
        self.back = json.loads(args.back_map.read_text())
        self.carrier = Carrier(args.to_map, args.gen, t.source / t.cfg["sources"][0],
                               t.source / t.cfg["places"] if t.cfg.get("places") else None,
                               t.dir / t.cfg["split_places"] if t.cfg.get("split_places") else None)
        self.t_stubs = stubs_of(t.source, sorted(set(t.cfg["sources"] + t.cfg["includes"])))
        self.s_stubs = stubs_of(s.source, s.cfg["sources"])
        self.s_defs = definitions_of(s.source, s.cfg["sources"], s.cfg.get("skip", []))
        self.t_places = read_places(t.source / t.cfg["places"]) if t.cfg.get("places") else {}
        self.s_places = read_places(s.source / s.cfg["places"]) if s.cfg.get("places") else {}
        self.t_symbols = read_symbols([t.source / p for p in t.cfg.get("symbols", [])])
        self.s_hand = {r[0]: s.dir / Path(s.cfg["hand"]).parent / r[1] for r in s.hand_rows() if len(r) > 1}
        self.s_cand = {r[0]: s.source / r[1] for r in read_tsv(s.source / s.cfg.get("candidates", "-"))
                       if len(r) > 1} if s.cfg.get("candidates") else {}
        self.s_joined = {}
        joined = s.source / "config/overlays/joined.tsv"
        for cols in read_tsv(joined):
            if len(cols) > 1:
                self.s_joined[cols[1]] = cols[0]
        self.t_host = set(t.cfg.get("host_functions", [])) | set(t.cfg.get("level_hooks", []))
        self.s_host = set(s.cfg.get("host_functions", [])) | set(s.cfg.get("level_hooks", []))
        for v, into in ((t, self.t_host), (s, self.s_host)):
            for cols in read_tsv(v.dir / v.cfg.get("libraries", "-")):
                into.add(cols[0])
        self.t_skip = t.cfg.get("skip", [])
        self.intentional = set()
        cats = t.source / "config/us/unit_categories.json"
        if cats.is_file():
            self.intentional = set(json.loads(cats.read_text()).get("intentional_asm", []))
        self.drafts = set()
        if t.cfg.get("drafts"):
            self.drafts = {line.split("#")[0].strip() for line in (t.dir / t.cfg["drafts"]["units"]).read_text().splitlines()} - {""}
        # The hand rows this tool wrote (their file says so): carried again on every run.
        self.ours = set()
        hand_dir = t.dir / Path(t.cfg.get("hand", "hand/hand.tsv")).parent
        for row in t.hand_rows():
            path = hand_dir / row[1].strip() if len(row) > 1 else None
            if path and path.is_file() and MARK in path.read_text(errors="replace")[:200]:
                self.ours.add(row[0])
        self._cuts: dict[tuple[str, str], dict[int, tuple[int, str]]] = {}
        self._asm_index = None
        self._texts: dict[str, str] = {}

    # -- where things are ---------------------------------------------------------------------

    def cuts(self, version: str, program: str) -> dict[int, tuple[int, str]]:
        """address -> (size, fingerprint) of the functions of one program of a version, as the map cuts them."""
        key = (version, program)
        if key not in self._cuts:
            import versionmap
            if not hasattr(self, "_programs"):
                self._programs = {}
            if version not in self._programs:
                try:
                    self._programs[version] = versionmap.programs(version)
                except (OSError, KeyError, SystemExit):      # no code of that version here
                    self._programs[version] = {}
            sections = self._programs[version].get(program, {})
            if not sections:
                self._cuts[key] = {}
                return self._cuts[key]
            self._cuts[key] = {a: (size, fp) for a, size, fp, _code in versionmap.functions(sections)} if sections else {}
        return self._cuts[key]

    def locations(self, name: str) -> list[tuple[int, int]]:
        """The target's places of NAME: its own address first, then the places its catalogue gives."""
        out = []
        m = ADDRESS_NAME.match(name)
        if m:
            out.append((int(m.group(1)) if m.group(1) else -1, int(m.group(2), 16)))
        elif name in self.t_symbols:
            out.append((-1, self.t_symbols[name]))
        for key in self.tgt.gen.where.get(name, []):
            if key not in out:
                out.append(key)
        for key in self.t_places.get(name, (None, 0, []))[2]:
            if key not in out:
                out.append(key)
        return out

    def us_asm(self, name: str, overlay: int, address: int) -> str | None:
        """The target's assembly of a function: its stub's listing, or a listing named after it."""
        t = self.tgt.source
        for rel, _macro, folder in self.t_stubs.get(name, []):
            for base in (t, t / "build/baseline"):
                p = base / folder
                if p.is_file():
                    return p.relative_to(ROOT).as_posix()
        if self._asm_index is None:
            self._asm_index = {}
            for d in (t / "config/us/overlays/asm", t / "build/baseline/config/us/expected/asm", t / "build/overlay-asm"):
                if d.is_dir():
                    for p in d.rglob("*.s"):
                        m = ADDRESS_NAME.match(p.stem)
                        if m:
                            key = (int(m.group(1)) if m.group(1) else -1, int(m.group(2), 16))
                            self._asm_index.setdefault(key, p.relative_to(ROOT).as_posix())
        return self._asm_index.get((overlay, address)) or self._asm_index.get((-1, address))

    # -- the source's C for a function ----------------------------------------------------------

    def source_c(self, program: str, address: int) -> dict | None:
        """What the source runs at a place: {name, how: matched|hand|functional|host, file}."""
        ov = overlay_of(program)
        names = self.src.gen.at.get((ov, address)) or (self.src.gen.at.get((-1, address)) if ov >= 0 else None) or []
        with_c = [n for n in names if n not in self.src.gen.no_c]
        for n in with_c:
            if n in self.s_defs and n not in self.s_stubs:
                return {"name": n, "how": "matched", "file": self.src.source / self.s_defs[n]}
        for n in with_c:
            if n in self.s_hand:
                return {"name": n, "how": "hand", "file": self.s_hand[n]}
        for n in with_c:
            if n in self.s_cand:
                return {"name": n, "how": "functional", "file": self.s_cand[n]}
        for n in with_c:
            if n in self.s_host:
                return {"name": n, "how": "host", "file": None}
        for n in names:
            if n in self.s_stubs:
                return {"name": n, "how": "asm", "file": None, "macro": self.s_stubs[n][0][1]}
        if names:
            return {"name": names[0], "how": "none", "file": None}
        return None

    def source_text(self, info: dict) -> tuple[str, Path, str | None]:
        """(the file to preprocess, where it is, the candidate marker or None)."""
        name, path = info["name"], info["file"]
        if info["how"] == "matched":
            return path.read_text(errors="replace"), path, None
        cand = path.read_text(errors="replace")
        marker = "openrac_carry_candidate_starts_here"
        host = self.s_stubs.get(name)
        if host:
            where = self.src.source / host[0][0]
            text = where.read_text(errors="replace")
            text = re.sub(r'^[ \t]*(?:INCLUDE_ASM|ASM_FUNC|LINKER_REMNANT)\(\s*(?:"[^"]*"\s*)+,\s*' + re.escape(name)
                          + r'\s*\)\s*;?', "", text, flags=re.M)
            return f"{text}\nint {marker};\n{cand}\n", where, marker
        return f"int {marker};\n{cand}\n", path, marker

    # -- one function ---------------------------------------------------------------------------

    def carry_one(self, us_name: str, program: str, us_address: int, info: dict, kind: str) -> tuple[str, list[str]]:
        text, where, marker = self.source_text(info)
        pre = preprocess(text, where, self.src)
        body, own, aliases = extract(pre, info["name"], marker)
        by_value = records_by_value(body, own, aliases)
        if by_value:
            raise LookupError("a record of its own passed or returned by value, which the port's C types "
                              "apart from the other side's: " + "; ".join(by_value))
        ov = overlay_of(program)
        suffix = "_c" + (f"{ov:02d}_" if ov >= 0 else "") + f"{us_address:x}"
        body = rename(body, own, suffix)
        sentinel = "openrac_carry_target"
        for name in aliases:
            body = re.sub(r"\b" + re.escape(name) + r"\b", sentinel, body)
        defined = set()
        carried, notes = carry_c(self.carrier, body, 0x00100000, 0x02000000, True, program, defined)
        carried = re.sub(r"\b" + sentinel + r"\b", us_name, carried)
        bad = [n for n in notes if "unpaired" in n or "no place found" in n and "left as it is" not in n]
        if "__NOT_CARRIED" in carried or bad:
            raise LookupError("names with no place in this version: " + "; ".join(bad or ["__NOT_CARRIED"]))
        try:
            what = info["file"].resolve().relative_to(self.src.source.resolve()).as_posix()
            origin = f"the decompilation of {self.src.cfg.get('title', self.src.cfg['id'])}, {what} ({self.src.cfg['source']})"
        except ValueError:
            origin = rel_root(info["file"])
        how = {"matched": "matched C there", "hand": "the port's hand C there",
               "functional": "functional C there, not matched"}[info["how"]]
        head = (f"/* {MARK} from {origin}: {info['name']}, "
                f"{how}.\n * Kind `{kind}` in the version map (../../README.md): the two versions have the same "
                f"instructions for it.\n * Names and addresses are this version's. Functional C, not matched C. */\n")
        conf = re.match(r"\s*(/\*\s*functional:.*?\*/)", info["file"].read_text(errors="replace"), re.S)
        if conf:
            head += conf.group(1) + "\n"
        if notes:
            head += "/* carry, not from a `same` function or an `exact` data address (read them):\n" + "".join(
                f" *   {n}\n" for n in notes) + " */\n"
        return head + "\n" + carried, notes

    def check(self, us_name: str, text: str) -> str | None:
        """Clang's first error for TEXT where hostgen puts it (at the end of the file of US_NAME's
        stub, in hostgen's prepared copy of it, with hostgen's own fixes for what Clang refuses),
        or None. Errors the file has without it do not count."""
        import clangast
        import prep
        rel = self.t_stubs[us_name][0][0]
        prepped = self.args.gen / "prep" / rel
        if not prepped.is_file():
            return None if self.args.no_check else "no prepared copy of its file (build the port once)"
        base = prepped.read_text(encoding="utf-8", errors="surrogateescape")
        # What this tool put there on an earlier run comes out first.
        mark = f"\n/* hostgen: {us_name} from a candidate, not the matched C */\n"
        if mark in base:
            at = base.index(mark)
            nxt = base.find("\n/* hostgen: ", at + len(mark))
            base = base[:at] + (base[nxt:] if nxt >= 0 else "")
        base = re.sub(r'^[ \t]*(?:INCLUDE_ASM|ASM_FUNC|LINKER_REMNANT)\(\s*(?:"[^"]*"\s*)+,\s*' + re.escape(us_name)
                      + r'\s*\)\s*;?[ \t]*(?:/\*.*?\*/)?[ \t]*$', "", base, flags=re.M)
        matched = set(prep._DEFINED.findall(base))
        if us_name in matched:
            # hostgen leaves a candidate's copy of a function its file defines out (without_definitions),
            # even where that definition sits in a branch it does not read (a draft under #else).
            return "its file defines it already (a draft hostgen does not read?); the copy would be left out"
        landed = base + mark + prep.own_declarations(prep.without_definitions(prep.prepare(text), matched), "_hccarry")
        flags = clangast.base_flags([Path(i) for i in self.tgt.cfg["includes"]] + [ROOT / "port/tools/hostgen/include"],
                                    self.tgt.cfg.get("defines", []))
        root = self.args.gen / "prep"
        errors = {}
        for label, body in (("without", base), ("with", landed)):
            tmp_rel = (Path(rel).parent / f".carry_check_{us_name}_{label}.c").as_posix()
            path = root / tmp_rel
            path.write_text(body, encoding="utf-8", errors="surrogateescape")
            try:
                for _ in range(12):
                    r = subprocess.run([clangast.clang_binary(), *flags, tmp_rel], cwd=root,
                                       capture_output=True, text=True, errors="replace")
                    if r.returncode == 0:
                        break
                    cur = path.read_text(encoding="utf-8", errors="surrogateescape")
                    fixed, more = clangast.fix_from_diagnostics(cur, r.stderr, tmp_rel)
                    if not more or fixed == cur:
                        break
                    path.write_text(fixed, encoding="utf-8", errors="surrogateescape")
            finally:
                path.unlink(missing_ok=True)
            errors[label] = [line.split(": error:", 1)[1].strip() for line in r.stderr.splitlines()
                             if ": error:" in line] if r.returncode else []
        new = [e for e in errors["with"] if e not in errors["without"]]
        return new[0] if new else None

    # -- the whole pass ---------------------------------------------------------------------------

    def targets(self) -> dict[str, dict]:
        """name -> {places, stub, aliases} for every function of the target with no C."""
        stubs = self.t_stubs
        names = list(dict.fromkeys(list(stubs) + self.tgt.gen.missing))
        out: dict[str, dict] = {}
        covered = []
        for name in names:
            locs = self.locations(name)
            if name in self.ours and name in stubs:
                # C this tool carried before: carried again, from what the source has now.
                out[name] = {"places": locs, "stub": True}
                continue
            has = sorted({n for loc in locs for n in self.tgt.gen.with_c(*loc)} - {name})
            if name in self.tgt.gen.where and name not in self.tgt.gen.no_c:
                has = has or [name]
            if name in stubs and name not in self.tgt.gen.no_c and name in self.tgt.gen.where:
                has = has or [name]
            if has or name in self.t_host:
                covered.append({"us_name": name, "why": "has C as " + ", ".join(has or ["host C / libraries"])})
                continue
            if name in stubs and self._has_c_by_candidate(name):
                covered.append({"us_name": name, "why": "replaced by the port's own C already"})
                continue
            out[name] = {"places": locs, "stub": name in stubs}
        # One function with several names: the one with a stub carries it, the others are aliases.
        by_place: dict[tuple[int, int], list[str]] = defaultdict(list)
        for name, t in out.items():
            if t["places"]:
                by_place[t["places"][0]].append(name)
        for place, group in by_place.items():
            if len(group) < 2:
                continue
            keep = sorted(group, key=lambda n: (not out[n]["stub"], n))[0]
            for n in group:
                if n != keep:
                    out[keep].setdefault("aliases", []).append(n)
                    out[n]["alias_of"] = keep
        self.covered = covered
        return out

    def _has_c_by_candidate(self, name: str) -> bool:
        used = set(self.tgt.gen.report.get("candidates_used", []))
        return name in used

    def run(self) -> dict:
        targets = self.targets()
        carried, near, write, skipped = [], [], [], []
        jobs = []
        for name, t in sorted(targets.items()):
            if t.get("alias_of"):
                continue
            entry = self.classify(name, t)
            kind = entry.pop("result")
            if kind == "skip":
                skipped.append(entry)
            elif kind == "near":
                near.append(entry)
            elif kind == "write":
                write.append(entry)
            else:
                jobs.append(entry)

        def work(entry):
            try:
                text, notes = self.carry_one(entry["us_name"], entry["program"], entry["address"], entry["info"], entry["kind"])
            except LookupError as e:
                return entry, None, str(e)
            err = self.check(entry["us_name"], text) if not self.args.no_check else None
            return entry, text, err

        with ThreadPoolExecutor(self.args.jobs) as ex:
            for entry, text, err in ex.map(work, jobs):
                if text is None or err:
                    skipped.append({"us_name": entry["us_name"], "pal_name": entry["info"]["name"],
                                    "pal_c": rel_root(entry["info"]["file"]),
                                    "why": f"does not carry: {err}" if text is None else f"where hostgen puts it: {err}"})
                    continue
                entry["text"] = text
                carried.append(entry)
        return {"carried": carried, "near": near, "write": write, "skipped": skipped, "covered": self.covered,
                "targets": targets}

    def classify(self, name: str, t: dict) -> dict:
        places = t["places"]
        aliases = t.get("aliases", [])
        base = {"us_name": name}
        if aliases:
            base["aliases"] = aliases
        if not places:
            return {**base, "result": "skip", "why": "no address: a name nothing of this version places"}
        ov, address = places[0]
        program = program_key(ov)
        stub = self.t_stubs.get(name)
        if stub:
            rel, macro, _folder = stub[0]
            unit = re.sub(r"^src/|\.c$", "", rel)
            if macro == "LINKER_REMNANT":
                return {**base, "result": "skip", "why": f"a linker remnant ({rel})"}
            if any(rel == s or rel.startswith(s.rstrip("/") + "/") for s in self.t_skip):
                return {**base, "result": "skip", "why": f"its file is left out of the build (hostgen.json skip: {rel})"}
        else:
            if not any(self.cuts(self.back["from"], program_key(o)).get(a) for o, a in places):
                return {**base, "result": "skip", "why": f"no function of this version starts at "
                        f"{program} 0x{address:08X} (a name for another version's address, or an entry "
                        f"inside a function)"}
            return {**base, "result": "skip", "why": f"no assembly stub of it in this decompilation for C to "
                    f"replace ({program} 0x{address:08X}); hostgen puts hand C only in place of one"}
        hit = None
        for o, a in places:
            p = program_key(o)
            raw = self.back["programs"].get(p, {}).get("functions", {}).get(f"{a:08X}")
            if raw:
                hit = (p, a, int(raw[0], 16), raw[1])
                break
        size = self.size(program, address, name)
        asm = self.us_asm(name, ov, address)
        if hit is None:
            if unit in self.intentional or stub[0][1] == "ASM_FUNC":
                return {**base, "result": "skip", "why": "handwritten (intentional assembly) with no counterpart"}
            return {**base, "result": "write", "us_asm": asm, "bytes": size,
                    "kind": "exe" if ov < 0 else "level"}
        program, address, pal_address, kind = hit
        if kind in ("called", "found"):
            kind = self.same_code(program, address, pal_address)
        info = self.source_c(program, pal_address)
        pal_name = info["name"] if info else None
        base.update({"pal_name": pal_name, "program": program, "address": address})
        if pal_name in self.s_joined:
            return {**base, "result": "skip", "why": f"a piece of {self.s_joined[pal_name]} in the PAL catalogue (joined.tsv)"}
        if info and info["how"] == "asm" and info.get("macro") == "LINKER_REMNANT":
            return {**base, "result": "skip", "why": f"a linker remnant in PAL ({pal_name}, asm/remnants)"}
        if info and info["how"] == "asm" and info.get("macro") == "ASM_FUNC":
            return {**base, "result": "skip", "why": f"handwritten in PAL ({pal_name}, ASM_FUNC, asm/handwritten) with no hand C"}
        if info and info["how"] == "host":
            return {**base, "result": "skip", "why": f"PAL's port runs {pal_name} as host C or a library; "
                    "this version needs its own entry there (rac1_version.h, libraries.tsv, host_functions)"}
        has_c = info is not None and info["how"] in ("matched", "hand", "functional")
        if kind == "near":
            if has_c:
                return {**base, "result": "near", "pal_c": rel_root(info["file"]), "us_asm": asm, "bytes": size}
            return {**base, "result": "write", "us_asm": asm, "bytes": size, "kind": "exe" if ov < 0 else "level",
                    "pal_name": pal_name, "pair": "near"}
        if not has_c:
            if unit in self.intentional:
                return {**base, "result": "skip", "why": "handwritten (intentional assembly); PAL has no C either"}
            return {**base, "result": "write", "us_asm": asm, "bytes": size, "kind": "exe" if ov < 0 else "level",
                    "pal_name": pal_name, "pair": "same, PAL has no C"}
        return {**base, "result": "carry", "kind": "same", "info": info, "us_asm": asm, "bytes": size}

    def same_code(self, program: str, us_address: int, pal_address: int) -> str:
        a = self.cuts(self.back["from"], program).get(us_address)
        b = self.cuts(self.back["to"], program).get(pal_address)
        return "same" if a and b and a == b else "near"

    def size(self, program: str, address: int, name: str) -> int | None:
        if name in self.t_places and self.t_places[name][1]:
            return self.t_places[name][1]
        cut = self.cuts(self.back["from"], program).get(address)
        return cut[0] if cut else None


def rel_root(path) -> str | None:
    if path is None:
        return None
    try:
        return Path(path).resolve().relative_to(ROOT).as_posix()
    except ValueError:
        return str(path)


# ---------------------------------------------------------------------------------------------
# Writing


def stale_rows(version: Version, stubs: dict) -> list[list[str]]:
    """hand.tsv rows whose function has no stub left to replace."""
    return [r for r in version.hand_rows() if r[0] not in stubs]


def write_results(c: Carry, res: dict) -> dict:
    t = c.tgt
    table = t.dir / t.cfg["hand"]
    folder = table.parent / "functional"
    lines = table.read_text().splitlines()
    stale = stale_rows(t, c.t_stubs)
    stale_names = {r[0] for r in stale}
    # What this tool carried before and does not carry now goes, with its file.
    carried_now = {e["us_name"] for e in res["carried"]}
    withdrawn = sorted(c.ours - carried_now - stale_names)
    for name in withdrawn:
        path = folder / f"{name}.c"
        if path.is_file():
            path.unlink()
    stale_names |= set(withdrawn)
    keep_lines = [line for line in lines if line.startswith("#") or not line.strip() or line.split("\t")[0] not in stale_names]
    still = {line.split("\t")[1].strip() for line in keep_lines if "\t" in line and not line.startswith("#")}
    removed = []
    for r in stale:
        if r[0] in withdrawn:
            continue
        f = r[1].strip() if len(r) > 1 else ""
        gone = False
        if f and f not in still and (table.parent / f).is_file():
            (table.parent / f).unlink()
            still.add(f)          # once
            gone = True
        removed.append({"us_name": r[0], "file": f, "file_removed": gone})
    have = {line.split("\t")[0] for line in keep_lines if "\t" in line and not line.startswith("#")}
    added = []
    for e in sorted(res["carried"], key=lambda e: e["us_name"]):
        name = e["us_name"]
        (folder / f"{name}.c").write_text(e["text"])
        if name not in have:
            keep_lines.append(f"{name}\tfunctional/{name}.c")
            have.add(name)
            added.append(name)
    table.write_text("\n".join(keep_lines) + "\n")
    return {"stale_removed": removed, "rows_added": added, "withdrawn": withdrawn}


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("--target", type=Path, required=True, help="the target's hostgen.json")
    ap.add_argument("--source", type=Path, required=True, help="the source's hostgen.json (the version that runs)")
    ap.add_argument("--to-map", type=Path, required=True, help="the map from the source to the target")
    ap.add_argument("--back-map", type=Path, required=True, help="the map from the target to the source")
    ap.add_argument("--gen", type=Path, required=True, help="the target's generated folder")
    ap.add_argument("--source-gen", type=Path, required=True, help="the source's generated folder")
    ap.add_argument("--out", type=Path, required=True, help="where the worklists go (near, write, skipped, covered .json)")
    ap.add_argument("--write", action="store_true", help="write the carried files and hand.tsv, drop stale rows")
    ap.add_argument("--no-check", action="store_true", help="do not check the carried C with Clang")
    ap.add_argument("--jobs", type=int, default=os.cpu_count() or 4)
    ap.add_argument("--only", nargs="*", help="only these target names")
    args = ap.parse_args(argv)
    args.out.mkdir(parents=True, exist_ok=True)
    c = Carry(args)
    if args.only:
        keep = set(args.only)
        orig = c.targets
        c.targets = lambda: {k: v for k, v in orig().items() if k in keep}
    res = c.run()
    args.out.mkdir(parents=True, exist_ok=True)
    near = [{k: e.get(k) for k in ("us_name", "pal_name", "pal_c", "us_asm", "bytes")} for e in res["near"]]
    write = [{k: e.get(k) for k in ("us_name", "us_asm", "bytes", "kind", "pal_name", "pair")} for e in res["write"]]
    (args.out / "near.json").write_text(json.dumps(near, indent=1) + "\n")
    (args.out / "write.json").write_text(json.dumps(write, indent=1) + "\n")
    (args.out / "skipped.json").write_text(json.dumps(res["skipped"], indent=1) + "\n")
    (args.out / "covered.json").write_text(json.dumps(res["covered"], indent=1) + "\n")
    carried = [{"us_name": e["us_name"], "pal_name": e["info"]["name"], "pal_c": rel_root(e["info"]["file"]),
                "how": e["info"]["how"], "program": e["program"]} for e in res["carried"]]
    (args.out / "carried.json").write_text(json.dumps(carried, indent=1) + "\n")
    summary = {"carried": len(carried), "near": len(near), "write": len(write), "skipped": len(res["skipped"]),
               "covered": len(res["covered"])}
    if args.write:
        w = write_results(c, res)
        (args.out / "stale.json").write_text(json.dumps(w["stale_removed"], indent=1) + "\n")
        summary["stale_removed"] = len(w["stale_removed"])
        summary["rows_added"] = len(w["rows_added"])
        summary["withdrawn"] = len(w["withdrawn"])
    print(json.dumps(summary))
    return 0


if __name__ == "__main__":
    sys.exit(main())
