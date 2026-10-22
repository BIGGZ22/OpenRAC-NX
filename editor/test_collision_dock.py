"""The Collision dock (addons/openrac_collision) end to end on a real level: its logic run headless
in Godot (test_collision_dock.gd) edits a copy of an exported, imported level project, and the
packer must find exactly those edits. Needs a disc and Godot, so it runs only when told where:

    OPENRAC_GODOT=/path/to/Godot OPENRAC_DOCK_DISC=baserom/SCES_509.16.iso \\
    OPENRAC_DOCK_PROJECT=assets/levels/rac1/pal/level_00 python3 -m unittest test_collision_dock

The project is a level exported with `extract.py godot --split` and imported once
(`godot --headless --editor --quit --path PROJECT`); it is copied, never changed. Unset, the test
is skipped (the unit tests in test_collision_edit.py cover the edit format without a disc)."""

import json
import os
import re
import shutil
import subprocess
import tempfile
import unittest
from pathlib import Path

import collision as col
import collision_scene as cs
from disc import Disc
from godot import install_addon
from level import load_level
import moby_collision_scene as mcs
from formats import unpack

GODOT, DISC, PROJECT = (os.environ.get(k) for k in ("OPENRAC_GODOT", "OPENRAC_DOCK_DISC", "OPENRAC_DOCK_PROJECT"))
HERE = Path(__file__).parent


def key(record: list[int]) -> tuple:
    return (col.canonical((tuple(record[0:3]), tuple(record[3:6]), tuple(record[6:9]))), record[9])


@unittest.skipUnless(GODOT and DISC and PROJECT, "set OPENRAC_GODOT, OPENRAC_DOCK_DISC and OPENRAC_DOCK_PROJECT")
class DockOnALevel(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.project = Path(self.tmp.name) / "level"
        shutil.copytree(PROJECT, self.project, symlinks=True)
        install_addon(self.project)
        self.number = int(re.search(r"(\d+)$", Path(PROJECT).name).group(1))
        self.level_dir = self.project / "levels" / f"level_{self.number:02}"

    def tearDown(self):
        self.tmp.cleanup()

    def test_edits_pack_as_made(self):
        expect = Path(self.tmp.name) / "expect.json"
        run = subprocess.run([GODOT, "--headless", "--path", str(self.project), "--script",
                              str(HERE / "test_collision_dock.gd"), "--", str(self.number), str(expect)],
                             capture_output=True, text=True, timeout=600)
        self.assertIn("collision dock test: ok", run.stdout, run.stdout + run.stderr)
        self.assertEqual(run.returncode, 0)
        wanted = json.loads(expect.read_text())
        text = (self.level_dir / f"level_{self.number:02}.tscn").read_text()
        with Disc(Path(DISC)) as disc:
            info = next(i for i in disc.survey()["levels"] if i["id"] == self.number)
            level = load_level(disc, info)
            block = level.block(unpack("<I", level.index, 0x14)[0])
            found = cs.scene_collision(self.project, self.level_dir, text)
            new, changes, problems = cs.pack_block(block, found)
            self.assertEqual(problems, [])
            before = col.block_triangles(col.parse(block))
            after = {(col.canonical(t[:3]), t[3]) for t in col.decode(new).triangles}
            removed = {key(r) for r in wanted["removed"]}
            added = {key(r) for r in wanted["repainted"] + wanted["moved"] + wanted["added"]}
            self.assertEqual(len(removed), 3)
            self.assertTrue(removed <= before)
            self.assertEqual(after, before - removed | added)
            # Every cell the edit did not reach keeps its bytes.
            old, again = col.parse(block), col.parse(new)
            reached = {c for k, _ in removed | added for c in col.cells_of(k)}
            for cell, leaf in old.cells.items():
                if cell not in reached:
                    self.assertEqual(col.encode_leaf(cell, again.cells[cell]), col.encode_leaf(cell, leaf), cell)
            # The class the test gave a box: its collision is the box's twelve triangles now.
            classes, class_changes, class_problems = mcs.pack_classes(level, self.project, self.level_dir)
            self.assertEqual(class_problems, [])
            (edited,) = [int(c) for c in wanted["classes"]]
            self.assertEqual(list(classes), [edited])
            from moby_collision import parse
            blob = parse(classes[edited])
            self.assertEqual((len(blob.primitives), len(blob.faces)), (0, 12))
            # The project as exported (with the dock installed) packs nothing.
            pristine = Path(PROJECT)
            level_dir = pristine / "levels" / f"level_{self.number:02}"
            self.assertIsNone(cs.scene_collision(pristine, level_dir,
                                                 (level_dir / f"level_{self.number:02}.tscn").read_text()))
            self.assertEqual(mcs.pack_classes(level, pristine, level_dir)[0], {})


if __name__ == "__main__":
    unittest.main()
