"""A small reader for the C the decompilation is written in: tokens and struct layouts, enough
for the behaviour catalogue (sdk/tools/catalogue.py, which imports it from this folder) and the
pvar fields of the level editor (gamedata.py) to know each struct's fields, offsets and types. It is
not a C compiler.
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
