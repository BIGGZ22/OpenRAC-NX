"""The port export (port.py) on synthetic levels: the decoders are replaced
by stand-ins returning small hand-made meshes, so no disc is needed."""

import json
from pathlib import Path
import subprocess
import sys
import tempfile
from types import SimpleNamespace
import unittest
from unittest import mock

import godot
import port
from formats import Texture
from level import Level
from mesh import Mesh
from moby_class import MobyClass
from mobys import moby_class_names
from port import PortLevelWriter, placement, sky_glb
from sky import Sky
from test_godot import attribute, read_glb


def texture(r: int, g: int, b: int) -> Texture:
    palette = bytearray(1024)
    palette[4:8] = bytes((r, g, b, 0x80))
    return Texture(2, 2, bytes((1, 1, 1, 1)), bytes(palette))


def square(name: str, key, size: float = 1.0, colours: bool = False) -> Mesh:
    h = size / 2
    mesh = Mesh(name, [(-h, -h, 0), (h, -h, 0), (-h, h, 0), (h, h, 0)], [(0, 1), (1, 1), (0, 0), (1, 0)],
                colours=[(1, 0, 0, 1), (0, 1, 0, 0.5), (0, 0, 1, 1), (1, 1, 1, 1)] if colours else None)
    mesh.add_face(key, (0, 1, 2))
    mesh.add_face(key, (1, 3, 2))
    return mesh


def translated(x: float, y: float, z: float, s: float = 1.0) -> list[float]:
    return [s, 0, 0, 0, 0, s, 0, 0, 0, 0, s, 0, x, y, z, 1]


def synthetic_level() -> Level:
    index = bytearray(0x100)
    for at in (0x08, 0x10, 0x14):  # terrain, sky and collision blocks
        index[at:at + 4] = (0x40).to_bytes(4, "little")
    level = Level(3, b"", {}, bytes(index), bytes(0x1000), b"", {"entry_point": 0x100000, "sections": []},
                  {("terrain", 0): texture(200, 0, 0), ("tie", 1): texture(0, 0, 200),
                   ("shrub", 2): texture(0, 200, 0), ("moby", 4): texture(9, 9, 9)})
    level.boundaries = [0x1000]
    return level


def synthetic_sky() -> Sky:
    shell = square("Sky_0", ("sky", 0), 40.0, colours=True)
    shell.add_face(None, (0, 2, 3))
    return Sky((10, 20, 30), [texture(50, 60, 70)], [shell, square("Sky_1", None, 30.0, colours=True)])


class SkyTests(unittest.TestCase):
    def test_shells_in_order_with_colours(self):
        doc, binary = read_glb(sky_glb(synthetic_sky(), lambda i: f"textures/sky_{i:04}.png"))
        self.assertEqual([n["name"] for n in doc["nodes"]], ["Sky_0", "Sky_1"])
        self.assertEqual(doc["scenes"][0]["nodes"], [0, 1])
        first = doc["meshes"][0]["primitives"]
        self.assertEqual(len(first), 2)  # textured and untextured faces
        colours = attribute(doc, binary, first[0]["attributes"]["COLOR_0"])
        self.assertEqual(colours[:3], [(1, 0, 0, 1), (0, 1, 0, 0.5), (0, 0, 1, 1)])
        textured, plain = (doc["materials"][p["material"]] for p in first)
        self.assertEqual(textured["alphaMode"], "BLEND")
        self.assertEqual(doc["images"][0]["uri"], "textures/sky_0000.png")
        self.assertNotIn("baseColorTexture", plain["pbrMetallicRoughness"])
        self.assertEqual(plain["alphaMode"], "BLEND")
        # The untextured material is shared by both shells.
        self.assertEqual(doc["meshes"][1]["primitives"][0]["material"], first[1]["material"])


class PlacementTests(unittest.TestCase):
    def test_fields_and_stored_w(self):
        p = {"index": 4, "class_id": 9, "matrix": translated(1, 2, 3), "stored_w": godot.STORED_W,
             "draw_distance": 50, "uid": 7}
        self.assertEqual(placement(p), {"index": 4, "class": 9, "matrix": translated(1, 2, 3),
                                        "draw_distance": 50, "uid": 7})
        self.assertEqual(placement({**p, "stored_w": 0.0})["matrix_w"], 0.0)


class WriterTests(unittest.TestCase):
    def export(self, out: Path) -> dict:
        ties = {5: square("tie_5", ("tie", 1))}
        shrubs = {6: square("shrub_6", ("shrub", 2), 0.5)}
        mobys = {7: MobyClass(0.25, square("moby_7", ("moby", 4))), 8: None}
        tie_placements = [
            {"index": 0, "class_id": 5, "matrix": translated(10, 20, 30), "stored_w": godot.STORED_W,
             "draw_distance": 100, "occlusion_index": -1, "directional_lights": 0, "uid": 11},
            {"index": 1, "class_id": 5, "matrix": translated(-10, 0, 0, 2), "stored_w": 0.0,
             "draw_distance": 100, "occlusion_index": 3, "directional_lights": 1, "uid": 12}]
        shrub_placements = [{"index": 0, "class_id": 6, "matrix": translated(0, 5, 0), "stored_w": godot.STORED_W,
                             "draw_distance": 20.0, "colour": [64, 80, 96], "directional_lights": 2}]
        moby_placements = [
            {"index": 0, "class_id": 7, "fields": {"spawn_id": 1, "group": -1, "pvar_index": -1, "colour": [1, 2, 3]},
             "rotation": (0.0, 0.0, 0.5), "scale": 2.0, "matrix": translated(3, 3, 3, 2)},
            {"index": 1, "class_id": 99, "fields": {"spawn_id": 2, "group": -1, "pvar_index": -1, "colour": [0, 0, 0]},
             "rotation": (0.0, 0.0, 0.0), "scale": 1.0, "matrix": translated(0, 0, 0)}]
        empty_collision = SimpleNamespace(cells=0, faces=0, triangles=[], surfaces=dict, hero_groups=0)
        with mock.patch.object(port, "sky", lambda block, *_: synthetic_sky()), \
                mock.patch.object(port, "PANORAMA", (16, 8)), \
                mock.patch.object(port, "terrain", lambda block, lod, lights=None, problems=None: [square("terrain_000", ("terrain", 0), 2.0)]), \
                mock.patch.object(port, "tie_classes", lambda level: ties), \
                mock.patch.object(port, "tie_instances", lambda gameplay, classes, *_: tie_placements), \
                mock.patch.object(port, "shrub_classes", lambda level: shrubs), \
                mock.patch.object(port, "shrub_instances", lambda gameplay, classes, *_: shrub_placements), \
                mock.patch.object(port, "moby_classes", lambda level: mobys), \
                mock.patch.object(port, "moby_instances", lambda gameplay, *_: moby_placements), \
                mock.patch.object(godot, "decode", lambda block: empty_collision):
            return PortLevelWriter(out, synthetic_level()).write(0)

    def test_files_manifest_and_placements(self):
        with tempfile.TemporaryDirectory() as temp:
            stats = self.export(Path(temp))
            level = Path(temp) / "level_03"
            files = sorted(p.relative_to(level).as_posix() for p in level.rglob("*") if p.is_file())
            self.assertEqual(files, ["bake-inputs/gameplay.bin", "manifest.json", "mobys/moby_7.glb", "placements.json", "shrubs/shrub_6.glb",
                                     "sky.glb", "sky.png", "terrain.glb", "textures/moby_0004.png",
                                     "textures/shrub_0002.png", "textures/sky_0000.png",
                                     "textures/terrain_0000.png", "textures/tie_0001.png", "ties/tie_5.glb"])
            manifest = json.loads((level / "manifest.json").read_text())
            self.assertEqual((manifest["format"], manifest["level"], manifest["game"]), (3, 3, "rac1"))
            self.assertEqual(manifest["terrain"], "terrain.glb")
            self.assertIsNone(manifest["collision"])
            self.assertEqual(manifest["sky"], {"mesh": "sky.glb", "panorama": "sky.png", "background": [10, 20, 30]})
            self.assertEqual(manifest["classes"]["tie"], {"5": {"mesh": "ties/tie_5.glb"}})
            names = moby_class_names()
            for class_id, mesh, scale in ((7, "mobys/moby_7.glb", 0.25), (8, None, 1.0)):
                entry = manifest["classes"]["moby"][str(class_id)]
                self.assertEqual((entry["mesh"], entry["scale"]), (mesh, scale))
                self.assertEqual(entry.get("name"), names.get(class_id))  # from moby_classes.tsv
            self.assertIsNone(manifest["classes"]["moby"]["99"]["mesh"])  # placed, but no class entry
            # Bounds in game axes (Z up): terrain, ties and shrubs, not mobys.
            self.assertEqual(manifest["bounds"], [[-11, -1, 0], [10.5, 20.5, 30]])
            self.assertEqual(stats["mobys"]["model_classes"], 1)
            self.assertEqual(stats["sky"]["shells"], 2)

            placements = json.loads((level / "placements.json").read_text())
            self.assertEqual(placements["format"], 3)
            ties = placements["ties"]
            self.assertEqual([t["class"] for t in ties], [5, 5])
            self.assertEqual(ties[0]["matrix"], translated(10, 20, 30))
            self.assertNotIn("matrix_w", ties[0])
            self.assertEqual(ties[1]["matrix_w"], 0.0)
            self.assertEqual(ties[1]["occlusion_index"], 3)
            self.assertEqual(placements["shrubs"][0]["colour"], [64, 80, 96])
            moby = placements["mobys"][0]
            self.assertEqual((moby["class"], moby["scale"], moby["colour"]), (7, 2.0, [1, 2, 3]))
            self.assertEqual(moby["rotation"], [0.0, 0.0, 0.5])
            self.assertEqual(moby["matrix"], translated(3, 3, 3, 2))

            # Meshes refer to the shared textures relative to themselves.
            doc, _ = read_glb((level / "ties" / "tie_5.glb").read_bytes())
            self.assertEqual(doc["images"][0]["uri"], "../textures/tie_0001.png")
            doc, _ = read_glb((level / "terrain.glb").read_bytes())
            self.assertEqual(doc["images"][0]["uri"], "textures/terrain_0000.png")

    def test_godot_export_unchanged(self):
        """The Godot writer still writes its scene and level.json; the port
        writer's overrides do not leak into it."""
        with tempfile.TemporaryDirectory() as temp:
            writer = godot.LevelWriter(Path(temp), synthetic_level())
            writer.dir.mkdir(parents=True)
            writer.write_objects("tie", {5: square("tie_5", ("tie", 1))},
                                 [{"index": 0, "class_id": 5, "matrix": translated(1, 2, 3),
                                   "stored_w": godot.STORED_W, "uid": 1}])
            writer.finish()
            self.assertTrue((writer.dir / "level_03.tscn").is_file())
            self.assertTrue((writer.dir / "level.json").is_file())
            self.assertIn("Tie_0000", (writer.dir / "level_03.tscn").read_text())


class CommandTests(unittest.TestCase):
    def test_port_command(self):
        extract = Path(__file__).with_name("extract.py")
        result = subprocess.run([sys.executable, str(extract), "port", "--help"], capture_output=True, text=True)
        self.assertEqual(result.returncode, 0)
        self.assertIn("--level", result.stdout)
        with tempfile.TemporaryDirectory() as temp:
            missing = Path(temp) / "missing.iso"
            result = subprocess.run([sys.executable, str(extract), "port", str(missing), str(Path(temp) / "out")],
                                    capture_output=True, text=True)
            self.assertEqual(result.returncode, 1)
            self.assertIn("extract:", result.stderr)


if __name__ == "__main__":
    unittest.main()
