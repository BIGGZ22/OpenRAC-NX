"""gamedata.py on a made-up gameplay file: the sections read and written again, the scene the
extractor writes for them, and what packs back."""

import math
import re
import struct
import tempfile
import unittest
from pathlib import Path

import gamedata as gd
import gameplay as gp
from godot import Scene, transform
from mobys import FIELDS, RECORD, SIZE, USUAL, moby_instances
from pack import pack_level

LAYOUT = {500: [("speed", 0x10, "f32", 0), ("count", 0x14, "s16", 0), ("ids", 0x18, "u8", 4)]}


def shape(matrix_rows: list[tuple], euler=(0.0, 0.0, 0.0)) -> bytes:
    """A shape body from its matrix rows (three axes, then the centre)."""
    cols = [list(r) for r in matrix_rows[:3]]
    inv = gd.inverse3(cols)
    out = struct.pack("<16f", *cols[0], 0.0, *cols[1], 0.0, *cols[2], 0.0, *matrix_rows[3], gd.SHAPE_W[3])
    out += struct.pack("<12f", *inv[0], 0.0, *inv[1], 0.0, *inv[2], 0.0)
    return out + struct.pack("<3f", *euler) + bytes(4)


def build() -> bytes:
    """A gameplay file with every section gamedata.py reads, and three mobys."""
    sections = {}
    sections[0x00] = gd.write_settings({"background": [10, 20, 30], "fog_colour": [40, 50, 60], "fog_near": 100.0,
                                        "fog_far": 2000.0, "fog_near_intensity": 255.0, "fog_far_intensity": 63.75,
                                        "death_height": 12.0, "ship_position": [50.0, 60.0, 70.0], "ship_yaw": 0.5,
                                        "ship_path": 1, "ship_camera_cuboids": [0, 1], "pad": [0, 0]})
    sections[0x08] = struct.pack("<i12x", 2) + gd.CAMERA.pack(17, 1.0, 2.0, 3.0, 0.1, 0.2, 0.3, 2) \
        + gd.CAMERA.pack(4, -1.0, -2.0, -3.0, 0.0, 0.0, 1.5, -1)
    sound = gd.SOUND_HEAD.pack(3, 0, 0, 3, 25.0) + shape([(4, 0, 0), (0, 4, 0), (0, 0, 4), (9, 8, 7)])
    sections[0x0C] = struct.pack("<i12x", 1) + sound
    records = []
    for i, fields in enumerate(({"class_id": 500, "pvar_index": 0, "group": 0, "x": 1.0},
                                {"class_id": 500, "pvar_index": 1, "group": 0, "x": 2.0, "rz": 0.5},
                                {"class_id": 7, "pvar_index": -1, "group": 2, "x": 3.0})):
        record = {"size": SIZE, **USUAL, "spawn_id": i, "scale": 1.0, "x": 0.0, "y": 0.0, "z": 0.0, "rx": 0.0,
                  "ry": 0.0, "rz": 0.0, "r": 128, "g": 128, "b": 128, **fields}
        records.append(RECORD.pack(*(record[f] for f in FIELDS)))
    sections[0x44] = struct.pack("<2i8x", 3, 4) + b"".join(records)
    sections[0x48] = gp.write_groups([[0, 1], None, [2]])
    sections[0x4C] = gp.write_shared(bytes(range(16)), [(1, 8, 0)])
    sections[0x50] = gp.write_fixups([(0, 0x20)])
    blocks = [struct.pack("<4f", 1.0, 2.5, -3.0, 0.0) + struct.pack("<fh2x4B", 7.5, 3, 1, 2, 3, 4) + bytes(4)
              + struct.pack("<i12x", 2),
              struct.pack("<4i", 0, 0x10, 0, 99) + struct.pack("<fh2x4B", 1.0, 5, 0, 0, 0, 0) + bytes(4),
              struct.pack("<f4x4if4B", 8.0, -1, 0, -1, -1, 1.5, 1, 1, 0, 4) + bytes(0x20),
              struct.pack("<4i", 5, 6, 7, 8)]
    table, data = [], b""
    for block in blocks:
        table.append((len(data), len(block)))
        data += block
    sections[0x54] = b"".join(struct.pack("<2i", *t) for t in table)
    sections[0x58] = data
    sections[0x5C] = gp.write_fixups([(1, 4)])
    sections[0x60] = struct.pack("<i12x", 2) + shape([(2, 0, 0), (0, 3, 0), (0, 0, 1), (10, 20, 30)]) \
        + shape([(0, 1, 0), (-1, 0, 0), (0, 0, 1), (5, 5, 5)], (0.0, 0.0, math.pi / 2))
    sections[0x64] = struct.pack("<i12x", 1) + shape([(1, 0, 0), (0, 1, 0), (0, 0, 1), (0, 0, 0)])
    sections[0x68] = struct.pack("<i12x", 0)
    sections[0x6C] = struct.pack("<i12x", 0)
    probe = bytearray(0x200)
    struct.pack_into("<2i", probe, gd.PATHS, 0x100, 0x180)
    struct.pack_into("<i", probe, 0x100, 0)
    sections[0x70] = gd.write_paths(bytes(probe), [[(0.0, 0.0, 0.0, -1.0), (10.0, 0.0, 0.0, -1.0)],
                                                   [(1.0, 2.0, 3.0, 0.0), (4.0, 5.0, 6.0, 1.0), (7.0, 8.0, 9.0, 2.0)]])
    sections[0x74] = gd.write_grind_paths(bytes(probe), [{"sphere": (5.0, 0.0, 1.0, 5.0), "words": (1, 0, 0, 0),
                                                          "points": [(0.0, 0.0, 1.0, 10.0), (10.0, 0.0, 1.0, 10.0)]}])
    out = bytearray(0xA0)
    for pointer, data in sections.items():
        struct.pack_into("<i", out, pointer, len(out))
        out += data + bytes(-len(data) % 16)
    return bytes(out)


def scene_text(g: bytes, gameplay_nodes: bool = True, pvars: bool = True) -> str:
    """The level scene as the extractor writes it: the mobys (with their pvars) and the gameplay
    nodes; an older project has neither."""
    with tempfile.TemporaryDirectory() as directory:
        scene = Scene()
        scene.resource("PackedScene", "res://levels/level_00/mobys/moby_500.tscn", "moby_500")
        scene.resource("PackedScene", "res://levels/level_00/mobys/moby_7.tscn", "moby_7")
        extra, names = gd.moby_metadata(g) if pvars else {}, gd.moby_names(g)
        scene.node("Level_00", kind="Node3D")
        scene.node("Game", ".", "Node3D")
        scene.node("Mobys", "Game", "Node3D")
        for p in moby_instances(g):
            fields = {f"metadata__rc1_{k}": v for k, v in {"index": p["index"], **p["fields"]}.items()}
            fields.update(extra.get(p["index"], {}))
            scene.node(names[p["index"]], "Game/Mobys", instance=f'ExtResource("moby_{p["class_id"]}")',
                       rotation_order=0, transform=transform(p["matrix"]), **fields)
        if gameplay_nodes:
            gd.write_scene(scene, Path(directory), "res://levels/level_00", g)
        return scene.text()


def edit(text: str, node: str, key: str, value: str) -> str:
    """TEXT with NODE's KEY set to VALUE (added when the node has none)."""
    m = re.search(r'^\[node name="%s"[^\n]*\n(?:(?!\[)[^\n]*\n)*' % re.escape(node), text, re.M)
    block = m.group(0)
    if re.search(r"^%s = " % re.escape(key), block, re.M):
        new = re.sub(r"^%s = .*$" % re.escape(key), lambda _: f"{key} = {value}", block, count=1, flags=re.M)
    else:
        new = block.rstrip("\n") + f"\n{key} = {value}\n"
    return text[:m.start()] + new + text[m.end():]


def changed_bytes(a: bytes, b: bytes) -> list[int]:
    return [i for i in range(max(len(a), len(b))) if i >= len(a) or i >= len(b) or a[i] != b[i]]


class GameData(unittest.TestCase):
    def setUp(self):
        self.saved = gd._layouts
        gd._layouts = LAYOUT
        self.g = build()
        self.text = scene_text(self.g)

    def tearDown(self):
        gd._layouts = self.saved

    def pack(self, text: str):
        return pack_level(0, self.g, text)

    def section(self, g: bytes, pointer: int) -> int:
        return gp.offset(g, pointer)

    def pvar_at(self, g: bytes, index: int, at: int) -> int:
        return gp.offset(g, gp.DATA) + gp.pvar_table(g, index + 1)[index][0] + at

    # --- the sections ---

    def test_sections_write_back_as_read(self):
        g = self.g
        for pointer, data in ((gd.PATHS, gd.write_paths(g, gd.read_paths(g))),
                              (gd.GRIND_PATHS, gd.write_grind_paths(g, gd.read_grind_paths(g))),
                              (gd.SETTINGS, gd.write_settings(gd.read_settings(g))),
                              (gd.CAMERAS, gd.write_table(g, gd.CAMERAS, gd.table(g, gd.CAMERAS, gd.CAMERA_SIZE))),
                              (gd.SOUNDS, gd.write_table(g, gd.SOUNDS, gd.table(g, gd.SOUNDS, gd.SOUND_SIZE))),
                              (0x60, gd.write_table(g, 0x60, gd.table(g, 0x60, gd.SHAPE_SIZE))),
                              (gp.GROUPS, gp.write_groups(gp.groups(g)))):
            at = gp.offset(g, pointer)
            self.assertEqual(g[at:at + len(data)], data, hex(pointer))
            self.assertEqual(len(data) % 16, 0, hex(pointer))

    def test_paths_are_read(self):
        paths = gd.read_paths(self.g)
        self.assertEqual([len(p) for p in paths], [2, 3])
        self.assertEqual(paths[1][2], (7.0, 8.0, 9.0, 2.0))
        grind, = gd.read_grind_paths(self.g)
        self.assertEqual((grind["sphere"], grind["words"][0], len(grind["points"])), ((5.0, 0.0, 1.0, 5.0), 1, 2))

    def test_shape_body_inverts_its_matrix(self):
        t = (0.0, -2.0, 0.0, 2.0, 0.0, 0.0, 0.0, 0.0, 3.0, 1.0, 2.0, 3.0)   # turned 90 degrees, scaled 2, 2, 3
        body = gd.shape_body(t, None)
        m = struct.unpack_from("<16f", body)
        inv = struct.unpack_from("<12f", body, 0x40)
        rows, inverse = [m[0:3], m[4:7], m[8:11]], [inv[0:3], inv[4:7], inv[8:11]]
        for a in range(3):
            for b in range(3):
                self.assertAlmostEqual(sum(rows[a][k] * inverse[k][b] for k in range(3)), float(a == b), places=6)
        self.assertEqual(m[12:16], (1.0, 2.0, 3.0, gd.SHAPE_W[3]))
        self.assertAlmostEqual(struct.unpack_from("<f", body, 0x78)[0], math.pi / 2, places=6)
        with self.assertRaises(gd.FormatError):
            gd.shape_body((0.0,) * 12, None)

    def test_scene_values_are_read(self):
        self.assertEqual(gd.parse_value('[1, 2.5, -3, 1e-05, "a\\"b", true, null]'), [1, 2.5, -3, 1e-05, 'a"b', True, None])
        self.assertEqual(gd.parse_value('{\n"points": PackedVector3Array(1, 2, 3)\n}')["points"].args, [1, 2, 3])
        self.assertEqual(gd.parse_value("Array[int]([1, 2])"), [1, 2])
        self.assertTrue(math.isnan(gd.parse_value("nan")))
        self.assertEqual(gd.text([1.0, 2, "x"]), '[1.0, 2, "x"]')

    def test_words_show_as_floats_or_integers(self):
        self.assertEqual([gd.word_value(struct.pack("<f", 2.5)), gd.word_value(struct.pack("<i", -1)),
                          gd.word_value(struct.pack("<i", 7)), gd.word_value(struct.pack("<I", 0x80000000))],
                         [2.5, -1, 7, -(1 << 31)])

    # --- the scene ---

    def test_the_scene_shows_the_data(self):
        nodes, _ = gd.parse_scene(self.text)
        moby = gd.meta_of(nodes["Game/Mobys/BoltCrate_0000"])
        self.assertEqual(moby["pv_speed"], 7.5)
        self.assertEqual(moby["pv_ids"], [1, 2, 3, 4])
        self.assertEqual(moby["link_20"], "Class7_0002")
        self.assertNotIn("pv_speed", gd.meta_of(nodes["Game/Mobys/Class7_0002"]))
        self.assertEqual(gd.meta_of(nodes["Game/MobyGroups"]), {"group_0": ["BoltCrate_0000", "BoltCrate_0001"],
                                                                 "group_2": ["Class7_0002"]})
        camera = gd.meta_of(nodes["Game/Cameras/Camera_0000"])
        self.assertEqual((camera["class"], camera["pv_cuboid"], camera["pv_activation"]), (17, 0, 4))
        self.assertEqual(gd.meta_of(nodes["Game/LevelSettings"])["death_height"], 12.0)
        for path in ("Game/Volumes/Cuboids/Cuboid_0001", "Game/Volumes/Spheres/Sphere_0000", "Game/Sounds/Sound_0000",
                     "Game/Paths/Path_0001", "Game/GrindPaths/GrindPath_0000", "Game/LevelSettings/Ship"):
            self.assertIn(path, nodes)
        self.assertNotIn("Game/Volumes/Cylinders/Cylinder_0000", nodes)

    def test_untouched_scene_packs_to_nothing(self):
        self.assertEqual(self.pack(self.text), (self.g, [], []))

    def test_godots_own_saving_changes_nothing(self):
        """Node types and unique_ids, other sub_resource ids, other float text and double-precision
        tilts are not edits."""
        text = re.sub(r'\[node name="([^"]+)" parent=', r'[node name="\1" type="Node3D" parent=', self.text)
        text = re.sub(r"(parent=\"[^\"]*\")", r"\1 unique_id=1234", text)
        text = text.replace('"path_0001"', '"Curve3D_x7k2p"').replace("PackedFloat32Array(", "PackedFloat64Array(")
        text = text.replace("metadata/rc1_pv_speed = 7.5", "metadata/rc1_pv_speed = 7.500000001")
        text = text.replace("metadata/rc1_death_height = 12.0", "metadata/rc1_death_height = 12")
        self.assertEqual(self.pack(text), (self.g, [], []))

    def test_an_older_project_packs_as_before(self):
        """A scene exported before the gameplay data: no gameplay nodes, no pvar metadata."""
        text = scene_text(self.g, gameplay_nodes=False, pvars=False)
        self.assertEqual(self.pack(text), (self.g, [], []))
        moved = edit(text, "BoltCrate_0001", "transform", "Transform3D(1, 0, 0, 0, 1, 0, 0, 0, 1, 7, 8, 9)")
        out, changes, problems = self.pack(moved)
        self.assertEqual((changes, problems), (["BoltCrate_0001: moved"], []))
        self.assertTrue(all(gp.offset(self.g, gp.MOBYS) <= i < gp.offset(self.g, gp.GROUPS) for i in changed_bytes(self.g, out)))

    # --- pvars and links ---

    def test_a_pvar_word(self):
        out, changes, problems = self.pack(edit(self.text, "BoltCrate_0000", "metadata/rc1_pvar",
                                                "[1.0, 2.5, -3.0, 42, 7.5, 3, 67305985, 0, 2, 0, 0, 0]"))
        self.assertEqual((changes, problems), (["BoltCrate_0000: pvars"], []))
        at = self.pvar_at(out, 0, 12)
        self.assertEqual(changed_bytes(self.g, out), [at])
        self.assertEqual(struct.unpack_from("<i", out, at)[0], 42)

    def test_a_typed_field(self):
        text = edit(self.text, "BoltCrate_0001", "metadata/rc1_pv_speed", "-0.25")
        text = edit(text, "BoltCrate_0001", "metadata/rc1_pv_ids", "[9, 8, 7, 6]")
        out, changes, problems = self.pack(text)
        self.assertEqual((changes, problems), (["BoltCrate_0001: pvars"], []))
        at = self.pvar_at(out, 1, 0x10)
        self.assertEqual(struct.unpack_from("<f4B", out, at + 0) [0], -0.25)
        self.assertEqual(struct.unpack_from("<4B", out, at + 8), (9, 8, 7, 6))
        self.assertTrue(all(at <= i < at + 12 for i in changed_bytes(self.g, out)))

    def test_a_field_out_of_its_range_is_not_packed(self):
        text = edit(self.text, "BoltCrate_0001", "metadata/rc1_pv_ids", "[300, 8, 7, 6]")
        text = edit(text, "BoltCrate_0001", "metadata/rc1_pv_speed", "inf")
        out, changes, problems = self.pack(text)
        self.assertEqual((out, changes), (self.g, []))
        self.assertEqual(len(problems), 2)

    def test_words_the_loader_rewrites_are_not_packed(self):
        """+0x4 is a pointer fixup and +0x8 a shared-data address of pvar block 1."""
        out, changes, problems = self.pack(edit(self.text, "BoltCrate_0001", "metadata/rc1_pvar",
                                                "[0, 32, 5, 99, 1.0, 5, 0, 0]"))
        self.assertEqual((out, changes), (self.g, []))
        self.assertEqual(len(problems), 2)
        self.assertIn("rewrites", problems[0])

    def test_a_pvar_list_of_another_size_is_not_packed(self):
        out, changes, problems = self.pack(edit(self.text, "BoltCrate_0000", "metadata/rc1_pvar", "[1, 2]"))
        self.assertEqual((out, changes), (self.g, []))
        self.assertIn("needs 12 words", problems[0])

    def test_a_link(self):
        out, changes, problems = self.pack(edit(self.text, "BoltCrate_0000", "metadata/rc1_link_20", '"BoltCrate_0001"'))
        self.assertEqual((changes, problems), (["BoltCrate_0000: pvars"], []))
        self.assertEqual(struct.unpack_from("<i", out, self.pvar_at(out, 0, 0x20))[0], 1)
        out, _, _ = self.pack(edit(self.text, "BoltCrate_0000", "metadata/rc1_link_20", '"-1"'))
        self.assertEqual(struct.unpack_from("<i", out, self.pvar_at(out, 0, 0x20))[0], -1)
        out, changes, problems = self.pack(edit(self.text, "BoltCrate_0000", "metadata/rc1_link_20", '"Nobody_0042"'))
        self.assertEqual((out, changes), (self.g, []))
        self.assertIn("no moby of the level", problems[0])

    def test_a_link_follows_a_deleted_moby_and_names_a_copy(self):
        """BoltCrate_0001 deleted: Class7_0002 becomes 1 and the link to it follows (pack.py); a copy of
        BoltCrate_0000 is new, and a link may name it."""
        text = re.sub(r'\[node name="BoltCrate_0001"[^\n]*\n(?:(?!\[)[^\n]*\n)*', "", self.text)
        copy = re.search(r'\[node name="BoltCrate_0000"[^\n]*\n(?:(?!\[)[^\n]*\n)*', text).group(0)
        copy = copy.replace('name="BoltCrate_0000"', 'name="BoltCrate_0000b"').replace("rc1_pv_speed = 7.5", "rc1_pv_speed = 1.5")
        text = text.replace('[node name="Class7_0002"', copy + '\n[node name="Class7_0002"')
        text = edit(text, "BoltCrate_0000", "metadata/rc1_link_20", '"BoltCrate_0000b"')
        out, changes, problems = self.pack(text)
        self.assertEqual(problems, [])
        self.assertEqual(len(moby_instances(out)), 3)
        new_pvar = moby_instances(out)[2]["fields"]["pvar_index"]
        self.assertEqual(struct.unpack_from("<i", out, self.pvar_at(out, 0, 0x20))[0], 2)        # the copy
        self.assertEqual(struct.unpack_from("<f", out, self.pvar_at(out, new_pvar, 0x10))[0], 1.5)  # its own block
        self.assertEqual(struct.unpack_from("<f", out, self.pvar_at(out, 0, 0x10))[0], 7.5)      # the source's
        self.assertEqual(struct.unpack_from("<i", out, self.pvar_at(out, new_pvar, 0x20))[0], 1)   # Class7_0002, now 1
        self.assertIn("BoltCrate_0000b: pvars", changes)

    # --- groups ---

    def test_a_group_list(self):
        text = edit(self.text, "MobyGroups", "metadata/rc1_group_0", '["BoltCrate_0000"]')
        text = edit(text, "MobyGroups", "metadata/rc1_group_1", '["BoltCrate_0001"]')
        out, changes, problems = self.pack(text)
        self.assertEqual(problems, [])
        self.assertEqual(gp.groups(out), [[0], [1], [2]])
        self.assertEqual([m["fields"]["group"] for m in moby_instances(out)], [0, 1, 2])

    def test_a_mobys_group_moves_it_between_lists(self):
        out, changes, problems = self.pack(edit(self.text, "BoltCrate_0000", "metadata/rc1_group", "2"))
        self.assertEqual(problems, [])
        self.assertEqual(gp.groups(out), [[1], None, [2, 0]])

    # --- volumes, paths, cameras, sounds, settings ---

    def test_a_volume_moved(self):
        out, changes, problems = self.pack(edit(self.text, "Cuboid_0000", "transform",
                                                "Transform3D(0, -2, 0, 2, 0, 0, 0, 0, 1, 11, 21, 31)"))
        self.assertEqual((changes, problems), (["Cuboid_0000: moved"], []))
        at = gp.offset(out, 0x60) + 0x10
        self.assertTrue(all(at <= i < at + 0x80 for i in changed_bytes(self.g, out)))
        self.assertEqual(struct.unpack_from("<4f", out, at + 0x30), (11.0, 21.0, 31.0, gd.SHAPE_W[3]))
        self.assertAlmostEqual(struct.unpack_from("<f", out, at + 0x78)[0], math.pi / 2, places=6)

    def test_a_moved_group_node_moves_its_volumes(self):
        text = edit(self.text, "Spheres", "transform", "Transform3D(1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 5)")
        out, changes, problems = self.pack(text)
        self.assertEqual(changes, ["Sphere_0000: moved"])
        self.assertEqual(struct.unpack_from("<3f", out, gp.offset(out, 0x64) + 0x10 + 0x30), (0.0, 0.0, 5.0))

    def test_a_volume_added_and_one_deleted(self):
        copy = re.search(r'\[node name="Cuboid_0001"[^\n]*\n(?:(?!\[)[^\n]*\n)*', self.text).group(0)
        text = self.text + "\n" + copy.replace('name="Cuboid_0001"', 'name="Cuboid_0001b"')
        text = re.sub(r'\[node name="Sphere_0000"[^\n]*\n(?:(?!\[)[^\n]*\n)*', "", text)
        out, changes, problems = self.pack(text)
        self.assertEqual(changes, ["Cuboid_0001b: added as Cuboid_0002"])
        self.assertIn("deleting volumes is not supported", problems[0])
        bodies = gd.table(out, 0x60, gd.SHAPE_SIZE)
        self.assertEqual(len(bodies), 3)
        self.assertEqual(bodies[2], bodies[1])
        self.assertEqual(gd.table(out, 0x64, gd.SHAPE_SIZE), gd.table(self.g, 0x64, gd.SHAPE_SIZE))
        self.assertEqual(gp.offset(out, 0x60) % 16, 0)

    def test_a_path_edited_and_one_added(self):
        text = self.text.replace("0, 0, 0, 0, 0, 0, 10, 0, 0", "0, 0, 0, 0, 0, 0, 10, 0, 4", 1)
        text = edit(text, "Path_0001", "metadata/rc1_w", "[0.0, 1.0, 5.0]")
        text += ('\n[sub_resource type="Curve3D" id="new"]\n_data = {\n"points": PackedVector3Array(0, 0, 0, 0, 0, 0, '
                 '1, 1, 1, 0, 0, 0, 0, 0, 0, 2, 2, 2),\n"tilts": PackedFloat32Array(0, 0)\n}\npoint_count = 2\n'
                 '\n[node name="MyPath" type="Path3D" parent="Game/Paths"]\ncurve = SubResource("new")\n')
        out, changes, problems = self.pack(text)
        self.assertEqual(problems, [])
        self.assertEqual(changes, ["Path_0000: points", "Path_0001: points", "MyPath: added as Path_0002"])
        paths = gd.read_paths(out)
        self.assertEqual(paths[0][1], (10.0, 0.0, 4.0, -1.0))
        self.assertEqual(paths[1][2][3], 5.0)
        self.assertEqual(paths[2], [(1.0, 1.0, 1.0, gd.PATH_W), (2.0, 2.0, 2.0, gd.PATH_W)])

    def test_a_grind_path(self):
        text = self.text.replace("0, 0, 0, 0, 0, 0, 10, 0, 1)", "0, 0, 0, 0, 0, 0, 20, 0, 1)")
        text = edit(text, "GrindPath_0000", "metadata/rc1_flag", "0")
        out, changes, problems = self.pack(text)
        self.assertEqual((changes, problems), (["GrindPath_0000: points, flag"], []))
        grind, = gd.read_grind_paths(out)
        self.assertEqual(grind["words"][0], 0)
        self.assertEqual(grind["points"][1], (20.0, 0.0, 1.0, 10.0))
        centre, radius = grind["sphere"][:3], grind["sphere"][3]
        self.assertTrue(all(math.dist(centre, p[:3]) <= radius for p in grind["points"]))

    def test_a_camera_and_a_sound(self):
        text = edit(self.text, "Camera_0001", "transform", "Transform3D(1, 0, 0, 0, 1, 0, 0, 0, 1, 4, 5, 6)")
        text = edit(text, "Camera_0000", "metadata/rc1_pv_activation", "1")
        text = edit(text, "Sound_0000", "metadata/rc1_range", "40.0")
        out, changes, problems = self.pack(text)
        self.assertEqual(problems, [])
        self.assertEqual(sorted(changes), ["Camera_0000: pvars", "Camera_0001: moved", "Sound_0000: range"])
        cameras = gd.table(out, gd.CAMERAS, gd.CAMERA_SIZE)
        self.assertEqual(gd.CAMERA.unpack(cameras[1]), (4, 4.0, 5.0, 6.0, 0.0, 0.0, 0.0, -1))
        self.assertEqual(out[self.pvar_at(out, 2, 0x1F)], 1)
        self.assertEqual(gd.SOUND_HEAD.unpack_from(gd.table(out, gd.SOUNDS, gd.SOUND_SIZE)[0])[4], 40.0)

    def test_cameras_are_neither_added_nor_deleted(self):
        text = re.sub(r'\[node name="Camera_0001"[^\n]*\n(?:(?!\[)[^\n]*\n)*', "", self.text)
        text += '\n[node name="Camera_0099" parent="Game/Cameras"]\n'
        out, changes, problems = self.pack(text)
        self.assertEqual((out, changes), (self.g, []))
        self.assertEqual(len(problems), 2)

    def test_level_settings_and_the_ship(self):
        text = edit(self.text, "LevelSettings", "metadata/rc1_death_height", "-4.5")
        text = edit(text, "LevelSettings", "metadata/rc1_fog_colour", "[1, 2, 3]")
        text = edit(text, "Ship", "transform", "Transform3D(0, -1, 0, 1, 0, 0, 0, 0, 1, 1, 2, 3)")
        out, changes, problems = self.pack(text)
        self.assertEqual(problems, [])
        settings = gd.read_settings(out)
        self.assertEqual((settings["death_height"], settings["fog_colour"], settings["ship_position"]),
                         (-4.5, [1, 2, 3], [1.0, 2.0, 3.0]))
        self.assertAlmostEqual(settings["ship_yaw"], math.pi / 2, places=6)
        at = gp.offset(out, gd.SETTINGS)
        self.assertTrue(all(at <= i < at + gd.SETTINGS_SIZE for i in changed_bytes(self.g, out)))

    def test_bad_settings_are_not_packed(self):
        text = edit(self.text, "LevelSettings", "metadata/rc1_fog_near", "nan")
        text = edit(text, "LevelSettings", "metadata/rc1_ship_path", "7")
        text = edit(text, "LevelSettings", "metadata/rc1_background", "[1, 2]")
        out, changes, problems = self.pack(text)
        self.assertEqual((out, changes), (self.g, []))
        self.assertEqual(len(problems), 3)


if __name__ == "__main__":
    unittest.main()
