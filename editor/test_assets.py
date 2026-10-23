"""The asset export and re-import (assets.py) on synthetic levels written with
the editor's own writers (godot.LevelWriter): no disc data."""

import hashlib
import io
import json
import math
from pathlib import Path
import struct
import tempfile
import unittest
from contextlib import redirect_stdout

import assets
import gltf_read
import overrides
import pngio
from formats import Texture, png
from gltf import Gltf
from godot import LevelWriter
from mesh import Mesh
from moby_anim import class_sequences, skeleton
from moby_class import MobyClass
from test_moby_anim import fixture

# Level 0's moby texture 3 and level 1's moby texture 7 are the same picture.
# The other disc (CrossDiscTests) numbers Ratchet's picture 5 in level 0 and 6 in level 1; a
# version of it with a picture of its own (9) has it a shade darker.
PICTURES = {0: {("moby", 3): (200, 10, 10), ("moby", 5): (200, 10, 10), ("moby", 9): (199, 10, 10),
                ("tie", 1): (10, 200, 10), ("terrain", 2): (90, 60, 30), ("terrain", 4): (90, 60, 30)},
            1: {("moby", 7): (200, 10, 10), ("moby", 6): (200, 10, 10), ("moby", 9): (199, 10, 10),
                ("tie", 1): (10, 10, 200), ("terrain", 2): (90, 60, 30), ("terrain", 4): (90, 60, 30)}}
OTHER_DISC = ((0, ("moby", 5)), (1, ("moby", 6)))
OTHER_PICTURE = ((0, ("moby", 9)), (1, ("moby", 9)))


def texture(rgb) -> Texture:
    palette = bytearray(1024)
    palette[0:4] = bytes((*rgb, 0x80))
    return Texture(4, 2, bytes(8), bytes(palette))


class Level:
    def __init__(self, number):
        self.id = number
        self.overlay = {"entry_point": 0, "sections": []}

    def texture(self, group, index):
        return texture(PICTURES[self.id][(group, index)])


def moby_mesh(key) -> Mesh:
    mesh = Mesh("moby_0", [(0, 0, 0), (1, 0, 1), (0, 1, 1)], [(0, 0), (1, 0), (0, 1)], normals=[(0, 0, 1)] * 3,
                colours=[(1.0, 1.0, 1.0, 1.0)] * 3, light_slots=[-1, -2, -1],
                skins=[((0, 256),), ((1, 256),), ((0, 128), (1, 128))])
    mesh.add_face(key, (0, 1, 2))
    return mesh


def write_levels(root: Path, version: str = "pal", keys=((0, ("moby", 3)), (1, ("moby", 7))),
                 game: str = "rac1") -> Path:
    """Two levels for the port, each with Ratchet (class 0, two joints) and tie class 3. KEYS: each
    level's texture for Ratchet (another disc may number the same picture otherwise). GAME: the
    game the manifests say they are from."""
    data = fixture()
    for number, moby_key in keys:
        writer = LevelWriter(root, Level(number))
        moby = MobyClass(0.5, moby_mesh(moby_key), 2, [], skeleton(data, class_sequences(data)))
        writer.moby_glb("mobys/moby_0.glb", moby)
        tie = Mesh("tie_3", [(0, 0, 0), (2, 0, 0), (0, 2, 0)], [(0, 0), (1, 0), (0, 1)])
        tie.add_face(("tie", 1), (0, 1, 2))
        writer.glb("ties/tie_3.glb", [tie], 1)
        (writer.dir / "textures").mkdir(parents=True, exist_ok=True)
        for name, tex in writer.textures.items():
            (writer.dir / "textures" / f"{name}.png").write_bytes(tex.png())
        manifest = {"format": assets.LEVELS_FORMAT, "game": game, "version": version, "level": number,
                    "classes": {"moby": {"0": {"mesh": "mobys/moby_0.glb", "scale": 0.5, "name": "ratchet"},
                                         "1": {"mesh": None, "scale": 1.0}},
                                "tie": {"3": {"mesh": "ties/tie_3.glb"}}, "shrub": {}},
                    "textures": sorted(f"textures/{n}.png" for n in writer.textures)}
        (writer.dir / "manifest.json").write_text(json.dumps(manifest))
    return root / "levels"


# The terrain's two fragments: 4 x 4 squares side by side, each corner lit differently (0x80 = 1.0).
SQUARE = [(0, 0), (4, 0), (0, 4), (4, 4)]
CORNER_COLOURS = [(0.5, 0.5, 0.5, 1.0), (1.5, 1.5, 1.5, 1.0), (1.0, 1.0, 1.0, 1.0), (1.875, 1.875, 1.875, 1.0)]


def add_terrain(levels: Path, level: int, key=("terrain", 2), shift: float = 0.0) -> None:
    """Give a level of write_levels' a terrain.glb as the port's writer has it: one node per
    fragment, unindexed, lit by its vertex colours. SHIFT moves it (another terrain)."""
    writer = LevelWriter(levels.parent, Level(level))
    fragments = []
    for n in range(2):
        mesh = Mesh(f"Terrain_{n:03}", [(x + 4 * n, y + shift, 0.0) for x, y in SQUARE],
                    [(x / 4, y / 4) for x, y in SQUARE], colours=list(CORNER_COLOURS))
        mesh.add_face(key, (0, 1, 2))
        mesh.add_face(key, (1, 3, 2))
        fragments.append(mesh)
    writer.glb("terrain.glb", fragments, 0)
    for name, tex in writer.textures.items():
        (writer.dir / "textures" / f"{name}.png").write_bytes(tex.png())
    manifest = json.loads((writer.dir / "manifest.json").read_text())
    manifest["terrain"] = "terrain.glb"
    manifest["textures"] = sorted(set(manifest["textures"]) | {f"textures/{n}.png" for n in writer.textures})
    (writer.dir / "manifest.json").write_text(json.dumps(manifest))


def quiet(function, *args, **kwargs):
    with redirect_stdout(io.StringIO()) as out:
        result = function(*args, **kwargs)
    return result, out.getvalue()


def rgb_png(width: int, height: int, rgb) -> bytes:
    return png(width, height, 2, bytes(rgb) * (width * height))


def close(a, b, places=4) -> bool:
    return all(abs(x - y) < 10 ** -places for x, y in zip(a, b))


class AssetTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.root = Path(self.tmp.name)
        self.levels = write_levels(self.root)
        self.export = self.root / "export"
        quiet(assets.export, self.levels, self.export, [0], "all")

    def tearDown(self):
        self.tmp.cleanup()

    def model(self) -> Path:
        return self.export / "level_00/mobys/moby_0_ratchet/moby_0.gltf"

    def import_(self):
        report = assets.Report()
        pack, out = quiet(assets.import_edits, self.export, self.levels, report)
        return pack, report, out

    def test_export_layout_and_manifests(self):
        summary = json.loads((self.export / "export.json").read_text())
        self.assertEqual((summary["game"], summary["version"], summary["levels"]), ("rac1", "pal", [0]))
        self.assertEqual((summary["models"], summary["textures"]), (2, 2))
        manifest = json.loads((self.model().parent / "asset.json").read_text())
        self.assertEqual((manifest["kind"], manifest["class"], manifest["name"], manifest["scale"]),
                         ("moby", 0, "ratchet", 0.5))
        self.assertEqual(manifest["source_sha256"],
                         hashlib.sha256((self.levels / "level_00/mobys/moby_0.glb").read_bytes()).hexdigest())
        self.assertEqual(manifest["textures"], {"moby_0003": [4, 2]})
        self.assertEqual(manifest["skeleton"]["joints"], 2)
        textures = json.loads((self.export / "level_00/textures/textures.json").read_text())["textures"]
        self.assertEqual(sorted(textures), ["moby_0003", "tie_0001"])
        self.assertEqual(textures["moby_0003"]["size"], [4, 2])
        self.assertTrue((self.export / "level_00/ties/tie_3/tie_3.gltf").is_file())
        self.assertTrue((self.export / "README.txt").is_file())
        self.assertNotIn("PAL", (self.export / "README.txt").read_text())

    def test_either_discs_levels_export_and_import_alike(self):
        levels = write_levels(self.root / "us", "ntsc")
        export = self.root / "us-export"
        quiet(assets.export, levels, export, [0], "all")
        for path in (export / "export.json", export / "level_00/textures/textures.json",
                     export / "level_00/mobys/moby_0_ratchet/asset.json"):
            self.assertEqual(json.loads(path.read_text())["version"], "ntsc", path)
        self.assertNotIn("NTSC", (export / "README.txt").read_text())
        big = rgb_png(16, 8, (255, 255, 0))
        (export / "level_00/textures/moby_0003.png").write_bytes(big)
        report = assets.Report()
        pack, _ = quiet(assets.import_edits, export, levels, report)
        self.assertEqual((pack.version, report.refused), ("ntsc", 0))
        self.assertEqual(pack.files["level_01/textures/moby_0007.png"], big)
        # An export is read back with the levels it came from, not another disc's.
        with self.assertRaisesRegex(assets.AssetError, "another disc"):
            quiet(assets.import_edits, export, self.levels, assets.Report())

    def test_a_later_game_s_levels_export_and_import_as_its_own(self):
        levels = write_levels(self.root / "uya", "ntsc", game="rac3")
        export = self.root / "uya-export"
        quiet(assets.export, levels, export, [0], "all")
        for path in (export / "export.json", export / "level_00/textures/textures.json",
                     export / "level_00/mobys/moby_0_ratchet/asset.json"):
            self.assertEqual(json.loads(path.read_text())["game"], "rac3", path)
        big = rgb_png(16, 8, (255, 255, 0))
        (export / "level_00/textures/moby_0003.png").write_bytes(big)
        pack, _ = quiet(assets.import_edits, export, levels, assets.Report())
        self.assertEqual((pack.game, pack.version), ("rac3", "ntsc"))
        manifest = assets.write_pack(pack, self.root / "uya-pack", "UYA", "me", "", None, levels)
        self.assertEqual((manifest["game"], manifest["version"]), ("rac3", "ntsc"))
        self.assertEqual(overrides.check(self.root / "uya-pack")[1], ["level_00/textures/moby_0003.png",
                                                                     "level_01/textures/moby_0007.png"])
        # Another game's levels do not take it.
        with self.assertRaisesRegex(assets.AssetError, "Up Your Arsenal's levels"):
            quiet(assets.import_edits, export, write_levels(self.root / "rc", "ntsc"), assets.Report())

    def test_export_turns_into_gltf_axes_at_the_class_scale(self):
        document = gltf_read.read(self.model())
        self.assertEqual(document.list("images")[0]["uri"], "../../textures/moby_0003.png")
        self.assertTrue(document.image_path(document.list("images")[0]).is_file())
        mesh = document.list("meshes")[0]["primitives"][0]
        positions = document.accessor(mesh["attributes"]["POSITION"])
        # Game (1, 0, 1) at scale 0.5 is glTF (0.5, 0.5, 0).
        self.assertTrue(any(close(p, (0.5, 0.5, 0.0)) for p in positions))
        # The skinned mesh in its bind pose: joint world times inverse bind is the identity.
        skin = document.list("skins")[0]
        for slot, joint in enumerate(skin["joints"]):
            world = document.node_world(joint)
            ibm = list(document.accessor(skin["inverseBindMatrices"])[slot])
            self.assertTrue(close(gltf_read.multiply(world, ibm), gltf_read.IDENTITY), slot)
        # The animations are kept, root keys turned with their joint.
        self.assertEqual(len(document.list("animations")), 2)

    def test_unchanged_export_imports_nothing(self):
        pack, report, _ = self.import_()
        self.assertEqual(pack.files, {})
        self.assertEqual(report.refused, 0)

    def test_upscaled_texture_goes_to_every_level_with_that_picture(self):
        big = rgb_png(16, 8, (255, 255, 0))
        (self.export / "level_00/textures/moby_0003.png").write_bytes(big)
        pack, report, out = self.import_()
        self.assertEqual(report.refused, 0, out)
        self.assertEqual(pack.files["level_00/textures/moby_0003.png"], big)
        self.assertEqual(pack.files["level_01/textures/moby_0007.png"], big)
        self.assertNotIn("level_01/textures/tie_0001.png", pack.files)

    def test_texture_of_another_shape_is_refused(self):
        (self.export / "level_00/textures/tie_0001.png").write_bytes(rgb_png(8, 8, (1, 2, 3)))
        pack, report, out = self.import_()
        self.assertEqual(report.refused, 1)
        self.assertIn("keep that shape, for example 8 x 4", out)
        self.assertEqual(pack.files, {})

    def edit_model(self, change):
        """Load the exported glTF, let CHANGE edit its JSON and buffer, and save it."""
        path = self.model()
        doc = json.loads(path.read_text())
        binary = bytearray((path.parent / "moby_0.bin").read_bytes())
        change(doc, binary)
        path.write_text(json.dumps(doc))
        (path.parent / "moby_0.bin").write_bytes(bytes(binary))

    def test_moved_vertex_comes_back_in_game_axes_for_each_level(self):
        def lift(doc, binary):
            accessor = doc["accessors"][doc["meshes"][0]["primitives"][0]["attributes"]["POSITION"]]
            view = doc["bufferViews"][accessor["bufferView"]]
            for i in range(accessor["count"]):
                at = view["byteOffset"] + i * 12
                x, y, z = struct.unpack_from("<3f", binary, at)
                if close((x, y, z), (0.5, 0.5, 0.0)):
                    struct.pack_into("<3f", binary, at, x, y + 0.25, z)  # glTF up is game z
            accessor.pop("min"), accessor.pop("max")
        self.edit_model(lift)
        pack, report, out = self.import_()
        self.assertEqual(report.refused, 0, out)
        self.assertEqual(sorted(p for p in pack.files if p.endswith(".glb")),
                         ["level_00/mobys/moby_0.glb", "level_01/mobys/moby_0.glb"])
        for level, name in ((0, "moby_0003"), (1, "moby_0007")):
            path = self.root / f"out_{level}.glb"
            path.write_bytes(pack.files[f"level_{level:02}/mobys/moby_0.glb"])
            document = gltf_read.read(path)
            primitive = document.list("meshes")[0]["primitives"][0]
            positions = document.accessor(primitive["attributes"]["POSITION"])
            # Model units again: (1, 0, 1) lifted by 0.25 / 0.5 in z.
            self.assertTrue(any(close(p, (1.0, 0.0, 1.5)) for p in positions), positions)
            self.assertTrue(any(close(p, (0.0, 1.0, 1.0)) for p in positions))
            names = [document.list("nodes")[j]["name"] for j in document.list("skins")[0]["joints"]]
            self.assertEqual(names, ["joint_000", "joint_001"])
            # Each level's own name for the same picture.
            self.assertEqual(document.list("images")[0]["uri"], f"../textures/{name}.png")
            lights = [r[0] for r in document.accessor(primitive["attributes"]["_LIGHT_SLOT"])]
            self.assertEqual(sorted(lights), [-2, -1, -1])
            weights = document.accessor(primitive["attributes"]["WEIGHTS_0"])
            self.assertTrue(all(abs(sum(w) - 1) < 1e-6 for w in weights))

    def test_unknown_bone_is_refused(self):
        def rename(doc, binary):
            for node in doc["nodes"]:
                if node.get("name", "").startswith("joint_001"):
                    node["name"] = "Bone.004" + node["name"][9:]
        self.edit_model(rename)
        pack, report, out = self.import_()
        self.assertEqual(report.refused, 1)
        self.assertIn("Bone.004", out)
        self.assertIn("joint_000 to joint_001", out)

    def test_missing_uvs_are_refused(self):
        self.edit_model(lambda doc, binary: doc["meshes"][0]["primitives"][0]["attributes"].pop("TEXCOORD_0"))
        _, report, out = self.import_()
        self.assertEqual(report.refused, 1)
        self.assertIn("no UV map", out)

    def test_new_picture_travels_with_the_model(self):
        new = rgb_png(8, 8, (0, 0, 255))
        (self.model().parent / "skin.png").write_bytes(new)

        def repaint(doc, binary):
            doc["images"][0]["uri"] = "skin.png"
        self.edit_model(repaint)
        pack, report, out = self.import_()
        self.assertEqual(report.refused, 0, out)
        own = [p for p in pack.files if "/textures/moby_ovr_" in p]
        self.assertEqual(len(own), 2)  # beside the model in both levels
        self.assertTrue(all(pack.files[p] == new for p in own))

    def test_levels_list_keeps_a_model_to_its_level(self):
        manifest_path = self.model().parent / "asset.json"
        manifest = json.loads(manifest_path.read_text())
        manifest["levels"] = [0]
        manifest_path.write_text(json.dumps(manifest))
        self.edit_model(lambda doc, binary: None)
        (self.model().parent / "moby_0.bin").write_bytes((self.model().parent / "moby_0.bin").read_bytes())
        # Touch the glTF text so it differs from the export.
        self.model().write_text(self.model().read_text() + "\n")
        pack, report, out = self.import_()
        self.assertEqual([p for p in pack.files if p.endswith(".glb")], ["level_00/mobys/moby_0.glb"], out)

    def test_refuses_levels_extracted_again(self):
        self.model().write_text(self.model().read_text() + "\n")
        glb = self.levels / "level_00/mobys/moby_0.glb"
        glb.write_bytes(glb.read_bytes() + b"\0\0\0\0")
        _, report, out = self.import_()
        self.assertEqual(report.refused, 1)
        self.assertIn("export again", out)

    def test_cli_writes_a_pack(self):
        (self.export / "level_00/textures/moby_0003.png").write_bytes(rgb_png(8, 4, (9, 9, 9)))
        out = self.root / "packs"
        code, text = quiet(assets.main, ["import", str(self.export), "--levels", str(self.levels),
                                         "--into", str(out), "--name", "Yellow Ratchet", "--author", "me"])
        self.assertEqual(code, 0, text)
        state = overrides.load_state(out)
        self.assertEqual(state["packs"], [{"id": "yellow-ratchet", "enabled": True}])
        manifest = json.loads((out / "packs/yellow-ratchet" / overrides.MANIFEST).read_text())
        self.assertEqual(manifest["replaces"]["level_01"], {"textures": ["moby_0007"]})
        # The same export imported again replaces its pack.
        (self.export / "level_00/textures/moby_0003.png").write_bytes(rgb_png(16, 8, (9, 9, 9)))
        code, _ = quiet(assets.main, ["import", str(self.export), "--levels", str(self.levels), "--into", str(out)])
        self.assertEqual(code, 0)
        self.assertEqual(len(overrides.load_state(out)["packs"]), 1)
        self.assertEqual(pngio.size((out / "packs/yellow-ratchet/level_00/textures/moby_0003.png").read_bytes()),
                         (16, 8))

    def test_unique_export_folder(self):
        code, text = quiet(assets.main, ["export", str(self.levels), str(self.export), "--level", "0", "--unique"])
        self.assertEqual(code, 0)
        self.assertTrue(text.strip().endswith(str(self.export.resolve()) + "-2"), text)
        self.assertTrue((self.root / "export-2/export.json").is_file())

    def test_export_refuses_the_repository_and_existing_folders(self):
        with self.assertRaisesRegex(assets.AssetError, "already exists"):
            assets.export(self.levels, self.export, [0], "mobys")
        with self.assertRaisesRegex(assets.AssetError, "only under assets/ or build/"):
            assets.export(self.levels, assets.ROOT / "editor" / "nope", [0], "mobys")


class CrossDiscTests(unittest.TestCase):
    """A pack made from one disc's levels, stacked for the other's, where the same pictures have other
    names."""

    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.root = Path(self.tmp.name)
        self.pal = write_levels(self.root / "pal")
        self.us = write_levels(self.root / "us", "ntsc", OTHER_DISC)
        self.export = self.root / "export"
        quiet(assets.export, self.pal, self.export, [0], "all")
        self.into = self.root / "overrides"

    def tearDown(self):
        self.tmp.cleanup()

    def pack(self, edit) -> Path:
        edit(self.export)
        code, text = quiet(assets.main, ["import", str(self.export), "--levels", str(self.pal), "--into",
                                         str(self.into), "--name", "Edited"])
        self.assertEqual(code, 0, text)
        return self.into / "packs/edited"

    def stack(self, levels: Path, name: str = "mods") -> tuple[dict, Path]:
        out = self.root / name
        return overrides.stack(self.into, out, None, levels), out

    def test_texture_and_model_go_where_the_other_disc_has_them(self):
        big = rgb_png(16, 8, (255, 255, 0))

        def edit(export):
            (export / "level_00/textures/moby_0003.png").write_bytes(big)
            model = export / "level_00/mobys/moby_0_ratchet/moby_0.gltf"
            model.write_text(model.read_text() + "\n")
        root = self.pack(edit)
        manifest = json.loads((root / overrides.MANIFEST).read_text())
        self.assertEqual(manifest["version"], "pal")
        picture = hashlib.sha256((self.pal / "level_00/textures/moby_0003.png").read_bytes()).hexdigest()
        self.assertEqual(manifest["originals"]["level_01/textures/moby_0007.png"],
                         {"sha256": picture, "size": [4, 2], "used_by": [["moby", 0, 0, 1]]})
        model = manifest["originals"]["level_00/mobys/moby_0.glb"]
        self.assertEqual((model["kind"], model["class"], model["textures"], model["class_textures"]),
                         ("moby", 0, {"moby_0003": picture}, ["moby_0003"]))

        summary, out = self.stack(self.us)
        self.assertEqual(summary["version"], "ntsc")
        self.assertEqual(sorted(summary["files"]), ["level_00/mobys/moby_0.glb", "level_00/textures/moby_0005.png",
                                                    "level_01/mobys/moby_0.glb", "level_01/textures/moby_0006.png"])
        self.assertEqual((out / "level_01/textures/moby_0006.png").read_bytes(), big)
        for level, name in ((0, "moby_0005"), (1, "moby_0006")):
            document = gltf_read.read(out / f"level_{level:02}/mobys/moby_0.glb")
            self.assertEqual(document.list("images")[0]["uri"], f"../textures/{name}.png")
            self.assertEqual(document.list("materials")[0]["name"], name)
        self.assertEqual(summary["skipped"], {}, summary["notes"])
        # On the disc it was made from it goes in as it is.
        summary, out = self.stack(self.pal, "mods-pal")
        self.assertEqual(sorted(summary["files"]), ["level_00/mobys/moby_0.glb", "level_00/textures/moby_0003.png",
                                                    "level_01/mobys/moby_0.glb", "level_01/textures/moby_0007.png"])

    def test_a_picture_the_other_disc_has_its_own_version_of(self):
        big = rgb_png(16, 8, (255, 255, 0))

        def edit(export):
            (export / "level_00/textures/moby_0003.png").write_bytes(big)
            model = export / "level_00/mobys/moby_0_ratchet/moby_0.gltf"
            model.write_text(model.read_text() + "\n")
        self.pack(edit)
        darker = write_levels(self.root / "darker", "ntsc", OTHER_PICTURE)
        summary, out = self.stack(darker)
        self.assertEqual(summary["skipped"], {}, summary["notes"])
        # Where Ratchet's model uses its own version of the picture, of the same size.
        self.assertEqual((out / "level_00/textures/moby_0009.png").read_bytes(), big)
        self.assertEqual((out / "level_01/textures/moby_0009.png").read_bytes(), big)
        document = gltf_read.read(out / "level_01/mobys/moby_0.glb")
        self.assertEqual(document.list("images")[0]["uri"], "../textures/moby_0009.png")

    def test_a_class_that_is_another_model_or_missing_is_left_out(self):
        self.pack(lambda export: (export / "level_00/mobys/moby_0_ratchet/moby_0.gltf").write_text(
            (export / "level_00/mobys/moby_0_ratchet/moby_0.gltf").read_text() + "\n"))
        # Here level 1's class 0 has no skeleton, and level 0 has no class 0.
        (self.us / "level_01/mobys/moby_0.glb").write_bytes(
            (self.us / "level_01/ties/tie_3.glb").read_bytes())
        manifest = json.loads((self.us / "level_00/manifest.json").read_text())
        del manifest["classes"]["moby"]["0"]
        (self.us / "level_00/manifest.json").write_text(json.dumps(manifest))
        summary, out = self.stack(self.us)
        self.assertEqual(summary["files"], {})
        self.assertEqual(summary["skipped"]["edited"], [
            "level_00/mobys/moby_0.glb (the level has no such class here)",
            "level_01/mobys/moby_0.glb (the class is another model here)"])
        self.assertTrue(any("other disc" in note for note in summary["notes"]))

    def test_a_pack_from_before_originals_matches_by_name_and_picture(self):
        green, yellow = rgb_png(8, 4, (0, 255, 0)), rgb_png(8, 4, (255, 255, 0))
        files = {"level_00/textures/tie_0001.png": green, "level_00/textures/moby_0003.png": yellow,
                 "level_01/textures/moby_0007.png": yellow, "level_02/placements.json": b"{}"}
        source = self.root / "old"
        for rel, data in files.items():
            (source / rel).parent.mkdir(parents=True, exist_ok=True)
            (source / rel).write_bytes(data)
        (source / overrides.MANIFEST).write_text(json.dumps(
            {"format": 1, "game": "rac1", "version": "pal", "name": "Old", "levels_format": 2}))
        overrides.add(source, self.into)
        summary, out = quiet(self.stack, self.us)[0]
        # tie_0001 is the same picture under the same name here; level 0's moby_0003 is not here.
        self.assertEqual(sorted(summary["files"]), ["level_00/textures/tie_0001.png"])
        self.assertEqual(summary["skipped"]["old"], [
            "level_00/textures/moby_0003.png (not the same picture here)",
            "level_01/textures/moby_0007.png (not the same picture here)",
            "level_02/placements.json (it belongs to the other disc's level data)"])
        summary, _ = self.stack(self.pal, "mods-pal")
        self.assertEqual(len(summary["files"]), 4)


def to_gltf(p) -> tuple:
    """A game point (Z up) in glTF's axes (Y up)."""
    return (p[0], p[2], -p[1])


def edited_terrain(path: Path, grid: int = 3, texture_name: str = "terrain_0002", colours: bool = False,
                   uvs: bool = True) -> None:
    """A terrain as Blender exports it after subdividing the first fragment into a GRID x GRID mesh
    (no vertex colours: Blender drops them), the second fragment kept with its colours as
    exported (half) when COLOURS."""
    gltf = Gltf()
    material = {("t",): gltf.material(texture_name, f"../textures/{texture_name}.png")}
    first = Mesh("Terrain_000", normals=[])
    for j in range(grid):
        for i in range(grid):
            x, y = 4 * i / (grid - 1), 4 * j / (grid - 1)
            first.positions.append(to_gltf((x, y, 0.0)))
            first.uvs.append((x / 4, y / 4))
            first.normals.append((0.0, 1.0, 0.0))
    for j in range(grid - 1):
        for i in range(grid - 1):
            a = j * grid + i
            first.add_face(("t",), (a, a + 1, a + grid))
            first.add_face(("t",), (a + 1, a + grid + 1, a + grid))
    second = Mesh("Terrain_001", [to_gltf((x + 4, y, 0.0)) for x, y in SQUARE], [(x / 4, y / 4) for x, y in SQUARE],
                  normals=[(0.0, 1.0, 0.0)] * 4,
                  colours=[(r / 2, g / 2, b / 2, a) for r, g, b, a in CORNER_COLOURS] if colours else None)
    second.add_face(("t",), (0, 1, 2))
    second.add_face(("t",), (1, 3, 2))
    for mesh in (first, second):
        index = gltf.mesh(mesh, material)
        if not uvs:
            for primitive in gltf.doc["meshes"][index]["primitives"]:
                primitive["attributes"].pop("TEXCOORD_0")
        gltf.node(mesh.name, index)
    path.write_bytes(gltf.glb())


def terrain_rows(data: bytes, root: Path) -> dict:
    """A port terrain.glb's nodes: name -> (positions, colours, image URIs)."""
    path = root / "read.glb"
    path.write_bytes(data)
    document = gltf_read.read(path)
    out = {}
    for node in document.list("nodes"):
        positions, colours, uris = [], [], []
        for primitive in document.list("meshes")[node["mesh"]]["primitives"]:
            attributes = primitive["attributes"]
            assert "_LIGHT_SLOT" not in attributes
            positions += document.accessor(attributes["POSITION"])
            colours += document.accessor(attributes["COLOR_0"])
            material = document.list("materials")[primitive["material"]]
            image = document.list("textures")[material["pbrMetallicRoughness"]["baseColorTexture"]["index"]]["source"]
            uris.append(document.list("images")[image]["uri"])
        out[node["name"]] = (positions, colours, uris)
    return out


class TerrainTests(unittest.TestCase):
    """The terrain as a family: exported for Blender, a denser one read back into a pack."""

    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.root = Path(self.tmp.name)
        self.levels = write_levels(self.root)
        add_terrain(self.levels, 0)
        self.export = self.root / "export"
        quiet(assets.export, self.levels, self.export, [0], "terrain")
        self.folder = self.export / "level_00/terrain"

    def tearDown(self):
        self.tmp.cleanup()

    def import_(self):
        report = assets.Report()
        pack, out = quiet(assets.import_edits, self.export, self.levels, report)
        return pack, report, out

    def test_export_layout_axes_and_half_colours(self):
        summary = json.loads((self.export / "export.json").read_text())
        self.assertEqual((summary["what"], summary["models"], summary["textures"]), ("terrain", 1, 1))
        manifest = json.loads((self.folder / "asset.json").read_text())
        self.assertEqual((manifest["kind"], manifest["source"], manifest["fragments"], manifest["colour_scale"]),
                         ("terrain", "terrain.glb", 2, 2.0))
        self.assertEqual(manifest["textures"], {"terrain_0002": [4, 2]})
        self.assertEqual(sorted(json.loads((self.export / "level_00/textures/textures.json").read_text())["textures"]),
                         ["terrain_0002"])
        document = gltf_read.read(self.folder / "terrain.gltf")
        self.assertEqual([n["name"] for n in document.list("nodes")], ["Terrain_000", "Terrain_001"])
        self.assertEqual(document.list("images")[0]["uri"], "../textures/terrain_0002.png")
        primitive = document.list("meshes")[0]["primitives"][0]
        positions = document.accessor(primitive["attributes"]["POSITION"])
        colours = document.accessor(primitive["attributes"]["COLOR_0"])
        corner = next(i for i, p in enumerate(positions) if close(p, to_gltf((4, 4, 0))))
        self.assertTrue(close(colours[corner], (0.9375, 0.9375, 0.9375, 1.0)), colours[corner])
        self.assertTrue(all(max(c) <= 1.0 for c in colours))

    def test_everything_holds_the_terrain_and_a_class_choice_does_not(self):
        quiet(assets.export, self.levels, self.root / "all", [0], "all")
        self.assertTrue((self.root / "all/level_00/terrain/terrain.gltf").is_file())
        self.assertEqual(json.loads((self.root / "all/export.json").read_text())["models"], 3)
        quiet(assets.export, self.levels, self.root / "one", [0], "all", [0])
        self.assertFalse((self.root / "one/level_00/terrain").exists())
        with self.assertRaisesRegex(assets.AssetError, "no terrain"):
            quiet(assets.export, self.levels, self.root / "none", [1], "terrain")

    def test_unchanged_terrain_imports_nothing(self):
        pack, report, _ = self.import_()
        self.assertEqual((pack.files, report.refused), ({}, 0))

    def test_denser_terrain_takes_the_original_s_colours(self):
        edited_terrain(self.folder / "terrain_edited.glb", colours=True)
        pack, report, out = self.import_()
        self.assertEqual(report.refused, 0, out)
        self.assertEqual(sorted(pack.files), ["level_00/terrain.glb"])
        self.assertIn("9 vertices took their colour", out)
        rows = terrain_rows(pack.files["level_00/terrain.glb"], self.root)
        positions, colours, uris = rows["Terrain_000"]
        self.assertEqual(len(positions), 9)       # three by three: four more than the original's four corners
        self.assertEqual(uris, ["textures/terrain_0002.png"])

        def colour_at(x, y):
            return colours[next(i for i, p in enumerate(positions) if close(p, (x, y, 0)))][0]
        # Corners are the original's; a new vertex blends the face it lies on: the middle of an edge,
        # and the centre, on the diagonal between (4, 0) and (0, 4).
        self.assertAlmostEqual(colour_at(0, 0), 0.5)
        self.assertAlmostEqual(colour_at(4, 4), 1.875)
        self.assertAlmostEqual(colour_at(2, 0), 1.0)
        self.assertAlmostEqual(colour_at(2, 2), 1.25)
        self.assertAlmostEqual(colour_at(0, 2), 0.75)
        # The second fragment's colours, exported at half, are the game's again.
        positions, colours, _ = rows["Terrain_001"]
        corner = next(i for i, p in enumerate(positions) if close(p, (8, 4, 0)))
        self.assertTrue(close(colours[corner], (1.875, 1.875, 1.875, 1.0)), colours[corner])

    def test_a_far_vertex_takes_the_nearest_surface(self):
        surface = assets.Surface(assets.original_triangles(gltf_read.read(self.levels / "level_00/terrain.glb")))
        # Raised 30 units above the middle of the far edge of the second fragment.
        self.assertTrue(close(surface.colour((8.0, 2.0, 30.0)), (1.6875, 1.6875, 1.6875, 1.0)))
        self.assertTrue(close(surface.colour((0.0, 0.0, 0.0)), CORNER_COLOURS[0]))

    def test_missing_uvs_or_pictures_are_refused(self):
        edited_terrain(self.folder / "terrain_edited.glb", uvs=False)
        pack, report, out = self.import_()
        self.assertEqual((report.refused, pack.files), (1, {}))
        self.assertIn("no UV map", out)
        # A material without a picture (Blender's default for new faces).
        edited_terrain(self.folder / "terrain_edited.glb")
        doc, binary = overrides.glb_parts((self.folder / "terrain_edited.glb").read_bytes())
        doc["materials"][0]["pbrMetallicRoughness"].pop("baseColorTexture")
        (self.folder / "terrain_edited.glb").write_bytes(overrides.glb_join(doc, binary))
        pack, report, out = self.import_()
        self.assertEqual((report.refused, pack.files), (1, {}))
        self.assertIn("faces without a picture", out)

    def test_a_new_picture_travels_with_the_terrain(self):
        new = rgb_png(8, 4, (0, 0, 255))
        (self.export / "level_00/textures/grass.png").write_bytes(new)
        edited_terrain(self.folder / "terrain_edited.glb", texture_name="grass")
        pack, report, out = self.import_()
        self.assertEqual(report.refused, 0, out)
        own = [p for p in pack.files if "/textures/terrain_ovr_" in p]
        self.assertEqual(len(own), 1)
        rows = terrain_rows(pack.files["level_00/terrain.glb"], self.root)
        self.assertEqual(rows["Terrain_000"][2], [own[0].replace("level_00/", "")])

    def test_levels_extracted_again_are_refused(self):
        edited_terrain(self.folder / "terrain_edited.glb")
        terrain = self.levels / "level_00/terrain.glb"
        terrain.write_bytes(terrain.read_bytes() + b"\0\0\0\0")
        _, report, out = self.import_()
        self.assertEqual(report.refused, 1)
        self.assertIn("export again", out)

    def test_pack_records_the_terrain_and_goes_to_the_other_disc(self):
        edited_terrain(self.folder / "terrain_edited.glb")
        into = self.root / "overrides"
        code, text = quiet(assets.main, ["import", str(self.export), "--levels", str(self.levels), "--into",
                                         str(into), "--name", "Dense"])
        self.assertEqual(code, 0, text)
        root = into / "packs/dense"
        manifest = json.loads((root / overrides.MANIFEST).read_text())
        self.assertEqual(manifest["replaces"], {"level_00": {"models": ["terrain"]}})
        original = manifest["originals"]["level_00/terrain.glb"]
        self.assertEqual((original["kind"], original["class_textures"]), ("terrain", ["terrain_0002"]))
        self.assertEqual(overrides.check(root)[3], [])
        # The other disc names the same picture terrain_0004; its terrain is the same.
        us = write_levels(self.root / "us", "ntsc", OTHER_DISC)
        add_terrain(us, 0, ("terrain", 4))
        summary = overrides.stack(into, self.root / "mods", None, us)
        self.assertEqual(summary["skipped"], {}, summary["notes"])
        rows = terrain_rows((self.root / "mods/level_00/terrain.glb").read_bytes(), self.root)
        self.assertEqual(rows["Terrain_000"][2], ["textures/terrain_0004.png"])
        self.assertEqual(len(rows["Terrain_000"][0]), 9)
        # A disc whose terrain is another leaves it out.
        moved = write_levels(self.root / "moved", "ntsc", OTHER_DISC)
        add_terrain(moved, 0, ("terrain", 4), shift=1.0)
        summary = overrides.stack(into, self.root / "mods-moved", None, moved)
        self.assertEqual(summary["skipped"]["dense"], ["level_00/terrain.glb (the level's terrain is another here)"])


class SkinTests(unittest.TestCase):
    def test_quantise_sums_to_256(self):
        self.assertEqual(assets.quantise(((0, 1 / 3), (1, 1 / 3), (2, 1 / 3))), ((0, 86), (1, 85), (2, 85)))
        self.assertEqual(sum(w for _, w in assets.quantise(((4, 0.7), (2, 0.2), (9, 0.1)))), 256)

    def test_five_bones_keep_the_strongest_four(self):
        nodes = [{"name": f"joint_{i:03}"} for i in range(5)] + [{"name": "m", "mesh": 0, "skin": 0}]
        # One triangle whose three corners each lean on five bones.
        parts = [struct.pack("<9f", 0, 0, 0, 1, 0, 0, 0, 1, 0), struct.pack("<6f", 0, 0, 1, 0, 0, 1),
                 bytes((0, 1, 2, 3)) * 3, bytes((4, 0, 0, 0)) * 3,
                 struct.pack("<4f", 0.3, 0.25, 0.2, 0.15) * 3, struct.pack("<4f", 0.1, 0, 0, 0) * 3]
        buffer, views = b"", []
        for part in parts:
            views.append({"buffer": 0, "byteOffset": len(buffer), "byteLength": len(part)})
            buffer += part
        kinds = [(5126, "VEC3"), (5126, "VEC2"), (5121, "VEC4"), (5121, "VEC4"), (5126, "VEC4"), (5126, "VEC4")]
        doc = {"asset": {"version": "2.0"}, "nodes": nodes, "skins": [{"joints": [0, 1, 2, 3, 4]}],
               "meshes": [{"primitives": [{"attributes": {"POSITION": 0, "TEXCOORD_0": 1, "JOINTS_0": 2,
                                                          "JOINTS_1": 3, "WEIGHTS_0": 4, "WEIGHTS_1": 5}}]}],
               "bufferViews": views,
               "accessors": [{"bufferView": i, "componentType": c, "count": 3, "type": t}
                             for i, (c, t) in enumerate(kinds)]}
        document = gltf_read.Document(doc, [buffer], Path("five.gltf"))
        identity = tuple(gltf_read.IDENTITY)
        original = ([f"joint_{i:03}" for i in range(5)], [identity] * 5)
        report = assets.Report()
        meshes, out = quiet(assets.read_edited, document, original, 5, 1.0, report, "test")
        self.assertIsNotNone(meshes, out)
        skin = meshes[0][1][0].skin
        self.assertEqual([j for j, _ in skin], [0, 1, 2, 3])
        self.assertAlmostEqual(sum(w for _, w in skin), 1.0)
        self.assertAlmostEqual(skin[0][1], 0.3 / 0.9)
        self.assertEqual(report.warnings, 1)


class MatrixTests(unittest.TestCase):
    def test_invert_and_trs(self):
        m = gltf_read.trs((1, 2, 3), (0, 0, math.sqrt(0.5), math.sqrt(0.5)), (2, 2, 2))
        self.assertTrue(close(gltf_read.multiply(m, gltf_read.invert(m)), gltf_read.IDENTITY))
        self.assertTrue(close(gltf_read.point(m, (1, 0, 0)), (1, 4, 3)))

    def test_turn_matches_its_quaternion(self):
        q = gltf_read.trs(rotation=assets.Q_TO_GLTF)
        self.assertTrue(close(q, assets.TO_GLTF))


class PngTests(unittest.TestCase):
    def test_indexed_with_transparency_and_rgb(self):
        palette = bytearray(1024)
        palette[4:8] = bytes((10, 20, 30, 0x40))
        indexed = Texture(2, 1, bytes((0, 1)), bytes(palette)).png()
        width, height, pixels = pngio.read(indexed)
        self.assertEqual((width, height), (2, 1))
        self.assertEqual(pixels[4:8], bytes((10, 20, 30, 0x80)))
        self.assertEqual(pngio.read(rgb_png(3, 2, (1, 2, 3)))[2][:8], bytes((1, 2, 3, 255, 1, 2, 3, 255)))

    def test_filtered_rows(self):
        import zlib
        rows = b"\x01" + bytes((10, 10, 10, 255, 5, 5, 5, 0)) + b"\x02" + bytes((1, 1, 1, 0, 1, 1, 1, 0))
        data = (b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">2I5B", 2, 2, 8, 6, 0, 0, 0))
                + chunk(b"IDAT", zlib.compress(rows)) + chunk(b"IEND", b""))
        _, _, pixels = pngio.read(data)
        self.assertEqual(pixels, bytes((10, 10, 10, 255, 15, 15, 15, 255, 11, 11, 11, 255, 16, 16, 16, 255)))

    def test_refuses_interlaced(self):
        data = bytearray(rgb_png(2, 2, (0, 0, 0)))
        data[28] = 1  # interlace method in IHDR
        with self.assertRaisesRegex(pngio.PngError, "interlac"):
            pngio.read(bytes(data))


def chunk(kind: bytes, payload: bytes) -> bytes:
    import zlib
    return struct.pack(">I", len(payload)) + kind + payload + struct.pack(">I", zlib.crc32(kind + payload))


if __name__ == "__main__":
    unittest.main()
