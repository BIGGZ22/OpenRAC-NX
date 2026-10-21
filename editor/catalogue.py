"""The moby behaviour catalogue for the mod SDK (experimental: names follow the decompilation and will
change in the C++ refactor; the hook names are the stable handle): for each Ratchet & Clank (PAL) moby class, the
function that updates it, what its pvars hold, and how a native mod can take that function.

    catalogue.py write           # regenerate sdk/catalogue/rac1/
    catalogue.py check           # exit 1 when those files are stale
    catalogue.py show <class>    # one class's entry, as JSON, on stdout

Writes, in sdk/catalogue/rac1/ (shipped with the mod SDK, sdk/tools/package_sdk.py):

    classes.json     the stable table for bindings and mods (format below): per class, its update
                     functions and their hook names, and its pvar fields
    behaviour.json   the catalogue, for tools (format below)
    behaviour.tsv    one row per class, the same facts flattened
    BEHAVIOUR.md     the catalogue to read: overview, hook points, update functions, pvars

Everything comes from the source tree, never from the disc: the decompilation (games/rac1:
config/overlays/names.tsv for the UpdateMoby_<class> roles, config/overlays/functions.tsv for
where each function sits in each level, include/names.h for readable names, src/ for the C,
nonmatching/functional.tsv for candidate C, include/moby.h for the moby record,
include/moby_pvars.h for the typed pvars), the port's tables for the PAL and NTSC-U discs
(port/game/rac1-pal: libraries.tsv for hook and wrap rows, split_places.tsv, hand/hand.tsv,
no_c.tsv; port/game/rac1-ntsc/libraries.tsv), the SDK's game header
(sdk/include/openrac/games/rac1.h), editor/moby_classes.tsv and editor/engine_helpers.tsv.

classes.json, format 1, the stable contract. Fields are only added within a format; one that
changes meaning or goes raises the format. One class per line:

    {"format": 1, "game": "rac1", "disc": "pal", "generated_by": "editor/catalogue.py",
     "moby": {"struct": "Moby", "size": 256,
              "class_field": {"name": "oclass", "offset": 166, "type": "s16"},   # filter a shared hook on it
              "update_field": {...}, "pvars_field": {...}},                     # +0x74, +0x78
     "classes": [{
         "id": 500, "name": "bolt_crate" or null,     # editor/moby_classes.tsv
         "levels": [level numbers it is updated in],
         "updates": [{                                # usually one; a class updated by another function
             "function": "func_L00_002D06C8",         #   in some levels has one entry per function
             "name": readable name or null,
             "levels": [...],
             "hook": "Moby_BoltCrate_Update",         # the hook point's name for mods: Moby_<Class>_Update
                                                      #   after the function's first named class (MODS.md 4)
             "hook_status": "available" | "needs_hook_row" | "wrapped" | "no_signature" | "needs_c",
             "signature": "void(u32)" or null,        # MODS.md notation; u32 is the moby's game address
             "gets_moby": true,                       # false: the C takes no parameter
             "shared_with": [other class ids the same function updates]}],
         "pvars": [{"struct": "update500", "size": 0x.., "checked": bool,   # false: the header's stated
                    "fields": [{"name", "offset", "type", "meaning"}]}]}]}  #   size disagrees

A hook name is one function's: every class in shared_with reaches the same hook point, so a
replacement for one class checks the class field and calls the original for the others.

behaviour.json, format 1 (everything, for tools; may grow):

    {"format": 1, "game": "rac1", "disc": "pal", "generated_by": "editor/catalogue.py",
     "moby": {"size", "class_field", "update_field", "pvars_field"},   # {name, offset, type}
     "levels": ["Veldin", ...],                      # level number -> name
     "counts": {...},
     "hook_points": {name: {symbol, ntsc_symbol, signature, in_sdk_header}},
     "functions": {symbol: {                         # each update function, by its PAL symbol
         "name": readable name or null, "classes": [ids], "levels": [where its code is placed],
         "split": {variant symbol: [levels]},        # places split_places.tsv gives another function
         "source": {"kind", "file", "defined_as"},   # kind: decomp | candidate | hand | assembly | none
         "in_port": bool, "signature": "void(u32)" or null,
         "port_row": libraries.tsv kind of another row for it (wrap, host...) or null,
         "hook": {"status", "point", "proposed", "row", "covers", "missing"},
         "pvar_struct": the struct its C reads the pvars as, or null,
         "calls": [symbols]}},                       # the game functions its C calls, in order
     "helpers": {symbol: {name, engine_helper, called_by, in_port, signature, hook_point}},
     "pvar_structs": {name: {size, declared_size, file, classes, fields: [{offset, declared_offset,
                             type, name, comment}]}},
     "classes": [{"id", "name", "levels", "updates": [{function, levels}], "shared_with": [ids],
                  "pvars": [struct names]}]}

hook.status is one of: hook_point (a hook point exists: the mod names it), needs_hook_row (C in the
port with a signature a hook can take: one `hook` row in libraries.tsv makes it one; `row` is that
row), wrapped (the port has its own row for it, so a hook row would clash), no_signature (the C
has a parameter a hook cannot pass), needs_c (no C in the port's build: a stub).

A level function (func_Lnn_...) is one C function placed in every level that has its code; its
hook point, like every function of it, is the same host function at each of those places
(hostgen's functions.c), so one row covers every level listed, but not the places split_places.tsv
gives to a variant. Signatures are as MODS.md section 4 writes them, read from the C's
definition; hostgen's game_protos.h is what decides in the end.
"""

import argparse
import json
import re
import sys
from collections import Counter
from dataclasses import dataclass, field
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))

from cparse import Types, strip
from mobys import CLASS_NAMES, DECOMPILATION, LEVEL_NAMES, c_definitions, names_h_symbols, update_roles

HERE = Path(__file__).parent
REPO = HERE.parent
PAL = REPO / "port" / "game" / "rac1-pal"
NTSC = REPO / "port" / "game" / "rac1-ntsc"
SDK_HEADER = REPO / "sdk" / "include" / "openrac" / "games" / "rac1.h"
OUT = REPO / "sdk" / "catalogue" / "rac1"
PVARS_HEADER = "moby_pvars.h"
# The decompilation's headers the struct layouts are read from (games/rac1/include).
TYPE_HEADERS = ("common.h", "vector.h", "moby.h", "hero.h", "pad.h", "records.h", "structs.h")
CLASSES_FORMAT = 1  # classes.json: raised only when a field changes meaning or goes
KEYWORDS = {"if", "while", "for", "switch", "return", "sizeof", "do", "else", "case"}
SYMBOL = re.compile(r"(?:func|FUN)_(?:L\d\d_)?[0-9A-Fa-f]{8}")

# A C type as a hook signature writes it (docs/port/MODS.md, section 4), and back as libraries.tsv does.
MODS_TYPES = {"void": "void", "int": "i32", "s32": "i32", "signed": "i32", "signed int": "i32", "long": "i32",
              "u32": "u32", "unsigned": "u32", "unsigned int": "u32", "unsigned long": "u32", "uint": "u32",
              "float": "f32", "f32": "f32", "short": "i16", "s16": "i16", "signed short": "i16",
              "u16": "u16", "unsigned short": "u16", "char": "i8", "s8": "i8", "signed char": "i8",
              "u8": "u8", "unsigned char": "u8", "s64": "i64", "long long": "i64", "u64": "u64",
              "unsigned long long": "u64"}
C_TYPES = {"void": "void", "i32": "int", "u32": "unsigned int", "f32": "float", "i16": "short",
           "u16": "unsigned short", "i8": "signed char", "u8": "unsigned char", "i64": "long long",
           "u64": "unsigned long long"}


@dataclass
class Inputs:
    """The texts the catalogue is made from (load() reads them from the tree; tests write them)."""
    names_tsv: str = ""
    functions_tsv: str = ""
    names_h: dict = field(default_factory=dict)        # readable name -> symbol
    sources: list = field(default_factory=list)        # (path, text): the decompilation's C
    candidates: dict = field(default_factory=dict)     # symbol -> path of its candidate C
    hand: dict = field(default_factory=dict)           # symbol -> path of the port's own C
    extra_sources: list = field(default_factory=list)  # (path, text) of the candidate and hand files
    no_c: dict = field(default_factory=dict)           # symbol -> no_c.tsv's c column
    libraries_pal: str = ""
    libraries_ntsc: str = ""
    split_places: str = ""
    headers: list = field(default_factory=list)        # (path, text) read for struct layouts
    pvars_header: tuple = ("", "")                     # (path, text) of moby_pvars.h
    class_names: dict = field(default_factory=dict)
    api: dict = field(default_factory=dict)            # engine_helpers.tsv: symbol or name -> meaning
    sdk_header: str = ""


def _read(path: Path) -> str:
    return path.read_text(errors="replace") if path.is_file() else ""


def _rel(path: Path) -> str:
    return path.relative_to(REPO).as_posix()


def _table(text: str) -> list[list[str]]:
    return [line.split("\t") for line in text.splitlines() if line.strip() and not line.startswith("#")]


def load() -> Inputs:
    """The inputs from this checkout."""
    inc = DECOMPILATION / "include"
    sources = [(_rel(p), _read(p)) for p in sorted((DECOMPILATION / "src").rglob("*.c"))
               if "libgcc" not in p.relative_to(DECOMPILATION / "src").parts]  # hostgen.json skips src/libgcc
    candidates = {cols[0]: _rel(DECOMPILATION / cols[1].strip())
                  for cols in _table(_read(DECOMPILATION / "nonmatching" / "functional.tsv")) if len(cols) >= 2}
    hand = {cols[0]: _rel(PAL / "hand" / cols[1].strip())
            for cols in _table(_read(PAL / "hand" / "hand.tsv")) if len(cols) >= 2}
    extra = [(p, _read(REPO / p)) for p in sorted(set(candidates.values()) | set(hand.values()))]
    no_c = {cols[0]: cols[3].strip() for cols in _table(_read(PAL / "no_c.tsv")) if len(cols) >= 4 and cols[0] != "name"}
    api = {}
    for cols in _table(_read(HERE / "engine_helpers.tsv")):
        if len(cols) >= 3 and cols[0] == "func":
            api[cols[1]] = cols[2]
    names = {int(cols[0]): cols[1] for cols in _table(_read(CLASS_NAMES)) if len(cols) >= 2}
    return Inputs(
        names_tsv=_read(DECOMPILATION / "config" / "overlays" / "names.tsv"),
        functions_tsv=_read(DECOMPILATION / "config" / "overlays" / "functions.tsv"),
        names_h=names_h_symbols(inc / "names.h"), sources=sources, candidates=candidates, hand=hand,
        extra_sources=extra, no_c=no_c,
        libraries_pal=_read(PAL / "libraries.tsv"), libraries_ntsc=_read(NTSC / "libraries.tsv"),
        split_places=_read(PAL / "split_places.tsv"),
        headers=[(_rel(inc / h), _read(inc / h)) for h in TYPE_HEADERS if (inc / h).is_file()],
        pvars_header=(_rel(inc / PVARS_HEADER), _read(inc / PVARS_HEADER)),
        class_names=names, api=api, sdk_header=_read(SDK_HEADER))


# --- reading the C ------------------------------------------------------------------------------

def file_aliases(text: str) -> dict[str, str]:
    """A file's own names for game symbols: `f_r(...) __asm__("func_...")` and `#define f_r func_...`."""
    out = dict(re.findall(r'\b([A-Za-z_]\w*)\s*\([^;{}()]*\)\s*__asm__\s*\(\s*"(func_\w+)"\s*\)', text))
    out.update(re.findall(r"^#define\s+(\w+)\s+(func_\w+)\s*$", text, re.M))
    return out


_DEFINITION = re.compile(r"^(?!extern\b)([A-Za-z_][\w \t\*]*?)\b([A-Za-z_]\w*)\s*\(((?:[^;{}()]|\([^;{}()]*\))*)\)\s*\{", re.M)


def definitions(text: str) -> dict[str, tuple[str, str, str]]:
    """name -> (return type, parameters, body) of each function TEXT defines (comments and
    preprocessor lines removed first)."""
    clean = strip(text)
    out, pos = {}, 0
    while True:
        m = _DEFINITION.search(clean, pos)
        if m is None:
            return out
        i, depth = m.end(), 1
        while depth and i < len(clean):
            depth += (clean[i] == "{") - (clean[i] == "}")
            i += 1
        if m.group(2) not in KEYWORDS:
            out.setdefault(m.group(2), (" ".join(m.group(1).split()), " ".join(m.group(3).split()), clean[m.end():i - 1]))
        pos = i


def _split_params(params: str) -> list[str]:
    out, depth, cur = [], 0, ""
    for ch in params:
        if ch == "," and depth == 0:
            out.append(cur)
            cur = ""
            continue
        depth += (ch == "(") - (ch == ")")
        cur += ch
    return [p.strip() for p in out + [cur] if p.strip()]


def mods_type(decl: str, aliases: dict, is_return: bool = False) -> str | None:
    """A declaration's type as a hook signature writes it, or None (a record by value, a double...)."""
    if "*" in decl or "[" in decl:
        return "u32"  # a pointer is a game address
    words = [w for w in re.findall(r"[A-Za-z_]\w*", decl)
             if w not in ("const", "volatile", "register", "static", "inline", "__inline__", "struct", "extern")]
    if not is_return and len(words) > 1 and " ".join(words) not in MODS_TYPES:
        words = words[:-1]  # the parameter's name
    text = " ".join(words)
    if text in MODS_TYPES:
        return MODS_TYPES[text]
    alias = aliases.get(text)
    if alias is not None:
        if alias.ptr or alias.dims:
            return "u32"
        return {"s32": "i32", "u32": "u32", "f32": "f32", "s16": "i16", "u16": "u16", "s8": "i8", "u8": "u8",
                "s64": "i64", "u64": "u64", "void": "void"}.get(alias.base)
    return None


def signature(ret: str, params: str, aliases: dict) -> str | None:
    """`void(u32)` for a definition's return type and parameter text, or None."""
    r = mods_type(ret, aliases, True)
    if r is None:
        return None
    parts = [] if params in ("", "void") else _split_params(params)
    if "..." in parts:
        return None
    kinds = [mods_type(p, aliases) for p in parts]
    if any(k is None or k == "void" for k in kinds):
        return None
    return f"{r}({','.join(kinds)})"


def c_signature(sig: str) -> str:
    """libraries.tsv's spelling of a signature: `void(u32)` -> `void (unsigned int)`."""
    ret, params = re.fullmatch(r"(\w+)\((.*)\)", sig).groups()
    return f"{C_TYPES[ret]} ({', '.join(C_TYPES[p] for p in params.split(',')) if params else 'void'})"


def struct_text(text: str, name: str) -> str | None:
    """The body of `struct NAME {...}` or `typedef struct ... {...} NAME;` in TEXT."""
    m = re.search(rf"\bstruct\s+{re.escape(name)}\s*\{{", text)
    if m:
        i, depth = m.end(), 1
        while depth and i < len(text):
            depth += (text[i] == "{") - (text[i] == "}")
            i += 1
        return text[m.end():i - 1]
    m = re.search(rf"\}}\s*{re.escape(name)}\s*;", text)
    if m:
        i, depth = m.start() - 1, 1
        while depth and i > 0:
            depth += (text[i] == "}") - (text[i] == "{")
            i -= 1
        return text[i + 2:m.start()]
    return None


def field_comments(body: str) -> dict[str, tuple[str | None, int | None]]:
    """field name -> (its comment, the offset a leading `/* 0x10 */` gives) in a struct's text."""
    out = {}
    for line in body.splitlines():
        lead = re.match(r"\s*/\*\s*(0x[0-9A-Fa-f]+)\s*\*/", line)
        code = re.sub(r"/\*.*?\*/|//.*", " ", line)
        m = re.search(r"\b([A-Za-z_]\w*)\s*(?:\[[^\]]*\]\s*)*(?::\s*\d+\s*)?;", code)
        if not m:
            continue
        notes = [c.strip() for c in re.findall(r"/\*(.*?)\*/|//(.*)$", line[m.end():]) for c in c if c.strip()]
        out[m.group(1)] = (" ".join(notes) or None, int(lead.group(1), 16) if lead else None)
    return out


def ctype_text(t) -> str:
    return t.base + "*" * t.ptr + "".join(f"[{d}]" for d in t.dims)


def pvar_type(body: str) -> list[str]:
    """Type names the C reads `->pvars` as, in order: a cast of it, or the pointer it is stored in."""
    found = []
    found += re.findall(r"\(\s*(?:struct\s+)?(\w+)\s*\*\s*\)\s*\(?\s*\w+\s*\)?\s*->\s*pvars\b", body)
    found += re.findall(r"(?:struct\s+)?(\w+)\s*\*\s*\w+\s*=\s*(?:\(\s*(?:struct\s+)?\w+\s*\*\s*\)\s*)?\(?\s*\w+\s*\)?\s*->\s*pvars\b", body)
    for var in re.findall(r"\b(\w+)\s*=\s*(?:\([^()]*\)\s*)?\(?\s*\w+\s*\)?\s*->\s*pvars\b", body):
        found += re.findall(rf"(?:struct\s+)?(\w+)\s*\*\s*{re.escape(var)}\s*[;,=)]", body)
    order = []
    for t in found:
        if t not in order:
            order.append(t)
    return order


# --- the catalogue ------------------------------------------------------------------------------

def _levels(text: str) -> list[int]:
    return sorted({int(item.split(":")[0]) for item in text.split(",") if ":" in item})


def _rows(text: str) -> list[list[str]]:
    return [[c.strip() for c in cols] for cols in _table(text)]


def build(inp: Inputs) -> dict:
    """The catalogue as behaviour.json holds it."""
    roles = update_roles(inp.names_tsv)
    where, defined_as = c_definitions(inp.sources, inp.names_h)
    readable = {}
    for name, sym in sorted(inp.names_h.items()):
        readable.setdefault(sym, name)
    places = {cols[0]: _levels(cols[5]) for cols in _rows(inp.functions_tsv) if len(cols) >= 6}
    split: dict[str, dict[str, list[int]]] = {}
    for cols in _rows(inp.split_places):
        if len(cols) >= 3:
            split.setdefault(cols[0], {}).setdefault(cols[1], [])
            split[cols[0]][cols[1]] = sorted(set(split[cols[0]][cols[1]]) | set(_levels(cols[2])))
    pal_rows = {cols[0]: cols for cols in _rows(inp.libraries_pal) if len(cols) >= 4}
    ntsc_hooks = {cols[2]: cols[0] for cols in _rows(inp.libraries_ntsc) if len(cols) >= 4 and cols[3] == "hook"}
    sdk_sigs = dict(re.findall(r'#define\s+OPENRAC_RAC1_SIG_(\w+)\s+"([^"]*)"', inp.sdk_header))

    # Types: the headers, then each file's own structs on top (copies, so files do not mix).
    base = Types()
    for _path, text in inp.headers:
        base.read(text)
    pvars_path, pvars_text = inp.pvars_header
    with_pvars = _copy(base)
    if pvars_text:
        with_pvars.read(pvars_text)

    # Bodies of every function defined in a source the port compiles.
    texts = dict(inp.sources) | dict(inp.extra_sources)
    bodies: dict[str, dict] = {}
    aliases_of: dict[str, dict] = {}

    def file_defs(path: str) -> dict:
        if path not in bodies:
            bodies[path] = definitions(texts.get(path, ""))
            aliases_of[path] = file_aliases(texts.get(path, ""))
        return bodies[path]

    def source_of(sym: str) -> dict:
        """Where the port gets the C of SYM, and the name it is defined under there."""
        path, is_c = where.get(sym, (None, False))
        if is_c:
            return {"kind": "decomp", "file": path, "defined_as": defined_as.get(sym, sym)}
        for kind, table in (("candidate", inp.candidates), ("hand", inp.hand)):
            if sym in table:
                path2 = table[sym]
                defs = file_defs(path2)
                name = next((n for n in [sym, readable.get(sym)] + [a for a, s in aliases_of[path2].items() if s == sym]
                             if n and n in defs), sym)
                return {"kind": kind, "file": path2, "defined_as": name}
        if inp.no_c.get(sym) == "translated":
            return {"kind": "assembly", "file": path, "defined_as": None}
        return {"kind": "none", "file": path, "defined_as": None}

    def definition(src: dict):
        if src["kind"] in ("decomp", "candidate", "hand") and src["file"]:
            return file_defs(src["file"]).get(src["defined_as"])
        return None

    def callees(body: str, path: str, self_sym: str) -> list[str]:
        out = []
        file_defs(path)
        for name in re.findall(r"\b([A-Za-z_]\w*)\s*\(", body):
            if name in KEYWORDS:
                continue
            sym = aliases_of[path].get(name) or inp.names_h.get(name) or (name if SYMBOL.fullmatch(name) else None)
            if sym and sym != self_sym and sym not in out:
                out.append(sym)
        return out

    file_types: dict[str, Types] = {}

    def types_of(path: str) -> Types:
        if path not in file_types:
            t = _copy(with_pvars)
            t.read(texts.get(path, ""))
            file_types[path] = t
        return file_types[path]

    hook_names = {cols[2] for cols in pal_rows.values() if cols[3] == "hook"} | set(ntsc_hooks) | set(sdk_sigs)
    proposed_names: set[str] = set()
    functions, structs, calls_of = {}, {}, {}
    for sym in sorted(roles):
        classes = sorted(roles[sym])
        src = source_of(sym)
        d = definition(src)
        sig = None
        body = ""
        if d is not None:
            ret, params, body = d
            sig = signature(ret, params, types_of(src["file"]).aliases)
        in_port = src["kind"] != "none"
        row = pal_rows.get(sym)
        levels = places.get(sym) or sorted({lv for c in classes for lv in roles[sym][c]})
        hook = hook_status(sym, classes, row, in_port, sig, readable.get(sym), hook_names, proposed_names,
                           levels, split.get(sym, {}), ntsc_hooks, inp.class_names)
        pv = None
        if body:
            t = types_of(src["file"])
            for name in pvar_type(body):
                s = t.structs.get(name) or (t.structs.get(t.aliases[name].base) if name in t.aliases else None)
                if s is not None and s.fields:
                    pv = s.name
                    if pv not in structs:
                        structs[pv] = struct_entry(s, t, src["file"], texts, inp)
                    break
        calls = callees(body, src["file"], sym) if body else []
        calls_of[sym] = calls
        functions[sym] = {
            "name": readable.get(sym),
            "classes": classes, "levels": levels, "split": split.get(sym, {}),
            "source": src, "in_port": in_port, "signature": sig,
            "port_row": row[3] if row and row[3] != "hook" else None,
            "hook": hook, "pvar_struct": pv, "calls": calls}

    # The typed pvars of moby_pvars.h, by class.
    header_pvars: dict[int, list[str]] = {}
    for m in re.finditer(r"typedef\s+struct\s+(\w+)\s+(\w+)\s*;\s*/\*(.*?)\(class (\d+)\)\s*\*/", pvars_text):
        header_pvars.setdefault(int(m.group(4)), []).append(m.group(1))
    for name in with_pvars.structs:
        m = re.fullmatch(r"update(\d+)", name)
        if m and name not in header_pvars.get(int(m.group(1)), []):
            header_pvars.setdefault(int(m.group(1)), []).append(name)
    for cid, names in header_pvars.items():
        for name in names:
            s = with_pvars.structs.get(name)
            if s is not None and name not in structs:
                structs[name] = struct_entry(s, with_pvars, pvars_path, {pvars_path: pvars_text}, inp)

    # Helpers: what the update functions call.
    called_by = Counter(c for calls in calls_of.values() for c in calls)
    api_syms = {inp.names_h.get(k, k) for k in inp.api}
    helpers = {}
    for sym, n in sorted(called_by.items()):
        src = source_of(sym)
        d = definition(src)
        sig = signature(d[0], d[1], types_of(src["file"]).aliases) if d else None
        row = pal_rows.get(sym)
        helpers[sym] = {"name": readable.get(sym), "engine_helper": sym in api_syms, "called_by": n,
                        "in_port": src["kind"] != "none", "source": src["kind"], "signature": sig,
                        "hook_point": row[2] if row and row[3] == "hook" else None,
                        "port_row": row[3] if row and row[3] != "hook" else None}

    # Classes.
    class_ids = sorted({c for classes in roles.values() for c in classes})
    by_class: dict[int, list] = {}
    for sym, classes in roles.items():
        for c, lv in classes.items():
            by_class.setdefault(c, []).append({"function": sym, "levels": sorted(lv)})
    class_list = []
    for c in class_ids:
        updates = sorted(by_class[c], key=lambda u: (u["levels"], u["function"]))
        shared = sorted({o for u in updates for o in functions[u["function"]]["classes"]} - {c})
        pvars = list(header_pvars.get(c, []))
        for u in updates:
            pv = functions[u["function"]]["pvar_struct"]
            if pv and pv not in pvars:
                pvars.append(pv)
        for pv in pvars:
            if pv in structs and c not in structs[pv]["classes"]:
                structs[pv]["classes"].append(c)
        class_list.append({"id": c, "name": inp.class_names.get(c), "levels": sorted({lv for u in updates for lv in u["levels"]}),
                           "updates": updates, "shared_with": shared, "pvars": pvars})
    for s in structs.values():
        s["classes"].sort()

    moby = with_pvars.structs.get("Moby")

    def moby_field(name):
        f = moby.named(name) if moby else None
        return {"name": name, "offset": f.offset, "type": ctype_text(f.ctype)} if f else None

    hook_points = {}
    for cols in pal_rows.values():
        if cols[3] == "hook":
            hook_points[cols[2]] = {"symbol": cols[0], "ntsc_symbol": ntsc_hooks.get(cols[2]),
                                    "signature": sdk_sigs.get(cols[2]), "in_sdk_header": cols[2] in sdk_sigs}
    statuses = Counter(f["hook"]["status"] for f in functions.values())
    counts = {"classes": len(class_list), "class_roles": sum(len(f["classes"]) for f in functions.values()),
              "update_functions": len(functions),
              "shared_update_functions": sum(1 for f in functions.values() if len(f["classes"]) > 1),
              "update_functions_by_source": dict(sorted(Counter(f["source"]["kind"] for f in functions.values()).items())),
              "update_functions_in_port": sum(1 for f in functions.values() if f["in_port"]),
              "update_functions_by_hook_status": dict(sorted(statuses.items())),
              "classes_with_named_class": sum(1 for c in class_list if c["name"]),
              "classes_with_pvar_struct": sum(1 for c in class_list if c["pvars"]),
              "pvar_structs": len(structs), "helpers": len(helpers)}
    return {"format": 1, "game": "rac1", "disc": "pal", "generated_by": "editor/catalogue.py",
            "moby": {"struct": "Moby", "size": moby.size if moby else None, "class_field": moby_field("oclass"),
                     "update_field": moby_field("update"), "pvars_field": moby_field("pvars")},
            "levels": list(LEVEL_NAMES), "counts": counts, "hook_points": dict(sorted(hook_points.items())),
            "functions": functions, "helpers": helpers, "pvar_structs": dict(sorted(structs.items())),
            "classes": class_list}


def _copy(t: Types) -> Types:
    out = Types()
    out.structs, out.aliases, out.symbols = dict(t.structs), dict(t.aliases), dict(t.symbols)
    return out


def struct_entry(s, types: Types, path: str, texts: dict, inp: Inputs) -> dict:
    """A pvar struct: its layout (cparse's, as the C compiles it) and the comments of its text."""
    body, where = None, path
    for p, text in [(path, texts.get(path, ""))] + list(inp.headers) + [inp.pvars_header]:
        body = struct_text(text, s.name)
        if body is not None:
            where = p
            break
    notes = field_comments(body or "")
    declared = None
    if where == inp.pvars_header[0]:
        m = re.search(rf"\bstruct\s+{re.escape(s.name)}\s*\{{\s*//\s*(0x[0-9A-Fa-f]+)", inp.pvars_header[1])
        declared = int(m.group(1), 16) if m else None
    fields = []
    for f in s.fields:
        if f.name.startswith("pad"):
            continue
        comment, lead = notes.get(f.name, (None, None))
        fields.append({"offset": f.offset, "declared_offset": lead if lead is not None and lead != f.offset else None,
                       "type": ctype_text(f.ctype), "name": f.name, "comment": comment})
    return {"size": s.size, "declared_size": declared, "exact": s.exact, "file": where, "classes": [], "fields": fields}


def hook_status(sym, classes, row, in_port, sig, name, taken, proposed_names, levels, split, ntsc_hooks, class_names):
    """How a mod can take an update function: the hook point it has, or what one needs."""
    cover = f"levels {_ranges(levels)}" + (
        "; not the places split_places.tsv gives to " + ", ".join(f"{v} (levels {_ranges(lv)})" for v, lv in sorted(split.items()))
        if split else "")
    if row is not None and row[3] == "hook":
        return {"status": "hook_point", "point": row[2], "proposed": None, "row": None, "covers": cover,
                "missing": None if row[2] in ntsc_hooks else "no hook row for it in port/game/rac1-ntsc/libraries.tsv"}
    proposed = hook_name(classes, class_names)
    if proposed in taken or proposed in proposed_names:
        # Two functions update classes of one name (a class per level): the class number tells them apart.
        proposed = hook_name(classes, class_names, numbered=True)
    proposed_names.add(proposed)
    if row is not None:
        return {"status": "wrapped", "point": None, "proposed": proposed, "row": None, "covers": cover,
                "missing": f"the port has a `{row[3]}` row for it (libraries.tsv: {row[2]}); a hook point on it "
                           "needs hostgen to put the hook around the port's host function, or the port's change moved"}
    if not in_port:
        return {"status": "needs_c", "point": None, "proposed": proposed, "row": None, "covers": cover,
                "missing": "no C in the port's build (a stub): C for it first, then a hook row"}
    if sig is None:
        return {"status": "no_signature", "point": None, "proposed": proposed, "row": None, "covers": cover,
                "missing": "its C has a parameter or result a hook cannot pass (MODS.md section 4)"}
    who = ", ".join(f"{c}" + (f" {class_names[c]}" if class_names.get(c) else "") for c in classes[:6]) + \
        (f" and {len(classes) - 6} more" if len(classes) > 6 else "")
    note = f"the update of class{'es' if len(classes) > 1 else ''} {who}: a hook point for mods (docs/port/MODS.md)"
    missing = "a `hook` row in port/game/rac1-pal/libraries.tsv (and rac1-ntsc's, with the NTSC symbol)"
    if sig == "void()":
        missing += "; its C takes no parameter, so a replacement does not get the moby"
    return {"status": "needs_hook_row", "point": None, "proposed": proposed,
            "row": f"{sym}\tmoby\t{proposed}\thook\t{note}\t{c_signature(sig)}", "covers": cover, "missing": missing}


def pascal(name: str) -> str:
    """bolt_crate -> BoltCrate; a name that starts with a digit keeps it after an underscore-free join."""
    return "".join(part[:1].upper() + part[1:] for part in re.split(r"[^0-9A-Za-z]+", name) if part)


def hook_name(classes: list[int], class_names: dict, numbered: bool = False) -> str:
    """The hook point's name for mods, docs/port/MODS.md section 4: Moby_<Class>_Update, after the
    first named class the function updates (by number), else Class<N> after its first class. A function
    shared by several classes is one hook point, named after that class; the catalogue's classes list
    which classes reach it."""
    named = [c for c in classes if class_names.get(c)]
    c = named[0] if named else classes[0]
    base = pascal(class_names[c]) if named else f"Class{c}"
    if numbered and named:
        base += str(c)
    return f"Moby_{base}_Update"


def _ranges(levels: list[int]) -> str:
    """0-3, 5, 7-18."""
    out, run = [], []
    for lv in sorted(levels):
        if run and lv == run[-1] + 1:
            run.append(lv)
            continue
        if run:
            out.append(f"{run[0]}-{run[-1]}" if len(run) > 1 else f"{run[0]}")
        run = [lv]
    if run:
        out.append(f"{run[0]}-{run[-1]}" if len(run) > 1 else f"{run[0]}")
    return ", ".join(out) or "none"


# --- writing ------------------------------------------------------------------------------------

TSV_COLUMNS = ("class", "class_name", "levels", "update_function", "update_name", "source_kind", "source_file",
               "in_port", "classes_sharing", "signature", "hook_status", "hook_point", "proposed_hook_point",
               "pvar_structs", "pvar_sizes")


def render_json(cat: dict) -> str:
    """The catalogue with one entry a line (a function, a helper, a struct, a class), so that a
    change to one shows as one line in a diff without the file growing by its indentation."""
    def one(v):
        return json.dumps(v, ensure_ascii=False, separators=(",", ":"))
    parts = []
    for key, value in cat.items():
        if isinstance(value, dict) and value and all(isinstance(v, dict) for v in value.values()):
            inner = ",\n".join(f"  {one(k)}: {one(v)}" for k, v in value.items())
            parts.append(f" {one(key)}: {{\n{inner}\n }}")
        elif isinstance(value, list) and value and all(isinstance(v, dict) for v in value):
            inner = ",\n".join(f"  {one(v)}" for v in value)
            parts.append(f" {one(key)}: [\n{inner}\n ]")
        else:
            parts.append(f" {one(key)}: {one(value)}")
    return "{\n" + ",\n".join(parts) + "\n}\n"


def render_tsv(cat: dict) -> str:
    lines = ["# The moby behaviour catalogue of Ratchet & Clank (PAL), one row per class and update function.",
             "# Generated by editor/catalogue.py from the source tree; do not edit. behaviour.json has the rest.",
             "# levels: where the class is updated by that function; classes_sharing: how many classes that function serves.",
             "\t".join(TSV_COLUMNS)]
    for c in cat["classes"]:
        for u in c["updates"]:
            f = cat["functions"][u["function"]]
            sizes = [f"{cat['pvar_structs'][p]['size']:#x}" for p in c["pvars"] if p in cat["pvar_structs"]]
            lines.append("\t".join(str(v) for v in (
                c["id"], c["name"] or "", _ranges(u["levels"]), u["function"], f["name"] or "", f["source"]["kind"],
                f["source"]["file"] or "", "yes" if f["in_port"] else "no", len(f["classes"]), f["signature"] or "",
                f["hook"]["status"], f["hook"]["point"] or "", f["hook"]["proposed"] or "", ",".join(c["pvars"]),
                ",".join(sizes))))
    return "\n".join(lines) + "\n"


def _cell(text) -> str:
    return str(text).replace("|", "\\|") if text not in (None, "") else ""


def render_md(cat: dict) -> str:
    n, moby = cat["counts"], cat["moby"]
    cls, upd, pv = moby["class_field"], moby["update_field"], moby["pvars_field"]
    hs = n["update_functions_by_hook_status"]
    src = n["update_functions_by_source"]
    names = {c["id"]: c["name"] for c in cat["classes"]}
    out = [
        "# Ratchet & Clank (PAL): the moby behaviour catalogue",
        "",
        "Generated by [editor/catalogue.py](../../../editor/catalogue.py) from the source tree (the",
        "decompilation and the port's tables, never the disc); do not edit. `python3 editor/catalogue.py write`",
        "regenerates it, `check` says whether it is stale. The same facts for tools are",
        "[behaviour.json](behaviour.json) (its format is in the generator's docstring) and, one row per class,",
        "[behaviour.tsv](behaviour.tsv).",
        "",
        "**Experimental.** The function and struct names come from the decompilation as it is today and",
        "will change in the C++ refactor; hook names (`Moby_<Class>_Update`) are the stable handle. This",
        "catalogue is for the mod SDK (native mods and the Luau binding), not for the level editor:",
        "behaviour is modded through the SDK. [classes.json](classes.json) is the stable table for",
        "bindings (its format is in the generator's docstring).",
        "",
        "## What it says",
        "",
        "Each frame the game calls every active moby's update function with the moby as its only argument",
        f"(`moby->update`, +{upd['offset']:#x}), and a class's state beyond the moby record lives in its pvars",
        f"(`moby->pvars`, +{pv['offset']:#x}). This catalogue gives, for each of the **{n['classes']} classes**",
        f"the levels' class tables name, the update function the decompilation knows for it: **{n['update_functions']}",
        f"functions serve {n['class_roles']} class roles**, and {n['shared_update_functions']} of them serve more than",
        "one class. A mod that takes a shared function sees every class it serves, so it tells them apart by",
        f"the moby's class number: **`{cls['type']} {cls['name']}` at +{cls['offset']:#x}** of the moby",
        f"(`*(int16_t*)(api->memory + moby + {cls['offset']:#x})`, struct Moby in",
        "[games/rac1/include/moby.h](../../../games/rac1/include/moby.h)).",
        "",
        "| The update functions | |", "|---|---:|",
        *[f"| {_SRC[k]} | {v} |" for k, v in src.items()],
        f"| with C in the port's build | {n['update_functions_in_port']} |",
        f"| hook points today | {hs.get('hook_point', 0)} |",
        f"| needing only a `hook` row in [libraries.tsv](../../../port/game/rac1-pal/libraries.tsv) (behaviour.json's `hook.row` is the row) | {hs.get('needs_hook_row', 0)} |",
        f"| with a row of the port's own that a hook would have to share (`wrap`) | {hs.get('wrapped', 0)} |",
        f"| with a signature a hook cannot take | {hs.get('no_signature', 0)} |",
        f"| without C in the port (stubs) | {hs.get('needs_c', 0)} |",
        "",
        f"{n['classes_with_pvar_struct']} classes have a typed pvar struct ({n['pvar_structs']} structs): the",
        "Wrench-derived ones of [moby_pvars.h](../../../games/rac1/include/moby_pvars.h), and those the update",
        "function's own C reads its pvars as (a struct of the C gives the part of the pvars that C names,",
        "so its size is a lower bound).",
        "",
        "## How a mod takes an update function",
        "",
        "A hook point is a `hook` row of a disc's `libraries.tsv` ([docs/port/MODS.md](../../../docs/port/MODS.md),",
        "section 4); hostgen keeps the game's C as `<symbol>__game` and writes `<symbol>` to call the mod's",
        "replacement when one is hooked. hostgen's `functions.c` registers that same host function at",
        "every place the function has, in every level whose program carries its code",
        "(config/overlays/functions.tsv), so one row makes one hook point for all those levels, and the",
        "moby loop's calls through `moby->update` reach it as direct calls do. Places that",
        "port/game/rac1-pal/split_places.tsv gives to a variant are another function and are not covered",
        "(the `covers` text says which). A hook point is per disc: rac1-ntsc needs its own row, with the",
        "NTSC decomp's symbol for the function, under the same name.",
        "",
        "- **Replace class X's update**: take its update function's hook point and, in the replacement, run",
        "  your code when the moby's class is X and call the original otherwise. That needs the hook point",
        "  (most need a row: `hook.status` is `needs_hook_row`). The mod interface has no way to point one",
        "  class at a function of its own instead (writing `moby->update` needs a game code address that",
        "  runs a host function).",
        *_no_moby(cat),
        "- **Hook after function Y**: take Y's hook point, call the original, then do your part. The",
        "  helper calls listed per function are the candidates; a helper that is not a hook point needs a row",
        "  like an update function, and one called by many update functions runs for all of them.",
        "",
        "## Hook points today",
        "",
        "| Hook point | PAL symbol | NTSC symbol | Signature | In the SDK header |",
        "|---|---|---|---|---|",
    ]
    for name, h in cat["hook_points"].items():
        out.append(f"| `{name}` | `{h['symbol']}` | {('`' + h['ntsc_symbol'] + '`') if h['ntsc_symbol'] else ''} | "
                   f"`{h['signature'] or ''}` | {'yes' if h['in_sdk_header'] else 'no'} |")
    hooked = [(s, f) for s, f in cat["functions"].items() if f["hook"]["status"] == "hook_point"]
    out += ["", f"{len(hooked)} of them are moby update functions" + (": " + ", ".join(f"`{f['hook']['point']}`" for _s, f in hooked) if hooked else "") + ".", ""]

    out += ["## Update functions", "",
            "One row per function. **Classes**: the classes it updates (more than one: filter on the class",
            f"field, +{cls['offset']:#x}). **Levels**: where its code is placed. **C**: where the port gets its C",
            "(decomp: games/rac1/src; candidate: nonmatching/; hand: the port's own; assembly: translated from",
            "its instructions). **Hook**: the hook point, or the name proposed for one and what it needs.",
            "**Calls**: the game functions its C calls that are not engine maths helpers, first ten.", "",
            "| Function | Name | Classes | Levels | C | Signature | Hook | Pvars | Calls |",
            "|---|---|---|---|---|---|---|---|---|"]
    for sym, f in cat["functions"].items():
        classes = ", ".join(f"{c}" + (f" {names[c]}" if names.get(c) else "") for c in f["classes"][:8])
        if len(f["classes"]) > 8:
            classes += f" and {len(f['classes']) - 8} more"
        h = f["hook"]
        hook = {"hook_point": f"**`{h['point']}`**", "needs_hook_row": f"row: `{h['proposed']}`",
                "wrapped": f"wrapped ({f['port_row']})", "no_signature": "no signature", "needs_c": "needs C"}[h["status"]]
        if f["hook"]["status"] == "needs_hook_row" and f["signature"] == "void()":
            hook += " (no moby)"
        calls = [c for c in f["calls"] if not cat["helpers"][c]["engine_helper"]][:10]
        calls_text = ", ".join(f"`{cat['helpers'][c]['name'] or c}`" for c in calls)
        out.append(f"| `{sym}` | {_cell(f['name'])} | {classes} | {_ranges(f['levels'])} | {f['source']['kind']} | "
                   f"{('`' + f['signature'] + '`') if f['signature'] else ''} | {hook} | {_cell(f['pvar_struct'])} | {calls_text} |")

    helpers = sorted(((s, h) for s, h in cat["helpers"].items() if not h["engine_helper"] and h["called_by"] > 1),
                     key=lambda x: (-x[1]["called_by"], x[0]))[:60]
    out += ["", "## Hook-after candidates", "",
            "The game functions update functions call most, engine maths helpers left out",
            "([editor/engine_helpers.tsv](../../../editor/engine_helpers.tsv) lists those): a hook on one runs",
            "for every update function that calls it.", "",
            "| Function | Name | Called by | C | Signature | Hook point |", "|---|---|---|---|---|---|"]
    for s, h in helpers:
        out.append(f"| `{s}` | {_cell(h['name'])} | {h['called_by']} | {h['source']} | "
                   f"{('`' + h['signature'] + '`') if h['signature'] else ''} | {_cell(h['hook_point'] or h['port_row'])} |")

    out += ["", "## Pvar structs", "",
            "Layouts as the C compiles them (cparse's offsets); where the header's own offset comment differs",
            "it is given too, and `declared` is the size its comment states. moby_pvars.h's layouts come from",
            "Wrench and are not all checked against the game: a struct whose sizes disagree is marked.", ""]
    for name, s in cat["pvar_structs"].items():
        flag = " (sizes disagree)" if s["declared_size"] not in (None, s["size"]) else ""
        cl = ", ".join(f"{c}" + (f" {names[c]}" if names.get(c) else "") for c in s["classes"]) or "none known"
        out += [f"### {name}", "",
                f"{s['size']:#x} bytes" + (f", declared {s['declared_size']:#x}{flag}" if s["declared_size"] is not None else "")
                + f"; in [{s['file']}](../../../{s['file']}); classes: {cl}.", "",
                "| Offset | Type | Name | Comment |", "|---|---|---|---|"]
        for fl in s["fields"]:
            off = f"{fl['offset']:#x}" + (f" (header: {fl['declared_offset']:#x})" if fl["declared_offset"] is not None else "")
            out.append(f"| {off} | `{fl['type']}` | {fl['name']} | {_cell(fl['comment'])} |")
        out.append("")

    out += ["## Classes", "",
            "Every class with an update function, by number: its name (editor/moby_classes.tsv), the levels it is",
            "updated in and the function.", "", "| Class | Name | Levels | Update function | Shares it with |",
            "|---|---|---|---|---|"]
    for c in cat["classes"]:
        fn = "; ".join(f"`{u['function']}`" + (f" ({_ranges(u['levels'])})" if len(c["updates"]) > 1 else "")
                       for u in c["updates"])
        shared = ", ".join(str(o) for o in c["shared_with"][:10]) + (f" and {len(c['shared_with']) - 10} more"
                                                                    if len(c["shared_with"]) > 10 else "")
        out.append(f"| {c['id']} | {_cell(c['name'])} | {_ranges(c['levels'])} | {fn} | {shared} |")
    return "\n".join(out).rstrip() + "\n"


def _no_moby(cat: dict) -> list[str]:
    """The note on update functions whose C takes no parameter."""
    bare = [(s, f) for s, f in cat["functions"].items() if f["signature"] == "void()"]
    if not bare:
        return []
    n = sum(len(f["classes"]) for _s, f in bare)
    which = ", ".join(f"`{f['name'] or s}` ({len(f['classes'])} class{'es' if len(f['classes']) > 1 else ''})" for s, f in bare)
    return [f"  {len(bare)} update function{'s' if len(bare) > 1 else ''} take no parameter in C ({which}, {n} classes",
            "  in all): a hook on one does not get the moby, so for those classes only that last way would do."]


_SRC = {"decomp": "C in the decompilation (src/)", "candidate": "candidate C (nonmatching/)",
        "hand": "the port's own C", "assembly": "C hostgen translates from the assembly", "none": "no C"}


def classes_table(cat: dict) -> dict:
    """classes.json: the stable part of the catalogue, per class, for bindings made from it (the
    format is in the module docstring; it changes only with its format number)."""
    out = []
    for c in cat["classes"]:
        updates = []
        for u in c["updates"]:
            f = cat["functions"][u["function"]]
            h = f["hook"]
            updates.append({"function": u["function"], "name": f["name"], "levels": u["levels"],
                            "hook": h["point"] or h["proposed"],
                            "hook_status": "available" if h["status"] == "hook_point" else h["status"],
                            "signature": f["signature"], "gets_moby": f["signature"] not in (None, "void()"),
                            "shared_with": [o for o in f["classes"] if o != c["id"]]})
        pvars = []
        for name in c["pvars"]:
            st = cat["pvar_structs"].get(name)
            if st is None:
                continue
            pvars.append({"struct": name, "size": st["size"], "checked": st["declared_size"] in (None, st["size"]),
                          "fields": [{"name": fl["name"], "offset": fl["offset"], "type": fl["type"],
                                      "meaning": fl["comment"]} for fl in st["fields"]]})
        out.append({"id": c["id"], "name": c["name"], "levels": c["levels"], "updates": updates, "pvars": pvars})
    return {"format": CLASSES_FORMAT, "game": cat["game"], "disc": cat["disc"], "generated_by": cat["generated_by"],
            "moby": cat["moby"], "classes": out}


def outputs(cat: dict) -> dict[Path, str]:
    return {OUT / "classes.json": render_json(classes_table(cat)), OUT / "behaviour.json": render_json(cat),
            OUT / "behaviour.tsv": render_tsv(cat), OUT / "BEHAVIOUR.md": render_md(cat)}


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    sub = ap.add_subparsers(dest="command", required=True)
    sub.add_parser("write", help="regenerate sdk/catalogue/rac1/")
    sub.add_parser("check", help="exit 1 when sdk/catalogue/rac1/ is stale")
    s = sub.add_parser("show", help="one class's entry, with its update functions")
    s.add_argument("class_id", type=int)
    args = ap.parse_args(argv)
    cat = build(load())
    if args.command == "show":
        c = next((c for c in cat["classes"] if c["id"] == args.class_id), None)
        if c is None:
            print(f"class {args.class_id}: no update function in the catalogue", file=sys.stderr)
            return 1
        entry = dict(c, functions={u["function"]: cat["functions"][u["function"]] for u in c["updates"]},
                     pvar_structs={p: cat["pvar_structs"][p] for p in c["pvars"] if p in cat["pvar_structs"]})
        print(json.dumps(entry, indent=1))
        return 0
    stale = []
    for path, text in outputs(cat).items():
        if args.command == "write":
            path.parent.mkdir(parents=True, exist_ok=True)
            if not path.is_file() or path.read_text() != text:
                path.write_text(text)
                print(f"wrote {_rel(path)}")
        elif not path.is_file() or path.read_text() != text:
            stale.append(_rel(path))
    if stale:
        print("stale: " + ", ".join(stale) + " (python3 editor/catalogue.py write)", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
