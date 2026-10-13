# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""One translation unit of the decompilation, written again as host C.

The input is Clang's AST of the console's C (clangast.py); the output is C
that keeps the game's memory in game memory (port/runtime/include/openrac/
guest.h):

- a pointer is a gaddr, the game's 32-bit address; pointer arithmetic is
  scaled by the pointee's size, which is the console's size because every
  record keeps its layout (pointers inside it are 4-byte gaddrs too);
- a global is the object at its retail address: GREF(T, 0x0013E650u);
- *p, p->f and p[i] read and write game memory through G();
- a local whose address is taken, and every local array, struct or union,
  lives in a frame on the game stack (GFRAME), because the game passes its
  address around; other locals stay host locals;
- a direct call goes to the callee's definition, its arguments matched to
  the definition's parameters the way the EE passes them (integers and
  floats in separate registers); a call through a pointer goes through
  GFN, the table of functions by code address;
- string literals are copied into game memory once (GSTR).

What cannot be written that way (inline assembly other than the three
quadword helpers, a few rare constructs) makes the function a stub that
says so at run time, and the report lists why.
"""

from __future__ import annotations

import re

from dataclasses import dataclass, field

import ctype
from program import BUILTINS, Program, canon, cname, has_body, symbol_of

VA_NAMES = {"va_list", "__builtin_va_list", "__gnuc_va_list"}
HW_RANGES = [(0x10000000, 0x12002000)]  # EE registers, VU memory, GS registers
# Level programs' addresses (rac1: 0x15F000 up, with their data): what a shared level function's
# globals are relocated in (_shared_level).
LEVEL_RELOCATE = (0x0015F000, 0x00400000)


# Type names the host's standard headers (included by guest.h) define.
HOST_TYPES = {
    "size_t", "ssize_t", "ptrdiff_t", "wchar_t", "max_align_t", "off_t", "time_t", "FILE",
    "int8_t", "int16_t", "int32_t", "int64_t", "uint8_t", "uint16_t", "uint32_t", "uint64_t",
    "intptr_t", "uintptr_t", "intmax_t", "uintmax_t",
}

# The call hostgen puts where Clang could not read inline assembly (clangast.py).
HOSTGEN_ASM = "openrac_hostgen_asm"

# C library maths functions on plain values, called by the host's name when
# the game uses them without defining them.
HOST_MATH = {
    "fabsf", "fabs", "sqrtf", "sqrt", "sinf", "sin", "cosf", "cos", "tanf", "tan", "asinf", "asin",
    "acosf", "acos", "atanf", "atan", "atan2f", "atan2", "floorf", "floor", "ceilf", "ceil", "powf",
    "pow", "fmodf", "fmod", "expf", "exp", "logf", "log", "abs", "labs",
}


class Unsupported(Exception):
    """A construct this translator does not write; the function becomes a stub."""


@dataclass
class LV:
    """An lvalue: in game memory (addr) or a host variable (text only)."""

    text: str
    addr: str | None = None
    rec: str | None = None       # for an object: its record type, as host C


@dataclass
class FnReport:
    name: str
    status: str                  # "translated", "stub"
    reason: str = ""
    passthrough: int = 0         # arguments passed through from the caller's registers
    hw: int = 0                  # accesses to hardware register addresses
    calls: set = field(default_factory=set)  # functions it calls or takes the address of


@dataclass
class UnitReport:
    unit: str
    functions: list[FnReport] = field(default_factory=list)
    problems: list[str] = field(default_factory=list)
    fixes: list[str] = field(default_factory=list)


@dataclass
class _Fn:
    """The function being translated."""

    name: str
    sig: ctype.Func
    params: list
    memory: dict = field(default_factory=dict)       # decl id -> frame field
    statics: dict = field(default_factory=dict)      # decl id -> static address variable
    host: dict = field(default_factory=dict)         # decl id -> host name
    frame: list = field(default_factory=list)        # (field, type)
    report: FnReport | None = None
    tail_id: str | None = None                       # the last statement, a call whose result is returned
    labels: dict = field(default_factory=dict)       # label decl id -> name
    int_params: list = field(default_factory=list)   # host names, in register order
    record_params: set = field(default_factory=set)  # host names of parameters passed as records
    float_params: list = field(default_factory=list)
    wide: set = field(default_factory=set)           # locals kept 64-bit (returns_64)


def _inner(n: dict) -> list:
    return n.get("inner", [])


def _strip_parens(n: dict) -> dict:
    while n.get("kind") == "ParenExpr":
        n = _inner(n)[0]
    return n


def hexaddr(a: int) -> str:
    return f"0x{a:08X}u"


class Unit:
    def __init__(self, program: Program, unit: str, ast: dict, report: UnitReport):
        self.program = program
        self.unit = unit
        self.ast = ast
        self.report = report
        self.decls: dict[str, dict] = {}
        self.tag_of: dict[str, str] = {}          # anonymous record/enum id -> typedef name
        self.typedefs: dict[str, ctype.Type] = {}  # sugared
        self.anon_records: set[str] = set()
        self.fn: _Fn | None = None
        self.local_names: dict[str, str] = {}
        self.emitted_tags: set[str] = set()  # the unnamed records written so far
        # Type names that are also a function's name (a names.h macro used as a
        # typedef name): renamed in this unit, since the port declares the function.
        self.type_renames: dict[str, str] = {}
        self._index(ast)
        for name in list(self.typedefs):
            if name in HOST_TYPES:
                # The game's own size_t or uint32_t: the host's headers define those.
                self.type_renames[name] = f"openrac_game_{name}"
                continue
            if name.startswith("func_") or name in program.functions:
                self.type_renames[name] = f"openrac_type_{name}"
                self.type_renames[f"openrac_td_{name}"] = f"openrac_td_type_{name}"

    # ---- What the unit declares ----

    def _index(self, node: dict) -> None:
        stack = [node]
        while stack:
            n = stack.pop()
            if "id" in n and n.get("kind", "").endswith("Decl"):
                self.decls[n["id"]] = n
            if n.get("kind") == "TypedefDecl":
                anon_word = None
                for c in _inner(n):
                    owned = c.get("ownedTagDecl")
                    if owned and not owned.get("name"):
                        self.tag_of[owned["id"]] = n["name"]
                        anon_word = c["type"]["qualType"].split(" ", 1)[0]
                try:
                    if anon_word:
                        self.typedefs[n["name"]] = ctype.Base(f"{anon_word} openrac_td_{n['name']}")
                    else:
                        self.typedefs[n["name"]] = ctype.parse(n["type"]["qualType"])
                except ctype.TypeError_:
                    pass
            stack.extend(reversed(_inner(n)))

    # ---- Types ----

    def ty(self, n: dict) -> ctype.Type:
        """The node's type as written (typedef names kept)."""
        try:
            return ctype.parse(n["type"]["qualType"])
        except ctype.TypeError_ as e:
            raise Unsupported(str(e)) from e

    def resolve(self, t: ctype.Type) -> ctype.Type:
        seen = 0
        while isinstance(t, ctype.Base) and t.name in self.typedefs and seen < 32:
            if t.name in VA_NAMES:
                return t
            nxt = self.typedefs[t.name]
            if isinstance(nxt, ctype.Base) and nxt.name in (f"struct {t.name}", f"union {t.name}", f"enum {t.name}"):
                return nxt
            t = nxt
            seen += 1
        return t

    def cls(self, t: ctype.Type) -> str:
        t = self.resolve(t)
        if isinstance(t, ctype.Ptr):
            return "ptr"
        if isinstance(t, ctype.Arr):
            return "arr"
        if isinstance(t, ctype.Func):
            return "func"
        name = t.name
        if name in VA_NAMES:
            return "valist"
        if name == "void":
            return "void"
        if name == "float":
            return "float"
        if name in ("double", "long double"):
            return "double"
        if name.startswith(("struct ", "union ")):
            return "rec"
        return "int"

    def host(self, t: ctype.Type) -> str:
        if isinstance(t, ctype.Base) and t.name in VA_NAMES:
            return "va_list"
        return ctype.host(self._rename_type(t))

    _FOUR_BYTE_ELEMENTS = {"float", "int", "unsigned int", "unsigned", "signed int", "s32", "u32", "f32"}

    def _frame_alignment(self, t: ctype.Type) -> str:
        """A local array of 16 bytes or more, whole quadwords of 32-bit elements (a vector, a
        matrix), starts on a quadword in the frame: the console's compiler placed such locals so,
        and the game copies them with quadword loads (lq), which the host makes movaps."""
        count = 1
        t = self.resolve(t)
        while isinstance(t, ctype.Arr):
            if t.size is None:
                return ""
            count *= t.size
            t = self.resolve(t.of)
        if isinstance(t, ctype.Base) and t.name in self._FOUR_BYTE_ELEMENTS and count >= 4 and count % 4 == 0:
            return " __attribute__((aligned(16)))"
        return ""

    def declare(self, t: ctype.Type, name: str) -> str:
        if isinstance(t, ctype.Base) and t.name in VA_NAMES:
            return f"va_list {name}"
        return ctype.declare(self._rename_type(t), name)

    # Types a function declares inside itself are written at file scope, before
    # the function (its frame may hold them), under names of their own.
    def _renamed(self, name: str) -> str:
        name = self.local_names.get(name, name)
        return self.type_renames.get(name, name)

    def _rename_type(self, t: ctype.Type) -> ctype.Type:
        if not self.local_names and not self.type_renames:
            return t
        if isinstance(t, ctype.Base):
            word, _, rest = t.name.partition(" ")
            if word in ("struct", "union", "enum") and rest:
                new = self._renamed(rest)
                return ctype.Base(f"{word} {new}", t.quals) if new != rest else t
            new = self._renamed(t.name)
            return ctype.Base(new, t.quals) if new != t.name else t
        if isinstance(t, ctype.Ptr):
            return t
        if isinstance(t, ctype.Arr):
            return ctype.Arr(self._rename_type(t.of), t.size)
        if isinstance(t, ctype.Func):
            return ctype.Func(self._rename_type(t.ret), tuple(self._rename_type(p) for p in t.params),
                              t.variadic, t.prototyped)
        return t

    def canon(self, t: ctype.Type) -> str | None:
        """The type as the shared prototypes write it, or None (a record by value)."""
        c = self.cls(t)
        if c == "ptr":
            return "gaddr"
        if c == "valist":
            return "va_list"
        if c == "rec":
            return None
        r = self.resolve(t)
        if isinstance(r, ctype.Base):
            if r.name.startswith("enum "):
                return "int"
            if r.name in self.typedefs or r.name in ctype.INT_NAMES or r.name in ctype.FLOAT_NAMES or r.name in (
                    "void", "double", "long double"):
                return r.name
            # A typedef of another file (a callee's signature): the program's.
            return canon(r, self.program.typedefs)
        return None

    def pointee_size(self, t: ctype.Type) -> str:
        r = self.resolve(t)
        if not isinstance(r, ctype.Ptr):
            raise Unsupported(f"pointer arithmetic on {ctype.host(t)}")
        to = r.to
        c = self.cls(to)
        if c in ("void", "func"):
            return "1"
        return f"(gaddr)sizeof({self.host(ctype.strip_quals(to))})"

    def gref(self, t: ctype.Type, addr: str) -> str:
        r = self.resolve(t)
        if isinstance(r, (ctype.Arr, ctype.Func)):
            return f"(*({self.declare(t, '(*)')})G({addr}))"
        return f"GREF({self.host(t)}, {addr})"

    # ---- Emission of the unit ----

    def emit(self) -> str:
        out = [
            f"/* Generated by port/tools/hostgen from {self.unit}. Do not edit: change the",
            " * decompilation, or hostgen, and build again. */",
            "#include <math.h>",
            "#include <stdarg.h>",
            "#include \"openrac/guest.h\"",
            "#include \"game_protos.h\"",
            "",
        ]
        statics_protos = []
        bodies = []
        for decl in _inner(self.ast):
            kind = decl.get("kind")
            if decl.get("isImplicit"):
                continue  # Clang's own builtin declarations
            if kind == "RecordDecl":
                out.extend(self.record(decl))
            elif kind == "EnumDecl":
                out.extend(self.enum(decl))
            elif kind == "TypedefDecl":
                out.extend(self.typedef(decl))
            elif kind == "VarDecl":
                self.file_var(decl)
            elif kind == "FunctionDecl" and has_body(decl):
                if not self._mine(decl):
                    continue
                proto, text = self.function(decl)
                if decl.get("storageClass") == "static":
                    statics_protos.append(proto + ";")
                bodies.append(text)
        return "\n".join(out + [""] + statics_protos + [""] + bodies) + "\n"

    def _mine(self, decl: dict) -> bool:
        """Definitions this unit translates: its own, and static ones from headers.
        A function the port writes itself (a library it replaces) is left out."""
        sym = self._defined_symbol(decl)
        if sym in BUILTINS:
            return False
        if sym in self.program.host and decl.get("storageClass") != "static":
            return False
        if decl.get("storageClass") != "static":
            first = self.program.functions.get(sym)
            if first is not None and first.unit is not None and first.unit != self.unit:
                # Defined in two units: the program keeps the first (index problems).
                self.report.problems.append(f"{sym} is defined again here; {first.unit}'s definition is used")
                return False
        if decl.get("_file") == self.unit:
            return True
        return decl.get("storageClass") == "static"

    def record_tag(self, decl: dict) -> str | None:
        if decl.get("name"):
            return self._renamed(decl["name"])
        if decl["id"] in self.tag_of:
            return self._renamed(f"openrac_td_{self.tag_of[decl['id']]}")
        return None

    def record(self, decl: dict, indent: str = "", member: bool = False,
               hoist: list[str] | None = None) -> list[str]:
        tag_word = decl.get("tagUsed", "struct")
        tag = self.record_tag(decl)
        if tag is None and not member:
            tag = ctype.anon_tag(f"{decl.get('_file', '')}:{decl.get('_line', 0)}:{decl.get('_col', 0)}")
        if not decl.get("completeDefinition"):
            return [f"{indent}{tag_word} {tag};"] if tag else []
        if tag:
            self.emitted_tags.add(tag)
        # A tag declared inside a record belongs to the enclosing scope in C, so
        # it is written before the outermost record instead of in it: with
        # Microsoft's extensions (Clang's default when targeting Windows) a
        # tagged declaration inside a record is an anonymous member.
        before = [] if hoist is None else hoist
        attrs = []
        lines = [f"{indent}{tag_word}{(' ' + tag) if tag else ''} {{"]
        children = _inner(decl)
        i = 0
        while i < len(children):
            c = children[i]
            k = c.get("kind")
            if k == "AlignedAttr":
                attrs.append(f"aligned({self._attr_value(c)})")
            elif k == "PackedAttr":
                attrs.append("packed")
            elif k == "RecordDecl":
                nxt = children[i + 1] if i + 1 < len(children) else {}
                if nxt.get("kind") == "FieldDecl" and nxt.get("isImplicit"):
                    # An anonymous member: written untagged, in place.
                    body = self.record(c, indent + "    ", member=True, hoist=before)
                    body[-1] = body[-1].rstrip(";") + ";"
                    lines.extend(body)
                    i += 2
                    continue
                before.extend(self.record(c, indent, hoist=before))
            elif k == "FieldDecl":
                lines.append(indent + "    " + self.field(c) + ";")
            i += 1
        attr = f" __attribute__(({', '.join(attrs)}))" if attrs else ""
        lines.append(f"{indent}}}{attr};")
        return lines if hoist is not None else before + lines

    def _attr_value(self, attr: dict) -> str:
        for c in _inner(attr):
            if "value" in c:
                return c["value"]
        return "16"

    def field(self, f: dict) -> str:
        t = self.ty(f)
        text = self.declare(t, f.get("name", ""))
        attrs = []
        width = None
        for c in _inner(f):
            if c.get("kind") == "AlignedAttr":
                attrs.append(f"aligned({self._attr_value(c)})")
            elif c.get("kind") == "PackedAttr":
                attrs.append("packed")
            elif f.get("isBitfield") and "value" in c:
                width = c["value"]
        if width is not None:
            text += f" : {width}"
        if attrs:
            text += f" __attribute__(({', '.join(attrs)}))"
        return text

    def enum(self, decl: dict) -> list[str]:
        tag = decl.get("name") or (f"openrac_td_{self.tag_of[decl['id']]}" if decl["id"] in self.tag_of else None)
        if tag and decl.get("name"):
            tag = self._renamed(tag)
        items = []
        for c in _inner(decl):
            if c.get("kind") != "EnumConstantDecl":
                continue
            value = next((x.get("value") for x in _inner(c) if "value" in x), None)
            items.append(f"    {c['name']}" + (f" = {value}" if value is not None else "") + ",")
        if not items:
            return [f"enum {tag};"] if tag else []
        return [f"enum{(' ' + tag) if tag else ''} {{", *items, "};"]

    def typedef(self, decl: dict) -> list[str]:
        name = decl["name"]
        if name in VA_NAMES:
            return []
        try:
            t = ctype.parse(decl["type"]["qualType"])
        except ctype.TypeError_ as e:
            self.report.problems.append(f"typedef {name}: {e}")
            return []
        if isinstance(t, ctype.Base) and t.name in VA_NAMES:
            return [f"typedef va_list {name};"]
        for c in _inner(decl):
            owned = c.get("ownedTagDecl")
            if owned and not owned.get("name") and isinstance(t, ctype.Base):
                word = t.name.split(" ", 1)[0]
                return [f"typedef {word} {self._renamed('openrac_td_' + name)} {self._renamed(name)};"]
        return [f"typedef {self.declare(t, self._renamed(name))};"]

    def file_var(self, decl: dict) -> None:
        name = decl.get("name", "")
        if name.startswith("openrac_asm_"):
            return
        if decl.get("storageClass") != "extern" and (decl.get("init") or decl.get("_file") == self.unit):
            self.report.problems.append(
                f"{name}: a global defined in C; the port reads the game's data from its executable, "
                "so this definition is not used")

    # ---- Functions ----

    def signature(self, sym: str, sig_t: ctype.Func, names: list[str], static: bool) -> str:
        params = []
        for t, n in zip(sig_t.params, names):
            c = self.canon(t)
            params.append(f"{c} {n}" if c is not None else self.declare(t, n))
        if sig_t.variadic:
            params.append("...")
        ret = self.canon(sig_t.ret)
        if ret is None:
            ret = self.host(sig_t.ret)
        # An unprototyped definition (`int f()`) stays one: callers may pass arguments.
        plist = ", ".join(params) if params else ("void" if sig_t.prototyped else "")
        name = cname(sym) + ("__game" if sym in self.program.wrap and not static else "")
        return f"{'static ' if static else ''}{ret} {name}({plist})"

    def _defined_symbol(self, decl: dict) -> str:
        sym = symbol_of(decl)
        if decl.get("storageClass") == "static":
            return sym
        return self.program.grouped(self.unit, sym)

    def function(self, decl: dict) -> tuple[str, str]:
        sym = self._defined_symbol(decl)
        static = decl.get("storageClass") == "static"
        try:
            sig_t = ctype.parse(decl["type"]["qualType"])
        except ctype.TypeError_ as e:
            raise SystemExit(f"{self.unit}: {sym}: {e}")
        if not isinstance(sig_t, ctype.Func):
            raise SystemExit(f"{self.unit}: {sym} has type {sig_t}")
        params = [c for c in _inner(decl) if c.get("kind") == "ParmVarDecl"]
        names = [p.get("name") or f"arg{i}" for i, p in enumerate(params)]
        # A definition that returns nothing and ends in a call returns that call's result, as the
        # console's result register did (program.py, inherit_results).
        known = self.program.lookup(self.unit, sym)
        inherits = known is not None and getattr(known, "inherits", False) and self.cls(sig_t.ret) == "void"
        if inherits:
            sig_t = ctype.Func(known.sig.ret, sig_t.params, sig_t.variadic, sig_t.prototyped)
        proto = self.signature(sym, sig_t, names, static)
        report = FnReport(sym, "translated")
        self.fn = _Fn(sym, sig_t, params, report=report)
        body = next(c for c in _inner(decl) if c.get("kind") == "CompoundStmt")
        if inherits:
            self.fn.tail_id = [s for s in _inner(body) if s.get("kind")][-1].get("id")
        local_types = self._local_types(body)
        self.local_names = {}
        for d in local_types:
            if d.get("name"):
                self.local_names[d["name"]] = f"{cname(sym)}__{d['name']}"
                self.local_names[f"openrac_td_{d['name']}"] = f"openrac_td_{cname(sym)}__{d['name']}"
        try:
            hoisted = []
            for d in local_types:
                k = d.get("kind")
                hoisted += self.record(d) if k == "RecordDecl" else self.enum(d) if k == "EnumDecl" else self.typedef(d)
            hoisted_text = "\n".join(hoisted) + "\n" if hoisted else ""
            frame, text = self._function_body(decl, body, params, names)
            missing = set(re.findall(r"\bopenrac_anon_\w+", frame + text)) - self.emitted_tags
            if missing:
                # Clang's JSON leaves out a record defined inside an expression
                # (a cast to an unnamed struct): its layout is not known here.
                raise Unsupported("an unnamed struct defined inside an expression")
            out = f"{hoisted_text}{frame}{proto}\n{text}"
        except Unsupported as e:
            report.status = "stub"
            report.reason = str(e)
            unused = " ".join(f"(void){n};" for n in names)
            ret_canon = self.canon(sig_t.ret)
            ret = "" if self.cls(sig_t.ret) == "void" else (
                " return 0;" if ret_canon else f" {{ {self.host(sig_t.ret)} zero_; memset(&zero_, 0, sizeof zero_); return zero_; }}")
            out = (f"/* Not translated: {e} */\n{proto} {{\n    {unused}\n"
                   f"    openrac_guest_missing(\"{sym}\");\n   {ret}\n}}\n")
        self.report.functions.append(report)
        self.fn = None
        self.local_names = {}
        return proto, out

    def _local_types(self, body: dict) -> list[dict]:
        out = []
        stack = [body]
        seen = set()
        while stack:
            n = stack.pop()
            # Anywhere in the body: in a declaration, or inside a cast or sizeof.
            if n.get("kind") in ("RecordDecl", "EnumDecl", "TypedefDecl") and n.get("id") not in seen:
                seen.add(n.get("id"))
                out.append(n)
                continue  # its own members are written with it
            stack.extend(reversed(_inner(n)))
        return out

    def _function_body(self, decl: dict, body: dict, params: list, names: list[str]) -> str:
        fn = self.fn
        memory_ids = self._memory_resident(body, params)
        for p, n in zip(params, names):
            fn.host[p["id"]] = n
            c = self.cls(self.ty(p))
            (fn.float_params if c == "float" else fn.int_params).append(n)
            if c == "rec":
                fn.record_params.add(n)
        self._collect_labels(body)
        prologue = []
        for p, n in zip(params, names):
            if p["id"] in memory_ids:
                f = self._frame_field(p, n)
                fn.memory[p["id"]] = f
        # Locals: frame fields for memory-resident ones, in declaration order (a set's order
        # changes from run to run, and with it the frame's layout: a candidate that overruns a
        # buffer would then fail in one build and not the next).
        order = []
        stack = [body]
        while stack:
            n = stack.pop()
            if n.get("kind") == "VarDecl" and n.get("id") in memory_ids:
                order.append(n["id"])
            stack.extend(reversed(_inner(n)))
        order += sorted(v for v in memory_ids if v not in set(order))
        for vid in order:
            d = self.decls.get(vid)
            if d is not None and d.get("kind") == "VarDecl" and vid not in fn.memory:
                if d.get("storageClass") == "static":
                    fn.statics[vid] = f"{d.get('name')}_at_"
                else:
                    fn.memory[vid] = self._frame_field(d, d.get("name", "v"))
        fn.wide = self._wide_locals(body)
        lines = self.compound(body, 1, top=True)
        frame_decl = []
        if fn.frame:
            fields = "\n".join(f"    {self.declare(t, f)}{self._frame_alignment(t)};" for f, t in fn.frame)
            frame_decl = [f"struct openrac_frame_{fn.name} {{\n{fields}\n}};"]
            prologue.append(f"    GFRAME(frame_, sizeof(struct openrac_frame_{fn.name}));")
            for p, n in zip(params, names):
                if p["id"] in fn.memory:
                    prologue.append(f"    {self._mem_text(p['id'])} = {n};")
        for vid, var in fn.statics.items():
            d = self.decls[vid]
            prologue.insert(0, f"    static gaddr {var};")
            init = self._static_init(d, var)
            prologue.append(f"    if ({var} == 0) {{ {var} = openrac_guest_static(sizeof({self.host(self.ty(d))})); {init}}}")
        if getattr(fn, "uses_self", False):
            prologue.insert(0, f"    const gaddr openrac_self_ = openrac_guest_entry((openrac_host_fn){cname(fn.name)});")
        text = "{\n" + "\n".join(prologue + lines[1:])
        return ("\n".join(frame_decl) + "\n" if frame_decl else ""), text

    def _frame_field(self, d: dict, name: str):
        fn = self.fn
        r = self.resolve(self.ty(d))
        while isinstance(r, ctype.Arr):
            if r.vla:
                raise Unsupported(f"variable-length array {name}")
            r = self.resolve(r.of)
        fname = f"{name}_{len(fn.frame)}"
        fn.frame.append((fname, self.ty(d)))
        return fname

    def _mem_text(self, vid: str) -> str:
        d = self.decls[vid]
        return self.gref(self.ty(d), self._mem_addr(vid))

    def _mem_addr(self, vid: str) -> str:
        fn = self.fn
        if vid in fn.statics:
            return fn.statics[vid]
        return f"(frame_ + (gaddr)offsetof(struct openrac_frame_{fn.name}, {fn.memory[vid]}))"

    def _static_init(self, d: dict, var: str) -> str:
        init = [c for c in _inner(d) if c.get("kind") not in ("AlignedAttr", "AsmLabelAttr")]
        if not d.get("init") or not init:
            return ""
        return self._store_init(self.ty(d), var, init[0]) + " "

    def _store_init(self, t: ctype.Type, addr: str, init: dict) -> str:
        c = self.cls(t)
        if c in ("rec", "arr"):
            return (f"{{ {self.declare(t, 'init_')} = {self.initializer(init, t)}; "
                    f"memcpy(G({addr}), &init_, sizeof init_); }}")
        return f"{self.gref(t, addr)} = {self.convert(init, t)};"

    def _memory_resident(self, body: dict, params: list) -> set[str]:
        """Locals and parameters that must live in game memory."""
        out = set()
        local_ids = {p["id"] for p in params}
        stack = [body]
        while stack:
            n = stack.pop()
            k = n.get("kind")
            if k == "VarDecl":
                local_ids.add(n["id"])
                if n.get("storageClass") != "extern" and self.cls(self.ty(n)) in ("rec", "arr"):
                    out.add(n["id"])
            elif k == "UnaryOperator" and n.get("opcode") == "&":
                target = _strip_parens(_inner(n)[0])
                while target.get("kind") in ("MemberExpr", "ArraySubscriptExpr") and not (
                        target.get("kind") == "MemberExpr" and target.get("isArrow")):
                    target = _strip_parens(_inner(target)[0])
                    if target.get("kind") == "ImplicitCastExpr" and target.get("castKind") == "ArrayToPointerDecay":
                        target = _strip_parens(_inner(target)[0])
                if target.get("kind") == "DeclRefExpr":
                    ref = target["referencedDecl"]
                    if ref.get("kind") in ("VarDecl", "ParmVarDecl"):
                        out.add(ref["id"])
            stack.extend(_inner(n))
        for p in params:
            if self.cls(self.ty(p)) == "rec":
                out.add(p["id"])
        return {i for i in out if i in local_ids or self.decls.get(i, {}).get("kind") == "ParmVarDecl"}

    def _collect_labels(self, body: dict) -> None:
        stack = [body]
        while stack:
            n = stack.pop()
            if n.get("kind") == "LabelStmt":
                self.fn.labels[n.get("declId", n.get("id"))] = n["name"]
            stack.extend(_inner(n))

    # ---- Statements ----

    def compound(self, n: dict, depth: int, top: bool = False) -> list[str]:
        pad = "    " * depth
        lines = ["    " * (depth - 1) + "{"]
        for s in _inner(n):
            lines.extend(self.stmt(s, depth))
        lines.append("    " * (depth - 1) + "}")
        return lines

    def body(self, n: dict, depth: int) -> list[str]:
        if n.get("kind") == "CompoundStmt":
            return self.compound(n, depth + 1)
        return ["    " * depth + "{", *self.stmt(n, depth + 1), "    " * depth + "}"]

    def stmt(self, n: dict, depth: int) -> list[str]:
        pad = "    " * depth
        k = n.get("kind")
        if k is None:
            return []
        if k == "CompoundStmt":
            return self.compound(n, depth + 1)
        if k == "DeclStmt":
            out = []
            for d in _inner(n):
                out.extend(self.local_decl(d, depth))
            return out
        if self.fn is not None and self.fn.tail_id is not None and n.get("id") == self.fn.tail_id:
            ret = self.host(ctype.strip_quals(self.fn.sig.ret))
            call = self.rv(n)
            if call.startswith("GFN(void (*)"):
                # Called by code address (a folded wrapper's own callee): through a pointer to a
                # function with the result this one returns, as the definition there has it.
                call = f"GFN({ret} (*)" + call[len("GFN(void (*)"):]
            return [pad + f"return ({ret})({call});"]
        if k == "ReturnStmt":
            kids = _inner(n)
            if not kids:
                # In a definition that returns its last call's result, an earlier return leaves
                # the register as it was: nothing a caller could use.
                return [pad + ("return 0;" if self.fn.tail_id is not None else "return;")]
            if self.cls(self.fn.sig.ret) == "void":
                return [pad + f"{self.rv(kids[0])};", pad + "return;"]
            return [pad + f"return {self.convert(kids[0], self.fn.sig.ret)};"]
        if k == "IfStmt":
            kids = _inner(n)
            if n.get("hasInit") or n.get("hasVar"):
                raise Unsupported("if with a declaration")
            cond, then = kids[0], kids[1]
            out = [pad + f"if ({self.rv(cond)})", *self.body(then, depth)]
            if n.get("hasElse") and len(kids) > 2:
                out += [pad + "else", *self.body(kids[2], depth)]
            return out
        if k == "WhileStmt":
            kids = _inner(n)
            return [pad + f"while ({self.rv(kids[0])})", *self.body(kids[1], depth)]
        if k == "DoStmt":
            kids = _inner(n)
            return [pad + "do", *self.body(kids[0], depth), pad + f"while ({self.rv(kids[1])});"]
        if k == "ForStmt":
            init, var, cond, inc, body = (_inner(n) + [{}] * 5)[:5]
            if var:
                raise Unsupported("for with a condition variable")
            init_text = ""
            pre = []
            if init:
                if init.get("kind") == "DeclStmt":
                    pre = self.stmt(init, depth)
                else:
                    init_text = self.rv(init)
            cond_text = self.rv(cond) if cond else ""
            inc_text = self.rv(inc) if inc else ""
            loop = [pad + f"for ({init_text}; {cond_text}; {inc_text})", *self.body(body, depth)]
            if pre:
                return [pad + "{", *pre, *["    " + line for line in loop], pad + "}"]
            return loop
        if k == "SwitchStmt":
            kids = _inner(n)
            return [pad + f"switch ({self.rv(kids[0])})", *self.body(kids[-1], depth)]
        if k == "CaseStmt":
            kids = _inner(n)
            lo = self._const(kids[0])
            if n.get("isGNURange"):
                hi = self._const(kids[1])
                label = f"case {lo} ... {hi}:"
                sub = kids[2] if len(kids) > 2 else {}
            else:
                label = f"case {lo}:"
                sub = kids[1] if len(kids) > 1 else {}
            return ["    " * max(depth - 1, 0) + label, *self.stmt(sub, depth)] if sub else [pad + label + " ;"]
        if k == "DefaultStmt":
            kids = _inner(n)
            return ["    " * max(depth - 1, 0) + "default:", *(self.stmt(kids[0], depth) if kids else [pad + ";"])]
        if k == "BreakStmt":
            return [pad + "break;"]
        if k == "ContinueStmt":
            return [pad + "continue;"]
        if k == "GotoStmt":
            name = self.fn.labels.get(n.get("targetLabelDeclId"))
            if name is None:
                raise Unsupported("goto to an unknown label")
            return [pad + f"goto {name};"]
        if k == "LabelStmt":
            kids = _inner(n)
            return [f"{n['name']}:", *(self.stmt(kids[0], depth) if kids else [pad + ";"])]
        if k == "NullStmt":
            return [pad + ";"]
        if k == "GCCAsmStmt":
            return [pad + self.asm_stmt(n)]
        if k in ("AttributedStmt",):
            return self.stmt(_inner(n)[-1], depth)
        # An expression statement.
        return [pad + self.rv(n) + ";"]

    def asm_stmt(self, n: dict) -> str:
        text = n.get("asmString", "") or ""
        first = text.strip().split()[0] if text.strip() else ""
        if first in ("", "sync", "sync.l", "sync.p"):
            return "GBARRIER();"
        raise Unsupported(f"inline assembly ({first})")

    def _const(self, n: dict) -> str:
        if "value" in n:
            return n["value"]
        return self.rv(n)

    # ---- Values the console keeps 64-bit in a register (hostgen.json "returns_64") ----
    #
    # A function that returns a 64-bit value which the matching C declares as int (GetEffectTex's
    # TEX0): on the console its caller's int local or expression still holds all 64 bits in the
    # register, and passes them on whole. Here such a call keeps its 64 bits, and a local that only
    # receives it is declared 64-bit.

    def _wide_call(self, n: dict) -> bool:
        while n.get("kind") in ("ParenExpr", "ImplicitCastExpr", "CStyleCastExpr"):
            if n.get("kind") == "CStyleCastExpr" and self.cls(self.ty(n)) != "int":
                return False
            n = _inner(n)[0]
        if n.get("kind") != "CallExpr":
            return False
        c = _strip_parens(_inner(n)[0])
        if c.get("kind") == "ImplicitCastExpr" and c.get("castKind") == "FunctionToPointerDecay":
            target = _strip_parens(_inner(c)[0])
            if target.get("kind") == "DeclRefExpr" and target["referencedDecl"].get("kind") == "FunctionDecl":
                return self._callee_symbol(target) in getattr(self.program, "returns_64", ())
        return False

    def _is_int32(self, t: ctype.Type) -> bool:
        r = self.resolve(t)
        return isinstance(r, ctype.Base) and r.name in ("int", "signed int", "unsigned int", "unsigned")

    def _wide_locals(self, body: dict) -> set:
        wide = set()
        returns = getattr(self.program, "returns_64", ())
        if not returns:
            return wide
        stack = [body]
        while stack:
            n = stack.pop()
            k = n.get("kind")
            if k == "VarDecl" and n.get("init") and n["id"] not in self.fn.memory and n.get("storageClass") not in ("static", "extern"):
                init = [c for c in _inner(n) if c.get("kind") not in ("AlignedAttr", "AsmLabelAttr", "SectionAttr")]
                if init and self._is_int32(self.ty(n)) and self._wide_call(init[0]):
                    wide.add(n["id"])
            elif k == "BinaryOperator" and n.get("opcode") == "=":
                a, b = _inner(n)
                a = _strip_parens(a)
                if a.get("kind") == "DeclRefExpr" and a["referencedDecl"].get("kind") == "VarDecl":
                    vid = a["referencedDecl"]["id"]
                    if vid not in self.fn.memory and self._is_int32(self.ty(a)) and self._wide_call(b):
                        wide.add(vid)
            stack.extend(_inner(n))
        return wide

    def local_decl(self, d: dict, depth: int) -> list[str]:
        pad = "    " * depth
        k = d.get("kind")
        if k != "VarDecl":  # types are written before the function (_local_types)
            return []
        fn = self.fn
        t = self.ty(d)
        name = d.get("name", "v")
        init = [c for c in _inner(d) if c.get("kind") not in ("AlignedAttr", "AsmLabelAttr", "SectionAttr")]
        init = init[0] if (d.get("init") and init) else None
        if d.get("storageClass") == "extern":
            return []
        if d["id"] in fn.statics:
            return []  # set up in the prologue
        if d["id"] in fn.memory:
            if init is None:
                return []
            return [pad + self._store_init(t, self._mem_addr(d["id"]), init)]
        if d.get("storageClass") == "static":
            fn.host[d["id"]] = name
            text = f"static {self.declare(t, name)}"
            if init is not None:
                value = self.initializer(init, t)
                if "GSTR" in value or "G(" in value:
                    raise Unsupported(f"static local {name} initialised with an address")
                text += f" = {value}"
            return [pad + text + ";"]
        fn.host[d["id"]] = name
        if d["id"] in fn.wide:
            t = ctype.Base("long long")
        text = self.declare(t, name)
        if init is not None:
            text += f" = {self.initializer(init, t)}"
        return [pad + text + ";"]

    def initializer(self, init: dict, t: ctype.Type) -> str:
        k = init.get("kind")
        if k == "InitListExpr":
            r = self.resolve(t)
            items = _inner(init)
            if "array_filler" in init and not items:
                # A partly initialised array: Clang gives the filler (the zeros after the last
                # element), then the elements, under array_filler and no inner.
                items = init["array_filler"][1:]
            if isinstance(r, ctype.Arr):
                parts = [self.initializer(i, r.of) for i in items]
                return "{" + ", ".join(parts) + "}" if parts else "{0}"
            if self.cls(t) == "rec":
                fields = self._record_fields(r)
                if init.get("field"):  # a union: the member initialised
                    name = init["field"].get("name", "")
                    ft = self._field_type(r, name)
                    return "{ ." + name + " = " + (self.initializer(items[0], ft) if items else "0") + " }"
                parts = []
                for item, (fname, ft) in zip(items, fields):
                    parts.append(self.initializer(item, ft))
                return "{" + ", ".join(parts) + "}" if parts else "{0}"
            return self.convert(items[0], t) if items else "0"
        if k == "ImplicitValueInitExpr":
            return "{0}" if self.cls(t) in ("rec", "arr") else "0"
        if k == "StringLiteral" and self.cls(t) == "arr":
            return init["value"]
        return self.convert(init, t)

    def _record_decl(self, r: ctype.Type) -> dict | None:
        if not isinstance(r, ctype.Base):
            return None
        word, _, tag = r.name.partition(" ")
        for d in self.decls.values():
            if d.get("kind") == "RecordDecl" and d.get("completeDefinition") and self.record_tag(d) == tag:
                return d
        for d in self.decls.values():
            if d.get("kind") == "RecordDecl" and d.get("completeDefinition") and ctype.anon_tag(
                    f"{d.get('_file', '')}:{d.get('_line', 0)}:{d.get('_col', 0)}") == tag:
                return d
        return None

    def _record_fields(self, r: ctype.Type) -> list[tuple[str, ctype.Type]]:
        d = self._record_decl(r)
        if d is None:
            raise Unsupported(f"initialiser for an unknown record {r}")
        out = []
        for c in _inner(d):
            if c.get("kind") == "FieldDecl":
                out.append((c.get("name", ""), self.ty(c)))
        return out

    def _field_type(self, r: ctype.Type, name: str) -> ctype.Type:
        for fname, ft in self._record_fields(r):
            if fname == name:
                return ft
        raise Unsupported(f"no field {name}")

    # ---- Expressions ----

    def convert(self, n: dict, t: ctype.Type) -> str:
        """n as a value of type t (an assignment's or a return's conversion)."""
        c = self.cls(t)
        v = self.rv(n)
        if c in ("rec", "arr", "valist", "void"):
            return v
        if c == "ptr":
            return f"(gaddr)({v})"
        return f"({self.host(ctype.strip_quals(t))})({v})"

    def rv(self, n: dict) -> str:
        k = n.get("kind")
        m = getattr(self, f"rv_{k}", None)
        if m is None:
            if n.get("valueCategory") == "lvalue":
                return self.lv(n).text
            raise Unsupported(f"expression {k}")
        return m(n)

    def rv_IntegerLiteral(self, n: dict) -> str:
        v = n["value"]
        t = self.resolve(self.ty(n))
        name = t.name if isinstance(t, ctype.Base) else "int"
        if "unsigned" in name and "long long" in name:
            return f"{v}ULL"
        if "long long" in name:
            return f"{v}LL"
        if "unsigned" in name:
            return f"{v}u"
        if name == "__int128" or name == "unsigned __int128":
            return f"((openrac_s128){v}LL)"
        return v

    def rv_CharacterLiteral(self, n: dict) -> str:
        return str(n["value"])

    def rv_FloatingLiteral(self, n: dict) -> str:
        v = n["value"]
        if not any(ch in v for ch in ".eEn"):
            v += ".0"
        t = self.resolve(self.ty(n))
        return f"{v}f" if isinstance(t, ctype.Base) and t.name == "float" else v

    def rv_StringLiteral(self, n: dict) -> str:
        return f"GSTR({n['value']})"

    def rv_ParenExpr(self, n: dict) -> str:
        if n.get("valueCategory") == "lvalue":
            return self.lv(n).text
        return f"({self.rv(_inner(n)[0])})"

    def rv_ConstantExpr(self, n: dict) -> str:
        return self.rv(_inner(n)[0])

    def rv_ImplicitCastExpr(self, n: dict) -> str:
        return self.cast(n, implicit=True)

    def rv_CStyleCastExpr(self, n: dict) -> str:
        return self.cast(n, implicit=False)

    def cast(self, n: dict, implicit: bool) -> str:
        kind = n["castKind"]
        child = _inner(n)[0]
        t = self.ty(n)
        if kind == "LValueToRValue":
            return self.lv(child).text
        if kind == "ArrayToPointerDecay":
            lv = self.lv(child)
            if lv.addr is None:
                raise Unsupported("an array that is not in game memory")
            return lv.addr
        if kind == "FunctionToPointerDecay":
            return self.function_value(child)
        if kind in ("NoOp",):
            return self.rv(child)
        if kind == "ToVoid":
            return f"(void)({self.rv(child)})"
        if kind == "NullToPointer":
            return "(gaddr)0"
        tc = self.cls(t)
        v = self.rv(child)
        if tc == "ptr":
            if kind in ("BitCast", "IntegralToPointer"):
                return f"(gaddr)({v})" if kind == "IntegralToPointer" else v
            raise Unsupported(f"cast {kind} to a pointer")
        if kind == "PointerToIntegral":
            host = self.host(ctype.strip_quals(t))
            r = self.resolve(t)
            if isinstance(r, ctype.Base) and "long long" in r.name:
                return f"(({host})(int32_t)({v}))"  # the EE sign-extends a 32-bit value
            return f"(({host})({v}))"
        if kind in ("PointerToBoolean", "IntegralToBoolean", "FloatingToBoolean"):
            return f"(({v}) != 0)"
        if kind in ("IntegralCast", "FloatingCast", "IntegralToFloating", "FloatingToIntegral", "BitCast"):
            if tc in ("rec", "arr"):
                raise Unsupported(f"cast {kind} to an aggregate")
            return f"(({self.host(ctype.strip_quals(t))})({v}))"
        raise Unsupported(f"cast {kind}")

    def function_value(self, n: dict) -> str:
        """The code address of a function designator, as a gaddr."""
        n = _strip_parens(n)
        if n.get("kind") == "DeclRefExpr" and n["referencedDecl"].get("kind") == "FunctionDecl":
            sym = self._callee_symbol(n)
            if self.fn is not None:
                self.fn.report.calls.add(sym)
            places = self.program.code_places(sym)
            if not places:
                raise Unsupported(f"the address of {sym}, which has none")
            if getattr(self.program, "relocate", None) and self.fn is not None                     and not self.fn.name.startswith("func_L") and places[0][0] < 0:
                return f"OPENRAC_CODE({hexaddr(places[0][1])})"
            level = self._shared_level()
            copy = self._copy_places()
            if copy is not None and places[0][0] >= 0:
                # One C for copies at several places: the address this copy forms (guest.h).
                home, canon, size = copy
                own = next((a for o, a in places if o == home), places[0][1])
                self.fn.uses_self = True
                return f"openrac_code_in_copy({home}, {hexaddr(canon)}, {size}u, openrac_self_, {hexaddr(own)})"
            if level is not None and places[0][0] >= 0:
                own = next((a for o, a in places if o == level), places[0][1])
                return f"OPENRAC_LCODE({level}, {hexaddr(own)})"
            return hexaddr(places[0][1])
        if n.get("kind") == "UnaryOperator" and n["opcode"] == "*":
            return self.rv(_inner(n)[0])
        raise Unsupported(f"function designator {n.get('kind')}")

    def _callee_symbol(self, ref: dict) -> str:
        d = self.decls.get(ref["referencedDecl"]["id"])
        sym = symbol_of(d) if d is not None else ref["referencedDecl"].get("name", "")
        sym = self.program.aliases.get((self.unit, sym), sym)
        if (self.unit, sym) in self.program.statics:
            return sym
        own = self.program.grouped(self.unit, sym)
        if own != sym and own in self.program.functions:
            return own  # the unit's group (a separate program) has its own
        sym = self.program.canonical(sym)
        f = self.program.functions.get(sym)
        if f is None or (f.unit is None and sym not in self.program.host):
            for g in self.program.groups:
                other = f"{sym}__{g['suffix']}"
                if other in self.program.functions:
                    return other  # only a group defines it
        return sym

    def rv_DeclRefExpr(self, n: dict) -> str:
        ref = n["referencedDecl"]
        kind = ref.get("kind")
        if kind == "EnumConstantDecl":
            return ref["name"]
        if kind == "FunctionDecl":
            return self.function_value(n)
        return self.lv(n).text

    def rv_UnaryOperator(self, n: dict) -> str:
        op = n["opcode"]
        child = _inner(n)[0]
        if op == "&":
            c = _strip_parens(child)
            if c.get("kind") == "DeclRefExpr" and c["referencedDecl"].get("kind") == "FunctionDecl":
                return self.function_value(c)
            lv = self.lv(child)
            if lv.addr is None:
                raise Unsupported("the address of a host local")
            return f"((gaddr)({lv.addr}))"
        if op == "*":
            return self.lv(n).text
        if op in ("++", "--"):
            lv = self.lv(child)
            t = self.ty(child)
            if self.cls(t) == "ptr":
                step = self.pointee_size(t)
                sign = "+" if op == "++" else "-"
                if n.get("isPostfix"):
                    return f"(({lv.text} {sign}= {step}) {'-' if sign == '+' else '+'} {step})"
                return f"({lv.text} {sign}= {step})"
            return f"({lv.text}{op})" if n.get("isPostfix") else f"({op}{lv.text})"
        if op in ("-", "+", "~", "!"):
            return f"({op}{self.rv(child)})"
        if op == "__extension__":
            return self.rv(child)
        raise Unsupported(f"unary {op}")

    def rv_BinaryOperator(self, n: dict) -> str:
        op = n["opcode"]
        a, b = _inner(n)
        if op == "=":
            lv = self.lv(a)
            sa = _strip_parens(a)
            if sa.get("kind") == "DeclRefExpr" and sa["referencedDecl"].get("id") in self.fn.wide:
                return f"({lv.text} = {self.convert(b, ctype.Base('long long'))})"
            return f"({lv.text} = {self.convert(b, self.ty(a))})"
        if op in ("+", "-"):
            ta, tb = self.ty(a), self.ty(b)
            pa, pb = self.cls(ta) == "ptr", self.cls(tb) == "ptr"
            if pa and pb and op == "-":
                return f"((int32_t)((gaddr)({self.rv(a)}) - (gaddr)({self.rv(b)})) / (int32_t){self.pointee_size(ta)})"
            if pa:
                return f"((gaddr)({self.rv(a)}) {op} (gaddr)({self.rv(b)}) * {self.pointee_size(ta)})"
            if pb:
                return f"((gaddr)({self.rv(b)}) + (gaddr)({self.rv(a)}) * {self.pointee_size(tb)})"
        if op == "/" and self.cls(self.ty(n)) == "float":
            return f"openrac_fdiv({self.rv(a)}, {self.rv(b)})"  # the EE's divide (guest.h)
        return f"({self.rv(a)} {op} {self.rv(b)})"

    def rv_CompoundAssignOperator(self, n: dict) -> str:
        op = n["opcode"]
        a, b = _inner(n)
        lv = self.lv(a)
        ta = self.ty(a)
        if self.cls(ta) == "ptr" and op in ("+=", "-="):
            return f"({lv.text} {op} (gaddr)({self.rv(b)}) * {self.pointee_size(ta)})"
        if op == "/=" and self.cls(ta) == "float" and self.cls(self.ty(b)) == "float":
            return f"({lv.text} = openrac_fdiv({lv.text}, {self.rv(b)}))"
        return f"({lv.text} {op} {self.rv(b)})"

    def rv_ConditionalOperator(self, n: dict) -> str:
        c, a, b = _inner(n)
        t = self.ty(n)
        if self.cls(t) == "ptr":
            return f"({self.rv(c)} ? (gaddr)({self.rv(a)}) : (gaddr)({self.rv(b)}))"
        return f"({self.rv(c)} ? {self.rv(a)} : {self.rv(b)})"

    def rv_UnaryExprOrTypeTraitExpr(self, n: dict) -> str:
        name = n.get("name")
        if name not in ("sizeof", "alignof", "__alignof"):
            raise Unsupported(name)
        if "argType" in n:
            t = ctype.parse(n["argType"]["qualType"])
        else:
            t = self.ty(_inner(n)[0])
        word = "sizeof" if name == "sizeof" else "_Alignof"
        return f"((unsigned int){word}({self.host(ctype.strip_quals(t))}))"

    def rv_StmtExpr(self, n: dict) -> str:
        lines = self.compound(_inner(n)[0], 1)
        return "({ " + " ".join(line.strip() for line in lines[1:-1]) + " })"

    def rv_ImplicitValueInitExpr(self, n: dict) -> str:
        return "0"

    def rv_PredefinedExpr(self, n: dict) -> str:
        return self.rv(_inner(n)[0])

    def rv_VAArgExpr(self, n: dict) -> str:
        ap = self.lv(_inner(n)[0]).text
        t = self.ty(n)
        c = self.cls(t)
        host = "gaddr" if c == "ptr" else self.host(ctype.strip_quals(t))
        return f"__builtin_va_arg({ap}, {host})"

    def rv_CallExpr(self, n: dict) -> str:
        return self.call(n)

    # ---- Lvalues ----

    def lv(self, n: dict) -> LV:
        k = n.get("kind")
        if k == "ParenExpr":
            return self.lv(_inner(n)[0])
        if k == "DeclRefExpr":
            return self.lv_decl(n)
        if k == "MemberExpr":
            return self.lv_member(n)
        if k == "ArraySubscriptExpr":
            a, b = _inner(n)
            if self.cls(self.ty(a)) == "ptr":
                p, i, pt = a, b, self.ty(a)
            else:
                p, i, pt = b, a, self.ty(b)
            addr = f"((gaddr)({self.rv(p)}) + (gaddr)({self.rv(i)}) * {self.pointee_size(pt)})"
            self._note_hw(p)
            return LV(self.gref(self.ty(n), addr), addr)
        if k == "UnaryOperator" and n["opcode"] == "*":
            child = _inner(n)[0]
            addr = self.rv(child)
            self._note_hw(child)
            return LV(self.gref(self.ty(n), addr), addr)
        if k == "StringLiteral":
            addr = f"GSTR({n['value']})"
            return LV(self.gref(self.ty(n), addr), addr)
        if k == "CompoundLiteralExpr":
            # An unnamed local: a slot in the frame, filled where it is written.
            fn = self.fn
            if fn is None:
                raise Unsupported("compound literal outside a function")
            t = self.ty(n)
            fname = f"literal_{len(fn.frame)}"
            fn.frame.append((fname, t))
            slot = f"(frame_ + (gaddr)offsetof(struct openrac_frame_{fn.name}, {fname}))"
            store = self._store_init(t, slot, _inner(n)[0])
            addr = f"({{ {store} {slot}; }})"
            return LV(self.gref(t, addr), addr)
        if k in ("ImplicitCastExpr", "CStyleCastExpr") and n.get("castKind") in ("NoOp", "LValueBitCast"):
            inner = self.lv(_inner(n)[0])
            if inner.addr is not None:
                return LV(self.gref(self.ty(n), inner.addr), inner.addr)
            return inner
        raise Unsupported(f"lvalue {k}")

    def _note_hw(self, n: dict) -> None:
        n = _strip_parens(n)
        while n.get("kind") in ("ImplicitCastExpr", "CStyleCastExpr"):
            n = _strip_parens(_inner(n)[0])
        if n.get("kind") == "IntegerLiteral":
            try:
                a = int(n["value"]) & 0xFFFFFFFF
            except ValueError:
                return
            if any(lo <= a < hi for lo, hi in HW_RANGES) and self.fn and self.fn.report:
                self.fn.report.hw += 1

    def lv_decl(self, n: dict) -> LV:
        ref = n["referencedDecl"]
        rid = ref["id"]
        fn = self.fn
        t = self.ty(n)
        if fn is not None and (rid in fn.memory or rid in fn.statics):
            addr = self._mem_addr(rid)
            return LV(self.gref(t, addr), addr)
        if fn is not None and rid in fn.host:
            return LV(fn.host[rid])
        d = self.decls.get(rid, ref)
        sym = symbol_of(d)
        if ref.get("kind") == "FunctionDecl":
            raise Unsupported(f"function {sym} as an lvalue")
        address = self.program.data_address(sym)
        if address is None:
            raise Unsupported(f"global {sym} has no known address")
        addr = hexaddr(address)
        if self._relocated(address):
            addr = f"OPENRAC_DATA({addr})"
        else:
            level = self._shared_level()
            if level is not None and LEVEL_RELOCATE[0] <= address < LEVEL_RELOCATE[1]:
                addr = f"OPENRAC_LDATA({level}, {addr})"
        return LV(self.gref(t, addr), addr)

    def _shared_level(self) -> int | None:
        """For a function several levels' programs carry (func_LNN_..., placed in more than one
        level), the level whose addresses it is written with: its globals and the functions whose
        addresses it takes are looked up for the level loaded (guest.h, level to level)."""
        fn = self.fn
        if fn is None or not getattr(self.program, "relocate", None):
            return None
        cached = getattr(fn, "shared_level", False)
        if cached is not False:
            return cached
        m = re.match(r"func_L(\d\d)_", fn.name)
        level = None
        if m and len({o for o, _ in self.program.code_places(fn.name)}) > 1:
            level = int(m.group(1))
        fn.shared_level = level
        return level

    def _copy_places(self) -> tuple[int, int, int] | None:
        """For a level function at more than one place (folded copies in one program, or copies
        in several levels' programs): the program it is written for, its place there and its
        size; else None."""
        fn = self.fn
        if fn is None or not getattr(self.program, "relocate", None) or not fn.name.startswith("func_L"):
            return None
        places = [(o, a) for o, a in self.program.code_places(fn.name) if o >= 0]
        if len(places) < 2:
            return None
        level = self._shared_level()
        home = level if level is not None else places[0][0]
        canon = next((a for o, a in places if o == home), places[0][1])
        size = getattr(self.program, "sizes", {}).get(fn.name) or 0x400
        return home, canon, size

    def _relocated(self, address: int) -> bool:
        """An executable function's global in the range that level programs move (hostgen.json
        "relocate"): its address is looked up for the level loaded (guest.h)."""
        rel = getattr(self.program, "relocate", None)
        return bool(rel) and self.fn is not None and not self.fn.name.startswith("func_L")             and rel[0] <= address < rel[1]

    def lv_member(self, n: dict) -> LV:
        name = n.get("name", "")
        base = _inner(n)[0]
        obj = self.object_of(base, n.get("isArrow", False))
        if name == "":
            return obj  # an anonymous member: its fields are the object's own
        decl = self.decls.get(n.get("referencedMemberDecl", ""), {})
        bitfield = decl.get("isBitfield", False)
        if obj.addr is None:
            return LV(f"({obj.text}.{name})")
        text = f"((({obj.rec} *)G({obj.addr}))->{name})"
        addr = None if bitfield else f"((gaddr)({obj.addr}) + (gaddr)offsetof({obj.rec}, {name}))"
        return LV(text, addr)

    def object_of(self, base: dict, arrow: bool) -> LV:
        """The record object a member is taken from."""
        b = _strip_parens(base)
        if arrow:
            pt = self.resolve(self.ty(base))
            if not isinstance(pt, ctype.Ptr):
                raise Unsupported("-> on a non-pointer")
            rec = self.host(ctype.strip_quals(pt.to))
            p = self.rv(base)
            self._note_hw(base)
            return LV(self.gref(pt.to, p), p, rec)
        if b.get("kind") == "MemberExpr" and b.get("name") == "":
            return self.object_of(_inner(b)[0], b.get("isArrow", False))
        if b.get("valueCategory") != "lvalue":
            return LV(f"({self.rv(b)})")
        lv = self.lv(b)
        if lv.addr is None:
            return LV(lv.text)
        if lv.rec is not None:
            return lv
        return LV(lv.text, lv.addr, self.host(ctype.strip_quals(self.ty(b))))

    # ---- Calls ----

    def call(self, n: dict) -> str:
        kids = _inner(n)
        callee, args = kids[0], kids[1:]
        result_t = self.ty(n)
        c = _strip_parens(callee)
        if c.get("kind") == "ImplicitCastExpr" and c.get("castKind") == "BuiltinFnToFnPtr":
            return self.builtin_call(_strip_parens(_inner(c)[0]), args)
        if c.get("kind") == "ImplicitCastExpr" and c.get("castKind") == "FunctionToPointerDecay":
            target = _strip_parens(_inner(c)[0])
            if target.get("kind") == "DeclRefExpr" and target["referencedDecl"].get("kind") == "FunctionDecl":
                sym = self._callee_symbol(target)
                places = self._folded_places(sym)
                copy = self._small_folded()
                callee_places = self.program.code_places(sym) if copy is not None and not places else []
                hook = self._level_hook(sym) if not places and not callee_places else None
                if hook is not None:
                    # A call to one of the executable's empty hook stubs, from engine code the level
                    # programs carry: each level fills the hook with its own function (guest.h).
                    exe_fn, size, home, home_fn, stub = hook
                    calls = getattr(self.fn, "hook_calls", None)
                    if calls is None:
                        calls = self.fn.hook_calls = {}
                    k = calls.get(sym, 0)
                    calls[sym] = k + 1
                    self.fn.report.calls.add(sym)
                    fp = f"openrac_hook_in_copy({hexaddr(exe_fn)}, {size}u, {home}, {hexaddr(home_fn)}, {hexaddr(stub)}, {k}u)"
                elif not places and callee_places:
                    # Inside a small function the catalogue folded (copies that differ in what they
                    # call): the function this copy's own call at that place calls (guest.h).
                    home, canon, size = copy
                    own = next((a for o, a in callee_places if o == home), callee_places[0][1])
                    self.fn.report.calls.add(sym)
                    self.fn.uses_self = True
                    fp = f"openrac_call_in_copy({home}, {hexaddr(canon)}, {size}u, openrac_self_, {hexaddr(own)})"
                elif not places:
                    return self.direct_call(sym, args, result_t)
                elif (sym in getattr(self.program, "splits", {})
                      and not getattr(self.program, "sizes", {}).get(self.fn.name)
                      and all(o < 0 for o, _ in self.program.code_places(self.fn.name))):
                    # Code of the executable alone calls the executable's own function.
                    return self.direct_call(sym, args, result_t)
                else:
                    # A name the catalogue folded: each place is its own function (guest.h,
                    # openrac_guest_nearest); the call goes through the place nearest this function,
                    # with the call site's own type.
                    # The k-th call to it here is taken to be this function's k-th call to any of
                    # its places (openrac_folded_call), the nearest place when that fails.
                    self.fn.report.calls.add(sym)
                    literal = ", ".join(f"0x{(o + 1) << 24 | a:08X}u" for o, a in places)
                    calls = getattr(self.fn, "folded_calls", None)
                    if calls is None:
                        calls = self.fn.folded_calls = {}
                    k = calls.get(sym, 0)
                    calls[sym] = k + 1
                    own_size = getattr(self.program, "sizes", {}).get(self.fn.name) or 0
                    fp = (f"openrac_folded_call((const uint32_t[]){{{literal}}}, {len(places)}, "
                          f"{self._own_address()}, {own_size}u, {k})")
            else:
                fp = self.function_value(target)
        else:
            fp = self.rv(callee)
        # A call through a code address: the call site's own type.
        pt = self.resolve(self.ty(callee))
        ft = self.resolve(pt.to) if isinstance(pt, ctype.Ptr) else pt
        if not isinstance(ft, ctype.Func):
            raise Unsupported("a call through something that is not a function pointer")
        hostft = ctype.Func(self._canon_type(ft.ret), tuple(self._canon_type(p) for p in ft.params),
                            ft.variadic, ft.prototyped)
        values = [self.rv(a) for a in args]
        return f"GFN({ctype.fn_pointer(hostft)}, {fp})({', '.join(values)})"

    def _level_hook(self, sym: str):
        """For a call to a hook stub (the game's `level_hooks`) from a function of the executable
        that the levels' programs also carry: that function's address in the executable, its size,
        its first level place, and the stub's address; else None."""
        fn = self.fn
        if fn is None or sym not in getattr(self.program, "level_hooks", ()):
            return None
        size = getattr(self.program, "sizes", {}).get(fn.name)
        if not size or not getattr(self.program, "relocate", None):
            return None
        places = self.program.code_places(fn.name)
        exe = [a for o, a in places if o < 0]
        levels = [(o, a) for o, a in places if o >= 0]
        stub = [a for o, a in self.program.code_places(sym) if o < 0]
        if not exe or not levels or not stub:
            return None
        return exe[0], size, levels[0][0], levels[0][1], stub[0]

    def _small_folded(self) -> tuple[int, int, int] | None:
        """For a small function (at most 64 bytes: a wrapper of a call or two) the catalogue folded
        (two or more places in one program, whose calls may go to different functions): the
        program of its first place, that place and its size; else None."""
        fn = self.fn
        if fn is None or not getattr(self.program, "relocate", None):
            return None
        size = getattr(self.program, "sizes", {}).get(fn.name)
        if not size or size > 64:
            return None
        places = self.program.code_places(fn.name)
        overlays = [o for o, _ in places]
        if len(overlays) == len(set(overlays)):
            return None
        home = places[0][0]
        return home, places[0][1], size

    def _folded_places(self, sym: str) -> list[tuple[int, int]]:
        """The places of a wrapper the catalogue folded, or []: a function of at most 64 bytes (a
        frame and a call or two) at two or more addresses of one program, each calling something
        else. Folded copies of a whole function are the same code and need nothing."""
        cache = getattr(self.program, "folded_cache", None)
        if cache is None:
            cache = self.program.folded_cache = {}
        if sym not in cache and getattr(self.program, "splits", {}).get(sym):
            # Some of its places are other functions (split_places.tsv): the caller's own jal says
            # which one it calls.
            cache[sym] = self.program.code_places(sym) + self.program.splits[sym]
        if sym not in cache:
            wrapper = 0 < (getattr(self.program, "sizes", {}).get(sym) or 0) <= 64
            places = self.program.code_places(sym) if wrapper and getattr(self.program, "relocate", None) else []
            overlays = [o for o, _ in places]
            cache[sym] = places if len(overlays) != len(set(overlays)) else []
        return cache[sym]

    def _own_address(self) -> str:
        """This function's address in the program loaded when it runs."""
        places = self.program.code_places(self.fn.name)
        if not places:
            return "0u"
        level = self._shared_level()
        if level is not None:
            own = next((a for o, a in places if o == level), places[0][1])
            return f"OPENRAC_LCODE({level}, {hexaddr(own)})"
        return hexaddr(places[0][1])

    def _canon_type(self, t: ctype.Type) -> ctype.Type:
        c = self.canon(t)
        if c is None:
            return t
        return ctype.Base(c)

    def builtin_call(self, ref: dict, args: list) -> str:
        name = ref["referencedDecl"]["name"] if "referencedDecl" in ref else ref.get("name", "")
        v = [self.rv(a) for a in args]
        if name == "__builtin_va_start":
            return f"__builtin_va_start({self.lv(args[0]).text}, {v[1]})"
        if name == "__builtin_va_end":
            return f"__builtin_va_end({self.lv(args[0]).text})"
        if name == "__builtin_va_copy":
            return f"__builtin_va_copy({self.lv(args[0]).text}, {self.lv(args[1]).text})"
        if name in ("__builtin_memcpy", "__builtin_memmove"):
            return f"((gaddr)({v[0]}), {name[10:]}(G({v[0]}), G({v[1]}), {v[2]}), (gaddr)({v[0]}))"
        if name == "__builtin_memset":
            return f"((gaddr)({v[0]}), memset(G({v[0]}), {v[1]}, {v[2]}), (gaddr)({v[0]}))"
        if name in ("__builtin_fabsf", "__builtin_sqrtf", "__builtin_fabs", "__builtin_sqrt", "__builtin_abs"):
            return f"{name}({', '.join(v)})"
        raise Unsupported(f"builtin {name}")

    @staticmethod
    def _reg_class(t_cls: str) -> str:
        return "f" if t_cls == "float" else "i"

    def direct_call(self, sym: str, args: list, result_t: ctype.Type) -> str:
        if sym in BUILTINS:
            return f"{BUILTINS[sym]}({', '.join(self.rv(a) for a in args)})"
        if sym == HOSTGEN_ASM:
            raise Unsupported("inline assembly Clang cannot read")
        f = self.program.lookup(self.unit, sym)
        if sym in ("memcpy", "memmove", "memset") and (f is None or (f.unit is None and sym not in self.program.host)):
            # The C library's, which the game declares by name without defining it (its own copies
            # sit at other addresses): on game memory, as the builtins are.
            v = [self.rv(a) for a in args]
            if sym == "memset":
                return f"((gaddr)({v[0]}), memset(G({v[0]}), {v[1]}, {v[2]}), (gaddr)({v[0]}))"
            return f"((gaddr)({v[0]}), {sym}(G({v[0]}), G({v[1]}), {v[2]}), (gaddr)({v[0]}))"
        if sym in HOST_MATH and (f is None or (f.unit is None and sym not in self.program.host)):
            # A C library maths function the game calls without defining it (declared or
            # not): the host's, on values.
            return f"{sym}({', '.join(self.rv(a) for a in args)})"
        if f is None:
            raise Unsupported(f"call to {sym}, which the program does not know")
        self.fn.report.calls.add(sym)
        sig = f.sig
        result_cls = self.cls(result_t)
        values = self._match_args(sym, sig, args)
        text = f"{cname(sym)}({', '.join(values)})"
        ret_cls = self.cls(sig.ret)
        if result_cls == "void":
            return text
        if ret_cls == "void" or self._reg_class(ret_cls) != self._reg_class(result_cls) or ret_cls == "rec" != result_cls:
            # On the console the caller read a register the callee did not set.
            return f"({text}, ({self.host(ctype.strip_quals(result_t))})0)"
        if result_cls == "ptr":
            return f"(gaddr)({text})"
        if result_cls == "rec":
            return text
        if sym in getattr(self.program, "returns_64", ()) and result_cls == "int":
            return f"((long long)({text}))"  # all 64 bits, as the register holds them
        return f"(({self.host(ctype.strip_quals(result_t))})({text}))"

    def _arg_class(self, a: dict) -> str:
        """Which register file the call site put the argument in."""
        t = self.ty(a)
        c = self.cls(t)
        if c == "double" and a.get("kind") == "ImplicitCastExpr" and a.get("castKind") == "FloatingCast":
            inner_t = self.ty(_inner(a)[0])
            if self.cls(inner_t) == "float":
                return "f"  # a float promoted by an unprototyped call: kept a float
        if c == "rec":
            return "agg"
        return self._reg_class(c)

    def _match_args(self, sym: str, sig, args: list) -> list[str]:
        if not sig.prototyped:
            return [self.rv(a) for a in args]
        classes = [self._arg_class(a) for a in args]
        pcls = [("agg" if self.cls(p) == "rec" else self._reg_class(self.cls(p))) for p in sig.params]
        if "agg" in classes or "agg" in pcls:
            if len(args) != len(sig.params):
                raise Unsupported(f"call to {sym} with records, and {len(args)} arguments for {len(sig.params)}")
            out = []
            for a, c, p, pc in zip(args, classes, sig.params, pcls):
                if c == "agg" and pc != "agg":
                    # A record where the callee takes a scalar: the EE passed the
                    # record's first eight bytes in the register.
                    canon_p = self.canon(p) or "int"
                    out.append(f"({{ {self.declare(self.ty(a), 'rec_')} = {self.rv(a)}; uint64_t bits_ = 0; "
                               f"memcpy(&bits_, &rec_, sizeof rec_ < 8 ? sizeof rec_ : 8); ({canon_p})bits_; }})")
                elif c != "agg" and pc == "agg":
                    raise Unsupported(f"call to {sym}: a scalar where it takes a record")
                else:
                    out.append(self._as_param(a, p))
            return out
        ints = [a for a, c in zip(args, classes) if c == "i"]
        floats = [a for a, c in zip(args, classes) if c == "f"]
        out = []
        ni = nf = 0
        for p, pc in zip(sig.params, pcls):
            if pc == "f":
                src, k, regs = floats, nf, self.fn.float_params
                nf += 1
            else:
                src, k, regs = ints, ni, self.fn.int_params
                ni += 1
            if k < len(src):
                out.append(self._as_param(src[k], p))
            elif k < len(regs):
                # The callee reads a register the call did not set: on the
                # console it still held the caller's own argument.
                self.fn.report.passthrough += 1
                if regs[k] in self.fn.record_params:
                    # The caller's own argument was a record: as for a record passed
                    # where a scalar is taken, the register held its first eight bytes.
                    canon_p = self.canon(p) or "int"
                    out.append(f"({{ uint64_t bits_ = 0; memcpy(&bits_, &{regs[k]}, sizeof {regs[k]} < 8 ? "
                               f"sizeof {regs[k]} : 8); ({canon_p})bits_; }})")
                else:
                    out.append(self._cast_to(regs[k], p))
            else:
                out.append(self._cast_to("0", p))
        if sig.variadic:
            extra = {id(a) for a in ints[ni:] + floats[nf:]}
            out += [self.rv(a) for a in args if id(a) in extra]
        return out

    def _as_param(self, a: dict, p: ctype.Type) -> str:
        c = self.cls(p)
        if (c == "valist") != (self.cls(self.ty(a)) == "valist"):
            # On the console a va_list is a pointer into the caller's argument area,
            # so C passes one where the other is declared; on the host it is the
            # compiler's own type, which a game address cannot stand for.
            raise Unsupported("a va_list passed as a game pointer, or the other way round")
        if c in ("rec", "valist"):
            return self.rv(a)
        return self._cast_to(self.rv(a), p)

    def _cast_to(self, v: str, p: ctype.Type) -> str:
        canon = self.canon(p)
        if canon is None:
            return v
        return f"({canon})({v})"
