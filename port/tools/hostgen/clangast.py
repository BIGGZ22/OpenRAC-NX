# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""Clang reads one translation unit as the console's C and gives its AST.

The unit is read for a 32-bit little-endian MIPS target, so every type has
the console's size and layout, in GNU C89 as GCC 2.95 accepted it. What
Clang refuses that GCC 2.95 accepted is fixed in the prepared copy, driven
by Clang's own diagnostics (fix_from_diagnostics), and recorded.
"""

from __future__ import annotations

import json
import os
import re
import shutil
import subprocess
from dataclasses import dataclass, field
from pathlib import Path

# GCC 2.95 accepted these; Clang makes them errors unless told otherwise.
RELAXED = [
    "-Wno-error=implicit-function-declaration",
    "-Wno-error=implicit-int",
    "-Wno-error=int-conversion",
    "-Wno-error=incompatible-pointer-types",
    "-Wno-error=incompatible-function-pointer-types",
    "-Wno-error=return-type",
    "-Wno-error=return-mismatch",
    "-Wno-error=int-to-pointer-cast",
]


def clang_binary() -> str:
    if os.environ.get("OPENRAC_CLANG"):
        return os.environ["OPENRAC_CLANG"]
    for name in ("clang", "clang-18", "clang-17", "clang-19", "clang-20"):
        path = shutil.which(name)
        if path:
            return path
    raise SystemExit("hostgen needs Clang (clang on PATH) to read the decompilation's C")


@dataclass
class Unit:
    path: Path                      # the prepared copy, relative paths inside
    ast: dict
    fixes: list[str] = field(default_factory=list)


def base_flags(includes: list[Path], defines: list[str]) -> list[str]:
    flags = [
        "--target=mipsel-linux-gnu", "-std=gnu89", "-fsyntax-only", "-w", "-ferror-limit=50",
        "-fsigned-char", "-nostdinc",
        # include_asm.h: no file-scope asm when M2CTX is set; INCLUDE_ASM marks the
        # function as one that is still assembly, which hostgen reads back.
        "-DM2CTX", "-DOPENRAC_HOSTGEN",
        "-DINCLUDE_ASM(FOLDER,NAME)=extern int openrac_asm_##NAME",
        "-DINCLUDE_RODATA(FOLDER,NAME)=",
    ] + RELAXED
    for inc in includes:
        flags += ["-I", str(inc)]
    flags += ["-include", "openrac_hostgen.h"]
    for d in defines:
        flags.append(f"-D{d}")
    return flags


def run(clang: str, path: Path, cwd: Path, flags: list[str], json_out: bool) -> subprocess.CompletedProcess:
    cmd = [clang, *flags]
    if json_out:
        cmd += ["-Xclang", "-ast-dump=json"]
    # Keep source locations comparable with hostgen's slash-separated unit names,
    # including the locations embedded in Clang's anonymous type names.
    cmd.append(path.as_posix())
    return subprocess.run(cmd, cwd=cwd, capture_output=True, text=True, errors="replace")


_DIAG = re.compile(r"^(?P<file>[^:\n]+):(?P<line>\d+):(?P<col>\d+): (?P<kind>error|note): (?P<msg>.*)$", re.M)


def fix_from_diagnostics(text: str, stderr: str, rel: str) -> tuple[str, list[str]]:
    """Fixes, in the prepared copy, what Clang refuses and GCC 2.95 took.

    - "conflicting types for 'f'" after an implicit declaration: f is called
      before it is declared or defined in the same file, so GCC 2.95 gave
      that call an implicit `int f()`. The call is renamed to an alias with
      exactly that declaration and f's symbol (an asm label), declared at
      the start of the function that makes the call; f's own declaration
      and definition stay as they are.
    - An inline assembly statement whose constraints or registers Clang
      does not know: replaced by a call to openrac_hostgen_asm() (on the
      same lines), so the function becomes a stub that says why.
    - "conflicting types for 'f'" after an earlier declaration or definition
      of f: the later declaration, and the calls after it, use an alias of
      f's symbol. So does a later definition of f that contradicts an
      earlier declaration (a shared header that says `void *` where the
      body takes `float *`): the alias is declared with f's symbol on the
      definition's own line, and the definition is the alias's.
    - An asm label Clang refuses (given after the function's first use, or
      a second spelling of one already given): removed from that
      declaration, and the name recorded as an alias of the label's symbol
      ("alias NAME SYMBOL" in the fixes), which hostgen follows.
    - "too few arguments to function call": a call to a K&R function with
      fewer arguments than its definition. The missing ones are passed as 0
      (on the console they were whatever the registers held).

    Both edits stay on their line, so line numbers do not move.
    """
    lines = text.split("\n")
    code_lines = _lines_outside_comments(text)
    fixes = []
    diags = list(_DIAG.finditer(stderr))
    inserts: dict[int, list[str]] = {}
    for k, d in enumerate(diags):
        if d["kind"] != "error" or not d["file"].replace("\\", "/").endswith(rel):
            continue
        msg = d["msg"]
        line = int(d["line"]) - 1
        col = int(d["col"]) - 1
        m = re.match(r"conflicting types for '(\w+)'", msg)
        if m:
            name = m.group(1)
            note = diags[k + 1] if k + 1 < len(diags) else None
            if note is None or note["kind"] != "note":
                continue
            if note["msg"].startswith(("previous definition is here", "previous declaration is here")):
                # A later declaration that contradicts an earlier one: it and the
                # calls after it use an alias of the same symbol (hostgen matches
                # every call to the definition's parameters anyway).
                alias = f"{name}__openrac_decl_{line + 1}"
                decl = re.sub(rf"\b{name}\b", alias, lines[line], count=1)
                what = "a second declaration"
                if re.search(r"__asm__\s*\(", decl):
                    # Already labelled with its symbol: the label stays the alias's.
                    n = 1
                else:
                    decl, n = re.subn(r"\)\s*;", f') __asm__("{name}");', decl, count=1)
                if n == 0 and note["msg"].startswith("previous declaration is here"):
                    # Not a declaration but the definition, which says other types than a
                    # declaration before it (a shared header's `void *` where the body takes
                    # `float *`). A definition cannot carry an asm label, so the alias is
                    # declared with it first, from the definition's own heading, on its line.
                    end = line
                    while end < len(lines) - 1 and end - line < 8 and "{" not in lines[end] and ";" not in lines[end]:
                        end += 1
                    heading = " ".join(lines[line:end] + [lines[end].split("{", 1)[0]])
                    if "{" in lines[end] and ";" not in heading:
                        heading = re.sub(rf"\b{name}\b", alias, heading, count=1).strip()
                        decl = f'{heading} __asm__("{name}"); {decl}'
                        what = "the definition"
                        n = 1
                if n == 0:
                    continue
                lines[line] = decl
                for j in range(line + 1, len(lines)):
                    lines[j] = re.sub(rf"\b{name}\b", alias, lines[j])
                fixes.append(f"line {line + 1}: {what} of {name} that contradicts the first, kept apart")
                continue
            if "implicit declaration" not in note["msg"]:
                continue
            use, use_col = int(note["line"]) - 1, int(note["col"]) - 1
            src = lines[use]
            # The call may spell f through a macro (include/names.h).
            word = re.match(r"\w+", src[use_col:])
            if word is None:
                continue
            alias = f"{name}__openrac_implicit_{use + 1}"
            lines[use] = src[:use_col] + alias + src[use_col + word.end():]
            at = _function_start(lines, code_lines, use)
            inserts.setdefault(at, []).append(f'int {alias}() __asm__("{name}");')
            fixes.append(f"line {use + 1}: {name} called before its declaration, as int {name}()")
            continue
        if msg.startswith(("cannot apply asm label to function after its first use", "conflicting asm label")):
            # The label may be on a declaration that runs over several lines, and for a conflict
            # it may be on the earlier declaration the note points at, not on this line.
            places = [line]
            note = diags[k + 1] if k + 1 < len(diags) else None
            if note is not None and note["kind"] == "note" and note["file"].replace("\\", "/").endswith(rel):
                places.append(int(note["line"]) - 1)
            for at in places:
                end = at
                while end < len(lines) - 1 and end - at < 6 and ";" not in lines[end] and "{" not in lines[end]:
                    end += 1
                window = "\n".join(lines[at:end + 1])
                lm = re.search(r"\b(\w+)\s*\(([^;{]*)\)\s*(__asm__|asm)\s*\(\s*\"([^\"]+)\"\s*\)", window)
                if not lm:
                    continue
                cut = window[:lm.start(3)] + re.sub(r"[^\n]", " ", window[lm.start(3):lm.end()]) + window[lm.end():]
                lines[at:end + 1] = cut.split("\n")
                if msg.startswith("cannot apply") or at != line:
                    fixes.append(f"alias {lm.group(1)} {lm.group(4)}")
                else:
                    fixes.append(f"line {line + 1}: second asm label {lm.group(4)} for {lm.group(1)} removed")
                break
            continue
        if re.match(r"(invalid (output|input) constraint|unknown register name|invalid operand in inline asm"
                    r"|couldn't allocate .* inline asm)", msg):
            replaced = _replace_asm(lines, line, col)
            if replaced:
                fixes.append(f"line {line + 1}: inline assembly Clang cannot read, made a stub")
            continue
        m = re.match(r"too few arguments to function call, expected (\d+), have (\d+)", msg)
        if m:
            want, have = int(m.group(1)), int(m.group(2))
            src = lines[line]
            pad = ", ".join(["0"] * (want - have))
            if have:
                pad = ", " + pad
            if 0 <= col < len(src) and src[col] == ")":
                lines[line] = src[:col] + pad + src[col:]
                fixes.append(f"line {line + 1}: passed {want - have} missing argument(s) as 0")
    for at, decls in inserts.items():
        lines[at] = " ".join(dict.fromkeys(decls)) + " " + lines[at]
    return "\n".join(lines), fixes


def _replace_asm(lines: list[str], line: int, col: int) -> bool:
    """Replaces the asm statement around (line, col) with openrac_hostgen_asm();
    keeping every line break."""
    text = "\n".join(lines)
    offsets = [0]
    for ln in lines:
        offsets.append(offsets[-1] + len(ln) + 1)
    at = offsets[line] + col
    starts = [m.start() for m in re.finditer(r"\b(?:__asm__|__asm|asm)\b", text[:at])]
    if not starts:
        return False
    start = starts[-1]
    j = text.find("(", start)
    depth = 0
    end = None
    while 0 <= j < len(text):
        ch = text[j]
        if ch == '"':
            j += 1
            while j < len(text) and text[j] != '"':
                j += 2 if text[j] == "\\" else 1
        elif ch == "(":
            depth += 1
        elif ch == ")":
            depth -= 1
            if depth == 0:
                end = text.find(";", j)
                break
        j += 1
    if end is None or end < 0:
        return False
    stmt = text[start:end + 1]
    call = "openrac_hostgen_asm();"
    blank = "".join("\n" if ch == "\n" else " " for ch in stmt[len(call):])
    new = text[:start] + call + blank + text[end + 1:]
    lines[:] = new.split("\n")
    return True


def _lines_outside_comments(text: str) -> set[int]:
    """The lines that do not start inside a block comment."""
    inside = False
    out = set()
    for i, line in enumerate(text.split("\n")):
        if not inside:
            out.add(i)
        j = 0
        while j < len(line):
            if inside:
                end = line.find("*/", j)
                if end < 0:
                    break
                inside = False
                j = end + 2
            else:
                start = line.find("/*", j)
                slash = line.find("//", j)
                if start < 0 or (0 <= slash < start):
                    break
                inside = True
                j = start + 2
    return out


_FUNC_START = re.compile(r"^(?:static\s+|extern\s+)?[A-Za-z_][\w\s\*]*\b\w+\s*\(")


def _function_start(lines: list[str], code_lines: set[int], use: int) -> int:
    """The line that starts the function around line use (or line 0)."""
    for i in range(use, -1, -1):
        if i in code_lines and _FUNC_START.match(lines[i]) and not lines[i].rstrip().endswith(";"):
            return i
    return 0


def parse_unit(clang: str, root: Path, rel: str, flags: list[str], attempts: int = 12) -> tuple[Unit | None, str]:
    """The AST of root/rel. Returns (None, errors) if Clang still refuses it."""
    path = root / rel
    fixes: list[str] = []
    for _ in range(attempts):
        check = run(clang, Path(rel), root, flags, json_out=False)
        if check.returncode == 0:
            out = run(clang, Path(rel), root, flags, json_out=True)
            if out.returncode != 0:
                return None, out.stderr
            ast = json.loads(out.stdout)
            annotate(ast)
            return Unit(path, ast, fixes), ""
        text = path.read_text(encoding="utf-8", errors="surrogateescape")
        fixed, more = fix_from_diagnostics(text, check.stderr, rel)
        if not more or fixed == text:
            return None, check.stderr
        path.write_text(fixed, encoding="utf-8", errors="surrogateescape")
        fixes += more
    return None, "too many rounds of fixes"


def annotate(ast: dict) -> None:
    """Gives every node its file and line ("_file", "_line").

    Clang's JSON names a location's file and line only when they differ
    from the previous location it printed, so they are followed here in the
    order Clang wrote them.
    """
    state = {"file": "", "line": 0}

    def visit_loc(loc: dict | None):
        if not loc:
            return None
        if "spellingLoc" in loc:
            visit_loc(loc["spellingLoc"])
            return visit_loc(loc["expansionLoc"])
        if "file" in loc:
            # As the units are named (POSIX paths), also where Clang writes "\".
            state["file"] = loc["file"].replace("\\", "/")
        if "line" in loc:
            state["line"] = loc["line"]
        return state["file"], state["line"], loc.get("col", 0)

    def walk(node: dict):
        # Clang 23 spells sizeof/pointer-difference types with internal aliases
        # that have no declaration in the AST. Use the target type it reports,
        # rather than leaking those aliases into the generated host C.
        typ = node.get("type", {})
        if typ.get("qualType") in ("__size_t", "__ptrdiff_t") and "desugaredQualType" in typ:
            typ["qualType"] = typ["desugaredQualType"]
        # Newer Clang puts ownership on the RecordType/EnumType itself instead
        # of an enclosing ElaboratedType. Keep the representation the readers
        # use for typedefs of anonymous records and enums.
        if node.get("isTagOwned") and "decl" in node:
            node.setdefault("ownedTagDecl", node["decl"])
        where = visit_loc(node.get("loc"))
        rng = node.get("range")
        begin = None
        if rng:
            begin = visit_loc(rng.get("begin"))
            visit_loc(rng.get("end"))
        spot = where or begin
        if spot:
            node["_file"], node["_line"], node["_col"] = spot
        for child in node.get("inner", ()):
            walk(child)

    walk(ast)
