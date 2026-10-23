# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""
Tests for layering.py: the real port/ against rules.tsv, then small made-up trees in a temporary
directory for each rule (a new violation fails, a removed or moved exception warns) and for
rules.tsv's own checks.

  python3 -m unittest discover -s port/tools/layering -v
"""
import contextlib
import io
import shutil
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import layering  # noqa: E402

# The made-up trees' rules: three layers, a wildcard module, one same-layer edge allowed.
RULES = """\
# A comment, and a blank line next.

search\t.
search\tlow/include
skip\tbuild/
skip\tgame/*/hand/
module\tlow/	low
module\tmid/a/\tmid/a
module\tmid/\tmid
module\tgame/common/\tgame/common
module\tgame/*/\tgame/*
module\ttests/\ttests
layer\t0\tlow
layer\t1\tmid mid/a
layer\t2\tgame/common
layer\t3\tgame/*
layer\t4\ttests
allow\tmid/a\tmid
"""


class Tree:
    """A made-up port tree in a temporary directory, with its rules.tsv beside it."""

    def __init__(self, test, rules=RULES):
        top = Path(tempfile.mkdtemp(prefix="layering_test_"))
        test.addCleanup(shutil.rmtree, top)
        self.rules_path = top / "rules.tsv"
        self.rules_path.write_text(rules, encoding="utf-8")
        self.root = top / "port"
        self.root.mkdir()

    def add(self, path, *lines):
        file = self.root / path
        file.parent.mkdir(parents=True, exist_ok=True)
        file.write_text("".join(line + "\n" for line in lines), encoding="utf-8")

    def except_(self, location, include, reason="P9-99 removes it"):
        with self.rules_path.open("a", encoding="utf-8") as rules:
            rules.write(f"except\t{location}\t{include}\t{reason}\n")

    def check(self):
        return layering.check(self.root, layering.load_rules(self.rules_path))

    def main(self, *extra):
        """layering.py's exit code and its standard output on this tree."""
        out = io.StringIO()
        with contextlib.redirect_stdout(out), contextlib.redirect_stderr(io.StringIO()):
            code = layering.main(["--root", str(self.root), "--rules", str(self.rules_path), *extra])
        return code, out.getvalue()


class RealTreeTest(unittest.TestCase):
    def test_port_has_no_violation_today(self):
        report = layering.check(layering.DEFAULT_ROOT, layering.load_rules(layering.DEFAULT_RULES))
        self.assertEqual(report.violations, [])
        self.assertGreater(len(report.edges), 0)

    def test_every_exception_names_a_tracked_file(self):
        rules = layering.load_rules(layering.DEFAULT_RULES)
        for known in rules.known.values():
            self.assertTrue((layering.DEFAULT_ROOT / known.path).is_file(), known.path)


class LayerTest(unittest.TestCase):
    def setUp(self):
        self.tree = Tree(self)
        self.tree.add("low/base.h", "#pragma once")
        self.tree.add("mid/mid.h", "#pragma once")

    def test_a_lower_layer_and_the_own_module_are_allowed(self):
        self.tree.add("mid/a/a.cpp", '#include "mid/a/a.h"', '#include "low/base.h"')
        self.tree.add("mid/a/a.h", "#pragma once")
        report = self.tree.check()
        self.assertEqual(report.violations, [])
        self.assertEqual(report.warnings, [])
        self.assertEqual(len(report.edges), 2)

    def test_a_new_include_of_a_higher_layer_fails(self):
        self.tree.add("low/base.cpp", "// A comment.", '#include "mid/mid.h"')
        report = self.tree.check()
        self.assertEqual(
            report.violations, ['low/base.cpp:2: layering: low must not include mid (#include "mid/mid.h")']
        )
        self.assertEqual(self.tree.main()[0], 1)

    def test_the_same_layer_needs_an_allow_row(self):
        self.tree.add("mid/a/a.h", '#include "mid/mid.h"')
        self.tree.add("mid/b.cpp", '#include "mid/a/a.h"')
        report = self.tree.check()
        self.assertEqual(len(report.violations), 1)
        self.assertIn("mid/b.cpp:1: layering: mid must not include mid/a", report.violations[0])

    def test_a_wildcard_module_is_one_module_per_directory(self):
        self.tree.add("game/common/host.h", '#include "low/base.h"')
        self.tree.add("game/one/one.c", '#include "game/common/host.h"')
        self.tree.add("game/two/two.c", '#include "game/one/one.h"')
        self.tree.add("game/one/one.h", "#pragma once")
        report = self.tree.check()
        self.assertEqual(len(report.violations), 1)
        self.assertIn("game/two must not include game/one", report.violations[0])

    def test_a_file_no_module_covers_fails(self):
        self.tree.add("stray/stray.cpp", '#include "low/base.h"')
        report = self.tree.check()
        self.assertEqual(len(report.violations), 1)
        self.assertTrue(report.violations[0].startswith("stray/stray.cpp:1: layering: no module row"))

    def test_edges_lists_each_module_pair_and_marks_the_violations(self):
        self.tree.add("low/base.cpp", '#include "mid/mid.h"')
        self.tree.add("mid/m.cpp", '#include "low/base.h"', '#include "low/base.h"')
        code, out = self.tree.main("--edges")
        self.assertEqual(code, 1)
        self.assertIn("low -> mid: 1  (against the layers)\n", out)
        self.assertIn("mid -> low: 2\n", out)


class ExceptionTest(unittest.TestCase):
    def setUp(self):
        self.tree = Tree(self)
        self.tree.add("mid/mid.h", "#pragma once")
        self.tree.add("low/base.cpp", "// Line 1.", '#include "mid/mid.h"')

    def test_a_listed_exception_passes(self):
        self.tree.except_("low/base.cpp:2", "mid/mid.h")
        report = self.tree.check()
        self.assertEqual(report.violations, [])
        self.assertEqual(report.warnings, [])
        self.assertEqual(self.tree.main()[0], 0)

    def test_a_removed_exception_warns_without_failing(self):
        self.tree.except_("low/base.cpp:2", "mid/mid.h")
        self.tree.add("low/base.cpp", "// The include is gone.")
        report = self.tree.check()
        self.assertEqual(report.violations, [])
        self.assertEqual(len(report.warnings), 1)
        self.assertIn("low/base.cpp:2: no longer breaks the layers", report.warnings[0])
        self.assertIn("remove the row", report.warnings[0])
        code, out = self.tree.main()
        self.assertEqual(code, 0)
        self.assertIn("(warning)", out)

    def test_an_exception_whose_edge_became_allowed_warns(self):
        self.tree.except_("low/base.cpp:2", "mid/mid.h")
        self.tree.add("low/base.cpp", "// Line 1.", '#include "low/other.h"')
        self.tree.add("low/other.h", "#pragma once")
        self.assertEqual(len(self.tree.check().warnings), 1)

    def test_a_moved_exception_warns_with_its_new_line(self):
        self.tree.except_("low/base.cpp:2", "mid/mid.h")
        self.tree.add("low/base.cpp", "// Line 1.", "// Line 2.", '#include "mid/mid.h"')
        report = self.tree.check()
        self.assertEqual(report.violations, [])
        self.assertEqual(len(report.warnings), 1)
        self.assertIn('"mid/mid.h" is now at line 3; update the row', report.warnings[0])

    def test_an_exception_covers_only_its_file_and_include(self):
        self.tree.except_("low/base.cpp:2", "mid/mid.h")
        self.tree.add("low/next.cpp", '#include "mid/mid.h"')
        report = self.tree.check()
        self.assertEqual(len(report.violations), 1)
        self.assertTrue(report.violations[0].startswith("low/next.cpp:1:"))


class ScopeTest(unittest.TestCase):
    def setUp(self):
        self.tree = Tree(self)
        self.tree.add("mid/mid.h", "#pragma once")

    def test_skipped_directories_and_generated_files_are_not_read(self):
        self.tree.add("build/release/gen.c", '#include "mid/mid.h"')
        self.tree.add("game/one/hand/f.c", '#include "tests/check.h"')
        self.tree.add("low/gen.c", "/* Generated from x by y. Do not edit. */", '#include "mid/mid.h"')
        self.tree.add(".cache/x.c", '#include "mid/mid.h"')
        self.tree.add("low/notes.txt", '#include "mid/mid.h"')
        report = self.tree.check()
        self.assertEqual(report.violations, [])
        self.assertEqual(report.edges, [])

    def test_a_skipped_file_is_still_a_module_when_included(self):
        self.tree.add("game/one/hand/f.c", "int f;")
        self.tree.add("tests/f_test.c", '#include "../game/one/hand/f.c"')
        self.tree.add("low/base.c", '#include "../game/one/hand/f.c"')
        report = self.tree.check()
        self.assertEqual([edge.target for edge in report.edges], ["game/one", "game/one"])
        self.assertEqual(len(report.violations), 1)
        self.assertIn("low must not include game/one", report.violations[0])

    def test_includes_without_a_directory_or_outside_every_module_are_ignored(self):
        self.tree.add("low/base.cpp", '#include "local.h"', '#include "imgui/imgui.h"')
        self.tree.add("low/system.cpp", "#include <mid/mid.h>")
        report = self.tree.check()
        self.assertEqual(report.edges, [])

    def test_every_source_suffix_is_read(self):
        for suffix in layering.SOURCE_SUFFIXES:
            self.tree.add(f"low/file{suffix}", '#include "mid/mid.h"')
        self.assertEqual(len(self.tree.check().violations), len(layering.SOURCE_SUFFIXES))


class ResolveTest(unittest.TestCase):
    def setUp(self):
        self.tree = Tree(self)

    def test_the_including_directory_comes_first(self):
        # low/mid/mid.h exists, so "mid/mid.h" from low/ is low's own file, as for the compiler.
        self.tree.add("low/mid/mid.h", "#pragma once")
        self.tree.add("mid/mid.h", "#pragma once")
        self.tree.add("low/base.cpp", '#include "mid/mid.h"')
        report = self.tree.check()
        self.assertEqual(report.violations, [])
        self.assertEqual(report.edges[0].target, "low")

    def test_a_search_directory_resolves_a_contract_header(self):
        self.tree.add("low/include/api/contract.h", "#pragma once")
        self.tree.add("mid/user.cpp", '#include "api/contract.h"')
        report = self.tree.check()
        self.assertEqual([(edge.source, edge.target) for edge in report.edges], [("mid", "low")])

    def test_a_header_missing_from_the_tree_is_named_from_the_root(self):
        self.tree.add("low/base.cpp", '#include "mid/generated.h"')
        report = self.tree.check()
        self.assertEqual(len(report.violations), 1)


class RulesTest(unittest.TestCase):
    def load(self, extra, base=RULES):
        tree = Tree(self, base + extra)
        return layering.load_rules(tree.rules_path)

    def test_the_rules_load(self):
        rules = self.load("")
        self.assertEqual(rules.module_of("game/common/x.h"), "game/common")
        self.assertEqual(rules.module_of("game/rac9/x.c"), "game/rac9")
        self.assertEqual(rules.module_of("mid/a/x.h"), "mid/a")
        self.assertIsNone(rules.module_of("elsewhere/x.h"))
        self.assertEqual(rules.rank("game/rac9"), 3)
        self.assertEqual(rules.rank("game/common"), 2)

    def test_an_allow_row_that_points_up_a_layer_is_refused(self):
        with self.assertRaisesRegex(layering.RulesError, "points up a layer"):
            self.load("allow\tlow\tmid\n")

    def test_a_module_without_a_layer_is_refused(self):
        tree = Tree(self, RULES + "module\tnew/\tnew\n")
        tree.add("new/x.cpp", "int x;")
        with self.assertRaisesRegex(layering.RulesError, "no layer row names the module new"):
            tree.check()
        self.assertEqual(tree.main()[0], 2)

    def test_malformed_rows_are_refused(self):
        for row, message in (
            ("unknown\tx\n", "unknown row"),
            ("module\tonly-one\n", "takes 2"),
            ("layer\tfirst\tx\n", "rank is a number"),
            ("module\ta/*/b/*/\tx/*\n", "at most"),
            ("module\ta/\tx/*\n", "at most"),
            ("except\tno-line\tmid/mid.h\twhy\n", "file:line"),
            ("except\tlow/a.c:1\tmid/mid.h\t\n", "takes 3"),
        ):
            with self.subTest(row=row), self.assertRaisesRegex(layering.RulesError, message):
                self.load(row)

    def test_an_exception_listed_twice_is_refused(self):
        row = "except\tlow/a.c:1\tmid/mid.h\twhy\n"
        with self.assertRaisesRegex(layering.RulesError, "listed twice"):
            self.load(row + row)


if __name__ == "__main__":
    unittest.main()
