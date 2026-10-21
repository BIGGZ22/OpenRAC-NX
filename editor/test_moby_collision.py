"""Moby class collision (moby_collision.py) on hand-built blobs: a crate's triangles and a
creature's primitives read and write byte for byte, edits, the checks and generated collision."""

import math
import struct
import unittest

from formats import FormatError
import moby_collision as mc

# A crate: the eight corners of a cube 2 x 1024 packed units across (1 model unit from its centre)
# and its twelve triangles, every face type 0x7f (no surface, footstep class 3), no primitives.
CORNERS = [(x, y, z, 0) for z in (-1024, 1024) for y in (-1024, 1024) for x in (-1024, 1024)]
CUBE = ((0, 2, 3, 1), (4, 5, 7, 6), (0, 1, 5, 4), (2, 6, 7, 3), (0, 4, 6, 2), (1, 3, 7, 5))


def crate_bytes() -> bytes:
    faces = []
    for a, b, c, d in CUBE:
        # The game's winding: its normal (v2 - v0) x (v1 - v0) points out.
        faces += [(a, c, b, 0x7f), (a, d, c, 0x7f)]
    out = struct.pack("<HHiii", 0, 0, 0, 4 * len(faces), 8 * len(CORNERS))
    out += b"".join(struct.pack("<4h", *v) for v in CORNERS)
    out += b"".join(bytes(f) for f in faces)
    return out


def record(kind: int, mask: int, body: bytes) -> bytes:
    return struct.pack("<bBH", kind, 4, mask) + body


def creature_bytes(spare: bytes = bytes(12)) -> bytes:
    prims = [
        record(1, 0x0003, spare + struct.pack("<4f", 10.0, -20.0, 300.0, 512.0)),           # sphere
        record(2, 0x0002, struct.pack("<iif3f4x", 3, 0, 256.0, 0.0, 0.0, 64.0)),            # sphere on joint 3
        record(3, 0x0001, struct.pack("<f8x", 900.0) + struct.pack("<4f", 0.0, 0.0, 0.0, 400.0)),  # cylinder
        record(4, 0x8002, struct.pack("<2h4xf16x", 1, 4, 128.0)),                           # capsule 1-4
    ]
    return struct.pack("<HHiii", 0, 5, 32 * len(prims), 0, 0) + b"".join(prims)


def normal(blob: mc.Blob, face) -> tuple:
    v0, v1, v2 = (blob.vertices[i][:3] for i in face[:3])
    e2, e1 = [b - a for a, b in zip(v0, v2)], [b - a for a, b in zip(v0, v1)]
    return (e2[1] * e1[2] - e2[2] * e1[1], e2[2] * e1[0] - e2[0] * e1[2], e2[0] * e1[1] - e2[1] * e1[0])


def outward(blob: mc.Blob) -> bool:
    """Every face's game normal points away from the mesh's centre."""
    centre = [sum(v[k] for v in blob.vertices) / len(blob.vertices) for k in range(3)]
    for f in blob.faces:
        mid = [sum(blob.vertices[i][k] for i in f[:3]) / 3 for k in range(3)]
        if sum(n * (m - c) for n, m, c in zip(normal(blob, f), mid, centre)) <= 0:
            return False
    return True


class FormatTest(unittest.TestCase):
    def test_crate_round_trip(self):
        data = crate_bytes()
        blob = mc.parse(data)
        self.assertEqual(mc.write(blob), data)
        self.assertEqual((len(blob.primitives), len(blob.vertices), len(blob.faces)), (0, 8, 12))
        self.assertEqual(mc.extent(data + bytes(8)), len(data))
        self.assertTrue(outward(blob))
        mc.validate(blob, 0)

    def test_creature_round_trip(self):
        data = creature_bytes()
        blob = mc.parse(data)
        self.assertEqual(mc.write(blob), data)
        sphere, joint, cylinder, capsule = blob.primitives
        self.assertEqual((sphere.kind, sphere.centre, sphere.radius, sphere.queries), (1, (10.0, -20.0, 300.0), 512.0, 3))
        self.assertEqual((joint.joints, joint.radius, joint.centre), ((3,), 256.0, (0.0, 0.0, 64.0)))
        self.assertEqual((cylinder.height, cylinder.radius), (900.0, 400.0))
        self.assertEqual((capsule.joints, capsule.radius, capsule.end), ((1, 4), 128.0, True))
        self.assertEqual(mc.derived_posed(blob), (0, 5))
        mc.validate(blob, 6)

    def test_unused_bytes_are_kept(self):
        data = creature_bytes(spare=bytes(range(1, 13)))
        self.assertEqual(mc.write(mc.parse(data)), data)

    def test_edit(self):
        blob = mc.parse(creature_bytes())
        blob.primitives[0].radius = 700.0
        blob.primitives.append(mc.Primitive(mc.CAPSULE, 64.0, joints=(5, 6)))
        mc.finish(blob)
        self.assertEqual([p.end for p in blob.primitives], [False] * 4 + [True])
        self.assertEqual(blob.posed, (7, 7))   # the new capsule's joints 5 and 6 are in both sets
        mc.validate(blob, 7)
        again = mc.parse(mc.write(blob))
        self.assertEqual(again.primitives[0].radius, 700.0)
        self.assertEqual(again.primitives[4].joints, (5, 6))

    def test_validators(self):
        def broken(change):
            blob = mc.parse(creature_bytes())
            change(blob)
            with self.assertRaises(FormatError):
                mc.validate(blob, 6)
        broken(lambda b: setattr(b.primitives[3], "end", False))
        broken(lambda b: setattr(b.primitives[0], "end", True))
        broken(lambda b: setattr(b.primitives[1], "joints", (6,)))       # the class has 6 joints
        broken(lambda b: setattr(b, "posed", (0, 4)))                   # the capsule's joint 4 is not posed
        broken(lambda b: setattr(b.primitives[0], "radius", 0.0))
        broken(lambda b: setattr(b.primitives[2], "height", -1.0))
        broken(lambda b: setattr(b.primitives[0], "radius", math.nan))
        broken(lambda b: setattr(b.primitives[0], "kind", 5))
        broken(lambda b: setattr(b.primitives[3], "joints", (1,)))
        crate = mc.parse(crate_bytes())
        crate.faces.append((0, 1, 8, 0x7f))
        with self.assertRaises(FormatError):
            mc.validate(crate, 0)
        crate = mc.parse(crate_bytes())
        crate.vertices += [(i, 0, 0, 0) for i in range(249)]          # 257: the last one unused, as on the disc
        mc.validate(crate, 0)
        crate.faces.append((0, 1, 256, 0x7f))                          # past what a byte index reaches
        with self.assertRaises(FormatError):
            mc.validate(crate, 0)
        crate.faces.pop()
        crate.vertices += [(0, 0, 0, 0)] * 2048
        with self.assertRaises(FormatError):
            mc.validate(crate, 0)

    def test_malformed_header(self):
        with self.assertRaises(FormatError):
            mc.parse(struct.pack("<HHiii", 0, 0, 33, 0, 0) + bytes(64))
        with self.assertRaises(FormatError):
            mc.parse(struct.pack("<HHiii", 0, 0, 32, 0, 0))


def cube(size: float = 1.0, centre=(0.0, 0.0, 0.0)) -> list[tuple]:
    """A cube's triangles, glTF winding (counter-clockwise from outside), model units."""
    blob = mc.parse(crate_bytes())
    return [tuple(tuple(c / 1024 * size + o for c, o in zip(p, centre)) for p in t) for t, _ in mc.triangles(blob)]


def ball(rings: int = 24, segments: int = 24) -> list[tuple]:
    def at(i, j):
        a, b = math.pi * i / rings, 2 * math.pi * j / segments
        return (math.sin(a) * math.cos(b), math.sin(a) * math.sin(b), math.cos(a))
    tris = []
    for i in range(rings):
        for j in range(segments):
            p, q, r, s = at(i, j), at(i + 1, j), at(i + 1, j + 1), at(i, j + 1)
            tris += [(p, q, r), (p, r, s)]
    return tris


class GenerateTest(unittest.TestCase):
    def test_mesh(self):
        blob = mc.generate(cube(), surface=0x68)
        self.assertEqual((len(blob.vertices), len(blob.faces)), (8, 12))
        self.assertTrue(all(f[3] == 0x68 for f in blob.faces))
        self.assertTrue(outward(blob))
        mc.validate(blob, 0)
        self.assertEqual(mc.write(mc.parse(mc.write(blob))), mc.write(blob))

    def test_scale(self):
        blob = mc.generate(cube(2.0), scale=2.0)   # drawn twice the model size: the same packed cube
        self.assertEqual(sorted(blob.vertices), sorted(CORNERS))

    def test_many_vertices(self):
        tris = ball(40, 40)
        self.assertGreater(len({p for t in tris for p in t}), 256)
        blob = mc.generate(tris)
        self.assertLessEqual(len(blob.vertices), 256)
        self.assertGreater(len(blob.faces), 100)
        mc.validate(blob, 0)

    def test_primitives(self):
        sphere = mc.generate(cube(), "sphere").primitives
        self.assertEqual([p.kind for p in sphere], [mc.SPHERE])
        self.assertAlmostEqual(sphere[0].radius, math.sqrt(3) * 1024, places=1)
        cylinder = mc.generate(cube(), "cylinder").primitives[0]
        self.assertEqual((cylinder.kind, cylinder.centre, cylinder.height), (mc.CYLINDER, (0.0, 0.0, -1024.0), 2048.0))
        self.assertAlmostEqual(cylinder.radius, math.sqrt(2) * 1024, places=1)
        box = mc.generate(ball(), "box")
        self.assertEqual((len(box.vertices), len(box.faces)), (8, 12))
        self.assertTrue(outward(box))
        for mode in ("sphere", "cylinder"):
            blob = mc.generate(cube(), mode)
            self.assertTrue(blob.primitives[-1].end)
            mc.validate(blob, 0)

    def test_limits(self):
        with self.assertRaises(FormatError):
            mc.generate(cube(40.0))              # past the 16-bit vertices
        mc.generate(cube(40.0), "sphere")        # a primitive's floats hold it
        with self.assertRaises(FormatError):
            mc.generate([])
        with self.assertRaises(FormatError):
            mc.generate(cube(), "teapot")


if __name__ == "__main__":
    unittest.main()
