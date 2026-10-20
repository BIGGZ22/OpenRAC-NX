"""Synthetic moby class blobs, their GLB and class scene; no disc data."""

from pathlib import Path
import struct
import tempfile
import unittest

from formats import FormatError
import games
from gltf import Gltf
from godot import write_marker
from mesh import Mesh
from moby_class import UNTEXTURED, moby_class
from test_godot import attribute, read_glb

REMAP = bytes((5, 9)) + bytes((255,)) * 14
UP = (0, 64)  # Normal angles: azimuth 0, elevation a quarter turn (+z).


def command(code, immediate, data, count):
    return struct.pack("<I", code << 24 | count << 16 | immediate) + data + bytes(-len(data) % 4)


def vif(sts, header, indices, blocks=b""):
    """A packet's VIF list: texture coordinates, index stream, GS blocks."""
    stream = header + bytes(indices)
    stream += bytes(-len(stream) % 4)
    data = command(0x75, 0x80c2, b"".join(struct.pack("<2h", *st) for st in sts), len(sts))
    data += command(0x6e, 0x812d, stream, len(stream) // 4)
    if blocks:
        data += command(0x6c, 0x812d + len(stream) // 4, blocks, len(blocks) // 16)
    return data + bytes(-len(data) % 16)


def record(skin, position, normal=UP):
    return bytes(skin) + bytes(normal) + struct.pack("<3h", *position)


def table(transfers, counts, copies, records, ids, compact=False):
    """A RAC1 vertex table: header, transfers, duplicates, vertices, one
    epilogue record carrying the IDs and the RGBA multipliers. COMPACT: the
    later games' (games.Format.compact_vertex_table): a header of eight u16
    and no multipliers after the epilogue."""
    total = sum(counts) + len(copies)
    size = 0x10 if compact else 0x20
    body = bytes(t for pair in transfers for t in pair)
    at = size + len(body)
    at += 2 if at % 4 else 0
    at += 4 if at % 8 else 0
    body += bytes(at - size - len(body)) + struct.pack(f"<{len(copies)}H", *copies)
    body += bytes(-(size + len(body)) % 16)
    vertices = size + len(body)
    epilogue = bytes(4) + struct.pack("<6H", *(ids + [0] * (6 - len(ids))))
    multipliers = vertices + (len(records) + 1) * 16
    if compact:
        return struct.pack("<8H", len(transfers), *counts, len(copies), total, vertices, 0) + body + \
            b"".join(records) + epilogue, total
    data = struct.pack("<8I", len(transfers), *counts, len(copies), total, vertices, multipliers)
    data += body + b"".join(records) + epilogue + bytes([0x80] * (total * 4)) + bytes(-total * 4 % 16)
    return data, total


def entry(list_offset, list_data, table_offset, table_data, total):
    return struct.pack("<IHHIBBBB", list_offset, len(list_data) // 16, 0, table_offset, len(table_data) // 16,
                       (total * 6 + 15) // 16, (total + 3) // 4, total)


def fixture(texture=1, compact=False):
    """Two packets. The first switches to texture slot 1 and draws three
    triangles, the last through a duplicate vertex; the second inherits the
    texture, blends joints with matrix slots left by the first and copies
    one of the first packet's vertices out of the cache."""
    block = bytearray(64)
    struct.pack_into("<i", block, 0x20, texture)
    list0 = vif([(0, 0), (4096, 0), (0, 4096), (4096, 4096), (8192, 0)], bytes((0xfe, 4, 0x81, 0)),
                [0, 0x82, 3, 4, 5, 1, 1, 1, 0], bytes(block))
    single = (0, 0, 0, 0xf4, 0, 0, 0, 0)  # Joint 0 from VU0 address 0, nothing stored.
    table0, total0 = table([(0, 0)], (0, 0, 4), [11 << 7],
                           [record(single, p) for p in ((0, 0, 0), (1024, 0, 0), (0, 1024, 0), (1024, 1024, 0))],
                           [10, 11, 12, 13], compact)
    list1 = vif([(0, 0), (4096, 0), (0, 4096), (4096, 4096)], bytes((0xfe, 0, 0, 0)), [0x81, 0x82, 3, 4, 1, 1, 1, 0])
    two = (0, 5 << 1, 4, 8, 64, 192, 12, 16)      # Keep joint 5 at 12; blend 4 and 8 into 16.
    three = (0, 12, 0, 4, 100, 100, 56, 0xf4)     # Blend 0, 4 and 12.
    reuse = (0, 3 << 1, 16, 0xf4, 0, 0, 0, 0)     # Load the blend at 16.
    table1, total1 = table([(2, 4), (3, 8)], (1, 1, 1), [13 << 7],
                           [record(two, (2048, 0, 0)), record(three, (3072, 0, 0)), record(reuse, (2048, 1024, 0))],
                           [20, 21, 22], compact)
    data = bytearray(0x400)
    struct.pack_into("<i4B4B", data, 0, 0x50, 2, 0, 0, 2, 6, 6, 0xff, 0xff)
    struct.pack_into("<f", data, 0x24, 2.0)
    data[0x50:0x60] = entry(0x80, list0, 0x100, table0, total0)
    data[0x60:0x70] = entry(0x200, list1, 0x280, table1, total1)
    data[0x80:0x80 + len(list0)] = list0
    data[0x100:0x100 + len(table0)] = table0
    data[0x200:0x200 + len(list1)] = list1
    data[0x280:0x280 + len(table1)] = table1
    if compact:
        data[0xb] = 0  # Going Commando reads its compact tables only when this byte is clear.
    return data


class MobyClassTests(unittest.TestCase):
    def test_packets_duplicates_texture_carry_and_winding(self):
        moby = moby_class(fixture(), REMAP, "moby_42")
        self.assertEqual((moby.scale, moby.joint_count), (2.0, 6))
        mesh = moby.mesh
        self.assertEqual(mesh.positions, [(0, 0, 0), (1, 0, 0), (0, 1, 0), (1, 1, 0), (1, 0, 0),
                                          (2, 0, 0), (3, 0, 0), (2, 1, 0), (1, 1, 0)])
        self.assertEqual(mesh.uvs[4], (2, 0))
        for value, expected in zip(mesh.normals[0], (0, 0, 1)):
            self.assertAlmostEqual(value, expected)
        # Every face turns towards +z, the stored normals.
        self.assertEqual(mesh.faces, {("moby", 9): [(0, 1, 2), (3, 2, 1), (4, 3, 2), (5, 6, 7), (6, 7, 8)]})

    def test_skins_resolve_vu0_slots_across_packets(self):
        skins = moby_class(fixture(), REMAP, "moby").skins
        self.assertEqual(skins[:5], [((0, 256),)] * 5)
        self.assertEqual(skins[5:], [((2, 64), (3, 192)), ((0, 100), (2, 100), (5, 56)),
                                     ((2, 64), (3, 192)), ((0, 256),)])

    def test_untextured_and_meshless_classes(self):
        self.assertEqual(set(moby_class(fixture(texture=-1), REMAP, "moby").mesh.faces), {UNTEXTURED})
        data = fixture()
        struct.pack_into("<i", data, 0, 0)
        moby = moby_class(data, REMAP, "moby")
        self.assertIsNone(moby.mesh)
        self.assertEqual(moby.scale, 2.0)

    def test_reject_bad_textures_programs_and_vertices(self):
        for texture in (-2, 2, 16):  # Chrome, an empty slot, past the slots.
            with self.subTest(texture=texture), self.assertRaises(FormatError):
                moby_class(fixture(texture=texture), REMAP, "moby")
        cases = {
            0x24: struct.pack("<f", 0.0),          # Scale.
            0x80: struct.pack("<H", 0x80c3),      # Texture coordinate address.
            0xa5: bytes((2,)),                    # Flush trailer.
            0xa8: bytes((5,)),                    # An index after the trailer.
            0x128: struct.pack("<H", 14 << 7),    # Duplicate of an unwritten cache slot.
            0x11c: struct.pack("<I", 0x70),       # No epilogue record.
            0x2b0 + 2: bytes((20,)),              # Two-way vertex loads an unwritten VU0 slot.
            0x2b0 + 16 + 6: bytes((57,)),         # Three-way weights sum to 257.
            0x5d: bytes((9,)),                    # Packet entry sizes.
            0x114: struct.pack("<I", 6),          # Vertex table total disagrees with its counts.
        }
        for offset, value in cases.items():
            data = fixture()
            data[offset:offset + len(value)] = value
            with self.subTest(offset=hex(offset)), self.assertRaises(FormatError):
                moby_class(data, REMAP, "moby")
        data = fixture()
        data[0x08] = 5  # Joint 5 is past a joint count of 5.
        with self.assertRaisesRegex(FormatError, "joint"):
            moby_class(data, REMAP, "moby")


class LaterGameTests(unittest.TestCase):
    def test_compact_vertex_tables_read_as_the_first_game_s(self):
        rac1 = moby_class(fixture(), REMAP, "moby")
        for fmt in (games.RAC2, games.RAC3, games.RAC4):
            with self.subTest(fmt.game):
                later = moby_class(fixture(compact=True), REMAP, "moby", fmt=fmt)
                self.assertEqual((later.mesh.positions, later.mesh.faces, later.skins),
                                 (rac1.mesh.positions, rac1.mesh.faces, rac1.skins))
                self.assertIsNone(later.mesh.colours)  # No colour multipliers to read.
        # Going Commando: a class whose byte 0xb is set keeps the first game's tables.
        data = fixture()
        data[0xb] = 1
        self.assertEqual(moby_class(data, REMAP, "moby", fmt=games.RAC2).mesh.positions, rac1.mesh.positions)
        with self.assertRaises(FormatError):
            moby_class(fixture(), REMAP, "moby", fmt=games.RAC3)

    def test_an_empty_texture_slot_is_drawn_untextured(self):
        with self.assertRaises(FormatError):
            moby_class(fixture(texture=2), REMAP, "moby")
        moby = moby_class(fixture(texture=2, compact=True), REMAP, "moby", fmt=games.RAC3)
        self.assertEqual(set(moby.mesh.faces), {UNTEXTURED})

    def test_without_joints(self):
        moby = moby_class(fixture(compact=True), REMAP, "moby", fmt=games.RAC2, joints=False)
        self.assertIsNone(moby.skeleton)
        self.assertIsNone(moby.mesh.skins)


class MobySceneTests(unittest.TestCase):
    def test_vertex_normals_make_an_indexed_glb(self):
        mesh = Mesh("moby_1", [(0, 0, 0), (1, 0, 0), (0, 1, 0), (5, 5, 5)], [(0, 0), (1, 0), (0, 1), (0, 0)],
                    normals=[(0, 0, 1), (0, 1, 0), (1, 0, 0), (0, 0, 1)])
        mesh.add_face(("moby", 2), (2, 0, 1))
        mesh.add_face(("moby", 2), (0, 1, 2))
        gltf = Gltf()
        gltf.node("moby_1", gltf.mesh(mesh, {("moby", 2): gltf.material("moby_0002", "m.png")}))
        doc, binary = read_glb(gltf.glb())
        primitive = doc["meshes"][0]["primitives"][0]
        # Vertices in order of first use; vertex 3 is unused and left out.
        self.assertEqual(attribute(doc, binary, primitive["attributes"]["POSITION"]), [(0, 1, 0), (0, 0, 0), (1, 0, 0)])
        self.assertEqual(attribute(doc, binary, primitive["attributes"]["NORMAL"]), [(1, 0, 0), (0, 0, 1), (0, 1, 0)])
        accessor = doc["accessors"][primitive["indices"]]
        view = doc["bufferViews"][accessor["bufferView"]]
        self.assertEqual((accessor["componentType"], accessor["type"], view["target"]), (5125, "SCALAR", 34963))
        self.assertEqual(struct.unpack_from("<6I", binary, view["byteOffset"]), (0, 1, 2, 1, 2, 0))

    def test_class_scene_with_model(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / "moby_11.tscn"
            write_marker(path, 11, "11 vendor", ("res://levels/level_03/mobys/moby_11.glb", 0.5, 1.25))
            text = path.read_text()
        self.assertIn('[ext_resource type="PackedScene" path="res://levels/level_03/mobys/moby_11.glb" id="model"]',
                      text)
        self.assertIn('[node name="Model" parent="." instance=ExtResource("model")]\n'
                      "transform = Transform3D(0.5, 0, 0, 0, 0.5, 0, 0, 0, 0.5, 0, 0, 0)", text)
        self.assertIn("transform = Transform3D(1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1.65)", text)
        self.assertNotIn("Marker", text)


if __name__ == "__main__":
    unittest.main()
