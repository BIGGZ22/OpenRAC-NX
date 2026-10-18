"""Moby behaviour as GDScript: translates a class's update function from the decompiled
C (games/rac1) to a script the editor attaches to the class scene.

    behaviour.py export <godot project> [--level N] [--classes 357,749,500]
    behaviour.py census
    behaviour.py show <class>        # the translated script on stdout

The script is for reading, and for running in Godot's preview against the RcEngine stub
(rc1/rc_engine.gd). It is not what the game runs: the native port is built from the C. Each
translation reports how much of the function it translated; what it could not read is left
as `pass  # not translated: <the C>` so nothing is dropped silently.

What is translated: the control flow (if, switch, for, while, return), declarations,
assignments, arithmetic, member access through the typed structs (the moby header from
include/moby.h and the class's pvars struct), and calls to engine helpers named in
behaviour_api.tsv. What is not: raw `*(T *)(p + 0x10)` memory access, `&` of a scalar that
is not a local or a struct member, assignments inside expressions, ++ in expressions,
do/goto, compound literals and anything in an engine helper's own body (those are stubs).
"""

import argparse
import re
import sys
from collections import Counter
from dataclasses import dataclass, field
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))

from cparse import CType, Parser, Types, function_source, lex, strip
from mobys import DECOMPILATION, moby_class_names, moby_logic

HERE = Path(__file__).parent
API = HERE / "behaviour_api.tsv"
SCRIPTS = HERE / "rc1"
MOBY_STRUCT = "Moby"
REPO = DECOMPILATION.parents[1]

GD_KEYWORDS = {"var", "const", "func", "class", "extends", "signal", "enum", "static", "match", "when", "in", "is", "as",
               "self", "super", "pass", "await", "void", "null", "true", "false", "not", "and", "or", "if", "elif",
               "else", "for", "while", "break", "continue", "return", "preload", "assert", "breakpoint", "namespace",
               "trait", "class_name", "range", "int", "float", "bool", "String"}
# Names the script's base class (Node3D) already has; a pvar of that name gets a pv_ prefix.
NODE_NAMES = {"name", "owner", "position", "rotation", "scale", "transform", "basis", "visible", "top_level", "global_position",
              "global_rotation", "global_transform", "quaternion", "process_mode", "script", "scene_file_path", "multiplayer",
              "unique_name_in_owner", "editor_description", "rotation_order", "rotation_edit_mode", "global_basis",
              "visibility_parent", "process_priority", "process_physics_priority", "physics_interpolation_mode",
              "auto_translate_mode", "get", "set", "free", "call", "emit_signal", "connect", "duplicate", "queue_free"}
FLOAT_VECS = {"Vec4f", "Vec4A", "Vec4"}
MASKS = {"u8": 0xFF, "u16": 0xFFFF, "u32": 0xFFFFFFFF}


class Untranslatable(Exception):
    pass


@dataclass
class Api:
    kind: str
    symbol: str
    gd: str
    doc: str


def load_api() -> dict[str, Api]:
    api = {}
    for line in API.read_text().splitlines():
        if line and not line.startswith("#"):
            kind, symbol, gd, doc = (line.split("\t") + [""])[:4]
            api[symbol] = Api(kind, symbol, gd, doc)
    macros = load_macros()
    for symbol, entry in list(api.items()):
        if symbol in macros:
            api.setdefault(macros[symbol], entry)
    return api


def load_macros() -> dict[str, str]:
    """names.h: readable name -> func_ symbol."""
    path = DECOMPILATION / "include" / "names.h"
    return dict(re.findall(r"^#define\s+(\w+)\s+(func_\w+)\s*$", path.read_text(), re.M)) if path.is_file() else {}


def load_types(source: Path | None) -> Types:
    types = Types()
    for header in ("common.h", "vector.h", "moby.h", "hero.h", "pad.h", "records.h", "structs.h"):
        path = DECOMPILATION / "include" / header
        if path.is_file():
            types.read(path.read_text())
    if source is not None:
        types.read(source.read_text(errors="replace"))
    return types


def gd_name(name: str) -> str:
    return name + "_" if name in GD_KEYWORDS else name


def export_name(name: str) -> str:
    return "pv_" + name if name in NODE_NAMES or name in GD_KEYWORDS else name


def default_expr(types: Types, t: CType, depth: int = 0) -> str:
    """GDScript for the zero value of a C type."""
    if t.ptr:
        return "null"
    if t.dims:
        n = t.dims[0]
        inner = CType(t.base, 0, t.dims[1:])
        if t.base in ("f32", "f64") and len(t.dims) == 1:
            return f"PackedFloat32Array([{', '.join(['0.0'] * n)}])" if n <= 4 else f"RcEngine.zeros_f({n})"
        if len(t.dims) == 1 and t.base in MASKS | {"s8": 0, "s16": 0, "s32": 0, "s64": 0}:
            return f"RcEngine.zeros_i({n})"
        return "[]" if depth > 1 else f"RcEngine.array_of({n}, func(): return {default_expr(types, inner, depth + 1)})"
    if t.base in ("f32", "f64"):
        return "0.0"
    if t.base in FLOAT_VECS:
        return "PackedFloat32Array([0.0, 0.0, 0.0, 0.0])"
    s = types.structs.get(t.base)
    if s is not None and depth < 3:
        body = ", ".join(f'"{f.name}": {default_expr(types, f.ctype, depth + 1)}' for f in s.fields
                         if not f.name.startswith("pad"))
        return f'RcStruct.of("{s.name}", {{{body}}})'
    return "0"


@dataclass
class Var:
    gd: str
    ctype: CType | None
    boxed: bool = False
    view: bool = False  # points at the moby, through a struct that lays it out differently
    moby: bool = False  # the moby itself (or an alias)


@dataclass
class Result:
    name: str
    source: str
    text: str = ""
    statements: int = 0
    unmapped_statements: int = 0
    untranslated: list = field(default_factory=list)
    pvar_struct: str | None = None
    calls: Counter = field(default_factory=Counter)
    unmapped_calls: Counter = field(default_factory=Counter)

    @property
    def translated(self):
        return self.statements - len(self.untranslated)

    def summary(self) -> str:
        t = self.translated
        pct = 100 * t / max(self.statements, 1)
        return (f"{self.name}: {t} of {self.statements} statements translated ({pct:.0f}%), "
                f"{self.unmapped_statements} of those call an engine function that is only a stub, "
                f"{len(self.untranslated)} not translated")


class Translator:
    def __init__(self, types: Types, api: dict[str, Api], macros: dict[str, str]):
        self.types, self.api, self.macros = types, api, macros
        self.moby = types.structs.get(MOBY_STRUCT)

    # --- scopes ---
    def declare(self, cname: str, ctype: CType | None, **kw) -> Var:
        gd = gd_name(cname)
        n = 2
        while gd in self.used:
            gd = f"{gd_name(cname)}_{n}"
            n += 1
        self.used.add(gd)
        var = Var(gd, ctype, **kw)
        self.scopes[-1][cname] = var
        return var

    def lookup(self, cname: str) -> Var | None:
        for scope in reversed(self.scopes):
            if cname in scope:
                return scope[cname]
        return None

    # --- types of expressions ---
    def ty(self, e) -> CType | None:
        k = e[0]
        if k == "id":
            v = self.lookup(e[1])
            return v.ctype if v else None
        if k == "paren":
            return self.ty(e[1])
        if k == "cast":
            return e[1]
        if k == "member":
            s = self.struct_of(e[1])
            f = s.named(e[2]) if s else None
            return f.ctype if f else None
        if k == "index":
            t = self.ty(e[1])
            return t.deref() if t else None
        if k == "un" and e[1] == "*":
            t = self.ty(e[2])
            return t.deref() if t else None
        if k == "un" and e[1] == "&":
            t = self.ty(e[2])
            return CType(t.base, t.ptr + 1) if t else None
        return None

    def struct_of(self, e):
        t = self.ty(e)
        return self.types.structs.get(t.base) if t and not t.dims else None

    def is_pointer(self, e) -> bool:
        t = self.ty(e)
        return bool(t and t.ptr and not t.dims)

    def is_moby_value(self, e) -> bool:
        while e[0] in ("paren", "cast"):
            e = e[-1] if e[0] == "cast" else e[1]
        if e[0] == "id":
            v = self.lookup(e[1])
            return bool(v and v.moby)
        return False

    # --- expressions: (text, precedence) ---
    def x(self, e, need: int = 0, null_ok: bool = False) -> str:
        text, prec = self.expr(e)
        return f"({text})" if prec < need else text

    def expr(self, e) -> tuple[str, int]:
        k = e[0]
        if k == "paren":
            return self.expr(e[1])
        if k == "num":
            return self.number(e[1]), 100
        if k == "str":
            return (str(ord(e[1][1:-1].encode().decode("unicode_escape"))) if e[1][0] == "'" else e[1]), 100
        if k == "id":
            return self.ident(e[1]), 100
        if k == "member":
            return self.member(e), 100
        if k == "index":
            return f"{self.x(e[1], 100)}[{self.x(e[2])}]", 100
        if k == "call":
            return self.call(e), 100
        if k == "cast":
            return self.cast(e)
        if k == "cond":
            return f"{self.x(e[2], 3)} if {self.x(e[1], 3)} else {self.x(e[3], 2)}", 2
        if k == "un" and e[1] in ("++", "--"):
            pass
        elif k == "un":
            return self.unary(e)
        if k == "bin":
            return self.binary(e)
        if k == "un" and e[1] in ("++", "--") and not self.no_hoist:
            # `if (--t == 0)`: do the step first, then test the value
            self.pre.extend(self.expr_stmt(e))
            return self.expr(e[2])
        if k == "assign" and e[1] == "=" and e[2][0] == "id" and e[3][0] == "num":
            # `if (a < (zero = 0.0f))`: the compiler idiom for a zero register; set it first
            self.pre.append(f"{self.x(e[2])} = {self.number(e[3][1])}")
            return self.expr(e[2])
        raise Untranslatable(f"{k} in an expression")

    def number(self, text: str) -> str:
        if re.match(r"0[xX]", text):
            return text.rstrip("uUlL")
        if re.search(r"[.eE]", text):
            text = text.rstrip("fF")
            return text + "0" if text.endswith(".") else ("0" + text if text.startswith(".") else text)
        return text.rstrip("uUlL")

    def ident(self, name: str) -> str:
        v = self.lookup(name)
        if v:
            return v.gd + (".v" if v.boxed else "")
        if name in ("NULL",):
            return "null"
        if name in ("TRUE", "FALSE"):
            return name.lower()
        symbol = self.real(name)
        a = self.api.get(symbol) or self.api.get(name) or self.api.get(re.sub(r"^D_L\d\d_", "D_", name))
        if a and a.kind == "data":
            return f"RcEngine.{a.gd}"
        if symbol.startswith("func_"):
            return f'&"{symbol}"'
        return f"RcEngine.globals.{name}"

    def real(self, name: str) -> str:
        """The retail symbol a source name stands for: through __asm__ aliases and names.h."""
        name = self.types.symbols.get(name, name)
        return self.macros.get(name, name)

    def member(self, e) -> str:
        _, obj, name, arrow = e
        t = self.ty(obj)
        s = self.types.structs.get(t.base) if t and not t.dims else None
        if s is not None and s.name in FLOAT_VECS and name in "xyzw":
            return f"{self.x(obj, 100)}[{'xyzw'.index(name)}]"
        if s is not None and s.name != MOBY_STRUCT and self.is_view(obj):
            f = s.named(name)
            mf = self.moby.at(f.offset) if f and self.moby else None
            if mf is None or mf.ctype.base != f.ctype.base and f.ctype.ptr == 0 and f.size != mf.size:
                raise Untranslatable(f"{s.name}.{name} (offset {f.offset:#x} is not a moby field)" if f
                                     else f"{s.name} has no {name}")
            return f"{self.x(obj, 100)}.{mf.name}"
        if s is not None and s.name == MOBY_STRUCT and not s.named(name):
            raise Untranslatable(f"moby has no field {name}")
        return f"{self.x(obj, 100)}.{export_name(name) if self.is_pvars(obj) else name}"

    def is_view(self, e) -> bool:
        while e[0] in ("paren", "cast"):
            e = e[-1] if e[0] == "cast" else e[1]
        if e[0] == "id":
            v = self.lookup(e[1])
            return bool(v and v.view)
        return False

    def is_pvars(self, e) -> bool:
        t = self.ty(e)
        return bool(t and self.pvar_struct and t.base == self.pvar_struct)

    def call(self, e) -> str:
        _, fn, args = e
        if fn[0] != "id":
            raise Untranslatable("call through a pointer")
        symbol = self.real(fn[1])
        a = self.api.get(symbol) or self.api.get(fn[1])
        arguments = [self.arg(x) for x in args]
        self.calls[a.gd if a else symbol] += 1
        if a and a.kind == "func":
            return f"RcEngine.{a.gd}({', '.join(arguments)})"
        self.unmapped[symbol] += 1
        self.stmt_unmapped = True
        return f'RcEngine.unmapped(&"{symbol}", [{", ".join(arguments)}])'

    def arg(self, e) -> str:
        if e[0] == "un" and e[1] == "&":
            return self.address(e[2])
        t = self.ty(e)
        if e[0] == "num" and e[1] == "0":
            return "null" if False else "0"
        return self.x(e)

    def address(self, e) -> str:
        t = self.ty(e)
        if e[0] == "paren":
            return self.address(e[1])
        if t is not None and (t.dims or (t.base in self.types.structs and not t.ptr) or t.base in FLOAT_VECS and not t.ptr):
            return self.x(e)
        if e[0] == "id":
            v = self.lookup(e[1])
            if v is not None and v.boxed:
                return v.gd
            if v is None:
                return f'RcRef.of(RcEngine.globals, "{e[1]}")'
            raise Untranslatable(f"address of {e[1]}")
        if e[0] == "member":
            if t is not None and t.ptr == 0 and t.base in self.types.structs:
                return self.x(e)
            if self.struct_of(e[1]) and self.struct_of(e[1]).name in FLOAT_VECS:
                return f"RcRef.of({self.x(e[1], 100)}, {'xyzw'.index(e[2])})"
            return f'RcRef.of({self.x(e[1], 100)}, "{self.member_field(e)}")'
        if e[0] == "index":
            return f"RcRef.of({self.x(e[1], 100)}, {self.x(e[2])})"
        raise Untranslatable("address of an expression")

    def member_field(self, e) -> str:
        text = self.member(e)
        return text.rsplit(".", 1)[1]

    def cast(self, e) -> tuple[str, int]:
        _, t, inner = e
        if t.ptr or t.base in self.types.structs or t.base in FLOAT_VECS:
            return self.expr(inner)
        if t.base in ("f32", "f64"):
            return f"float({self.x(inner)})", 100
        if t.base == "void":
            return self.expr(inner)
        if self.is_pointer(inner):
            return self.expr(inner)
        text = f"int({self.x(inner)})"
        if t.base in MASKS:
            return f"{text} & {MASKS[t.base]:#x}", 9
        return text, 100

    def unary(self, e) -> tuple[str, int]:
        _, op, inner = e
        if op == "!":
            return f"not {self.x(inner, 6)}", 5
        if op in ("-", "~"):
            return f"{op}{self.x(inner, 13)}", 13
        if op == "+":
            return self.expr(inner)
        if op == "&":
            return self.address(inner), 100
        if op == "*":
            q = inner
            while q[0] == "paren":
                q = q[1]
            if q[0] == "cast" and q[2][0] == "un" and q[2][1] == "&":  # *(unsigned short *)&x is x
                return self.expr(q[2][2])
            t = self.ty(inner)
            if t is not None and t.ptr and t.base in self.types.structs:
                return self.expr(inner)
            if t is not None and t.dims:
                return f"{self.x(inner, 100)}[0]", 100
            raise Untranslatable("dereference of a raw pointer")
        raise Untranslatable(f"{op} inside an expression")

    PREC = {"*": 12, "/": 12, "%": 12, "+": 11, "-": 11, "<<": 10, ">>": 10, "&": 9, "^": 8, "|": 7,
            "<": 6, ">": 6, "<=": 6, ">=": 6, "==": 6, "!=": 6, "&&": 4, "||": 3}

    def binary(self, e) -> tuple[str, int]:
        _, op, a, b = e
        if op == "+" and a[0] == "id" and self.lookup(a[1]) is None:
            entry = self.api.get(re.sub(r"^D_L\d\d_", "D_", self.real(a[1])))
            if entry is not None and entry.gd == "moby_table":  # moby_table + (i << 8) is mobys[i]
                inner = b[1] if b[0] == "paren" else b
                if inner[0] == "bin" and inner[1] == "<<" and inner[3] == ("num", "8"):
                    return f"RcEngine.moby_at({self.x(inner[2])})", 100
                raise Untranslatable("arithmetic on the moby table")
        if op in "+-" and (self.is_pointer(a) or (op == "+" and self.is_pointer(b))) and not (
                op == "+" and b[0] == "num" and self.is_moby_value(a)):
            raise Untranslatable("pointer arithmetic")
        if op == "+" and b[0] == "num" and self.is_moby_value(a) and self.moby is not None:
            f = self.moby.at(int(b[1].rstrip("uUlL"), 0))
            if f is not None:
                return f"{self.x(a, 100)}.{f.name}", 100
        prec = self.PREC[op]
        left, right = self.operand(a, b, prec, True), self.operand(b, a, prec, False)
        word = {"&&": "and", "||": "or"}.get(op, op)
        return f"{left} {word} {right}", prec

    def operand(self, e, other, prec, left: bool) -> str:
        if op_is_compare(prec) and e[0] == "num" and e[1] == "0" and self.is_pointer(other):
            return "null"
        text, p = self.expr(e)
        need = prec if left and prec != 6 else prec + 1
        return f"({text})" if p < need else text

    # --- statements ---
    def function(self, name: str, source: Path) -> Result:
        text = source.read_text(errors="replace")
        found = function_source(text, name)
        if found is None:
            raise Untranslatable(f"{name} is not C in {source.name}")
        head, params, body = found
        self.types = load_types(source)
        self.moby = self.types.structs.get(MOBY_STRUCT)
        self.used, self.scopes = set(), [{}]
        self.calls, self.unmapped = Counter(), Counter()
        self.pvar_struct = None
        self.stmt_unmapped = False
        self.pre = []
        self.no_hoist = False
        result = Result(name, str(source))
        self.result = result
        void = head.split()[-1] == "void" if head.split() else False
        # the one parameter: the moby, possibly through a struct of its own
        ptoks = lex(params)
        base, i = self.types.parse_type(ptoks, 0)
        _, ptr, _, _ = self.types.parse_declarator(ptoks, i)
        moby_t = CType(base.base, ptr)
        view = base.base != MOBY_STRUCT and base.base in self.types.structs
        self.declare("moby", moby_t, moby=True, view=view)
        pname = ptoks[i + ptr] if i + ptr < len(ptoks) else "moby"
        self.scopes[0][pname] = self.scopes[0]["moby"]
        parser = Parser(lex(body), self.types)
        parser.names.add(pname)
        stmts = parser.parse_block_body()
        self.parser = parser
        self.boxed = self.find_boxed(stmts)
        lines = self.block(stmts, 1)
        signature = f"func rc_update(moby: RcMobyData){'' if not void else ' -> void'}:"
        result.text = "\n".join([signature, *(lines or ["\tpass"])])
        result.pvar_struct = self.pvar_struct
        result.calls, result.unmapped_calls = self.calls, self.unmapped
        return result

    def find_boxed(self, stmts) -> set:
        """Local scalars whose address is taken: they become RcRef boxes."""
        found = set()
        decls = {}

        def walk(n):
            if isinstance(n, tuple):
                if n and n[0] == "decl":
                    for name, t, _ in n[1]:
                        decls[name] = t
                if n and n[0] == "un" and n[1] == "&" and isinstance(n[2], tuple) and n[2][0] == "id":
                    found.add(n[2][1])
                for c in n:
                    walk(c)
            elif isinstance(n, list):
                for c in n:
                    walk(c)

        walk(stmts)
        return {n for n in found if n in decls and not decls[n].dims
                and (decls[n].ptr or (decls[n].base not in self.types.structs and decls[n].base not in FLOAT_VECS))}

    def block(self, stmts, depth: int) -> list[str]:
        self.scopes.append({})
        out = []
        for s in stmts:
            out += self.stmt(s, depth)
        self.scopes.pop()
        return out

    def leaves(self, s) -> int:
        k = s[0]
        if k in ("block",):
            return sum(self.leaves(c) for c in s[1])
        if k == "if":
            return 1 + self.leaves(s[2]) + (self.leaves(s[3]) if s[3] else 0)
        if k == "switch":
            return 1 + sum(self.leaves(c) for c in s[2])
        if k in ("for", "while"):
            return 1 + self.leaves(s[-1])
        if k in ("empty", "case", "default"):
            return 0
        return 1

    def c_text(self, s) -> str:
        return self.parser.spans.get(id(s)) or ""

    def stmt(self, s, depth: int) -> list[str]:
        k = s[0]
        if k in ("empty",):
            return []
        pad = "\t" * depth
        if k == "raw":
            return self.fail(s, depth, s[2] and f"{s[2]}", raw=s[1])
        self.stmt_unmapped = False
        outer_pre, mine = self.pre, []
        self.pre = mine
        mark = len(self.result.untranslated)
        saved = (Counter(self.calls), Counter(self.unmapped), dict(self.scopes[-1]), set(self.used),
                 self.result.statements, self.result.unmapped_statements)
        try:
            lines = self.emit(s, depth)
            self.pre = outer_pre
            lines = [pad + p for p in mine] + lines
        except Untranslatable as e:
            self.pre = outer_pre
            self.calls, self.unmapped = saved[0], saved[1]
            self.scopes[-1].clear()
            self.scopes[-1].update(saved[2])
            del self.result.untranslated[mark:]
            self.result.statements, self.result.unmapped_statements = saved[4], saved[5]
            return self.fail(s, depth, str(e))
        if k not in ("block", "if", "switch", "for", "while", "case", "default"):
            self.result.statements += 1
            if self.stmt_unmapped:
                self.result.unmapped_statements += 1
        else:
            self.result.statements += 1
            if self.stmt_unmapped:
                self.result.unmapped_statements += 1
        return lines

    def fail(self, s, depth, why, raw=None) -> list[str]:
        n = max(self.leaves(s), 1) if s[0] != "raw" else 1
        text = raw or self.parser.spans.get(id(s), s[0])
        text = " ".join(text.split())
        self.result.statements += n
        self.result.untranslated.append((text, why))
        pad = "\t" * depth
        short = text if len(text) <= 150 else text[:147] + "..."
        return [f"{pad}pass  # not translated ({why}): {short}"]

    def body(self, s, depth) -> list[str]:
        inner = s[1] if s[0] == "block" else [s]
        lines = self.block(inner, depth)
        return lines or ["\t" * depth + "pass"]

    def emit(self, s, depth) -> list[str]:
        k = s[0]
        pad = "\t" * depth
        if k == "block":
            return self.block(s[1], depth)
        if k == "decl":
            return [pad + line for line in self.decl(s)]
        if k == "expr":
            return [pad + line for line in self.expr_stmt(s[1])]
        if k == "return":
            return [pad + ("return" if s[1] is None else f"return {self.x(s[1])}")]
        if k in ("break", "continue"):
            return [pad + k]
        if k == "if":
            return self.if_stmt(s, depth, "if")
        if k == "while":
            self.no_hoist = True
            try:
                head = f"{pad}while {self.x(s[1])}:"
            finally:
                self.no_hoist = False
            self.no_continue_check(s[2])
            return [head, *self.body(s[2], depth + 1)]
        if k == "for":
            return self.for_stmt(s, depth)
        if k == "switch":
            return self.switch_stmt(s, depth)
        raise Untranslatable(k)

    def no_continue_check(self, s):
        pass

    def if_stmt(self, s, depth, word) -> list[str]:
        pad = "\t" * depth
        _, cond, then, other = s
        out = [f"{pad}{word} {self.x(cond)}:", *self.body(then, depth + 1)]
        if other is not None:
            if other[0] == "if":
                self.result.statements += 1
                self.no_hoist = True
                try:
                    out += self.if_stmt(other, depth, "elif")
                finally:
                    self.no_hoist = False
            else:
                inner = self.body(other, depth + 1)
                out += [f"{pad}else:", *inner]
        return out

    def for_stmt(self, s, depth) -> list[str]:
        pad = "\t" * depth
        _, init, cond, step, body = s
        r = self.range_loop(init, cond, step)
        if r is None:
            raise Untranslatable("a for loop that is not a counting loop")
        name, rng = r
        self.scopes.append({})  # a loop variable of its own, as GDScript does not reuse a declared one
        var = self.declare(name, CType("s32"))
        text = f"{pad}for {var.gd} in {rng}:"
        try:
            return [text, *self.body(body, depth + 1)]
        finally:
            self.scopes.pop()

    def range_loop(self, init, cond, step):
        if not (init and init[0] == "assign" and init[1] == "=" and init[2][0] == "id" and cond and cond[0] == "bin"
                and cond[2] == init[2] and step):
            return None
        name, op = init[2][1], cond[1]
        start, stop = self.x(init[3]), self.x(cond[3])
        inc = (step[0] in ("post",) or (step[0] == "un" and step[1] in ("++", "--"))) and step[-1] == init[2] or None
        if step[0] == "assign" and step[2] == init[2] and step[1] in ("+=", "-=") and step[3][0] == "num":
            by = self.number(step[3][1])
            sign = 1 if step[1] == "+=" else -1
        elif inc:
            by, sign = "1", 1 if (step[1] if step[0] in ("post", "un") else "") == "++" else -1
        else:
            return None
        if sign == 1 and op in ("<", "<="):
            stop = stop if op == "<" else f"{stop} + 1"
            return name, f"range({start}, {stop})" if by == "1" else f"range({start}, {stop}, {by})"
        if sign == -1 and op in (">", ">="):
            stop = stop if op == ">" else f"{stop} - 1"
            return name, f"range({start}, {stop}, -{by})"
        return None

    def switch_stmt(self, s, depth) -> list[str]:
        pad = "\t" * depth
        _, subject, flat = s
        cases, cur, labels = [], None, []
        for n in flat:
            if n[0] in ("case", "default"):
                if cur is not None and cur[1]:
                    cases.append(cur)
                    cur = None
                if cur is None:
                    cur = ([], [])
                cur[0].append(n)
            else:
                if cur is None:
                    raise Untranslatable("statements before the first case")
                cur[1].append(n)
        if cur is not None:
            cases.append(cur)
        groups = []
        for i, (labs, stmts) in enumerate(cases):
            while len(stmts) == 1 and stmts[0][0] == "block":
                stmts = stmts[0][1]
            body = strip_break(stmts)
            if body is None and i != len(cases) - 1 and not ends_flow(stmts):
                raise Untranslatable("a case that falls through")
            groups.append((labs, body if body is not None else stmts))
        if any(contains(b, "continue") for _, b in groups):
            raise Untranslatable("continue inside a switch")
        numeric = all(l[0] == "default" or l[1][0] == "num" for labs, _ in groups for l in labs)
        inner = any(contains(b, "break") for _, b in groups)
        out = []
        if numeric and not inner:
            out.append(f"{pad}match {self.x(subject)}:")
            for labs, b in groups:
                pattern = ", ".join("_" if l[0] == "default" else self.number(l[1][1]) for l in labs)
                lines = self.block(b, depth + 2)
                out += [f"{pad}\t{pattern}:", *(lines or [f"{pad}\t\tpass"])]
            return out
        out.append(f"{pad}for _sw in 1:  # C switch: a break leaves this one-pass loop")
        sv = f"_sv{depth}"
        out.append(f"{pad}\tvar {sv} = {self.x(subject)}")
        first = True
        default = None
        for labs, b in groups:
            if any(l[0] == "default" for l in labs):
                default = (labs, b)
                if len(labs) == 1:
                    continue
            cond = " or ".join(f"{sv} == {self.x(l[1])}" for l in labs if l[0] == "case")
            lines = self.block(b, depth + 2)
            out += [f"{pad}\t{'if' if first else 'elif'} {cond}:", *(lines or [f"{pad}\t\tpass"])]
            first = False
        if default is not None:
            lines = self.block(default[1], depth + 2)
            out += [f"{pad}\telse:" if not first else f"{pad}\tif true:", *(lines or [f"{pad}\t\tpass"])]
        return out

    def decl(self, s) -> list[str]:
        out = []
        for name, t, init in s[1]:
            boxed = name in self.boxed
            if init is not None and init[0] == "brace":
                if not (t.dims and len(t.dims) == 1 and t.base in ("f32", "f64", *FLOAT_VECS) and not t.ptr
                        or t.base in FLOAT_VECS) or any(i[0] == "brace" for i in init[1]):
                    raise Untranslatable("an initializer list")
                n = t.dims[0] if t.dims else 4
                vals = [self.x(i) for i in init[1]] + ["0.0"] * n
                value = f"PackedFloat32Array([{', '.join(vals[:n])}])"
            else:
                value = self.x(init) if init is not None else None
            moby = view = False
            if init is not None and t.ptr and self.is_moby_value(init):
                moby = True
                view = t.base != MOBY_STRUCT
            pv = init is not None and t.ptr and self.is_pvars_init(init)
            if pv and self.pvar_struct is None:
                self.pvar_struct = t.base
            var = self.declare(name, t, boxed=boxed, moby=moby, view=view)
            if boxed:
                out.append(f"var {var.gd} := RcRef.new()")
                if value is not None:
                    out.append(f"{var.gd}.v = {value}")
            elif value is not None:
                out.append(f"var {var.gd} = {value}")
            else:
                out.append(f"var {var.gd}{': float' if t.is_float else ''} = {default_expr(self.types, t)}")
        return out

    def is_pvars_init(self, e) -> bool:
        while e[0] in ("paren", "cast"):
            e = e[-1] if e[0] == "cast" else e[1]
        if e[0] != "member":
            return False
        obj = e[1]
        if not self.is_moby_value(obj):
            return False
        s = self.struct_of(obj)
        if s is None:
            return False
        f = s.named(e[2])
        return bool(f and f.offset == 0x78 and f.ctype.ptr)

    def lhs(self, e) -> str:
        if e[0] in ("id", "member", "index") or (e[0] == "un" and e[1] == "*"):
            return self.x(e)
        if e[0] == "paren":
            return self.lhs(e[1])
        raise Untranslatable("assignment target")

    def expr_stmt(self, e) -> list[str]:
        k = e[0]
        if k == "comma":
            return [l for x in e[1] for l in self.expr_stmt(x)]
        if k == "paren":
            return self.expr_stmt(e[1])
        if k == "assign":
            _, op, lhs, rhs = e
            t = self.ty(lhs)
            target = self.lhs(lhs)
            if t is not None and not t.ptr and (t.dims or t.base in FLOAT_VECS or t.base in self.types.structs):
                if op != "=":
                    raise Untranslatable("compound assignment of a struct")
                return [f"RcEngine.qcopy({target}, {self.x(rhs)})"]
            if self.contains_assign(rhs):
                raise Untranslatable("assignment inside an expression")
            if op == "=" and t is not None and t.ptr and lhs[0] == "id" and self.is_pvars_init(rhs):
                if self.pvar_struct is None:
                    self.pvar_struct = t.base
            mask = MASKS.get(t.base) if t is not None and not t.ptr and not t.dims else None
            value = self.x(rhs)
            if t is not None and t.ptr and rhs[0] == "num" and rhs[1] == "0":
                value = "null"
            if op == "=":
                if mask and rhs[0] != "num":
                    return [f"{target} = ({value}) & {mask:#x}"]
                return [f"{target} = {value}"]
            inner = f"{target} {op[:-1]} {self.x(rhs, self.PREC.get(op[:-1], 0) + 1)}"
            if mask:
                return [f"{target} = ({inner}) & {mask:#x}"]
            return [f"{target} {op} {value}"]
        if k in ("post", "un") and (e[1] in ("++", "--")):
            inner = e[2]
            t = self.ty(inner)
            target = self.lhs(inner)
            mask = MASKS.get(t.base) if t is not None and not t.ptr and not t.dims else None
            sign = "+" if e[1] == "++" else "-"
            if mask:
                return [f"{target} = ({target} {sign} 1) & {mask:#x}"]
            return [f"{target} {sign}= 1"]
        if k == "call":
            return [self.x(e)]
        raise Untranslatable(f"{k} as a statement")

    def contains_assign(self, e) -> bool:
        if isinstance(e, tuple):
            if e and e[0] == "assign" or (e and e[0] in ("post", "un") and e[1] in ("++", "--")):
                return True
            return any(self.contains_assign(c) for c in e)
        if isinstance(e, list):
            return any(self.contains_assign(c) for c in e)
        return False


def op_is_compare(prec: int) -> bool:
    return prec == 6


def strip_break(stmts):
    """The case body without its final break, or None when it does not end in one."""
    stmts = list(stmts)
    while stmts and stmts[-1][0] == "block":
        inner = strip_break(stmts[-1][1])
        if inner is None:
            return None if not ends_flow(stmts) else stmts
        stmts = stmts[:-1] + inner
        return stmts
    if stmts and stmts[-1][0] == "break":
        return stmts[:-1]
    return None


def ends_flow(stmts) -> bool:
    last = stmts[-1] if stmts else None
    if last is None:
        return False
    if last[0] in ("return", "continue", "break"):
        return True
    if last[0] == "block":
        return ends_flow(last[1])
    return False


def contains(stmts, kind: str) -> bool:
    """True if a KIND statement is in STMTS outside nested loops/switches (break) or outside nested loops (continue)."""
    for s in stmts:
        if s[0] == kind:
            return True
        if s[0] == "block" and contains(s[1], kind):
            return True
        if s[0] == "if" and (contains([s[2]], kind) or (s[3] and contains([s[3]], kind))):
            return True
        if kind == "continue" and s[0] == "switch" and contains(s[2], kind):
            return True
    return False


# --- generated files ---------------------------------------------------------------------------------------------

GD_TYPES = {"f32": "float", "f64": "float", "u8": "int", "s8": "int", "u16": "int", "s16": "int", "u32": "int",
            "s32": "int", "u64": "int", "s64": "int"}
MAX_EXPORT_ARRAY = 16


def gd_field_decl(types: Types, f, export: bool, name: str) -> list[str]:
    """`var`/`@export var` lines for one struct field (a pvar or a moby header field)."""
    t = f.ctype
    default = default_expr(types, t)
    doc = f"## +{f.offset:#x} {c_type_text(t)}"
    if t.ptr or t.base not in GD_TYPES and t.base not in FLOAT_VECS or (t.dims and len(t.dims) > 1):
        return [doc, f"var {name} = {default}"]
    if t.dims or t.base in FLOAT_VECS:
        kind = "PackedFloat32Array" if t.base in ("f32", "f64") or t.base in FLOAT_VECS else "PackedInt32Array"
        small = t.base in FLOAT_VECS or (t.dims and t.dims[0] <= MAX_EXPORT_ARRAY)
        return [doc, f"{'@export ' if export and small else ''}var {name}: {kind} = {default}"]
    return [doc, f"{'@export ' if export else ''}var {name}: {GD_TYPES[t.base]} = {default}"]


def c_type_text(t: CType) -> str:
    return t.base + "*" * t.ptr + "".join(f"[{d}]" for d in t.dims)


def moby_data_script(types: Types | None = None) -> str:
    """rc1/rc_moby_data.gd: the moby header record, from include/moby.h."""
    types = types or load_types(None)
    moby = types.structs[MOBY_STRUCT]
    out = ["class_name RcMobyData", "extends RefCounted",
           "## The moby's 0x100-byte header record as the translated update functions see it:",
           "## the fields of struct Moby in games/rac1/include/moby.h, with their offsets.",
           "## Generated by `editor/behaviour.py data`; edit the header, not this file.", "",
           "# =============================================================================",
           "# VARIABLES", "# =============================================================================", ""]
    for f in moby.fields:
        if f.name.startswith("pad") or f.name == "update":
            continue
        if f.name == "pvars":
            out += ["## +0x74 The class's private variables: the node carrying the class script.", "var pvars: Object = null", ""]
            continue
        lines = gd_field_decl(types, f, False, f.name)
        if f.name == "pclass":  # a class record the preview can read fields of
            lines[-1] = "var pclass = RcStruct.new()"
        out += [*lines, ""]
    return "\n".join(out).rstrip() + "\n"


def find_source(function: str, hint: str | None) -> tuple[Path, str] | None:
    """The file holding C for FUNCTION, the decompilation's own else the port's functional C, and
    the name it is defined by there: its func_ symbol or its readable name (names.h)."""
    names = [function] + [name for name, symbol in load_macros().items() if symbol == function]
    for path in ([REPO / hint] if hint else []) + [DECOMPILATION / "nonmatching" / "functional" / f"{function}.c"]:
        if not path.is_file():
            continue
        text = path.read_text(errors="replace")
        for name in names:
            if function_source(text, name):
                return path, name
    return None


def translate_class(class_id: int, level: int) -> tuple[Result, dict] | None:
    logic = moby_logic(level).get(class_id)
    if logic is None:
        return None
    found = find_source(logic["function"], logic["file"])
    if found is None:
        return None
    source, name = found
    tr = Translator(load_types(source), load_api(), load_macros())
    return tr.function(name, source), logic


def class_script(class_id: int, level: int) -> tuple[str, Result] | None:
    """The .gd for a moby class, or None when its update function is not C."""
    done = translate_class(class_id, level)
    if done is None:
        return None
    result, logic = done
    name = moby_class_names().get(class_id)
    types = load_types(Path(result.source))
    where = Path(result.source).relative_to(REPO).as_posix()
    out = ["extends RcMoby",
           f"## Moby class {class_id}" + (f": {name}" if name else "") + ".",
           f"## Updated every frame by {logic['function']} ({where}).",
           f"## Translated from that C by editor/behaviour.py: {result.summary()}.",
           "## The native port runs the C, not this script. This is for reading and for the preview",
           "## (set rc_preview on the node), against the RcEngine stub in rc1/rc_engine.gd."]
    for text, why in result.untranslated:
        out.append("## Not translated: " + (text if len(text) <= 100 else text[:97] + "..."))
        out.append(f"##   because {why}")
    out += ["", "# =============================================================================",
            "# VARIABLES", "# =============================================================================", ""]
    struct = types.structs.get(result.pvar_struct or "")
    if struct is not None:
        out.append(f"# --- Private variables (pvars: struct {struct.name}, {struct.size:#x} bytes) ---")
        out.append("")
        for f in struct.fields:
            if f.name.startswith("pad") or f.size == 0:
                continue
            out += [*gd_field_decl(types, f, True, export_name(f.name)), ""]
    else:
        out += ["# This function reads no pvars, or through a layout this tool did not read.", ""]
    out += ["", "# =============================================================================",
            "# METHODS", "# =============================================================================", "",
            "## One game frame of this class's behaviour.",
            result.text.replace("func rc_update(moby: RcMobyData)", "func rc_update(moby: RcMobyData)", 1)]
    return "\n".join(out).rstrip() + "\n", result


def attach(level_dir: Path, class_id: int, script_res: str) -> None:
    """Point the class scene's root node at the script (idempotent)."""
    path = level_dir / "mobys" / f"moby_{class_id}.tscn"
    text = path.read_text()
    if "id=\"behaviour\"" in text:
        return
    ext = f'[ext_resource type="Script" path="{script_res}" id="behaviour"]\n'
    lines = text.split("\n")
    last_ext = max((i for i, l in enumerate(lines) if l.startswith("[ext_resource")), default=0)
    lines.insert(last_ext + 1, ext.rstrip("\n"))
    node = next(i for i, l in enumerate(lines) if l.startswith("[node name="))
    lines.insert(node + 1, 'script = ExtResource("behaviour")')
    path.write_text("\n".join(lines))


def export(project: Path, level: int, classes: list[int]) -> list[Result]:
    """Write and attach the scripts for CLASSES into the exported level project at PROJECT."""
    level_dir = project / "levels" / f"level_{level:02}"
    out = []
    for class_id in classes:
        made = class_script(class_id, level)
        if made is None:
            print(f"class {class_id}: no C update function in level {level}", file=sys.stderr)
            continue
        text, result = made
        target = level_dir / "mobys" / "behaviour" / f"moby_{class_id}.gd"
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_text(text)
        attach(level_dir, class_id, f"res://levels/level_{level:02}/mobys/behaviour/moby_{class_id}.gd")
        out.append(result)
        print(result.summary())
    return out


def census() -> str:
    """How much of the update-function corpus is C, how it is written and which helpers it calls."""
    table = DECOMPILATION / "config" / "overlays" / "names.tsv"
    roles = {}
    for line in table.read_text().splitlines():
        if line.startswith("#") or "\t" not in line:
            continue
        function, r = line.split("\t", 1)
        classes = re.findall(r"UpdateMoby_(\d+):", r)
        if classes:
            roles[function] = classes
    macros, api = load_macros(), load_api()
    reverse = {v: k for k, v in macros.items()}
    calls, functions, raw_style, typed_style, with_c = Counter(), Counter(), 0, 0, 0
    sizes = []
    files = list((DECOMPILATION / "src").rglob("*.c")) + list((DECOMPILATION / "nonmatching" / "functional").glob("*.c"))
    texts = {p: p.read_text(errors="replace") for p in files}
    aliases = {}
    for text in texts.values():
        aliases.update(re.findall(r'\b(\w+)\s*(?:\([^;{}]*\)|\[\])?\s*__asm__\s*\(\s*"(\w+)"\s*\)', text))
    for function in roles:
        found = next((function_source(t, function) for t in texts.values() if function in t and function_source(t, function)), None)
        if not found:
            continue
        with_c += 1
        body = found[2]
        sizes.append(body.count("\n"))
        body = re.sub(r"\b([A-Za-z_]\w*)(\s*\()", lambda m: aliases.get(m.group(1), m.group(1)) + m.group(2), body)
        names = set(re.findall(r"\b([A-Za-z_]\w*)\s*\(", body)) - {"if", "while", "for", "switch", "return", "sizeof"}
        if re.search(r"\*\s*\(\s*(?:unsigned\s+)?\w+\s*\*\s*\)\s*\(", body):
            raw_style += 1
        else:
            typed_style += 1
        for n in re.findall(r"\b([A-Za-z_]\w*)\s*\(", body):
            if n in names:
                calls[n] += 1
        for n in names:
            functions[n] += 1
    lines = [f"{len(roles)} update functions ({sum(len(c) for c in roles.values())} class roles), {with_c} are C in the checkout",
             f"{raw_style} use raw *(T *)(p + off) access somewhere, {typed_style} only typed struct access",
             "helper  functions-calling-it  calls  api-name"]
    for n, k in functions.most_common(60):
        sym = macros.get(n, n)
        a = api.get(n) or api.get(sym)
        if n.startswith("func_") or n in macros or a:
            lines.append(f"{n:34} {k:4} {calls[n]:5}  {a.gd if a else '-'}")
    return "\n".join(lines)


def corpus() -> str:
    """Run the translator over every update function that is C and report how much it translated."""
    table = DECOMPILATION / "config" / "overlays" / "names.tsv"
    functions = []
    for line in table.read_text().splitlines():
        if not line.startswith("#") and "\t" in line:
            f, r = line.split("\t", 1)
            if "UpdateMoby_" in r:
                functions.append(f)
    files = list((DECOMPILATION / "src").rglob("*.c")) + list((DECOMPILATION / "nonmatching" / "functional").glob("*.c"))
    where = {}
    for p in files:
        text = p.read_text(errors="replace")
        for f in functions:
            if f not in where and f in text and function_source(text, f):
                where[f] = p
    api, macros = load_api(), load_macros()
    total = done = 0
    buckets = Counter()
    reasons = Counter()
    failed = []
    cache = {}
    for f in functions:
        if f not in where:
            continue
        try:
            if where[f] not in cache:
                cache[where[f]] = load_types(where[f])
            res = Translator(cache[where[f]], api, macros).function(f, where[f])
        except Exception as e:  # a function the tool cannot even start on
            failed.append((f, f"{type(e).__name__}: {e}"))
            buckets["tool failed"] += 1
            continue
        total += res.statements
        done += res.translated
        share = res.translated / max(res.statements, 1)
        buckets["100%" if share == 1 else ">=90%" if share >= .9 else ">=50%" if share >= .5 else "<50%"] += 1
        for _, why in res.untranslated:
            reasons[re.sub(r"[\w.]+ \(offset.*|\(.*", "", why).strip()[:60] or why[:60]] += 1
    lines = [f"{len(where)} update functions are C; {total} statements, {done} translated ({100 * done / max(total, 1):.1f}%)",
             "functions by share translated: " + ", ".join(f"{k}: {v}" for k, v in sorted(buckets.items())),
             "why statements were not translated:"]
    lines += [f"  {n:5}  {why}" for why, n in reasons.most_common(15)]
    lines += [f"  tool failed on {f}: {why}" for f, why in failed[:8]]
    return "\n".join(lines)


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    sub = ap.add_subparsers(dest="command", required=True)
    e = sub.add_parser("export", help="write and attach class scripts in an exported project")
    e.add_argument("project", type=Path)
    e.add_argument("--level", type=int, default=0)
    e.add_argument("--classes", default="357,749,500")
    s = sub.add_parser("show", help="print one class's script")
    s.add_argument("class_id", type=int)
    s.add_argument("--level", type=int, default=0)
    sub.add_parser("census", help="what the update functions call")
    sub.add_parser("corpus", help="translate every C update function and report the share translated")
    sub.add_parser("data", help="regenerate rc1/rc_moby_data.gd")
    args = ap.parse_args()
    if args.command == "export":
        export(args.project, args.level, [int(c) for c in args.classes.split(",")])
    elif args.command == "show":
        made = class_script(args.class_id, args.level)
        print(made[0] if made else "no C update function", end="")
    elif args.command == "census":
        print(census())
    elif args.command == "corpus":
        print(corpus())
    else:
        (SCRIPTS / "rc_moby_data.gd").write_text(moby_data_script())


if __name__ == "__main__":
    main()
