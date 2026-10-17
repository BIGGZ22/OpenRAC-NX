# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""C types as Clang prints them, parsed, and printed back for the host.

Clang's JSON AST gives every type as a string in C declarator syntax
("int (*)(char *, float)", "S [4]", "struct (unnamed struct at f.c:3:1)").
parse() turns one into a small tree; host() and declare() print it back as
the port compiles it: every pointer becomes a gaddr, the game's 32-bit
address (port/runtime/include/openrac/guest.h). Everything else keeps its
name, so typedefs, struct tags and the console's layouts carry over.
"""

from __future__ import annotations

import re
from dataclasses import dataclass, field

QUALIFIERS = {"const", "volatile", "restrict", "__restrict", "__restrict__"}

# Words that make up a builtin type name ("unsigned long long int").
BUILTIN_WORDS = {
    "void", "char", "short", "int", "long", "float", "double", "signed", "unsigned",
    "_Bool", "__int128", "_Complex",
}
TAG_WORDS = {"struct", "union", "enum"}


class TypeError_(ValueError):
    """A type string this parser does not understand."""


@dataclass(frozen=True)
class Base:
    name: str                        # "int", "unsigned char", "S", "struct Foo"
    quals: frozenset = frozenset()   # const, volatile

    def is_void(self) -> bool:
        return self.name == "void"


@dataclass(frozen=True)
class Ptr:
    to: "Type"
    quals: frozenset = frozenset()


@dataclass(frozen=True)
class Arr:
    of: "Type"
    size: int | None                 # None: unknown size ("int []"), or variable (vla)
    vla: str | None = None           # a variable length: the expression Clang printed


@dataclass(frozen=True)
class Func:
    ret: "Type"
    params: tuple = ()
    variadic: bool = False
    prototyped: bool = True          # False for "int ()": no parameter list


Type = Base | Ptr | Arr | Func

# "struct (unnamed struct at f.c:3:1)", "union S::(anonymous at f.c:1:25)".
UNNAMED = re.compile(
    r"(?:(struct|union|enum)\s+)?(?:\w+::)*\((?:unnamed|anonymous)(?: (struct|union|enum))? at ([^()]*?)\)")


def anon_tag(where: str) -> str:
    """The tag hostgen gives a record that has none, from where it is."""
    stem = re.sub(r"[^A-Za-z0-9]+", "_", where).strip("_")
    return f"openrac_anon_{stem}"


def _unnamed(m: re.Match) -> str:
    """An unnamed record gets a tag of its own; an unnamed enum is an int, as on the console
    (`enum { OCT, DEC, HEX } base;` in newlib's _vfprintf_r): its constants are written apart."""
    kind = m.group(1) or m.group(2)
    return "int" if kind == "enum" else f"{kind} {anon_tag(m.group(3))}"


_ATTRIBUTE = re.compile(r"__attribute__\s*\(\((?:[^()]|\([^()]*\))*\)\)")


def _tokens(text: str) -> list[str]:
    text = _ATTRIBUTE.sub("", text)
    text = UNNAMED.sub(_unnamed, text)
    toks = re.findall(r"[A-Za-z_][A-Za-z_0-9]*|\d+|\.\.\.|[()*\[\],+\-/<>&|^%~!.]", text)
    rest = re.sub(r"[A-Za-z_][A-Za-z_0-9]*|\d+|\.\.\.|[()*\[\],+\-/<>&|^%~!.]|\s+", "", text)
    if rest:
        raise TypeError_(f"unexpected characters {rest!r} in type {text!r}")
    return toks


class _Parser:
    def __init__(self, text: str):
        self.text = text
        self.toks = _tokens(text)
        self.i = 0

    def peek(self, k: int = 0) -> str | None:
        j = self.i + k
        return self.toks[j] if j < len(self.toks) else None

    def take(self, want: str | None = None) -> str:
        tok = self.peek()
        if tok is None or (want is not None and tok != want):
            raise TypeError_(f"expected {want or 'a token'} at {self.i} in {self.text!r}")
        self.i += 1
        return tok

    def specifiers(self) -> Base:
        quals = set()
        words: list[str] = []
        while (tok := self.peek()) is not None:
            if tok in QUALIFIERS:
                quals.add(self.take())
            elif tok in TAG_WORDS:
                self.take()
                words = [tok, self.take()]
            elif tok in BUILTIN_WORDS:
                words.append(self.take())
            elif re.match(r"[A-Za-z_]", tok) and not words:
                words.append(self.take())  # a typedef name
            else:
                break
        if not words:
            raise TypeError_(f"no type name in {self.text!r}")
        return Base(" ".join(words), frozenset(q for q in quals if q in ("const", "volatile")))

    # An abstract declarator, read inside out: what it does to the type it is
    # given is returned as a function, applied once the base is known.
    def declarator(self):
        if self.peek() == "*":
            self.take()
            quals = set()
            while self.peek() in QUALIFIERS:
                quals.add(self.take())
            inner = self.declarator()
            q = frozenset(x for x in quals if x in ("const", "volatile"))
            return lambda t: inner(Ptr(t, q))
        return self.direct()

    def direct(self):
        inner = lambda t: t  # noqa: E731
        if self.peek() == "(" and self.peek(1) in ("*", "(", "["):
            self.take("(")
            inner = self.declarator()
            self.take(")")
        suffixes = []
        while self.peek() in ("[", "("):
            if self.take() == "[":
                size = None
                vla = None
                if self.peek() != "]":
                    words = []
                    depth = 0
                    while self.peek() is not None and (self.peek() != "]" or depth):
                        tok = self.take()
                        depth += {"[": 1, "]": -1}.get(tok, 0)
                        words.append(tok)
                    if len(words) == 1 and words[0].isdigit():
                        size = int(words[0])
                    else:
                        vla = " ".join(words)
                self.take("]")
                suffixes.append(("arr", size, vla))
            else:
                suffixes.append(("fn",) + self.params())

        def build(t: Type) -> Type:
            for s in reversed(suffixes):
                if s[0] == "arr":
                    t = Arr(t, s[1], s[2])
                else:
                    t = Func(t, s[1], s[2], s[3])
            return inner(t)

        return build

    def params(self):
        params: list[Type] = []
        variadic = False
        if self.peek() == ")":
            self.take()
            return (), False, False
        while True:
            if self.peek() == "...":
                self.take()
                variadic = True
            else:
                params.append(self.type_name())
            if self.peek() == ",":
                self.take()
                continue
            self.take(")")
            break
        if len(params) == 1 and params[0] == Base("void"):
            params = []
        return tuple(params), variadic, True

    def type_name(self) -> Type:
        base = self.specifiers()
        return self.declarator()(base)


def parse(text: str) -> Type:
    p = _Parser(text)
    t = p.type_name()
    if p.peek() is not None:
        raise TypeError_(f"trailing {p.toks[p.i:]} in {text!r}")
    return t


# ---- Printing for the host ----

def host(t: Type) -> str:
    """The host type, as an abstract type name (for casts and sizeof)."""
    return declare(t, "").strip()


def declare(t: Type, name: str) -> str:
    """A host declaration of name with type t ("gaddr p", "int a[4]")."""
    if isinstance(t, Ptr):
        quals = " ".join(sorted(t.quals))
        return f"gaddr{' ' + quals if quals else ''}{(' ' + name) if name else ''}"
    if isinstance(t, Arr):
        dims = f"[{t.size}]" if t.size is not None else f"[{t.vla}]" if t.vla else "[]"
        inner = f"({name})" if name.startswith("*") else name
        return declare(t.of, f"{inner}{dims}")
    if isinstance(t, Func):
        params = ", ".join(host(p) for p in t.params)
        if t.variadic:
            params = f"{params}, ..." if params else "..."
        elif not params and t.prototyped:
            params = "void"
        inner = f"({name})" if name.startswith("*") else name
        return declare(t.ret, f"{inner}({params})")
    quals = " ".join(q for q in ("const", "volatile") if q in t.quals)
    return f"{quals + ' ' if quals else ''}{t.name}{(' ' + name) if name else ''}"


def fn_pointer(t: Func) -> str:
    """The host function pointer type for a call through a game code address."""
    return declare(t, "(*)").replace("((*))", "(*)")


def strip_quals(t: Type) -> Type:
    if isinstance(t, Base):
        return Base(t.name)
    if isinstance(t, Ptr):
        return Ptr(t.to)
    return t


def is_pointer(t: Type) -> bool:
    return isinstance(t, Ptr)


INT_NAMES = {
    "char", "signed char", "unsigned char", "short", "short int", "signed short",
    "unsigned short", "unsigned short int", "int", "signed", "signed int", "unsigned",
    "unsigned int", "long", "long int", "unsigned long", "long long", "long long int",
    "unsigned long long", "unsigned long long int", "signed long long", "_Bool",
    "__int128", "unsigned __int128",
}
FLOAT_NAMES = {"float"}


@dataclass
class Typedefs:
    """What each typedef name stands for, so a type can be seen through them."""

    table: dict[str, Type] = field(default_factory=dict)

    def resolve(self, t: Type) -> Type:
        seen = 0
        while isinstance(t, Base) and t.name in self.table and seen < 32:
            t = self.table[t.name]
            seen += 1
        return t
