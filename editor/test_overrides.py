"""Overrides packs (overrides.py): checking, adding from folders and zips, the
order and state file, and stacking them for the port's --mods. Synthetic files."""

import io
import json
from contextlib import redirect_stderr, redirect_stdout
from pathlib import Path
import tempfile
import unittest
import zipfile

import overrides
from formats import png
from gltf import Gltf
from mesh import Mesh


def rgb_png(width: int, height: int, rgb) -> bytes:
    return png(width, height, 2, bytes(rgb) * (width * height))


def glb(joint_name: str = "joint_000") -> bytes:
    gltf = Gltf()
    mesh = Mesh("m", [(0, 0, 0), (1, 0, 0), (0, 1, 0)], [(0, 0)] * 3, normals=[(0, 0, 1)] * 3,
                skins=[((0, 256),)] * 3)
    mesh.add_face(("moby", 1), (0, 1, 2))
    index = gltf.mesh(mesh, {("moby", 1): gltf.material("moby_0001", "../textures/moby_0001.png")})
    joint = gltf.node(joint_name)
    gltf.node("m", index, skin=gltf.skin([joint], [[1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1]]))
    return gltf.glb()


def write_pack(root: Path, name: str, files: dict, **fields) -> Path:
    manifest = {"format": 1, "game": "rac1", "version": "pal", "name": name, "author": "someone", **fields}
    root.mkdir(parents=True)
    (root / overrides.MANIFEST).write_text(json.dumps(manifest))
    for rel, data in files.items():
        (root / rel).parent.mkdir(parents=True, exist_ok=True)
        (root / rel).write_bytes(data)
    return root


def levels(root: Path, version: str = "pal") -> Path:
    """A levels folder the stack is for: one level of VERSION's, with nothing in it."""
    (root / "level_00").mkdir(parents=True)
    (root / "level_00/manifest.json").write_text(json.dumps(
        {"format": overrides.LEVELS_FORMAT, "game": "rac1", "version": version, "level": 0,
         "classes": {"moby": {}, "tie": {}, "shrub": {}}, "textures": []}))
    return root


def run(*argv) -> tuple[int, str]:
    with redirect_stdout(io.StringIO()) as out, redirect_stderr(io.StringIO()):
        code = overrides.main([str(a) for a in argv])
    return code, out.getvalue()


class PackTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.root = Path(self.tmp.name)
        self.into = self.root / "overrides"

    def tearDown(self):
        self.tmp.cleanup()

    def test_check_lists_used_ignored_and_problems(self):
        pack = write_pack(self.root / "hd", "HD textures", {
            "level_00/textures/moby_0001.png": rgb_png(8, 8, (1, 2, 3)),
            "level_00/mobys/moby_0.glb": glb(),
            "level_03/mobys/moby_7.glb": glb("Bone"),
            "level_00/textures/broken.png": b"not a png",
            "README.md": b"hello", "notes/other.bin": b"?"})
        manifest, used, ignored, problems = overrides.check(pack)
        self.assertEqual(manifest["name"], "HD textures")
        self.assertEqual(used, ["level_00/mobys/moby_0.glb", "level_00/textures/moby_0001.png"])
        self.assertEqual(ignored, ["notes/other.bin"])
        self.assertEqual(len(problems), 2)
        self.assertTrue(any("joint_NNN" in p for p in problems))
        self.assertEqual(overrides.describe(used),
                         {"level_00": {"models": ["moby_0"], "textures": ["moby_0001"]}})

    def test_models_of_levels_before_the_metal_packets_are_flagged(self):
        for made, flagged in ((1, True), (2, False), (overrides.LEVELS_FORMAT, False)):
            pack = write_pack(self.root / f"made-{made}", "Old", {"level_00/mobys/moby_0.glb": glb()},
                              levels_format=made)
            problems = overrides.check(pack)[3]
            self.assertEqual(any("levels of format" in p for p in problems), flagged, made)

    def test_wrong_game_or_nothing_inside_is_refused(self):
        other = write_pack(self.root / "uya", "UYA", {"level_00/textures/a.png": rgb_png(2, 2, (0, 0, 0))},
                           game="rac3")
        with self.assertRaisesRegex(overrides.PackError, "not a pack for Ratchet & Clank"):
            overrides.check(other)
        empty = write_pack(self.root / "empty", "Empty", {"README.txt": b"x"})
        with self.assertRaisesRegex(overrides.PackError, "replaces nothing"):
            overrides.check(empty)
        with self.assertRaisesRegex(overrides.PackError, "not an overrides pack"):
            overrides.check(self.root)

    def test_zip_with_a_top_folder_and_zip_slip(self):
        archive = self.root / "pack.zip"
        with zipfile.ZipFile(archive, "w") as z:
            z.writestr("Cool Pack/" + overrides.MANIFEST,
                       json.dumps({"format": 1, "game": "rac1", "version": "pal", "name": "Cool"}))
            z.writestr("Cool Pack/level_01/textures/tie_0002.png", rgb_png(4, 4, (5, 5, 5)))
        new, ignored, problems = overrides.add(archive, self.into)
        self.assertEqual((new, ignored, problems), ("cool", [], []))
        self.assertTrue((self.into / "packs/cool/level_01/textures/tie_0002.png").is_file())
        bad = self.root / "bad.zip"
        with zipfile.ZipFile(bad, "w") as z:
            z.writestr(overrides.MANIFEST, json.dumps({"format": 1, "game": "rac1", "version": "pal", "name": "x"}))
            z.writestr("../evil.png", b"x")
        with self.assertRaisesRegex(overrides.PackError, "outside its folder"):
            overrides.add(bad, self.into)

    def test_order_state_and_remove(self):
        for name in ("One", "Two", "Three"):
            overrides.add(write_pack(self.root / name, name, {"level_00/textures/x.png": rgb_png(2, 2, (1, 1, 1))}),
                          self.into)
        self.assertEqual(run("move", "three", "first", "--into", self.into)[0], 0)
        self.assertEqual(run("disable", "one", "--into", self.into)[0], 0)
        state = overrides.load_state(self.into)
        self.assertEqual(state["packs"], [{"id": "three", "enabled": True}, {"id": "one", "enabled": False},
                                          {"id": "two", "enabled": True}])
        self.assertEqual(run("remove", "two", "--into", self.into)[0], 0)
        self.assertFalse((self.into / "packs/two").exists())
        self.assertEqual([p["id"] for p in overrides.load_state(self.into)["packs"]], ["three", "one"])
        self.assertEqual(run("enable", "nope", "--into", self.into)[0], 1)
        # A second pack of the same name gets an id of its own.
        new, _, _ = overrides.add(write_pack(self.root / "again", "One", {"level_00/textures/x.png":
                                                                          rgb_png(2, 2, (1, 1, 1))}), self.into)
        self.assertEqual(new, "one-2")

    def test_stack_layers_packed_levels_and_packs_in_order(self):
        packed = self.root / "packed"
        (packed / "level_02").mkdir(parents=True)
        (packed / "level_02/gameplay_pal.wad").write_bytes(b"packed")
        (packed / "level_02/placements.json").write_text("{}")
        (packed / "patches.tsv").write_text("# header\n100\t4\tlevel_02/gameplay_pal.wad\n")
        (packed / "changes.txt").write_text("not for the port\n")
        red, blue = rgb_png(2, 2, (255, 0, 0)), rgb_png(4, 4, (0, 0, 255))
        overrides.add(write_pack(self.root / "a", "Red", {"level_00/textures/x.png": red,
                                                          "level_00/mobys/moby_0.glb": glb()}), self.into)
        overrides.add(write_pack(self.root / "b", "Blue", {"level_00/textures/x.png": blue}), self.into)
        overrides.add(write_pack(self.root / "c", "Off", {"level_00/textures/y.png": blue}), self.into)
        run("disable", "off", "--into", self.into)
        out = self.root / "mods"
        pal = levels(self.root / "levels")
        code, text = run("stack", "--into", self.into, "--out", out, "--packed", packed, "--levels", pal)
        self.assertEqual(code, 0, text)
        self.assertEqual((out / "level_00/textures/x.png").read_bytes(), blue)  # the later pack wins
        self.assertTrue((out / "level_00/mobys/moby_0.glb").is_file())
        self.assertFalse((out / "level_00/textures/y.png").exists())
        self.assertFalse((out / "changes.txt").exists())
        self.assertEqual((out / "level_02/gameplay_pal.wad").read_bytes(), b"packed")
        self.assertIn("100\t4\tlevel_02/gameplay_pal.wad", (out / "patches.tsv").read_text())
        summary = json.loads((out / "stack.json").read_text())
        self.assertEqual([layer["id"] for layer in summary["layers"]], ["packed", "red", "blue"])
        self.assertTrue(any("Blue wins over red" in note for note in summary["notes"]))
        # Stacked again: replaced whole; a folder it did not make is refused.
        run("disable", "blue", "--into", self.into)
        overrides.stack(self.into, out, packed)
        self.assertEqual((out / "level_00/textures/x.png").read_bytes(), red)
        stranger = self.root / "stranger"
        stranger.mkdir()
        with self.assertRaisesRegex(overrides.PackError, "not made by this tool"):
            overrides.stack(self.into, stranger)
        # A folder per disc: the stack made before them, straight in mods/, is made again in mods/pal.
        overrides.stack(self.into, out / "pal", packed, pal)
        self.assertEqual(sorted(p.name for p in out.iterdir()), ["pal"])
        self.assertEqual((out / "pal/level_00/textures/x.png").read_bytes(), red)

    def test_packed_levels_of_the_other_disc_are_left_out(self):
        packed = self.root / "packed"
        (packed / "level_02").mkdir(parents=True)
        (packed / "level_02/gameplay_ntsc.wad").write_bytes(b"packed")
        (packed / "patches.tsv").write_text("100\t4\tlevel_02/gameplay_ntsc.wad\n")
        (packed / overrides.PACKED).write_text(json.dumps({"game": "rac1", "version": "ntsc"}))
        summary = overrides.stack(None, self.root / "mods", packed, levels(self.root / "levels"))
        self.assertEqual(summary["files"], {})
        self.assertEqual(summary["skipped"]["packed"], ["level_02/gameplay_ntsc.wad", "patches.tsv"])
        self.assertTrue(any("pack them again" in n for n in summary["notes"]))
        summary = overrides.stack(None, self.root / "mods-us", packed, levels(self.root / "us", "ntsc"))
        self.assertEqual(summary["files"], {"level_02/gameplay_ntsc.wad": "packed"})

    def test_stack_with_nothing_still_writes_an_empty_list(self):
        out = self.root / "mods"
        overrides.stack(None, out, None)
        self.assertTrue((out / "patches.tsv").read_text().startswith("#"))

    def test_zip_command_round_trips(self):
        overrides.add(write_pack(self.root / "p", "Share me", {"level_00/textures/x.png": rgb_png(2, 2, (3, 3, 3))}),
                      self.into)
        archive = self.root / "share.zip"
        self.assertEqual(run("zip", "share-me", archive, "--into", self.into)[0], 0)
        manifest, used, _, _ = overrides.check(archive)
        self.assertEqual((manifest["name"], used), ("Share me", ["level_00/textures/x.png"]))


if __name__ == "__main__":
    unittest.main()
