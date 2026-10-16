"""The asset export and re-import (assets.py) on synthetic levels written with
the editor's own writers (godot.LevelWriter): no disc data."""

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
from godot import LevelWriter
from mesh import Mesh
from moby_anim import class_sequences, skeleton
from moby_class import MobyClass
from test_moby_anim import fixture

# Level 0's moby texture 3 and level 1's moby texture 7 are the same picture.
PICTURES = {0: {("moby", 3): (200, 10, 10), ("tie", 1): (10, 200, 10)},
            1: {("moby", 7): (200, 10, 10), ("tie", 1): (10, 10, 200)}}


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


def write_levels(root: Path) -> Path:
    """Two levels for the port, each with Ratchet (class 0, two joints) and tie class 3."""
    data = fixture()
    for number, moby_key in ((0, ("moby", 3)), (1, ("moby", 7))):
        writer = LevelWriter(root, Level(number))
        moby = MobyClass(0.5, moby_mesh(moby_key), 2, [], skeleton(data, class_sequences(data)))
        writer.moby_glb("mobys/moby_0.glb", moby)
        tie = Mesh("tie_3", [(0, 0, 0), (2, 0, 0), (0, 2, 0)], [(0, 0), (1, 0), (0, 1)])
        tie.add_face(("tie", 1), (0, 1, 2))
        writer.glb("ties/tie_3.glb", [tie], 1)
        (writer.dir / "textures").mkdir(parents=True, exist_ok=True)
        for name, tex in writer.textures.items():
            (writer.dir / "textures" / f"{name}.png").write_bytes(tex.png())
        manifest = {"format": assets.LEVELS_FORMAT, "game": "rac1", "version": "pal", "level": number,
                    "classes": {"moby": {"0": {"mesh": "mobys/moby_0.glb", "scale": 0.5, "name": "ratchet"},
                                         "1": {"mesh": None, "scale": 1.0}},
                                "tie": {"3": {"mesh": "ties/tie_3.glb"}}, "shrub": {}},
                    "textures": sorted(f"textures/{n}.png" for n in writer.textures)}
        (writer.dir / "manifest.json").write_text(json.dumps(manifest))
    return root / "levels"


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
                         assets.file_hash(self.levels / "level_00/mobys/moby_0.glb"))
        self.assertEqual(manifest["textures"], {"moby_0003": [4, 2]})
        self.assertEqual(manifest["skeleton"]["joints"], 2)
        textures = json.loads((self.export / "level_00/textures/textures.json").read_text())["textures"]
        self.assertEqual(sorted(textures), ["moby_0003", "tie_0001"])
        self.assertEqual(textures["moby_0003"]["size"], [4, 2])
        self.assertTrue((self.export / "level_00/ties/tie_3/tie_3.gltf").is_file())
        self.assertTrue((self.export / "README.txt").is_file())

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
