# SPDX-License-Identifier: GPL-3.0-or-later
"""Compile translated anonymous typedefs using the installed host Clang.

This needs no POSIX runtime harness and also exercises Windows source paths.
"""
import json
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest
from unittest import mock

import clangast
import hostgen


class ClangAliases(unittest.TestCase):
    def test_size_and_ptrdiff_aliases_are_the_targets_types(self):
        # Fedora's Clang writes memmove's parameter as __size_t, with no declaration and, for an
        # implicit builtin's, no desugared type; also inside the function's own type.
        ast = {"kind": "TranslationUnitDecl", "inner": [
            {"kind": "FunctionDecl", "name": "memmove",
             "type": {"qualType": "void *(void *, const void *, __size_t)"},
             "inner": [{"kind": "ParmVarDecl", "type": {"qualType": "__size_t"}},
                       {"kind": "ParmVarDecl", "type": {"qualType": "__ptrdiff_t *"}},
                       {"kind": "ParmVarDecl", "type": {"qualType": "my__size_t"}}]},
        ]}
        clangast.annotate(ast)
        fn = ast["inner"][0]
        self.assertEqual(fn["type"]["qualType"], "void *(void *, const void *, unsigned int)")
        self.assertEqual([p["type"]["qualType"] for p in fn["inner"]], ["unsigned int", "int *", "my__size_t"])


class ClangRunsWithoutACompilerCache(unittest.TestCase):
    def test_ccache_cannot_answer_for_a_rewritten_prepared_copy(self):
        # clang on PATH may be ccache's wrapper. Its direct cache keys a file by inode and
        # second-granular mtime, and hostgen rewrites its prepared copies between checks, so a
        # hit can report the previous content's verdict (it hid pause.c's conflicting types
        # from the syntax check, where the AST dump then refused the same file).
        with mock.patch.object(clangast.subprocess, "run") as run:
            run.return_value = subprocess.CompletedProcess([], 0, "", "")
            clangast.run("clang", Path("src/game/pause.c"), Path("/tmp"), ["-fsyntax-only"], False)
        env = run.call_args.kwargs["env"]
        self.assertEqual(env.get("CCACHE_DISABLE"), "1")


@unittest.skipUnless(shutil.which("clang"), "needs Clang")
class ClangCompatibility(unittest.TestCase):
    def test_nested_source_and_anonymous_typedefs(self):
        with tempfile.TemporaryDirectory(prefix="hostgen compatibility ") as tmp:
            root = Path(tmp)
            source = root / "source"
            (source / "src/nested").mkdir(parents=True)
            (source / "src/nested/example.c").write_text(
                "typedef struct { int value; } Item;\n"
                "typedef union { Item item; int other; } Payload;\n"
                "typedef enum { VALUE = 7 } Kind;\n"
                "int func_00100000(Item *item, Item *end, Kind kind) {\n"
                "  Payload p; p.item.value = item->value;\n"
                "  return p.item.value + kind + sizeof(Payload) * item->value + (end - item);\n"
                "}\n"
            )
            config = root / "game.json"
            config.write_text(json.dumps({"name": "compatibility", "sources": ["src"],
                                          "includes": [], "defines": []}))
            output = root / "generated"
            self.assertEqual(hostgen.main([
                "--game", str(config), "--source", str(source), "--out", str(output),
                "--jobs", "2", "--quiet",
            ]), 0)
            generated = output / "nested/example.c"
            self.assertIn("func_00100000(", generated.read_text())
            report = json.loads((output / "report.json").read_text())
            self.assertEqual(report["units_unreadable"], [])
            self.assertEqual(report["index_problems"], [])
            runtime = Path(__file__).resolve().parents[2] / "runtime/include"
            compiled = subprocess.run([
                shutil.which("clang"), "-std=gnu11", "-fsyntax-only", "-I", str(runtime),
                "-I", str(output), str(generated),
            ], capture_output=True, text=True)
            self.assertEqual(compiled.returncode, 0, compiled.stderr)

    def test_gcc_label_and_redeclaration_leniencies(self):
        # What GCC 2.95 took and Clang refuses (rac1/ntsc's synced tree, 2026-10-15): a variable
        # given a second asm label (GCC kept the first), a label repeating a variable's name after
        # its use, a function labelled one way here and another way in a header included after
        # its use, and an extern of an anonymous struct contradicting a function-local one.
        with tempfile.TemporaryDirectory(prefix="hostgen leniencies ") as tmp:
            root = Path(tmp)
            source = root / "source"
            (source / "src").mkdir(parents=True)
            (source / "include").mkdir()
            (source / "include/fast_math.h").write_text('float absolute_float(float x) __asm__("FUN_001f99c0");\n')
            (source / "src/example.c").write_text(
                'extern float ref __asm__("D_0015ED6C");\n'
                'extern float table[];\n'
                'float absolute_float(float input) __asm__("func_001F99C0");\n'
                "int func_00100000(void) {\n"
                "  extern struct { char pad[0x18]; int moby; } D_00174140;\n"
                "  return (int)(absolute_float(table[0]) + D_00174140.moby);\n"
                "}\n"
                'extern float ref __asm__("D_0015EE6C");\n'
                'extern float table[] __asm__("table");\n'
                "extern struct { char pad[0x28]; float height; } D_00174140;\n"
                '#include "fast_math.h"\n'
                "float func_00100040(void) {\n"
                "  return ref * 2.0f + D_00174140.height + absolute_float(-1.0f);\n"
                "}\n"
            )
            config = root / "game.json"
            config.write_text(json.dumps({"name": "leniencies", "sources": ["src"],
                                          "includes": ["include"], "defines": []}))
            output = root / "generated"
            self.assertEqual(hostgen.main([
                "--game", str(config), "--source", str(source), "--out", str(output),
                "--jobs", "2", "--quiet",
            ]), 0)
            report = json.loads((output / "report.json").read_text())
            self.assertEqual(report["units_unreadable"], [])
            text = (output / "example.c").read_text()
            self.assertIn("func_00100040(", text)
            self.assertNotIn("D_0015EE6C", text)   # the first label is the variable's

    def test_a_definition_that_contradicts_a_shared_header(self):
        # A header shared by the callers says `void *` and `void` where the definitions take
        # `float *` and return int (rac1/pal's include/callees.h against its candidates): the
        # unit is read all the same, and each function is translated under its own symbol.
        with tempfile.TemporaryDirectory(prefix="hostgen contradiction ") as tmp:
            root = Path(tmp)
            source = root / "source"
            (source / "src").mkdir(parents=True)
            (source / "include").mkdir()
            (source / "include/callees.h").write_text(
                "extern void func_00100000(void *, void *);\n"
                "extern void func_00100040(void);\n"
            )
            (source / "src/example.c").write_text(
                '#include "callees.h"\n'
                "void func_00100000(float *out, float *in) {\n"
                "  out[0] = in[0] * 2.0f;\n"
                "}\n"
                "int func_00100040(void)\n"  # its brace on the next line
                "{\n"
                "  return 7;\n"
                "}\n"
                "int func_00100080(float *v) {\n"
                "  func_00100000(v, v);\n"
                "  return func_00100040();\n"
                "}\n"
            )
            config = root / "game.json"
            config.write_text(json.dumps({"name": "contradiction", "sources": ["src"],
                                          "includes": ["include"], "defines": []}))
            output = root / "generated"
            self.assertEqual(hostgen.main([
                "--game", str(config), "--source", str(source), "--out", str(output),
                "--jobs", "2", "--quiet",
            ]), 0)
            report = json.loads((output / "report.json").read_text())
            self.assertEqual(report["units_unreadable"], [])
            generated = output / "example.c"
            text = generated.read_text()
            for name in ("func_00100000(", "func_00100040(", "func_00100080("):
                self.assertIn(name, text)
            self.assertNotIn("__openrac_decl_", text)  # the aliases stay in the prepared copy
            runtime = Path(__file__).resolve().parents[2] / "runtime/include"
            compiled = subprocess.run([
                shutil.which("clang"), "-std=gnu11", "-fsyntax-only", "-I", str(runtime),
                "-I", str(output), str(generated),
            ], capture_output=True, text=True)
            self.assertEqual(compiled.returncode, 0, compiled.stderr)


if __name__ == "__main__":
    unittest.main()
