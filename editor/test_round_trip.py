"""The level editor's whole round trip on a real disc, as the launcher's Mods page runs it: a level
read into a Godot project, a crate and a tie moved in its scene, the edits packed, packed again
over the first pack, and stacked into the folder the game reads with --mods.

It needs the image of a Ratchet & Clank disc (PAL SCES-50916, NTSC-U SCUS-97199), which is never
in the repository: OPENRAC_RAC1_PAL_DISC / OPENRAC_RAC1_NTSC_DISC name one, else the usual places
are looked in (games/rac1/baserom, ../rac1-decomp/baserom). Without one the tests are skipped.
Level 0 (Veldin) is the one tested: players reported it, and it is the first a new game loads.
About two and a half minutes a disc, most of it the levels the game draws.
"""

import json
import os
import re
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

from formats import wad
from mobys import moby_instances

EDITOR = Path(__file__).resolve().parent
ROOT = EDITOR.parent
DISCS = {
    "pal": ("OPENRAC_RAC1_PAL_DISC", "SCES_509.16.iso"),
    "ntsc": ("OPENRAC_RAC1_NTSC_DISC", "SCUS_971.99.iso"),
}


def find_disc(version: str) -> Path | None:
    variable, name = DISCS[version]
    if os.environ.get(variable):
        return Path(os.environ[variable])
    for folder in (ROOT / "games" / "rac1" / "baserom", ROOT.parent / "rac1-decomp" / "baserom"):
        if (folder / name).is_file():
            return folder / name
    return None


def run(*args: str) -> subprocess.CompletedProcess:
    return subprocess.run([sys.executable, *args], cwd=ROOT, capture_output=True, text=True)


def nudge(scene: Path, node: str, axis: int, by: float) -> list[float]:
    """Moves the node NAME of the scene file by BY along AXIS (0 x, 1 y, 2 z), as saving in Godot
    after a move would; its new origin."""
    text = scene.read_text(encoding="utf-8")
    at = text.index(f'[node name="{node}"')
    start = text.index("transform = Transform3D(", at) + len("transform = Transform3D(")
    end = text.index(")", start)
    values = [float(v) for v in text[start:end].split(", ")]
    values[9 + axis] += by
    scene.write_text(text[:start] + ", ".join(repr(v) for v in values) + text[end:], encoding="utf-8")
    return values[9:12]


def first_node(scene: Path, pattern: str) -> str:
    """The name of the first node whose header matches PATTERN."""
    match = re.search(r'\[node name="([^"]+)"[^\]]*' + pattern, scene.read_text(encoding="utf-8"))
    assert match, pattern
    return match.group(1)


class RoundTrip(unittest.TestCase):
    def check_version(self, version: str):
        disc = find_disc(version)
        if disc is None:
            self.skipTest(f"no {version} disc image (set {DISCS[version][0]})")
        with tempfile.TemporaryDirectory() as tmp:
            # Outside the repository, as an installed launcher's data folder is.
            data = Path(tmp)
            levels, packed, mods = data / "levels" / version, data / "packed" / version, data / "mods" / version

            done = run("editor/extract.py", "godot", str(disc), str(levels), "--split", "--level", "0", "--anywhere")
            self.assertEqual(done.returncode, 0, done.stderr)
            project = levels / "level_00"
            scene = project / "levels" / "level_00" / "level_00.tscn"
            self.assertTrue((project / "project.godot").is_file())
            self.assertIn('run/main_scene="res://levels/level_00/level_00.tscn"',
                          (project / "project.godot").read_text(encoding="utf-8"))
            self.assertTrue(scene.is_file())

            # Nothing edited: nothing packed.
            done = run("editor/extract.py", "pack", str(disc), str(levels), str(packed), "--anywhere")
            self.assertEqual(done.returncode, 0, done.stderr)
            self.assertIn("No level differs from the disc", done.stdout)
            self.assertFalse((packed / "level_00").exists())

            # Outside the repository only when told so.
            done = run("editor/extract.py", "pack", str(disc), str(levels), str(packed))
            self.assertNotEqual(done.returncode, 0)
            self.assertIn("under this repository", done.stderr)

            # A crate raised, a tie lowered.
            crate = first_node(scene, r'instance=ExtResource\("moby_500"\)')
            tie = first_node(scene, r'parent="Game/Ties"')
            crate_at = nudge(scene, crate, 2, 10.0)
            tie_at = nudge(scene, tie, 2, -3.0)
            done = run("editor/extract.py", "pack", str(disc), str(levels), str(packed), "--anywhere")
            self.assertEqual(done.returncode, 0, done.stderr)
            self.assertIn(f"{crate}: moved", done.stdout)
            self.assertIn(f"tie {tie}: moved", done.stdout)
            self.assertIn("Packed 1 level(s)", done.stdout)

            name = f"level_00/gameplay_{version}.wad"
            listed = (packed / "patches.tsv").read_text().splitlines()
            self.assertEqual(len(listed), 2, listed)
            self.assertTrue(listed[1].endswith(f"\t{name}"), listed)
            index = int(re.search(r"\d+$", crate).group())
            moved = [p for p in moby_instances(wad((packed / name).read_bytes())) if p["index"] == index]
            self.assertEqual(len(moved), 1)
            for got, want in zip(moved[0]["matrix"][12:15], crate_at):
                self.assertAlmostEqual(got, want, places=3)
            placements = json.loads((packed / "level_00" / "placements.json").read_text())
            tie_index = int(re.search(r"\d+$", tie).group())
            placed = [t for t in placements["ties"] if t["index"] == tie_index]
            self.assertEqual(len(placed), 1)
            self.assertAlmostEqual(placed[0]["matrix"][14], tie_at[2], places=3)

            # Packing again (the Pack button after more editing) writes over the first pack.
            nudge(scene, crate, 0, 1.0)
            done = run("editor/extract.py", "pack", str(disc), str(levels), str(packed), "--anywhere")
            self.assertEqual(done.returncode, 0, done.stderr)
            again = [p for p in moby_instances(wad((packed / name).read_bytes())) if p["index"] == index]
            self.assertAlmostEqual(again[0]["matrix"][12], crate_at[0] + 1.0, places=3)

            # Play with mods: the packed levels stacked into the folder the game reads.
            port = data / "port" / version
            done = run("editor/extract.py", "port", str(disc), str(port), "--level", "0", "--anywhere", "--replace")
            self.assertEqual(done.returncode, 0, done.stderr)
            (data / "overrides").mkdir()
            done = run("editor/overrides.py", "stack", "--into", str(data / "overrides"), "--out", str(mods),
                       "--packed", str(packed), "--levels", str(port))
            self.assertEqual(done.returncode, 0, done.stderr)
            self.assertEqual((mods / name).read_bytes(), (packed / name).read_bytes())
            self.assertTrue((mods / "level_00" / "placements.json").is_file())
            self.assertIn(name, (mods / "patches.tsv").read_text())

    def test_pal(self):
        self.check_version("pal")

    def test_ntsc(self):
        self.check_version("ntsc")


if __name__ == "__main__":
    unittest.main()
