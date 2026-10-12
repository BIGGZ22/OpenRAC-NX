"""pack.pack_level on a made-up gameplay file and scene."""

import math
import struct
import unittest

from godot import transform
from mobys import FIELDS, POINTER, RECORD, SIZE, USUAL, moby_instances
from pack import pack_level, placement, scene_mobys


def gameplay(records: list[dict]) -> bytes:
    start = 0x100
    out = bytearray(start + 0x10 + len(records) * SIZE)
    struct.pack_into("<I", out, POINTER, start)
    struct.pack_into("<i", out, start, len(records))
    for i, fields in enumerate(records):
        record = {"size": SIZE, **USUAL, "spawn_id": i, "group": -1, "pvar_index": -1, "class_id": 500,
                  "scale": 1.0, "x": 0.0, "y": 0.0, "z": 0.0, "rx": 0.0, "ry": 0.0, "rz": 0.0,
                  "r": 10, "g": 20, "b": 30, **fields}
        RECORD.pack_into(out, start + 0x10 + i * SIZE, *(record[f] for f in FIELDS))
    return bytes(out)


def scene(data: bytes, edit=None) -> str:
    """The moby nodes as the extractor writes them; EDIT changes the lines of one node."""
    lines = ['[ext_resource type="PackedScene" path="res://levels/level_00/mobys/moby_500.tscn" id="moby_500"]',
             '[ext_resource type="PackedScene" path="res://levels/level_00/mobys/moby_7.tscn" id="moby_7"]', ""]
    for p in moby_instances(data):
        node = [f'[node name="Moby_{p["index"]:04}" parent="Game/Mobys" instance=ExtResource("moby_{p["class_id"]}")]',
                "rotation_order = 0", f"transform = {transform(p['matrix'])}", f"metadata/rc1_index = {p['index']}"]
        node += [f"metadata/rc1_{k} = {v}" for k, v in p["fields"].items()]
        if edit:
            node = edit(p["index"], node)
        lines += node + [""]
    return "\n".join(lines)


class PackLevel(unittest.TestCase):
    def setUp(self):
        self.data = gameplay([{"x": 1.5, "y": -2.25, "z": 3.0, "rz": 0.7, "scale": 2.0},
                              {"x": 10.0, "rx": 0.3, "ry": -0.4, "rz": 1.1, "draw_distance": 128},
                              {"x": -5.0, "y": 0.1}])

    def test_untouched_scene_changes_nothing(self):
        out, changes, problems = pack_level(0, self.data, scene(self.data))
        self.assertEqual((out, changes, problems), (self.data, [], []))

    def test_godots_own_saving_changes_nothing(self):
        """Godot adds a type and a unique_id to the node, writes rc1_class on each instance and
        rounds floats its own way."""
        def as_godot(index, node):
            node[0] = node[0].replace(' parent=', ' type="Node3D" parent=').replace(" instance=", " unique_id=77 instance=")
            return node + ["metadata/rc1_class = 500"]
        out, changes, problems = pack_level(0, self.data, scene(self.data, as_godot))
        self.assertEqual((out, changes, problems), (self.data, [], []))

    def test_a_moved_moby_changes_only_its_record(self):
        def move(index, node):
            if index == 1:
                node[2] = "transform = Transform3D(0, -3, 0, 3, 0, 0, 0, 0, 3, 7, 8, 9)"
            return node
        out, changes, problems = pack_level(0, self.data, scene(self.data, move))
        self.assertEqual(changes, ["Moby_0001: moved"])
        self.assertEqual(problems, [])
        before, after = moby_instances(self.data), moby_instances(out)
        self.assertEqual((before[0], before[2]), (after[0], after[2]))
        self.assertEqual(after[1]["matrix"][12:15], [7.0, 8.0, 9.0])
        self.assertAlmostEqual(after[1]["scale"], 3.0, places=5)
        self.assertAlmostEqual(after[1]["rotation"][2], math.pi / 2, places=5)
        self.assertEqual(after[1]["fields"], before[1]["fields"])

    def test_values_and_class(self):
        def edit(index, node):
            if index == 2:
                node[0] = node[0].replace("moby_500", "moby_7")
                node = [line.replace("rc1_spawn_id = 2", "rc1_spawn_id = 99") for line in node]
                node.append("metadata/rc1_draw_distance = 200")
            return node
        out, changes, problems = pack_level(0, self.data, scene(self.data, edit))
        self.assertEqual(changes, ["Moby_0002: class 500 -> 7, values"])
        after = moby_instances(out)[2]
        self.assertEqual((after["class_id"], after["fields"]["spawn_id"], after["fields"]["draw_distance"]), (7, 99, 200))

    def test_added_and_deleted_mobys_are_reported(self):
        def edit(index, node):
            if index == 0:
                return []                                             # deleted
            if index == 2:
                return node + ["", node[0].replace("Moby_0002", "Moby_0003")] + node[1:]   # duplicated
            return node
        out, changes, problems = pack_level(0, self.data, scene(self.data, edit))
        self.assertEqual(out, self.data)
        self.assertEqual(len(problems), 2)
        self.assertIn("a copy of Moby_0002", problems[0])
        self.assertIn("Moby_0000: deleted", problems[1])


class Placement(unittest.TestCase):
    def test_angles_come_back(self):
        from mobys import matrix
        from pack import scene_transform
        for rotation in ((0.3, -0.4, 1.1), (0.0, 0.0, 0.0), (-2.0, 1.2, 3.0), (0.5, math.pi / 2, 0.0)):
            m = matrix((1.0, 2.0, 3.0), rotation, 1.75)
            position, angles, scale = placement(scene_transform(m))
            again = matrix(position, angles, scale)
            for a, b in zip(m, again):
                self.assertAlmostEqual(a, b, places=6)

    def test_uneven_scale_is_refused(self):
        from formats import FormatError
        with self.assertRaises(FormatError):
            placement((2, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0))

    def test_scene_reader(self):
        nodes = scene_mobys('[node name="X" parent="Game/Mobys"]\nmetadata/rc1_index = 4\nmetadata/rc1_colour = [1, 2, 3]\n')
        self.assertEqual(nodes[0]["meta"], {"index": 4, "colour": [1, 2, 3]})
        self.assertEqual(nodes[0]["transform"][:3], (1.0, 0.0, 0.0))


if __name__ == "__main__":
    unittest.main()
