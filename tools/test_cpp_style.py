# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""tools/cpp_style.py on small C++ snippets: the blank lines around control blocks, the
control blocks without a comment, and which files the tool covers. The clang-format pass runs
only when a clang-format is installed: python3 -m unittest discover -s tools"""

import tempfile
import unittest
from pathlib import Path

import cpp_style


def lines(*rows: str) -> str:
    return "\n".join(rows)


class SpaceBlocksTests(unittest.TestCase):
    def test_a_block_gets_a_blank_line_before_and_after(self):
        before = lines("void f() {", "    int a = 1;", "    if (a) {", "        g();", "    }", "    return;", "}")
        after = lines("void f() {", "    int a = 1;", "", "    if (a) {", "        g();", "    }", "", "    return;", "}")
        self.assertEqual(cpp_style.space_blocks(before), after)

    def test_none_at_the_start_or_end_of_the_enclosing_block(self):
        text = lines("void f() {", "    if (a) {", "        g();", "    }", "}")
        self.assertEqual(cpp_style.space_blocks(text), text)

    def test_none_between_a_brace_and_its_else(self):
        text = lines("void f() {", "    h();", "", "    if (a) {", "        g();", "    } else if (b) {", "        i();",
                     "    } else {", "        j();", "    }", "}")
        self.assertEqual(cpp_style.space_blocks(text), text)

    def test_comment_lines_above_a_block_stay_with_it(self):
        before = lines("void f() {", "    h();", "    // Why the check.", "    // More of why.", "    if (a) {", "        g();",
                       "    }", "}")
        after = lines("void f() {", "    h();", "", "    // Why the check.", "    // More of why.", "    if (a) {",
                      "        g();", "    }", "}")
        self.assertEqual(cpp_style.space_blocks(before), after)

    def test_a_do_while_is_one_block(self):
        before = lines("void f() {", "    h();", "    do {", "        g();", "    } while (a);", "    h();", "}")
        after = lines("void f() {", "    h();", "", "    do {", "        g();", "    } while (a);", "", "    h();", "}")
        self.assertEqual(cpp_style.space_blocks(before), after)

    def test_a_wrapped_condition_is_one_block(self):
        before = lines("void f() {", "    h();", "    if (", "        a", "        && b", "    ) {", "        g();", "    }",
                       "    h();", "}")
        after = lines("void f() {", "    h();", "", "    if (", "        a", "        && b", "    ) {", "        g();",
                      "    }", "", "    h();", "}")
        self.assertEqual(cpp_style.space_blocks(before), after)

    def test_a_label_before_a_block_needs_no_blank_line(self):
        text = lines("switch (a) {", "case 1:", "    if (b) {", "        g();", "    }", "", "    break;", "case 2:", "    break;", "}")
        self.assertEqual(cpp_style.space_blocks(text), text)

    def test_keywords_inside_names_are_not_blocks(self):
        text = lines("void f() {", "    h();", "    done(a);", "    for_each(b);", "    iffy();", "}")
        self.assertEqual(cpp_style.space_blocks(text), text)

    def test_comments_strings_and_directives_are_left_alone(self):
        text = lines("void f() {", "    h();", "    /* if (a) {", "       g();", "    } */", '    s = "if (a) {";',
                     "#define LOOP() while (1) { g(); }", "    h();", "}")
        self.assertEqual(cpp_style.space_blocks(text), text)

    def test_a_region_fenced_for_the_tool_is_left_alone(self):
        text = lines("void f() {", "    h();", "    // clang-format off: a table.", "    if (a) { g(); }", "    h();",
                     "    // clang-format on", "}")
        self.assertEqual(cpp_style.space_blocks(text), text)

    def test_a_class_or_function_end_gets_no_blank_line(self):
        text = lines("namespace n {", "", "struct S {", "    int a;", "};", "void f() {", "    h();", "}", "", "}  // namespace n")
        self.assertEqual(cpp_style.space_blocks(text), text)

    def test_the_result_is_stable(self):
        text = lines("void f() {", "    h();", "    for (int n = 0; n < 3; ++n) {", "        if (n) {", "            g();", "        }",
                     "        i();", "    }", "    j();", "}")
        once = cpp_style.space_blocks(text)
        self.assertEqual(cpp_style.space_blocks(once), once)


class CommentsTests(unittest.TestCase):
    def test_a_block_without_a_comment_is_listed(self):
        text = lines("void f() {", "    h();", "", "    if (a) {", "        g();", "    }", "}")
        self.assertEqual(cpp_style.uncommented(text), [4])

    def test_a_block_with_a_comment_is_not(self):
        text = lines("void f() {", "    h();", "", "    // The reason.", "    if (a) {", "        g();", "    }", "}")
        self.assertEqual(cpp_style.uncommented(text), [])

    def test_a_null_check_of_what_the_line_before_fetched_is_not(self):
        text = lines("void f() {", "    Thing* t = find();", "    if (!t) {", "        return;", "    }", "}")
        self.assertEqual(cpp_style.uncommented(text), [])

    def test_a_null_check_of_something_else_is_listed(self):
        text = lines("void f() {", "    Thing* t = find();", "    if (!other) {", "        return;", "    }", "}")
        self.assertEqual(cpp_style.uncommented(text), [3])

    def test_an_else_is_not_a_block_of_its_own(self):
        text = lines("void f() {", "    // Why.", "    if (a) {", "        g();", "    } else {", "        h();", "    }", "}")
        self.assertEqual(cpp_style.uncommented(text), [])


class ScopeTests(unittest.TestCase):
    def test_port_and_sdk_cpp_are_covered(self):
        self.assertTrue(cpp_style.in_scope(cpp_style.ROOT / "port" / "media" / "idct.cpp"))
        self.assertTrue(cpp_style.in_scope(cpp_style.ROOT / "sdk" / "include" / "openrac" / "mod.hpp"))

    def test_games_and_other_trees_are_not(self):
        self.assertFalse(cpp_style.in_scope(cpp_style.ROOT / "games" / "rac1" / "src" / "x.cpp"))
        self.assertFalse(cpp_style.in_scope(cpp_style.ROOT / "launcher" / "x.cpp"))

    def test_hand_c_build_output_c_and_objective_cpp_are_not(self):
        self.assertFalse(cpp_style.in_scope(cpp_style.ROOT / "port" / "game" / "rac1" / "hand" / "x.h"))
        self.assertFalse(cpp_style.in_scope(cpp_style.ROOT / "port" / "build" / "release" / "x.cpp"))
        self.assertFalse(cpp_style.in_scope(cpp_style.ROOT / "port" / "game" / "common" / "lib" / "x.c"))
        self.assertFalse(cpp_style.in_scope(cpp_style.ROOT / "port" / "platform" / "x.mm"))

    def test_a_generated_file_is_recognised_by_its_first_lines(self):
        self.assertTrue(cpp_style.is_generated("// Generated.\n// Do not edit.\nint x;\n"))
        self.assertFalse(cpp_style.is_generated("// A file.\n//\n// Do not edit by hand later.\n"))


@unittest.skipUnless(cpp_style.find_clang_format(), "clang-format is not installed")
class LayoutTests(unittest.TestCase):
    def test_braces_are_added_and_the_result_is_stable(self):
        source = "#include <vector>\nint f(int a){ int b=a; if(a) return b; return 0; }\n"
        with tempfile.TemporaryDirectory() as folder:
            path = Path(folder) / "x.cpp"
            path.write_text(source, encoding="utf-8")
            command = cpp_style.find_clang_format()
            once = cpp_style.formatted(command, path)
            path.write_text(once, encoding="utf-8")
            self.assertIn("if (a) {\n        return b;\n    }", once)
            self.assertEqual(cpp_style.formatted(command, path), once)


if __name__ == "__main__":
    unittest.main()
