# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""What the whole program defines: every function, its signature, its address.

A call is translated against the callee's definition, not against what the
calling file declared (the decompilation's files often declare a callee
differently from its definition: other argument types, another order of
float and integer arguments, fewer arguments). So every unit is read once
first, to know each definition (index_unit), before any is translated.
"""

from __future__ import annotations

import json
import re
from dataclasses import dataclass, field
from pathlib import Path

import ctype

# How a game's decompilation names code and data by address, as regular
# expressions with an `addr` group (hexadecimal) and, for code in a level's
# program, an `overlay` group (decimal). These are rac1/pal's (func_00123456,
# func_L05_00123456, D_00123456, with an optional _suffix for a variant); a
# game's hostgen.json gives its own under "names".
DEFAULT_CODE_NAMES = [r"^func_(?:L(?P<overlay>\d\d)_)?(?P<addr>[0-9A-Fa-f]{6,8})(?:_\w*)?$"]
DEFAULT_DATA_NAMES = [r"^(?:D|func)_(?:L(?P<overlay>\d\d)_)?(?P<addr>[0-9A-Fa-f]{6,8})(?:_\w*)?$"]

ASM_MARK = "openrac_asm_"

# A data symbol given as an asm label with a constant offset (`D_L16_0017C218+12`).
_SYMBOL_OFFSET = re.compile(r"^\s*([A-Za-z_]\w*)\s*([+-])\s*(0[xX][0-9A-Fa-f]+|\d+)\s*$")

# Helpers whose bodies are the decompilation's inline assembly
# (include/common.h): the runtime has host versions (guest.h).
BUILTINS = {
    "qcopy": "openrac_qcopy",
    "qcopy_nc": "openrac_qcopy",
    "qzero": "openrac_qzero",
}


def cname(symbol: str) -> str:
    """The host C name of a game function. Names the game shares with the C
    library or the host's own program (memcpy, sprintf, main, _start) get a
    prefix; func_ names stay as they are."""
    if symbol.startswith("func_"):
        return symbol
    return f"game_{symbol}"


@dataclass
class Signature:
    ret: ctype.Type
    params: tuple
    variadic: bool = False
    prototyped: bool = True

    @staticmethod
    def of(t: ctype.Type) -> "Signature":
        if not isinstance(t, ctype.Func):
            raise ctype.TypeError_(f"not a function type: {t}")
        return Signature(t.ret, tuple(t.params), t.variadic, t.prototyped)


@dataclass
class Function:
    symbol: str                 # the linker name (asm label if any)
    sig: Signature
    unit: str | None = None     # where it is defined; None: not defined in C
    static: bool = False
    asm: bool = False           # still assembly in the decompilation
    declared: list = field(default_factory=list)  # signatures seen in declarations
    tail: str | None = None     # a definition returning nothing that ends in a call: the function called
    inherits: bool = False      # its result is that call's (inherit_results): sig.ret is the callee's


@dataclass
class Program:
    functions: dict[str, Function] = field(default_factory=dict)
    statics: dict[tuple[str, str], Function] = field(default_factory=dict)
    places: dict[str, list[tuple[int, int]]] = field(default_factory=dict)  # symbol -> (overlay, addr)
    sizes: dict[str, int] = field(default_factory=dict)  # catalogued symbol -> bytes
    splits: dict[str, list] = field(default_factory=dict)  # symbol -> places split off to variants
    split_parent: dict[str, str] = field(default_factory=dict)  # a variant -> the symbol it was split from
    split_twin: dict[str, str] = field(default_factory=dict)  # an executable function -> the variant at its level copies
    symbols: dict[str, int] = field(default_factory=dict)  # other named globals -> address
    typedefs: dict[str, set] = field(default_factory=dict)  # name -> desugared type strings
    host: set = field(default_factory=set)  # functions the port writes by hand (port/game/<game>/)
    from_assembly: set = field(default_factory=set)  # C left out for C from the instructions (hostgen.json "from_assembly")
    wrap: set = field(default_factory=set)  # translated as <name>__game, called by a host <name>
    aliases: dict = field(default_factory=dict)  # (unit, name) -> symbol, from labels Clang refused
    by_place: dict = field(default_factory=dict)  # (overlay, address) -> the symbol defined there
    groups: list = field(default_factory=list)  # [{"path", "suffix", "overlay"}]: separate programs
    code_names: list = field(default_factory=lambda: [re.compile(p) for p in DEFAULT_CODE_NAMES])
    data_names: list = field(default_factory=lambda: [re.compile(p) for p in DEFAULT_DATA_NAMES])

    def set_names(self, code: list[str] | None, data: list[str] | None) -> None:
        if code:
            self.code_names = [re.compile(p) for p in code]
        if data:
            self.data_names = [re.compile(p) for p in data]

    def lookup(self, unit: str, symbol: str) -> Function | None:
        return self.statics.get((unit, symbol)) or self.functions.get(symbol)

    def group_of(self, unit: str) -> dict | None:
        """The group (a separate program, such as a menu loaded over part of
        the executable) that a unit belongs to, if any."""
        for g in self.groups:
            if unit == g["path"] or unit.startswith(g["path"].rstrip("/") + "/"):
                return g
        return None

    def grouped(self, unit: str, symbol: str) -> str:
        """The symbol a definition in unit has: its group's own, if in one."""
        g = self.group_of(unit)
        return f"{symbol}__{g['suffix']}" if g else symbol

    def code_places(self, symbol: str) -> list[tuple[int, int]]:
        """Where a function sits: (overlay, address) pairs; overlay -1 is the executable."""
        for g in self.groups:
            tail = "__" + g["suffix"]
            if symbol.endswith(tail):
                return [(g["overlay"], addr) for _, addr in self.code_places(symbol[:-len(tail)])]
        named: list[tuple[int, int]] = []
        for pattern in self.code_names:
            m = pattern.match(symbol)
            if m:
                overlay = m.groupdict().get("overlay")
                named = [(int(overlay) if overlay is not None else -1, int(m.group("addr"), 16))]
                break
        if not named and symbol in self.symbols:
            named = [(-1, self.symbols[symbol])]
        if symbol not in self.places and named:
            # Another spelling of the same function (func_L00_00256F14 for the catalogue's
            # FUN_L00_00256f14) has the catalogue's places in the other levels.
            other = self._place_owner().get(named[0])
            if other is not None and other != symbol:
                exe = [p for p in named if p[0] < 0 and p not in self.places[other]]
                return exe + self.places[other]
        if symbol in self.places:
            # A function of the executable that the levels' programs also
            # carry is in the catalogue with its level places only: it keeps
            # its place in the executable too, first, as its own address.
            exe = [p for p in named if p[0] < 0 and p not in self.places[symbol]]
            return exe + self.places[symbol]
        return named

    def _place_owner(self) -> dict:
        """The catalogue's functions by their own address: (overlay, address) of the name."""
        owner = getattr(self, "_owner_cache", None)
        if owner is None or len(owner) == 0 and self.places:
            owner = {}
            for sym in self.places:
                for pattern in self.code_names:
                    m = pattern.match(sym)
                    if m:
                        overlay = m.groupdict().get("overlay")
                        owner.setdefault((int(overlay) if overlay is not None else -1, int(m.group("addr"), 16)), sym)
                        break
            self._owner_cache = owner
        return owner

    def data_address(self, symbol: str) -> int | None:
        if symbol in self.symbols:
            return self.symbols[symbol]
        for pattern in self.data_names + self.code_names:
            m = pattern.match(symbol)
            if m:
                return int(m.group("addr"), 16)
        offset = _SYMBOL_OFFSET.match(symbol)
        if offset:
            # An asm label with an offset (`__asm__("D_L16_0017C218+12")`): the assembler's
            # symbol plus the constant, as the console's linker resolves it.
            base = self.data_address(offset.group(1))
            if base is not None:
                text = offset.group(3)
                n = int(text, 16) if text[:2] in ("0x", "0X") else int(text, 10)
                return (base + n if offset.group(2) == "+" else base - n) & 0xFFFFFFFF
        return None

    def data_level(self, symbol: str) -> int | None:
        """The level a data name says it is of (D_L05_..., the names' `overlay` group), or None."""
        offset = _SYMBOL_OFFSET.match(symbol)
        if offset:
            symbol = offset.group(1)
        for pattern in self.data_names:
            m = pattern.match(symbol)
            if m and m.groupdict().get("overlay") is not None:
                return int(m.group("overlay"))
        return None

    def index_places(self) -> None:
        """Every function with C (or the port's own) by where it sits, so that
        two spellings of one address (FUN_0020c828, func_0020C828) meet."""
        self.by_place = {}
        self.host_place = {}
        for sym in sorted(self.host):
            if sym:
                # The port's own function replaces whatever the decompilation has at its place,
                # under any spelling (a table carried from another version names it func_<addr>
                # where this decompilation has FUN_<addr> with C of its own).
                for place in self.code_places(sym):
                    self.host_place.setdefault(place, sym)
                    self.by_place[place] = self.host_place[place]
        for sym, f in sorted(self.functions.items()):
            if f.unit is not None:
                for place in self.code_places(sym):
                    self.by_place.setdefault(place, sym)

    def code_tables(self) -> dict[int, list[tuple[int, str]]]:
        """Every function with a code address, per overlay, as functions.c registers them for GFN:
        (address, symbol), sorted. Worked out once (after the program is indexed)."""
        tables = getattr(self, "_code_tables", None)
        if tables is not None:
            return tables
        tables = {}
        for sym, f in sorted(self.functions.items()):
            if prototype(f, self.typedefs) is None:
                continue
            # Two spellings of one address (FUN_0020c828, func_0020C828): the one with C answers a
            # call through the address too, not whichever sorts first.
            own = self.canonical(sym)
            if own != sym and own in self.functions and prototype(self.functions[own], self.typedefs) is not None:
                continue
            for overlay, addr in self.code_places(sym):
                tables.setdefault(overlay, []).append((addr, sym))
                # hostgen.json "entries": a function whose C starts this many bytes into its
                # catalogue place answers calls through that address too.
                offset = getattr(self, "entries", {}).get(sym)
                if offset:
                    tables[overlay].append((addr + offset, sym))
        for entries in tables.values():
            entries.sort()
        self._code_tables = tables
        return tables

    def function_at(self, overlay: int, address: int) -> str | None:
        """The function a call through address finds with overlay loaded: the first functions.c
        lists there, which the runtime keeps (guest.cpp); None when there is none."""
        index = getattr(self, "_function_at", None)
        if index is None:
            index = {}
            for o, entries in self.code_tables().items():
                for addr, sym in entries:
                    index.setdefault((o, addr), sym)
            self._function_at = index
        return index.get((overlay, address))

    def canonical(self, symbol: str) -> str:
        """The symbol with C at the same place as symbol, or symbol: the port's own function
        there before the decompilation's C."""
        if symbol not in self.host:
            for place in self.code_places(symbol):
                other = getattr(self, "host_place", {}).get(place)
                if other is not None and other != symbol:
                    return other
        f = self.functions.get(symbol)
        if f is not None and (f.unit is not None or symbol in self.host):
            return symbol
        for place in self.code_places(symbol):
            other = self.by_place.get(place)
            if other is not None:
                return other
        return symbol

    def stubs(self) -> list[Function]:
        """Functions called or declared but defined nowhere in C, nor by the host."""
        return sorted((f for f in self.functions.values() if f.unit is None and f.symbol not in self.host),
                      key=lambda f: f.symbol)


def parse_type(node: dict, desugared: bool = True) -> ctype.Type:
    t = node["type"]
    text = t.get("desugaredQualType", t["qualType"]) if desugared else t["qualType"]
    return ctype.parse(text)


def symbol_of(decl: dict) -> str:
    return decl.get("mangledName") or decl.get("name", "")


def has_body(decl: dict) -> bool:
    return any(c.get("kind") == "CompoundStmt" for c in decl.get("inner", ()))


def tail_call(decl: dict, by_id: dict) -> str | None:
    """The function a definition's last statement calls, when that statement is a direct call:
    on the console its result is still in the result register when the definition returns."""
    body = next((c for c in decl.get("inner", ()) if c.get("kind") == "CompoundStmt"), None)
    stmts = [s for s in (body or {}).get("inner", ()) if s.get("kind")]
    if not stmts or stmts[-1].get("kind") != "CallExpr" or not stmts[-1].get("inner"):
        return None
    callee = stmts[-1]["inner"][0]
    while callee.get("kind") in ("ImplicitCastExpr", "ParenExpr") and callee.get("inner"):
        callee = callee["inner"][0]
    ref = callee.get("referencedDecl") or {}
    if callee.get("kind") != "DeclRefExpr" or ref.get("kind") != "FunctionDecl":
        return None
    d = by_id.get(ref.get("id"))
    return symbol_of(d) if d is not None else ref.get("name")


def inherit_results(program: Program) -> list[str]:
    """A definition that returns nothing and ends in a call leaves that call's result in the
    result register, and the decompilation's callers read it there (a wrapper matched as `void
    f(p) { g(p, 0x20); }` whose callers declare it `float f(void *)`). Such a definition is given
    its callee's result type, so the translation returns the value (lower.py); through a chain of
    them too. Only scalar results, and only from a callee that has C. Returns the names changed."""
    changed = []
    again = True
    while again:
        again = False
        for f in list(program.functions.values()) + list(program.statics.values()):
            if f.unit is None or f.tail is None or f.inherits or canon(f.sig.ret, program.typedefs) != "void":
                continue
            if f.symbol in program.wrap or f.symbol in program.host:
                continue  # the port's own C around it or in its place says what it returns
            sym = program.aliases.get((f.unit, f.tail), f.tail)
            g = program.statics.get((f.unit, sym))
            if g is None:
                own = program.grouped(f.unit, sym)
                g = program.functions.get(own) if own != sym else None
            if g is None:
                g = program.functions.get(program.canonical(sym))
            if g is None or g is f or (g.unit is None and g.symbol not in program.host):
                continue
            ret = canon(g.sig.ret, program.typedefs)
            if ret in (None, "void", "va_list"):
                continue
            result = ctype.Ptr(ctype.Base("void")) if ret == "gaddr" else ctype.Base(ret)
            f.sig = Signature(result, f.sig.params, f.sig.variadic, f.sig.prototyped)
            f.inherits = True
            again = True
            changed.append(f.symbol)
    return changed


def index_unit(program: Program, unit: str, ast: dict) -> list[str]:
    """Adds one unit's definitions and declarations. Returns problems found."""
    problems = []
    by_id = {d.get("id"): d for d in ast.get("inner", ()) if d.get("kind") == "FunctionDecl"}
    for decl in ast.get("inner", ()):
        kind = decl.get("kind")
        if kind == "TypedefDecl":
            t = decl["type"]
            text = t.get("desugaredQualType", t["qualType"])
            for c in decl.get("inner", ()):
                owned = c.get("ownedTagDecl")
                if owned and not owned.get("name"):
                    # A typedef of an unnamed enum or record is named after
                    # itself; what matters for a signature is which it is.
                    text = ("enum " if owned.get("kind") == "EnumDecl" else "struct ") + decl["name"]
            program.typedefs.setdefault(decl["name"], set()).add(text)
            continue
        if kind == "VarDecl" and decl.get("name", "").startswith(ASM_MARK):
            name = program.grouped(unit, decl["name"][len(ASM_MARK):])
            f = program.functions.get(name)
            if f is None:
                program.functions[name] = Function(name, Signature(ctype.Base("int"), (), False, False), asm=True)
            else:
                f.asm = True
            continue
        if kind != "FunctionDecl":
            continue
        sym = symbol_of(decl)
        if sym in BUILTINS or sym.startswith("__builtin") or sym == "openrac_hostgen_asm":
            continue
        if decl.get("isImplicit") and not (sym.startswith("func_") or sym in program.functions):
            # Clang's own declaration of a library builtin (fabsf, memcpy): only a
            # function the game itself names is indexed from an implicit declaration.
            if not any(p.match(sym) for p in program.code_names):
                continue
        try:
            sig = Signature.of(parse_type(decl))
        except ctype.TypeError_ as e:
            problems.append(f"{sym}: {e}")
            continue
        static = decl.get("storageClass") == "static"
        if has_body(decl) and not static:
            sym = program.grouped(unit, sym)
        if has_body(decl):
            from_main = decl.get("_file", "").endswith(unit) or not static
            tail = tail_call(decl, by_id) if canon(sig.ret) == "void" else None
            if static:
                if not from_main and not decl.get("_file", "").endswith(".c"):
                    # A static function from a header is translated in every unit that has it.
                    pass
                program.statics[(unit, sym)] = Function(sym, sig, unit, static=True, tail=tail)
                continue
            f = program.functions.get(sym)
            if f is not None and f.unit is not None and f.unit != unit:
                problems.append(f"{sym} is defined in {f.unit} and in {unit}; keeping the first")
                continue
            declared = f.declared if f else []
            program.functions[sym] = Function(sym, sig, unit, asm=f.asm if f else False, declared=declared, tail=tail)
        else:
            f = program.functions.get(sym)
            if f is None:
                f = program.functions[sym] = Function(sym, sig)
            f.declared.append(sig)
            if f.unit is None:
                f.sig = best_declaration(f.declared)
    return problems


def best_declaration(sigs: list[Signature]) -> Signature:
    """For a function defined nowhere in C: the declaration that says most."""
    proto = [s for s in sigs if s.prototyped]
    if not proto:
        return sigs[0]
    return max(proto, key=lambda s: len(s.params))


def read_places(path: Path, program: Program) -> int:
    """Each level function's address in each level that has it
    (config/overlays/functions.tsv of rac1/pal: name, kind, size,
    fingerprint, levels, places as level:address,...)."""
    if not path.exists():
        return 0
    count = 0
    for line in path.read_text().splitlines():
        if line.startswith("#") or not line.strip():
            continue
        cols = line.split("\t")
        if len(cols) < 6:
            continue
        places = []
        for item in cols[5].split(","):
            level, _, addr = item.partition(":")
            if level and addr:
                places.append((int(level), int(addr, 16)))
        if places:
            program.places[cols[0]] = places
            if cols[2].isdigit():
                program.sizes[cols[0]] = int(cols[2])
            count += 1
    return count


def read_split_places(path: Path, program: Program) -> int:
    """Places the catalogue folded into a function by its fingerprint although their code differs
    in a constant or a field offset (port/game/<id>/split_places.tsv: function, the variant's name,
    its places as level:address,...): each variant gets those places as its own, and calls to the
    function are resolved through the caller's own jal (lower.py, _folded_places).
    An optional fourth column names the executable's function whose level copies the variant's
    places are (its twin: the C calls it by its own name): the C's calls to the twin are calls to
    the variant's family where lower.py counts them (openrac_folded_call's k)."""
    if not path.exists():
        return 0
    count = 0
    for line in path.read_text().splitlines():
        if line.startswith("#") or not line.strip():
            continue
        cols = line.split("\t")
        if len(cols) < 3:
            continue
        name, variant = cols[0].strip(), cols[1].strip()
        places = []
        for item in cols[2].split(","):
            level, _, addr = item.strip().partition(":")
            if level and addr:
                places.append((int(level), int(addr, 16)))
        if not places:
            continue
        if name in program.places:
            program.places[name] = [p for p in program.places[name] if p not in places]
        program.places[variant] = places
        if name in program.sizes:
            program.sizes[variant] = program.sizes[name]
        program.splits.setdefault(name, []).extend(places)
        if len(cols) > 3 and cols[3].strip():
            program.split_twin[cols[3].strip()] = variant
        program.split_parent[variant] = name
        count += 1
    return count


def merge(into: Program, part: Program) -> list[str]:
    """Adds one unit's index (made in another process) to the program's."""
    problems = []
    for sym, f in part.functions.items():
        have = into.functions.get(sym)
        if have is None:
            into.functions[sym] = f
            continue
        have.asm = have.asm or f.asm
        if f.unit is not None:
            if have.unit is not None and have.unit != f.unit:
                problems.append(f"{sym} is defined in {have.unit} and in {f.unit}; keeping the first")
                continue
            declared = have.declared + f.declared
            into.functions[sym] = Function(sym, f.sig, f.unit, asm=have.asm, declared=declared, tail=f.tail)
        else:
            have.declared.extend(f.declared)
            if have.unit is None and have.declared:
                have.sig = best_declaration(have.declared)
    into.statics.update(part.statics)
    into.aliases.update(part.aliases)
    if part.groups and not into.groups:
        into.groups = part.groups
    for name, types in part.typedefs.items():
        into.typedefs.setdefault(name, set()).update(types)
    return problems


def canon(t: ctype.Type, typedefs: dict | None = None, depth: int = 0) -> str | None:
    """A type from a definition's signature, as the shared prototypes write it."""
    if isinstance(t, ctype.Ptr):
        return "gaddr"
    if not isinstance(t, ctype.Base):
        return None
    name = t.name
    if name in ("va_list", "__builtin_va_list", "__gnuc_va_list"):
        return "va_list"
    if name.startswith("enum "):
        return "int"
    if name in ctype.INT_NAMES or name in ctype.FLOAT_NAMES or name in ("void", "double", "long double"):
        return name
    if typedefs and name in typedefs and depth < 16:
        seen = set()
        for text in typedefs[name]:
            try:
                seen.add(canon(ctype.parse(text), typedefs, depth + 1))
            except ctype.TypeError_:
                seen.add(None)
        if len(seen) == 1:
            return seen.pop()
    return None


_SPLAT_SYMBOL = re.compile(r"^\s*([A-Za-z_][\w.$]*)\s*=\s*(0x[0-9A-Fa-f]+)\s*;")


def read_catalogue(path: Path, program: Program) -> int:
    """Seed functions absent from C using a name/address/size TSV.

    Retain signatures learned from C. Let asmc infer registers for entries
    declared nowhere. Catalogue sizes are not reliable function boundaries.
    """
    count = 0
    for line in path.read_text().splitlines():
        if not line.strip() or line.startswith('#'):
            continue
        cols = line.split('\t')
        if len(cols) < 3:
            raise ValueError(f'{path}: expected name, address and size')
        name, address = cols[0], int(cols[1], 0)
        if name not in program.functions:
            program.functions[name] = Function(
                name, Signature(ctype.Base('int'), (), False, False), asm=True)
            count += 1
        program.symbols.setdefault(name, address)
    return count


def read_symbols(path: Path, program: Program) -> int:
    """Named symbols and their addresses from a splat symbol file
    (`name = 0x00123456; // comment`), or from a JSON table of the values a decompilation's
    own build derived for names that state no address ({"values": {name: "0x..."}}: Going
    Commando's build/boot/placed.json)."""
    if not path.exists():
        return 0
    count = 0
    if path.suffix == ".json":
        for name, value in json.loads(path.read_text(errors="replace")).get("values", {}).items():
            if isinstance(value, str) and re.fullmatch(r"[A-Za-z_]\w*", name):
                program.symbols.setdefault(name, int(value, 0))
                count += 1
        return count
    for line in path.read_text(errors="replace").splitlines():
        m = _SPLAT_SYMBOL.match(line)
        if m:
            program.symbols.setdefault(m.group(1), int(m.group(2), 16))
            count += 1
    return count


def prototype(f, typedefs: dict | None = None) -> str | None:
    """A function's declaration as game_protos.h writes it (parameters a0, a1, ...), or None
    when a type in its signature has no form there (a record by value)."""
    sig = f.sig
    ret = canon(sig.ret, typedefs)
    if ret is None:
        return None
    if not sig.prototyped:
        return f"{ret} {cname(f.symbol)}()"
    params = []
    for i, p in enumerate(sig.params):
        c = canon(p, typedefs)
        if c is None:
            return None
        params.append(f"{c} a{i}")
    if sig.variadic:
        params.append("...")
    return f"{ret} {cname(f.symbol)}({', '.join(params) or 'void'})"
