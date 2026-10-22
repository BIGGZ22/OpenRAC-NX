"""The GLB writer, scene text and project files; synthetic meshes only."""

import json
from pathlib import Path
import random
import struct
import tempfile
import unittest

from gltf import Gltf
from godot import ADDON_CFG, GAME_TO_GODOT, Raw, Scene, install_addon, number, transform, write_project
from mesh import Mesh


def read_glb(data: bytes) -> tuple[dict, bytes]:
    magic, version, length = struct.unpack_from("<3I", data)
    assert (magic, version, length) == (0x46546C67, 2, len(data))
    size, kind = struct.unpack_from("<I4s", data, 12)
    assert kind == b"JSON" and size % 4 == 0
    doc = json.loads(data[20:20 + size])
    binary_size, kind = struct.unpack_from("<I4s", data, 20 + size)
    assert kind == b"BIN\0" and binary_size == len(data) - 28 - size
    return doc, data[28 + size:]


def attribute(doc: dict, binary: bytes, index: int) -> list[tuple]:
    accessor = doc["accessors"][index]
    view = doc["bufferViews"][accessor["bufferView"]]
    width = int(accessor["type"][3])
    return list(struct.iter_unpack(f"<{width}f", binary[view["byteOffset"]:view["byteOffset"] + view["byteLength"]]))


def triangle() -> Mesh:
    mesh = Mesh("tie_7", [(0, 0, 0), (1, 0, 0), (0, 1, 0)], [(0, 0), (1, 0), (0, 1)])
    mesh.add_face(("tie", 3), (0, 1, 2))
    return mesh


class GltfTests(unittest.TestCase):
    def test_layout_attributes_and_shared_texture(self):
        gltf = Gltf()
        material = gltf.material("tie_0003", "../textures/tie_0003.png", cutout=True)
        gltf.node("tie_7", gltf.mesh(triangle(), {("tie", 3): material}))
        doc, binary = read_glb(gltf.glb())
        self.assertFalse([k for k, v in doc.items() if v == []])
        for view in doc["bufferViews"]:
            self.assertEqual(view["byteOffset"] % 4, 0)
        self.assertEqual(doc["images"], [{"uri": "../textures/tie_0003.png"}])
        self.assertEqual((doc["materials"][0]["alphaMode"], doc["materials"][0]["alphaCutoff"]), ("MASK", 0.5))
        primitive = doc["meshes"][0]["primitives"][0]
        self.assertEqual(attribute(doc, binary, primitive["attributes"]["NORMAL"]), [(0, 0, 1)] * 3)
        self.assertEqual(attribute(doc, binary, primitive["attributes"]["TEXCOORD_0"]), [(0, 0), (1, 0), (0, 1)])
        self.assertEqual(doc["accessors"][primitive["attributes"]["POSITION"]]["max"], [1, 1, 0])
        self.assertEqual(doc["scenes"][0]["nodes"], [0])

    def test_materials_are_shared(self):
        gltf = Gltf()
        self.assertEqual(gltf.material("a", "a.png"), gltf.material("a", "a.png"))
        self.assertNotEqual(gltf.material("a", "a.png"), gltf.material("a", "a.png", cutout=True))


class SceneTests(unittest.TestCase):
    def test_numbers_round_trip_as_float32(self):
        self.assertEqual([number(x) for x in (0.0, -0.0, 1.0, 0.1, 312.456787)], ["0", "0", "1", "0.1", "312.4568"])
        rng = random.Random(1)
        for _ in range(2000):
            x = struct.unpack("<f", struct.pack("<f", rng.uniform(-1e4, 1e4)))[0]
            self.assertEqual(struct.pack("<f", float(number(x))), struct.pack("<f", x))

    def test_transform_lists_basis_rows_then_origin(self):
        self.assertEqual(transform(GAME_TO_GODOT), "Transform3D(1, 0, 0, 0, 0, 1, 0, -1, 0, 0, 0, 0)")
        m = [1, 2, 3, 0, 4, 5, 6, 0, 7, 8, 9, 0, 10, 20, 30, 1]
        self.assertEqual(transform(m), "Transform3D(1, 4, 7, 2, 5, 8, 3, 6, 9, 10, 20, 30)")

    def test_scene_text(self):
        scene = Scene()
        glb = scene.resource("PackedScene", "res://a.glb", "a")
        colour = scene.subresource("Environment", "environment", background_mode=2,
                                   ambient_light_color=Raw("Color(1, 1, 1, 1)"))
        scene.node("Root", kind="Node3D")
        scene.node("World", ".", "WorldEnvironment", environment=colour)
        scene.node("A", ".", instance=glb, transform=transform(GAME_TO_GODOT),
                   visible=False, metadata__rc1_colour=[1, 2, 3], metadata__rc1_draw_distance=2.5)
        scene.editable.append("A")
        self.assertEqual(scene.text(), """[gd_scene format=3]

[ext_resource type="PackedScene" path="res://a.glb" id="a"]

[sub_resource type="Environment" id="environment"]
background_mode = 2
ambient_light_color = Color(1, 1, 1, 1)

[node name="Root" type="Node3D"]

[node name="World" type="WorldEnvironment" parent="."]
environment = SubResource("environment")

[node name="A" parent="." instance=ExtResource("a")]
transform = Transform3D(1, 0, 0, 0, 0, 1, 0, -1, 0, 0, 0, 0)
visible = false
metadata/rc1_colour = [1, 2, 3]
metadata/rc1_draw_distance = 2.5

[editable path="A"]
""")

    def test_project_files(self):
        with tempfile.TemporaryDirectory() as temp:
            project = Path(temp) / "project"
            write_project(project, [3, 5])
            text = (project / "project.godot").read_text()
            self.assertIn('run/main_scene="res://levels/level_03/level_03.tscn"', text)
            self.assertIn('"meshes/generate_lods": false', text)
            self.assertTrue((project / "rc1" / "level.gd").is_file())
            self.assertTrue((project / "rc1" / "check.gd").is_file())
            # The Collision dock, enabled.
            self.assertTrue((project / "addons" / "openrac_collision" / "plugin.cfg").is_file())
            self.assertIn(f'[editor_plugins]\n\nenabled=PackedStringArray("{ADDON_CFG}")', text)

    def test_addon_into_an_older_project(self):
        """An export made before the dock gets it on update; plugins of the modder's stay."""
        with tempfile.TemporaryDirectory() as temp:
            plain, other = Path(temp) / "plain", Path(temp) / "other"
            for project in (plain, other):
                project.mkdir()
            (plain / "project.godot").write_text("config_version=5\n\n[application]\n\nconfig/name=\"x\"\n")
            (other / "project.godot").write_text(
                'config_version=5\n\n[editor_plugins]\n\nenabled=PackedStringArray("res://addons/mine/plugin.cfg")\n')
            for project in (plain, other):
                install_addon(project)
                install_addon(project)          # twice: still once
                text = (project / "project.godot").read_text()
                self.assertEqual(text.count(ADDON_CFG), 1)
                self.assertEqual(text.count("[editor_plugins]"), 1)
                self.assertTrue((project / "addons" / "openrac_collision" / "plugin.gd").is_file())
            self.assertIn(f'enabled=PackedStringArray("{ADDON_CFG}", "res://addons/mine/plugin.cfg")',
                          (other / "project.godot").read_text())


if __name__ == "__main__":
    unittest.main()
