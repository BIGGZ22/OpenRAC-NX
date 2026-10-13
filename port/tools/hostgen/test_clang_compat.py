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

import hostgen


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
