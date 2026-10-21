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
- A file that declares `typedef char *va_list;` itself calls it game_va_list: the console's
  va_list, a game pointer, not the host's (own_va_list).
- With candidates (hostgen.json "candidates"), a function the decompilation
  still has as an assembly stub gets the candidate's C at the end of its file
  (use_candidates).
- With aliases (hostgen.json "aliases"), a readable name the decompilation
  defines as a macro for an address name (`#define DrawFlare func_0022F258`)
  is written as that address name, outside comments, strings and `#define`
  lines (unalias): the same code for Clang, and the passes here, which know
  game symbols by their address names, see every definition and call.
"""

from __future__ import annotations

import json
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


# A one-line macro of the unit's own: `#define NAME body` or `#define NAME(a, b) body`.
_DEFINE = re.compile(r"^[ \t]*#[ \t]*define[ \t]+(\w+)(?:\(([^()]*)\))?[ \t]+(\S.*?)[ \t]*$")
_UNDEF = re.compile(r"^[ \t]*#[ \t]*undef[ \t]+(\w+)")


def _expand_line(line: str, name: str, params: list[str] | None, body: str) -> str:
    """LINE with every use of the macro NAME written out (outside strings); a use whose
    arguments do not close on this line, or are not as many as the parameters, stays."""
    out, pos = [], 0
    for m in re.finditer(r'"(?:\\.|[^"\\\n])*"|\b' + re.escape(name) + r"\b", line):
        if m.group(0).startswith('"'):
            continue
        if params is None:
            out.append(line[pos:m.start()] + body)
            pos = m.end()
            continue
        at = m.end()
        while at < len(line) and line[at] in " \t":
            at += 1
        if at >= len(line) or line[at] != "(":
            continue
        depth, end = 0, None
        for i in range(at, len(line)):
            depth += line[i] == "("
            depth -= line[i] == ")"
            if depth == 0:
                end = i
                break
        if end is None:
            continue
        inner = line[at + 1:end]
        cuts = _top_level_commas(inner)
        args = [inner[a + 1:b].strip() for a, b in zip([-1] + cuts, cuts + [len(inner)])]
        if args == [""] and not params:
            args = []
        if len(args) != len(params):
            continue
        given = dict(zip(params, args))
        text = re.sub(r"\b(" + "|".join(map(re.escape, params)) + r")\b",
                      lambda p: given[p.group(1)], body) if params else body
        out.append(line[pos:m.start()] + text)
        pos = end + 1
    return "".join(out) + line[pos:]


def expand_cast_macros(text: str) -> str:
    """A one-line macro of the unit whose body defines a record inside a cast is written out where
    it is used, so name_cast_records finds the record in each function (Clang's JSON leaves a
    record out that a macro defined: rac1/pal's `#define S_I(p, o) (((struct { char _p[o]; int v;
    } *)(p))->v)`). Uses on preprocessor lines, and after an #undef, stay as they are; every line
    stays where it was."""
    if not _CAST_RECORD.search(text) or "define" not in text:
        return text
    lines = text.split("\n")
    for i, line in enumerate(lines):
        m = _DEFINE.match(line)
        if not m or line.endswith("\\") or not _CAST_RECORD.search(m.group(3)) or "#" in m.group(3):
            continue
        name, body = m.group(1), m.group(3)
        params = None if m.group(2) is None else [p.strip() for p in m.group(2).split(",") if p.strip()]
        if params is not None and any(not re.fullmatch(r"[A-Za-z_]\w*", p) for p in params):
            continue  # variadic
        for j in range(i + 1, len(lines)):
            u = _UNDEF.match(lines[j])
            if u and u.group(1) == name:
                break
            if lines[j].lstrip().startswith("#") or name not in lines[j]:
                continue
            lines[j] = _expand_line(lines[j], name, params, body)
    return "\n".join(lines)


# A 128-bit integer written as a type name: "int __attribute__((mode(TI)))" before a "*" or a ")";
# the mode sets the width whatever integer type it is written on ("unsigned long long", "long"...).
_WIDE_CAST = re.compile(r"\b(?:(unsigned|signed)\s+)?(?:long\s+long|long|int|short|char)(?:\s+int)?"
                        r"\s+__attribute__\(\(mode\(TI\)\)\)(?=\s*[*)])")


def name_wide_casts(text: str) -> str:
    """Clang keeps a mode attribute on a declaration and drops it from a type name, so a copy
    through `*(int __attribute__((mode(TI))) *)p` moved 4 bytes of the 16 (8 through `unsigned long
    long __attribute__((mode(TI)))`). The type is written as openrac_ti / openrac_uti
    (openrac_hostgen.h), which carry the attribute in their typedefs."""
    return _WIDE_CAST.sub(lambda m: "openrac_uti" if m.group(1) == "unsigned" else "openrac_ti", text)


_VA_BUILTINS = ("vfprintf", "vprintf", "vsprintf", "vsnprintf", "vfscanf", "vscanf", "vsscanf")
_LABELLED_VA_BUILTIN = re.compile(
    r"\b(" + "|".join(_VA_BUILTINS) + r")\s*\([^;{}]*\)\s*__asm__\(\s*\"(?:FUN|func)_")


def rename_va_builtins(text: str) -> str:
    """A game function that is named like a C library function taking a va_list (the US
    decompilation's `vfprintf(FILE *, const char *, void *) __asm__("FUN_00116da8")`) is renamed
    NAME_game in its file. Clang otherwise takes it for the library's own, gives its definition the
    host's va_list while its body uses the game's pointer, and the translation casts one to the other
    (a struct on Linux on ARM, where it does not compile). The assembler label keeps the symbol."""
    names = set(_LABELLED_VA_BUILTIN.findall(text))
    if not names:
        return text
    word = re.compile(r"\b(" + "|".join(sorted(names)) + r")\b(?=\s*\()")
    out, pos = [], 0
    for m in _SKIP.finditer(text):
        out.append(word.sub(r"\1_game", text[pos:m.start()]))
        out.append(m.group(0))
        pos = m.end()
    out.append(word.sub(r"\1_game", text[pos:]))
    return "".join(out)


_OWN_VA_LIST = re.compile(r"^[ \t]*typedef[ \t]+char[ \t]*\*[ \t]*va_list[ \t]*;", re.M)
_VA_LIST_WORD = re.compile(r"\bva_list\b")


def own_va_list(text: str) -> str:
    """A file that declares `typedef char *va_list;` itself (no <stdarg.h>) means the console's
    va_list: a pointer into the caller's argument area, which its C walks itself (newlib's
    _vfprintf_r reads `*(int *)((ap += 8) - 8)`). hostgen takes the name va_list for the host's
    own type, which a game address cannot stand for, so in such a file the name becomes
    game_va_list, a plain pointer typedef."""
    if not _OWN_VA_LIST.search(text) or re.search(r"#[ \t]*include[ \t]*<stdarg\.h>", text):
        return text
    out = []
    pos = 0
    for m in _SKIP.finditer(text):
        out.append(_VA_LIST_WORD.sub("game_va_list", text[pos:m.start()]))
        out.append(m.group(0))
        pos = m.end()
    out.append(_VA_LIST_WORD.sub("game_va_list", text[pos:]))
    return "".join(out)


# What unalias leaves alone: comments, literals and preprocessor definitions; and the words it reads.
_ALIAS_TOKEN = re.compile(
    r'"(?:\\.|[^"\\\n])*"'
    r"|'(?:\\.|[^'\\\n])*'"
    r"|/\*.*?\*/"
    r"|//[^\n]*"
    r"|^[ \t]*#[ \t]*define\b(?:\\\n|[^\n])*"
    r"|\b[A-Za-z_]\w*\b",
    re.S | re.M,
)
_ALIAS_DEFINE = re.compile(r'^[ \t]*#[ \t]*define[ \t]+([A-Za-z_]\w*)[ \t]+((?:D|func)_\w+)[ \t]*$', re.M)


def read_aliases(text: str) -> dict[str, str]:
    """readable name -> address name, from a header of `#define NAME func_ADDR` lines."""
    return {m.group(1): m.group(2) for m in _ALIAS_DEFINE.finditer(text) if m.group(1) != m.group(2)}


_CARRIED = re.compile(r"\br1_(func|D)_(?:L\d\d_)?[0-9A-Fa-f]{8}\b")


def read_carried_aliases(text: str) -> dict[str, str]:
    """carried name -> address name, from a build's table of derived values (JSON, "values":
    {name: "0x00123456"}). Going Commando's C keeps rac1-decomp's names for what it has not
    named yet (`r1_func_0011DB98`, `r1_D_00130420`: rac1's addresses, not this game's); its own
    build reads their values from the retail bytes. Each becomes the address name of its value
    here (`func_0011F818`, `D_00134DB8`)."""
    values = json.loads(text).get("values", {})
    out = {}
    for name, value in values.items():
        m = _CARRIED.fullmatch(name)
        if m and isinstance(value, str):
            out[name] = f"{m.group(1)}_{int(value, 0):08X}"
    return out


def unalias(text: str, aliases: dict[str, str]) -> str:
    """TEXT with each readable name of ALIASES written as its address name (see the top)."""
    if not aliases:
        return text

    def one(m: re.Match) -> str:
        word = m.group(0)
        if word.lstrip().startswith("#"):
            # A carried name keeps its meaning inside a macro's body too (a `#define SIFCMD`
            # over `r1_D_00158300`); readable names stay as they are there.
            return _CARRIED.sub(lambda c: aliases.get(c.group(0), c.group(0)), word)
        return aliases.get(word, word)
    return _ALIAS_TOKEN.sub(one, text)


def prepare(text: str) -> str:
    return rename_va_builtins(own_va_list(name_wide_casts(name_cast_records(expand_cast_macros(widen_long(split_multiline_strings(blank_file_scope_asm(text))))))))


# The path may be several adjacent strings over as many lines (the NTSC decomp's long paths).
_INCLUDE_ASM = re.compile(r'^[ \t]*(?:INCLUDE_ASM|ASM_FUNC|LINKER_REMNANT)\(\s*(?:"[^"]*"\s*)+,\s*(\w+)\s*\)\s*;?[ \t]*(?:/\*.*?\*/)?[ \t]*$', re.M)


# A file-scope declaration of a game symbol (data or function) with no assembler name of its own.
_GAME_DECL = re.compile(
    r'(?:^|(?<=; /\*hc\*/ ))(?:extern[ \t]+)?[A-Za-z_][\w \t\*]*?[ \t\*]((?:D|func)_\w+)([^;{}=]*);', re.M)
_DEFINED = re.compile(r'^[A-Za-z_][\w \t\*]*?[ \t\*]((?:D|func)_\w+)\s*\([^;{}]*\)\s*\{', re.M)


_MULTI_DECL = re.compile(
    r'^((?:extern[ \t]+)?[A-Za-z_][\w \t]*?)([ \t\*]+(?:D|func)_\w+[^;{}=]*);', re.M)


def _top_level_commas(s: str) -> list[int]:
    depth, at = 0, []
    for i, c in enumerate(s):
        if c in "([":
            depth += 1
        elif c in ")]":
            depth -= 1
        elif c == "," and depth == 0:
            at.append(i)
    return at


def split_declarators(text: str) -> str:
    """One declaration per name: `extern float f(int), g(float);` becomes
    `extern float f(int); /*hc*/ extern float g(float);`, on the same line (the comment marks where
    own_declarations may start a declaration in the middle of a line).

    own_declarations gives each declared name an assembler name after its declarator; with several
    declarators in one statement it would put the first name's assembler name after the last one,
    and every call to one of them would reach another (Batalia's func_L08_002F7318 called the
    int-to-float conversion for sqrt). Commas inside a parameter list stay."""
    def split(m: re.Match) -> str:
        base, rest = m.group(1).rstrip(), m.group(2)
        cuts = _top_level_commas(rest)
        if not cuts:
            return m.group(0)
        parts, last = [], 0
        for c in cuts:
            parts.append(rest[last:c])
            last = c + 1
        parts.append(rest[last:])
        return "; /*hc*/ ".join(base + (p if p[:1] in " \t*" else " " + p) for p in parts) + ";"
    return _MULTI_DECL.sub(split, text)


def own_declarations(text: str, tag: str) -> str:
    """The candidate's declarations of game symbols, under names of its own.

    Several candidates can end up in one file, beside the matched C, and each
    declares what it uses its own way (a table as int * here, as char there).
    C rejects two declarations of one name with different types, so each
    declared name becomes NAME<tag> with the assembler name NAME: the same
    symbol for hostgen, a separate declaration for Clang. Names the candidate
    defines keep theirs."""
    text = split_declarators(text)
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
        text = re.sub(r'(?:^|(?<=; /\*hc\*/ ))((?:extern[ \t]+)?[A-Za-z_][\w \t\*]*?[ \t\*]' + name + tag
                      + r'\b[^;{}=]*?)((?:[ \t]+[A-Z][A-Z0-9_]*(?:\([^;()]*\))?)*)[ \t]*;',
                      lambda m: f'{m.group(1)} __asm__("{name}"){m.group(2)};', text, flags=re.M)
    return text


def own_statics(text: str, tag: str) -> str:
    """The candidate's own file-scope statics (helpers and tables), under names of its own.

    Each candidate is a file of its own in the decompilation, so two may keep a static helper of
    one name (two sky candidates' `sky_sign_set`); put in one file, Clang rejects the second.
    A static is nobody else's symbol, so NAME becomes NAME<tag> wherever the candidate uses it."""
    names = []
    for m in re.finditer(r"^static\b[^;{=(\[]*?\b([A-Za-z_]\w*)[ \t]*[(\[=;]", text, re.M):
        if m.group(1) not in names:
            names.append(m.group(1))
    for name in names:
        text = re.sub(r'"(?:\\.|[^"\\\n])*"|\b' + name + r'\b',
                      lambda m: m.group(0) if m.group(0).startswith('"') else name + tag, text)
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
            suffix = f'_hc{len(added)}'
            tail.append(f"\n/* hostgen: {name} from a candidate, not the matched C */\n"
                        f"{own_statics(own_declarations(without_definitions(prepare(text), matched), suffix), suffix)}")
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


def apply_decomp_workarounds(rel: str, text: str) -> str:
    """Temporary workarounds for upstream decompilation syntax issues.

    FIXME: These conflicting declarations must be fixed upstream in rac1-decomp.
    This is a temporary workaround so that the native port builds cleanly without
    modifying the decompilation repository. Once fixed upstream, this function should be removed.
    """
    if rel.endswith("src/overlays/l13_gemlik/vendor_002EBD00.c"):
        # func_L00_001F2BE8 has conflicting prototypes in local block vs file scope
        idx = text.find("void func_L13_002EE590(")
        end_idx = text.find("extern void func_L00_00251328(", idx) if idx != -1 else -1
        if idx != -1 and end_idx != -1:
            chunk = text[idx:end_idx]
            chunk = re.sub(
                r"extern\s+int\s+func_L00_001F2BE8\s*\(void\s*\*,\s*float,\s*int,\s*void\s*\*,\s*void\s*\);",
                '/* Temporary workaround (FIXME: fix in rac1-decomp) */\n    extern int func_L00_001F2BE8_loc(void *, float, int, void *, void *) __asm__("func_L00_001F2BE8");',
                chunk,
            )
            chunk = chunk.replace("func_L00_001F2BE8(", "func_L00_001F2BE8_loc(")
            text = text[:idx] + chunk + text[end_idx:]

    elif rel.endswith("src/overlays/l14_oltanis/vendor_002E0538.c"):
        # Local declarations in func_L14_002EE990 conflict with file-scope declarations later
        idx = text.find("void func_L14_002EE990(")
        end_idx = text.find("func_001F49B0(func_L14_002EF078", idx) if idx != -1 else -1
        if idx != -1 and end_idx != -1:
            chunk = text[idx:end_idx]
            for sym in ("D_L14_001E00D0", "D_L14_001E0350", "D_L14_001E03A0", "D_L14_001E06A0", "D_L14_00161D40"):
                chunk = re.sub(
                    r"extern\s+([A-Za-z0-9_]+)\s+" + sym + r"(\[[^\]]*\])?(\[[^\]]*\])?(\[[^\]]*\])?;",
                    r'/* Temporary workaround (FIXME: fix in rac1-decomp) */\n    extern \1 ' + sym + r'_loc\2\3\4 __asm__("' + sym + r'");',
                    chunk,
                )
                chunk = re.sub(r"\b" + sym + r"\b(?!_loc|\s*__asm__)", sym + "_loc", chunk)
            text = text[:idx] + chunk + text[end_idx:]

    elif rel.endswith("src/overlays/l18_veldin2/vendor_002F9D48.c"):
        # Local ML_9780 D_L18_0016D2E0 conflicts with ML D_L18_0016D2E0 at line 581
        idx = text.find("void func_L18_002FABE0(")
        end_idx = text.find("extern int func_001FA8A8(", idx) if idx != -1 else -1
        if idx != -1 and end_idx != -1:
            chunk = text[idx:end_idx]
            chunk = re.sub(
                r"extern\s+ML_9780\s+D_L18_0016D2E0;",
                '/* Temporary workaround (FIXME: fix in rac1-decomp) */\n    extern ML_9780 D_L18_0016D2E0_loc __asm__("D_L18_0016D2E0");',
                chunk,
            )
            chunk = re.sub(r"\bD_L18_0016D2E0\b(?!_loc|\s*__asm__)", "D_L18_0016D2E0_loc", chunk)
            text = text[:idx] + chunk + text[end_idx:]

    elif rel.endswith("src/overlays/shared/map_002465F8.c"):
        # Local int D_L00_001842F0[] and char D_00137C80[] conflict with file-scope struct declarations
        idx = text.find("void func_L00_00246EC0(")
        end_idx = text.find("typedef u32 u128", idx) if idx != -1 else -1
        if idx != -1 and end_idx != -1:
            chunk = text[idx:end_idx]
            chunk = re.sub(
                r"extern\s+int\s+D_L00_001842F0\[\];",
                '/* Temporary workaround (FIXME: fix in rac1-decomp) */\n    extern int D_L00_001842F0_loc[] __asm__("D_L00_001842F0");',
                chunk,
            )
            chunk = re.sub(
                r"extern\s+char\s+D_00137C80\[\];",
                '/* Temporary workaround (FIXME: fix in rac1-decomp) */\n    extern char D_00137C80_loc[] __asm__("D_00137C80");',
                chunk,
            )
            chunk = re.sub(r"\bD_L00_001842F0\b(?!_loc|\s*__asm__)", "D_L00_001842F0_loc", chunk)
            chunk = re.sub(r"\bD_00137C80\b(?!_loc|\s*__asm__)", "D_00137C80_loc", chunk)
            text = text[:idx] + chunk + text[end_idx:]

    elif rel.endswith("src/overlays/shared/update_0029B6A0.c"):
        # Local s32 D_L00_00173F00[] conflicts with file-scope struct declaration
        idx = text.find("void func_L00_0029BA20(")
        end_idx = text.find("extern void func_001FFDA0(", idx) if idx != -1 else -1
        if idx != -1 and end_idx != -1:
            chunk = text[idx:end_idx]
            chunk = re.sub(
                r"extern\s+s32\s+D_L00_00173F00\[\];",
                '/* Temporary workaround (FIXME: fix in rac1-decomp) */\n    extern s32 D_L00_00173F00_loc[] __asm__("D_L00_00173F00");',
                chunk,
            )
            chunk = re.sub(r"\bD_L00_00173F00\b(?!_loc|\s*__asm__)", "D_L00_00173F00_loc", chunk)
            text = text[:idx] + chunk + text[end_idx:]

    return text


def copy_tree(src: Path, dst: Path, patterns=("*.c", "*.h", "*.inc"),
              candidates: dict[str, str] | None = None, used: set[str] | None = None,
              drafts: tuple[str, set[str]] | None = None, prefix: str = "",
              aliases: dict[str, str] | None = None) -> int:
    """Prepares every C file under src into dst; only rewrites what changed.
    aliases (readable name -> address name) are written as their address names first (unalias).
    candidates (function name -> C) replace their assembly stubs (use_candidates);
    the names used are added to used. drafts (a guard macro, the files it applies to, named
    from the decompilation's top with prefix as src's own name) has those files' own drafts
    read (use_draft)."""
    count = 0
    for pattern in patterns:
        for path in src.rglob(pattern):
            out = dst / path.relative_to(src)
            text = prepare(unalias(path.read_text(encoding="utf-8", errors="surrogateescape"), aliases or {}))
            if candidates and path.suffix == ".c":
                text = use_candidates(text, candidates, used if used is not None else set())
            if drafts and (Path(prefix) / path.relative_to(src)).as_posix() in drafts[1]:
                text = use_draft(text, drafts[0])
            rel = (Path(prefix) / path.relative_to(src)).as_posix()
            text = apply_decomp_workarounds(rel, text)
            if not out.exists() or out.read_text(encoding="utf-8", errors="surrogateescape") != text:
                out.parent.mkdir(parents=True, exist_ok=True)
                out.write_text(text, encoding="utf-8", errors="surrogateescape")
            count += 1
    # A file the decompilation moved or removed leaves its old copy behind, which would define
    # its functions a second time.
    if dst.is_dir():
        for pattern in patterns:
            for out in dst.rglob(pattern):
                if not (src / out.relative_to(dst)).exists():
                    out.unlink()
    return count
