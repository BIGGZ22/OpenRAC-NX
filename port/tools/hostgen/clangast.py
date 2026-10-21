# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""Clang reads one translation unit as the console's C and gives its AST.

The unit is read for a 32-bit little-endian MIPS target, so every type has
the console's size and layout, in GNU C89 as GCC 2.95 accepted it. What
Clang refuses that GCC 2.95 accepted is fixed in the prepared copy, driven
by Clang's own diagnostics (fix_from_diagnostics), and recorded.
"""

from __future__ import annotations

import contextlib
import json
import os
import re
import shutil
import subprocess
import time
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


# Clang's internal aliases for the target's size_t and ptrdiff_t (mipsel: 32-bit).
_TARGET_TYPES = {"__size_t": "unsigned int", "__ptrdiff_t": "int"}
_CLANG_ALIASES = re.compile(r"\b(?:__size_t|__ptrdiff_t)\b")


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
    - A later global declaration that contradicts a function-local extern
      gets its own C name and the same assembler symbol.
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
        note = diags[k + 1] if k + 1 < len(diags) else None
        if (d["kind"] == "error" and d["msg"].startswith("conflicting asm label")
                and not d["file"].replace("\\", "/").endswith(rel) and note is not None and note["kind"] == "note"
                and note["file"].replace("\\", "/").endswith(rel)):
            # A header gives a function another label than a declaration earlier in this file
            # (FUN_001f99c0 there, func_001F99C0 here). GCC kept the first: this file's declaration
            # and its uses take an alias name with its own label, and the header's stands alone.
            at = int(note["line"]) - 1
            lm = re.search(r"\b(\w+)\s*\([^;{]*\)\s*(?:__asm__|asm)\s*\(\s*\"([^\"]+)\"\s*\)", lines[at])
            if lm:
                name = lm.group(1)
                alias = f"{name}__openrac_decl_{at + 1}"
                lines[at] = lines[at][:lm.start(1)] + alias + lines[at][lm.end(1):]
                for j in range(at + 1, len(lines)):
                    lines[j] = re.sub(rf"\b{name}\b", alias, lines[j])
                fixes.append(f"line {at + 1}: {name}, labelled {lm.group(2)} here and otherwise in a header, kept apart")
            continue
        if d["kind"] != "error" or not d["file"].replace("\\", "/").endswith(rel):
            continue
        msg = d["msg"]
        line = int(d["line"]) - 1
        col = int(d["col"]) - 1
        m = re.match(r"redeclaration of '(\w+)' with a different type", msg)
        # an extern of a variable: plain, of an anonymous struct ({...}), or with attributes
        if m and re.match(r"^\s*extern\b(?:[^;{}]|\{[^{}]*\})*;\s*$", lines[line]) \
                and not re.search(rf"\b{m.group(1)}\s*\(", lines[line]):
            name = m.group(1)
            alias = f"{name}__openrac_decl_{line + 1}"
            decl = re.sub(rf"\b{name}\b", alias, lines[line], count=1)
            if not re.search(r"\b(?:__asm__|asm|SDATA|SBSS)\b", decl):
                label = f' __asm__("{name}")'
                attrs = re.search(r"((?:[ \t]+(?:__attribute__\s*\(\([^;]*?\)\)|[A-Z][A-Z0-9_]*(?:\([^;()]*\))?))+)\s*;\s*$", decl)
                if attrs:
                    pos = attrs.start(1)
                    decl = decl[:pos] + label + decl[pos:]
                else:
                    decl = re.sub(r";\s*$", label + ";", decl)
            lines[line] = decl
            for j in range(line + 1, len(lines)):
                lines[j] = re.sub(rf"\b{name}\b", alias, lines[j])
            fixes.append(f"line {line + 1}: global declaration of {name} kept apart from a local extern")
            continue
        m = re.match(r"(?:conflicting types for|redefinition of) '(\w+)'", msg)
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
        if msg.startswith(("cannot apply asm label to variable after its first use", "conflicting asm label")):
            # A variable's label: GCC 2.95 kept the first label a name was given (rac1/ntsc's
            # l16 rocket.c reads D_0015ED6C where a later line says D_0015EE6C), and took a label
            # after a use. The label at the error's column goes; when it names another symbol than
            # the variable's own, the name is recorded as an alias of it, which hostgen follows.
            src = lines[line]
            vm = re.compile(r"\b(\w+)(\s*\[[^\]]*\])?\s*(?:__asm__|asm)\s*\(\s*\"([^\"]+)\"\s*\)").search(src, max(col - 1, 0))
            if vm and vm.start() <= col + 1 and not re.match(r"\s*\(", src[vm.end(1):]):
                lines[line] = src[:vm.end(2) if vm.group(2) else vm.end(1)] + src[vm.end():]
                if msg.startswith("conflicting"):
                    fixes.append(f"line {line + 1}: second asm label {vm.group(3)} for {vm.group(1)} removed (GCC kept the first)")
                elif vm.group(3) != vm.group(1):
                    fixes.append(f"alias {vm.group(1)} {vm.group(3)}")
                else:
                    fixes.append(f"line {line + 1}: asm label repeating {vm.group(1)} after its use removed")
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


def drop_superseded_declarations(root: Path, rel: str, text: str, stderr: str) -> list[str]:
    """A function this unit defines, declared otherwise by an earlier line of a header.

    "conflicting types for 'f'" at f's definition, with a note that the
    previous declaration is in another file (a header that declares every
    callee as `void *`, say). The definition is the function's real
    signature, so the header's one-line declaration is blanked in the
    prepared copy of the header (no line moves). Returns what was done."""
    done = []
    diags = list(_DIAG.finditer(stderr))
    for k, d in enumerate(diags[:-1]):
        m = re.match(r"conflicting types for '(\w+)'", d["msg"])
        note = diags[k + 1]
        if d["kind"] != "error" or not m or note["kind"] != "note" or not note["msg"].startswith("previous declaration is here"):
            continue
        name = m.group(1)
        if not d["file"].replace("\\", "/").endswith(rel):
            continue
        defline = text.split("\n")[int(d["line"]) - 1]
        if ";" in defline.split("{")[0] or "(" not in defline:
            continue  # a declaration, not a definition: fix_from_diagnostics aliases it
        other = Path(note["file"])
        other = other if other.is_absolute() else root / other
        if not other.is_file() or other.resolve() == (root / rel).resolve():
            continue
        with _header_lock(root):
            fix = _supersede(other, int(note["line"]) - 1, name, rel, defline)
        if fix:
            done.append(fix)
    return done


@contextlib.contextmanager
def _header_lock(root: Path):
    """One unit at a time rewrites the prepared headers: units are read in parallel, and two
    that each read a header, changed a line of it and wrote it back left only one of the two
    changes."""
    lock = root / ".openrac-header.lock"
    for _ in range(6000):
        try:
            os.mkdir(lock)
            break
        except FileExistsError:
            time.sleep(0.01)
    else:
        raise RuntimeError(f"hostgen: {lock} was held for a minute; remove it if no hostgen is running")
    try:
        yield
    finally:
        os.rmdir(lock)


def _supersede(other: Path, at: int, name: str, rel: str, defline: str) -> str | None:
    """drop_superseded_declarations' change to one header line, under the header lock."""
    lines = other.read_text(encoding="utf-8", errors="surrogateescape").split("\n")
    if at >= len(lines) or name not in lines[at] or "{" in lines[at] or not lines[at].rstrip().endswith(";"):
        return None
    # The header's line becomes the definition's own prototype, so the other files that take the
    # function from that header still have it (a matched debug menu that calls sound_update
    # by the header's declaration, while a candidate defines it as returning int). A
    # definition whose signature does not fit on its first line, or a static one, is dropped.
    sig = defline.split("{")[0].strip()
    whole = sig.endswith(")") and sig.count("(") == sig.count(")") and not sig.startswith("static")
    lines[at] = f"{sig};" if whole else ""
    _write(other, "\n".join(lines))
    return (f"{other.name}:{at + 1}: declaration of {name} "
            + (f"replaced by {rel}'s definition" if whole else f"dropped, {rel} defines it differently"))


def _bracketed(s: str, at: int) -> int | None:
    """The index of the "]" that closes the "[" at s[at], or None."""
    depth = 0
    for i in range(at, len(s)):
        depth += s[i] == "["
        depth -= s[i] == "]"
        if depth == 0:
            return i
    return None


def name_vla_lengths(text: str, ast: dict, rel: str) -> tuple[str, list[str]]:
    """A variable-length array whose length is an expression (`char s[*(int *)(d + 4) * 64]`)
    gets that length in a local of its own, computed just before it on the same line:
    `unsigned int s__vlan = (...); char s[s__vlan];`. The length is still computed once, where the
    array is declared; the lowering takes a length that is a local by name. No line moves."""
    raw = text.encode("utf-8", errors="surrogateescape")
    edits = []

    def walk(node: dict):
        loc, rng = node.get("loc") or {}, node.get("range") or {}
        if (node.get("kind") == "VarDecl" and node.get("_file", "").endswith(rel)
                and "offset" in loc and "spellingLoc" not in loc and "offset" in rng.get("begin", {})):
            qual = node.get("type", {}).get("qualType", "")
            name = node.get("name", "")
            b = qual.find("[")
            e = _bracketed(qual, b) if b != -1 else None
            first = qual[b + 1:e].strip() if e is not None else ""
            if first and not re.fullmatch(r"\d+|[A-Za-z_]\w*", first):
                at = loc["offset"] + len(name)
                if raw[loc["offset"]:at].decode("utf-8", "surrogateescape") == name:
                    while at < len(raw) and raw[at:at + 1] in b" \t":
                        at += 1
                    close = _bracketed(raw.decode("latin-1"), at) if raw[at:at + 1] == b"[" else None
                    if close is not None:
                        expr = raw[at + 1:close].decode("utf-8", "surrogateescape")
                        if "\n" not in expr and "//" not in expr and "/*" not in expr:
                            edits.append((rng["begin"]["offset"], at + 1, close, name, expr.strip()))
        for c in node.get("inner", []):
            walk(c)

    walk(ast)
    if not edits:
        return text, []
    # Several arrays of one declaration (`float u[a + 1], v[b + 1];`) share its start: each change
    # is made from the end of the file back, the lengths before the locals in front of them.
    done, ops = [], []
    for k, (begin, start, close, name, expr) in enumerate(sorted(edits)):
        local = f"{name}__vlan"
        ops.append((start, 1, close, local.encode()))
        ops.append((begin, 0, begin, f"unsigned int {local} = ({expr}); ".encode(), k))
        done.append(f"variable-length array {name}: its length {expr} in {local}")
    for op in sorted(ops, key=lambda o: (o[0], o[1], o[4] if len(o) > 4 else 0), reverse=True):
        at, _, end, new = op[:4]
        raw = raw[:at] + new + raw[end:]
    return raw.decode("utf-8", errors="surrogateescape"), done


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
            text = path.read_text(encoding="utf-8", errors="surrogateescape")
            named, more = name_vla_lengths(text, ast, rel)
            if more:
                _write(path, named)
                fixes += more
                continue
            return Unit(path, ast, fixes), ""
        text = path.read_text(encoding="utf-8", errors="surrogateescape")
        fixed, more = fix_from_diagnostics(text, check.stderr, rel)
        header_fixes = drop_superseded_declarations(root, rel, text, check.stderr)
        if header_fixes:
            fixes += header_fixes
            continue
        if not more or fixed == text:
            return None, check.stderr
        _write(path, fixed)
        fixes += more
    return None, "too many rounds of fixes"


def _write(path: Path, text: str) -> None:
    """Writes the prepared copy again. On Windows a file another program has open cannot be
    written, and an editor's indexer or a virus scanner opens a file just written for a moment:
    the write is tried again for a few seconds before it is given up.

    The new text goes to a file of its own first and then takes the old one's place: a shared
    header is rewritten while other units are being read (drop_superseded_declarations), and a
    Clang that opened it half written read a header with no declarations in it."""
    tmp = path.with_name(f"{path.name}.{os.getpid()}.tmp")
    for attempt in range(60):
        try:
            tmp.write_text(text, encoding="utf-8", errors="surrogateescape")
            os.replace(tmp, path)
            return
        except PermissionError:
            if attempt == 59:
                raise
            time.sleep(0.05)


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
        # Newer Clang (Fedora's, 22 and later) spells sizeof and pointer-difference
        # types with internal aliases that have no declaration in the AST, also
        # inside larger types (a function's, a pointer's). They are the target's,
        # mipsel's: written as such, rather than leaking into the generated host C.
        typ = node.get("type", {})
        if typ.get("qualType") in ("__size_t", "__ptrdiff_t") and "desugaredQualType" in typ:
            typ["qualType"] = typ["desugaredQualType"]
        for key in ("qualType", "desugaredQualType"):
            text = typ.get(key)
            if text and "__" in text:
                typ[key] = _CLANG_ALIASES.sub(lambda m: _TARGET_TYPES[m.group(0)], text)
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
