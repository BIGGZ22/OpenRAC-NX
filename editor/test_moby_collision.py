"""Moby class collision (moby_collision.py, moby_collision_scene.py) on hand-built blobs: a crate's
triangles and a creature's primitives read and write byte for byte, edits, the checks, the class
scene's Collision node and its reading back, generated collision, and the packing into a core."""

import contextlib
import io
import math
import re
import struct
import tempfile
import unittest
from pathlib import Path

from formats import FormatError
from godot import write_marker
import moby_collision as mc
import moby_collision_scene as mcs

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


class Level:
    """Enough of level.Level for the packer: a core with one class blob and the class table."""

    def __init__(self, core: bytes, classes: dict[int, int]):
        self.core = core
        self.index = bytearray(0x100)
        struct.pack_into("<I", self.index, 0x8C, len(core))
        self.entries = [struct.pack("<ii24x", start, cid) for cid, start in classes.items()]
        self.problems = []

    def table(self, at: int, size: int) -> list[bytes]:
        assert (at, size) == (0x18, 32)
        return self.entries


def class_core(blob: bytes, joints: int = 0) -> tuple[bytes, int]:
    """A core data with a class at 0x40: its header (collision at +0x60, scale 0.5, JOINTS joints,
    their skeleton matrices after the collision), then 64 spare bytes."""
    start, at = 0x40, 0x60
    core = bytearray(start + at)
    struct.pack_into("<B", core, start + 8, joints)
    struct.pack_into("<i", core, start + 0x10, at)
    struct.pack_into("<f", core, start + 0x24, 0.5)
    core += blob + bytes(-len(blob) % 16)
    if joints:
        struct.pack_into("<i", core, start + 0x14, len(core) - start)
        for j in range(joints):  # joint j at (j * 100, 0, 0): the inverse bind moves it back
            core += struct.pack("<3f4x3f4x3f4x3f4x", 1, 0, 0, 0, 1, 0, 0, 0, 1, -100.0 * j, 0, 0)
    core += bytes(64)
    return bytes(core), start


class SceneTest(unittest.TestCase):
    def export(self, blob_bytes: bytes, joints: int = 0, class_id: int = 500):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        project = Path(self.tmp.name)
        (project / "project.godot").write_text("config_version=5\n")
        level_dir = project / "levels" / "level_03"
        core, start = class_core(blob_bytes, joints)
        level = Level(core, {class_id: start})
        _at, blob = mcs.class_collision(core, start)
        positions = mcs.joint_positions(core, start)
        files = {}

        def add(scene):
            files.update(mcs.add_class_collision_nodes(scene, blob, positions, 0.5, class_id, level_dir, "res://levels/level_03")["files"])
        path = level_dir / "mobys" / f"moby_{class_id}.tscn"
        write_marker(path, class_id, f"{class_id} crate", None, "crate", None, add)
        mcs.write_manifest(level_dir, {class_id: f"mobys/moby_{class_id}.tscn"}, files)
        return project, level_dir, level, path

    def test_scene_has_the_contract(self):
        _project, _level_dir, _level, path = self.export(crate_bytes())
        text = path.read_text()
        self.assertIn('[node name="Collision" type="Node3D" parent="."]', text)
        self.assertIn("visible = false", text)
        self.assertIn('[node name="Mesh_7f" parent="Collision"', text)
        self.assertIn('metadata/rc1_collision_kind = "mesh"', text)
        self.assertIn("metadata/rc1_surface = 127", text)

    def test_untouched_packs_nothing(self):
        for data, joints in ((crate_bytes(), 0), (creature_bytes(), 6)):
            project, level_dir, level, _path = self.export(data, joints)
            self.assertEqual(mcs.pack_classes(level, project, level_dir), ({}, [], []))
            # A re-saved scene (the manifest no longer matches) reads back as the disc's too.
            (level_dir / "mobys" / mcs.MANIFEST).unlink()
            self.assertEqual(mcs.pack_classes(level, project, level_dir), ({}, [], []))

    def test_primitives_read_back(self):
        project, level_dir, level, path = self.export(creature_bytes(), 6)
        text = path.read_text()
        disc = mc.parse(creature_bytes())
        positions = mcs.joint_positions(level.core, 0x40)
        self.assertEqual([round(c) for c in positions[3]], [300, 0, 0])
        edited, blob, problems = mcs.scene_blob(project, text, disc, positions)
        self.assertEqual((edited, problems), (False, []))
        # Grow the sphere (node scale 2) and move the joint sphere up by one model unit.
        text = re.sub(r'(\[node name="Sphere_00"[^\n]*\]\ntransform = Transform3D\()1, 0, 0, 0, 1, 0, 0, 0, 1',
                      r"\g<1>2, 0, 0, 0, 2, 0, 0, 0, 2", text)
        text = re.sub(r'(\[node name="JointSphere_01"[^\n]*\]\ntransform = Transform3D\(1, 0, 0, 0, 1, 0, 0, 0, 1, '
                      r'[^,]+, [^,]+, )([^)]+)\)', lambda m: m.group(1) + str(float(m.group(2)) + 1) + ")", text)
        edited, blob, problems = mcs.scene_blob(project, text, disc, positions)
        self.assertEqual((edited, problems), (True, []))
        self.assertEqual(blob.primitives[0].radius, 1024.0)
        self.assertAlmostEqual(blob.primitives[1].centre[2], 64.0 + 1024.0, places=2)
        self.assertEqual(blob.primitives[2:], disc.primitives[2:])
        mc.validate(blob, 6)
        path.write_text(text)
        edits, changes, problems = mcs.pack_classes(level, project, level_dir)
        self.assertEqual((list(edits), problems), ([500], []))
        self.assertIn("every moby of the class in this level", changes[0])

    def test_delete_and_add(self):
        project, level_dir, level, path = self.export(creature_bytes(), 6)
        text = path.read_text()
        disc = mc.parse(creature_bytes())
        positions = mcs.joint_positions(level.core, 0x40)
        gone = re.sub(r'\[node name="Capsule_03"[^\[]*', "", text)
        _edited, blob, _ = mcs.scene_blob(project, gone, disc, positions)
        self.assertEqual([p.kind for p in blob.primitives], [1, 2, 3])
        self.assertTrue(blob.primitives[-1].end)
        self.assertEqual(blob.posed, (0, 5))           # the header's own counts still cover the joints
        added = gone.replace("[node name=\"Moby_500\"", '[sub_resource type="SphereMesh" id="new"]\nradius = 2.0\n\n'
                             "[node name=\"Moby_500\"") + (
            '\n[node name="New" type="MeshInstance3D" parent="Collision"]\n'
            "transform = Transform3D(1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 3)\nmesh = SubResource(\"new\")\n")
        _edited, blob, problems = mcs.scene_blob(project, added, disc, positions)
        self.assertEqual(problems, [])
        self.assertEqual((blob.primitives[-1].kind, blob.primitives[-1].radius, blob.primitives[-1].centre),
                         (mc.SPHERE, 2048.0, (0.0, 0.0, 3072.0)))
        empty = re.sub(r'\[node name="\w+_\d\d" type="MeshInstance3D" parent="Collision"[^\[]*', "", text)
        edited, blob, _ = mcs.scene_blob(project, empty, disc, positions)
        self.assertEqual((edited, blob), (True, None))

    def test_bad_edits_are_reported(self):
        project, _level_dir, level, path = self.export(creature_bytes(), 6)
        positions = mcs.joint_positions(level.core, 0x40)
        tilted = re.sub(r'(\[node name="Cylinder_02"[^\n]*\]\ntransform = Transform3D\()[^)]*\)',
                        r"\g<1>1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0)", path.read_text())
        edited, blob, problems = mcs.scene_blob(project, tilted, mc.parse(creature_bytes()), positions)
        self.assertFalse(edited)
        self.assertIn("upright", problems[0])

    def test_triangles_edit(self):
        project, level_dir, level, path = self.export(crate_bytes())
        # Scale the crate's mesh node by two: every corner moves out.
        text = path.read_text().replace('metadata/rc1_collision_kind = "mesh"',
                                        'transform = Transform3D(2, 0, 0, 0, 2, 0, 0, 0, 2, 0, 0, 0)\n'
                                        'metadata/rc1_collision_kind = "mesh"')
        path.write_text(text)
        edits, _changes, problems = mcs.pack_classes(level, project, level_dir)
        self.assertEqual(problems, [])
        blob = mc.parse(edits[500])
        self.assertEqual(sorted(blob.vertices), sorted((2 * x, 2 * y, 2 * z, 0) for x, y, z, _ in CORNERS))
        self.assertTrue(outward(blob))
        # A glTF edited with another surface type: the material's name gives the faces' type byte.
        glb = level_dir / "mobys" / "collision" / "moby_500_7f.glb"
        glb.write_bytes(glb.read_bytes().replace(b"rc1_surface_7f", b"rc1_surface_08"))
        edits, _changes, _problems = mcs.pack_classes(level, project, level_dir)
        self.assertEqual({f[3] for f in mc.parse(edits[500]).faces}, {8})

    def test_generated_into_scene(self):
        project, level_dir, level, path = self.export(crate_bytes())
        blob = mc.generate(cube(0.5), "mesh", 0x08, 0.5)
        mcs.replace_in_scene(path, blob)
        text = path.read_text()
        self.assertEqual(len(re.findall(r'parent="Collision"', text)), 1)
        self.assertNotIn("moby_500_7f.glb", text)
        edits, _changes, problems = mcs.pack_classes(level, project, level_dir)
        self.assertEqual(problems, [])
        self.assertEqual(mc.parse(edits[500]).faces, blob.faces)
        sphere = mc.generate(cube(), "sphere")
        mcs.replace_in_scene(path, sphere)
        edits, _changes, problems = mcs.pack_classes(level, project, level_dir)
        self.assertEqual(problems, [])
        self.assertEqual(mc.parse(edits[500]).primitives[0].kind, mc.SPHERE)
        self.assertEqual(mc.parse(edits[500]).faces, [])

    def test_command(self):
        project, level_dir, level, path = self.export(crate_bytes())
        from gltf import Gltf
        model = Path(self.tmp.name) / "model.glb"
        gltf = Gltf()
        rows = [p for t in cube() for p in t]
        gltf.doc["meshes"].append({"name": "m", "primitives": [{"attributes": {"POSITION": gltf.floats(rows, bounds=True)},
                                                                 "mode": 4}]})
        gltf.node("m", 0)
        model.write_bytes(gltf.glb())
        out = Path(self.tmp.name) / "blob.bin"
        with contextlib.redirect_stdout(io.StringIO()):
            self.assertEqual(mc.main(["generate", str(model), "--out", str(out), "--axes", "game"]), 0)
            self.assertEqual(mc.main(["generate", str(model), "--scene", str(path), "--mode", "cylinder"]), 0)
        self.assertEqual(len(mc.parse(out.read_bytes()).faces), 12)
        self.assertIn("Cylinder_00", path.read_text())


class CoreTest(unittest.TestCase):
    def test_in_place_and_moved(self):
        core, start = class_core(crate_bytes())
        level = Level(core, {500: start})
        small = mc.write(mc.generate(cube(), "sphere"))
        out, index, notes = mcs.core_with_classes(core, bytes(level.index), level, {500: small})
        self.assertEqual((len(out), notes), (len(core), []))
        self.assertEqual(mcs.class_collision(out, start)[1], mc.parse(small))
        self.assertEqual(index, bytes(level.index))
        big = mc.write(mc.generate(ball(), "mesh"))
        out, index, notes = mcs.core_with_classes(core, bytes(level.index), level, {500: big})
        self.assertEqual(len(notes), 1)
        self.assertGreater(len(out), len(core))
        self.assertEqual(struct.unpack_from("<I", index, 0x8C)[0], len(out))
        at, blob = mcs.class_collision(out, start)
        self.assertEqual((at + start) % 16, 0)
        self.assertEqual(mc.write(blob), big)
        self.assertFalse(any(out[start + 0x60:start + 0x60 + len(crate_bytes())]))   # the old place is zeroed
        out, _index, _notes = mcs.core_with_classes(core, bytes(level.index), level, {500: None})
        self.assertEqual(mcs.class_collision(out, start), (0, None))


if __name__ == "__main__":
    unittest.main()
