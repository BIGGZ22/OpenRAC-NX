"""Synthetic shrub packets and placement records; no disc data."""

import math
import struct
import unittest

from formats import FormatError
from shrubs import billboard_fade, shrub_instances, shrub_mesh


def command(code, immediate=0, data=b"", count=0):
    return struct.pack("<I", code << 24 | count << 16 | immediate) + data + bytes(-len(data) % 4)


def fixture(*, strip=False, padded=False):
    """One packet: a material, a GIF tag and a triangle (or a four-vertex strip).

    GS addresses: material 0..4, tag 5, vertices from 6 in steps of 3.
    padded repeats the last vertex three times, as short retail packets do.
    """
    vertices = [(0, 0, 0), (1024, 0, 0), (0, 1024, 0)] + ([(1024, 1024, 0)] if strip else [])
    count = len(vertices) + (3 if padded else 0)
    tag = len(vertices) | 1 << 46 | (4 if strip else 3) << 47 | 3 << 60
    material = bytes(64)  # GS address (+12) 0 and TEX0 slot (+48) 0.
    control = struct.pack("<4i", 1, 1, count, 6) + struct.pack("<QII", tag, 0x412, 5) + material
    positions = b"".join(struct.pack("<4h", *v, 6 + i * 3) for i, v in enumerate(vertices))
    uvs = b"".join(struct.pack("<4h", 4096 * (i % 2), 4096 * (i > 1), 4096, 0) for i in range(len(vertices)))
    if padded:
        positions += positions[-8:] * 3
        uvs += uvs[-8:] * 3
    stream = (command(1, 0x404) + command(5) + command(0x6c, 0x8000, control, 6) + command(5)
              + command(0x6d, 0x8006, positions, count) + command(5)
              + command(0x6d, 0x8006 + count, uvs, count))
    data = bytearray(0x200 + 24 * 8)
    struct.pack_into("<f", data, 0x20, 2)            # Scale.
    struct.pack_into("<h", data, 0x28, 1)            # Packet count.
    struct.pack_into("<I", data, 0x2c, 0x200)        # Normals.
    struct.pack_into("<ii", data, 64, 0x80, len(stream))
    data[0x80:0x80 + len(stream)] = stream
    struct.pack_into("<4h", data, 0x200, 0, 0, 32767, 0)  # Normal 0 faces +Z.
    return data


class ShrubTests(unittest.TestCase):
    def test_billboard_fade_truncates_to_a_byte(self):
        data = fixture()
        self.assertEqual(billboard_fade(data), -1)  # no billboard record
        struct.pack_into("<i", data, 0x1c, 0x100)
        for fade, byte in ((40.7, 40), (256.0, 0), (300.5, 44), (math.inf, -1)):
            struct.pack_into("<f", data, 0x100, fade)
            self.assertEqual(billboard_fade(data), byte)

    def test_triangle_list_scale_texture_and_padding(self):
        for padded in (False, True):
            mesh = shrub_mesh(fixture(padded=padded), bytes((5,)), "shrub_42")
            self.assertEqual(mesh.normals, [(0, 0, 1)] * 3)
            self.assertEqual(mesh.positions, [(0, 0, 0), (2, 0, 0), (0, 2, 0)])
            self.assertEqual(mesh.faces, {("shrub", 5): [(0, 1, 2)]})

    def test_triangle_strip_uses_stored_normals_for_winding(self):
        mesh = shrub_mesh(fixture(strip=True), bytes((5,)), "shrub")
        self.assertEqual(len(mesh.positions), 4)
        self.assertEqual(mesh.faces, {("shrub", 5): [(0, 1, 2), (3, 2, 1)]})

    def test_reject_bad_references_programs_and_materials(self):
        original = fixture()
        for offset, value in ((0x28, 2), (0x114 + 6, 25), (0x80 + 8, 1)):
            data = bytearray(original)
            data[offset] = value
            with self.subTest(offset=offset), self.assertRaises(FormatError):
                shrub_mesh(data, bytes((5,)), "shrub")
        with self.assertRaisesRegex(FormatError, "no texture"):
            shrub_mesh(original, bytes((255,)), "shrub")
        with self.assertRaises(FormatError):
            shrub_mesh(original[:0x202], bytes((5,)), "shrub")
        data = bytearray(original)
        tag, = struct.unpack_from("<Q", data, 0x80 + 32)
        struct.pack_into("<Q", data, 0x80 + 32, (tag & ~(7 << 47)) | (2 << 47))  # GS lines.
        with self.assertRaisesRegex(FormatError, "primitive"):
            shrub_mesh(data, bytes((5,)), "shrub")

    def test_placements_keep_matrix_colour_and_lights(self):
        data = bytearray(0x50 + 16 + 0x70)
        struct.pack_into("<I", data, 0x3c, 0x50)
        struct.pack_into("<I", data, 0x50, 1)
        struct.pack_into("<if", data, 0x60, 42, 50)
        struct.pack_into("<16f", data, 0x70, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 10, 20, 30, .01)
        struct.pack_into("<3i", data, 0xb0, 255, 128, 0)
        struct.pack_into("<i", data, 0xc0, 7)
        instance, = shrub_instances(data, {42})
        self.assertEqual(instance["matrix"][12:], [10, 20, 30, 1])
        self.assertAlmostEqual(instance["stored_w"], .01)
        self.assertEqual((instance["draw_distance"], instance["colour"], instance["directional_lights"]),
                         (50, [255, 128, 0], 7))
        with self.assertRaisesRegex(FormatError, "unknown class"):
            shrub_instances(data, {99})
        struct.pack_into("<f", data, 0x70, math.nan)
        with self.assertRaisesRegex(FormatError, "matrix"):
            shrub_instances(data, {42})


if __name__ == "__main__":
    unittest.main()
