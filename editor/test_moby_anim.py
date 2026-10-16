"""Synthetic moby skeletons and sequences, and their GLB; no disc data."""

import math
from pathlib import Path
import struct
import tempfile
import unittest

from formats import FormatError, Texture
from godot import LevelWriter
from mesh import Mesh
from moby_anim import class_sequences, decompose, skeleton
from moby_class import MobyClass
from test_godot import read_glb

H = math.sqrt(0.5)


def frame(rate, time, quats, scales=(), translations=()):
    """A frame: header, quaternions (game convention), scale and translation records."""
    payload = b"".join(struct.pack("<4h", *q) for q in quats)
    payload += b"".join(struct.pack("<3H2B", *s) for s in scales)
    payload += b"".join(struct.pack("<3h2B", *t, 0) for t in translations)
    payload += bytes(-len(payload) % 16)
    quat_bytes = 8 * len(quats)
    translations_at = quat_bytes + 8 * len(scales)
    qwc = (translations_at + 8 * len(translations) + 15) >> 4
    return struct.pack("<fh5H", rate, time, qwc, quat_bytes, len(scales), translations_at, len(translations)) + payload


def fixture(joint_two_parent=0x70000000):
    """Two joints: 0 at z = 1, and 1 one unit along x from it, turned 90
    degrees about z. Sequence 0 loops at a rate override of 0.5: the bind
    pose, then joint 0 turned 90 degrees about x and raised to z = 2 with
    joint 1 scaled by 2 after the chain. Sequence 1 holds the bind pose and
    stops on its last frame."""
    data = bytearray(0x400)
    data[8], data[0xc] = 2, 2
    struct.pack_into("<2i", data, 0x14, 0x100, 0x180)
    struct.pack_into("<2i", data, 0x48, 0x200, 0x280)
    # Inverse bind matrices, rows: S0 = [I | (0, 0, -1024)], S1 = (P0 * L1)^-1.
    struct.pack_into("<16f", data, 0x100, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, -1024, 1)
    struct.pack_into("<16f", data, 0x140, 0, -1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1024, -1024, 1)
    struct.pack_into("<3fI", data, 0x180, 0, 0, 1024, 0)
    struct.pack_into("<3fI", data, 0x190, 1024, 0, 0, joint_two_parent)
    bind = [(0, 0, 0, 32767), (0, 0, -23170, 23170)]  # Game quaternions: conjugates of glTF's.
    frames = [frame(0.0, 0, bind),
              frame(0.0, 16, [(-23170, 0, 0, 23170), bind[1]], scales=[(8192, 8192, 8192, 1, 0)],
                    translations=[(0, 0, 2048, 0)])]
    still = [frame(0.25, 0, bind), frame(0.0, 32, bind)]
    for at, rate, items in ((0x200, 0.5, frames), (0x280, 0.0, still)):
        struct.pack_into("<B", data, at + 0x10, len(items))
        struct.pack_into("<f", data, at + 0x18, rate)
        cursor = at + 0x30
        for k, item in enumerate(items):
            struct.pack_into("<I", data, at + 0x1c + 4 * k, cursor)
            data[cursor:cursor + len(item)] = item
            cursor += len(item)
    return bytes(data)


def close(a, b, places=4):
    return all(abs(x - y) < 10 ** -places for x, y in zip(a, b))


class SkeletonTests(unittest.TestCase):
    def test_bind_pose_from_inverse_bind_matrices(self):
        bones = skeleton(fixture(), class_sequences(fixture()))
        first, second = bones.joints
        self.assertEqual((first.parent, second.parent), (None, 0))
        self.assertTrue(close(first.translation, (0, 0, 1)) and close(first.rotation, (0, 0, 0, 1)))
        self.assertTrue(close(second.translation, (1, 0, 0)) and close(second.rotation, (0, 0, H, H)))
        self.assertTrue(close(second.scale, (1, 1, 1)))
        self.assertEqual(second.inverse_bind[12:], [0, 1, -1, 1])
        self.assertTrue(second.post_scale and not first.post_scale)

    def test_collapsed_joint(self):
        # A joint scaled to nothing on one axis or all: kept, with no rotation of its own.
        translation, rotation, scale = decompose([[0, 0, 0], [0, 2, 0], [0, 0, 0]], [1, 2, 3])
        self.assertEqual((translation, rotation, scale), ((1, 2, 3), (0.0, 0.0, 0.0, 1.0), (0.0, 2.0, 0.0)))

    def test_keys_times_loop_and_channels(self):
        moving, still = skeleton(fixture(), class_sequences(fixture())).animations
        self.assertEqual((moving.name, moving.loop), ("seq_00", True))
        self.assertTrue(close(moving.times, (0, 2 / 60, 4 / 60), 9))
        self.assertEqual(sorted(moving.tracks), [(0, "rotation"), (0, "translation"), (1, "post_scale")])
        rotation = moving.tracks[(0, "rotation")]
        self.assertTrue(close(rotation[1], (H, 0, 0, H)) and close(rotation[2], (0, 0, 0, 1)))
        self.assertEqual(moving.tracks[(0, "translation")], [(0, 0, 1), (0, 0, 2), (0, 0, 1)])
        self.assertEqual(moving.tracks[(1, "post_scale")], [(1, 1, 1), (2, 2, 2), (1, 1, 1)])
        # A rate of 0 on the last frame stops there; every key is the bind
        # pose, so joint 0 is held still to give glTF a channel.
        self.assertEqual((still.loop, still.times), (False, [0, 4 / 60]))
        self.assertEqual(list(still.tracks), [(0, "rotation")])

    def test_no_skeleton_and_bad_layouts(self):
        data = bytearray(fixture())
        struct.pack_into("<i", data, 0x14, 0)
        self.assertIsNone(skeleton(bytes(data), class_sequences(bytes(data))))
        # No sequences: still a skeleton (the port poses it from the game's memory), no animations.
        bare = skeleton(fixture(), [])
        self.assertIsNotNone(bare)
        self.assertEqual(bare.animations, [])
        with self.assertRaisesRegex(FormatError, "parent"):
            skeleton(fixture(0x70000040), class_sequences(fixture()))  # Joint 1 as its own parent.
        data = bytearray(fixture())
        data[0x8] = 1  # A frame holds two quaternions for one joint.
        with self.assertRaisesRegex(FormatError, "frame"):
            skeleton(bytes(data), class_sequences(bytes(data)))


class FakeLevel:
    id = 0
    overlay = {"entry_point": 0, "sections": []}

    @staticmethod
    def texture(group, index):
        return Texture(1, 1, bytes(1), bytes(1024))


class SkinnedGlbTests(unittest.TestCase):
    def test_joints_skin_post_node_and_animations(self):
        data = fixture()
        mesh = Mesh("moby_5", [(0, 0, 0), (1, 0, 1), (0, 1, 1)], [(0, 0)] * 3, normals=[(0, 0, 1)] * 3,
                    skins=[((0, 256),), ((1, 256),), ((0, 128), (1, 128))])
        mesh.add_face(("moby", 3), (0, 1, 2))
        moby = MobyClass(0.5, mesh, 2, [], skeleton(data, class_sequences(data)))
        with tempfile.TemporaryDirectory() as tmp:
            writer = LevelWriter(Path(tmp), FakeLevel())
            writer.moby_glb("mobys/moby_5.glb", moby)
            doc, binary = read_glb((Path(tmp) / "levels/level_00/mobys/moby_5.glb").read_bytes())
        names = [node["name"] for node in doc["nodes"]]
        self.assertEqual(names, ["joint_000", "joint_001", "joint_001_post", "moby_5"])
        self.assertEqual(doc["scenes"][0]["nodes"], [0, 3])
        self.assertEqual((doc["nodes"][0]["children"], doc["nodes"][1]["children"]), ([1], [2]))
        self.assertEqual(doc["skins"][0]["joints"], [0, 1, 2])
        self.assertEqual(doc["nodes"][3]["skin"], 0)
        # Joint 1's vertices use its post node, skin joint 2.
        attributes = doc["meshes"][0]["primitives"][0]["attributes"]
        view = doc["bufferViews"][doc["accessors"][attributes["JOINTS_0"]]["bufferView"]]
        self.assertEqual(binary[view["byteOffset"]:view["byteOffset"] + 12], bytes((0, 0, 0, 0, 2, 0, 0, 0, 0, 2, 0, 0)))
        loop, still = doc["animations"]
        self.assertEqual((loop["name"], still["name"]), ("seq_00_loop", "seq_01"))
        targets = [(c["target"]["node"], c["target"]["path"]) for c in loop["channels"]]
        self.assertEqual(targets, [(0, "rotation"), (0, "translation"), (2, "scale")])
        rotation = doc["accessors"][loop["samplers"][0]["output"]]
        self.assertEqual((rotation["componentType"], rotation["normalized"], rotation["count"]), (5122, True, 3))
        # The still sequence's only channel never changes: one key at time 0.
        sampler = still["samplers"][0]
        self.assertEqual([doc["accessors"][sampler[k]]["count"] for k in ("input", "output")], [1, 1])


if __name__ == "__main__":
    unittest.main()
