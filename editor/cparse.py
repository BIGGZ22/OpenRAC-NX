"""A small parser for the C the decompilation is written in, enough to translate
a moby update function (behaviour.py): tokens, struct layouts and statements.

It is not a C compiler. It reads the statement forms those functions use (blocks,
declarations, if/else, switch, for, while, return, break, expression statements)
and the C expression grammar. A statement it cannot read becomes a ("raw", text)
node, so one odd line costs one statement, not the function.
"""

import re
from dataclasses import dataclass, field

TOKEN = re.compile(r"""
    (?P<num>0[xX][0-9a-fA-F]+[uUlL]*|(?:\d+\.\d*|\.\d+)(?:[eE][-+]?\d+)?[fF]?|\d+[eE][-+]?\d+[fF]?|\d+[uUlL]*)
  | (?P<id>[A-Za-z_]\w*)
  | (?P<str>"(?:\\.|[^"\\])*")
  | (?P<chr>'(?:\\.|[^'\\])+')
  | (?P<op><<=|>>=|\.\.\.|->|\+\+|--|<<|>>|<=|>=|==|!=|&&|\|\||[-+*/%&|^]=|[-+*/%&|^~!<>=?:;,.(){}\[\]])
""", re.X)

BASE = {"u8": ("u8", 1), "s8": ("s8", 1), "char": ("s8", 1), "u16": ("u16", 2), "s16": ("s16", 2), "short": ("s16", 2),
        "u32": ("u32", 4), "s32": ("s32", 4), "int": ("s32", 4), "f32": ("f32", 4), "float": ("f32", 4),
        "u64": ("u64", 8), "s64": ("s64", 8), "f64": ("f64", 8), "double": ("f64", 8), "void": ("void", 1)}
TYPE_WORDS = {"const", "volatile", "unsigned", "signed", "struct", "union", "enum", "short", "long", "int", "char",
              "float", "double", "void", "register", "static", "extern", "inline", "__inline__"}
STORAGE = {"register", "static", "extern", "inline", "__inline__", "const", "volatile"}


def strip(text: str) -> str:
    """Comments, preprocessor lines and compiler attributes removed."""
    text = re.sub(r"/\*.*?\*/", " ", text, flags=re.S)
    text = re.sub(r"//[^\n]*", " ", text)
    text = re.sub(r"^[ \t]*#(?:[^\n\\]|\\\n)*", " ", text, flags=re.M)
    text = re.sub(r"__attribute__\s*\(\((?:[^()]|\([^()]*\))*\)\)", " ", text)
    text = re.sub(r"__asm__\s*\([^)]*\)", " ", text)
    return re.sub(r"\b(?:SDATA|SBSS)\s*\([^)]*\)", " ", text)


def lex(text: str) -> list[str]:
    return [m.group(0) for m in TOKEN.finditer(strip(text))]


@dataclass(frozen=True)
class CType:
    """A C type: base ('u8', 'f32', 'void' or a struct name), pointer depth and array dimensions."""
    base: str
    ptr: int = 0
    dims: tuple = ()

    @property
    def is_pointer(self):
        return self.ptr > 0

    @property
    def is_float(self):
        return self.base in ("f32", "f64") and not self.ptr and not self.dims

    def deref(self):
        if self.dims:
            return CType(self.base, self.ptr, self.dims[1:])
        return CType(self.base, max(self.ptr - 1, 0))


@dataclass
class Field:
    name: str
    ctype: CType
    offset: int
    size: int


@dataclass
class Struct:
    name: str
    fields: list = field(default_factory=list)
    size: int = 0
    align: int = 1
    exact: bool = True  # False once a member of unknown size (a nested union) was met

    def at(self, offset: int):
        return next((f for f in self.fields if f.offset == offset), None)

    def named(self, name: str):
        return next((f for f in self.fields if f.name == name), None)


class Types:
    """Struct layouts and typedefs read from C text."""

    def __init__(self):
        self.structs: dict[str, Struct] = {}
        self.aliases: dict[str, CType] = {}
        self.symbols: dict[str, str] = {}  # `extern f_alias(...) __asm__("f")`: alias -> real symbol

    def size_align(self, t: CType) -> tuple[int, int]:
        count = 1
        for d in t.dims:
            count *= d
        if t.ptr:
            return 4 * count, 4
        if t.base in self.structs:
            s = self.structs[t.base]
            return s.size * count, s.align
        size = next((n for b, n in BASE.values() if b == t.base), 4)
        return size * count, min(size, 8)

    def resolve(self, name: str) -> CType:
        return self.aliases.get(name, CType(name))

    def is_type_name(self, tok: str) -> bool:
        return tok in TYPE_WORDS or tok in BASE or tok in self.structs or tok in self.aliases

    def read(self, text: str) -> None:
        """Add the typedefs and structs in TEXT."""
        for alias, real in re.findall(r'\b(\w+)\s*(?:\([^;{}]*\)|\[\])?\s*__asm__\s*\(\s*"(\w+)"\s*\)', text):
            self.symbols[alias] = real
        toks = lex(text)
        i = 0
        while i < len(toks):
            if toks[i] == "typedef":
                i = self._typedef(toks, i + 1)
            elif toks[i] in ("struct", "union") and i + 2 < len(toks) and toks[i + 2] == "{":
                i = self._struct_body(toks, i, toks[i + 1])
            else:
                i += 1

    def _typedef(self, toks, i):
        if toks[i] in ("struct", "union"):
            tag = toks[i + 1] if toks[i + 1] != "{" else None
            j = i + (2 if tag else 1)
            if j < len(toks) and toks[j] == "{":
                end = self._struct_body(toks, i, tag)
                names = []
                while end < len(toks) and toks[end] != ";":
                    if re.match(r"\w", toks[end]):
                        names.append(toks[end])
                    end += 1
                s = self.structs.get(tag or "") or self.structs.get(f"<anon@{i}>")
                for n in names:
                    if tag is None and s is not None:
                        s.name = n
                        self.structs[n] = s
                    elif s is not None:
                        self.structs[n] = s
                self.structs.pop(f"<anon@{i}>", None)
                return end + 1
        j = i
        while j < len(toks) and toks[j] != ";":
            j += 1
        chunk = toks[i:j]
        if "(" in chunk and chunk.index("(") + 2 < len(chunk) and chunk[chunk.index("(") + 1] == "*":
            self.aliases[chunk[chunk.index("(") + 2]] = CType("void", 1)  # typedef R (*Name)(...)
        elif "(" not in chunk and len(chunk) >= 2:
            t, k = self.parse_type(chunk, 0)
            if k < len(chunk):
                name, ptr, dims = self.parse_declarator(chunk, k)[:3]
                self.aliases[name] = CType(t.base, t.ptr + ptr, dims or t.dims)
        return j + 1

    def _struct_body(self, toks, i, tag):
        """Parse struct/union TOKS[i] ... {...}; return the index after the closing brace."""
        union = toks[i] == "union"
        j = i + 1 + (1 if toks[i + 1] != "{" else 0)
        assert toks[j] == "{"
        j += 1
        s = Struct(tag or f"<anon@{i}>")
        off = 0
        while j < len(toks) and toks[j] != "}":
            if toks[j] in ("struct", "union") and (toks[j + 1] == "{" or (j + 2 < len(toks) and toks[j + 2] == "{")):
                nested_tag = toks[j + 1] if toks[j + 1] != "{" else None
                j2 = self._struct_body(toks, j, nested_tag)
                inner = self.structs.get(nested_tag or f"<anon@{j}>")
                names = []
                while toks[j2] != ";":
                    if re.match(r"\w", toks[j2]):
                        names.append(toks[j2])
                    j2 += 1
                j = j2 + 1
                self.structs.pop(f"<anon@{j}>", None)
                size, align = (inner.size, inner.align) if inner else (0, 1)
                for n in names or [None]:
                    off = (off + align - 1) // align * align
                    if n:
                        s.fields.append(Field(n, CType(inner.name if inner else "void"), off, size))
                    off = max(off, off + size) if not union else 0
                    s.size = max(s.size, off + (size if union else 0))
                    s.align = max(s.align, align)
                continue
            k = j
            while toks[k] != ";":
                k += 1
            decl = toks[j:k]
            j = k + 1
            if not decl:
                continue
            if "(" in decl:  # a function pointer member: `void (*update)(struct Moby *)`
                k = decl.index("(")
                if k + 2 < len(decl) and decl[k + 1] == "*":
                    off = (off + 3) // 4 * 4
                    s.fields.append(Field(decl[k + 2], CType("void", 1), off, 4))
                    off += 4
                    s.align = max(s.align, 4)
                continue
            base, m = self.parse_type(decl, 0)
            while m < len(decl):
                name, ptr, dims, m = self.parse_declarator(decl, m)
                if m < len(decl) and decl[m] == ":":  # bit field: not modelled
                    s.exact = False
                    break
                t = CType(base.base, base.ptr + ptr, dims or base.dims)
                size, align = self.size_align(t)
                if not union:
                    off = (off + align - 1) // align * align
                s.fields.append(Field(name, t, 0 if union else off, size))
                if union:
                    s.size = max(s.size, size)
                else:
                    off += size
                s.align = max(s.align, align)
                if m < len(decl) and decl[m] == ",":
                    m += 1
        if not union:
            s.size = (off + s.align - 1) // s.align * s.align
        s.size = (s.size + s.align - 1) // s.align * s.align
        self.structs[s.name] = s
        return j + 1

    def parse_type(self, toks, i) -> tuple[CType, int]:
        """The type specifier at TOKS[i:] (no declarator): (CType without pointer, next index)."""
        unsigned, long_count, base = False, 0, None
        while i < len(toks):
            t = toks[i]
            if t in STORAGE:
                i += 1
            elif t == "unsigned":
                unsigned = True
                i += 1
            elif t == "signed":
                i += 1
            elif t in ("struct", "union", "enum"):
                base = CType(toks[i + 1]) if t != "enum" else CType("s32")
                i += 2
            elif t == "long":
                long_count += 1
                i += 1
            elif t in BASE and base is None:
                base = CType(BASE[t][0])
                i += 1
            elif t in self.aliases and base is None:
                base = self.aliases[t]
                i += 1
            elif t in self.structs and base is None:
                base = CType(self.structs[t].name if t in self.structs else t)
                i += 1
            else:
                break
        if long_count and (base is None or base.base == "s32"):
            base = CType("s64")
        if base is None:
            base = CType("s32") if (unsigned or long_count) else None
        if base is None:
            raise ParseError(f"no type at {toks[i:i + 3]}")
        if unsigned and not base.ptr:
            base = CType({"s8": "u8", "s16": "u16", "s32": "u32", "s64": "u64"}.get(base.base, base.base), base.ptr, base.dims)
        return base, i

    def parse_declarator(self, toks, i):
        ptr = 0
        while i < len(toks) and toks[i] in ("*", "const", "volatile"):
            ptr += toks[i] == "*"
            i += 1
        name = toks[i]
        i += 1
        dims = []
        while i < len(toks) and toks[i] == "[":
            j = i + 1
            while toks[j] != "]":
                j += 1
            dims.append(self.const_int(toks[i + 1:j]))
            i = j + 1
        return name, ptr, tuple(dims), i

    def const_int(self, toks) -> int:
        """A constant array dimension such as 0x138 - 0x70 or 4 (1 when it is not constant)."""
        try:
            return int(eval(" ".join(re.sub(r"[uUlL]+$", "", t) for t in toks), {"__builtins__": {}}))
        except Exception:
            return 1


class ParseError(Exception):
    pass


# --- statements and expressions ---------------------------------------------------------------------------------

BIN_PREC = {"*": 12, "/": 12, "%": 12, "+": 11, "-": 11, "<<": 10, ">>": 10, "<": 9, ">": 9, "<=": 9, ">=": 9,
            "==": 8, "!=": 8, "&": 7, "^": 6, "|": 5, "&&": 4, "||": 3}
ASSIGN = {"=", "+=", "-=", "*=", "/=", "%=", "&=", "|=", "^=", "<<=", ">>="}


class Parser:
    """Statements and expressions out of a token list. Expressions are tuples:
    ("num", text) ("id", name) ("str", text) ("member", obj, field) ("index", a, i) ("call", f, args)
    ("un", op, e) ("post", op, e) ("bin", op, a, b) ("assign", op, lhs, rhs) ("cond", c, a, b)
    ("cast", CType, e) ("sizeof", text) ("comma", [e]). Statements:
    ("block", [s]) ("decl", [(name, CType, init)]) ("expr", e) ("if", c, t, f) ("switch", e, [s])
    ("case", e) ("default",) ("break",) ("continue",) ("return", e) ("for", init, cond, step, body)
    ("while", c, body) ("raw", text) ("empty",)."""

    def __init__(self, toks, types: Types, locals_: set | None = None):
        self.t, self.i, self.types = toks, 0, types
        self.names = set(locals_ or ())
        self.spans: dict[int, str] = {}  # id(statement) -> its tokens as text

    # token helpers
    def peek(self, k=0):
        return self.t[self.i + k] if self.i + k < len(self.t) else ""

    def take(self, want=None):
        tok = self.peek()
        if want is not None and tok != want:
            raise ParseError(f"expected {want!r}, got {tok!r} near {' '.join(self.t[max(self.i - 4, 0):self.i + 4])}")
        self.i += 1
        return tok

    def skip_statement(self, start):
        """Recover: drop tokens from START to the end of the statement and return them as text."""
        self.i = start
        depth = 0
        while self.i < len(self.t):
            tok = self.take()
            if tok in "({[":
                depth += 1
            elif tok in ")}]":
                depth -= 1
                if depth < 0:
                    self.i -= 1
                    break
                if depth == 0 and tok == "}" and self.peek() not in ("else", "while", ",", ";"):
                    break
            elif tok == ";" and depth == 0:
                break
        return " ".join(self.t[start:self.i])

    # statements
    def parse_block_body(self):
        out = []
        while self.peek() not in ("}", ""):
            out.append(self.statement())
        return out

    def statement(self):
        start = self.i
        try:
            node = self._statement()
        except (ParseError, IndexError, AssertionError) as e:
            node = ("raw", self.skip_statement(start), str(e))
        self.spans[id(node)] = " ".join(self.t[start:self.i])
        return node

    def is_decl_start(self):
        tok = self.peek()
        if tok in TYPE_WORDS and tok not in ("struct", "union") or tok in BASE:
            return True
        if tok in ("struct", "union", "enum"):
            return True
        return (tok in self.types.aliases or tok in self.types.structs) and tok not in self.names \
            and self.peek(1) not in ("(", "=", ".", "->", "[")

    def _statement(self):
        tok = self.peek()
        if tok == "{":
            self.take()
            body = self.parse_block_body()
            self.take("}")
            return ("block", body)
        if tok == ";":
            self.take()
            return ("empty",)
        if tok == "if":
            self.take()
            self.take("(")
            cond = self.expr()
            self.take(")")
            then = self.statement()
            other = None
            if self.peek() == "else":
                self.take()
                other = self.statement()
            return ("if", cond, then, other)
        if tok == "switch":
            self.take()
            self.take("(")
            cond = self.expr()
            self.take(")")
            self.take("{")
            body = self.parse_block_body()
            self.take("}")
            return ("switch", cond, body)
        if tok == "case":
            self.take()
            e = self.expr()
            self.take(":")
            return ("case", e)
        if tok == "default":
            self.take()
            self.take(":")
            return ("default",)
        if tok in ("break", "continue"):
            self.take()
            self.take(";")
            return (tok,)
        if tok == "return":
            self.take()
            e = None if self.peek() == ";" else self.expr()
            self.take(";")
            return ("return", e)
        if tok == "for":
            self.take()
            self.take("(")
            init = None if self.peek() == ";" else self.expr()
            self.take(";")
            cond = None if self.peek() == ";" else self.expr()
            self.take(";")
            step = None if self.peek() == ")" else self.expr()
            self.take(")")
            return ("for", init, cond, step, self.statement())
        if tok == "while":
            self.take()
            self.take("(")
            cond = self.expr()
            self.take(")")
            return ("while", cond, self.statement())
        if tok in ("do", "goto"):
            raise ParseError(f"{tok} is not translated")
        if self.is_decl_start():
            return self.declaration()
        e = self.expr()
        self.take(";")
        return ("expr", e)

    def declaration(self):
        base, _ = None, None
        j = self.i
        base, j = self.types.parse_type(self.t, j)
        self.i = j
        items = []
        while True:
            name, ptr, dims, self.i = self.types.parse_declarator(self.t, self.i)
            t = CType(base.base, base.ptr + ptr, dims or base.dims)
            init = None
            if self.peek() == "=":
                self.take()
                init = self.initializer()
            items.append((name, t, init))
            self.names.add(name)
            if self.peek() == ",":
                self.take()
                continue
            break
        self.take(";")
        return ("decl", items)

    def initializer(self):
        if self.peek() == "{":
            self.take()
            items = []
            while self.peek() != "}":
                items.append(self.initializer())
                if self.peek() == ",":
                    self.take()
            self.take("}")
            return ("brace", items)
        return self.assign_expr()

    # expressions
    def expr(self):
        e = self.assign_expr()
        if self.peek() == ",":
            items = [e]
            while self.peek() == ",":
                self.take()
                items.append(self.assign_expr())
            return ("comma", items)
        return e

    def assign_expr(self):
        lhs = self.cond_expr()
        if self.peek() in ASSIGN:
            op = self.take()
            return ("assign", op, lhs, self.assign_expr())
        return lhs

    def cond_expr(self):
        c = self.bin_expr(3)
        if self.peek() == "?":
            self.take()
            a = self.expr()
            self.take(":")
            return ("cond", c, a, self.cond_expr())
        return c

    def bin_expr(self, minimum):
        lhs = self.unary()
        while self.peek() in BIN_PREC and BIN_PREC[self.peek()] >= minimum:
            op = self.take()
            lhs = ("bin", op, lhs, self.bin_expr(BIN_PREC[op] + 1))
        return lhs

    def cast_ahead(self):
        """True when the tokens at self.i are ( type-name *... )."""
        if self.peek() != "(":
            return False
        tok = self.peek(1)
        return tok in TYPE_WORDS or tok in BASE or ((tok in self.types.structs or tok in self.types.aliases)
                                                    and tok not in self.names)

    def unary(self):
        tok = self.peek()
        if tok in ("-", "+", "!", "~", "*", "&"):
            self.take()
            return ("un", tok, self.unary())
        if tok in ("++", "--"):
            self.take()
            return ("un", tok, self.unary())
        if tok == "sizeof":
            self.take()
            start = self.i
            if self.peek() == "(":
                depth = 0
                while True:
                    t = self.take()
                    depth += (t == "(") - (t == ")")
                    if depth == 0:
                        break
            else:
                self.unary()
            return ("sizeof", " ".join(self.t[start:self.i]))
        if self.cast_ahead():
            self.take("(")
            base, self.i = self.types.parse_type(self.t, self.i)
            ptr = 0
            while self.peek() in ("*", "const"):
                ptr += self.take() == "*"
            self.take(")")
            if self.peek() == "{":
                raise ParseError("compound literal")
            return ("cast", CType(base.base, base.ptr + ptr, base.dims), self.unary())
        return self.postfix()

    def postfix(self):
        e = self.primary()
        while True:
            tok = self.peek()
            if tok == "(":
                self.take()
                args = []
                while self.peek() != ")":
                    args.append(self.assign_expr())
                    if self.peek() == ",":
                        self.take()
                self.take(")")
                e = ("call", e, args)
            elif tok == "[":
                self.take()
                idx = self.expr()
                self.take("]")
                e = ("index", e, idx)
            elif tok in (".", "->"):
                self.take()
                e = ("member", e, self.take(), tok)
            elif tok in ("++", "--"):
                self.take()
                e = ("post", tok, e)
            else:
                return e

    def primary(self):
        tok = self.take()
        if tok == "(":
            e = self.expr()
            self.take(")")
            return ("paren", e)
        if re.match(r"\d|\.\d", tok):
            return ("num", tok)
        if tok[0] in "\"'":
            return ("str", tok)
        if re.match(r"[A-Za-z_]", tok):
            return ("id", tok)
        raise ParseError(f"unexpected {tok!r}")


def function_source(text: str, name: str) -> tuple[str, str, str] | None:
    """(signature tokens text, body text, return type text) of the C function NAME in TEXT, or None."""
    clean = strip(text)
    for m in re.finditer(rf"(?<![\w])((?:[A-Za-z_][\w\s\*]*?))\b{re.escape(name)}\s*\(([^;{{}}]*?)\)\s*\{{", clean):
        i, depth = m.end(), 1
        while depth and i < len(clean):
            depth += (clean[i] == "{") - (clean[i] == "}")
            i += 1
        return m.group(1).strip(), m.group(2).strip(), clean[m.end():i - 1]
    return None
