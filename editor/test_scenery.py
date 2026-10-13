"""scenery.pack_family on made-up ties."""

import unittest

from godot import transform
from scenery import matrix_of, pack_family
from pack import scene_nodes, scene_transform

VALUES = ("draw_distance", "occlusion_index", "directional_lights", "uid")


def tie(index: int, x: float, class_id: int = 2020) -> dict:
    matrix = [1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, x, 2.0, 3.0, 1.0]
    return {"index": index, "class_id": class_id, "matrix": matrix, "stored_w": 0.01, "draw_distance": 140,
            "occlusion_index": index, "directional_lights": 0, "uid": index + 1, "ambient": [0] * 64}


def scene(ties: list[dict], edit=None) -> str:
    """The tie nodes as the extractor writes them; EDIT changes the lines of one node."""
    lines = ['[ext_resource type="PackedScene" path="res://levels/level_00/ties/tie_2020.glb" id="tie_2020"]',
             '[ext_resource type="PackedScene" path="res://levels/level_00/ties/tie_2021.glb" id="tie_2021"]', ""]
    for p in ties:
        node = [f'[node name="Tie_{p["index"]:04}" parent="Game/Ties" instance=ExtResource("tie_{p["class_id"]}")]',
                f"transform = {transform(p['matrix'])}", f"metadata/rc1_index = {p['index']}",
                f"metadata/rc1_draw_distance = {p['draw_distance']}", f"metadata/rc1_uid = {p['uid']}"]
        if edit:
            node = edit(p["index"], node)
        lines += node + [""]
    return "\n".join(lines)


class Ties(unittest.TestCase):
    def setUp(self):
        self.ties = [tie(0, 10.0), tie(1, 20.0), tie(2, 30.0, 2021)]

    def pack(self, text: str):
        return pack_family(scene_nodes(text, "Ties", "tie"), self.ties, {2020, 2021}, VALUES)

    def test_an_untouched_scene_is_the_discs(self):
        self.assertEqual(self.pack(scene(self.ties)), (None, [], []))

    def test_the_readers_matrix_comes_back(self):
        self.assertEqual(matrix_of(scene_transform(self.ties[1]["matrix"])), self.ties[1]["matrix"])

    def test_moved_copied_and_deleted(self):
        def edit(index, node):
            if index == 0:
                return []                                                          # deleted
            if index == 1:
                node[1] = "transform = Transform3D(1, 0, 0, 0, 2, 0, 0, 0, 3, 21, 2, 3)"   # moved, each axis its scale
                return node
            copy = [node[0].replace("Tie_0002", "Tie_0003"), "transform = Transform3D(1, 0, 0, 0, 1, 0, 0, 0, 1, 7, 8, 9)"]
            return node + [""] + copy + node[2:]
        out, changes, problems = self.pack(scene(self.ties, edit))
        self.assertEqual(problems, [])
        self.assertEqual(changes, ["Tie_0001: moved", "Tie_0003: added, after 2", "0000: deleted"])
        self.assertEqual([p["class_id"] for p in out], [2020, 2021, 2021])
        self.assertEqual(out[0]["matrix"][12:15], [21.0, 2.0, 3.0])
        self.assertEqual((out[0]["matrix"][5], out[0]["matrix"][10]), (2.0, 3.0))
        self.assertEqual(out[1], self.ties[2])                                     # as the disc has it
        self.assertEqual(out[2]["matrix"][12:15], [7.0, 8.0, 9.0])
        self.assertEqual(out[2]["uid"], self.ties[2]["uid"])                       # takes after what it copies

    def test_a_class_the_level_lacks_is_refused(self):
        text = scene(self.ties).replace('path="res://levels/level_00/ties/tie_2021.glb"', 'path="res://levels/level_00/ties/tie_9.glb"')
        out, changes, problems = pack_family(scene_nodes(text, "Ties", "tie"), self.ties, {2020, 2021}, VALUES)
        self.assertIn("class 9 is not one of the level's", problems[0])


if __name__ == "__main__":
    unittest.main()
