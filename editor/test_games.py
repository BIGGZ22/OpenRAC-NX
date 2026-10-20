"""The game/format table and what the shared readers do with each game's values.

Synthetic records only: the fixtures of the readers' own tests, laid out as the later games
lay them out (games.py says where); no disc data.
"""

import json
from pathlib import Path
import struct
import tempfile
import unittest

import games
from games import RAC1, RAC2, RAC3, RAC4
from godot import problem_summary, write_project
from level import unswizzle
from moby_anim import bind_pose
from mobys import moby_instances
from shrubs import shrub_instances
from sky import sky
from ties import tie_instances, tie_mesh
import wads
import test_moby_anim
import test_sky
import test_ties

ROOT = Path(__file__).resolve().parents[1]


class TableTests(unittest.TestCase):
    def test_ratchet_and_clank_keeps_the_values_the_readers_had(self):
        self.assertEqual((RAC1.ties, RAC1.shrubs, RAC1.mobys, RAC1.tie_record), (0x34, 0x3c, 0x44, 0xe0))
        self.assertEqual((RAC1.moby.size, RAC1.moby.layout), (0x78, "<7if4i3f3f2if4i3i2i"))
        self.assertEqual((RAC1.tie_header.packet_count, RAC1.tie_header.texture_count, RAC1.tie_header.ad_gifs,
                          RAC1.tie_header.normals), (0x20, 0x23, 0x2c, 0x0c))
        self.assertTrue(RAC1.tie_ambient and RAC1.gadgets and RAC1.ratchet_sequences and RAC1.moby_glow)
        self.assertTrue(RAC1.lighting and RAC1.class_names and RAC1.packable)
        self.assertFalse(RAC1.lenient or RAC1.stash or RAC1.compact_vertex_table or RAC1.swizzled_textures)

    def test_later_games_are_early_access(self):
        for fmt in (RAC2, RAC3, RAC4):
            with self.subTest(fmt.game):
                self.assertTrue(fmt.lenient and fmt.stash and fmt.compact_vertex_table)
                self.assertFalse(fmt.packable or fmt.lighting or fmt.class_names or fmt.gadgets)
                self.assertEqual(fmt.tie_record, 0x60)
        self.assertEqual((RAC2.moby.size, RAC3.moby.size, RAC4.moby.size), (0x88, 0x88, 0x70))
        self.assertEqual((RAC4.ties, RAC4.shrubs, RAC4.mobys), (0x08, 0x14, 0x30))
        self.assertTrue(RAC4.swizzled_textures and RAC4.skeleton_3x4 and not RAC4.ratchet_sequences)

    def test_records_lay_out_their_fields(self):
        for record in (games.RAC1_MOBY, games.GC_MOBY, games.UYA_MOBY, games.DL_MOBY):
            with self.subTest(size=record.size):
                self.assertEqual(struct.calcsize(record.layout), record.size)
                self.assertEqual(record.fields[0], "size")
                for name in ("class_id", "spawn_id", "group", "pvar_index", "x", "rz", "scale", "r", "b"):
                    self.assertIn(name, record.fields)
                self.assertTrue(set(record.usual) <= set(record.fields))
        with self.assertRaises(ValueError):
            games.MobyRecord(0x10, "<4i", ("size",))

    def test_level_names_and_versions(self):
        self.assertEqual(RAC1.level_name(0), "Veldin")
        self.assertEqual(RAC2.level_name(30), "Insomniac Museum")
        self.assertEqual(RAC3.level_name(1), "Veldin")
        self.assertEqual(RAC4.level_name(42), "Catacrom Graveyard (multiplayer)")
        self.assertEqual(RAC4.level_name(99), "Level 99")
        self.assertTrue(games.known("rac1", "pal") and games.known("rac3", "ntsc"))
        self.assertFalse(games.known("rac3", "pal") or games.known("rac9", "ntsc"))
        self.assertEqual(games.title("rac4"), "Ratchet: Deadlocked")
        self.assertEqual(games.of(object()), RAC1)

    def test_game_json_agrees(self):
        for game, versions in games.VERSIONS.items():
            catalogue = json.loads((ROOT / "games" / game / "game.json").read_text())
            self.assertEqual(sorted(catalogue["versions"]), sorted(versions), game)
        for version in wads.VERSIONS:
            catalogue = json.loads((ROOT / "games" / version.game / "game.json").read_text())
            entry = catalogue["versions"][version.key]
            self.assertEqual((entry["serial"], entry["boot"]["size"], entry["boot"]["sha1"]),
                             (version.executable, version.executable_size, version.executable_sha1))
            self.assertEqual(catalogue["title"], games.FORMATS[version.game].title)


class SwizzleTests(unittest.TestCase):
    def test_both_directions_are_one_permutation_and_its_inverse(self):
        for width, height in ((32, 4), (64, 8), (128, 32)):
            with self.subTest(size=(width, height)):
                pixels = bytes(i * 7 % 256 for i in range(width * height))
                rows = unswizzle(pixels, width)
                self.assertEqual(sorted(rows), sorted(pixels))
                self.assertEqual(unswizzle(rows, width, inverse=True), pixels)

    def test_the_first_bytes_of_a_block(self):
        # Stored bytes 0..3 spread four columns apart, the odd ones two rows down; byte 16, the
        # next run, is turned by one place (Wrench's map_pixel_index_rac4, worked by hand for a
        # 32-wide texture).
        stored = bytearray(32 * 4)
        for i in (0, 1, 2, 3, 16):
            stored[i] = 10 + i
        rows = unswizzle(bytes(stored), 32)
        self.assertEqual(rows[0], 10)
        self.assertEqual([rows.index(v) for v in (11, 12, 13, 26)], [2 * 32 + 4, 8, 2 * 32 + 12, 4])


def gc_tie(winding: int = 0, game=RAC2) -> bytearray:
    """test_ties' tie, with the class header laid out as Going Commando's on: packet count at
    0x0c, texture count at 0x0f, AD GIF records' offset at 0x1c."""
    data = test_ties.fixture()
    count, textures, gifs = data[0x20], data[0x23], struct.unpack_from("<I", data, 0x2c)[0]
    data[0x20], data[0x23] = 0, 0
    struct.pack_into("<I", data, 0x2c, 0)
    data[0x0c], data[0x0f] = count, textures
    struct.pack_into("<I", data, 0x1c, gifs)
    data[0xaf] = winding  # The first strip's last byte.
    return data


class ReaderTests(unittest.TestCase):
    def test_tie_class_header_and_winding_flag(self):
        rac1 = tie_mesh(test_ties.fixture(), test_ties.REMAP, "tie")
        rac2 = tie_mesh(gc_tie(), test_ties.REMAP, "tie", RAC2)
        self.assertEqual((rac2.positions, rac2.faces), (rac1.positions, rac1.faces))
        self.assertEqual(set(rac2.light_slots), {0})  # Not read for the later games.
        with self.assertRaises(Exception):
            tie_mesh(gc_tie(winding=32), test_ties.REMAP, "tie", RAC2)  # Going Commando: 0 or 1 only.
        turned = tie_mesh(gc_tie(winding=32), test_ties.REMAP, "tie", RAC3)
        self.assertEqual(turned.faces[("tie", 7)], [(2, 1, 0)])

    def test_tie_and_shrub_instances_of_the_later_games(self):
        data = bytearray(0x40 + 0x60 + 0x70 + 0x20)
        struct.pack_into("<I", data, RAC2.ties, 0x40)
        struct.pack_into("<I", data, 0x40, 1)
        struct.pack_into("<4i", data, 0x50, 42, 140, 0, 3)
        struct.pack_into("<16f", data, 0x60, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 5, 6, 7, 0.01)
        struct.pack_into("<2i", data, 0x50 + 0x50, 9, 1234)
        tie, = tie_instances(bytes(data), {42}, RAC2)
        self.assertEqual((tie["directional_lights"], tie["uid"], tie["ambient"]), (9, 1234, None))
        problems = []
        self.assertEqual(tie_instances(bytes(data), {43}, RAC2, problems), [])
        self.assertEqual(len(problems), 1)
        shrubs = bytearray(0x100)
        struct.pack_into("<I", shrubs, RAC4.shrubs, 0x20)
        struct.pack_into("<I", shrubs, 0x20, 1)
        struct.pack_into("<if", shrubs, 0x30, 8, 50.0)
        struct.pack_into("<16f", shrubs, 0x40, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0.01)
        shrub, = shrub_instances(bytes(shrubs), {8}, RAC4)
        self.assertEqual((shrub["class_id"], shrub["draw_distance"]), (8, 50.0))

    def test_moby_records_of_each_game(self):
        for fmt in (RAC2, RAC3, RAC4):
            with self.subTest(fmt.game):
                record = fmt.moby
                values = {name: 0 for name in record.fields}
                values.update(record.usual)
                values.update(size=record.size, class_id=500, spawn_id=7, group=-1, pvar_index=3, scale=2.0,
                              x=1.0, y=2.0, z=3.0, r=10, g=20, b=30, bolts=5)
                packed = struct.pack(record.layout, *(values[name] for name in record.fields))
                gameplay = bytearray(0x100) + struct.pack("<i12x", 1) + packed
                struct.pack_into("<I", gameplay, fmt.mobys, 0x100)
                moby, = moby_instances(bytes(gameplay), fmt)
                self.assertEqual((moby["class_id"], moby["scale"], moby["matrix"][12:15]), (500, 2.0, [1.0, 2.0, 3.0]))
                self.assertEqual(moby["fields"], {"spawn_id": 7, "group": -1, "pvar_index": 3,
                                                  "colour": [10, 20, 30], "bolts": 5})

    def test_sky_shell_header_of_up_your_arsenal_and_deadlocked(self):
        data = test_sky.fixture()
        struct.pack_into("<hh", data, 0x80, 1, 0)  # Two s16 then the shell's rotation.
        struct.pack_into("<3h", data, 0x84, 101, 200, 300)
        self.assertEqual(len(sky(bytes(data), RAC3).shells[0].faces[("sky", 0)]), 1)
        with self.assertRaises(Exception):
            sky(bytes(data))  # Read as two s32, the odd rotation makes the shell untextured.

    def test_deadlocked_skeleton_rows(self):
        data = bytearray(test_moby_anim.fixture())
        expected = bind_pose(bytes(data), 2, 0x100, 0x180)
        # The same matrices as three rows of four floats, translation in the fourth column.
        for j in range(2):
            rows = [struct.unpack_from("<4f", data, 0x100 + j * 64 + r * 16) for r in range(4)]
            packed = b"".join(struct.pack("<4f", *rows[r][:3], rows[3][r]) for r in range(3))
            data[0x300 + j * 48:0x300 + (j + 1) * 48] = packed
        # Parents by number: 0x7f for none, joint 0 with bit 7 set (Wrench's recover_moby_joints).
        struct.pack_into("<I", data, 0x18c, 0x7f)
        struct.pack_into("<I", data, 0x19c, 0x80)
        joints, rest = bind_pose(bytes(data), 2, 0x300, 0x180, three_rows=True)
        self.assertEqual(rest, expected[1])
        for mine, theirs in zip(joints, expected[0]):
            self.assertEqual(mine.parent, theirs.parent)
            for a, b in zip(mine.translation + mine.rotation, theirs.translation + theirs.rotation):
                self.assertAlmostEqual(a, b, 5)


class WriterTests(unittest.TestCase):
    def test_problems_are_summed_up_per_class(self):
        lines = ["tie class 9: not extracted (bad)", "tie instance 3: left out with its class 9",
                 "tie instance 4: left out with its class 9", "moby class 2: animations left out (x)"]
        self.assertEqual(problem_summary(lines), ["tie class 9: not extracted (bad)",
                                                  "moby class 2: animations left out (x)",
                                                  "tie class 9: 2 instance(s) left out with it"])

    def test_a_later_game_s_project_says_so(self):
        with tempfile.TemporaryDirectory() as temp:
            write_project(Path(temp) / "one", [30], "NTSC-U", "rac2")
            write_project(Path(temp) / "all", [1, 2], "NTSC-U", "rac4")
            self.assertIn('config/name="Ratchet & Clank: Going Commando: Insomniac Museum (NTSC-U, early access)"',
                          (Path(temp) / "one/project.godot").read_text())
            self.assertIn('config/name="Ratchet: Deadlocked levels (NTSC-U, early access)"',
                          (Path(temp) / "all/project.godot").read_text())


if __name__ == "__main__":
    unittest.main()
