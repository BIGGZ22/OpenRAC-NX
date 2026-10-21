# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""hostgen's tests: small programs in the console's C, translated, compiled
for the host with a minimal runtime (tests/harness.c), run, and their output
checked. Each program is what the decompilation does in some function; none
is game code.

    python3 -m unittest discover -s port/tools/hostgen
"""

from __future__ import annotations

import json
import re
import shutil
import subprocess
import sys
import tempfile
import textwrap
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

import asm  # noqa: E402
import clangast  # noqa: E402
import ctype  # noqa: E402
import hostgen  # noqa: E402
import prep  # noqa: E402

sys.path.insert(0, str(HERE / "tests"))
import mips  # noqa: E402

RUNTIME_INCLUDE = HERE.parent.parent / "runtime" / "include"

COMMON_H = """
#ifndef COMMON_H
#define COMMON_H
typedef signed char s8;
typedef short s16;
typedef int s32;
typedef long s64;
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long u64;
typedef float f32;
extern void test_print(int);
extern void test_print_float(float);
extern void test_print_str(char *);
static __inline__ void qcopy(void *dst, void *src) {
    __asm__ __volatile__("lq $2,0x0(%1)\\n\\tsq $2,0x0(%0)" : : "r"(dst), "r"(src) : "$2", "memory");
}
#endif
"""

HOST_FUNCTIONS = ["test_print", "test_print_float", "test_print_str"]


def have_compiler() -> bool:
    return shutil.which("clang") is not None and sys.platform != "win32"


@unittest.skipUnless(have_compiler(), "needs Clang on a POSIX host")
class Translate(unittest.TestCase):
    def run_program(self, files: dict[str, str], places: str = "", config: dict | None = None,
                    extra_files: dict[str, str] | None = None, host_c: str = "",
                    stubs: int = 0, hostgen_args: list[str] | None = None,
                    cflags: list[str] | None = None) -> list[str]:
        """Translates, builds and runs files (name -> C); returns the printed lines.
        config adds to hostgen.json; extra_files are written beside it (tables);
        host_c is linked in (the port's own C); stubs is how many functions are
        expected to become stubs."""
        with tempfile.TemporaryDirectory() as tmp:
            tmp = Path(tmp)
            src = tmp / "game"
            (src / "include").mkdir(parents=True)
            (src / "include" / "common.h").write_text(COMMON_H)
            for name, text in files.items():
                path = src / "src" / name
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_text('#include "common.h"\n' + textwrap.dedent(text))
            cfg = {"name": "test", "sources": ["src"], "includes": ["include", "src"],
                   "host_functions": HOST_FUNCTIONS}
            if places:
                (src / "functions.tsv").write_text(places)
                cfg["places"] = "functions.tsv"
            cfg.update(config or {})
            for name, text in (extra_files or {}).items():
                (tmp / name).parent.mkdir(parents=True, exist_ok=True)
                (tmp / name).write_text(text)
            (tmp / "hostgen.json").write_text(json.dumps(cfg))
            out = tmp / "gen"
            rc = hostgen.main(["--game", str(tmp / "hostgen.json"), "--source", str(src), "--out", str(out),
                               "--jobs", "2", "--quiet", *(hostgen_args or [])])
            self.assertEqual(rc, 0)
            report = json.loads((out / "report.json").read_text())
            self.last_report = report
            self.assertEqual(report["functions_stubbed"], stubs, report["stubbed"])
            sources = [out / u.strip() for u in (out / "units.txt").read_text().splitlines()]
            self.last_generated = "\n".join(s.read_text() for s in sources)
            sources += [out / "stubs.c", out / "functions.c", out / "hooks.c", out / "libraries.c", HERE / "tests" / "harness.c"]
            if host_c:
                (tmp / "host.c").write_text('#include "openrac/guest.h"\n' + textwrap.dedent(host_c))
                sources.append(tmp / "host.c")
            exe = tmp / "prog"
            build = subprocess.run(
                ["clang", "-std=gnu11", "-O1", "-w", "-fno-strict-aliasing", "-fwrapv", "-fsigned-char",
                 *(cflags or []), "-I", str(RUNTIME_INCLUDE), "-I", str(out), "-o", str(exe), *map(str, sources)],
                capture_output=True, text=True)
            self.assertEqual(build.returncode, 0, build.stderr)
            run = subprocess.run([str(exe)], capture_output=True, text=True, timeout=30)
            self.assertEqual(run.returncode, 0, run.stderr)
            return run.stdout.split()

    def test_globals_structs_and_pointer_arithmetic(self):
        out = self.run_program({"a.c": """
            typedef struct Node { struct Node *next; short id; char pad; float f; s64 big; int arr[3]; } Node;
            extern Node D_00200000[4];
            extern int D_00200100;
            int test_main(void) {
                Node *n = D_00200000;
                Node *last = &D_00200000[3];
                int i;
                for (i = 0; i < 4; i++) {
                    n[i].id = i * 10;
                    n[i].next = i < 3 ? &n[i + 1] : 0;
                    n[i].big = (s64)i << 40;
                }
                test_print(sizeof(Node));            /* the console's layout: big is 8-aligned, 40 in all */
                test_print(last - n);                /* pointer difference, in elements */
                test_print(n->next->next->id);
                test_print((int)(n[3].big >> 40));
                test_print(*(short *)((char *)D_00200000 + 40 * 2 + 4));  /* raw offset access */
                D_00200100 = (int)&D_00200000[1];
                test_print(D_00200100 - 0x200000);   /* addresses are the console's */
                n++;
                test_print(n->id);
                test_print(n[-1].id);
                return 0;
            }
        """})
        self.assertEqual(out, ["40", "3", "20", "3", "20", "40", "10", "0"])

    def test_locals_in_game_memory(self):
        out = self.run_program({"a.c": """
            void fill(int *p, int n) { int i; for (i = 0; i < n; i++) p[i] = i * i; }
            void set(float *f) { *f = 2.5f; }
            typedef struct { int a; int b[2]; } Pair;
            int sum(Pair *p) { return p->a + p->b[0] + p->b[1]; }
            int test_main(void) {
                int arr[5];
                float x = 1.0f;
                Pair pr;
                int total = 0;
                int i;
                fill(arr, 5);
                for (i = 0; i < 5; i++) total += arr[i];
                test_print(total);
                set(&x);
                test_print_float(x);
                pr.a = 1; pr.b[0] = 2; pr.b[1] = 3;
                test_print(sum(&pr));
                test_print(sum(&(Pair){4, {5, 6}}));
                return 0;
            }
        """})
        self.assertEqual(out, ["30", "2.500", "6", "15"])

    def test_a_local_vector_or_matrix_starts_on_a_quadword(self):
        out = self.run_program({"a.c": """
            typedef float Vec[4] __attribute__((aligned(16)));
            typedef struct { float f[4]; } Quad __attribute__((aligned(16)));
            struct Holder { char c; Quad q; };
            static void copy(Quad* to, Quad* from) { *to = *from; }
            int test_main(void) {
                char pad4[9];
                Quad qa, qb;
                struct Holder h;
                pad4[0] = 4;
                qa.f[0] = 5.0f;
                copy(&qb, &qa);
                test_print((int)((unsigned long)&qa & 15));
                test_print((int)((char*)&h.q - (char*)&h));
                test_print((int)qb.f[0] + pad4[0] - 4);
                char pad[3];
                float v[4];
                char pad2[5];
                float m[4][4];
                char pad3[7];
                Vec t;
                pad[0] = 1; pad2[0] = 2; pad3[0] = 3; v[0] = 1.0f; m[0][0] = 2.0f; t[0] = 3.0f;
                test_print((int)((unsigned long)&v[0] & 15));
                test_print((int)((unsigned long)&m[0][0] & 15));
                test_print((int)((unsigned long)&t[0] & 15));
                return pad[0] - 1 + pad2[0] - 2 + pad3[0] - 3;
            }
        """})
        self.assertEqual(out, ["0", "16", "5", "0", "0", "0"])

    def test_a_three_float_local_has_a_quadword_slot(self):
        # The console's compiler gives an array or record with no machine mode of its own a
        # slot of whole quadwords; the game copies 16-byte points into `float v[3]`, and the
        # fourth word must not land in the next local (Umbris's moby 1142: a path index
        # became a segment length's bits, and the next lookup read far outside the path).
        out = self.run_program({"a.c": """
            typedef struct { float x, y, z; } P3;
            static void put4(float *to) { to[0] = 1.0f; to[1] = 2.0f; to[2] = 3.0f; to[3] = 7.5f; }
            static void set(int *to, int n) { *to = n; }
            int test_main(void) {
                float v[3];
                int i;
                P3 p;
                int j;
                short pair[2];
                int k;
                set(&i, 5); set(&j, 6); set(&k, 8);
                put4(v);
                put4(&p.x);
                pair[0] = 1; pair[1] = 2;
                test_print(i);
                test_print(j);
                test_print(k + pair[0] + pair[1] - 3);
                test_print((int)((unsigned long)&v[0] & 15));
                test_print((int)((unsigned long)&p & 15));
                test_print((int)v[2] + (int)p.z);
                return 0;
            }
        """})
        self.assertEqual(out, ["5", "6", "8", "0", "0", "6"])

    def test_a_bare_return_in_a_function_with_a_result(self):
        # Matching C keeps the original's `return;` in an int (or pointer) function: old GCC took
        # it, and today's Clang refuses to compile it. The translation returns zero.
        out = self.run_program({"bare.c": """
            int early(int x) {
                if (x < 0) {
                    return;
                }
                return x * 2;
            }
            char *nothing(int x) {
                if (x) {
                    return;
                }
                return 0;
            }
            int test_main(void) {
                test_print(early(4));
                test_print(early(-1));
                test_print(nothing(1) == 0);
                return 0;
            }
        """})
        self.assertEqual(out, ["8", "0", "1"])

    def test_a_call_short_of_arguments_is_reported(self):
        # The caller has no argument of its own for the callee's second: the register held
        # whatever the caller left there, so the call is filled with 0 and listed.
        out = self.run_program({
            "callee.c": """
                int two(int a, int b) { return a + b; }
            """,
            "caller.c": """
                extern int two(int);
                int test_main(void) { test_print(two(5)); test_print(two(6)); return 0; }
            """,
        })
        # Each call counts in a site of its own (OPENRAC_REACH): its first call registers it.
        self.assertEqual(out, ["reach", "test_main", "5", "reach", "test_main", "6"])
        self.assertEqual(self.last_report["arguments_filled_with_zero"],
                         {"test_main": ["two argument 2", "two argument 2"]})
        self.assertEqual(self.last_generated.count("openrac_reach_site site_"), 2)

    def test_calls_follow_the_definition(self):
        # The caller declares the callee with its float and integer arguments
        # swapped, and with one argument fewer: on the EE that call still
        # works (floats and integers travel in separate registers; the
        # missing one is the caller's own, still in its register).
        out = self.run_program({
            "callee.c": """
                int mix(int a, float b, int c) { test_print(a); test_print_float(b); return c; }
            """,
            "caller.c": """
                extern int mix(float, int);
                int test_main(void) { return run(7); }
                int run(int unused) { test_print(mix(1.5f, 3)); return 0; }
            """,
        })
        self.assertEqual(out, ["reach", "run", "3", "1.500", "0"])

    def test_a_call_through_a_cast_of_a_function_follows_the_definition(self):
        # The decompilation casts a function to another order of its float and integer
        # parameters to get the retail argument setup (partupd_00280428.c). On the EE that is
        # the function's own call; on the host it goes through the definition's type, with
        # each kind's arguments in order: Windows x64 gives arguments slots by position, and
        # the cast's type put the integer where the callee reads its second float.
        out = self.run_program({
            "callee.c": """
                void func_00110000(int *out, int n, float a, float b) { *out = n * 100 + (int)a * 10 + (int)b; }
            """,
            "caller.c": """
                extern void func_00110000(int *, int, float, float);
                int test_main(void) {
                    int r;
                    ((void (*)(int *, float, float, int))func_00110000)(&r, 3.0f, 4.0f, 2);
                    test_print(r);
                    return 0;
                }
            """,
        })
        self.assertEqual(out, ["234"])
        self.assertIn("GFN(void (*)(gaddr, int, float, float), ", self.last_generated)
        self.assertNotIn("GFN(void (*)(gaddr, float, float, int), ", self.last_generated)

    def test_a_cast_to_fewer_arguments_passes_the_callers_own(self):
        # rac3's text code casts a width function to take only its float (`((s32 (*)(f32))f)(1.0f)`):
        # the integers stay what the caller's registers held. Through the definition's type, as a
        # direct call: on Windows x64 the cast's float would land where the callee reads a pointer.
        out = self.run_program({
            "callee.c": """
                int func_00110000(int *p, int n, float x) { return n * 10 + (int)x; }
            """,
            "caller.c": """
                extern int func_00110000(int *, int, float);
                int func_00120000(int *p, int n) { return ((int (*)(float))func_00110000)(2.0f); }
                int test_main(void) {
                    test_print(func_00120000(0, 5));
                    return 0;
                }
            """,
        })
        self.assertEqual(out, ["52"])
        self.assertIn("GFN(int (*)(gaddr, int, float), ", self.last_generated)

    def test_a_callee_typedef_the_caller_does_not_know(self):
        # The callee's file names its float and its pointer by typedefs the caller's file does
        # not have: the call still passes the float in a float register and the pointer in an
        # integer one, and the float result comes back.
        out = self.run_program({
            "callee.c": """
                typedef float real_t;
                typedef int word_t;
                real_t half(word_t *p, word_t n, real_t x) { test_print(*p + n); return x * 0.5f; }
            """,
            "caller.c": """
                extern float half(void *p, void *n, float x);
                int test_main(void) { int v = 40; test_print_float(half(&v, (void *)2, 3.0f)); return 0; }
            """,
        })
        self.assertEqual(out, ["42", "1.500"])

    def test_the_ports_own_function_replaces_another_spelling_of_its_place(self):
        # The decompilation has C for 0x00100100 under one spelling; the port's own function
        # (host_functions) names the same place another way. Calls by either name, and through the
        # address, reach the port's.
        out = self.run_program({
            "a.c": """
                int FUN_00100100(void) { return 1; }
                extern int func_00100100(void);
                int test_main(void) {
                    int (*p)(void) = (int (*)(void))0x00100100;
                    test_print(FUN_00100100());
                    test_print(p());
                    return 0;
                }
            """,
        }, config={"host_functions": HOST_FUNCTIONS + ["func_00100100"],
                   "names": {"code": [r"^func_(?P<addr>[0-9A-Fa-f]{8})$", r"^FUN_(?P<addr>[0-9A-Fa-f]{8})$"]}},
           host_c="int func_00100100(void) { return 2; }")
        self.assertEqual(out, ["2", "2"])

    def test_a_long_long_copy_of_the_games_compiler_moves_a_quadword(self):
        # SN's GCC 2.95.3 has a 128-bit long long (hostgen.json "long_long_128"): rac1's CannonUpdate
        # clears and copies vectors through `*(long long *)v` (por + sq, lq + sq), all four words.
        # Without the setting long long is Clang's 64 bits, and only x and y move.
        files = {"a.c": """
            int test_main(void) {
                float v[4];
                float w[4];
                v[0] = 1.0f; v[1] = 2.0f; v[2] = 3.0f; v[3] = 4.0f;
                w[0] = w[1] = w[2] = w[3] = 0.0f;
                *(long long *)w = *(long long *)v;
                test_print((int)(w[0] + w[1] + w[2] + w[3]));
                *(long long *)v = 0;
                test_print((int)(v[0] + v[1] + v[2] + v[3]));
                return 0;
            }
        """}
        self.assertEqual(self.run_program(files, config={"long_long_128": {"except_compilers": []}}), ["10", "0"])
        self.assertEqual(self.run_program(files), ["3", "7"])

    def test_pass_through(self):
        out = self.run_program({
            "callee.c": "int two(int a, int b) { return a * 10 + b; }",
            "caller.c": """
                extern int two(int);
                int wrap(int x, int y) { return two(x); }
                int test_main(void) { test_print(wrap(4, 2)); return 0; }
            """,
        })
        self.assertEqual(out, ["42"])

    def test_a_result_passes_through_a_definition_that_ends_in_a_call(self):
        # Matched as returning nothing, the wrappers still leave their last call's result in
        # the register, and their callers (which declare them with a result) read it: through
        # two of them here, a float and an integer, with an early return that leaves nothing.
        out = self.run_program({
            "callee.c": """
                float height(void *p, int flags, float r) { return (float)flags + r; }
                int count(int n) { return n + 1; }
                void ground(void *p) { height(p, 0x20, 0.5f); }
                void ground2(void *p) { ground(p); }
                void counted(int n) { if (n < 0) return; count(n); }
            """,
            "caller.c": """
                extern float ground2_f(void *) __asm__("ground2");
                extern int counted_i(int) __asm__("counted");
                int test_main(void) {
                    test_print_float(ground2_f(0));
                    test_print(counted_i(4));
                    test_print(counted_i(-1));
                    return 0;
                }
            """,
        })
        self.assertEqual(out, ["32.500", "5", "0"])

    def test_pass_through_of_a_record_parameter(self):
        # The callee is defined with three parameters and called with two: the
        # third register still held the caller's own third argument, a record.
        out = self.run_program({
            "callee.c": """
                typedef struct { int a; int b; } Pair;
                int three(int *x, int *y, int z) { return *x + *y + (z != 0); }
            """,
            "caller.c": """
                typedef struct { int a; int b; } Pair;
                extern int three(int *, int *);
                int use(int *x, int *y, Pair p) { return three(x, y) + p.b; }
                int test_main(void) {
                    int u = 1, v = 2;
                    Pair p = {5, 7};
                    test_print(use(&u, &v, p));
                    return 0;
                }
            """,
        })
        self.assertEqual(out, ["11"])

    def test_a_32_bit_argument_to_a_64_bit_parameter_is_sign_extended(self):
        # The call site declares 32 bits, the definition takes 64: the EE's register held the value
        # sign-extended (FontPrintWindow's colour, a void * there, FontPrint's u64). The result of a
        # "returns_64" function keeps all its bits, as the register does.
        out = self.run_program({"a.c": """
            void callee(unsigned long long x) { test_print((int)(x >> 32)); test_print((int)(x & 0xFFFF)); }
            unsigned long long tex(int i) { return ((unsigned long long)i << 32) | 0x8000ABCDu; }
        """, "b.c": """
            extern void callee(unsigned int x);
            extern void callee_too(void *x) __asm__("callee");
            extern void callee_int(int x) __asm__("callee");
            extern int tex(int i);
            int test_main(void) {
                unsigned int table[2];
                table[0] = 0x80001234u;
                table[1] = 0x00005678u;
                callee(table[0]);
                callee(table[1]);
                callee_too((void *)0x80009ABCu);
                callee_int(tex(3));
                return 0;
            }
        """}, config={"returns_64": ["tex"]})
        self.assertEqual(out, ["-1", "4660", "0", "22136", "-1", "39612", "3", "43981"])

    def test_a_parameter_only_passed_on_to_64_bits_is_64_bit(self):
        # FontPrintCenter's `int c` is FontPrint's u64 colour: the EE passes the register on with all its bits,
        # so the parameter is 64-bit here, and each caller converts its own value (an int sign-extended).
        out = self.run_program({"a.c": """
            void callee(int y, unsigned long long x) { test_print((int)(x >> 32)); test_print((int)(x & 0xFFFF)); }
        """, "b.c": """
            extern void callee(void *y, void *x);
            int middle(int y, int c) { callee((void *)y, (void *)c); return y; }
            int outer(int c) { return middle(1, c); }
            int test_main(void) {
                int colour = (int)0x80001234u;
                middle(2, colour);
                outer(colour);
                middle(3, 0x5678);
                return 0;
            }
        """})
        self.assertEqual(out, ["-1", "4660", "-1", "4660", "0", "22136"])
        self.assertIn("middle: parameter 2 (unsigned long long)", self.last_report["params_widened"])

    def test_a_widened_parameter_is_whole_in_64_bit_casts_and_its_low_half_in_arithmetic(self):
        # DrawFramedText's `s32 color` (rac1): `(s32)color >> 24` is a sra of the register's low half,
        # `(s64)color & ...` an and of the whole register, and the colour is passed on to a u64 as well; its
        # caller hands it a colour that is zero-extended in the register (FastTweenColor's).
        out = self.run_program({"a.c": """
            void callee(int y, unsigned long long x) { test_print((int)(x >> 32)); test_print((int)(x >> 16)); }
            unsigned long long tween(int c) { return (unsigned int)c; }
        """, "b.c": """
            extern void callee(int y, unsigned long long x);
            extern int tween(int c);
            void framed(int y, int c) {
                int alpha = (int)c >> 24;
                test_print(alpha);
                test_print(c + 1);
                callee(y, (long long)c & (long long)(int)0xFF000000);
                callee(y, c);
            }
            int test_main(void) {
                int zero_extended = tween((int)0x80001234u);
                framed(1, zero_extended);
                framed(2, (int)0x80001234u);
                return 0;
            }
        """}, config={"returns_64": ["tween"]})
        self.assertEqual(out, ["-128", "-2147478987", "0", "32768", "0", "32768",
                               "-128", "-2147478987", "-1", "-32768", "-1", "-32768"])
        self.assertIn("unsigned long long c", self.last_generated)

    def test_a_returns_64_result_returned_as_it_is_is_64_bit_and_constants_beside_it_are_sign_extended(self):
        # Pause_GetFadeColor (rac1) is an int that returns FastTweenColor's colour, zero-extended in the EE's result
        # register: its callers keep all 64 bits (Pause_DrawDescriptionBox hands them on to FontPrintWindow's u64). A
        # local that receives such a colour and, on another path, a constant (DrawMenuItemList's `color =
        # 0x80FFA888`) holds the constant sign-extended, as lui/ori leave it; so does a widened result's other return
        # value.
        out = self.run_program({"a.c": """
            void callee(unsigned long long x) { test_print((int)(x >> 32)); test_print((int)(x & 0xFFFF)); }
            unsigned long long tween(int c) { return (unsigned int)c; }
        """, "b.c": """
            extern void callee(int x);
            extern int tween(int c);
            int fade(int c) { return tween(c); }
            int fade_or(int c) { if (c == 0) { return 0x80009ABCu; } return fade(c); }
            void pick(int sel, int c) {
                int color;
                if (sel) {
                    color = 0x80005678u;
                } else {
                    color = fade(c);
                }
                callee(color);
            }
            int test_main(void) {
                pick(0, (int)0x80001234u);
                pick(1, 0);
                callee(fade_or((int)0x80001234u));
                callee(fade_or(0));
                return 0;
            }
        """}, config={"returns_64": ["tween"]})
        self.assertEqual(out, ["0", "4660", "-1", "22136", "0", "4660", "-1", "39612"])
        self.assertEqual(self.last_report["results_widened"], ["fade", "fade_or"])

    def test_an_unsigned_value_beside_a_returns_64_result_in_a_local_is_sign_extended(self):
        # RibbonTrailDraw (rac1 func_L10_002DBA50) keeps FastTweenColor's colour in an unsigned int local, which
        # hostgen keeps 64-bit, and on another path stores `(color & 0xFFFFFF) + (a << 24)` there: addu leaves the sum
        # sign-extended in the register, where C's conversion of an unsigned int to 64 bits zero-extends it.
        out = self.run_program({"a.c": """
            void callee(unsigned long long x) { test_print((int)(x >> 32)); test_print((int)(x & 0xFFFF)); }
            unsigned long long tween(int c) { return (unsigned int)c; }
        """, "b.c": """
            extern void callee(unsigned int x);
            extern unsigned int tween(int c);
            void trail(int sel, int c) {
                unsigned int color;
                color = tween(c);
                if (sel) {
                    color = (color & 0xFFFFFF) + ((unsigned int)sel << 31);
                }
                callee(color);
            }
            int test_main(void) {
                trail(0, (int)0x80001234u);
                trail(1, 0x1234);
                return 0;
            }
        """}, config={"returns_64": ["tween"]})
        self.assertEqual(out, ["0", "4660", "-1", "4660"])

    def test_a_spilled_local_keeps_a_returns_64_result_sign_extended(self):
        # The freeze screen (rac1 func_001FBE80) keeps three FastTweenColor colours: two in saved registers, whole,
        # and one in a stack slot (sw after the call, lw before its use), sign-extended from its low half. hostgen.json
        # "spilled_locals" names that one.
        out = self.run_program({"a.c": """
            void callee(unsigned long long x) { test_print((int)(x >> 32)); test_print((int)(x & 0xFFFF)); }
            unsigned long long tween(int c) { return (unsigned int)c; }
        """, "b.c": """
            extern void callee(int x);
            extern int tween(int c);
            void colours(int c) {
                int c1 = 0x10;
                int c2 = 0x20;
                if (c != 0) {
                    c1 = tween(c);
                    c2 = tween(c);
                }
                callee(c1);
                callee(c2);
            }
            int test_main(void) {
                colours((int)0x80001234u);
                return 0;
            }
        """}, config={"returns_64": ["tween"], "spilled_locals": {"colours": ["c1"]}})
        self.assertEqual(out, ["-1", "4660", "0", "4660"])

    def test_va_list_passed_as_a_pointer_becomes_a_stub(self):
        # The console's C passes a va_list where a pointer is declared (and the
        # other way round); the host cannot, so the caller becomes a stub and the
        # program still builds.
        self.run_program({"a.c": """
            #include <stdarg.h>
            int vcount(int n, va_list ap) { return n; }
            extern int vcount_p(int n, void *ap) __asm__("vcount");
            int wrap(int n, void *ap) { return vcount_p(n, ap); }
            int test_main(void) { test_print(vcount(3, 0)); return 0; }
        """}, stubs=1)
        self.assertIn("a va_list passed as a game pointer, or the other way round",
                      self.last_report["stubbed"].values())

    def test_a_files_own_va_list_is_a_game_pointer(self):
        # rac1's vfprintf.c declares typedef char *va_list itself and walks the console's
        # argument area, eight bytes a slot; the caller passes it as a pointer.
        out = self.run_program({"a.c": """
            typedef char *va_list;
            int vsum(int n, va_list ap) {
                int s = 0;
                while (n--) s += (*(int *)((ap += 8) - 8));
                return s;
            }
            extern int vsum_p(int n, void *ap) __asm__("vsum");
            int wrap(int n, void *ap) { return vsum_p(n, ap); }
            extern long D_00200000[3];
            int test_main(void) {
                D_00200000[0] = 4; D_00200000[1] = 5; D_00200000[2] = 6;
                test_print(wrap(3, D_00200000));
                return 0;
            }
        """})
        self.assertEqual(out, ["15"])

    def test_a_local_of_an_unnamed_enum(self):
        # newlib's _vfprintf_r: enum { OCT, DEC, HEX } base; an int, its constants usable.
        out = self.run_program({"a.c": """
            int pick(int c) {
                enum { OCT, DEC, HEX } base;
                base = c == 'x' ? HEX : c == 'o' ? OCT : DEC;
                return base == HEX ? 16 : base == OCT ? 8 : 10;
            }
            int test_main(void) { test_print(pick('x')); test_print(pick('d')); return 0; }
        """})
        self.assertEqual(out, ["16", "10"])

    def test_two_functions_with_the_same_local_enum(self):
        # The NTSC decomp declares `enum { JUMP = 7 };` inside several functions; hoisted to file
        # scope the constant is written once, and one that differs keeps the function's own name.
        out = self.run_program({"a.c": """
            int one(int c) { enum { JUMP = 7, STAY = 1 }; return c == JUMP ? STAY : JUMP; }
            int two(int c) { enum { JUMP = 7 }; return c + JUMP; }
            int three(int c) { enum { JUMP = 9 }; return c + JUMP; }
            int test_main(void) { test_print(one(7)); test_print(two(1)); test_print(three(1)); return 0; }
        """})
        self.assertEqual(out, ["1", "8", "10"])

    def test_candidate_replaces_an_assembly_stub(self):
        # The decompilation still has func_00110100 as assembly; its table of
        # candidates offers C for it, which goes at the end of the file.
        out = self.run_program({"a.c": """
            #define INCLUDE_ASM(dir, name)
            extern int D_00200000;
            INCLUDE_ASM("asm", func_00110100); /* a note the decompilation keeps beside it */
            extern int func_00110100(int);
            int test_main(void) { D_00200000 = 40; test_print(func_00110100(2)); return 0; }
        """}, config={"candidates": "candidates.tsv"}, extra_files={
            "game/candidates.tsv": "# name\tfile\nfunc_00110100\tcand/func_00110100.c\n",
            "game/cand/func_00110100.c": "int func_00110100(int x) { return D_00200000 + x; }\n",
        })
        self.assertEqual(out, ["42"])
        self.assertEqual(self.last_report["candidates_used"], ["func_00110100"])

    def test_a_candidate_steps_aside_for_what_the_file_has_matched(self):
        """One file of the port's own C for two functions; the decompilation has since matched one:
        the copy of that one is left out, or the file would define it twice and not be read."""
        source = (
            'int func_00100000(int a) { return a + 1; }\n'
            'INCLUDE_ASM("asm", func_00100010);\n'
        )
        both = (
            'int func_00100010(int a) { return a * 2; }\n'
            '/* the same at a joint: "}" in a string, and a brace in a comment } */\n'
            'int func_00100000(int a) {\n    const char *s = "}";\n    if (a) { return a + 1; }\n    return s[0];\n}\n'
        )
        used = set()
        text = prep.use_candidates(source, {"func_00100010": both, "func_00100000": both}, used)
        self.assertEqual(used, {"func_00100010"})
        self.assertEqual(text.count("int func_00100000(int a)"), 1)      # the matched one only
        self.assertIn("return a * 2;", text)
        self.assertIn("func_00100000 is matched C in this file now", text)
        self.assertNotIn('const char *s = "}"', text)

    def test_a_file_the_decompilation_moved_leaves_no_copy_behind(self):
        """The NTSC decomp moved src/assembly/sdk/library/cmd_sem_init.c to src/sdk/library/:
        the old copy, kept from an earlier build, defined cmd_sem_init a second time."""
        with tempfile.TemporaryDirectory() as tmp:
            src, dst = Path(tmp) / "src", Path(tmp) / "prep"
            (src / "assembly").mkdir(parents=True)
            (src / "assembly" / "f.c").write_text("int f(void) { return 1; }\n")
            prep.copy_tree(src, dst)
            (src / "assembly" / "f.c").rename(src / "f.c")
            prep.copy_tree(src, dst)
            self.assertEqual(sorted(p.relative_to(dst).as_posix() for p in dst.rglob("*.c")), ["f.c"])

    def test_function_pointers_by_code_address(self):
        out = self.run_program({"a.c": """
            int func_00110000(int x) { return x + 1; }
            int func_L05_00300000(int x) { return x + 500; }
            typedef int (*Fn)(int);
            extern Fn D_00200000[2];
            int test_main(void) {
                D_00200000[0] = func_00110000;
                D_00200000[1] = (Fn)0x00300000;
                test_print(D_00200000[0](1));
                test_print((int)D_00200000[0] - 0x110000);
                return 0;
            }
        """})
        self.assertEqual(out, ["2", "0"])

    def test_bitfields_unions_and_anonymous_members(self):
        out = self.run_program({"a.c": """
            typedef struct {
                unsigned lo : 4, hi : 4;
                union { int i; float f; };
                struct { short x, y; } pos;
            } Thing;
            extern Thing D_00200000;
            int test_main(void) {
                Thing *t = &D_00200000;
                t->lo = 3; t->hi = 9;
                t->f = 1.0f;
                t->pos.y = -2;
                test_print(*(unsigned char *)t);
                test_print(t->i);
                test_print(t->pos.y);
                test_print(sizeof(Thing));
                return 0;
            }
        """})
        self.assertEqual(out, ["147", "1065353216", "-2", "12"])

    def test_control_flow_strings_and_statics(self):
        out = self.run_program({"a.c": """
            int count(void) { static int n; int *p = &n; return ++*p; }
            int len(char *s) { int n = 0; while (*s++) n++; return n; }
            int test_main(void) {
                int i = 0;
                char *s = "ratchet";
                count(); count();
                test_print(count());
                test_print(len(s));
                test_print_str(s + 3);
                switch (len("ab")) { case 1: test_print(1); break; case 2: test_print(2); default: test_print(9); }
            again:
                i++;
                if (i < 3) goto again;
                do { i += 10; } while (i < 30);
                test_print(i);
                return 0;
            }
        """})
        self.assertEqual(out, ["3", "7", "chet", "2", "9", "33"])

    def test_quadword_helpers_and_64_bit_long(self):
        out = self.run_program({"a.c": """
            extern int D_00200000[4];
            extern int D_00200010[4];
            int test_main(void) {
                long v = 1;
                D_00200000[0] = 5; D_00200000[3] = 8;
                qcopy(D_00200010, D_00200000);
                test_print(D_00200010[0] + D_00200010[3]);
                v <<= 40;
                test_print((int)(v >> 38));
                test_print(sizeof(long));
                return 0;
            }
        """})
        self.assertEqual(out, ["13", "4", "8"])

    def test_call_before_declaration(self):
        # GCC 2.95 took a call made before the callee's declaration as int f().
        out = self.run_program({"a.c": """
            int test_main(void) { test_print(later(2)); return 0; }
            extern int later(int);
            int later(int x) { return x * 3; }
        """})
        self.assertEqual(out, ["6"])

    def test_functions_without_c_are_stubs(self):
        out = self.run_program({"a.c": """
            extern int func_00120000(int);
            int test_main(void) { test_print(func_00120000(1)); return 0; }
        """})
        self.assertEqual(out, ["missing", "func_00120000", "0"])

    def test_a_stub_counts_its_calls_in_its_own_site(self):
        # The stub's site logs the first call only; the rest are counted in its word.
        out = self.run_program({"a.c": """
            extern int func_00120000(int);
            int test_main(void) { func_00120000(1); func_00120000(2); test_print(3); return 0; }
        """})
        self.assertEqual(out, ["missing", "func_00120000", "3"])

    def test_later_global_declaration_keeps_local_extern_type(self):
        out = self.run_program({"a.c": """
            int func_00100000(void) {
                extern int D_00200000;
                return D_00200000;
            }
            extern unsigned int D_00200000;
            int test_main(void) {
                D_00200000 = 7;
                test_print(func_00100000());
                return 0;
            }
        """})
        self.assertEqual(out, ["7"])


    def test_a_games_own_names_and_symbols(self):
        # A game with non-default LVL/FUN names configures its patterns; a
        # symbol file gives named globals their addresses.
        out = self.run_program({"a.c": """
            int LVL_3_ENDAKO_FUN_00300000(int x) { return x + 3; }
            int BOOT_FUN_00110000(int x) { return x * 2; }
            typedef int (*Fn)(int);
            extern Fn gTable[2];
            int test_main(void) {
                gTable[0] = BOOT_FUN_00110000;
                test_print((int)gTable[0] - 0x110000);
                test_print(gTable[0](21));
                return 0;
            }
        """}, config={
            "names": {"code": ["^LVL_(?P<overlay>\\d+)_\\w+?_FUN_(?P<addr>[0-9A-Fa-f]{8})$",
                               "^(?:\\w+?_)?FUN_(?P<addr>[0-9A-Fa-f]{8})$"]},
            "symbols": ["symbols.txt"]}, extra_files={"game/symbols.txt": "gTable = 0x00200000; // a table\n"})
        self.assertEqual(out, ["0", "42"])

    def test_gcc_295_habits(self):
        # Strings across lines, a later declaration that contradicts the
        # definition, inline assembly Clang cannot read, a variable-length
        # array: the program still builds; the assembly becomes a stub.
        out = self.run_program({"a.c": """
            int twice(int x) { return 2 * x; }
            extern int twice(int, int);
            int odd(void) { int r; __asm__("mfhi %0" : "=h"(r)); return r; }
            int vla(int n) { int a[n + 1]; a[0] = n; return a[0]; }
            int test_main(void) {
                char *s = "two
            lines";
                test_print(twice(4, 9));
                test_print(s[3] == '\\n');
                test_print(vla(6));
                return 0;
            }
        """}, stubs=1)
        self.assertEqual(out, ["8", "1", "6"])
        self.assertIn("inline assembly Clang cannot read", self.last_report["stubbed"].values())

    def test_float_to_integer_as_the_ee_converts(self):
        # Out of the target's range C says nothing and the hosts differ (ARM gives 0 for a
        # negative float into an unsigned type): the EE's conversion on every host.
        out = self.run_program({"a.c": """
            #include "common.h"
            int test_main(void) {
                u8 alpha;
                volatile float minus = -1.5f, big = 3e9f, huge = 1e20f, fifty = 50.7f;
                u32 u = minus;
                u8 b = (u8)(minus * 2.0f);
                alpha = fifty;
                test_print((int)(u == 0xFFFFFFFFu));
                test_print(b);
                test_print((int)((u32)big == 3000000000u));
                test_print((s32)huge == 0x7FFFFFFF);
                test_print((s32)-huge == (s32)0x80000000);
                test_print(alpha);
                return 0;
            }
        """})
        self.assertEqual(out, ["1", "253", "1", "1", "1", "50"])

    def test_variable_length_array_of_a_local_length(self):
        # A one-dimensional array whose length is a local goes on the guest stack for its
        # block, as on the console, and its address can be passed on.
        out = self.run_program({"a.c": """
            int sum(int *p, int n) { int s = 0; while (n--) s += *p++; return s; }
            int fill(int count) {
                int r = 0;
                if (count != 0) {
                    int found[count];
                    int i;
                    for (i = 0; i < count; i++) found[i] = i * 3;
                    r = sum(found, count);
                }
                return r;
            }
            int test_main(void) {
                test_print(fill(5));
                test_print(fill(0));
                test_print(fill(40));
                return 0;
            }
        """})
        self.assertEqual(out, ["30", "0", "2340"])

    def test_variable_length_array_of_fixed_rows(self):
        # rac1's func_L12_00304038: float v[count][4], a row of four floats per element.
        out = self.run_program({"a.c": """
            void row(float *r, int i) { r[0] = i; r[3] = i * 2; }
            int rows(int count) {
                float v[count][4];
                int i, s = 0;
                for (i = 0; i < count; i++) row(v[i], i);
                for (i = 0; i < count; i++) s += (int)v[i][0] + (int)v[i][3];
                return s;
            }
            int test_main(void) { test_print(rows(6)); test_print(rows(1)); return 0; }
        """})
        self.assertEqual(out, ["45", "0"])

    def test_variable_length_array_of_an_expression(self):
        # rac1's func_L10_002DBA50: char scratch[*(int *)(data + 4) * 0x40]. The length goes into a
        # local of its own on the same line, computed once where the array is declared.
        out = self.run_program({"a.c": """
            int fill(int *data) {
                char scratch[(unsigned int)*(data + 1) * 0x40];
                float rows[data[0] + 1][4], more[data[1] + 2];
                int i, s = 0;
                more[data[1] + 1] = 2;
                for (i = 0; i < data[1] * 0x40; i++) scratch[i] = (char)i;
                for (i = 0; i <= data[0]; i++) rows[i][3] = i;
                for (i = 0; i < data[1] * 0x40; i++) s += scratch[i];
                return s + (int)rows[data[0]][3] + (int)more[data[1] + 1];
            }
            int test_main(void) {
                int d[2];
                d[0] = 4; d[1] = 1;
                test_print(fill(d));
                return 0;
            }
        """})
        self.assertEqual(out, [str(sum(range(64)) + 6)])

    def test_a_macro_whose_body_casts_to_an_unnamed_record(self):
        # rac1's help_0020CDF0.c (`#define DEFA (((struct { T a[1]; } *)D)->a)`) and
        # vendor_002C0310.c (`#define S_I(p, o) (((struct { char _p[o]; int v; } *)(p))->v)`):
        # written out where they are used, the records are named in each function.
        out = self.run_program({"a.c": """
            typedef struct { int x; short y; } T;
            #define FIRST (((struct { T a[1]; } *)table)->a)
            #define AT(p, o) (((struct { char _p[o]; int v; } *)(p))->v)
            int test_main(void) {
                int table[8];
                FIRST[1].x = 7;
                AT(table, 4) = 3;
                test_print(table[2] + table[1]);
                test_print(AT((char *)table, 8));
                return 0;
            }
        """})
        self.assertEqual(out, ["10", "7"])

    def test_cast_macros_keep_lines_and_skip_other_directives(self):
        src = ("#define AT(p, o) (((struct { char _p[o]; int v; } *)(p))->v)\n"
               "int f(char *s) { return AT(s, 8) + AT(s,\n"
               "   12); }\n"
               "#define B AT(x, 4)\n"
               "#undef AT\n"
               "int g(char *s) { return AT(s, 4); }\n")
        out = prep.expand_cast_macros(src).split("\n")
        self.assertEqual(len(out), len(src.split("\n")))
        self.assertEqual(out[1], "int f(char *s) { return (((struct { char _p[8]; int v; } *)(s))->v) + AT(s,")
        self.assertEqual(out[3], "#define B AT(x, 4)")
        self.assertEqual(out[5], "int g(char *s) { return AT(s, 4); }")

    def test_alloca_takes_game_stack_until_the_function_returns(self):
        # rac1's func_L13_002EE590: __builtin_alloca for a copy of a list. The space is in game
        # memory (its address is passed on) and is given back when the function returns.
        out = self.run_program({"a.c": """
            int sum(int *p, int n) { int s = 0; while (n--) s += *p++; return s; }
            int copy(int count) {
                int *list;
                int i;
                if (count == 0) return -1;
                list = __builtin_alloca(count * 4);
                for (i = 0; i < count; i++) list[i] = i + 1;
                return sum(list, count);
            }
            int test_main(void) {
                int i, s = 0;
                for (i = 0; i < 4000; i++) s = copy(100);  /* 400 bytes each: 1.6 MB if kept */
                test_print(s);
                test_print(copy(0));
                return 0;
            }
        """})
        self.assertEqual(out, ["5050", "-1"])

    def test_libraries_bind_to_shared_replacements(self):
        # The game's table binds an address to a shared replacement by name;
        # hostgen writes the binding (libraries.c) with the shared signature.
        out = self.run_program({"a.c": """
            extern int func_00120000(char *, int);
            int test_main(void) { test_print(func_00120000("ratchet", 3)); return 0; }
        """}, config={"libraries": "libraries.tsv", "library_api": "api.tsv"}, extra_files={
            "libraries.tsv": "# symbol\tlibrary\tname\tport\nfunc_00120000\tlibc\tstrnlen\thost\n",
            "api.tsv": "# name\tlibrary\tsignature\nstrnlen\tlibc\tint (char *, int)\n",
        }, host_c="""
            int openrac_lib_strnlen(gaddr s, int n) {
                int i = 0;
                while (i < n && ((char *)G(s))[i]) i++;
                return i + 100;
            }
        """)
        self.assertEqual(out, ["103"])

    def test_hook_points_run_the_game_until_a_mod_takes_them(self):
        # A `hook` row (docs/port/MODS.md): the C is kept as <name>__game, hooks.c writes <name>,
        # which calls the slot a mod filled or else the game's C, and lists it with its signature.
        out = self.run_program({"a.c": """
            extern void test_take(void);
            int func_00100000(int x, float f) { return x * 2 + (int)f; }
            int test_main(void) {
                test_print(func_00100000(5, 1.0f));
                test_take();
                test_print(func_00100000(5, 1.0f));
                return 0;
            }
        """}, places="func_00100000\t0x00100000\t0x40\n",
            config={"libraries": "libraries.tsv", "host_functions": HOST_FUNCTIONS + ["test_take"]},
            extra_files={"libraries.tsv": "# symbol\tlibrary\tname\tport\nfunc_00100000\tgame\tTwice_Plus\thook\n"},
            host_c="""
            #include <string.h>
            typedef struct { const char* name; const char* symbol; const char* signature; void** slot; void* original; } point;
            extern const point openrac_game_hook_points[];
            extern const int openrac_game_hook_point_count;
            void game_test_print(int v);
            static int (*s_original)(int, float);
            static int replacement(int x, float f) { return 1000 + s_original(x, f); }
            void game_test_take(void) {
                const point* p = &openrac_game_hook_points[0];
                game_test_print(openrac_game_hook_point_count);
                game_test_print(strcmp(p->name, "Twice_Plus") == 0 && strcmp(p->signature, "i32(i32,f32)") == 0);
                s_original = (int (*)(int, float))p->original;
                *p->slot = (void*)replacement;
            }
        """)
        self.assertEqual(out, ["11", "1", "1", "1011"])

    def test_a_hook_point_under_a_wrap_of_the_ports(self):
        # A function the port wraps (`wrap`) and mods hook (`hook`): the port's <name> stays, and
        # calls <name>__hooked, which hooks.c writes: the mod's replacement, or else the game's C.
        out = self.run_program({"a.c": """
            extern void test_take(void);
            int func_00100000(int x) { return x * 2; }
            int test_main(void) {
                test_print(func_00100000(5));
                test_take();
                test_print(func_00100000(5));
                return 0;
            }
        """}, places="func_00100000\t0x00100000\t0x40\n",
            config={"libraries": "libraries.tsv", "host_functions": HOST_FUNCTIONS + ["test_take"]},
            extra_files={"libraries.tsv": "# symbol\tlibrary\tname\tport\n"
                                          "func_00100000\tgame\tTwice\twrap\n"
                                          "func_00100000\tgame\tTwice\thook\n"},
            host_c="""
            typedef struct { const char* name; const char* symbol; const char* signature; void** slot; void* original; } point;
            extern const point openrac_game_hook_points[];
            void game_test_print(int v);
            int func_00100000__hooked(int x);
            int func_00100000(int x) { return 1 + func_00100000__hooked(x); }  /* the port's wrap */
            static int (*s_original)(int);
            static int replacement(int x) { return 100 + s_original(x); }
            void game_test_take(void) {
                const point* p = &openrac_game_hook_points[0];
                s_original = (int (*)(int))p->original;
                *p->slot = (void*)replacement;
            }
        """)
        self.assertEqual(out, ["11", "111"])

    def test_two_hook_points_of_one_name_are_refused(self):
        with tempfile.TemporaryDirectory() as tmp:
            table = Path(tmp) / "libraries.tsv"
            table.write_text("func_00100000\tx\tSame\thook\nfunc_00100100\tx\tSame\thook\n")
            with self.assertRaises(ValueError):
                hostgen.hooked(Path(tmp), {"libraries": "libraries.tsv"})


    def test_literal_addresses_reach_the_loaded_levels_globals(self):
        # The NTSC decomp's render_capture_effect_pass read the vendor's salesman flag through
        # *(s32 *)0x1610A8: a literal address in the range the levels move is the same global a
        # name at that address is (OPENRAC_DATA), not the executable's copy under the level. The
        # host's relocation here moves the range up by 0x1000, as a loaded level would.
        out = self.run_program({"a.c": """
            typedef struct { int a; int b; } Pair;
            extern int D_00300010[2];
            extern Pair D_00300100;
            int test_main(void) {
                int *p = (int *)0x00300010;
                *(s32 *)0x00300010 = 5;
                ((int *)0x00300010)[1] = 6;
                ((Pair *)0x00300100)->b = 7;
                *(s32 *)(0x00300100 + 0) = 8;
                *(s32 *)0x00200000 = 9;               /* below the range: as it is */
                test_print(D_00300010[0]);
                test_print(D_00300010[1]);
                test_print(D_00300100.b);
                test_print(D_00300100.a);
                test_print(p[1]);
                test_print(*(s32 *)0x00201000);       /* nothing went to the executable's copy */
                test_print(*(s32 *)0x00200000);
                return 0;
            }
        """}, config={"relocate": [0x300000, 0x400000]}, host_c="""
            gaddr openrac_relocate_low = 0x300000, openrac_relocate_high = 0x400000;
            gaddr openrac_relocate_data(gaddr a) { return a + 0x1000; }
            gaddr openrac_relocate_code(gaddr a) { return a + 0x1000; }
        """)
        self.assertEqual(out, ["5", "6", "7", "8", "6", "0", "9"])
        self.assertEqual(sorted(self.last_report["literal_addresses"]["test_main"]), [
            "0x300010 -> OPENRAC_DATA(0x00300010u)", "0x300010 -> OPENRAC_DATA(0x00300010u)",
            "0x300010 -> OPENRAC_DATA(0x00300010u)", "0x300100 -> OPENRAC_DATA(0x00300100u)",
            "0x300100 -> OPENRAC_DATA(0x00300100u)"])

    def test_a_separate_program_in_its_own_group(self):
        # Up Your Arsenal's menu is a program of its own, loaded over part of
        # the executable: the same address names another function there.
        out = self.run_program({
            "exe.c": """
                int func_00110000(void) { return 1; }
                extern int menu_main(void);
                int test_main(void) { test_print(func_00110000()); test_print(menu_main()); return 0; }
            """,
            "menu/menu.c": """
                int func_00110000(void) { return 2; }
                int menu_main(void) { return func_00110000() * 10; }
            """,
        }, config={"groups": [{"path": "src/menu", "suffix": "menu", "overlay": 100}]})
        self.assertEqual(out, ["1", "20"])

    def test_split_places_twin_counts_with_the_variants_calls(self):
        # The catalogue folds the level copy of an executable function (func_00110080, at 0x300080
        # in level 3) into a folded function (func_00110000) as a variant; engine code calls the
        # executable function by its own name before the folded one (rac1/pal's frame draw: packet
        # calls, then AA_BlurPass). split_places.tsv's fourth column names the twin, so the folded
        # call is the caller's second call to the family (k = 1), not its first.
        rows = "func_00110000\tfunc_L03_00300080\t03:00300080{}\n"
        program = {"a.c": """
            void func_00110000(void) {}
            void func_00110080(void) {}
            void func_L03_00300080(void) {}
            void func_00110100(void) { func_00110080(); func_00110000(); }
            int test_main(void) { test_print(0); return 0; }
        """}
        places = ("func_00110000\texe\t8\t0\t1\t03:00300000,03:00300080\n"
                  "func_00110100\texe\t16\t0\t1\t03:00300100\n")
        for twin, k in (("", 0), ("\tfunc_00110080", 1)):
            out = self.run_program(program, places=places, config={"split_places": "split_places.tsv"},
                                   extra_files={"split_places.tsv": "# function\tvariant\tplaces\ttwin\n"
                                                + rows.format(twin)},
                                   host_c="""
                                       gaddr openrac_guest_nearest(const uint32_t *p, int n, gaddr c) { return 0; }
                                       gaddr openrac_folded_call(const uint32_t *p, int n, gaddr c, uint32_t s, int k) { return 0; }
                                   """)
            self.assertEqual(out, ["0"])
            calls = re.findall(r"openrac_folded_call\(\(const uint32_t\[\]\)\{0x00110000u, [^;]*?, (\d+)\)\)",
                               self.last_generated)
            self.assertEqual(calls, [str(k)], twin or "no twin")


class InputsDigest(unittest.TestCase):
    """The fingerprint that lets a build skip a translation it already has."""

    def test_changes_with_what_hostgen_reads(self):
        with tempfile.TemporaryDirectory() as tmp:
            source, game = Path(tmp) / "decomp", Path(tmp) / "game" / "g"
            (source / "src").mkdir(parents=True)
            (source / "include").mkdir()
            (source / "asm").mkdir()
            game.mkdir(parents=True)
            (source / "src" / "a.c").write_text("int a(void) { return 1; }\n")
            (source / "asm" / "big.s").write_text("nop\n")
            config = game / "hostgen.json"
            config.write_text("{}")
            cfg = {"sources": ["src"], "includes": ["include"], "symbols": []}
            first = hostgen.inputs_digest(source, config, cfg, [])
            self.assertEqual(first, hostgen.inputs_digest(source, config, cfg, []))
            # What it does not read leaves it as it is.
            (source / "asm" / "big.s").write_text("nop\nnop\n")
            self.assertEqual(first, hostgen.inputs_digest(source, config, cfg, []))
            # The C, the game's configuration and the options change it.
            (source / "src" / "a.c").write_text("int a(void) { return 2; }\n")
            second = hostgen.inputs_digest(source, config, cfg, [])
            self.assertNotEqual(first, second)
            config.write_text('{"x": 1}')
            self.assertNotEqual(second, hostgen.inputs_digest(source, config, cfg, []))
            self.assertNotEqual(
                hostgen.inputs_digest(source, config, cfg, []), hostgen.inputs_digest(source, config, cfg, ["src/a.c"])
            )

    def test_changes_with_the_assembly_and_disc_it_translates(self):
        # With the assembly stage on, the assembly the setup wrote and the player's disc are
        # inputs too: a new setup, or a disc set up after a build, must not reuse the old C.
        with tempfile.TemporaryDirectory() as tmp:
            source, game = Path(tmp) / "decomp", Path(tmp) / "game" / "g"
            (source / "src").mkdir(parents=True)
            (source / "asm").mkdir()
            game.mkdir(parents=True)
            (source / "src" / "a.c").write_text("int a(void) { return 1; }\n")
            (source / "asm" / "f.s").write_text("nop\n")
            disc = Path(tmp) / "disc.iso"
            disc.write_bytes(b"\0" * 16)
            config = game / "hostgen.json"
            config.write_text("{}")
            cfg = {"sources": ["src"], "includes": [], "symbols": [],
                   "asm": {"enabled": True, "dirs": ["asm"]}}
            first = hostgen.inputs_digest(source, config, cfg, [])
            (source / "asm" / "f.s").write_text("nop\nnop\n")
            second = hostgen.inputs_digest(source, config, cfg, [])
            self.assertNotEqual(first, second)
            off = {**cfg, "asm": {**cfg["asm"], "enabled": False}}
            self.assertNotEqual(second, hostgen.inputs_digest(source, config, off, []))
            with_disc = {**cfg, "asm": {**cfg["asm"], "retail": {"image": str(disc)}}}
            third = hostgen.inputs_digest(source, config, with_disc, [])
            self.assertNotEqual(second, third)
            disc.write_bytes(b"\0" * 32)
            self.assertNotEqual(third, hostgen.inputs_digest(source, config, with_disc, []))


class SupersededDeclarations(unittest.TestCase):
    def test_header_declaration_a_definition_contradicts_takes_its_prototype(self):
        # Other files that take f from the header still have it, now with its real type.
        with tempfile.TemporaryDirectory() as d:
            root = Path(d)
            (root / "include").mkdir()
            (root / "include/h.h").write_text("int keep;\nextern void f(void *, int);\nint tail;\n")
            text = '#include "include/h.h"\nvoid f(int *p, int n) {\n}\n'
            err = ("a.c:2:6: error: conflicting types for 'f'\n"
                   "include/h.h:2:13: note: previous declaration is here\n")
            done = clangast.drop_superseded_declarations(root, "a.c", text, err)
            self.assertEqual(len(done), 1)
            self.assertEqual((root / "include/h.h").read_text(), "int keep;\nvoid f(int *p, int n);\nint tail;\n")

    def test_a_definition_over_several_lines_still_drops_the_declaration(self):
        with tempfile.TemporaryDirectory() as d:
            root = Path(d)
            (root / "include").mkdir()
            (root / "include/h.h").write_text("extern void f(void *, int);\n")
            text = '#include "include/h.h"\nvoid f(int *p,\n       int n) {\n}\n'
            err = ("a.c:2:6: error: conflicting types for 'f'\n"
                   "include/h.h:1:13: note: previous declaration is here\n")
            clangast.drop_superseded_declarations(root, "a.c", text, err)
            self.assertEqual((root / "include/h.h").read_text(), "\n")


class CandidateStatics(unittest.TestCase):
    def test_two_candidates_keep_their_own_static_helpers(self):
        import prep
        a = "static int sign(float f) { return f < 0; }\nint fa(float x) { return sign(x); }\n"
        b = "static int sign(float f) { return f <= 0; }\nstatic int table[2];\nint fb(float x) { return sign(x) + table[0]; }\n"
        a2, b2 = prep.own_statics(a, "_hc1"), prep.own_statics(b, "_hc2")
        self.assertIn("static int sign_hc1(float f)", a2)
        self.assertIn("return sign_hc1(x)", a2)
        self.assertIn("static int sign_hc2(float f)", b2)
        self.assertIn("static int table_hc2[2]", b2)
        self.assertIn("table_hc2[0]", b2)
        self.assertIn("int fb(float x)", b2)


class SharedLibraries(unittest.TestCase):
    def test_header_and_table_agree(self):
        game = HERE.parent.parent / "game" / "common"
        names = [line.split("\t")[0] for line in (game / "libraries.tsv").read_text().splitlines()
                 if line.strip() and not line.startswith("#")]
        header = (game / "include" / "openrac" / "game_lib.h").read_text()
        for name in names:
            with self.subTest(name):
                self.assertIn(f" openrac_lib_{name}(", header)
        self.assertEqual(header.count(" openrac_lib_"), len(names))


class Places(unittest.TestCase):
    def test_an_executable_function_the_levels_carry_keeps_its_address(self):
        import re
        from program import Program
        program = Program()
        program.code_names = [re.compile(r"^func_(?:L(?P<overlay>\d\d)_)?(?P<addr>[0-9A-Fa-f]{8})$")]
        program.places["func_001E94E8"] = [(0, 0x001EB1B0), (1, 0x0020CD30)]
        program.places["func_L00_001EB0B0"] = [(0, 0x001EB0B0), (1, 0x0020CCB0)]
        self.assertEqual(program.code_places("func_001E94E8"),
                         [(-1, 0x001E94E8), (0, 0x001EB1B0), (1, 0x0020CD30)])
        self.assertEqual(program.code_places("func_L00_001EB0B0"), [(0, 0x001EB0B0), (1, 0x0020CCB0)])
        self.assertEqual(program.code_places("func_00112380"), [(-1, 0x00112380)])

    @unittest.skipUnless(have_compiler(), "needs Clang on a POSIX host")
    def test_shared_level_data_uses_the_games_relocation_range(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            src = root / "game"
            (src / "src").mkdir(parents=True)
            (src / "src" / "a.c").write_text(
                "extern int D_L00_00410000;\n"
                "int func_L00_00420000(void) { return D_L00_00410000; }\n")
            (src / "functions.tsv").write_text(
                "func_L00_00420000\tshared\t4\tfingerprint\t2\t00:00420000,01:00430000\n")
            (root / "hostgen.json").write_text(json.dumps({
                "sources": ["src"], "includes": [], "places": "functions.tsv",
                "relocate": [0x1A7C80, 0x500000],
            }))
            out = root / "gen"
            self.assertEqual(hostgen.main(["--game", str(root / "hostgen.json"),
                                           "--source", str(src), "--out", str(out),
                                           "--jobs", "1", "--quiet"]), 0)
            self.assertIn("OPENRAC_LDATA(0, 0x00410000u)", (out / "a.c").read_text())

    @unittest.skipUnless(have_compiler(), "needs Clang on a POSIX host")
    def test_a_data_label_with_an_offset_is_the_symbol_plus_it(self):
        # rac1/ntsc's l16 00214598.c: extern f32 D_L16_0017C224[] __asm__("D_L16_0017C218+12").
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            src = root / "game"
            (src / "src").mkdir(parents=True)
            (src / "src" / "a.c").write_text(
                "extern float row[] __asm__(\"D_L00_00410000+12\");\n"
                "extern float back[] __asm__(\"D_00200010 - 0x10\");\n"
                "float func_L00_00420000(void) { return row[1] + back[0]; }\n")
            (src / "functions.tsv").write_text(
                "func_L00_00420000\tshared\t4\tfingerprint\t2\t00:00420000,01:00430000\n")
            (root / "hostgen.json").write_text(json.dumps({
                "sources": ["src"], "includes": [], "places": "functions.tsv",
                "relocate": [0x1A7C80, 0x500000],
            }))
            out = root / "gen"
            self.assertEqual(hostgen.main(["--game", str(root / "hostgen.json"),
                                           "--source", str(src), "--out", str(out),
                                           "--jobs", "1", "--quiet"]), 0)
            text = (out / "a.c").read_text()
            self.assertIn("OPENRAC_LDATA(0, 0x0041000Cu)", text)
            self.assertIn("0x00200000u", text)
            self.assertNotIn("openrac_stub", text)

    @unittest.skipUnless(have_compiler(), "needs Clang on a POSIX host")
    def test_code_declared_as_data_is_relocated_as_code(self):
        # rac1's space.c: extern u8 resident_effect_entry[] __asm__("func_0022F4C0"), passed on as
        # resident_effect_entry + 8, the entry of the function the catalogue starts 8 bytes early.
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            src = root / "game"
            (src / "src").mkdir(parents=True)
            (src / "src" / "a.c").write_text(
                "typedef unsigned char u8;\n"
                "extern u8 entry[] __asm__(\"func_00300000\");\n"
                "extern void func_00320000(void *);\n"
                "void func_00310000(void) { func_00320000(entry + 8); func_00320000(entry); }\n")
            (root / "hostgen.json").write_text(json.dumps({
                "sources": ["src"], "includes": [],
                "relocate": [0x1A7C80, 0x500000],
            }))
            out = root / "gen"
            self.assertEqual(hostgen.main(["--game", str(root / "hostgen.json"),
                                           "--source", str(src), "--out", str(out),
                                           "--jobs", "1", "--quiet"]), 0)
            text = (out / "a.c").read_text()
            self.assertIn("OPENRAC_CODE((gaddr)(0x00300000u) + (gaddr)(8)", text)
            self.assertIn("OPENRAC_CODE(0x00300000u)", text)
            self.assertNotIn("OPENRAC_DATA", text)

    @unittest.skipUnless(have_compiler(), "needs Clang on a POSIX host")
    def test_literal_addresses_take_their_globals_relocation(self):
        # A literal address goes the way a D_ name at that address would: the loaded level's in an
        # executable function, the level's own copy in a function several levels carry, as it is
        # in a function of one level; a function pointer as code.
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            src = root / "game"
            (src / "src").mkdir(parents=True)
            (src / "src" / "a.c").write_text(
                "typedef void (*fn_t)(void);\n"
                "int func_00300000(void) { return *(int *)0x00410000 + *(int *)0x00100000; }\n"
                "void func_00300100(void) { ((fn_t)0x00320000)(); }\n"
                "int func_L00_00420000(void) { return *(int *)0x00410004; }\n"
                "void func_L00_00420100(void) { *(int *)0x00410008 = 1; }\n")
            (src / "functions.tsv").write_text(
                "func_L00_00420000\tshared\t4\tfingerprint\t2\t00:00420000,01:00430000\n"
                "func_L00_00420100\tl00\t4\tfingerprint\t1\t00:00420100\n")
            (root / "hostgen.json").write_text(json.dumps({
                "sources": ["src"], "includes": [], "places": "functions.tsv",
                "relocate": [0x1A7C80, 0x500000],
            }))
            out = root / "gen"
            self.assertEqual(hostgen.main(["--game", str(root / "hostgen.json"),
                                           "--source", str(src), "--out", str(out),
                                           "--jobs", "1", "--quiet"]), 0)
            text = (out / "a.c").read_text()
            self.assertIn("OPENRAC_DATA(0x00410000u)", text)
            self.assertNotIn("OPENRAC_DATA(0x00100000u)", text)
            self.assertIn("OPENRAC_CODE(0x00320000u)", text)
            self.assertIn("OPENRAC_LDATA(0, 0x00410004u)", text)
            self.assertNotIn("(0, 0x00410008u)", text)
            report = json.loads((out / "report.json").read_text())
            self.assertEqual(report["literal_addresses"], {
                "func_00300000": ["0x410000 -> OPENRAC_DATA(0x00410000u)"],
                "func_00300100": ["0x320000 -> OPENRAC_CODE(0x00320000u)"],
                "func_L00_00420000": ["0x410004 -> OPENRAC_LDATA(0, 0x00410004u)"],
                "func_L00_00420100": ["0x410008 -> kept"],
            })

    @unittest.skipUnless(have_compiler(), "needs Clang on a POSIX host")
    def test_small_folded_executable_function_forms_its_own_globals(self):
        # rac1's func_0023BF48 (movie.c, printf on a buffer of its own) is also, by its instructions,
        # a Novalis moby's update, whose first argument is a draw callback of that level.
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            src = root / "game"
            (src / "src").mkdir(parents=True)
            (src / "src" / "a.c").write_text(
                "extern char D_00200000[];\n"
                "extern void func_00320000(void *, char *);\n"
                "void func_00300000(char *fmt) { func_00320000(D_00200000, fmt); }\n")
            (src / "functions.tsv").write_text(
                "func_00300000\texe\t36\tfingerprint\t2\t00:00440000,01:00450000,01:00460000\n")
            (root / "hostgen.json").write_text(json.dumps({
                "sources": ["src"], "includes": [], "places": "functions.tsv",
                "relocate": [0x1A7C80, 0x500000],
            }))
            out = root / "gen"
            self.assertEqual(hostgen.main(["--game", str(root / "hostgen.json"),
                                           "--source", str(src), "--out", str(out),
                                           "--jobs", "1", "--quiet"]), 0)
            text = (out / "a.c").read_text()
            self.assertIn("openrac_data_in_copy(-1, 0x00300000u, 36u, openrac_self_, 0x00200000u)", text)
            self.assertIn("openrac_guest_entry((openrac_host_fn)func_00300000)", text)

    @unittest.skipUnless(have_compiler(), "needs Clang on a POSIX host")
    def test_a_folded_call_of_shared_code_with_copies_in_one_program_is_the_running_copys(self):
        # rac1's func_L00_002E0490 (several levels' code) has two copies in level 7's program, and
        # each registers its draw callback through its own place of the folded AddDrawCallback.
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            src = root / "game"
            (src / "src").mkdir(parents=True)
            (src / "src" / "a.c").write_text(
                "extern void func_L00_00320000(int);\n"
                "void func_L00_00300000(int a) { func_L00_00320000(a); }\n")
            (src / "functions.tsv").write_text(
                "func_L00_00300000\tshared\t104\tfingerprint\t3\t00:00300000,07:00310000,07:00310100\n"
                "func_L00_00320000\tshared\t40\tfingerprint\t3\t00:00320000,07:00330000,07:00330040\n")
            (root / "hostgen.json").write_text(json.dumps({
                "sources": ["src"], "includes": [], "places": "functions.tsv",
                "relocate": [0x1A7C80, 0x500000],
            }))
            out = root / "gen"
            self.assertEqual(hostgen.main(["--game", str(root / "hostgen.json"),
                                           "--source", str(src), "--out", str(out),
                                           "--jobs", "1", "--quiet"]), 0)
            text = (out / "a.c").read_text()
            self.assertIn("(openrac_self_ != 0 ? openrac_self_ : OPENRAC_LCODE(0, 0x00300000u)), 104u, 0)", text)
            self.assertIn("openrac_guest_entry((openrac_host_fn)func_L00_00300000)", text)

    def generate(self, root: Path, source: str, places: str, cfg: dict | None = None) -> Path:
        src = root / "game"
        (src / "src").mkdir(parents=True)
        (src / "src" / "a.c").write_text(source)
        (src / "functions.tsv").write_text(places)
        (root / "hostgen.json").write_text(json.dumps({
            "sources": ["src"], "includes": [], "places": "functions.tsv", **(cfg or {})}))
        out = root / "gen"
        self.assertEqual(hostgen.main(["--game", str(root / "hostgen.json"), "--source", str(src),
                                       "--out", str(out), "--jobs", "1", "--quiet"]), 0)
        return out

    def test_the_address_of_one_place_of_a_folded_wrapper_is_that_place(self):
        # rac1's Suck Cannon passes func_L00_00238128, the second of the five level 0 wrappers the
        # catalogue folds into func_L00_00237B70, as a HUD callback: each wrapper calls another
        # function, so the address must be that wrapper's, not the folded function's first.
        with tempfile.TemporaryDirectory() as tmp:
            out = self.generate(Path(tmp),
                "void func_L00_00237B70(void) {}\n"
                "extern void cb(void) __asm__(\"func_L00_00238128\");\n"
                "extern void func_00300000(int);\n"
                "void func_L00_00240000(void) { func_00300000((int)cb); func_00300000((int)func_L00_00237B70); }\n",
                "func_L00_00237B70\tshared\t28\tfp\t2\t00:00237B70,00:00238128,01:00337B70,01:00338128\n"
                "func_L00_00240000\tlevel\t64\tfp2\t1\t00:00240000\n")
            text = (out / "a.c").read_text()
            self.assertIn("0x00238128u", text)
            self.assertIn("0x00237B70u", text)

    def test_the_address_of_a_folded_wrapper_is_the_one_the_code_forms(self):
        # rac1's Umbris sea classes register func_00216270 as their draw callback: the catalogue
        # folds 32-byte wrappers that each call another function into that name, and each class's
        # own wrapper is a later place in the level, not the first. The address is found in the
        # caller's code at run time, the first place when it forms none.
        with tempfile.TemporaryDirectory() as tmp:
            out = self.generate(Path(tmp),
                "void func_L07_0028E068(void) {}\n"
                "extern void func_00300000(void (*)(void), void *);\n"
                "void func_L07_002F9438(char *m) { func_00300000(func_L07_0028E068, m); }\n",
                "func_L07_0028E068\tshared\t32\tfw\t2\t07:0028E068,07:002F9418,08:0028F000\n"
                "func_L07_002F9438\tshared\t64\tfu\t2\t07:002F9438,08:002FA000\n",
                {"relocate": [0x15F000, 0x500000]})
            text = (out / "a.c").read_text()
            self.assertIn("openrac_folded_address((const uint32_t[]){", text)
            self.assertIn("0x082F9418u", text)
            self.assertIn("0x0828E068u", text)

    def test_a_larger_function_placed_twice_in_a_level_calls_its_copys_own_callee(self):
        # rac1's func_L02_002D75F8 (76 bytes) is two of Eudora's circular saw hooks: 04:002C53B0
        # calls the grab (func_L00_002DCDA8 there), 04:002C5400 the placing. Each copy's call goes
        # where that copy's own jal goes, as in a folded wrapper of 64 bytes or less; a callee the
        # function's own level does not have stays a direct call.
        with tempfile.TemporaryDirectory() as tmp:
            out = self.generate(Path(tmp),
                "extern int func_L00_002DCDA8(void *, void *, void *);\n"
                "extern int func_00300000(void *);\n"
                "int func_L02_002D75F8(char *a, void *p, void *t) {\n"
                "    int r = func_L00_002DCDA8(a, p, t);\n"
                "    if (r == 0) { if (a[0x20] == 9) a[0x20] = 1; } else a[0x20] = 9;\n"
                "    return r + func_00300000(a);\n"
                "}\n",
                "func_L00_002DCDA8\tshared\t400\tfg\t3\t00:002DCDA8,02:002E7EC8,04:002DF068\n"
                "func_L02_002D75F8\tshared\t76\tfw\t2\t02:002D75F8,02:002D76C8,04:002C53B0,04:002C5400\n",
                {"relocate": [0x15F000, 0x500000]})
            text = (out / "a.c").read_text()
            self.assertIn("openrac_call_in_copy(2, 0x002D75F8u, 76u, openrac_self_, 0x002E7EC8u)", text)
            self.assertIn("openrac_guest_entry((openrac_host_fn)func_L02_002D75F8)", text)
            self.assertIn("func_00300000((gaddr)(a))", text)

    def test_a_call_to_another_levels_function_follows_the_definition(self):
        # rac1/ntsc's Rilgar creature (FUN_L05_002d1688) declared level 0's vector scale as
        # (float, void *, void *) and called it through its level 0 address with that type: right
        # on the EE and System V, where each kind has registers of its own; on Windows x64 the
        # scale wrote through a pointer read from a register nobody set (0.8.0's crash). The
        # call goes through the definition's type, each kind's arguments in order.
        with tempfile.TemporaryDirectory() as tmp:
            out = self.generate(Path(tmp),
                "void func_L00_001FF500(float *out, float *a, float len) { out[0] = a[0] * len; }\n"
                "extern void scale(float, void *, void *) __asm__(\"func_L00_001FF500\");\n"
                "extern float steer(float, void *, void *) __asm__(\"func_L00_0025BE00\");\n"
                "float func_L00_0025BE00(void *a, void *b, float x) { return x; }\n"
                "float func_L05_002D1688(char *moby, float len) {\n"
                "    float w[4];\n"
                "    scale(len, w, moby + 0xC0);\n"
                "    return steer(len, moby, w);\n"
                "}\n",
                "func_L00_001FF500\tshared\t64\tfa\t1\t00:001FF500\n"
                "func_L00_0025BE00\tshared\t64\tfc\t1\t00:0025BE00\n"
                "func_L05_002D1688\tlevel\t64\tfb\t1\t05:002D1688\n")
            text = (out / "a.c").read_text()
            self.assertIn("GFN(void (*)(gaddr, gaddr, float), OPENRAC_LCODE(0, 0x001FF500u))(", text)
            self.assertNotIn("GFN(void (*)(float, gaddr, gaddr)", text)
            self.assertIn("GFN(float (*)(gaddr, gaddr, float), OPENRAC_LCODE(0, 0x0025BE00u))(", text)
            import abicheck
            problems, stats = abicheck.check(out)
            self.assertEqual(problems, [])
            self.assertEqual(stats["known"], 2)

    def test_abicheck_finds_a_call_whose_positions_differ(self):
        with tempfile.TemporaryDirectory() as tmp:
            gen = Path(tmp)
            (gen / "game_protos.h").write_text(
                "void func_L00_001FF500(gaddr a0, gaddr a1, float a2);\nint func_00100000(gaddr a0);\n")
            (gen / "functions.c").write_text(
                "static const openrac_fn_entry k_exe[] = {\n"
                "    {0x00100000u, (openrac_host_fn)func_00100000, \"func_00100000\"},\n};\n"
                "static const openrac_fn_entry k_overlay_00[] = {\n"
                "    {0x001FF500u, (openrac_host_fn)func_L00_001FF500, \"func_L00_001FF500\"},\n};\n")
            (gen / "a.c").write_text(
                "void func_L05_002D1688(gaddr moby)\n{\n"
                "    GFN(void (*)(float, gaddr, gaddr), OPENRAC_LCODE(0, 0x001FF500u))(1.0f, moby, moby);\n"
                "    GFN(void (*)(gaddr, gaddr, float), OPENRAC_LCODE(0, 0x001FF500u))(moby, moby, 1.0f);\n"
                "    float x = GFN(float (*)(gaddr), OPENRAC_CODE(0x00100000u))(moby);\n"
                "    GFN(float (*)(gaddr), OPENRAC_CODE(0x00100000u))(moby);\n"
                "    GFN(void (*)(float), GREF(gaddr, moby))(x);\n"
                "}\n")
            import abicheck
            problems, stats = abicheck.check(gen)
            self.assertEqual(len(problems), 2, problems)
            self.assertIn("a.c:3:", problems[0])
            self.assertIn("argument 1", problems[0])
            self.assertIn("a.c:5:", problems[1])
            self.assertIn("reads a float result, returns int", problems[1])
            self.assertEqual(stats, {"calls": 5, "known": 4, "unknown": 1})

    def test_an_entry_inside_a_catalogue_place_answers_calls(self):
        # hostgen.json "entries": rac1's HUD gauge draw starts 0x20 bytes into its catalogue place.
        with tempfile.TemporaryDirectory() as tmp:
            out = self.generate(Path(tmp),
                "int func_L06_0024A080(char *p) { return p[0]; }\n",
                "func_L06_0024A080\tshared\t940\tfp\t2\t06:0024A080,07:00262970\n",
                {"entries": {"func_L06_0024A080": "0x20"}})
            text = (out / "functions.c").read_text()
            for addr in ("0x0024A080u", "0x0024A0A0u", "0x00262970u", "0x00262990u"):
                self.assertIn(f"{{{addr}, (openrac_host_fn)func_L06_0024A080", text)

    @unittest.skipUnless(have_compiler(), "needs Clang on a POSIX host")
    def test_a_copys_call_keeps_a_returns_64_result_whole(self):
        # rac1's EnvMapA_Draw (Kalebo, two copies) passes GetEffectTex's 64-bit TEX0 to
        # VU1_addGSregister; its unit declares GetEffectTex int. Called through the running copy's
        # place, the call keeps the 64 bits a direct call keeps (hostgen.json "returns_64").
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            src = root / "game"
            (src / "src").mkdir(parents=True)
            (src / "src" / "a.c").write_text(
                "extern int func_L00_00320000(int);\n"
                "extern void func_00330000(int, long long);\n"
                "void func_L00_00300000(void) { func_00330000(6, func_L00_00320000(41)); }\n")
            (src / "src" / "b.c").write_text(
                "unsigned long long func_L00_00320000(int i) { return (unsigned long long)i << 40; }\n")
            (src / "functions.tsv").write_text(
                "func_L00_00300000\tshared\t76\tfingerprint\t3\t00:00300000,00:00300050,01:00310000\n"
                "func_L00_00320000\tshared\t40\tfingerprint\t2\t00:00320000,01:00340000\n")
            (root / "hostgen.json").write_text(json.dumps({
                "sources": ["src"], "includes": [], "places": "functions.tsv",
                "relocate": [0x1A7C80, 0x500000], "returns_64": ["func_L00_00320000"],
            }))
            out = root / "gen"
            self.assertEqual(hostgen.main(["--game", str(root / "hostgen.json"),
                                           "--source", str(src), "--out", str(out),
                                           "--jobs", "1", "--quiet"]), 0)
            text = (out / "a.c").read_text()
            self.assertIn("openrac_call_in_copy(0, 0x00300000u, 76u, openrac_self_, 0x00320000u)", text)
            self.assertIn("GFN(unsigned long long (*)(int", text)
            self.assertNotIn("GFN(int (*)(int", text)


class TypeStrings(unittest.TestCase):
    def test_declarators(self):
        cases = {
            "int": "int x",
            "char *": "gaddr x",
            "S [4]": "S x[4]",
            "void (*)(int)": "gaddr x",
            "int (*[4])(void)": "gaddr x[4]",
            "float *[3][2]": "gaddr x[3][2]",
            "struct (unnamed struct at src/a.c:3:9)": "struct openrac_anon_src_a_c_3_9 x",
            "union S::(anonymous at a.c:1:25)": "union openrac_anon_a_c_1_25 x",
            "enum (unnamed enum at src/a.c:5:2)": "int x",
            "void (int) __attribute__((noreturn))": "void x(int)",
            "struct Moby *[count]": "gaddr x[count]",
        }
        for text, want in cases.items():
            with self.subTest(text):
                self.assertEqual(ctype.declare(ctype.parse(text), "x"), want)

    def test_function_pointer_type(self):
        t = ctype.parse("int (float, int, ...)")
        self.assertEqual(ctype.fn_pointer(t), "int (*)(float, int, ...)")


class Preparation(unittest.TestCase):
    def test_readable_names_are_written_as_their_address_names(self):
        # rac1/pal's names.h: a definition under its readable name is still the matched C of
        # func_0022F4C8, so the port's own copy of that function steps aside.
        aliases = prep.read_aliases('#define DrawQuads    func_0022F4C8\n#define D_X D_00100000\n')
        self.assertEqual(aliases, {"DrawQuads": "func_0022F4C8", "D_X": "D_00100000"})
        src = ('#define DrawQuads func_0022F4C8\n'
               'void DrawQuads(char *m) { /* DrawQuads */ char *s = "DrawQuads"; DrawQuadsX(m); }\n'
               'INCLUDE_ASM("asm", func_0022F4C0);\n')
        out = prep.unalias(src, aliases)
        self.assertIn('#define DrawQuads func_0022F4C8\nvoid func_0022F4C8(char *m) { /* DrawQuads */ '
                      'char *s = "DrawQuads"; DrawQuadsX(m); }', out)
        both = 'void func_0022F4C0(void) {}\nvoid func_0022F4C8(char *m) {}\n'
        text = prep.use_candidates(out, {"func_0022F4C0": both}, set())
        self.assertEqual(text.count("void func_0022F4C8(char *m)"), 1)

    def test_carried_names_become_the_address_names_of_their_values(self):
        # Going Commando keeps rac1-decomp's names (rac1's addresses) for what it has not named;
        # its build's table of derived values says where each is in this game, macro bodies too.
        table = json.dumps({"values": {"r1_func_0011DB98": "0x0011f818", "r1_D_00158300": "0x00140300",
                                       "D_00115E38": "0x00115e38", "Fbbb7bee1_FUN_00282A38": "0x00282a38"}})
        aliases = prep.read_carried_aliases(table)
        self.assertEqual(aliases, {"r1_func_0011DB98": "func_0011F818", "r1_D_00158300": "D_00140300"})
        src = ('extern char r1_D_00158300[];\n'
               '#define SIFCMD ((char *) r1_D_00158300)\n'
               'void f(void) { r1_func_0011DB98(1, 2); /* r1_func_0011DB98 */ }\n')
        out = prep.unalias(src, aliases)
        self.assertIn('extern char D_00140300[];\n#define SIFCMD ((char *) D_00140300)\n', out)
        self.assertIn('func_0011F818(1, 2); /* r1_func_0011DB98 */', out)

    def test_a_table_of_derived_values_gives_symbols(self):
        program = hostgen.Program()
        with tempfile.TemporaryDirectory() as d:
            path = Path(d) / "placed.json"
            path.write_text(json.dumps({"values": {"BOOT_D_001395B8": "0x001395b8", "§.text": "0x1"}}))
            self.assertEqual(hostgen.read_symbols(path, program), 1)
        self.assertEqual(program.symbols, {"BOOT_D_001395B8": 0x001395B8})

    def test_each_declarator_keeps_its_own_assembler_name(self):
        # Batalia: several names in one extern statement each reach their own symbol, not the first's.
        src = ('extern float func_A(int),func_B(float), *D_C;\n'
               'void f(void) { func_B(1.0f); func_A(2); }\n')
        out = prep.own_declarations(src, "_hc1")
        self.assertIn('func_A_hc1(int) __asm__("func_A")', out)
        self.assertIn('func_B_hc1(float) __asm__("func_B")', out)
        self.assertIn('*D_C_hc1 __asm__("D_C")', out)
        self.assertIn("func_B_hc1(1.0f); func_A_hc1(2);", out)
        self.assertEqual(out.count("\n"), src.count("\n"))

    def test_a_labelled_va_list_library_name_is_renamed(self):
        # The US vfprintf: named like the library's, bound to a game address, taking a game pointer.
        src = ('s32 vfprintf(FILE *fp, const char *f, void *ap) __asm__("FUN_00116da8");\n'
               's32 vfprintf(FILE *fp, const char *f, void *ap) { return vfprintf(fp, f, ap); }\n'
               'char *s = "vfprintf(";\n')
        out = prep.rename_va_builtins(src)
        self.assertEqual(out.count("vfprintf_game("), 3)
        self.assertIn('"vfprintf("', out)
        self.assertEqual(prep.rename_va_builtins("int vfprintf(void *, const char *, void *);\n"),
                         "int vfprintf(void *, const char *, void *);\n")

    def test_commas_in_a_parameter_list_do_not_split(self):
        src = "extern int func_A(int, float);\n"
        self.assertEqual(prep.split_declarators(src), src)

    def test_long_constant_suffix_is_widened(self):
        # The EE's long is 64 bits: 1UL << 60 must stay a 64-bit shift.
        src = 'x = 1UL << 60; y = 0xFE00L; z = 3LU; w = 5ULL; s = "2L";'
        self.assertEqual(prep.widen_long(src),
                         'x = 1ULL << 60; y = 0xFE00LL; z = 3LLU; w = 5ULL; s = "2L";')

    def test_long_is_64_bit(self):
        src = 'long a; unsigned long b; long long c; char *s = "long"; /* long */'
        self.assertEqual(prep.widen_long(src),
                         'long long a; unsigned long long b; long long c; char *s = "long"; /* long */')

    def test_long_long_of_the_games_compiler_is_128_bit(self):
        # SN's GCC 2.95.3 has a 128-bit long long: a quadword clear or copy through it moves 16 bytes.
        src = ('*(long long *)&b = 0; unsigned long long p; signed long long int q; long x;\n'
               'typedef union { unsigned long long v __attribute__((mode(TI))); float f[4]; } H;\n'
               '*(unsigned long long __attribute__((mode(TI))) *)a = 0; /* long long */ s = "long long"; n = 5LL;\n')
        out = prep.widen_long_long(src).split("\n")
        self.assertEqual(out[0], "*(openrac_ti *)&b = 0; openrac_uti p; openrac_ti q; long x;")
        self.assertEqual(out[1], "typedef union { openrac_uti v __attribute__((mode(TI))); float f[4]; } H;")
        self.assertEqual(out[2], '*(unsigned long long __attribute__((mode(TI))) *)a = 0; /* long long */ s = "long long"; n = 5LL;')
        # widen_long runs after it: the 64-bit long stays the only long long.
        self.assertEqual(prep.prepare(prep.widen_long_long(src)).split("\n")[0],
                         "*(openrac_ti *)&b = 0; openrac_uti p; openrac_ti q; long long x;")

    def test_long_long_128_units_leave_out_the_other_compilers_objects(self):
        with tempfile.TemporaryDirectory() as d:
            root = Path(d)
            for rel in ("src/core/a.c", "src/core/lib.c", "src/game/b.c", "src/libgcc/c.c"):
                (root / rel).parent.mkdir(parents=True, exist_ok=True)
                (root / rel).write_text("long long x;\n")
            (root / "config").mkdir()
            (root / "config/objects").write_text("# object address compiler\n"
                                                 "build-sn/core/a.o 0x100000 sn\nbuild-sn/core/lib.o 0x100100 ee29\n")
            cfg = {"sources": ["src"], "skip": ["src/libgcc"],
                   "long_long_128": {"except_objects": "config/objects", "except_compilers": ["ee29"]}}
            self.assertEqual(hostgen.read_long_long_128(root, cfg), {"src/core/a.c", "src/game/b.c"})
            self.assertEqual(hostgen.read_long_long_128(root, {"sources": ["src"], "skip": []}), set())

    def test_file_scope_asm_is_blanked_keeping_lines(self):
        src = 'int a;\n__asm__(".section .text\n\tnop\n");\nint b;\nvoid f(void) { __asm__("sync"); }\n'
        out = prep.blank_file_scope_asm(src)
        self.assertEqual(out.count("\n"), src.count("\n"))
        self.assertNotIn("nop", out)
        self.assertIn('__asm__("sync")', out)

    def test_a_record_defined_in_a_cast_is_named_before_its_function(self):
        # Clang's JSON leaves such a record out; named and defined ahead of the function (the
        # statement may be a loop's whole body), hostgen can lay it out.
        src = ("int g;\n"
               "void f(int i)\n"
               "{\n"
               "    for (i = 0; i < 8; i++)\n"
               "        ((struct { char pad[8];\n"
               "                   int v[8]; } *)&g)->v[i] = -1;\n"
               "}\n"
               "#define PEEK(p) ((struct { int x; } *)(p))\n")
        out = prep.name_cast_records(src)
        lines = out.split("\n")
        self.assertEqual(len(lines), len(src.split("\n")))
        self.assertEqual(lines[1], "struct openrac_cast_5_0 { char pad[8]; int v[8]; }; void f(int i)")
        self.assertIn("((struct openrac_cast_5_0", lines[4])
        self.assertEqual(lines[5].strip(), "*)&g)->v[i] = -1;")
        self.assertEqual(lines[7], "#define PEEK(p) ((struct { int x; } *)(p))")  # no function: as it was
        self.assertEqual(prep.name_cast_records("int a;\n"), "int a;\n")

    def test_wide_cast_is_named(self):
        """A 128-bit integer as a type name becomes openrac_ti: Clang drops the attribute there."""
        src = ("*(int __attribute__((mode(TI))) *)a = *(unsigned int __attribute__((mode(TI))) *)(b + 0x10);\n"
               "typedef int Quad __attribute__((mode(TI)));\n"
               "int __attribute__((mode(TI))) whole;\n"
               "print __attribute__((mode(TI))) *p;\n")
        out = prep.name_wide_casts(src).split("\n")
        self.assertEqual(out[0], "*(openrac_ti *)a = *(openrac_uti *)(b + 0x10);")
        self.assertEqual(out[1], "typedef int Quad __attribute__((mode(TI)));")
        self.assertEqual(out[2], "int __attribute__((mode(TI))) whole;")
        self.assertEqual(out[3], "print __attribute__((mode(TI))) *p;")

    def test_a_mode_ti_cast_on_any_integer_type_is_128_bit(self):
        # The mode sets the width whatever integer type it is written on (rac1's Giant Clank update
        # once copied the hero's joint point through `*(unsigned long long __attribute__((mode(TI))) *)`).
        src = ("*(unsigned long long __attribute__((mode(TI))) *)a = *(long __attribute__((mode(TI))) *)b;\n"
               "x = (signed char __attribute__((mode(TI))))y + (unsigned short int __attribute__((mode(TI))))z;\n"
               "unsigned long long v __attribute__((mode(TI)));\n")
        out = prep.name_wide_casts(src).split("\n")
        self.assertEqual(out[0], "*(openrac_uti *)a = *(openrac_ti *)b;")
        self.assertEqual(out[1], "x = (openrac_ti)y + (openrac_uti)z;")
        self.assertEqual(out[2], "unsigned long long v __attribute__((mode(TI)));")

    def test_candidate_replaces_a_stub_whose_path_is_several_strings(self):
        # A long path written as adjacent strings over three lines: the stub is found, and the
        # lines it took stay, so nothing below it moves.
        src = ('INCLUDE_ASM("config/asm/a/"\n'
               '            "FUN_00100000.s",\n'
               '            FUN_00100000);\n'
               'int after;\n')
        used: set[str] = set()
        out = prep.use_candidates(src, {"FUN_00100000": "int FUN_00100000(void) { return 1; }\n"}, used)
        self.assertEqual(used, {"FUN_00100000"})
        self.assertEqual(out.split("\n")[3], "int after;")
        self.assertIn("int FUN_00100000(void) { return 1; }", out)

    def test_draft_is_read_where_the_stub_stands(self):
        stub = ('#ifndef NON_MATCHING\n'
                'INCLUDE_ASM("a.s", FUN_00100000);\n'
                '#else\n'
                'int FUN_00100000(void) { return 1; }\n'
                '#endif\n')
        out = prep.use_draft(stub, "NON_MATCHING")
        self.assertTrue(out.startswith("#if 0"))
        self.assertEqual(out.count("\n"), stub.count("\n"))
        # A candidate took the stub's place: the draft is not read as well.
        taken = stub.replace('INCLUDE_ASM("a.s", FUN_00100000);', "")
        self.assertEqual(prep.use_draft(taken, "NON_MATCHING"), taken)
        # Another guard is left alone.
        self.assertEqual(prep.use_draft(stub, "OTHER"), stub)


ASM_CONFIG = {"asm": {"enabled": True, "dirs": ["asm"], "gp": "0x00180000"}}


@unittest.skipUnless(have_compiler(), "needs Clang on a POSIX host")
class FromAssembly(unittest.TestCase):
    """C from assembly (asmc.py): a function the test's decompilation has only as assembly (an
    INCLUDE_ASM stub, with an assembly file tests/mips.py writes the way the setup writes one),
    translated, built with the C that calls it, run, and its output checked."""
    run_program = Translate.run_program

    def run_asm(self, c: str, functions: dict[str, tuple[int, str]], symbols=None, tables=None) -> list[str]:
        files = {f"game/asm/{name}.s": mips.assemble(name, start, text, symbols, (tables or {}).get(name))
                 for name, (start, text) in functions.items()}
        return self.run_program({"a.c": c}, config=ASM_CONFIG, extra_files=files)

    def test_branches_and_delay_slots(self):
        # sum of n..1, the counter decremented in the delay slot after the test read it; a likely
        # branch whose delay slot runs only when it is taken.
        out = self.run_asm("""
            INCLUDE_ASM("asm", func_00100000);
            extern int func_00100000(int n);
            INCLUDE_ASM("asm", func_00100100);
            extern int func_00100100(int x);
            int test_main(void) {
                test_print(func_00100000(4));
                test_print(func_00100000(0));
                test_print(func_00100100(3));
                test_print(func_00100100(0));
                return 0;
            }
        """, {"func_00100000": (0x00100000, """
                daddu $2, $0, $0
                beq $4, $0, done
                nop
            loop:
                addu $2, $2, $4
                bne $4, $0, loop      # $4 is tested, then decremented: one more turn with 0
                addiu $4, $4, -1
            done:
                jr $31
                nop
            """), "func_00100100": (0x00100100, """
                addiu $2, $0, 100
                bnel $4, $0, out
                addiu $2, $0, 7       # only when taken
                addiu $2, $2, 1
            out:
                jr $31
                nop
            """)})
        self.assertEqual(out, ["10", "0", "7", "101"])
        self.assertEqual(self.last_report["functions_from_assembly"], 2)

    def test_calls_pass_arguments_by_the_callees_prototype(self):
        # The assembly calls C: integers in $a0.., floats in $f12.., the result in $v0 / $f0;
        # its own frame on the game stack, a local's address passed to C.
        out = self.run_asm("""
            int func_00200000(int a, float b, int c) {
                test_print(a); test_print_float(b); test_print(c);
                return a + c;
            }
            float func_00200100(float x) { return x * 2.0f; }
            void func_00200200(int *p) { test_print(*p); *p = 77; }
            INCLUDE_ASM("asm", func_00100000);
            extern int func_00100000(int a, int b, float f);
            int test_main(void) { test_print(func_00100000(5, 6, 1.5f)); return 0; }
        """, {"func_00100000": (0x00100000, """
                addiu $29, $29, -32
                sq $31, 16($29)
                sw $16, 0($29)
                daddu $16, $5, $0      # keep b
                sw $5, 4($29)
                daddu $6, $5, $0       # c = b
                jal func_00200000      # (a, f, b)
                mov.s $f12, $f12
                addu $16, $16, $2      # b + (a + b)
                jal func_00200100
                mov.s $f12, $f12
                mfc1 $3, $f0
                jal func_00200200      # the address of the local at 4($sp)
                addiu $4, $29, 4
                lw $2, 4($29)
                addu $2, $2, $16
                lq $31, 16($29)
                lw $16, 0($29)
                jr $31
                addiu $29, $29, 32
            """)}, symbols={"func_00200000": 0x00200000, "func_00200100": 0x00200100, "func_00200200": 0x00200200})
        self.assertEqual(out, ["5", "1.500", "6", "6", "94"])

    def test_floats(self):
        # (a * b + c) / d, a compare and its branch, a conversion to an integer.
        out = self.run_asm("""
            INCLUDE_ASM("asm", func_00100000);
            extern int func_00100000(float a, float b, float c, float d);
            INCLUDE_ASM("asm", func_00100100);
            extern float func_00100100(float a, float b);
            int test_main(void) {
                test_print(func_00100000(2.0f, 3.0f, 4.0f, 0.5f));
                test_print_float(func_00100100(1.0f, 2.0f));
                test_print_float(func_00100100(5.0f, 2.0f));
                test_print_float(func_00100100(-1.0f, 0.0f));
                return 0;
            }
        """, {"func_00100000": (0x00100000, """
                mul.s $f0, $f12, $f13
                add.s $f0, $f0, $f14
                div.s $f0, $f0, $f15
                cvt.w.s $f1, $f0
                jr $31
                mfc1 $2, $f1
            """), "func_00100100": (0x00100100, """
                c.lt.s $f12, $f13
                bc1t less
                sub.s $f0, $f12, $f13
                jr $31
                nop
            less:
                div.s $f0, $f12, $f13  # by zero: the EE's largest float, not infinity
                jr $31
                neg.s $f0, $f0
            """)})
        self.assertEqual(out[0], "20")
        self.assertEqual(out[1:3], ["-0.500", "3.000"])
        self.assertEqual(out[3], "340282346638528859811704183484516925440.000")

    def test_loads_and_stores(self):
        out = self.run_asm("""
            extern unsigned char D_00300000[32];
            INCLUDE_ASM("asm", func_00100000);
            extern int func_00100000(unsigned char *p);
            int test_main(void) {
                int i;
                for (i = 0; i < 32; i++) D_00300000[i] = (unsigned char)(i * 16 + 1);
                test_print(func_00100000(D_00300000));
                for (i = 16; i < 32; i++) test_print(D_00300000[i]);
                return 0;
            }
        """, {"func_00100000": (0x00100000, """
                lb $2, 15($4)          # 0xF1: -15
                lbu $3, 15($4)         # 241
                addu $2, $2, $3        # 226
                lh $3, 2($4)           # 0x3121
                addu $2, $2, $3
                lq $8, 0($4)           # all 16 bytes
                sq $8, 16($4)          # copied
                lwl $9, 4($4)          # an unaligned word at 1: bytes 1..4
                lwr $9, 1($4)
                sw $9, 16($4)
                ld $10, 8($4)
                dsrl32 $10, $10, 0
                jr $31
                addu $2, $2, $10       # + the high word of the second doubleword (signed)
            """)}, symbols={"D_00300000": 0x00300000})
        # 226 + 0x3121 + (bytes 12..15 as a word: 0xF1E1D1C1 = -237907519)
        self.assertEqual(out[0], str(226 + 0x3121 + (0xF1E1D1C1 - (1 << 32))))
        self.assertEqual(out[1:5], ["17", "33", "49", "65"])          # bytes 1..4 stored at 16..19
        self.assertEqual(out[5:], [str((i * 16 + 1) & 0xFF) for i in range(4, 16)])

    def test_jump_table_and_globals(self):
        # A switch through a table of code addresses in the game's data, the table's address
        # formed with %hi/%lo; a global through $gp.
        source = """
                sltiu $2, $4, 3
                beq $2, $0, other
                sll $4, $4, 2
                lui $2, %hi(jtbl_00300000)
                addiu $2, $2, %lo(jtbl_00300000)
                addu $4, $4, $2
                lw $4, 0($4)
                jr $4
                nop
            case0:
                jr $31
                addiu $2, $0, 10
            case1:
                jr $31
                lw $2, -0x10($28)      # D_0017FFF0, through $gp
            case2:
                lui $3, %hi(D_00300010)
                lw $2, %lo(D_00300010)($3)
                jr $31
                addiu $2, $2, 1
            other:
                jr $31
                addiu $2, $0, -1
        """
        labels = mips.label_addresses(0x00100000, source)
        out = self.run_asm(f"""
            extern unsigned int D_00300000[8];
            extern int D_0017FFF0;
            INCLUDE_ASM("asm", func_00100000);
            extern int func_00100000(int);
            int test_main(void) {{
                D_00300000[0] = 0x{labels['case0']:08X}u;
                D_00300000[1] = 0x{labels['case1']:08X}u;
                D_00300000[2] = 0x{labels['case2']:08X}u;
                D_00300000[4] = 41;
                D_0017FFF0 = 20;
                test_print(func_00100000(0)); test_print(func_00100000(1));
                test_print(func_00100000(2)); test_print(func_00100000(9));
                return 0;
            }}
        """, {"func_00100000": (0x00100000, source)},
            symbols={"jtbl_00300000": 0x00300000, "D_00300010": 0x00300010},
            tables={"func_00100000": {"jtbl_00300000": ["case0", "case1", "case2"]}})
        self.assertEqual(out, ["10", "20", "42", "-1"])

    def test_mmi_and_vu0(self):
        out = self.run_asm("""
            extern float D_00300000[12];
            extern int D_00300100[8];
            INCLUDE_ASM("asm", func_00100000);
            extern void func_00100000(float *v, int *w);
            int test_main(void) {
                int i;
                for (i = 0; i < 4; i++) { D_00300000[i] = (float)(i + 1); D_00300000[4 + i] = 10.0f * (i + 1); }
                for (i = 0; i < 4; i++) D_00300100[i] = i + 1;
                func_00100000(D_00300000, D_00300100);
                for (i = 8; i < 12; i++) test_print_float(D_00300000[i]);
                for (i = 4; i < 8; i++) test_print(D_00300100[i]);
                return 0;
            }
        """, {"func_00100000": (0x00100000, """
                lqc2 $vf1, 0($4)            # (1, 2, 3, 4)
                lqc2 $vf2, 16($4)           # (10, 20, 30, 40)
                vmulbc.xyz $vf3, $vf1, $vf2y   # (20, 40, 60, w kept)
                vadd.w $vf3, $vf1, $vf2     # w = 44
                sqc2 $vf3, 32($4)
                lq $8, 0($5)                # (1, 2, 3, 4)
                pextlw $9, $8, $8           # (1, 1, 2, 2)
                pcpyld $10, $9, $8          # (1, 2, 1, 1)
                jr $31
                sq $10, 16($5)
            """)}, symbols={})
        self.assertEqual(out, ["20.000", "40.000", "60.000", "44.000", "1", "2", "1", "1"])

    def test_parallel_halfword_multiply(self):
        # Going Commando's sprite colour code: eight signed products, the even ones in rd, all
        # of them in LO and HI, read back saturated to halfwords.
        out = self.run_asm("""
            extern short D_00300200[32];
            INCLUDE_ASM("asm", func_00100000);
            extern void func_00100000(short *v);
            int test_main(void) {
                static const short a[8] = {1, -2, 3, -4, 5, -6, 7, 300};
                int i;
                for (i = 0; i < 8; i++) { D_00300200[i] = a[i]; D_00300200[8 + i] = i == 7 ? 200 : 10; }
                func_00100000(D_00300200);
                for (i = 16; i < 24; i += 2) test_print((D_00300200[i] & 0xFFFF) | (D_00300200[i + 1] << 16));
                for (i = 24; i < 32; i++) test_print(D_00300200[i]);
                return 0;
            }
        """, {"func_00100000": (0x00100000, """
                lq $8, 0($4)
                lq $9, 16($4)
                pmulth $10, $8, $9
                sq $10, 32($4)
                pmfhl.sh $11, $0, $0
                jr $31
                sq $11, 48($4)
            """)}, symbols={})
        self.assertEqual(out, ["10", "30", "50", "70", "10", "-20", "30", "-40", "50", "-60", "70", "32767"])

    def test_parallel_word_multiplies_shifts_and_divides(self):
        # The R5900's two-result word operations (HARDWARE_SEMANTICS.md, item 10): two 64-bit
        # products, not a sum; the variable shifts, sign-extended to doublewords; the divides.
        out = self.run_asm("""
            extern int D_00300000[56];
            INCLUDE_ASM("asm", func_00100000);
            extern void func_00100000(int *v);
            int test_main(void) {
                static const int s[4] = {3, 100, -5, 200}, t[4] = {7, 0, 11, 0};
                int i;
                for (i = 0; i < 4; i++) { D_00300000[i] = s[i]; D_00300000[4 + i] = t[i]; }
                func_00100000(D_00300000);
                for (i = 8; i < 56; i++) test_print(D_00300000[i]);
                return 0;
            }
        """, {"func_00100000": (0x00100000, """
                lq $8, 0($4)
                lq $9, 16($4)
                pmultw $10, $8, $9     # (3 * 7, -5 * 11) as doublewords
                sq $10, 32($4)
                pmaddw $11, $8, $9     # HI:LO plus the products again
                sq $11, 48($4)
                pmfhi $12
                sq $12, 64($4)
                pmflo $13
                sq $13, 80($4)
                psllvw $14, $9, $8     # rt << rs: (3 << 7, -5 << 11)
                sq $14, 96($4)
                psravw $15, $9, $8
                sq $15, 112($4)
                psrlvw $16, $9, $8
                sq $16, 128($4)
                pdivw $0, $8, $9       # quotients to LO, remainders to HI
                pmfhl.lw $17, $0, $0
                sq $17, 144($4)
                pmultuw $18, $8, $9    # 0xFFFFFFFB * 11, unsigned
                sq $18, 160($4)
                pmfhl.slw $19, $0, $0  # HI:LO saturated to 32 bits
                sq $19, 176($4)
                pdivbw $0, $8, $9      # each word over halfword 0 of rt (7)
                pmflo $20
                sq $20, 192($4)
                pmfhi $21
                jr $31
                sq $21, 208($4)
            """)}, symbols={})
        self.assertEqual([int(x) for x in out], [
            21, 0, -55, -1,  42, 0, -110, -1,  0, 0, -1, -1,  42, 0, -110, -1,
            384, 0, -10240, -1,  0, 0, -1, -1,  0, 0, 2097151, 0,  0, 3, 0, -5,
            21, 0, -55, 10,  21, 0, 2147483647, 0,  0, 14, 0, 28,  3, 2, -5, 4])

    def test_parallel_halfword_multiply_add(self):
        # The halfword family on LO and HI's eight words (item 10): phmadh's pair sums (and the
        # higher product in the odd words), pmaddh's accumulation, pmfhl.lh, pmthl, padsbh, phmsbh.
        out = self.run_asm("""
            extern short D_00300000[16];
            extern int D_00300100[28];
            extern short D_00300200[16];
            INCLUDE_ASM("asm", func_00100000);
            extern void func_00100000(short *v, int *w, short *h);
            int test_main(void) {
                int i;
                for (i = 0; i < 8; i++) { D_00300000[i] = (short)(i + 1); D_00300000[8 + i] = (short)(10 * (i + 1)); }
                func_00100000(D_00300000, D_00300100, D_00300200);
                for (i = 0; i < 28; i++) test_print(D_00300100[i]);
                for (i = 0; i < 16; i++) test_print(D_00300200[i]);
                return 0;
            }
        """, {"func_00100000": (0x00100000, """
                lq $8, 0($4)           # 1 .. 8
                lq $9, 16($4)          # 10 .. 80
                phmadh $10, $8, $9
                sq $10, 0($5)
                pmfhl.uw $11, $0, $0   # the odd words: the higher products
                sq $11, 16($5)
                pmaddh $12, $8, $9
                sq $12, 32($5)
                pmfhl.lh $13, $0, $0
                sq $13, 0($6)
                pmthl $0, $9, $0       # rt's words to LO 0, HI 0, LO 2, HI 2
                pmfhl.lw $14, $0, $0
                sq $14, 48($5)
                pmfhl.uw $15, $0, $0   # the odd words kept
                sq $15, 64($5)
                phmsbh $16, $8, $9
                sq $16, 80($5)
                pmfhl.uw $17, $0, $0
                sq $17, 96($5)
                padsbh $18, $9, $8     # rs - rt below, rs + rt above
                jr $31
                sq $18, 16($6)
            """)}, symbols={})
        self.assertEqual([int(x) for x in out], [
            50, 250, 610, 1130,  40, 160, 360, 640,  60, 340, 860, 1620,
            10 | 20 << 16, 30 | 40 << 16, 50 | 60 << 16, 70 | 80 << 16,  80, 320, 720, 1280,
            30, 70, 110, 150,  -41, -161, -361, -641,
            60, 80, 340, 320, 860, 720, 1620, 1280,  9, 18, 27, 36, 55, 66, 77, 88])

    def test_conditional_call_tests_all_64_bits(self):
        # bltzal on a register whose low word is positive and whose doubleword is negative
        # (item 3): the EE calls; $ra is set either way.
        out = self.run_asm("""
            int func_00100100(int x) { return x * 2; }
            INCLUDE_ASM("asm", func_00100000);
            extern int func_00100000(int a);
            int test_main(void) { test_print(func_00100000(-1)); test_print(func_00100000(1)); return 0; }
        """, {"func_00100000": (0x00100000, """
                addiu $29, $29, -16
                sq $31, 0($29)
                addiu $2, $0, 0
                dsll32 $8, $4, 0
                ori $8, $8, 1          # (a << 32) | 1
                bltzal $8, func_00100100
                addiu $4, $0, 21       # its argument, in the delay slot
                lq $31, 0($29)
                jr $31
                addiu $29, $29, 16
            """)}, symbols={"func_00100100": 0x00100100})
        self.assertEqual(out, ["42", "0"])

    def test_conversions_saturate_by_sign(self):
        # cvt.w.s and vftoi0 of floats with exponent 255 (a NaN to the host, a large number to
        # the EE) and of 3e9 (item 5): 0x7FFFFFFF or 0x80000000 by the sign, never x86's 0x80000000.
        out = self.run_asm("""
            extern unsigned int D_00300000[16];
            INCLUDE_ASM("asm", func_00100000);
            extern void func_00100000(unsigned int *v);
            int test_main(void) {
                int i;
                D_00300000[0] = 0x7FFFEF02u; D_00300000[1] = 0xFFFFEF02u;
                D_00300000[2] = 0x4F32D05Eu; D_00300000[3] = 0xBFC00000u; /* 3e9, -1.5 */
                func_00100000(D_00300000);
                for (i = 4; i < 12; i++) test_print((int)D_00300000[i]);
                return 0;
            }
        """, {"func_00100000": (0x00100000, """
                lwc1 $f0, 0($4)
                cvt.w.s $f1, $f0
                mfc1 $2, $f1
                sw $2, 16($4)
                lwc1 $f0, 4($4)
                cvt.w.s $f1, $f0
                mfc1 $2, $f1
                sw $2, 20($4)
                lqc2 $vf1, 0($4)
                vftoi0.xyzw $vf2, $vf1
                jr $31
                sqc2 $vf2, 32($4)
            """)}, symbols={})
        self.assertEqual([int(x) for x in out], [2147483647, -2147483648, 0, 0,
                                                 2147483647, -2147483648, 2147483647, -1])

    def test_vu0_macro_memory_and_vf0(self):
        # VSQI/VLQD/VILWR/VISWR reach VU0's data memory at (vi & 0xFF) * 16, not main RAM; the
        # increments and decrements are vi's (item 8); vf0 stays (0, 0, 0, 1) (item 4).
        out = self.run_asm("""
            extern float D_00300000[24];
            INCLUDE_ASM("asm", func_00100000);
            extern void func_00100000(float *v);
            int test_main(void) {
                int i;
                for (i = 0; i < 4; i++) D_00300000[i] = (float)(i + 1);
                func_00100000(D_00300000);
                for (i = 4; i < 12; i++) test_print_float(D_00300000[i]);
                test_print(((int *)D_00300000)[12]);
                test_print(((int *)D_00300000)[13]);
                for (i = 16; i < 20; i++) test_print_float(D_00300000[i]);
                test_print_float(((float *)0x11004020)[0]);
                test_print(((int *)0x11004020)[2]);
                return 0;
            }
        """, {"func_00100000": (0x00100000, """
                addiu $8, $0, 0x102    # vi1 = 2 (only its low eight bits address a quadword)
                ctc2 $8, $vi1
                lqc2 $vf1, 0($4)
                vsqi.xyzw $vf1, $vi1   # to quadword 2; vi1 = 3
                vsqi.xyzw $vf0, $vi1   # (0, 0, 0, 1) to 3; vi1 = 4
                vlqd.xyzw $vf2, $vi1   # vi1 = 3: (0, 0, 0, 1)
                vlqd.xy $vf3, $vi1     # vi1 = 2: x and y of (1, 2, 3, 4)
                sqc2 $vf2, 16($4)
                sqc2 $vf3, 32($4)
                addiu $9, $0, 0x1234
                ctc2 $9, $vi2
                viswr.z $vi2, $vi1     # the word z of quadword 2
                vilwr.z $vi3, $vi1
                vlqi.xyzw $vf0, $vi1   # vf0 is not written; vi1 = 3
                cfc2 $2, $vi3
                sw $2, 48($4)
                cfc2 $3, $vi1
                sw $3, 52($4)
                qmtc2 $8, $vf0         # nor here
                jr $31
                sqc2 $vf0, 64($4)
            """)}, symbols={})
        self.assertEqual(out, ["0.000", "0.000", "0.000", "1.000", "1.000", "2.000", "0.000", "0.000",
                               "4660", "259", "0.000", "0.000", "0.000", "1.000", "1.000", "4660"])

    def test_vu0_flags_and_clipping(self):
        # MAC and STATUS after a macro subtraction, read with CFC2 as the collision code does; the
        # clip flag of two VCLIPw; the invalid flag of 0 / 0 (item 9).
        out = self.run_asm("""
            extern float D_00300000[12];
            extern int D_00300100[4];
            INCLUDE_ASM("asm", func_00100000);
            extern void func_00100000(float *v, int *w);
            int test_main(void) {
                static const float v[12] = {1, -2, 0, 5,  1, 1, 0, 4,  10, -10, 3, 2};
                int i;
                for (i = 0; i < 12; i++) D_00300000[i] = v[i];
                func_00100000(D_00300000, D_00300100);
                for (i = 0; i < 4; i++) test_print(D_00300100[i]);
                return 0;
            }
        """, {"func_00100000": (0x00100000, """
                lqc2 $vf1, 0($4)
                lqc2 $vf2, 16($4)
                lqc2 $vf4, 32($4)
                vsub.xyz $vf3, $vf1, $vf2   # (0, -3, 0): x and z zero, y negative
                cfc2 $2, $vi17
                sw $2, 0($5)
                cfc2 $3, $vi16
                sw $3, 4($5)
                vclipw.xyz $vf4, $vf2       # (10, -10, 3) against 4: +x, -y
                vclipw.xyz $vf1, $vf2       # inside
                cfc2 $6, $vi18
                sw $6, 8($5)
                raw 0x4A0003BC              # vdiv Q, $vf0x, $vf0x: 0 / 0
                cfc2 $7, $vi16
                jr $31
                sw $7, 12($5)
            """)}, symbols={})
        self.assertEqual([int(x) for x in out], [0x4A, 0xC3, 9 << 6, 0xC3 | 0x10 | 0x400])

    def test_declared_nowhere_gets_its_parameters_from_its_registers(self):
        # Called only through its code address; the C calls it as (int, int).
        out = self.run_asm("""
            INCLUDE_ASM("asm", func_00100000);
            int test_main(void) {
                int (*f)(int, int) = (int (*)(int, int))0x00100000;
                test_print(f(30, 12));
                return 0;
            }
        """, {"func_00100000": (0x00100000, """
                jr $31
                addu $2, $4, $5
            """)})
        self.assertEqual(out, ["42"])

    def test_what_it_cannot_write_stays_a_stub_and_is_reported(self):
        out = self.run_asm("""
            INCLUDE_ASM("asm", func_00100000);
            extern int func_00100000(void);
            int test_main(void) { test_print(func_00100000()); return 0; }
        """, {"func_00100000": (0x00100000, """
                syscall
                jr $31
                nop
            """)})
        self.assertEqual(out, ["missing", "func_00100000", "0"])
        self.assertEqual(self.last_report["functions_from_assembly"], 0)
        self.assertIn("a system call", self.last_report["assembly_unsupported"]["func_00100000"])

    def test_an_input_no_caller_passes_stays_a_stub(self):
        # $3 read before it is written: a hand-written routine's own convention, or a fragment.
        self.run_asm("""
            INCLUDE_ASM("asm", func_00100000);
            extern int func_00100000(int);
            int test_main(void) { return func_00100000(1) * 0; }
        """, {"func_00100000": (0x00100000, """
                jr $31
                addu $2, $4, $3
            """)})
        self.assertIn("$3", self.last_report["assembly_unsupported"]["func_00100000"])

    def test_a_routine_taking_registers_runs_in_place(self):
        # The helper takes $3 (no prototype can say so): the call runs its code in place and comes
        # back through $ra; the function's own return is the saved $ra's.
        out = self.run_asm("""
            INCLUDE_ASM("asm", func_00100000);
            extern int func_00100000(int);
            int test_main(void) { test_print(func_00100000(4)); test_print(func_00100000(10)); return 0; }
        """, {"func_00100000": (0x00100000, """
                addiu $29, $29, -16
                sq $31, 0($29)
                jal func_00100100
                addiu $3, $4, 5
                jal func_00100100
                daddu $3, $2, $0
                lq $31, 0($29)
                jr $31
                addiu $29, $29, 16
            """), "func_00100100": (0x00100100, """
                jr $31
                sll $2, $3, 1
            """)}, symbols={"func_00100100": 0x00100100})
        self.assertEqual(out, ["36", "60"])

    def test_console_form(self):
        # The form for the console's compiler (rac1-decomp's tools/fcheck.py): the function under a
        # name of its own with the function's symbol, calls by retail address with every argument
        # register, memory at the console's addresses.
        with tempfile.TemporaryDirectory() as tmp:
            out = Path(tmp) / "console"
            files = {"game/asm/func_00100000.s": mips.assemble("func_00100000", 0x00100000, """
                addiu $29, $29, -16
                sq $31, 0($29)
                jal func_00200000
                lw $4, 0x10($4)
                lq $31, 0($29)
                jr $31
                addiu $29, $29, 16
            """, {"func_00200000": 0x00200000})}
            self.run_program({"a.c": """
                int func_00200000(int x) { return x; }
                INCLUDE_ASM("asm", func_00100000);
                extern int func_00100000(int *p);
                int test_main(void) { return 0; }
            """}, config={**ASM_CONFIG}, extra_files=files, hostgen_args=["--asm-console", str(out)])
            text = (out / "func_00100000.c").read_text()
        self.assertIn('int openrac_asmc_func_00100000(gaddr a0) __asm__("func_00100000");', text)
        self.assertIn("((int (*)(openrac_u64, openrac_u64, openrac_u64, openrac_u64, openrac_u64, openrac_u64, "
                      "openrac_u64, openrac_u64, float, float, float, float, float, float, float, float, openrac_u64, openrac_u64, openrac_u64, openrac_u64))0x00200000u)", text)
        self.assertIn("#define OPENRAC_EE_PTR(a) ((void *)(openrac_u32)(a))", text)
        self.assertNotIn("GFN(", text)


class Decoding(unittest.TestCase):
    def test_words(self):
        cases = {
            0x27BDFFE0: ("addiu", dict(rs=29, rt=29, imm=-32)),
            0x03E00008: ("jr", dict(rs=31)),
            0x0C080000: ("jal", dict(target=0x00200000)),
            0x7FB00010: ("sq", dict(rs=29, rt=16, imm=16)),
            0x46020000: ("add.s", dict(fs=0, ft=2, fd=0)),
            0x4A0208A8: ("vadd", dict(fs=1, ft=2, fd=2)),
            0x70A84389: ("pcpyld", dict(rs=5, rt=8, rd=8)),
        }
        for word, (op, fields) in cases.items():
            with self.subTest(hex(word)):
                i = asm.decode(word, 0x00100000)
                self.assertEqual(i.op, op)
                for k, v in fields.items():
                    self.assertEqual(getattr(i, k), v, k)

    def test_file_reading(self):
        text = mips.assemble("func_00100000", 0x00100000, """
            beq $4, $0, end
            nop
        end:
            jr $31
            nop
        """)
        path = Path(self.id().replace(".", "_") + ".s")
        try:
            path.write_text(text)
            (block,) = asm.parse_file(path)
        finally:
            path.unlink()
        self.assertEqual(block.name, "func_00100000")
        self.assertEqual([l.addr for l in block.lines], [0x00100000, 0x00100004, 0x00100008, 0x0010000C])
        self.assertEqual(block.labels, {0x00100008: ".L00100008"})


if __name__ == "__main__":
    unittest.main()
