"""Collision editing: the block model's exact round trip, edits, limits, and the scene's way back.
Hand-built blocks; no disc data."""

import hashlib
import json
import struct
import tempfile
import unittest
from pathlib import Path

import collision as col
import collision_scene as cs
from formats import FormatError
from godot import Scene


def cell_point(cell, dx, dy, dz):
    """A point near a cell's centre, in model units (1/64), on the leaves' grid."""
    return ((4 * cell[0] + 2) * 64 + 4 * dx, (4 * cell[1] + 2) * 64 + 4 * dy, (4 * cell[2] + 2) * 64 + dz)


def sample() -> col.Block:
    """Two cells, one quad and a triangle in the first, a triangle in the second, and a hero group;
    laid out in an order no sort would give (the disc's own orders are not sorted either)."""
    a, b = (10, 20, 5), (11, 20, 5)
    pa = [cell_point(a, x, y, 0) for x, y in ((-10, -10), (10, -10), (10, 10), (-10, 10), (0, 0))]
    pa[4] = cell_point(a, 0, 0, 40)
    pb = [cell_point(b, x, y, 0) for x, y in ((-5, -5), (5, -5), (0, 5))]
    model = col.Block(z=(5, 1), slabs={5: (20, 1)}, rows={(5, 20): (10, 2)},
                      cells={a: col.Leaf(pa, [((0, 1, 2, 3), 0x1f), ((0, 1, 4), 0x0c)]),
                             b: col.Leaf(pb, [((0, 1, 2), 0x2b)])},
                      slab_order=[5], row_order=[(5, 20)], leaf_order=[b, a],
                      heroes=[col.HeroGroup((100, 100, 100, 50), [(65, 63, 64), (129, 66, 64), (67, 127, 64)],
                                            [(0, 1, 2), (0, 1, 1)])])  # off the cells' grid; one degenerate
    return model


class BlockTests(unittest.TestCase):
    def test_round_trip(self):
        data = col.write(sample())
        self.assertEqual(len(data) % 0x40, 0)
        again = col.parse(data)
        self.assertEqual(again.leaf_order, [(11, 20, 5), (10, 20, 5)])
        self.assertEqual(col.write(again), data)
        # The decoder (what the editor shows) sees the same triangles as the model.
        shown = {(col.canonical(t[:3]), t[3]) for t in col.decode(data).triangles}
        self.assertEqual(shown, col.block_triangles(again))
        self.assertEqual(again.heroes[0].triangles, [(0, 1, 2), (0, 1, 1)])

    def test_no_hero_section(self):
        model = sample()
        model.heroes = None
        data = col.write(model)
        self.assertEqual(struct.unpack_from("<I", data, 4)[0], 0)
        self.assertEqual(col.write(col.parse(data)), data)

    def test_remove_half_a_quad(self):
        model = sample()
        a = (10, 20, 5)
        leaf = model.cells[a]
        half = col.face_triangles(leaf, (0, 1, 2, 3))[1][1]
        col.edit(model, {(half, 0x1f)}, [])
        faces = model.cells[a].faces
        self.assertEqual(model.cells[a].quads, 0)
        self.assertEqual(len(faces), 2)
        shown = col.block_triangles(col.parse(col.write(model)))
        self.assertNotIn((half, 0x1f), shown)
        self.assertEqual(len(shown), 3)

    def test_remove_a_whole_cell(self):
        model = sample()
        b = (11, 20, 5)
        (key,) = [k for _, k in col.face_triangles(model.cells[b], (0, 1, 2))]
        col.edit(model, {(key, 0x2b)}, [])
        self.assertNotIn(b, model.cells)
        self.assertEqual(col.parse(col.write(model)).leaf_order, [(10, 20, 5)])

    def test_untouched_cells_keep_their_bytes(self):
        model = sample()
        before = col.encode_leaf((11, 20, 5), model.cells[(11, 20, 5)])
        p = (cell_point((10, 20, 5), -8, -8, 10), cell_point((10, 20, 5), 8, -8, 10), cell_point((10, 20, 5), 0, 8, 10))
        col.edit(model, set(), [(p, 0x05)])
        self.assertEqual(col.encode_leaf((11, 20, 5), model.cells[(11, 20, 5)]), before)
        self.assertIn((col.canonical(p), 0x05), col.block_triangles(model))

    def test_added_triangle_goes_into_every_cell_it_touches(self):
        model = sample()
        # From cell (10, 20, 5) across to (12, 21, 5): new cells, a wider row, a new row.
        p = (cell_point((10, 20, 5), 0, 0, 0), cell_point((12, 20, 5), 0, 0, 0), cell_point((12, 21, 5), 0, 0, 0))
        rewritten, new, pieces = col.edit(model, set(), [(p, 0x1f)])
        self.assertEqual(pieces, 1)
        for cell in [(10, 20, 5), (11, 20, 5), (12, 20, 5), (12, 21, 5)]:
            self.assertIn(cell, model.cells)
        self.assertGreater(new, 0)
        col.validate(model)
        again = col.parse(col.write(model))
        self.assertEqual(again.rows[(5, 20)], (10, 3))
        self.assertIn((col.canonical(p), 0x1f), col.block_triangles(again))

    def test_large_triangle_is_split_to_fit(self):
        model = sample()
        c = (50, 50, 5)
        p = (cell_point(c, 0, 0, 0), cell_point(c, 16 * 50, 0, 0), cell_point(c, 0, 16 * 50, 0))  # 50 units
        _, _, pieces = col.edit(model, set(), [(p, 0x1f)])
        self.assertGreater(pieces, 1)
        col.validate(model)
        data = col.write(model)                     # every vertex fits its cell
        self.assertEqual(col.write(col.parse(data)), data)

    def test_overlap(self):
        cell = (1, 1, 1)
        inside = (cell_point(cell, 0, 0, 0), cell_point(cell, 1, 0, 0), cell_point(cell, 0, 1, 0))
        self.assertTrue(col.overlaps(inside, cell))
        self.assertFalse(col.overlaps(inside, (2, 1, 1)))
        # A triangle whose bounds cover the cell but which passes by its corner.
        far = ((0, 0, 0), (3 * 256, 0, 0), (0, 3 * 256, 3 * 256))
        self.assertFalse(col.overlaps(far, (2, 2, 0)))
        self.assertTrue(col.overlaps(far, (0, 0, 0)))

    def test_limits(self):
        model = sample()
        a = (10, 20, 5)
        model.cells[a].vertices += [cell_point(a, i % 20, i // 20, 0) for i in range(260)]
        with self.assertRaisesRegex(FormatError, "vertices, 255 at most"):
            col.validate(model)
        model = sample()
        model.z = (250, 10)
        with self.assertRaisesRegex(FormatError, "outside 0-255"):
            col.validate(model)
        model = sample()
        model.heroes[0].triangles *= 300
        with self.assertRaisesRegex(FormatError, "hero group 0"):
            col.validate(model)

    def test_hero_group(self):
        g = col.hero_group([((64, 64, 64), (640, 64, 64), (64, 640, 64))])
        self.assertEqual(g.triangles, [(0, 1, 2)])
        cx, cy, cz, r = g.sphere
        self.assertTrue(all(sum((p[i] - (cx, cy, cz)[i]) ** 2 for i in range(3)) <= r * r for p in g.vertices))


class SceneTests(unittest.TestCase):
    """Export a level's collision into a project, edit the scene's text, read it back."""

    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.project = Path(self.tmp.name)
        self.level_dir = self.project / "levels" / "level_00"
        self.level_dir.mkdir(parents=True)
        self.block = col.write(sample())
        self.manifest = cs.export(self.level_dir, self.block)
        scene = Scene()
        scene.node("Level_00", kind="Node3D")
        scene.node("Game", ".", "Node3D")
        cs.add_collision_nodes(scene, "res://levels/level_00", self.manifest)
        self.text = scene.text()

    def tearDown(self):
        self.tmp.cleanup()

    def read(self, text):
        return cs.scene_collision(self.project, self.level_dir, text)

    def test_export(self):
        self.assertEqual(sum(t["triangles"] for t in self.manifest["tiles"].values()),
                         len(col.block_triangles(sample())))
        self.assertEqual(list(self.manifest["hero_walls"]), ["HeroWall_000"])
        written = json.loads((self.level_dir / "collision" / "collision.json").read_text())
        self.assertEqual(written, self.manifest)
        self.assertIn('metadata/rc1_collision = 1', self.text)

    def test_unedited_scene_packs_to_nothing(self):
        self.assertIsNone(self.read(self.text))
        # Godot re-saving the scene adds unique ids to node headers: still no edit.
        resaved = self.text.replace('parent="Game/Collision"', 'parent="Game/Collision" unique_id=12345')
        self.assertIsNone(self.read(resaved))

    def test_glb_read_back_is_exact(self):
        """Forcing the slow path on an unchanged scene: the tiles read back as the same triangles."""
        found = cs.scene_collision(self.project, self.level_dir, self.text + "\n")
        self.assertIsNone(found)
        tile = next(iter(self.manifest["tiles"]))
        text = self.text.replace(f'[node name="{tile}" parent="Game/Collision"',
                                 f'[node name="{tile}" parent="Game/Collision"') + \
            '\n[node name="Nothing" type="Node3D" parent="Game/Collision"]\n'
        found = self.read(text)
        self.assertEqual(found["triangles"], col.block_triangles(sample()))
        new, changes, problems = cs.pack_block(self.block, found)
        self.assertIsNone(new)
        self.assertEqual(problems, [])

    def test_delete_a_tile(self):
        tile = next(iter(self.manifest["tiles"]))
        lines = self.text.split("\n\n")
        text = "\n\n".join(b for b in lines if f'name="{tile}"' not in b)
        new, changes, _ = cs.pack_block(self.block, self.read(text))
        self.assertIsNotNone(new)
        self.assertEqual(col.decode(new).triangles, [])
        self.assertIn("removed", changes[0])

    def test_add_a_box(self):
        text = self.text + '''
[sub_resource type="BoxMesh" id="box"]
size = Vector3(2, 2, 2)

[node name="Wall" type="MeshInstance3D" parent="Game/Collision"]
transform = Transform3D(1, 0, 0, 0, 1, 0, 0, 0, 1, 50, 60, 30)
mesh = SubResource("box")
metadata/rc1_surface = 12
'''
        new, changes, problems = cs.pack_block(self.block, self.read(text))
        self.assertEqual(problems, [])
        shown = {(col.canonical(t[:3]), t[3]) for t in col.decode(new).triangles}
        boxes = [k for k in shown if k[1] == 12 and k not in col.block_triangles(sample())]
        self.assertEqual(len(boxes), 12)
        # Outward: the top face's normal points up (+Z), in glTF's counter-clockwise winding.
        top = [k for k, _ in boxes if all(p[2] == 31 * 64 for p in k)]
        a, b, c = top[0]
        n = ((b[0] - a[0]) * (c[1] - a[1]) - (b[1] - a[1]) * (c[0] - a[0]))
        self.assertGreater(n, 0)

    def test_move_a_tile(self):
        tile = next(iter(self.manifest["tiles"]))
        text = self.text.replace(f'[node name="{tile}" parent="Game/Collision" instance=ExtResource("{tile.lower()}")]',
                                 f'[node name="{tile}" parent="Game/Collision" instance=ExtResource("{tile.lower()}")]\n'
                                 'transform = Transform3D(1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 2)')
        found = self.read(text)
        new, changes, _ = cs.pack_block(self.block, found)
        moved = {(col.canonical(t[:3]), t[3]) for t in col.decode(new).triangles}
        self.assertEqual({tuple(p[2] for p in k) for k, _ in moved},
                         {tuple(p[2] + 128 for p in k) for k, _ in col.block_triangles(sample())})

    def test_surface_on_a_tile(self):
        tile = next(iter(self.manifest["tiles"]))
        text = self.text.replace(f'instance=ExtResource("{tile.lower()}")]',
                                 f'instance=ExtResource("{tile.lower()}")]\nmetadata/rc1_surface = 0')
        new, _, _ = cs.pack_block(self.block, self.read(text))
        self.assertEqual({t[3] for t in col.decode(new).triangles}, {0})

    def test_delete_a_hero_wall(self):
        text = "\n\n".join(b for b in self.text.split("\n\n") if 'name="HeroWall_000"' not in b)
        new, changes, _ = cs.pack_block(self.block, self.read(text))
        self.assertEqual(col.parse(new).heroes, [])
        self.assertIn("hero wall 0 deleted", changes[0])

    def test_unreadable_tile_is_kept(self):
        tile, entry = next(iter(self.manifest["tiles"].items()))
        (self.level_dir / entry["file"]).write_bytes(b"not a glb")
        found = self.read(self.text)
        new, _, problems = cs.pack_block(self.block, found)
        self.assertIsNone(new)
        self.assertTrue(any("left as it was" in p for p in problems))

    def test_old_project(self):
        (self.level_dir / "collision" / "collision.json").unlink()
        self.assertIsNone(self.read(self.text))


class DockEditTests(SceneTests):
    """The collision dock's edits (metadata on Game/Collision, addons/openrac_collision) read back,
    packed and decoded: exactly the edit, and nothing else."""

    def edited(self, **edits) -> str:
        lines = [f"metadata/{key} = PackedInt32Array({', '.join(str(v) for r in records for v in r)})"
                 for key, records in edits.items()]
        return self.text.replace("metadata/rc1_collision = 1", "\n".join(["metadata/rc1_collision = 1", *lines]))

    def packed(self, text: str) -> tuple[set, list[str], list[str], bytes]:
        new, changes, problems = cs.pack_block(self.block, self.read(text))
        self.assertIsNotNone(new)
        return {(col.canonical(t[:3]), t[3]) for t in col.decode(new).triangles}, changes, problems, new

    @staticmethod
    def record(key: tuple, kind: int, turn: int = 0) -> tuple:
        points = key[turn:] + key[:turn]
        return (*(c for p in points for c in p), kind)

    def faces(self) -> list[tuple]:
        return sorted(col.block_triangles(sample()))

    def test_no_edits_pack_nothing(self):
        self.assertIsNone(self.read(self.edited(rc1_removed=[], rc1_added=[])))

    def test_repaint(self):
        key, kind = next(f for f in self.faces() if f[1] == 0x0c)
        shown, changes, problems, new = self.packed(self.edited(
            rc1_removed=[self.record(key, kind, turn=1)], rc1_added=[self.record(key, 0x8a)]))
        self.assertEqual(problems, [])
        self.assertEqual(shown, set(self.faces()) - {(key, kind)} | {(key, 0x8a)})
        self.assertIn("1 triangles removed, 1 added", changes[0])
        # The cell without that face keeps its bytes.
        old, again = col.parse(self.block), col.parse(new)
        self.assertEqual(col.encode_leaf((11, 20, 5), old.cells[(11, 20, 5)]),
                         col.encode_leaf((11, 20, 5), again.cells[(11, 20, 5)]))

    def test_delete(self):
        face = self.faces()[0]
        shown, _, problems, _ = self.packed(self.edited(rc1_removed=[self.record(*face)]))
        self.assertEqual((shown, problems), (set(self.faces()) - {face}, []))

    def test_move(self):
        key, kind = next(f for f in self.faces() if f[1] == 0x2b)
        moved = tuple((x + 16, y, z + 64) for x, y, z in key)
        shown, _, _, _ = self.packed(self.edited(rc1_removed=[self.record(key, kind)],
                                                 rc1_added=[self.record(moved, kind)]))
        self.assertEqual(shown, set(self.faces()) - {(key, kind)} | {(moved, kind)})

    def test_add_quad_on_the_grid(self):
        # Corners off the grid snap to it: 1/16 across, 1/64 up.
        a, b, c, d = (2561, 5122, 1408), (2690, 5122, 1408), (2690, 5250, 1408), (2561, 5250, 1408)
        shown, _, _, _ = self.packed(self.edited(rc1_added=[(*a, *b, *c, 0x1f), (*a, *c, *d, 0x1f)]))
        added = shown - set(self.faces())
        self.assertEqual(len(added), 2)
        self.assertTrue(all(p[0] % 4 == 0 and p[1] % 4 == 0 for key, _ in added for p in key))

    def test_hero_face_removed(self):
        group = sample().heroes[0]
        tri = tuple(group.vertices[i] for i in group.triangles[0])
        new, changes, problems = cs.pack_block(self.block, self.read(self.edited(
            rc1_hero_removed=[(0, *(c for p in tri for c in p))])))
        self.assertEqual(problems, [])
        self.assertEqual(col.parse(new).heroes[0].triangles, [(0, 1, 1)])
        self.assertIn("collision: hero wall 0 edited", changes)

    def test_stale_removal_is_a_problem(self):
        new, _, problems = cs.pack_block(self.block, self.read(self.edited(rc1_removed=[(0,) * 9 + (31,)])))
        self.assertIsNone(new)
        self.assertTrue(any("no longer in the tiles" in p for p in problems))

    def test_malformed(self):
        with self.assertRaisesRegex(FormatError, "ten per face"):
            self.read(self.edited(rc1_added=[(1, 2, 3)]))


class LevelDataTests(unittest.TestCase):
    def test_core_in_place_and_moved(self):
        core, index = bytes(range(256)) * 4, bytearray(0x90)
        struct.pack_into("<I", index, 0x14, 0x40)
        struct.pack_into("<I", index, 0x8C, len(core))
        out, new_index, notes = cs.core_with(core, bytes(index), 0x40, 0x80, b"\x01" * 0x40)
        self.assertEqual(out[0x40:0xC0], b"\x01" * 0x40 + bytes(0x40))
        self.assertEqual((new_index, notes), (bytes(index), []))
        out, new_index, notes = cs.core_with(core, bytes(index), 0x40, 0x80, b"\x02" * 0xC0)
        self.assertEqual(struct.unpack_from("<I", new_index, 0x14)[0], len(core))
        self.assertEqual(struct.unpack_from("<I", new_index, 0x8C)[0], len(out))
        self.assertEqual(out[len(core):], b"\x02" * 0xC0)
        self.assertEqual(out[0x40:0xC0], bytes(0x80))       # the old place, no longer read
        self.assertEqual(out[:0x40] + out[0xC0:len(core)], core[:0x40] + core[0xC0:])
        self.assertTrue(notes)

    def test_level_data_patches(self):
        data = bytearray(16 * 2048)
        struct.pack_into("<ii", data, 2 * 8, 2048 + 100, 0x90)      # the core index
        struct.pack_into("<ii", data, 10 * 8, 5 * 2048 + 10, 3000)  # the core stream
        index = bytes(0x90)
        moved = bytearray(index)
        moved[0x14] = 1
        runs = cs.level_data_patches(bytes(data), b"core", bytes(moved), index, lambda core, head: b"WAD" + core)
        self.assertEqual([offset for offset, _ in runs], [2048, 5 * 2048])
        self.assertEqual(len(runs[1][1]), 2 * 2048)
        self.assertEqual(runs[1][1][10:17], b"WADcore")
        self.assertEqual(runs[0][1][100 + 0x14], 1)
        with self.assertRaisesRegex(FormatError, "more than its place"):
            cs.level_data_patches(bytes(data), b"x", index, index, lambda core, head: bytes(4000))


if __name__ == "__main__":
    unittest.main()
