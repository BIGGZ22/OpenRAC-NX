# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""Tests for derived.py: where the names a decompilation's build derives come from.

    python3 -m unittest port/tools/hostgen/test_derived.py      (no Clang needed)
"""
from __future__ import annotations

import io
import json
import sys
import tempfile
import unittest
from contextlib import redirect_stderr, redirect_stdout
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

import derived  # noqa: E402
import hostgen  # noqa: E402

REPO = HERE.parent.parent.parent
CFG = {"id": "test", "sources": ["src"], "includes": ["include", "src"],
       "derived": {"build": "build/boot/placed.json", "table": "derived_names.json"}}


class Fixture:
    def __init__(self, d: Path):
        self.source = d / "decomp"
        self.config_dir = d / "game"
        (self.source / "src").mkdir(parents=True)
        (self.source / "include").mkdir()
        self.config_dir.mkdir()
        (self.source / "src" / "a.c").write_text(
            "extern int r1_func_0011DB98(void);\n"
            "int f(void) { return r1_func_0011DB98() + F0123abcd_FUN_00200000(); }\n")
        (self.source / "include" / "a.h").write_text("extern int r1_D_00130420;\n")

    def write_build(self, values: dict) -> Path:
        p = self.source / "build" / "boot" / "placed.json"
        p.parent.mkdir(parents=True, exist_ok=True)
        p.write_text(json.dumps({"program": "boot", "functions": {}, "values": values}))
        return p


class Resolve(unittest.TestCase):
    def test_none_without_derived(self):
        with tempfile.TemporaryDirectory() as d:
            self.assertIsNone(derived.resolve(Path(d), Path(d), {"id": "rac1-pal"}))

    def test_build_first_then_the_copy(self):
        with tempfile.TemporaryDirectory() as d:
            fx = Fixture(Path(d))
            table = fx.config_dir / "derived_names.json"
            table.write_text(json.dumps({"values": {}}))
            self.assertEqual(derived.resolve(fx.source, fx.config_dir, CFG), table)
            build = fx.write_build({})
            self.assertEqual(derived.resolve(fx.source, fx.config_dir, CFG), build)

    def test_neither_is_an_error(self):
        with tempfile.TemporaryDirectory() as d:
            fx = Fixture(Path(d))
            with self.assertRaises(derived.Missing) as e:
                derived.resolve(fx.source, fx.config_dir, CFG)
            self.assertIn("derived.py", str(e.exception))

    def test_hostgen_stops_without_a_table(self):
        with tempfile.TemporaryDirectory() as d:
            fx = Fixture(Path(d))
            config = fx.config_dir / "hostgen.json"
            config.write_text(json.dumps(CFG))
            err = io.StringIO()
            with redirect_stderr(err), redirect_stdout(io.StringIO()):
                status = hostgen.main(["--game", str(config), "--source", str(fx.source),
                                       "--out", str(Path(d) / "out"), "--list"])
            self.assertEqual(status, 1)
            self.assertIn("no table of the names", err.getvalue())


class Table(unittest.TestCase):
    def test_make_and_cover(self):
        with tempfile.TemporaryDirectory() as d:
            fx = Fixture(Path(d))
            build = {"program": "boot", "values": {"r1_func_0011DB98": "0x0011f818", "D_00130000": "0x00130000"}}
            table = derived.make_table(fx.source, CFG, build, "abc")
            self.assertEqual(table["source"], {"commit": "abc", "program": "boot", "table": "build/boot/placed.json"})
            self.assertEqual(table["values"]["r1_func_0011DB98"], "0x0011f818")
            # Used, but the build derived nothing for them (their functions stay assembly there).
            self.assertEqual(table["unresolved"], ["F0123abcd_FUN_00200000", "r1_D_00130420"])
            self.assertEqual(derived.uncovered(fx.source, CFG, table), [])
            # A name the C starts to use after the table was written.
            (fx.source / "src" / "b.c").write_text("int g(void) { return r1_func_00120000(); }\n")
            self.assertEqual(derived.uncovered(fx.source, CFG, table), ["r1_func_00120000"])

    def test_write_and_check(self):
        with tempfile.TemporaryDirectory() as d:
            fx = Fixture(Path(d))
            config = fx.config_dir / "hostgen.json"
            config.write_text(json.dumps(CFG))
            with redirect_stdout(io.StringIO()), redirect_stderr(io.StringIO()):
                self.assertEqual(derived.main(["--game", str(config), "--source", str(fx.source)]), 1)
                fx.write_build({"r1_func_0011DB98": "0x0011f818"})
                self.assertEqual(derived.main(["--game", str(config), "--source", str(fx.source)]), 0)
                self.assertEqual(derived.main(["--game", str(config), "--source", str(fx.source), "--check"]), 0)
                (fx.source / "src" / "b.c").write_text("int g(void) { return r1_D_00150000; }\n")
                self.assertEqual(derived.main(["--game", str(config), "--source", str(fx.source), "--check"]), 1)
            values = json.loads((fx.config_dir / "derived_names.json").read_text())["values"]
            self.assertEqual(values, {"r1_func_0011DB98": "0x0011f818"})


class Committed(unittest.TestCase):
    """Going Commando's copy covers the C OpenRAC carries (a sync that adds derived names needs
    the copy written again from a build of that commit)."""

    def test_rac2_covers_its_c(self):
        config = REPO / "port" / "game" / "rac2-ntsc" / "hostgen.json"
        cfg = json.loads(config.read_text())
        source = REPO / cfg["source"]
        if not (source / "src").is_dir():
            self.skipTest("no games/rac2/ntsc")
        table = json.loads((config.parent / cfg["derived"]["table"]).read_text())
        self.assertEqual(derived.uncovered(source, cfg, table), [])

    def test_only_rac2_has_one(self):
        games = sorted(p.parent.name for p in (REPO / "port" / "game").glob("*/hostgen.json")
                       if "derived" in json.loads(p.read_text()))
        self.assertEqual(games, ["rac2-ntsc"])


if __name__ == "__main__":
    unittest.main()
