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
- string literals are copied into game memory once (GSTR);
- in the functions hostgen.json "ps2_float" names, float + - * /, the
  comparisons, a float's truth and the conversions between int and float
  are the EE FPU's own arithmetic, computed in software (openrac/ps2float.h:
  results cut, no infinities, NaNs or denormals), where the host's
  arithmetic gives a result the game tells apart.

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
# The EE's float operations in software (openrac/ps2float.h), by C operator: the functions
# hostgen.json "ps2_float" names (Unit._ps2).
PS2_ARITH = {"+": "openrac_ps2_add", "-": "openrac_ps2_sub", "*": "openrac_ps2_mul", "/": "openrac_ps2_div"}
HW_RANGES = [(0x10000000, 0x12002000)]  # EE registers, VU memory, GS registers
# Level programs' addresses (rac1: 0x15F000 up, with their data): what a shared level function's
# globals are relocated in (_shared_level).
LEVEL_RELOCATE = (0x0015F000, 0x00400000)
# A level function's name, with its level: rac1/pal's func_L05_..., the NTSC decomp's FUN_L05_....
LEVEL_NAME = re.compile(r"(?:func|FUN)_L(\d\d)_")


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
    zero_args: list = field(default_factory=list)  # calls short of arguments the caller had none for
    hw: int = 0                  # accesses to hardware register addresses
    # Literal addresses cast to pointers (`*(s32 *)0x1610A8`) and what each became: "0x... -> C".
    literals: list = field(default_factory=list)
    calls: set = field(default_factory=set)  # functions it calls or takes the address of
    # Direct calls whose call site disagrees with the callee's definition (arity, kinds, result):
    # "callee: what differs", for port/tools/hostgen/priority.py.
    mismatches: list = field(default_factory=list)
    # Instructions of its asm statements that the host does not run (asm_stmt): what they compute is
    # missing from the translation.
    asm_skipped: list = field(default_factory=list)


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
    vla: dict = field(default_factory=dict)          # decl id -> guest address of a variable-length array
    host: dict = field(default_factory=dict)         # decl id -> host name
    frame: list = field(default_factory=list)        # (field, type)
    report: FnReport | None = None
    tail_id: str | None = None                       # the last statement, a call whose result is returned
    labels: dict = field(default_factory=dict)       # label decl id -> name
    int_params: list = field(default_factory=list)   # host names, in register order
    record_params: set = field(default_factory=set)  # host names of parameters passed as records
    float_params: list = field(default_factory=list)
    wide: set = field(default_factory=set)           # locals kept 64-bit (returns_64)
    wide_params: set = field(default_factory=set)    # parameters only passed on to 64-bit ones (widen_forwarded)
    wide_result: bool = False                        # returns a "returns_64" call's result whole (widen_returns)
    sibling_named: set = field(default_factory=set)  # folded functions the body also names by another place's name


def _inner(n: dict) -> list:
    return n.get("inner", [])


def _strip_parens(n: dict) -> dict:
    while n.get("kind") == "ParenExpr":
        n = _inner(n)[0]
    return n


def _in(address: int, rng) -> bool:
    return bool(rng) and rng[0] <= address < rng[1]


def _constant(n: dict) -> int | None:
    """The value of an integer constant expression of literals (`0x1610A8`, `(u32)0x160F00 + 8`), or
    None."""
    n = _strip_parens(n)
    k = n.get("kind")
    if k == "IntegerLiteral":
        try:
            return int(n["value"], 0)
        except ValueError:
            return None
    if k == "ConstantExpr" or (k in ("ImplicitCastExpr", "CStyleCastExpr")
                               and n.get("castKind") in ("IntegralCast", "NoOp")):
        return _constant(_inner(n)[0])
    if k == "BinaryOperator" and n.get("opcode") in ("+", "-", "|"):
        a, b = (_constant(c) for c in _inner(n)[:2])
        if a is None or b is None:
            return None
        return a + b if n["opcode"] == "+" else a - b if n["opcode"] == "-" else a | b
    return None


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
        self.enum_consts: dict[str, str | None] = {}  # enumerators written so far -> value text
        self.const_names: dict[str, str] = {}  # a function's enumerators renamed apart from another's
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

    def sig_cls(self, t: ctype.Type) -> str:
        """cls of a type from another unit's signature (a callee's parameter or result): a typedef
        this unit does not declare (the callee's file has `f32`, the caller's only `float`) is
        read through the whole program's typedefs, so a float stays a float and a pointer a
        pointer, as the registers the console used for them."""
        c = self.cls(t)
        r = self.resolve(t)
        if c == "int" and isinstance(r, ctype.Base) and r.name not in self.typedefs and r.name not in ctype.INT_NAMES:
            k = canon(r, self.program.typedefs)
            if k in ctype.FLOAT_NAMES:
                return "float"
            if k == "gaddr":
                return "ptr"
            if k in ("double", "long double"):
                return "double"
        return c

    def host(self, t: ctype.Type) -> str:
        if isinstance(t, ctype.Base) and t.name in VA_NAMES:
            return "va_list"
        return ctype.host(self._rename_type(t))

    def _frame_slot(self, t: ctype.Type, name: str) -> str:
        """A frame field for a local, in the slot the console's compiler gave it. An array or
        record with no machine mode of its own (a `float v[3]`, a `float m[4][4]`, a 12-byte
        struct) gets a slot aligned to 16 and rounded up to 16 bytes (OPENRAC_FRAME_SIZE and
        OPENRAC_FRAME_ALIGN, in openrac/guest.h): the game copies such locals with quadword
        loads and stores (lq/sq, which the host makes movaps), and a 16-byte copy into a
        `float v[3]` puts its fourth word in the slot's padding there, not in the next local."""
        decl = self.declare(t, name)
        r = self.resolve(t)
        aggregate = isinstance(r, ctype.Arr) or (
            isinstance(r, ctype.Base) and r.name.split(" ")[0] in ("struct", "union"))
        if not aggregate or (isinstance(r, ctype.Base) and r.name in VA_NAMES):
            return f"    {decl};"
        tn = self.host(t)
        return (f"    union {{ {decl}; char {name}_slot_[OPENRAC_FRAME_SIZE({tn})]; }}"
                f" __attribute__((aligned(OPENRAC_FRAME_ALIGN({tn}))));")

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
        if getattr(self, "uses_ps2", False):
            out.insert(out.index("#include \"openrac/guest.h\"") + 1, "#include \"openrac/ps2float.h\"")
        return "\n".join(out + [""] + statics_protos + [""] + bodies) + "\n"

    def _mine(self, decl: dict) -> bool:
        """Definitions this unit translates: its own, and static ones from headers.
        A function the port writes itself (a library it replaces) is left out."""
        sym = self._defined_symbol(decl)
        if sym in BUILTINS:
            return False
        if sym in self.program.host and decl.get("storageClass") != "static":
            return False
        if sym in self.program.from_assembly and decl.get("storageClass") != "static":
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
            name = c["name"]
            if name in self.enum_consts:
                # Hoisted from several functions (each declared `enum { X = 7 };` locally): one
                # file-scope constant when they agree, the function's own name when they differ.
                if self.enum_consts[name] == value and value is not None:
                    continue
                if self.fn is not None:
                    self.const_names[name] = f"{cname(self.fn.name)}__{name}"
                    name = self.const_names[name]
            self.enum_consts.setdefault(name, value)
            items.append(f"    {name}" + (f" = {value}" if value is not None else "") + ",")
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
        # typedef ... T __attribute__((aligned(16))): objects of T (a vector the game copies with
        # lq/sq, which the host makes movaps) keep the alignment the console gave them.
        aligned = [f"aligned({self._attr_value(c)})" for c in _inner(decl) if c.get("kind") == "AlignedAttr"]
        attr = f" __attribute__(({', '.join(aligned)}))" if aligned else ""
        for c in _inner(decl):
            owned = c.get("ownedTagDecl")
            if owned and not owned.get("name") and isinstance(t, ctype.Base):
                word = t.name.split(" ", 1)[0]
                return [f"typedef {word} {self._renamed('openrac_td_' + name)} {self._renamed(name)}{attr};"]
        return [f"typedef {self.declare(t, self._renamed(name))}{attr};"]

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
        widened = dict(getattr(known, "widened", None) or {}) if known is not None and not static else {}
        if widened:
            # Parameters the C only passes on to 64-bit ones (program.py widen_forwarded): 64-bit here, as the
            # EE's register reaches the callee with all its bits.
            sig_t = ctype.Func(sig_t.ret, tuple(ctype.Base(widened[i]) if i in widened else t
                                               for i, t in enumerate(sig_t.params)), sig_t.variadic, sig_t.prototyped)
        inherits = known is not None and getattr(known, "inherits", False) and self.cls(sig_t.ret) == "void"
        if inherits:
            sig_t = ctype.Func(known.sig.ret, sig_t.params, sig_t.variadic, sig_t.prototyped)
        # A 32-bit result that is a "returns_64" call's result returned as it is: 64 bits (program.py widen_returns).
        wide_result = known is not None and not static and getattr(known, "wide_result", False)
        if wide_result:
            sig_t = ctype.Func(known.sig.ret, sig_t.params, sig_t.variadic, sig_t.prototyped)
        proto = self.signature(sym, sig_t, names, static)
        report = FnReport(sym, "translated")
        self.fn = _Fn(sym, sig_t, params, report=report)
        self.fn.wide_params = {params[i].get("id") for i in widened if i < len(params)}
        self.fn.wide_result = wide_result
        body = next(c for c in _inner(decl) if c.get("kind") == "CompoundStmt")
        self.fn.sibling_named = self._sibling_named(body)
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
                   f"    static openrac_reach_site site_ = {{\"{sym}\", \"untranslated\", 0, 0, 0}};\n"
                   f"    openrac_guest_reach(&site_);\n   {ret}\n}}\n")
        self.report.functions.append(report)
        self.fn = None
        self.local_names = {}
        self.const_names = {}
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
                if d.get("storageClass") == "extern":
                    continue
                if d.get("storageClass") == "static":
                    fn.statics[vid] = f"{d.get('name')}_at_"
                elif self._vla_length(d) is not None:
                    fn.vla[vid] = f"{d.get('name', 'v')}_vla{len(fn.vla)}"
                    fn.memory[vid] = None
                else:
                    fn.memory[vid] = self._frame_field(d, d.get("name", "v"))
        fn.wide = self._wide_locals(body)
        lines = self.compound(body, 1, top=True)
        frame_decl = []
        if fn.frame:
            fields = "\n".join(self._frame_slot(t, f) for f, t in fn.frame)
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
        if getattr(fn, "uses_alloca", False):
            # alloca's space is the frame's: given back when the function returns, by any path.
            prologue.insert(0, "    gaddr openrac_alloca_sp_ __attribute__((cleanup(openrac_guest_frame_end))) = "
                               "openrac_guest_sp;")
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

    def _vla_length(self, d: dict) -> str | None:
        """The length of a variable-length array whose length is a local or parameter by name
        (`struct Moby *found[count]`), and whose elements have a fixed size (`float v[count][4]`),
        else None."""
        r = self.resolve(self.ty(d))
        if not isinstance(r, ctype.Arr) or not r.vla:
            return None
        inner = self.resolve(r.of)
        while isinstance(inner, ctype.Arr):
            if inner.size is None:
                return None  # a variable length inside, or an unknown one
            inner = self.resolve(inner.of)
        return r.vla.strip() if re.fullmatch(r"[A-Za-z_]\w*", r.vla.strip()) else None

    def _vla_decl(self, d: dict) -> str:
        """Space for a variable-length array on the guest stack, given back when its block ends,
        as the console's frame gives it back."""
        fn = self.fn
        length = self._vla_length(d)
        ids = [i for i, h in fn.host.items() if self.decls.get(i, {}).get("name") == length]
        ids += [i for i in fn.memory if fn.memory[i] is not None and self.decls.get(i, {}).get("name") == length]
        if len(set(ids)) != 1:
            raise Unsupported(f"variable-length array {d.get('name')}")
        vid = ids[0]
        n = self._mem_text(vid) if vid in fn.memory else fn.host[vid]
        of = self.resolve(self.ty(d)).of
        return f"GFRAME({fn.vla[d['id']]}, (uint32_t)({n}) * (uint32_t)sizeof({self.host(of)}));"

    def _mem_text(self, vid: str) -> str:
        d = self.decls[vid]
        return self.gref(self.ty(d), self._mem_addr(vid))

    def _mem_addr(self, vid: str) -> str:
        fn = self.fn
        if vid in fn.statics:
            return fn.statics[vid]
        if vid in fn.vla:
            return fn.vla[vid]
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
                if n.get("storageClass") != "extern":
                    local_ids.add(n["id"])
                    if self.cls(self.ty(n)) in ("rec", "arr"):
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
                    if ref.get("kind") in ("VarDecl", "ParmVarDecl") and ref.get("storageClass") != "extern":
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
                # A bare return in a function with a result (old GCC takes `return;` in an int
                # function, the matching C keeps it, today's Clang refuses it), or in a definition
                # that returns its last call's result: the register is left as it was, nothing a
                # caller could use. Zero, for a scalar or a pointer.
                ret = self.cls(self.fn.sig.ret)
                if self.fn.tail_id is not None or ret in ("int", "ptr", "float", "double"):
                    return [pad + "return 0;"]
                return [pad + "return;"]
            if self.cls(self.fn.sig.ret) == "void":
                return [pad + f"{self.rv(kids[0])};", pad + "return;"]
            value = (self._wide_value(kids[0], self.fn.sig.ret) if self.fn.wide_result
                     else self.convert(kids[0], self.fn.sig.ret))
            return [pad + f"return {value};"]
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
        """An asm statement (its instructions from clangast.asm_strings): an ordering barrier or a
        nop (sync, sync.l, sync.p, nop: the console's scheduling) is GBARRIER. Other instructions
        (the VU0's, COP0's, a sqrt.s) are code the host does not run: the statement stays a barrier,
        as before, and the function's report lists what it skips (report.json
        "inline_assembly_skipped"), since what those instructions compute is missing from the
        translation. One whose text hostgen could not read (a header's, a macro's) is a barrier."""
        text = n.get("asmString")
        if text is None:
            return "GBARRIER();"
        words = text.replace(";", "\n").split("\n")
        ops = [w.strip().split()[0] for w in words if w.strip()]
        cons = n.get("asmConstraints") or {}
        if not ops and cons.get("out"):
            # No instructions, only operands: each output gets the input tied to it ("0" for output 0), the
            # value the register holds (kputchar's pointer to the SIO's transmit register, hidden this way from
            # the optimizer). An output tied to none keeps whatever it held, as before, and is reported.
            kids = _inner(n)
            outs, ins = kids[:len(cons["out"])], kids[len(cons["out"]):]
            stmts = ["GBARRIER();"]
            for k, (c, o) in enumerate(zip(cons["out"], outs)):
                if c.startswith("+"):
                    continue
                tied = [i for i, ic in enumerate(cons["in"]) if ic.strip() == str(k)]
                if not tied or tied[0] >= len(ins):
                    self._asm_skipped([f"(output {k} without an input)"])
                    continue
                stmts.append(f"({self.lv(o).text} = {self.convert(ins[tied[0]], self.ty(o))});")
            return " ".join(stmts)
        if not all(op in ("sync", "sync.l", "sync.p", "nop") for op in ops):
            self._asm_skipped(ops)
        return "GBARRIER();"

    def _asm_skipped(self, what: list[str]) -> None:
        """Notes instructions (or operands) of an asm statement the host does not run in the report of
        the function being translated."""
        if self.fn is not None and self.fn.report is not None:
            skipped = self.fn.report.asm_skipped
            skipped.extend(w for w in dict.fromkeys(what) if w not in skipped)

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

    def _wide_value(self, n: dict, t: ctype.Type) -> str:
        """N as the 64-bit value T that the EE's register holds: a "returns_64" call's result, a local kept 64-bit
        or a widened parameter all their bits; an unsigned int or a pointer sign-extended from 32 bits, as every
        32-bit load and operation leaves it (lui/ori, addiu, lw, addu), where C's conversion would zero-extend it: an
        unsigned wide local's other values (RibbonTrailDraw's `color = (color & 0xFFFFFF) + (a << 24)` is
        sign-extended in the register, where FastTweenColor's colour is zero-extended), a wide result's other return
        values. Anything else converts as C converts it (a signed int: sign-extended already)."""
        whole = self._wide_name(n)
        if whole is not None:
            return f"({self.host(ctype.strip_quals(t))})({whole})"
        if self._sign_extends(n):
            return f"({self.host(ctype.strip_quals(t))})(int32_t)({self.rv(n)})"
        return self.convert(n, t)

    def _sign_extends(self, n: dict) -> bool:
        """N is an unsigned int or a pointer (not a "returns_64" call's result) that the EE keeps sign-extended in a
        register, where C's conversion to 64 bits would zero-extend it (_wide_value). A signed or narrower value
        converts as the register holds it already."""
        if self._wide_call(n) or self._wide_name(n) is not None:
            return False
        if self.cls(self.ty(n)) == "ptr":
            return True
        r = self.resolve(self.ty(n))
        return isinstance(r, ctype.Base) and r.name in ("unsigned int", "unsigned")

    def _wide_name(self, n: dict) -> str | None:
        """The host name of the local kept 64-bit (returns_64) or the widened parameter (widen_forwarded) that N reads
        as it is: through parentheses, reads, NoOp casts and conversions between 32-bit integers, which are no
        instruction on the EE. None for anything else."""
        if self.fn is None or not (self.fn.wide or self.fn.wide_params):
            return None
        while _inner(n) and (n.get("kind") == "ParenExpr" or (n.get("kind") in ("ImplicitCastExpr", "CStyleCastExpr")
                                                               and (n.get("castKind") in ("LValueToRValue", "NoOp")
                                                                    or (n.get("castKind") == "IntegralCast"
                                                                        and self._is_int32(self.ty(n)))))):
            n = _inner(n)[0]
        ref = n.get("referencedDecl", {}) if n.get("kind") == "DeclRefExpr" else {}
        if ref.get("id") in self.fn.wide or ref.get("id") in self.fn.wide_params:
            return self.fn.host.get(ref.get("id"))
        return None

    def _is_int32(self, t: ctype.Type) -> bool:
        r = self.resolve(t)
        return isinstance(r, ctype.Base) and r.name in ("int", "signed int", "unsigned int", "unsigned")

    def _wide_locals(self, body: dict) -> set:
        wide = set()
        returns = getattr(self.program, "returns_64", ())
        if not returns:
            return wide
        # Locals the original keeps in a stack slot across calls (hostgen.json "spilled_locals"): GCC stores the
        # 32-bit value it believes the result is (sw) and loads it back sign-extended (lw), so the 64 bits are gone.
        spilled = set(getattr(self.program, "spilled_locals", {}).get(self.fn.name, ()))
        stack = [body]
        while stack:
            n = stack.pop()
            k = n.get("kind")
            if k == "VarDecl" and n.get("init") and n["id"] not in self.fn.memory and n.get("storageClass") not in ("static", "extern"):
                init = [c for c in _inner(n) if c.get("kind") not in ("AlignedAttr", "AsmLabelAttr", "SectionAttr")]
                if init and self._is_int32(self.ty(n)) and self._wide_call(init[0]) and n.get("name") not in spilled:
                    wide.add(n["id"])
            elif k == "BinaryOperator" and n.get("opcode") == "=":
                a, b = _inner(n)
                a = _strip_parens(a)
                if a.get("kind") == "DeclRefExpr" and a["referencedDecl"].get("kind") == "VarDecl":
                    vid = a["referencedDecl"]["id"]
                    if (vid not in self.fn.memory and self._is_int32(self.ty(a)) and self._wide_call(b)
                            and a["referencedDecl"].get("name") not in spilled):
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
        if d["id"] in fn.vla:
            return [pad + self._vla_decl(d)]
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
            if init is not None and (self._sign_extends(init) or self._wide_name(init) is not None):
                return [pad + f"{self.declare(t, name)} = {self._wide_value(init, t)};"]
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
            wide = self._wide_param(child)
            if wide is not None:
                # A parameter made 64-bit for what it passes on (program.py widen_forwarded), read as its own C type
                # where the C computes with it: the EE's 32-bit arithmetic and shifts read the register's low half
                # (DrawFramedText's `(s32)color >> 24`, a sra).
                return f"(({self.host(ctype.strip_quals(self.ty(child)))})({wide}))"
            return self.lv(child).text
        if kind == "IntegralCast" and self._wide(t):
            wide = self._wide_param(child)
            if wide is not None:
                # ... and whole where the C converts it to a 64-bit integer, which is no instruction on the EE
                # (DrawFramedText's `(s64)color & ...`, an and of the register as the caller left it).
                return f"(({self.host(ctype.strip_quals(t))})({wide}))"
        if kind == "ArrayToPointerDecay":
            if self._code_variable(child):
                return self.function_value(child)
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
            if kind == "IntegralToPointer" and (lit := self._literal_pointer(child, t)) is not None:
                return lit
            if kind in ("BitCast", "IntegralToPointer"):
                return f"(gaddr)({v})" if kind == "IntegralToPointer" else v
            raise Unsupported(f"cast {kind} to a pointer")
        if kind == "PointerToIntegral":
            host = self.host(ctype.strip_quals(t))
            r = self.resolve(t)
            if isinstance(r, ctype.Base) and "long long" in r.name:
                return f"(({host})(int32_t)({v}))"  # the EE sign-extends a 32-bit value
            return f"(({host})({v}))"
        if kind == "FloatingToBoolean" and self.cls(self.ty(child)) == "float" and self._ps2():
            return f"(!openrac_ps2_eq({v}, 0.0f))"  # every zero is zero (no denormals)
        if kind == "IntegralToFloating" and tc == "float" and self._ps2_int_source(self.ty(child)) and self._ps2():
            return f"openrac_ps2_itof((int32_t)({v}))"  # cvt.s.w: cut to 24 bits
        if kind in ("PointerToBoolean", "IntegralToBoolean", "FloatingToBoolean"):
            return f"(({v}) != 0)"
        if (kind == "FloatingToIntegral" and tc == "int" and self.cls(self.ty(child)) == "float"
                and self._ps2_int_source(t) and self._ps2()):
            # cvt.w.s: saturating by the sign, also for an exponent of 255 (a NaN to the host).
            return f"(({self.host(ctype.strip_quals(t))})openrac_ps2_ftoi({v}))"
        if kind == "FloatingToIntegral" and tc == "int":
            # The EE's own conversion (guest.h): C leaves a value out of the target's range
            # undefined, and the hosts differ there (x86 wraps a negative float into an unsigned
            # int, ARM gives 0), so a HUD's alpha or offset computed below zero came out different
            # on a phone.
            host = self.host(ctype.strip_quals(t))
            r = self.resolve(t)
            name = r.name if isinstance(r, ctype.Base) else ""
            if "long long" in name:
                return f"(({host})({v}))"
            if name.startswith("unsigned") and "char" not in name and "short" not in name:
                return f"(({host})openrac_f2u({v}))"
            return f"(({host})openrac_f2i({v}))"
        if kind in ("IntegralCast", "FloatingCast", "IntegralToFloating", "FloatingToIntegral", "BitCast"):
            if tc in ("rec", "arr"):
                raise Unsupported(f"cast {kind} to an aggregate")
            return f"(({self.host(ctype.strip_quals(t))})({v}))"
        raise Unsupported(f"cast {kind}")

    def _literal_pointer(self, child: dict, t) -> str | None:
        """A constant address made a pointer (`*(s32 *)0x1610A8`, `((S *)0x160F08)->f`): lowered as
        the global a name at that address would be (global_address), so in a range the levels move
        it is the loaded level's, not the executable's copy under it. A function pointer is code:
        OPENRAC_CODE in an executable function, OPENRAC_LCODE in one several levels carry. None
        when it is not a constant, or nothing relocates it (the plain address is right)."""
        address = _constant(child)
        if address is None or self.fn is None or not getattr(self.program, "relocate", None):
            return None
        address &= 0xFFFFFFFF
        r = self.resolve(t)
        pointee = self.resolve(ctype.strip_quals(r.to)) if isinstance(r, ctype.Ptr) else None
        if isinstance(pointee, ctype.Func):
            if self._relocated(address):
                text = f"OPENRAC_CODE({hexaddr(address)})"
            elif (level := self._shared_level()) is not None and _in(address, self.program.relocate):
                text = f"OPENRAC_LCODE({level}, {hexaddr(address)})"
            else:
                text = None
        else:
            text = self.global_address(address, self._exe_code_at(address))
            if text == hexaddr(address):
                text = None
        if self.fn.report is not None and (text is not None or _in(address, self.program.relocate)):
            # In the range but kept: a function of one level, written with that level's addresses.
            self.fn.report.literals.append(f"{address:#08x} -> {text or 'kept'}")
        return text

    def _exe_code_at(self, address: int) -> bool:
        """An executable function starts at `address` (its code read as data through a literal)."""
        if self.fn is None or LEVEL_NAME.match(self.fn.name):
            return False
        places = getattr(self.program, "by_place", {})
        return (-1, address) in places

    def function_value(self, n: dict) -> str:
        """The code address of a function designator, as a gaddr."""
        n = _strip_parens(n)
        if n.get("kind") == "DeclRefExpr" and (n["referencedDecl"].get("kind") == "FunctionDecl"
                                               or self._code_variable(n)):
            sym = self._callee_symbol(n)
            named = self._folded_place_name(n, sym)
            address = self.code_address(named)
            # A body that also names another place of the fold by that place's own name tells the
            # places apart itself: the folded name is its own first place, which the code forms
            # there (code_address). RegisterPartTypes stores 0x26AB10 and its copy 0x26B2B8
            # (func_L00_0026B2B8) in entries 5 and 7, and its code forms 0x26B2B8 first: the k-th
            # formation of any place put 0x26B2B8 in entry 5.
            sibling = self.fn is not None and sym in self.fn.sibling_named
            places = self._folded_places(sym) if named == sym and self.fn is not None and not sibling else []
            own_size = getattr(self.program, "sizes", {}).get(self.fn.name) or 0 if places else 0
            if places and own_size:
                # A name the catalogue folded, whose places in one program are other functions (each
                # wrapper calls its own; split_places.tsv): the place the caller's own code forms,
                # its k-th address of any of them (openrac_folded_address), this one when it forms
                # none. The first place in the level was taken before: Umbris's sea classes
                # registered a wrapper of another routine as their draw callback, and the sea was
                # never drawn.
                literal = ", ".join(f"0x{(o + 1) << 24 | a:08X}u" for o, a in places)
                taken = getattr(self.fn, "folded_addresses", None)
                if taken is None:
                    taken = self.fn.folded_addresses = {}
                k = taken.get(sym, 0)
                taken[sym] = k + 1
                return (f"openrac_folded_address((const uint32_t[]){{{literal}}}, {len(places)}, "
                        f"{self._own_address()}, {own_size}u, {k}, {address})")
            return address
        if n.get("kind") == "UnaryOperator" and n["opcode"] == "*":
            return self.rv(_inner(n)[0])
        raise Unsupported(f"function designator {n.get('kind')}")

    def code_address(self, sym: str) -> str:
        """The code address of the function sym as the function being translated forms it, as a
        gaddr: the copy in the program loaded. The assembly's translation (asmc.py) asks here too."""
        if self.fn is not None:
            self.fn.report.calls.add(sym)
        places = self.program.code_places(sym)
        if not places:
            raise Unsupported(f"the address of {sym}, which has none")
        if getattr(self.program, "relocate", None) and self.fn is not None                     and not LEVEL_NAME.match(self.fn.name) and places[0][0] < 0:
            return f"OPENRAC_CODE({hexaddr(places[0][1])})"
        # A place split off as a variant of its own (split_places.tsv) is still where this name
        # sits in that level: the address the level's own instruction forms.
        places = places + [p for p in getattr(self.program, "splits", {}).get(sym, []) if p not in places]
        level = self._shared_level()
        copy = self._copy_places()
        rel = getattr(self.program, "relocate", None)
        if copy is not None and places[0][0] < 0 and rel and rel[0] <= places[0][1] < rel[1]:
            home, canon, size = copy
            mine = [a for o, a in places if o == home]
            if mine:
                # An executable function taken by name in code written for a level: the executable's
                # address is level code while that runs (the level's program overwrites it there). The
                # place the code forms in its level, and in another level's copy the same instructions'
                # (guest.h); of a function the catalogue folded (one name for small routines at several
                # places, the `return 0` leaf func_001E97C8), whichever of them this code forms. Where no
                # copy forms one: a function with one place there is that place relocated
                # (OPENRAC_LCODE), a folded one the executable's address (level 18 has nothing at level
                # 0's first place of func_001E9768).
                self.fn.uses_self = True
                fallback = f"OPENRAC_LCODE({home}, {hexaddr(mine[0])})" if len(mine) == 1 else hexaddr(places[0][1])
                return (f"openrac_code_formed({home}, {hexaddr(canon)}, {size}u, openrac_self_, {hexaddr(mine[0])}, "
                        f"{fallback})")
        if copy is not None and places[0][0] >= 0:
            # One C for copies at several places: the address this copy forms (guest.h).
            home, canon, size = copy
            own = next((a for o, a in places if o == home), places[0][1])
            self.fn.uses_self = True
            return f"openrac_code_in_copy({home}, {hexaddr(canon)}, {size}u, openrac_self_, {hexaddr(own)})"
        if level is not None and (places[0][0] >= 0 or sum(o == level for o, _ in places) == 1):
            # An executable function the levels' programs also carry (its catalogue places)
            # counts as theirs here: the copy in the level loaded, as the level's own
            # instruction forms it, not the executable's address under the level program. Not
            # one the catalogue folds at several places of a level (func_001E9768, an empty
            # routine at 43 places of level 0): which of them is the copy's own is not known
            # here, and level 18 has none at level 0's first; the executable's does the same.
            own = next((a for o, a in places if o == level), places[0][1])
            return f"OPENRAC_LCODE({level}, {hexaddr(own)})"
        # A function of one level only: the place of the other in that level (its program is
        # loaded whenever this runs), not the first level's.
        levels = {o for o, _ in self.program.code_places(self.fn.name)} if self.fn is not None else set()
        if len(levels) == 1 and min(levels) >= 0:
            here = next((a for o, a in places if o == min(levels)), None)
            if here is not None:
                return hexaddr(here)
        return hexaddr(places[0][1])

    def _code_variable(self, n: dict) -> bool:
        """A variable that names code: `extern char f[] __asm__("func_L06_00300AB0")`, the way matched
        C takes a function's address. Its address is the function's, found as for a function
        designator (the copy in the level loaded), not the one place its name has."""
        n = _strip_parens(n)
        if n.get("kind") != "DeclRefExpr" or n["referencedDecl"].get("kind") != "VarDecl":
            return False
        d = self.decls.get(n["referencedDecl"]["id"])
        if d is None or d.get("storageClass") != "extern":
            return False
        sym = self.program.canonical(symbol_of(d))
        return sym in self.program.functions and bool(self.program.code_places(sym))

    def _folded_place_name(self, ref: dict, sym: str) -> str:
        """For the address of a function: the name as written when it is one place of a function
        the catalogue folded several places of one program into (func_L00_00238128, the second
        of func_L00_00237B70's five level 0 wrappers), so the address is that place's and not
        the folded function's first (each wrapper calls another function)."""
        d = self.decls.get(ref["referencedDecl"]["id"])
        raw = symbol_of(d) if d is not None else ref["referencedDecl"].get("name", "")
        raw = self.program.aliases.get((self.unit, raw), raw)
        if raw == sym or raw in self.program.places:
            return sym
        named = self.program.code_places(raw)
        if len(named) != 1:
            return sym
        overlay = named[0][0]
        folded = [p for p in self.program.code_places(sym) if p[0] == overlay]
        return raw if len(folded) > 1 and named[0] in folded else sym

    def _sibling_named(self, body: dict) -> set:
        """The folded functions whose address the body also takes by the name of another of their
        places in one program (_folded_place_name): it tells their places apart itself."""
        out: set = set()
        stack = [body]
        while stack:
            n = stack.pop()
            if not isinstance(n, dict):
                continue
            if n.get("kind") == "DeclRefExpr" and (n.get("referencedDecl") or {}).get("kind") == "FunctionDecl":
                try:
                    sym = self._callee_symbol(n)
                    if self._folded_place_name(n, sym) != sym:
                        out.add(sym)
                except (KeyError, AttributeError):
                    pass
            stack.extend(c for c in n.get("inner", ()) if isinstance(c, dict))
        return out

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
            return self.const_names.get(ref["name"], ref["name"])
        if kind == "FunctionDecl":
            return self.function_value(n)
        return self.lv(n).text

    def _ps2(self) -> bool:
        """Whether this function's float arithmetic is the EE's (hostgen.json "ps2_float")."""
        if self.fn is None or self.fn.name not in getattr(self.program, "ps2_float", ()):
            return False
        self.uses_ps2 = True
        return True

    def _ps2_binary(self, op: str, x: str, y: str) -> str | None:
        """A float operator as the EE's (openrac/ps2float.h): arithmetic cuts its result,
        comparisons are of sign and magnitude with every zero equal (c.lt.s, c.le.s, c.eq.s). The
        operands stay in source order (a > b is gt(a, b), not lt(b, a)): with a call on each side,
        the left one is made first, as the game's code makes it."""
        if op in PS2_ARITH:
            return f"{PS2_ARITH[op]}({x}, {y})"
        return {"<": f"openrac_ps2_lt({x}, {y})", ">": f"openrac_ps2_gt({x}, {y})",
                "<=": f"openrac_ps2_le({x}, {y})", ">=": f"openrac_ps2_ge({x}, {y})",
                "==": f"openrac_ps2_eq({x}, {y})", "!=": f"(!openrac_ps2_eq({x}, {y}))"}.get(op)

    def _ps2_int_source(self, t: ctype.Type) -> bool:
        """An integer the EE converts to or from a float with cvt.s.w / cvt.w.s: a signed one of
        32 bits or fewer, or a narrower unsigned one (an unsigned int or a 64-bit integer takes
        other code: the conversions of guest.h, as in any function)."""
        r = self.resolve(ctype.strip_quals(t))
        name = r.name if isinstance(r, ctype.Base) else ""
        return name.startswith("enum ") or name in (
            "int", "signed int", "signed", "short", "signed short", "short int", "signed short int",
            "signed char", "char", "unsigned char", "unsigned short", "unsigned short int", "_Bool",
            "s8", "s16", "s32", "u8", "u16")

    def rv_UnaryOperator(self, n: dict) -> str:
        op = n["opcode"]
        child = _inner(n)[0]
        if op == "!" and self.cls(self.ty(child)) == "float" and self._ps2():
            return f"openrac_ps2_eq({self.rv(child)}, 0.0f)"  # every zero is zero (no denormals)
        if op == "&":
            c = _strip_parens(child)
            if c.get("kind") == "DeclRefExpr" and (c["referencedDecl"].get("kind") == "FunctionDecl"
                                                   or self._code_variable(c)):
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
                return f"({lv.text} = {self._wide_value(b, ctype.Base('long long'))})"
            return f"({lv.text} = {self.convert(b, self.ty(a))})"
        if op in ("+", "-"):
            ta, tb = self.ty(a), self.ty(b)
            pa, pb = self.cls(ta) == "ptr", self.cls(tb) == "ptr"
            if pa and pb and op == "-":
                return f"((int32_t)((gaddr)({self.rv(a)}) - (gaddr)({self.rv(b)})) / (int32_t){self.pointee_size(ta)})"
            code = self._code_as_data(a) if pa else None
            if code is not None:
                # An address inside a function the C declares as data (an entry 8 bytes into
                # what the catalogue starts earlier): a code address, relocated as one, offset
                # included, so a loaded level gives its own copy of the code.
                return f"OPENRAC_CODE((gaddr)({hexaddr(code)}) {op} (gaddr)({self.rv(b)}) * {self.pointee_size(ta)})"
            if pa:
                return f"((gaddr)({self.rv(a)}) {op} (gaddr)({self.rv(b)}) * {self.pointee_size(ta)})"
            if pb:
                return f"((gaddr)({self.rv(b)}) + (gaddr)({self.rv(a)}) * {self.pointee_size(tb)})"
        if (self.cls(self.ty(a)) == "float" and self.cls(self.ty(b)) == "float"
                and op in (*PS2_ARITH, "<", ">", "<=", ">=", "==", "!=") and self._ps2()):
            return self._ps2_binary(op, self.rv(a), self.rv(b))
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
        if (op[:-1] in PS2_ARITH and self.cls(ta) == "float" and self.cls(self.ty(b)) == "float"
                and self._ps2()):
            if lv.addr is not None:  # read and written once, through its address
                ref = self.gref(ta, "pa_")
                return f"({{ gaddr pa_ = (gaddr)({lv.addr}); {ref} = {PS2_ARITH[op[:-1]]}({ref}, {self.rv(b)}); }})"
            return f"({lv.text} = {PS2_ARITH[op[:-1]]}({lv.text}, {self.rv(b)}))"
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
        named_level = self.program.data_level(sym) if hasattr(self.program, "data_level") else None
        addr = self.global_address(address, self._code_symbol(sym) is not None, named_level)
        return LV(self.gref(t, addr), addr)

    def global_address(self, address: int, code: bool = False, named_level: int | None = None) -> str:
        """The address of a global at `address` as the running program finds it: in a range the
        levels move (hostgen.json "relocate"), the loaded level's (guest.h). `code`: the address is
        an executable function's code read as data; `named_level`: the level its name gives
        (D_L00_...). A global's name and a literal address of it (`*(s32 *)0x1610A8`) take the
        same path."""
        addr = hexaddr(address)
        if self._relocated(address) and code:
            # A function's code declared as data: where that code is in the program loaded.
            addr = f"OPENRAC_CODE({addr})"
        elif self._relocated(address) and named_level is not None:
            # A level's datum by its name (D_L00_...) in a function that is not a level's (a
            # static helper of a level file): that level's, looked up for the level loaded, as in
            # the level functions beside it; OPENRAC_DATA would take it for the executable's.
            addr = f"OPENRAC_LDATA({named_level}, {addr})"
        elif self._relocated(address) and (small := self._small_folded()) is not None and small[0] < 0:
            # A small executable function the catalogue folded with level functions of the same
            # instructions on other operands: the address the running copy forms itself (guest.h),
            # OPENRAC_DATA's when it runs as the executable's or is called directly. rac1:
            # func_0023BF48 (the movie's printf, a0 its buffer) is also a Novalis moby's update,
            # whose a0 is a draw callback of its level; as data that was 0x001613B8.
            home, canon, size = small
            self.fn.uses_self = True
            addr = f"openrac_data_in_copy({home}, {hexaddr(canon)}, {size}u, openrac_self_, {addr})"
        elif self._relocated(address):
            addr = f"OPENRAC_DATA({addr})"
        else:
            level = self._shared_level()
            low, high = getattr(self.program, "relocate", None) or LEVEL_RELOCATE
            copies = self._copies_in_one_program()
            if copies is not None and low <= address < high:
                # Copies of this function in one program, each on its own globals: the one the
                # running copy uses (guest.h).
                home, canon, size = copies
                self.fn.uses_self = True
                addr = f"openrac_data_in_copy({home}, {hexaddr(canon)}, {size}u, openrac_self_, {addr})"
            elif level is not None and low <= address < high:
                addr = f"OPENRAC_LDATA({level}, {addr})"
        return addr

    def _variant_call(self, sym: str) -> tuple[int, int, int, int] | None:
        """For a call from a level function placed in several levels to a level function missing
        from some of them (a level whose copy of the caller calls a variant of its own there): the
        level the caller is written for, its place there, its size, and the callee's place there."""
        fn = self.fn
        if fn is None or not getattr(self.program, "relocate", None) or not LEVEL_NAME.match(fn.name):
            return None
        caller = [(o, a) for o, a in self.program.code_places(fn.name) if o >= 0]
        callee = [(o, a) for o, a in self.program.code_places(sym) if o >= 0]
        caller_levels, callee_levels = {o for o, _ in caller}, {o for o, _ in callee}
        if len(caller_levels) < 2 or not callee_levels or caller_levels <= callee_levels:
            return None
        level = self._shared_level()
        home = level if level is not None else min(caller_levels)
        if home not in callee_levels:
            return None
        canon = next(a for o, a in caller if o == home)
        own = next(a for o, a in callee if o == home)
        return home, canon, getattr(self.program, "sizes", {}).get(fn.name) or 0x400, own

    def _copies_in_one_program(self) -> tuple[int, int, int] | None:
        """For a level function the catalogue places more than once in one program (the same code
        on other globals): the program it is written for, its place there and its size."""
        fn = self.fn
        if fn is None or not getattr(self.program, "relocate", None) or not LEVEL_NAME.match(fn.name):
            return None
        places = [(o, a) for o, a in self.program.code_places(fn.name) if o >= 0]
        overlays = [o for o, _ in places]
        if len(overlays) == len(set(overlays)):
            return None
        level = self._shared_level()
        home = level if level is not None else places[0][0]
        canon = next((a for o, a in places if o == home), places[0][1])
        return home, canon, getattr(self.program, "sizes", {}).get(fn.name) or 0x400

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
        m = LEVEL_NAME.match(fn.name)
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
        if fn is None or not getattr(self.program, "relocate", None) or not LEVEL_NAME.match(fn.name):
            return None
        places = [(o, a) for o, a in self.program.code_places(fn.name) if o >= 0]
        if len(places) < 2:
            return None
        level = self._shared_level()
        home = level if level is not None else places[0][0]
        canon = next((a for o, a in places if o == home), places[0][1])
        size = getattr(self.program, "sizes", {}).get(fn.name) or 0x400
        return home, canon, size

    def _code_symbol(self, sym: str) -> int | None:
        """The executable address of sym when it names a function of the executable (its name is
        a code name, e.g. func_0022F4C0) and this is an executable function in a program whose
        levels move code (hostgen.json "relocate"); otherwise None."""
        if not getattr(self.program, "relocate", None) or self.fn is None or LEVEL_NAME.match(self.fn.name):
            return None
        if not any(p.match(sym) for p in self.program.code_names):
            return None
        places = self.program.code_places(sym)
        return places[0][1] if places and places[0][0] < 0 else None

    def _code_as_data(self, n: dict) -> int | None:
        """Of an array declared over a function's code (extern u8 x[] __asm__("func_..."), its
        decayed address): the function's executable address, else None."""
        n = _strip_parens(n)
        while n.get("kind") in ("ImplicitCastExpr", "CStyleCastExpr") and n.get("castKind") in (
                "ArrayToPointerDecay", "NoOp", "BitCast"):
            n = _strip_parens(_inner(n)[0])
        if n.get("kind") != "DeclRefExpr" or n["referencedDecl"].get("kind") != "VarDecl":
            return None
        ref = n["referencedDecl"]
        if self.fn is not None and (ref["id"] in self.fn.memory or ref["id"] in self.fn.statics
                                    or ref["id"] in self.fn.host):
            return None
        d = self.decls.get(ref["id"], ref)
        return self._code_symbol(symbol_of(d))

    def _relocated(self, address: int) -> bool:
        """An executable function's global in the range that level programs move (hostgen.json
        "relocate"): its address is looked up for the level loaded (guest.h)."""
        rel = getattr(self.program, "relocate", None)
        return bool(rel) and self.fn is not None and not LEVEL_NAME.match(self.fn.name)             and rel[0] <= address < rel[1]

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
        named = None  # the function a call by name reaches through a code address
        wide_ret = None
        if c.get("kind") == "ImplicitCastExpr" and c.get("castKind") == "FunctionToPointerDecay":
            target = _strip_parens(_inner(c)[0])
            if target.get("kind") == "DeclRefExpr" and target["referencedDecl"].get("kind") == "FunctionDecl":
                sym = self._callee_symbol(target)
                if sym in getattr(self.program, "returns_64", ()):
                    # A function returning 64 bits that this unit declares int (hostgen.json
                    # "returns_64"): called through a code address below, it keeps them as a direct
                    # call does (EnvMapA_Draw's GetEffectTex TEX0 in the copy at 0x2EB460).
                    own_f = self.program.lookup(self.unit, sym)
                    if own_f is not None:
                        wide_ret = self._canon_type(own_f.sig.ret)
                named = sym
                # Calls to a folded function and to the variants split off it (split_places.tsv)
                # are counted together, in the order they come: retail's k-th call to any of the
                # function's places is the k-th of them all (openrac_folded_call). Counted per name,
                # a function calling three variants before the function itself took its call for the
                # first variant's: the frame draw ran draw callback list 3 in place of list 1, and
                # no list-1 callback (the nanotech crates' sphere among them) was ever drawn.
                # An executable function whose level copies are variants (split_places.tsv's fourth
                # column: the variant's twin) is called by its own name and counts with them, in the
                # family of the first function its variant was split off (a variant may be split off
                # a variant): the frame draw (func_001F3D78) calls func_00234F40 and func_00234EE0,
                # whose level copies the catalogue folds into AA_BlurPass (func_001FB848), eight
                # times before AA_BlurPass; counted without them, AA_BlurPass took the first of those
                # jals, and in every level the port ran func_00234F40's copy (a packet call) in
                # place of the blur pass.
                split_parent = getattr(self.program, "split_parent", {})
                twin = getattr(self.program, "split_twin", {}).get(sym)
                if twin is None:
                    parent = split_parent.get(sym, sym)
                else:
                    parent, seen = twin, {twin}
                    while parent in split_parent and split_parent[parent] not in seen:
                        parent = split_parent[parent]
                        seen.add(parent)
                family_k = None
                if self.fn is not None and parent in getattr(self.program, "splits", {}):
                    family = getattr(self.fn, "family_calls", None)
                    if family is None:
                        family = self.fn.family_calls = {}
                    family_k = family.get(parent, 0)
                    family[parent] = family_k + 1
                places = self._folded_places(sym)
                copy = self._small_folded()
                if copy is None and not places:
                    # A larger level function the catalogue placed twice in one program: its copies
                    # are the same instructions, and may call different functions all the same.
                    # rac1's func_L02_002D75F8 (76 bytes) is both of the circular saw's (class 340)
                    # Suck Cannon hooks on Eudora: the one at 0x2C53B0 calls the grab, the one at
                    # 0x2C5400 places the moby in the cannon. Called as the C names it, the second
                    # ran the grab with garbage for its position and owner, and the saw's next
                    # update read the cannon's joints through that owner.
                    copies = self._copies_in_one_program()
                    if copies is not None and any(o == copies[0] for o, _ in self.program.code_places(sym)):
                        copy = copies
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
                    named = None  # each level fills the hook with its own function: the call site's type
                elif not places and callee_places:
                    # Inside a function the catalogue folded (copies that differ in what they
                    # call): the function this copy's own call at that place calls (guest.h).
                    home, canon, size = copy
                    own = next((a for o, a in callee_places if o == home), callee_places[0][1])
                    self.fn.report.calls.add(sym)
                    self.fn.uses_self = True
                    fp = f"openrac_call_in_copy({home}, {hexaddr(canon)}, {size}u, openrac_self_, {hexaddr(own)})"
                    named = self._function_at(home, own) or sym
                elif not places and (variant := self._variant_call(sym)) is not None:
                    # Engine code several levels carry, calling a level function some of those levels
                    # have another of in its place: the function this copy's own call calls.
                    home, canon, size, own = variant
                    self.fn.report.calls.add(sym)
                    self.fn.uses_self = True
                    fp = f"openrac_call_in_copy({home}, {hexaddr(canon)}, {size}u, openrac_self_, {hexaddr(own)})"
                    named = self._function_at(home, own) or sym
                elif not places and (cross := self._cross_level(sym)) is not None:
                    # A function of one level's program calling, by name, a function the catalogue
                    # lists in other levels only (the decompilation names the copy by its twin):
                    # the loaded level's own copy of it (OPENRAC_LCODE, guest.h).
                    self.fn.report.calls.add(sym)
                    fp = f"OPENRAC_LCODE({cross[0]}, {hexaddr(cross[1])})"
                    named = self._function_at(cross[0], cross[1]) or sym
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
                    if family_k is not None:
                        k = family_k
                    own_size = getattr(self.program, "sizes", {}).get(self.fn.name) or 0
                    copies = self._copies_in_one_program()
                    own = self._own_address()
                    if copies is not None and self._shared_level() is not None:
                        # This function is several levels' code and has more than one copy in some
                        # program, each calling its own place (level 7's two func_L00_002E0490 call
                        # AddDrawCallback's places 0x2214C8 and 0x2216D0): the k-th call of the copy
                        # that runs, when it was called through its code address (guest.h).
                        self.fn.uses_self = True
                        own = f"(openrac_self_ != 0 ? openrac_self_ : {own})"
                    fp = (f"openrac_folded_call((const uint32_t[]){{{literal}}}, {len(places)}, "
                          f"{own}, {own_size}u, {k})")
                    if copies is not None and self._shared_level() is None:
                        # This function is itself one of several copies in one level's program, and a
                        # copy may call another function at that place than the first copy does (Drek's
                        # Fleet's third window-glass callback calls a reflection helper of its own): the
                        # function the running copy's own call goes to (guest.h).
                        home, canon, size = copies
                        self.fn.uses_self = True
                        fp = f"openrac_call_in_copy({home}, {hexaddr(canon)}, {size}u, openrac_self_, {fp})"
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
        if wide_ret is not None and self.cls(hostft.ret) == "int" and self.cls(wide_ret) == "int":
            hostft = ctype.Func(wide_ret, hostft.params, hostft.variadic, hostft.prototyped)
        recast = self._recast_call(c, ft, args, result_t, fp)
        if recast is not None:
            return recast
        if named is not None:
            defined = self._call_as_defined(named, fp, args, result_t)
            if defined is not None:
                return defined
        values = [self.rv(a) for a in args]
        return f"GFN({ctype.fn_pointer(hostft)}, {fp})({', '.join(values)})"

    def _function_at(self, overlay: int, address: int) -> str | None:
        """The function a call through that place finds (Program.function_at), or None."""
        at = getattr(self.program, "function_at", None)
        return at(overlay, address) if at is not None else None

    def _call_as_defined(self, sym: str, fp: str, args: list, result_t: ctype.Type) -> str | None:
        """A call by name that goes through a code address (a folded name's place, a level's copy,
        another level's function): through the definition's own type, its arguments matched to its
        parameters as the EE passes them (each kind in its order, as a direct call), when the call
        site's declaration puts a kind at another position or reads another kind of result. On the
        EE, System V and AArch64 integers and floats travel in registers of their own, so the
        declaration's order did not matter there; on Windows x64 a position decides the register
        whatever the type, and `f(float, void *, void *)` reaching `f(void *, void *, float)` read
        its pointers from registers nobody set (rac1/ntsc's Rilgar, 0.8.0). None when the call
        site's type already lines up with the definition, or the definition cannot be matched so."""
        f = self.program.lookup(self.unit, sym)
        if f is None or not f.sig.prototyped or f.sig.variadic:
            return None
        sig = f.sig
        classes = [self._arg_class(a) for a in args]
        pcls = [("agg" if self.sig_cls(p) == "rec" else self._reg_class(self.sig_cls(p))) for p in sig.params]
        if "agg" in classes or "agg" in pcls or any(self.cls(p) == "valist" for p in sig.params):
            return None
        if self._folded_places(sym) and any(classes.count(k) > pcls.count(k) for k in ("i", "f")):
            # A name the catalogue folded (a small wrapper at several places of one program, each
            # calling another function): its definition is one of them, and a call site that passes
            # more of a kind than that one takes calls another, whose type is the call site's.
            # PartType74Update's colour tween goes through a place of func_L00_00237B70 with
            # (float, int, int); as the folded definition's (void *) the tween got neither the
            # fraction nor the second colour, and the fraction's IntToFloat never ran.
            return None
        result_cls = self.cls(result_t)
        ret_cls = self.sig_cls(sig.ret)
        if ret_cls == "rec":
            return None
        common = min(len(classes), len(pcls))
        same_ret = result_cls == "void" or (ret_cls != "void"
                                            and self._reg_class(ret_cls) == self._reg_class(result_cls))
        if classes[:common] == pcls[:common] and same_ret:
            return None  # the call site's own type passes them alike
        self.fn.report.calls.add(sym)
        self._note_mismatch(sym, sig, args, result_cls, ret_cls)
        own = ctype.Func(self._canon_type(sig.ret), tuple(self._canon_type(p) for p in sig.params), False, True)
        text = f"GFN({ctype.fn_pointer(own)}, {fp})({', '.join(self._match_args(sym, sig, args))})"
        if result_cls == "void":
            return text
        if ret_cls == "void" or self._reg_class(ret_cls) != self._reg_class(result_cls):
            # On the console the caller read a register the callee did not set.
            return f"({text}, ({self.host(ctype.strip_quals(result_t))})0)"
        if result_cls == "ptr":
            return f"(gaddr)({text})"
        return f"(({self.host(ctype.strip_quals(result_t))})({text}))"

    def _recast_call(self, callee: dict, ft: ctype.Func, args: list, result_t: ctype.Type, fp: str) -> str | None:
        """A call through a named function cast to another prototype, `((void (*)(void *, float,
        int))f)(p, x, n)` for an `f(void *, int, float)`: the decompilation's way to set up the
        arguments in another order. On the EE the integers and the floats go in registers of their
        own, each kind in order, so the call is f's own; the host's ABI may not assign them that way
        (Windows x64 gives each argument a slot by position), so the call goes through f's own
        type with each kind's arguments in its order, as a direct call matches them (_call_as_defined).
        None when the callee is not such a cast or the cast's type already lines up."""
        if callee.get("kind") != "CStyleCastExpr" or not ft.prototyped or ft.variadic:
            return None
        inner = _strip_parens(_inner(callee)[0])
        if inner.get("kind") == "ImplicitCastExpr" and inner.get("castKind") == "FunctionToPointerDecay":
            inner = _strip_parens(_inner(inner)[0])
        if inner.get("kind") != "DeclRefExpr" or inner["referencedDecl"].get("kind") != "FunctionDecl":
            return None
        # Also a cast to fewer arguments of one kind (`((s32 (*)(f32))f)(1.0f)` for an
        # `f(u8 *, s32, f32)`, rac3's text width): f's own type, the parameters the call does not
        # pass taken as a direct call takes them (the caller's own, passed through).
        sym = self._callee_symbol(inner)
        if "__" in sym and self.program.group_of(self.unit) is None:
            # A separate program's function standing in for one of the executable with no C
            # (_callee_symbol): the address is the executable's too, and its function is found
            # there while no other program is loaded over it.
            base = sym.rsplit("__", 1)[0]
            if base in self.program.functions:
                sym = base
        return self._call_as_defined(sym, fp, args, result_t)

    def _cross_level(self, sym: str) -> tuple[int, int] | None:
        """For a call from a function of some levels' programs to a level function placed in none
        of those levels: the callee's first place; else None."""
        if self.fn is None:
            return None
        mine = self.program.code_places(self.fn.name)
        theirs = self.program.code_places(sym)
        if not mine or not theirs or any(o < 0 for o, _ in mine) or any(o < 0 for o, _ in theirs):
            return None
        if {o for o, _ in mine} & {o for o, _ in theirs}:
            return None
        return min(theirs)

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
            # Any function with copies in one program: a wrapper's call other functions, a larger
            # function's use other globals (_copies_in_one_program); the caller's jal says which.
            places = self.program.code_places(sym) if getattr(self.program, "relocate", None) else []
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
        if len({o for o, _ in places}) > 1:
            # Engine code at a place in the executable and one in each level program: the copy the
            # loaded program has (the executable's is overwritten while a level is loaded).
            literal = ", ".join(f"0x{(o + 1) << 24 | a:08X}u" for o, a in places)
            return f"openrac_guest_nearest((const uint32_t[]){{{literal}}}, {len(places)}, 0u)"
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
        if name == "__builtin_alloca" and self.fn is not None:
            # Space on the game stack, as the console's alloca takes it from the frame.
            self.fn.uses_alloca = True
            return f"openrac_guest_frame((uint32_t)({v[0]}))"
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
        ret_cls = self.sig_cls(sig.ret)
        self._note_mismatch(sym, sig, args, result_cls, ret_cls)
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

    def _note_mismatch(self, sym: str, sig, args: list, result_cls: str, ret_cls: str) -> None:
        """Record a direct call whose site disagrees with the callee's definition in what the EE's
        registers carry: fewer or more integer (or float) arguments than it takes, or a result it
        does not return. Another order of the two kinds is the same call on the EE. For
        report.json's call_site_mismatches."""
        if not sig.prototyped:
            return
        what = []
        kinds = [self._arg_class(a) for a in args]
        pkinds = [("agg" if self.sig_cls(p) == "rec" else self._reg_class(self.sig_cls(p))) for p in sig.params]
        for k, label in (("i", "integer"), ("f", "float"), ("agg", "record")):
            n, m = kinds.count(k), pkinds.count(k)
            if n != m and not (sig.variadic and n > m):
                what.append(f"{n} {label} arguments for {m}")
        if result_cls != "void" and (ret_cls == "void" or self._reg_class(ret_cls) != self._reg_class(result_cls)):
            what.append(f"reads a {result_cls} result, returns {ret_cls}")
        if what:
            self.fn.report.mismatches.append(f"{sym}: {'; '.join(what)}")

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
        pcls = [("agg" if self.sig_cls(p) == "rec" else self._reg_class(self.sig_cls(p))) for p in sig.params]
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
        counted = False
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
                # Nothing of the caller's own in that register either: on the console it held
                # whatever the caller last put there, which C does not say (hostgen's report).
                detail = f"{sym} argument {len(out) + 1}"
                self.fn.report.zero_args.append(detail)
                if not counted:
                    # Counted once a call, in a site of the call's own (guest.h, openrac_reach_site):
                    # the run's OPENRAC_REACH file says which of these the game reached.
                    counted = True
                    out.append(self._cast_to(
                        f"({{ static openrac_reach_site site_ = {{\"{self.fn.name}\", \"zero-args\", "
                        f"\"{detail}\", 0, 0}}; openrac_guest_reach(&site_); 0; }})", p))
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
        if self.fn is not None and self.fn.wide_params and self._wide(p):
            # A parameter of the function's own that it only passes on (widen_forwarded): all 64 bits, unchanged,
            # whatever casts the C puts around it.
            n = a
            while n.get("kind") in ("ImplicitCastExpr", "CStyleCastExpr", "ParenExpr") and _inner(n):
                n = _inner(n)[0]
            ref = n.get("referencedDecl", {}) if n.get("kind") == "DeclRefExpr" else {}
            if ref.get("id") in self.fn.wide_params and ref.get("id") in self.fn.host:
                return self._cast_to(self.fn.host[ref["id"]], p)
        v = self.rv(a)
        if (self._wide(p) and self.cls(self.ty(a)) in ("int", "ptr") and not self._wide(self.ty(a))
                and not self._names_a_variable(a) and not self._wide_call(a)):
            # A 32-bit value (an int, an unsigned, a pointer: the call site's own type) that the
            # call computes or loads, where the definition takes 64 bits: on the EE the register
            # holds it sign-extended, as every 32-bit load and operation leaves it (rac1:
            # FontPrintWindow hands its colour on as a `void *` loaded from a table to FontPrint's
            # u64, 0x80FFA888 is 0xFFFFFFFF80FFA888 there). A variable or parameter passed as it is
            # keeps whatever its register held, as the port carries it (a TEX0 the matching C keeps
            # in an int, a parameter passed on); so does a call of a "returns_64" function (all its
            # bits: Effect_DrawEntryList(..., GetEffectTex(0x2C), ...)).
            v = f"(int32_t)({v})"
        return self._cast_to(v, p)

    def _wide_param(self, n: dict) -> str | None:
        """The host name of the widened parameter (widen_forwarded) N reads, through parentheses, NoOp casts
        and the read itself; None for anything else."""
        if self.fn is None or not self.fn.wide_params:
            return None
        while (n.get("kind") == "ParenExpr" or (n.get("kind") in ("ImplicitCastExpr", "CStyleCastExpr")
                                                 and n.get("castKind") in ("LValueToRValue", "NoOp"))) and _inner(n):
            n = _inner(n)[0]
        ref = n.get("referencedDecl", {}) if n.get("kind") == "DeclRefExpr" else {}
        if ref.get("id") in self.fn.wide_params and ref.get("id") in self.fn.host:
            return self.fn.host[ref["id"]]
        return None

    def _names_a_variable(self, a: dict) -> bool:
        """The argument is a variable or parameter by name (through casts and parentheses)."""
        n = a
        while n.get("kind") in ("ImplicitCastExpr", "CStyleCastExpr", "ParenExpr") and _inner(n):
            n = _inner(n)[0]
        return n.get("kind") == "DeclRefExpr" and n.get("referencedDecl", {}).get("kind") in ("VarDecl", "ParmVarDecl")

    _WIDE_INTS = frozenset({"long long", "long long int", "unsigned long long", "unsigned long long int",
                            "signed long long", "signed long long int"})

    def _wide(self, t: ctype.Type) -> bool:
        """An integer type of 64 bits (the EE's long, long long and their typedefs)."""
        r = self.resolve(t)
        if self.cls(r) != "int" or not isinstance(r, ctype.Base):
            return False
        k = r.name if r.name in ctype.INT_NAMES else canon(r, self.program.typedefs if self.program else None)
        return k in self._WIDE_INTS

    def _cast_to(self, v: str, p: ctype.Type) -> str:
        canon = self.canon(p)
        if canon is None:
            return v
        return f"({canon})({v})"
