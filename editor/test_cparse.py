"""cparse.py, the small C reader behind the behaviour catalogue and the pvar fields: tokens, types
and struct layouts as the PS2 compiler lays them out, on C written for the test.

These pin what the reader does today, quirks included (each says so where it is one): the
catalogue's offsets and the editor's pvar fields are made from it.
"""

from pathlib import Path
import sys
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parent))

from cparse import CType, Field, Types, lex, strip  # noqa: E402

SOURCE = r"""
#include "common.h"
#define LONG_MACRO 4 \
    + 1
/* a block
   comment */
typedef unsigned char uchar; // a line comment
typedef struct Vec { f32 x, y, z; } Vec;
typedef struct { u8 a; u32 b; s16 c[3]; } Anon;
typedef void (*Callback)(int);
typedef s32 Grid[2][3];
struct Moby {
    Vec pos;
    u8 flag;
    struct Moby *next;
    void (*update)(struct Moby *);
    union { u32 word; f32 value; } u;
    unsigned short hw;
    long long big;
    char name[0x10 - 4];
    int count[N];
    uchar tail;
};
struct Bits { u32 a : 3; u32 b : 5; };
union U { u8 b; f64 d; };
extern int f_alias(int) __asm__("f");
extern f32 table[] __asm__("D_00123456");
static const unsigned int value SDATA(0) = 1;
int g __attribute__((aligned(16)));
"""


def read(text: str = SOURCE) -> Types:
    types = Types()
    types.read(text)
    return types


def layout(types: Types, name: str) -> list[tuple]:
    return [(f.name, f.ctype, f.offset, f.size) for f in types.structs[name].fields]


class LexTests(unittest.TestCase):
    def test_tokens(self):
        self.assertEqual(lex(r"""x = 0x10u + 1.5f - .5e3 >>= a->b; "s\"t" 'c' ..."""),
                         ["x", "=", "0x10u", "+", "1.5f", "-", ".5e3", ">>=", "a", "->", "b", ";",
                          r'"s\"t"', "'c'", "..."])
        self.assertEqual(lex("a<<=b++ != c && d || e[1]"),
                         ["a", "<<=", "b", "++", "!=", "c", "&&", "d", "||", "e", "[", "1", "]"])

    def test_strip_drops_comments_preprocessor_lines_and_attributes(self):
        text = strip(SOURCE)
        for gone in ("#include", "LONG_MACRO", "+ 1", "a block", "a line comment", "__attribute__", "SDATA",
                     "__asm__"):
            self.assertNotIn(gone, text)
        self.assertIn("typedef unsigned char uchar;", text)
        self.assertEqual(lex("static int value SDATA(0) = 1;"), ["static", "int", "value", "=", "1", ";"])


class CTypeTests(unittest.TestCase):
    def test_pointers_floats_and_dereferencing(self):
        grid = CType("f32", 0, (2, 3))
        self.assertEqual(grid.deref(), CType("f32", 0, (3,)))
        self.assertEqual(grid.deref().deref(), CType("f32"))
        self.assertFalse(grid.is_float)
        self.assertTrue(CType("f32").is_float and CType("f64").is_float)
        self.assertFalse(CType("f32", 1).is_float)
        self.assertTrue(CType("u8", 1).is_pointer)
        self.assertEqual(CType("u8", 2).deref(), CType("u8", 1))
        self.assertEqual(CType("u8").deref(), CType("u8"))     # not a pointer: stays itself


class LayoutTests(unittest.TestCase):
    def setUp(self):
        self.types = read()

    def test_typedef_structs_named_and_anonymous(self):
        f32 = CType("f32")
        self.assertEqual(layout(self.types, "Vec"), [("x", f32, 0, 4), ("y", f32, 4, 4), ("z", f32, 8, 4)])
        self.assertEqual((self.types.structs["Vec"].size, self.types.structs["Vec"].align), (12, 4))
        self.assertEqual(layout(self.types, "Anon"),
                         [("a", CType("u8"), 0, 1), ("b", CType("u32"), 4, 4), ("c", CType("s16", 0, (3,)), 8, 6)])
        self.assertEqual(self.types.structs["Anon"].name, "Anon")
        self.assertEqual(self.types.structs["Anon"].size, 16)

    def test_a_struct_with_every_kind_of_member(self):
        moby = self.types.structs["Moby"]
        self.assertEqual((moby.size, moby.align, moby.exact), (64, 8, True))
        union = moby.named("u").ctype.base
        self.assertEqual(layout(self.types, "Moby"), [
            ("pos", CType("Vec"), 0, 12),
            ("flag", CType("u8"), 12, 1),
            ("next", CType("Moby", 1), 16, 4),
            ("update", CType("void", 1), 20, 4),         # a function pointer: a void *
            ("u", CType(union), 24, 4),
            ("hw", CType("u16"), 28, 2),
            ("big", CType("s64"), 32, 8),
            ("name", CType("s8", 0, (12,)), 40, 12),     # 0x10 - 4
            ("count", CType("s32", 0, (1,)), 52, 4),     # N is not a constant here: 1
            ("tail", CType("u8"), 56, 1),
        ])
        # A quirk: the nested union keeps its placeholder name and stays in structs.
        self.assertTrue(union.startswith("<anon@"))
        self.assertEqual(layout(self.types, union), [("word", CType("u32"), 0, 4), ("value", CType("f32"), 0, 4)])

    def test_unions_and_bit_fields(self):
        self.assertEqual(layout(self.types, "U"), [("b", CType("u8"), 0, 1), ("d", CType("f64"), 0, 8)])
        self.assertEqual((self.types.structs["U"].size, self.types.structs["U"].align), (8, 8))
        bits = self.types.structs["Bits"]
        self.assertEqual((bits.fields, bits.size, bits.exact), ([], 0, False))   # not modelled: inexact

    def test_lookups(self):
        moby = self.types.structs["Moby"]
        self.assertEqual(moby.at(12), Field("flag", CType("u8"), 12, 1))
        self.assertIsNone(moby.at(13))
        self.assertEqual(moby.named("tail").offset, 56)
        self.assertIsNone(moby.named("missing"))


class TypedefAndSymbolTests(unittest.TestCase):
    def setUp(self):
        self.types = read()

    def test_aliases_and_asm_symbols(self):
        self.assertEqual(self.types.aliases, {"uchar": CType("u8"), "Callback": CType("void", 1),
                                              "Grid": CType("s32", 0, (2, 3))})
        self.assertEqual(self.types.symbols, {"f_alias": "f", "table": "D_00123456"})
        self.assertEqual(self.types.resolve("uchar"), CType("u8"))
        self.assertEqual(self.types.resolve("Unknown"), CType("Unknown"))

    def test_type_names(self):
        for name in ("Vec", "uchar", "long", "u32", "struct"):
            self.assertTrue(self.types.is_type_name(name), name)
        self.assertFalse(self.types.is_type_name("value"))

    def test_sizes_and_alignments(self):
        self.assertEqual(self.types.size_align(CType("Vec", 0, (4,))), (48, 4))
        self.assertEqual(self.types.size_align(CType("u16")), (2, 2))
        self.assertEqual(self.types.size_align(CType("f64")), (8, 8))
        self.assertEqual(self.types.size_align(CType("Moby", 1, (3,))), (12, 4))
        self.assertEqual(self.types.size_align(CType("Unknown")), (4, 4))   # an unknown type: a word

    def test_type_specifiers(self):
        cases = {
            "unsigned long long x": (CType("u64"), 3),
            "long x": (CType("s64"), 1),
            "unsigned x": (CType("u32"), 1),
            "short int x": (CType("s16"), 1),     # `int` after a base is the declarator's name here
            "enum Kind x": (CType("s32"), 2),
            "const struct Moby *p": (CType("Moby"), 3),
            "unsigned uchar x": (CType("u8"), 2),
            "static volatile f32 x": (CType("f32"), 3),
        }
        for text, expected in cases.items():
            with self.subTest(text=text):
                self.assertEqual(self.types.parse_type(text.split(), 0), expected)
        # No type at all. A quirk: parse_type raises ParseError, which cparse.py does not define,
        # so today it is a NameError; any exception is what callers can rely on.
        with self.assertRaises(Exception):
            self.types.parse_type(["x"], 0)

    def test_declarators_and_constant_dimensions(self):
        self.assertEqual(self.types.parse_declarator(["*", "const", "*", "p", "[", "2", "]", "[", "4", "]", ";"], 0),
                         ("p", 2, (2, 4), 10))
        self.assertEqual(self.types.const_int(["0x138", "-", "0x70"]), 200)
        self.assertEqual(self.types.const_int(["4u"]), 4)
        self.assertEqual(self.types.const_int(["N"]), 1)
        self.assertEqual(self.types.const_int(["__import__", "(", "'os'", ")"]), 1)   # no builtins: 1

    def test_read_adds_to_what_was_read(self):
        types = read("typedef struct A { u32 a; } A;")
        types.read("struct B { A first; u8 b; };")
        self.assertEqual(layout(types, "B"), [("first", CType("A"), 0, 4), ("b", CType("u8"), 4, 1)])
        self.assertEqual(types.structs["B"].size, 8)


if __name__ == "__main__":
    unittest.main()
