# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""
Tests for check_conventions.py: each rule on small made-up files, the scope, and a whole run
on a throwaway git repository.

  python3 -m unittest discover -s port/tools/conventions
"""
import shutil
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import check_conventions as cc  # noqa: E402

LICENCE = "// SPDX-License-Identifier: GPL-3.0-or-later\n// Copyright (c) 2026 the OpenRAC contributors\n"


def rules(findings):
    """The rule names of `findings`, in order."""
    return [finding.rule for finding in findings]


class ScopeTest(unittest.TestCase):
    def test_port_sources_are_in_scope(self):
        self.assertTrue(cc.in_scope("port/renderer/gl.cpp"))
        self.assertTrue(cc.in_scope("port/game/rac1/host/hand.c"))
        self.assertTrue(cc.in_scope("port/runtime/include/openrac/guest.h"))

    def test_hand_c_build_trees_and_other_files_are_not(self):
        self.assertFalse(cc.in_scope("port/game/rac1-pal/hand/functional/x.c"))
        self.assertFalse(cc.in_scope("port/build/release/gen/a.c"))
        self.assertFalse(cc.in_scope("port/README.md"))
        self.assertFalse(cc.in_scope("games/rac1/src/main.c"))

    def test_generated_files_say_so_in_their_first_lines(self):
        self.assertTrue(cc.is_generated("/* Generated from x by y. Do not edit. */\n"))
        self.assertFalse(cc.is_generated(LICENCE))

    def test_a_header_is_cpp_when_its_code_opens_a_namespace(self):
        self.assertTrue(cc.is_cpp("port/common/log.h", "namespace openrac::log {\n}\n"))
        self.assertTrue(
            cc.is_cpp("port/runtime/include/openrac/memory.h", "namespace openrac::runtime {\n}\n")
        )
        self.assertFalse(cc.is_cpp("port/common/x.h", "// namespace in a comment\nint f(void);\n"))
        self.assertTrue(cc.is_cpp("port/a.cpp", ""))
        self.assertFalse(cc.is_cpp("port/a.c", "namespace x {}\n"))


class StripTest(unittest.TestCase):
    def test_comments_and_literals_are_blanked_and_lines_kept(self):
        text = 'int a; // new thing\n/* free(\n */ f("delete x", \'n\');\n'
        stripped = cc.strip_comments_and_strings(text)
        self.assertEqual(stripped.count("\n"), text.count("\n"))
        self.assertNotIn("new", stripped)
        self.assertNotIn("free", stripped)
        self.assertNotIn("delete", stripped)
        self.assertIn('f("", \'\');', stripped)


class EveryFileTest(unittest.TestCase):
    def test_a_clean_file_has_no_findings(self):
        self.assertEqual(cc.check_every_file("port/a.cpp", LICENCE + "int a;\n"), [])

    def test_the_c_licence_form_counts(self):
        text = "/* SPDX-License-Identifier: GPL-3.0-or-later */\nint a;\n"
        self.assertEqual(cc.check_every_file("port/a.c", text), [])

    def test_licence_tab_and_dash_are_reported_with_their_lines(self):
        text = "int a;\n\tint b;\n// a \u2014 b\n"
        found = cc.check_every_file("port/a.cpp", text)
        self.assertEqual(rules(found), ["licence", "tab", "dash"])
        self.assertEqual([finding.line for finding in found], [1, 2, 3])


class NewCppTest(unittest.TestCase):
    def check(self, body, path="port/common/thing.cpp", stems=frozenset({"guest"})):
        return cc.check_new_cpp(path, LICENCE + body, stems)

    def test_a_clean_file_has_no_findings(self):
        body = (
            "namespace openrac {\nnamespace {\n\nconstexpr int kSize = 4;\n\n}  // namespace\n"
            "Thing::Thing() = default;\nThing& operator=(const Thing&) = delete;\n"
            "auto box = std::make_unique<Box>();\npool.free(slot);\n}  // namespace openrac\n"
        )
        self.assertEqual(self.check(body), [])

    def test_the_two_licence_lines_are_required(self):
        found = cc.check_new_cpp("port/a.cpp", "// SPDX-License-Identifier: MIT\nint a;\n", set())
        self.assertEqual(rules(found), ["licence"])

    def test_static_at_namespace_scope(self):
        self.assertEqual(rules(self.check("static int count;\n")), ["static"])
        self.assertEqual(self.check('static_assert(sizeof(int) == 4, "");\n'), [])
        self.assertEqual(self.check("void f() {\n    static int calls = 0;\n}\n"), [])

    def test_raw_new_and_delete(self):
        self.assertEqual(rules(self.check("Box* box = new Box;\n")), ["new-delete"])
        self.assertEqual(rules(self.check("delete box;\n")), ["new-delete"])
        self.assertEqual(rules(self.check("delete[] boxes;\n")), ["new-delete"])
        self.assertEqual(self.check("int new_size = 0;\n// a new box\n"), [])

    def test_a_name_that_starts_with_delete_is_not_a_delete(self):
        self.assertEqual(self.check("void delete_textures(int n, const unsigned* names);\n"), [])
        self.assertEqual(self.check("entries.push_back({glDelete, delete_buffers});\n"), [])

    def test_c_allocation(self):
        self.assertEqual(rules(self.check("void* p = malloc(4);\n")), ["malloc-free"])
        self.assertEqual(rules(self.check("std::free(p);\n")), ["malloc-free"])
        self.assertEqual(self.check("pool->free(p);\nallocator::free(p);\n"), [])

    def test_using_namespace_only_in_headers(self):
        self.assertEqual(self.check("using namespace openrac;\n"), [])
        found = self.check("using namespace std;\n", path="port/common/thing.h")
        self.assertEqual(rules(found), ["using-namespace"])

    def test_extern_c_only_where_a_contract_is_implemented(self):
        self.assertEqual(rules(self.check('extern "C" void f();\n')), ["extern-c"])
        self.assertEqual(self.check('extern "C" void f();\n', path="port/runtime/guest.cpp"), [])
        self.assertEqual(
            self.check('extern "C" void f();\n', path="port/game/common/lib/mc.cpp"), []
        )
        self.assertEqual(self.check('extern "C" void f();\n', path="port/tests/x_test.cpp"), [])

    def test_console_output_only_in_tests_and_main_files(self):
        self.assertEqual(rules(self.check('std::printf("x");\n')), ["logging"])
        self.assertEqual(rules(self.check("std::cerr << 1;\n")), ["logging"])
        self.assertEqual(self.check('std::printf("x");\n', path="port/extract/main.cpp"), [])
        self.assertEqual(self.check('std::printf("x");\n', path="port/tests/a_test.cpp"), [])
        self.assertEqual(self.check('log::info("printf(x)");\n'), [])


class NewCTest(unittest.TestCase):
    def test_a_missing_copyright_line_is_an_advisory(self):
        found = cc.check_new_c("port/a.c", "/* SPDX-License-Identifier: GPL-3.0-or-later */\n")
        self.assertEqual(rules(found), ["licence"])
        self.assertFalse(found[0].gating)

    def test_both_c_forms_count(self):
        block = "/* SPDX-License-Identifier: GPL-3.0-or-later\n * Copyright (c) 2026 the OpenRAC contributors\n"
        lines = "/* SPDX-License-Identifier: GPL-3.0-or-later */\n/* Copyright (c) 2026 the OpenRAC contributors */\n"
        self.assertEqual(cc.check_new_c("port/a.c", block), [])
        self.assertEqual(cc.check_new_c("port/a.c", lines), [])


@unittest.skipUnless(shutil.which("git"), "needs git")
class RunTest(unittest.TestCase):
    """A whole run on a throwaway repository: old files keep their faults, new ones may not."""

    def setUp(self):
        self.folder = tempfile.TemporaryDirectory()
        self.root = Path(self.folder.name)
        self.git("init", "-q")
        self.git("config", "user.email", "test@example.invalid")
        self.git("config", "user.name", "test")
        self.write("port/old.cpp", LICENCE + "static int old_count;\n")
        self.write("port/runtime/include/openrac/guest.h", "/* SPDX-License-Identifier: MIT */\n")
        self.git("add", "-A")
        self.git("commit", "-q", "-m", "base")

    def tearDown(self):
        self.folder.cleanup()

    def git(self, *args):
        subprocess.run(["git", *args], cwd=self.root, check=True, capture_output=True)

    def write(self, path, text):
        full = self.root / path
        full.parent.mkdir(parents=True, exist_ok=True)
        full.write_text(text, encoding="utf-8")

    def test_without_since_only_the_every_file_rules_run(self):
        self.write("port/new.cpp", LICENCE + "static int count;\n")
        self.assertEqual(cc.run(self.root), [])

    def test_only_c_headers_of_the_contract_directories_are_contracts(self):
        self.write("port/runtime/include/openrac/memory.h", "namespace openrac::runtime {\n}\n")
        self.assertEqual(cc.contract_stems(self.root), {"guest"})

    def test_with_since_new_cpp_files_are_held_to_the_new_code_rules(self):
        self.write("port/new.cpp", LICENCE + "static int count;\n")
        found = cc.run(self.root, since="HEAD", clang_format="/nonexistent/clang-format")
        failed = [finding for finding in found if finding.gating]
        self.assertEqual([(f.path, f.rule) for f in failed if f.rule != "format"], [("port/new.cpp", "static")])
        self.assertNotIn("port/old.cpp", {finding.path for finding in found})


if __name__ == "__main__":
    unittest.main()
