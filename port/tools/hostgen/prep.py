# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""The decompilation's sources, prepared for Clang to read as the console's C.

hostgen never changes the decompilation. It copies the files it reads into
its own build directory and changes the copies in ways that keep every line
where it was:

- `long` becomes `long long`: the EE's GCC has a 64-bit long, where Clang's
  32-bit MIPS target has a 32-bit one. `long long` stays as it is. So does an
  integer constant's L suffix: `1UL << 60` is a 64-bit shift on the console.
- File-scope `__asm__(...)` statements are blanked: they only lay out the
  matching build's object files (padding, sections).
- A string literal that runs across lines, which GCC 2.95 accepted, becomes
  one literal per line, concatenated: the same string, the same lines.
- A struct or union defined inside a cast, `((struct { ... } *)p)->field`,
  gets a name and is defined on the first line of its function: Clang's JSON
  leaves out a record defined inside an expression, so hostgen could not lay
  it out and the whole function became a stub.
- With candidates (hostgen.json "candidates"), a function the decompilation
  still has as an assembly stub gets the candidate's C at the end of its file
  (use_candidates).
"""

from __future__ import annotations

import re
from pathlib import Path

# A C token that can contain the word long without being the keyword.
_SKIP = re.compile(
    r'"(?:\\.|[^"\\])*"'       # string literal (one line)
    r"|'(?:\\.|[^'\\])*'"      # character literal
    r"|/\*.*?\*/"              # block comment
    r"|//[^\n]*",              # line comment
    re.S,
)
_LONG = re.compile(r"\blong\b(\s+(?:long|double)\b)?")
# An integer constant with one L in its suffix (L, UL, LU), not LL.
_LONG_SUFFIX = re.compile(r"\b((?:0[xX][0-9A-Fa-f]+|\d+)(?:[uU]?))([lL])(?![lL])([uU]?)\b")


def widen_long(text: str) -> str:
    """Every `long` that is not part of `long long` or `long double` becomes `long long`."""
    out = []
    pos = 0
    for m in _SKIP.finditer(text):
        out.append(_widen(text[pos:m.start()]))
        out.append(m.group(0))
        pos = m.end()
    out.append(_widen(text[pos:]))
    return "".join(out)


def _widen(code: str) -> str:
    code = _LONG.sub(lambda m: m.group(0) if m.group(1) else "long long", code)
    return _LONG_SUFFIX.sub(lambda m: m.group(1) + m.group(2) * 2 + m.group(3), code)


def blank_file_scope_asm(text: str) -> str:
    """Blanks `__asm__(...);` statements that start a line at file scope."""
    chars = list(text)
    depth = 0          # braces: file scope is depth 0
    i = 0
    n = len(text)
    while i < n:
        c = text[i]
        if c == "/" and text.startswith("/*", i):
            end = text.find("*/", i + 2)
            i = n if end < 0 else end + 2
            continue
        if c == "/" and text.startswith("//", i):
            end = text.find("\n", i)
            i = n if end < 0 else end
            continue
        if c in "\"'":
            i = _skip_literal(text, i)
            continue
        if c == "{":
            depth += 1
        elif c == "}":
            depth -= 1
        elif depth == 0 and (i == 0 or text[i - 1] == "\n") and text.startswith("__asm__", i):
            end = _statement_end(text, i)
            if end is not None:
                for j in range(i, end):
                    if chars[j] != "\n":
                        chars[j] = " "
                i = end
                continue
        i += 1
    return "".join(chars)


def _skip_literal(text: str, i: int) -> int:
    quote = text[i]
    i += 1
    while i < len(text) and text[i] != quote:
        i += 2 if text[i] == "\\" else 1
    return i + 1


def _statement_end(text: str, i: int) -> int | None:
    """The index after the `;` that ends the asm statement starting at i."""
    j = text.find("(", i)
    if j < 0:
        return None
    depth = 0
    while j < len(text):
        c = text[j]
        if c in "\"'":
            j = _skip_literal(text, j)
            continue
        if c == "(":
            depth += 1
        elif c == ")":
            depth -= 1
            if depth == 0:
                k = j + 1
                while k < len(text) and text[k] in " \t\r\n":
                    k += 1
                return k + 1 if k < len(text) and text[k] == ";" else j + 1
        j += 1
    return None


def split_multiline_strings(text: str) -> str:
    """Ends a string literal at each line break inside it and reopens it on
    the next line, keeping the newline as \\n."""
    out = []
    i, n = 0, len(text)
    while i < n:
        c = text[i]
        if c == "/" and text.startswith("/*", i):
            end = text.find("*/", i + 2)
            end = n if end < 0 else end + 2
            out.append(text[i:end])
            i = end
            continue
        if c == "/" and text.startswith("//", i):
            end = text.find("\n", i)
            end = n if end < 0 else end
            out.append(text[i:end])
            i = end
            continue
        if c == "'":
            end = _skip_literal(text, i)
            out.append(text[i:end])
            i = end
            continue
        if c == '"':
            out.append(c)
            i += 1
            while i < n and text[i] != '"':
                if text[i] == "\\" and i + 1 < n:  # an escape, or a continued line: kept
                    out.append(text[i:i + 2])
                    i += 2
                    continue
                if text[i] == "\n":
                    out.append('\\n"\n"')
                else:
                    out.append(text[i])
                i += 1
            if i < n:
                out.append('"')
                i += 1
            continue
        out.append(c)
        i += 1
    return "".join(out)


# A cast to a record defined on the spot, without a tag: "(struct { ... }".
_CAST_RECORD = re.compile(r"\(\s*(struct|union)\s*\{([^{}]*)\}")
# The first line of a function's definition (clangast's _FUNC_START).
_FUNCTION_START = re.compile(r"^(?:static\s+|extern\s+)?[A-Za-z_][\w\s\*]*\b\w+\s*\(")


def name_cast_records(text: str) -> str:
    """A record defined inside a cast gets a name (openrac_cast_LINE_N) and its definition goes
    to the start of the first line of the function around it; every line stays where it was."""
    if not _CAST_RECORD.search(text):
        return text
    lines = text.split("\n")
    out = []
    last = 0
    definitions: dict[int, list[str]] = {}
    for m in _CAST_RECORD.finditer(text):
        line = text.count("\n", 0, m.start())
        start = None
        for i in range(line, -1, -1):
            if i != line and lines[i].startswith("}"):
                break  # the end of the function before: this cast is in none
            if _FUNCTION_START.match(lines[i]) and not lines[i].rstrip().endswith(";"):
                start = i
                break
        if start is None:
            continue  # not in a function (a macro in a header): left as it is
        name = f"openrac_cast_{line + 1}_{len(definitions.get(start, []))}"
        body = " ".join(m.group(2).split())
        definitions.setdefault(start, []).append(f"{m.group(1)} {name} {{ {body} }};")
        # The lines the record ran over are kept, empty.
        out.append(text[last:m.start()] + f"({m.group(1)} {name}" + "\n" * m.group(0).count("\n"))
        last = m.end()
    lines = ("".join(out) + text[last:]).split("\n")
    for start, records in definitions.items():
        lines[start] = " ".join(records) + " " + lines[start]
    return "\n".join(lines)


# A 128-bit integer written as a type name: "int __attribute__((mode(TI)))" before a "*" or a ")".
_WIDE_CAST = re.compile(r"\b(unsigned\s+)?int\s+__attribute__\(\(mode\(TI\)\)\)(?=\s*[*)])")


def name_wide_casts(text: str) -> str:
    """Clang keeps a mode attribute on a declaration and drops it from a type name, so a copy
    through `*(int __attribute__((mode(TI))) *)p` moved 4 bytes of the 16. The type is written as
    openrac_ti / openrac_uti (openrac_hostgen.h), which carry the attribute in their typedefs."""
    return _WIDE_CAST.sub(lambda m: "openrac_uti" if m.group(1) else "openrac_ti", text)


def prepare(text: str) -> str:
    return name_wide_casts(name_cast_records(widen_long(split_multiline_strings(blank_file_scope_asm(text)))))


# The path may be several adjacent strings over as many lines (the NTSC decomp's long paths).
_INCLUDE_ASM = re.compile(r'^[ \t]*(?:INCLUDE_ASM|ASM_FUNC|LINKER_REMNANT)\(\s*(?:"[^"]*"\s*)+,\s*(\w+)\s*\)\s*;?[ \t]*(?:/\*.*?\*/)?[ \t]*$', re.M)


# A file-scope declaration of a game symbol (data or function) with no assembler name of its own.
_GAME_DECL = re.compile(
    r'^(?:extern[ \t]+)?[A-Za-z_][\w \t\*]*?[ \t\*]((?:D|func)_\w+)([^;{}=]*);', re.M)
_DEFINED = re.compile(r'^[A-Za-z_][\w \t\*]*?[ \t\*]((?:D|func)_\w+)\s*\([^;{}]*\)\s*\{', re.M)


def own_declarations(text: str, tag: str) -> str:
    """The candidate's declarations of game symbols, under names of its own.

    Several candidates can end up in one file, beside the matched C, and each
    declares what it uses its own way (a table as int * here, as char there).
    C rejects two declarations of one name with different types, so each
    declared name becomes NAME<tag> with the assembler name NAME: the same
    symbol for hostgen, a separate declaration for Clang. Names the candidate
    defines keep theirs."""
    defined = set(_DEFINED.findall(text))
    names = []
    for m in _GAME_DECL.finditer(text):
        name, tail = m.group(1), m.group(2)
        if name in defined or "__asm__" in tail or "SDATA(" in tail or "__attribute__" in tail or "," in tail.split("(")[0]:
            continue
        if name not in names:
            names.append(name)
    for name in names:
        text = re.sub(r'"(?:\\.|[^"\\\n])*"|\b' + name + r'\b',
                      lambda m: m.group(0) if m.group(0).startswith('"') else name + tag, text)
        # The assembler name goes after the declarator, before attribute macros (MACRO_ADDR).
        text = re.sub(r'^((?:extern[ \t]+)?[A-Za-z_][\w \t\*]*?[ \t\*]' + name + tag
                      + r'\b[^;{}=]*?)((?:[ \t]+[A-Z][A-Z0-9_]*(?:\([^;()]*\))?)*)[ \t]*;',
                      lambda m: f'{m.group(1)} __asm__("{name}"){m.group(2)};', text, flags=re.M)
    return text


def without_definitions(text: str, names: set[str]) -> str:
    """TEXT without the definitions of the functions NAMES (each from its first line to its closing
    brace), a comment in the place of each.

    One file of the port's own C can hold several functions (hand/): when the decompilation
    matches one of them, the file it is put into for the others would define that one twice, and
    Clang refuses the whole file. The matched C stands; the copy steps aside."""
    for name in names:
        while True:
            m = re.search(r'^[A-Za-z_][\w \t\*]*?[ \t\*]' + re.escape(name) + r'\s*\([^;{}]*\)\s*\{', text, re.M)
            if not m:
                break
            depth, at = 0, m.end() - 1
            while at < len(text):
                c = text[at]
                if c == '"' or c == "'":            # a brace in a string or a character is not one
                    close = at + 1
                    while close < len(text) and text[close] != c:
                        close += 2 if text[close] == "\\" else 1
                    at = close
                elif text.startswith("/*", at):
                    at = text.find("*/", at) + 1
                elif text.startswith("//", at):
                    at = text.find("\n", at)
                elif c == "{":
                    depth += 1
                elif c == "}":
                    depth -= 1
                    if depth == 0:
                        break
                at += 1
            text = (text[:m.start()] + f"/* hostgen: {name} is matched C in this file now; this copy is left out */"
                    + text[at + 1:])
    return text


def use_candidates(text: str, candidates: dict[str, str], used: set[str]) -> str:
    """Puts candidate C in place of the assembly stubs it is written for.

    A candidate is C for a function the decompilation still has as assembly:
    a near miss that does what the retail function does without matching its
    bytes. The stub's line is blanked and the candidate goes at the end of the
    file, after every declaration it may rely on, so no line of the file moves."""
    tail = []
    added: set[int] = set()
    matched = set(_DEFINED.findall(text))   # what this file has as C of its own

    def blank(m: re.Match) -> str:
        name = m.group(1)
        if name not in candidates:
            return m.group(0)
        used.add(name)
        text = candidates[name]
        if id(text) not in added:   # one file of C for several functions goes in once
            added.add(id(text))
            tail.append(f"\n/* hostgen: {name} from a candidate, not the matched C */\n"
                        f"{own_declarations(without_definitions(prepare(text), matched), f'_hc{len(added)}')}")
        return "\n" * m.group(0).count("\n")   # a stub over several lines: the lines stay

    text = _INCLUDE_ASM.sub(blank, text)
    return text + "".join(tail)


def use_draft(text: str, guard: str) -> str:
    """Takes a file's own draft in place of its assembly stub: a decompilation that keeps pending
    C under the `#else` of `#ifndef GUARD` around the stub (the NTSC decomp's NON_MATCHING) has
    that branch read. Only where the stub is still there: a candidate that replaced it stands."""
    def swap(m: re.Match) -> str:
        return f"#if 0 /* hostgen: the draft below, not the stub */{m.group(1)}" if _INCLUDE_ASM.search(m.group(1)) else m.group(0)
    return re.sub(r"^#ifndef[ \t]+" + re.escape(guard) + r"\b[^\n]*(\n.*?^#else\b)", swap, text, flags=re.M | re.S)


def copy_tree(src: Path, dst: Path, patterns=("*.c", "*.h", "*.inc"),
              candidates: dict[str, str] | None = None, used: set[str] | None = None,
              drafts: tuple[str, set[str]] | None = None, prefix: str = "") -> int:
    """Prepares every C file under src into dst; only rewrites what changed.
    candidates (function name -> C) replace their assembly stubs (use_candidates);
    the names used are added to used. drafts (a guard macro, the files it applies to, named
    from the decompilation's top with prefix as src's own name) has those files' own drafts
    read (use_draft)."""
    count = 0
    for pattern in patterns:
        for path in src.rglob(pattern):
            out = dst / path.relative_to(src)
            text = prepare(path.read_text(encoding="utf-8", errors="surrogateescape"))
            if candidates and path.suffix == ".c":
                text = use_candidates(text, candidates, used if used is not None else set())
            if drafts and (Path(prefix) / path.relative_to(src)).as_posix() in drafts[1]:
                text = use_draft(text, drafts[0])
            if not out.exists() or out.read_text(encoding="utf-8", errors="surrogateescape") != text:
                out.parent.mkdir(parents=True, exist_ok=True)
                out.write_text(text, encoding="utf-8", errors="surrogateescape")
            count += 1
    return count
