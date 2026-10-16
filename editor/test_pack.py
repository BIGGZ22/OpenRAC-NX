"""pack.pack_level on a made-up gameplay file and scene."""

import math
import struct
import unittest

import gameplay as gp
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



def level() -> bytes:
    """A gameplay file with what belongs to its mobys: three of them (the second with pvars, in
    group 1 with the third), seven spare runtime mobys, two pvar blocks whose first word is a
    moby's place in the table (a link) and whose second is a pointer into the block, and a
    shared record for the second block."""
    out = bytearray(gameplay([{"x": 1.0}, {"x": 2.0, "pvar_index": 1, "group": 1}, {"x": 3.0, "group": 1}]))
    struct.pack_into("<i", out, 0x100 + 4, 7)

    def section(pointer: int, data: bytes) -> None:
        out.extend(bytes(-len(out) % 16))
        struct.pack_into("<i", out, pointer, len(out))
        out.extend(data)

    section(gp.GROUPS, gp.write_groups([None, [1, 2]]))
    section(gp.TABLE, struct.pack("<4i", 0, 16, 16, 16))
    section(gp.DATA, struct.pack("<8i", 2, 4, 0, 0, 0, 8, 0x55, 0))
    section(gp.LINKS, gp.write_fixups([(0, 0), (1, 0)]))
    section(gp.POINTERS, gp.write_fixups([(1, 4)]))
    section(gp.SHARED, gp.write_shared(b"\x07" * 16, [(1, 8, 4)]))
    out.extend(bytes(-len(out) % 16))
    return bytes(out)


class AddAndDelete(unittest.TestCase):
    def setUp(self):
        self.data = level()

    def pvar(self, data: bytes, index: int) -> tuple:
        at, size = gp.pvar_table(data, index + 1)[index]
        return struct.unpack_from(f"<{size // 4}i", data, gp.offset(data, gp.DATA) + at)

    def test_a_copy_is_a_new_moby_with_a_spawn_number_pvars_and_a_place_in_its_group(self):
        def edit(index, node):
            if index != 1:
                return node
            copy = [node[0].replace("Moby_0001", "Moby_0003")] + node[1:]
            copy[2] = "transform = Transform3D(1, 0, 0, 0, 1, 0, 0, 0, 1, 9, 8, 7)"
            return node + [""] + copy
        out, changes, problems = pack_level(0, self.data, scene(self.data, edit))
        self.assertEqual(problems, [])
        self.assertEqual(changes, ["Moby_0003: added, after moby 1"])
        mobys = moby_instances(out)
        self.assertEqual(len(mobys), 4)
        self.assertEqual(gp.records(out)[:3], gp.records(self.data))            # the table's own: as they were
        self.assertEqual(struct.unpack_from("<i", out, gp.offset(out, gp.MOBYS) + 4)[0], 7)   # the spare mobys
        new = mobys[3]["fields"]
        self.assertEqual(mobys[3]["matrix"][12:15], [9.0, 8.0, 7.0])
        self.assertEqual((new["spawn_id"], new["pvar_index"], new["group"]), (3, 2, 1))
        self.assertEqual(gp.groups(out), [None, [1, 2, 3]])
        self.assertEqual(self.pvar(out, 2), self.pvar(out, 1))                  # a block of its own, the same
        self.assertNotEqual(gp.pvar_table(out, 3)[2][0], gp.pvar_table(out, 3)[1][0])
        self.assertEqual(gp.pvar_table(out, 3)[2][0] % 16, 0)
        self.assertEqual(gp.fixups(out, gp.LINKS), [(0, 0), (1, 0), (2, 0)])
        self.assertEqual(gp.fixups(out, gp.POINTERS), [(1, 4), (2, 4)])
        self.assertEqual(gp.shared(out), (b"\x07" * 16, [(1, 8, 4), (2, 8, 4)]))

    def test_a_class_scene_dropped_in_takes_after_the_levels_first_moby_of_its_class(self):
        text = scene(self.data) + ('\n[node name="Crate" parent="Game/Mobys" instance=ExtResource("moby_500")]\n'
                                   "transform = Transform3D(1, 0, 0, 0, 1, 0, 0, 0, 1, 4, 5, 6)\n")
        out, changes, problems = pack_level(0, self.data, text)
        self.assertEqual((changes, problems), (["Crate: added, after moby 0"], []))
        new = moby_instances(out)[3]
        self.assertEqual((new["class_id"], new["matrix"][12:15], new["fields"]["spawn_id"]), (500, [4.0, 5.0, 6.0], 3))

    def test_a_deleted_moby_closes_the_table_and_what_names_mobys_follows(self):
        out, changes, problems = pack_level(0, self.data, scene(self.data, lambda i, node: [] if i == 1 else node))
        self.assertEqual((changes, problems), (["BoltCrate_0001: deleted"], []))
        self.assertEqual(gp.records(out), [gp.records(self.data)[0], gp.records(self.data)[2]])
        self.assertEqual(gp.groups(out), [None, [1]])                           # the third moby is now the second
        self.assertEqual(self.pvar(out, 0)[0], 1)                               # block 0 linked to it: follows
        self.assertEqual(self.pvar(out, 1)[0], 0)                               # block 1's is the first: stays
        self.assertEqual(len(out), len(self.data))                              # nothing grew: all in place

    def test_a_link_to_a_deleted_moby_becomes_none(self):
        out, _, _ = pack_level(0, self.data, scene(self.data, lambda i, node: [] if i == 2 else node))
        self.assertEqual(self.pvar(out, 0)[0], -1)
        self.assertEqual(gp.groups(out), [None, [1]])


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


class Names(unittest.TestCase):
    def test_a_node_is_named_after_its_class(self):
        from mobys import node_name
        names = {11: "vendor", 8: "blarg_heavy_interceptor"}
        self.assertEqual(node_name(11, 12, names), "Vendor_0012")
        self.assertEqual(node_name(8, 40, names), "BlargHeavyInterceptor_0040")
        self.assertEqual(node_name(809, 1, names), "Class809_0001")

    def test_any_name_packs(self):
        """The packer goes by rc1_index, not by the node's name."""
        data = gameplay([{"x": 1.0}])
        text = scene(data).replace("Moby_0000", "Vendor_0000")
        self.assertEqual(pack_level(0, data, text), (data, [], []))


class PackDisc(unittest.TestCase):
    """pack.pack on either disc: the copy its game loads is the one replaced."""

    def test_each_disc_packs_its_own_gameplay_copy(self):
        import json
        import tempfile
        from pathlib import Path
        import disc
        from formats import wad
        from pack import pack
        from wadpack import compress
        data = gameplay([{"x": 1.0}, {"x": 2.0}])
        stored = compress(data)
        sectors = -(-len(stored) // disc.SECTOR)
        survey = {"levels": [{"id": 0, "ranges": {"gameplay_ntsc": {"lba": 100, "sectors": sectors},
                                                  "gameplay_pal": {"lba": 200, "sectors": sectors}}}]}

        def move(index, node):
            if index == 1:
                node[2] = "transform = Transform3D(1, 0, 0, 0, 1, 0, 0, 0, 1, 5, 6, 7)"
            return node
        for version, lba in ((disc.NTSC, 100), (disc.PAL, 200)):
            class Disc:
                pass
            image = Disc()
            image.version = version
            image.sectors = lambda first, count: stored + bytes(count * disc.SECTOR - len(stored))
            with tempfile.TemporaryDirectory() as tmp:
                project, out = Path(tmp) / "levels", Path(tmp) / "packed"
                (project / "level_00/levels/level_00").mkdir(parents=True)
                (project / "level_00/levels/level_00/level_00.tscn").write_text(scene(data, move))
                self.assertEqual(pack(image, survey, project, out), (1, 1))
                name = f"level_00/gameplay_{version.key}.wad"
                self.assertEqual((out / "patches.tsv").read_text().splitlines()[1], f"{lba}\t{sectors}\t{name}")
                self.assertEqual(moby_instances(wad((out / name).read_bytes()))[1]["matrix"][12:15], [5.0, 6.0, 7.0])
                self.assertEqual(json.loads((out / "packed.json").read_text())["version"], version.key)


if __name__ == "__main__":
    unittest.main()
