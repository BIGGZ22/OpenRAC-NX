"""Assembly must not become C evidence through a source or preprocessor shortcut."""
import json
from pathlib import Path
import sys
import tempfile
from types import SimpleNamespace
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from c_source_policy import policy_errors
import compile_boot_trial as boot
import level_c_gate as level


class SourcePolicyTests(unittest.TestCase):
    def test_retired_padding_probe_is_not_public_c_even_when_disabled(self):
        source = '#ifdef RAC5_WITNESS_PADDING\nint RAC5_padding_sentinel(int x) { return x; }\n#endif'
        self.assertTrue(any("alignment probes" in error for error in policy_errors(source)))

    def test_comments_literals_and_symbol_only_labels_are_allowed(self):
        source = '''#include "header.h"
// asm("nop") and register bindings in explanatory text do not emit code.
void ctor(void) __asm__("_GLOBAL_$I$unit.cppabc");
# 1 "unit.c"
extern int data __asm__("retail.data");
const char *description = "asm(\\\"syscall\\\")";
void f(void) { char brace = '}'; }
'''
        self.assertEqual(policy_errors(source), [])

    def test_rejects_instruction_assembly_and_register_pins(self):
        for source in (
            'void f(void) { __asm__ volatile("syscall"); }',
            'void f(void) { asm(".word 0x40026000"); }',
            'void f(void) { __asm__("" ::: "memory"); }',
            'asm(".text\\nnop");',
            'register int state __asm__("$2");',
            'int state __asm__("$2");',
            '#define HIDDEN __asm__\nvoid f(void) { HIDDEN("nop"); }',
            'void f(void) { __a\\\nsm__("nop"); }',
        ):
            with self.subTest(source=source):
                self.assertTrue(policy_errors(source))

    def test_boot_gate_rejects_preprocessed_assembly_before_cc1(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source = root / "unit.c"
            source.write_text('void f(void) { HIDDEN(); }\n')
            item = SimpleNamespace(name="unit", source_sha256=boot.normalized_digest(source))
            profile = {"id": "synthetic", "flags": {"cpp": [], "cc1": [], "as": []}}
            def fake_cpp(command, cwd, log, stdout=None):
                stdout.write_text('void f(void) { __asm__("nop"); }\n')
            with patch.object(boot, "check_profile", return_value=profile), \
                 patch.object(boot.cflags, "load", return_value={}), \
                 patch.object(boot, "REPLACEMENTS", (item,)), \
                 patch.object(boot, "source_path", return_value=source), \
                 patch.object(boot, "run", side_effect=fake_cpp) as run:
                with self.assertRaisesRegex(ValueError, "preprocessed input"):
                    boot.compile_objects(Path("cpp"), Path("cc1"), Path("as"), root / "out", None)
                self.assertEqual(run.call_count, 1)

    def test_level_gate_rejects_preprocessed_assembly_before_cc1(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "unit.c").write_text('void f(void) { HIDDEN(); }\n')
            profile = root / "profile.json"
            profile.write_text(json.dumps({"flags": {"cc1": []}}))
            def fake_cpp(command, cwd, log, stdout=None):
                stdout.write_text('void f(void) { __asm__("nop"); }\n')
            with patch.object(level, "ROOT", root), patch.object(level, "OUTPUT", root / "export"), \
                 patch.object(level, "BASE_PROFILE", profile), \
                 patch.object(level, "run", side_effect=fake_cpp) as run:
                with self.assertRaisesRegex(ValueError, "preprocessed input"):
                    level.compile_sources({}, [{"source": "unit.c"}], Path("cpp"), Path("cc1"),
                                          Path("as"), root / "out")
                self.assertEqual(run.call_count, 1)

    def test_profile_cannot_silently_keep_an_obsolete_witness_count(self):
        profile = json.loads(boot.PROFILE.read_text())
        profile["expected_function_witnesses"] = 20
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "profile.json"
            path.write_text(json.dumps(profile))
            with patch.object(boot, "PROFILE", path):
                with self.assertRaisesRegex(ValueError, "witness counts"):
                    boot.check_profile(Path("cpp"), Path("cc1"), Path("as"))
