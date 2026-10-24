"""Hand-built VIF and geometry fixtures; no disc data."""

import struct
import unittest
from unittest import mock

from formats import FormatError
from gltf import Gltf
from terrain import packets, strip_faces, terrain
from test_godot import attribute


def command(code, immediate=0, data=b"", count=0):
    return struct.pack("<I", code << 24 | count << 16 | immediate) + data + bytes(-len(data) % 4)


def fixture():
    # A four-corner plane with a repeated seam vertex and translated origin.
    vu = [0] * 20
    vu[0], vu[6], vu[7], vu[13], vu[18] = 4, 24, 40, 60, 64
    low = (command(0x30, data=struct.pack("<4I", 40, 40, 40, 40))
           + command(5, 1)
           + command(0x6e, 0xc03c, bytes((0, 1, 2, 3)), 1)
           + command(5)
           + command(0x6e, 0x8040, bytes((132, 0, 0, 0, 0, 0, 0, 0)), 2))
    textures = bytes(80)  # material 0
    infos = [(0, 0, 4096, 0), (4096, 0, 4096, 2), (0, 4096, 4096, 4),
             (4096, 4096, 4096, 6), (-4096, 8192, 4096, 0)]
    positions = [(0, 0, 0), (1024, 0, 0), (0, 1024, 0), (1024, 1024, 0)]
    common = (command(0x6d, 0xc000, struct.pack("<20H", *vu), 5)
              + command(0x6c, 0x8009, textures, 5)
              + command(0x30, data=struct.pack("<4I", 0x45000000, 0x45000000, 0, 24))
              + command(5, 1)
              + command(0x6d, 0x8028, b"".join(struct.pack("<4h", *i) for i in infos), 5)
              + command(0x30, data=struct.pack("<4i", 2048, -1024, 3072, 0))
              + command(1, 0x102)
              + command(0x69, 0x8018, b"".join(struct.pack("<3h", *p) for p in positions), 4)
              + command(1, 0x404) + command(5))
    data = bytearray(128) + low + common
    struct.pack_into("<II", data, 0, 64, 1)
    struct.pack_into("<4f", data, 64, 2.5, -0.5, 3, 1)
    struct.pack_into("<I3H", data, 80, 64, 0, len(low), len(low) + len(common))
    data[64 + 0x28] = 1
    return data


def fine_fixture(*, empty=False, migration=False):
    data = fixture()
    shared, high = struct.unpack_from("<2H", data, 64 + 0x16)
    vu = list(struct.unpack_from("<20H", data, 128 + shared + 4))
    vu[2], vu[4] = (0, 0) if empty else (1, 1)
    vu[9], vu[11], vu[14], vu[15], vu[16], vu[17] = 45, 46, 70, 71, 72, 73
    struct.pack_into("<20H", data, 128 + shared + 4, *vu)
    index_row = command(0x30, data=struct.pack("<4i", 40, 40, 40, 40))
    info_row = command(0x30, data=struct.pack("<4i", 0x45000000, 0x45000000, 24, 24))
    origin = command(0x30, data=struct.pack("<4i", 2048, -1024, 3072, 0))
    ref = b""
    if not empty:
        ref = index_row + command(5, 1) + command(0x6e, 0xc046, bytes(4), 1)
        if migration:
            ref += command(0x6e, 0xc047, bytes(4), 1)
        ref += info_row + command(0x6d, 0x802d, struct.pack("<4h", 2048, 0, 2, 8), 1)
    ref += origin + command(1, 0x102)
    if not empty:
        ref += command(0x69, 0x8020, struct.pack("<3h", 512, 0, 512), 1)
    # LOD-1 byte size is counted from the shared program, in qwords.
    ref += bytes(-(high - shared + len(ref)) % 16)
    fine = b"" if empty else command(0x69, 0x8022, struct.pack("<3h", 512, 512, 1024), 1)
    fine += command(5) + command(1, 0x404)
    fine += command(0x6e, 0x8040, bytes((131, 0, 0, 0, 0, 0, 0, 0)), 2)
    fine += index_row + command(5, 1)
    fine += command(0x6e, 0xc03c, bytes((0, 1, 2, 0) if empty else (0, 5, 6, 0)), 1)
    if not empty:
        fine += command(0x6e, 0xc048, bytes((2, 0, 0, 0)), 1)
        if migration:
            fine += command(0x6e, 0xc049, bytes(4), 1)
        fine += info_row + command(0x6d, 0x802e, struct.pack("<4h", 2048, 2048, 8, 10), 1)
    fine += command(5)
    fine += bytes(-(len(ref) + len(fine)) % 16)
    struct.pack_into("<H", data, 64 + 0x1a, high)
    struct.pack_into("<H", data, 64 + 0x1e, high + len(ref) + len(fine))
    data[64 + 0x22] = (high - shared + len(ref)) // 16
    data[64 + 0x23] = (len(ref) + len(fine)) // 16
    return data + ref + fine


class TerrainTests(unittest.TestCase):
    def test_authored_normals_follow_positions_across_uv_seams(self):
        data = fixture()
        offset = len(data) - 128
        struct.pack_into("<H", data, 64 + 0x30, offset)
        data[64 + 0x3c] = 4
        angles = [(0, 32), (32, 32), (64, 32), (96, 32)]
        data += bytes(16) + b"".join(struct.pack("<HBBHH", 0, a, e, 0, 0) for a, e in angles)
        with mock.patch("terrain.lighting.light_records", return_value=[(128, 128, 128, 128)] * 4):
            mesh, = terrain(data, 2, lights=([], []))
        self.assertEqual(len(mesh.normals), len(mesh.positions))
        self.assertEqual(mesh.normals[0], mesh.normals[4])
        self.assertNotEqual(mesh.normals[0], mesh.normals[1])
        for normal in mesh.normals:
            self.assertAlmostEqual(sum(component * component for component in normal), 1)
        self.assertAlmostEqual(mesh.normals[0][2], 2 ** -0.5)
        writer = Gltf()
        writer.mesh(mesh, {("terrain", 0): 0})
        primitive = writer.doc["meshes"][0]["primitives"][0]
        exported = attribute(writer.doc, writer.buffer, primitive["attributes"]["NORMAL"])
        self.assertEqual(len(exported), 4)
        self.assertIn("indices", primitive)
        self.assertAlmostEqual(exported[0][2], 2 ** -0.5)

    def test_fine_positions_uvs_and_faces(self):
        for migration in (False, True):
            mesh, = terrain(fine_fixture(migration=migration), 0)
            self.assertEqual(mesh.name, "Terrain_000")
            self.assertIn((2.5, -1, 3.5), mesh.positions)
            self.assertIn((2.5, -0.5, 4), mesh.positions)
            self.assertIn((0.5, 0.5), mesh.uvs)
            self.assertEqual(mesh.faces, {("terrain", 0): [(0, 5, 6)]})
            self.assertEqual(len(mesh.positions), 7)
            self.assertEqual(mesh.bounds(), ((2, -1, 3), (3, 0, 4)))

    def test_fine_without_refinements_and_coarse_compatibility(self):
        mesh, = terrain(fine_fixture(empty=True), 0)
        self.assertEqual(len(mesh.positions), 5)
        self.assertEqual(mesh.faces, {("terrain", 0): [(0, 1, 2)]})
        self.assertEqual(terrain(fine_fixture(), 2), terrain(fixture(), 2))

    def test_bad_fine_parents_addresses_and_bounds(self):
        original = fine_fixture()
        begin, = struct.unpack_from("<H", original, 64 + 0x1a)
        parent_packet = 128 + begin + 24
        info_packet = parent_packet + 8 + 20
        # Bad parent-info reference, wrong VU destination, and bad stage size.
        for offset, value in ((parent_packet + 4, 99), (parent_packet, 1),
                              (info_packet + 8, 99), (info_packet + 10, 0),
                              (64 + 0x22, 0), (64 + 0x23, 0)):
            data = bytearray(original)
            data[offset] = value
            with self.subTest(offset=offset), self.assertRaises(FormatError):
                terrain(data, 0)
        for data in (original[:-8], original[:128]):
            with self.assertRaises(FormatError):
                terrain(data, 0)
        with self.assertRaisesRegex(FormatError, "LOD must"):
            terrain(original, 1)

    def test_transformed_textured_plane(self):
        mesh, = terrain(fixture(), 2)
        self.assertEqual(mesh.positions[:2], [(2, -1, 3), (3, -1, 3)])
        self.assertEqual(mesh.uvs[0], (0, 0))
        self.assertEqual(mesh.uvs[4], (-0.5, 2))  # Negative coordinates are halved.
        self.assertEqual(mesh.faces, {("terrain", 0): [(2, 1, 0), (2, 3, 1)]})
        self.assertEqual(mesh.bounds(), ((2, -1, 3), (3, 0, 3)))

    def test_strip_material_changes_continuation_and_winding(self):
        strips = struct.pack("<16b", -124, 0, 5, 0, 3, 0, 0, 0,
                             -125, 0, 0, 0, 0, 0, 0, 0)
        faces = strip_faces(bytes(range(10)) + bytes(2), strips, 2)
        self.assertEqual(faces, [(1, (2, 1, 0)), (1, (2, 3, 1)),
                                 (1, (4, 5, 6)), (0, (7, 8, 9))])

    def test_one_mesh_per_fragment(self):
        original = fixture()
        header = bytearray(original[64:128])
        struct.pack_into("<I", header, 16, 128)
        data = original[:64] + header + header + original[128:]
        struct.pack_into("<I", data, 4, 2)
        meshes = terrain(data, 2)
        self.assertEqual([m.name for m in meshes], ["Terrain_000", "Terrain_001"])
        self.assertEqual((meshes[0].positions, meshes[0].uvs, meshes[0].faces),
                         (meshes[1].positions, meshes[1].uvs, meshes[1].faces))

    def test_bad_indices_and_program(self):
        data = fixture()
        data[128 + 20 + 4 + 4] = 99  # First index, after STROW and STMOD.
        with self.assertRaisesRegex(FormatError, "face index"):
            terrain(data, 2)
        data = fixture()
        data[128 + 20] = 2  # STMOD difference mode.
        with self.assertRaisesRegex(FormatError, "cycle or mode"):
            terrain(data, 2)

    def test_truncated_and_unsupported_packets(self):
        for data in (b"\0", command(0x69, 0x8000, bytes(4), 1), command(0x20),
                     command(0x79), command(0xec)):
            with self.subTest(data=data[:4]), self.assertRaises(FormatError):
                packets(data)
        # V3-16 is six bytes plus alignment; NUM = 0 means 256 elements.
        self.assertEqual(len(packets(command(0x69, 0x8000, bytes(6), 1))[0].data), 6)
        self.assertEqual(packets(command(0x6e, 0x8000, bytes(1024)))[0].count, 256)

    def test_reject_bad_strips(self):
        for strips, indices in ((bytes((4, 0, 0, 0, 0, 0, 0, 0)), bytes(4)),
                                (bytes((133, 0, 0, 0, 0, 0, 0, 0)), bytes(5)),
                                (bytes((132, 0, 1, 0, 0, 0, 0, 0)), bytes(4)),
                                (bytes((132, 0, 0, 0)), bytes(4)),
                                (bytes((132, 0, 0, 0, 0, 0, 0, 0)), bytes(3))):
            with self.subTest(strips=strips), self.assertRaises(FormatError):
                strip_faces(indices, strips, 1)


if __name__ == "__main__":
    unittest.main()
