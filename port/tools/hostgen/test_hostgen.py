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
import shutil
import subprocess
import sys
import tempfile
import textwrap
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

import clangast  # noqa: E402
import ctype  # noqa: E402
import hostgen  # noqa: E402
import prep  # noqa: E402

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
                    stubs: int = 0) -> list[str]:
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
                               "--jobs", "2", "--quiet"])
            self.assertEqual(rc, 0)
            report = json.loads((out / "report.json").read_text())
            self.last_report = report
            self.assertEqual(report["functions_stubbed"], stubs, report["stubbed"])
            sources = [out / u.strip() for u in (out / "units.txt").read_text().splitlines()]
            self.last_generated = "\n".join(s.read_text() for s in sources)
            sources += [out / "stubs.c", out / "functions.c", out / "libraries.c", HERE / "tests" / "harness.c"]
            if host_c:
                (tmp / "host.c").write_text('#include "openrac/guest.h"\n' + textwrap.dedent(host_c))
                sources.append(tmp / "host.c")
            exe = tmp / "prog"
            build = subprocess.run(
                ["clang", "-std=gnu11", "-O1", "-w", "-fno-strict-aliasing", "-fwrapv", "-fsigned-char",
                 "-I", str(RUNTIME_INCLUDE), "-I", str(out), "-o", str(exe), *map(str, sources)],
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


class SupersededDeclarations(unittest.TestCase):
    def test_header_declaration_a_definition_contradicts_is_dropped(self):
        with tempfile.TemporaryDirectory() as d:
            root = Path(d)
            (root / "include").mkdir()
            (root / "include/h.h").write_text("int keep;\nextern void f(void *, int);\nint tail;\n")
            text = '#include "include/h.h"\nvoid f(int *p, int n) {\n}\n'
            err = ("a.c:2:6: error: conflicting types for 'f'\n"
                   "include/h.h:2:13: note: previous declaration is here\n")
            done = clangast.drop_superseded_declarations(root, "a.c", text, err)
            self.assertEqual(len(done), 1)
            self.assertEqual((root / "include/h.h").read_text(), "int keep;\n\nint tail;\n")


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


if __name__ == "__main__":
    unittest.main()
