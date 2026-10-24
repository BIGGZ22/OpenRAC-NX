"""Synthetic tie packets and placement records; no disc data."""

import math
import struct
import unittest

from formats import FormatError
from ties import tie_instances, tie_mesh

REMAP = bytes((7, 8)) + bytes(14)


def fixture():
    data = bytearray(0x120 + 160 + 16)
    struct.pack_into("<I", data, 0, 0x70)
    data[0x20], data[0x23] = 1, 2
    struct.pack_into("<I", data, 0x2c, 0x120)
    struct.pack_into("<f", data, 0x40, 2)
    struct.pack_into("<I", data, 0x70, 16)
    data[0x78], data[0x79] = 4, 6
    data[0x7a], data[0x7b] = 20, 2  # Light slots at 0x80 + 20 qwords: four regular, one extended.
    data[0x1c0:0x1c8] = bytes((1, 2, 3, 4, 5, 9, 9, 255))
    struct.pack_into("<i", data, 0x80, 16)
    struct.pack_into("<4i", data, 0x90, 0, 80, 0, 0)
    data[0xa3], data[0xa8] = 2, 12
    data[0xac:0xb4] = bytes((3, 0, 6, 0, 3, 0, 22, 0))
    # Unsorted GS writes, one vertex reused by the second strip.
    vertices = [(0, 1024, 0, 13, 0, 4096, 4096, 0),
                (0, 0, 0, 7, -4096, 0, 4096, 23),
                (1024, 0, 0, 10, 4096, 0, 4096, 0),
                (1024, 1024, 0, 26, 4096, 4096, 4096, 0)]
    for i, v in enumerate(vertices):
        struct.pack_into("<3hH3hH", data, 0xc0 + i*16, *v)
    struct.pack_into("<4H4h3hH", data, 0x100, 0, 0, 0, 29, 0, 0, 1024, 0, 0, 0, 4096, 0)
    return data


class TieTests(unittest.TestCase):
    def test_authored_slot_normals_are_retained(self):
        data = fixture()
        offset = len(data)
        struct.pack_into("<I", data, 0x0c, offset)
        data += bytes(64 * 8)
        for slot in range(64):
            struct.pack_into("<4h", data, offset + slot * 8, 0, 19660, 26213, 0)
        struct.pack_into("<4h", data, offset + 3 * 8, 32767, 0, 0, 0)
        mesh = tie_mesh(data, REMAP, "tie")
        self.assertEqual(len(mesh.normals), len(mesh.positions))
        self.assertEqual(mesh.normals[1], (1, 0, 0))
        self.assertAlmostEqual(mesh.normals[0][1], .6, places=4)
        self.assertAlmostEqual(mesh.normals[0][2], .8, places=4)

    def test_gs_order_material_change_and_repeated_vertex(self):
        mesh = tie_mesh(fixture(), REMAP, "tie_42")
        self.assertEqual(mesh.positions, [(0, 0, 0), (2, 0, 0), (0, 2, 0), (0, 0, 0), (2, 2, 0), (0, 0, 2)])
        self.assertEqual(mesh.uvs[0], (-1, 0))
        self.assertEqual(mesh.faces, {("tie", 7): [(0, 1, 2)], ("tie", 8): [(3, 4, 5)]})
        self.assertEqual(mesh.light_slots, [2, 3, 1, 2, 4, 5])

    def test_bad_materials_gs_addresses_counts_and_q(self):
        for offset, value in ((0x90, 1), (0xaf, 2), (0xac, 4), (0xc6, 12), (0xcc, 1)):
            data = fixture()
            data[offset] = value
            with self.subTest(offset=offset), self.assertRaises(FormatError):
                tie_mesh(data, REMAP, "tie")
        with self.assertRaisesRegex(FormatError, "no texture"):
            tie_mesh(fixture(), bytes((7, 255)), "tie")
        with self.assertRaises(FormatError):
            tie_mesh(fixture()[:0x105], REMAP, "tie")

    def test_padding_duplicate_and_conflict(self):
        data = fixture()
        data[0xf0:0x100] = data[0xd0:0xe0]  # A duplicate leaves address 26 unwritten.
        with self.assertRaises(FormatError):
            tie_mesh(data, REMAP, "tie")
        data = fixture()
        struct.pack_into("<H", data, 0xc6, 7)  # Two different vertices at one address.
        with self.assertRaisesRegex(FormatError, "conflicting"):
            tie_mesh(data, REMAP, "tie")

    def test_placements_keep_fields_and_stored_w(self):
        data = bytearray(0x60 + 0xe0)
        struct.pack_into("<I", data, 0x34, 0x50)
        struct.pack_into("<I", data, 0x50, 1)
        struct.pack_into("<4i", data, 0x60, 42, 140, 0, 3)
        matrix = [0, .5, 0, 0, -.5, 0, 0, 0, 0, 0, .5, 0, 10, 20, 30, .01]
        struct.pack_into("<16f", data, 0x70, *matrix)
        struct.pack_into("<2i", data, 0x60 + 0xd0, 9, 1234)
        instance, = tie_instances(data, {42})
        self.assertEqual(instance["matrix"], matrix[:15] + [1])
        self.assertAlmostEqual(instance["stored_w"], .01)
        self.assertEqual((instance["draw_distance"], instance["occlusion_index"],
                          instance["directional_lights"], instance["uid"]), (140, 3, 9, 1234))
        with self.assertRaisesRegex(FormatError, "unknown class"):
            tie_instances(data, {43})
        with self.assertRaises(FormatError):
            tie_instances(data[:-1], {42})
        struct.pack_into("<f", data, 0x70, math.nan)
        with self.assertRaisesRegex(FormatError, "matrix"):
            tie_instances(data, {42})


if __name__ == "__main__":
    unittest.main()
