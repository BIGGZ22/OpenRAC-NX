# SPDX-License-Identifier: GPL-3.0-or-later
import math
import random
from pathlib import Path
import struct
import tempfile
import unittest

import seed_regions as seeds
import moby_seeds as mobys
import json


def matrix():
    return [0.1, 0, 0, 0, 0, 0.05, 0, 0, 0, 0, 0.5, 0, -4, -2.5, -30, 1]


def gameplay(flags):
    start = 160
    data = bytearray(start + 16 + len(flags) * 144)
    struct.pack_into('<I', data, 0x80, start)
    struct.pack_into('<i', data, start, len(flags))
    for i, flag in enumerate(flags):
        at = start + 16 + len(flags) * 16 + i * 128
        struct.pack_into('<16f', data, at, *matrix())
        struct.pack_into('<2I', data, at + 0x40, 0x80808080, 0x80404040)
        struct.pack_into('<2i', data, at + 0x48, -1, 3)
        struct.pack_into('<I', data, at + 0x50, flag)
    return bytes(data)


class AuthoredSeeds(unittest.TestCase):
    coarse = {'origin': [0, 0, 0], 'spacing': [32, 32, 32], 'dimensions': [5, 5, 5]}

    def test_profile_spacing_overrides(self):
        profile = {'dense_spacing': [4, 6, 2]}
        self.assertEqual(seeds.profile_spacing(profile), [4, 6, 2])
        self.assertEqual(seeds.profile_spacing(profile, horizontal=8), [8, 8, 2])
        self.assertEqual(seeds.profile_spacing(profile, vertical=1), [4, 6, 1])
        self.assertEqual(seeds.profile_spacing(profile, 8, 1), [8, 8, 1])
        self.assertEqual(profile['dense_spacing'], [4, 6, 2])
        with self.assertRaises(ValueError):
            seeds.profile_spacing(profile, vertical=0)

    def test_optional_and_fog_only(self):
        self.assertEqual(seeds.transitions(bytes(148)), [])
        records = seeds.transitions(gameplay([0, 2, 1, 3]))
        self.assertEqual(records[2]['light_sets'], [11, 3])
        regions, skipped = seeds.plan_regions(records, self.coarse)
        self.assertEqual([r['source_transition'] for r in regions], [2, 3])
        self.assertEqual(len(skipped), 2)
        self.assertEqual(seeds.plan_regions([], self.coarse), ([], []))

    def test_transform_all_corners(self):
        lo, hi = seeds.cuboid_bounds(matrix())
        for a, b in zip(lo + hi, [30, 30, 58, 50, 70, 62]):
            self.assertAlmostEqual(a, b)
        c = math.sqrt(0.5) / 10
        rotated = [c, c, 0, 0, -c, c, 0, 0, 0, 0, 0.5, 0, 0, 0, 0, 1]
        lo, hi = seeds.cuboid_bounds(rotated)
        self.assertAlmostEqual(lo[0], -math.sqrt(200))
        self.assertAlmostEqual(hi[1], math.sqrt(200))

    def test_padding_clipping_and_budget(self):
        regions, _ = seeds.plan_regions(seeds.transitions(gameplay([1])), self.coarse)
        bounds = regions[0]['bounds']
        self.assertLess(bounds[0], 30)
        self.assertGreater(bounds[3], 50)
        tiny = {'origin': [32, 32, 32], 'spacing': [32, 32, 32], 'dimensions': [2, 2, 2]}
        clipped, _ = seeds.plan_regions(seeds.transitions(gameplay([1])), tiny)
        self.assertTrue(clipped)
        for r in clipped:
            self.assertTrue(all(32 <= v <= 64 for v in r['bounds']))
        boxes = seeds.split_bounds([0, 0, 0, 128, 32, 32], 4, 1024)
        self.assertTrue(len(boxes) > 1)
        self.assertTrue(all(seeds.probe_count(b, 4) <= 1024 for b in boxes))
        self.assertEqual(min(b[0] for b in boxes), 0)
        self.assertEqual(max(b[3] for b in boxes), 128)
        ordered = sorted(boxes, key=lambda b: b[0])
        self.assertTrue(all(a[3] > b[0] for a, b in zip(ordered, ordered[1:])))

    def test_bad_data(self):
        for data in (b'', gameplay([1])[:-1]):
            with self.assertRaises(ValueError):
                seeds.transitions(data)
        records = seeds.transitions(gameplay([1]))
        records[0]['inverse'] = [0] * 16
        regions, skipped = seeds.plan_regions(records, self.coarse)
        self.assertEqual(regions, [])
        self.assertIn('singular', skipped[0]['reason'])
        with self.assertRaises(ValueError):
            seeds.plan_regions([], self.coarse, spacing=32)

    def test_sparse_metadata(self):
        header = struct.pack('<4I', 0x5047524f, 3, 1, 9) + b'rac1-ntsc'
        header += struct.pack('<IQQ6f4If', 3, 1, 2, 0, 0, 0, 32, 32, 32, 5, 5, 5, 8, 1234.5)
        with tempfile.TemporaryDirectory() as folder:
            file = Path(folder)/'base.orpg'
            file.write_bytes(header)
            actual = seeds.coarse_header(file)
            self.assertEqual(actual['level'], 3)
            self.assertEqual(actual['dimensions'], [5, 5, 5])
            self.assertEqual(actual['max_distance'], 1234.5)
            self.assertEqual(actual['bake_version'], 1)
            self.assertEqual(actual['probe_format_version'], 3)
            for version, revision in ((1, 1), (2, 1), (3, 99)):
                file.write_bytes(struct.pack('<3I', 0x5047524f, version, revision)+header[12:])
                with self.assertRaises(ValueError):
                    seeds.coarse_header(file)

    def test_optional_registration(self):
        with tempfile.TemporaryDirectory() as folder:
            coarse = Path(folder)/'coarse.orpg'
            self.assertIsNone(seeds.register_regions(coarse, []))
            self.assertFalse(Path(str(coarse) + '.regions.json').exists())
            sidecar = seeds.register_regions(coarse, [Path(folder)/'bakes'/'region.orpg'])
            import json
            value = json.loads(sidecar.read_text())
            self.assertEqual(Path(value['regions'][0]), Path('bakes/region.orpg'))
            with self.assertRaises(FileExistsError):
                seeds.register_regions(coarse, [Path(folder)/'other.orpg'])

    def test_explicit_registration_replacement(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            coarse = root/'coarse.orpg'
            sidecar = seeds.register_regions(coarse, [root/'old.orpg'])
            previous = sidecar.read_bytes()
            new = root/'new'
            new.mkdir()
            seeds.register_regions(coarse, [new/'region.orpg'], new)
            self.assertEqual((new/'previous-regions.json').read_bytes(), previous)
            self.assertEqual(Path(json.loads(sidecar.read_text())['regions'][0]), Path('new/region.orpg'))
            # A failed backup must leave the currently active registry untouched.
            current = sidecar.read_bytes()
            with self.assertRaises(FileExistsError):
                seeds.register_regions(coarse, [root/'bad.orpg'], new)
            self.assertEqual(sidecar.read_bytes(), current)


class MobySeeds(unittest.TestCase):
    def test_incremental_merges_match_exhaustive_selection(self):
        # Independent exhaustive selection guards tie order and stale pair scores.
        def exhaustive(regions, spacing, budget):
            regions = regions.copy()
            while True:
                choices = []
                for i, a in enumerate(regions):
                    for j in range(i+1, len(regions)):
                        b = regions[j]
                        if any(a['bounds'][k] > b['bounds'][k+3]
                               or b['bounds'][k] > a['bounds'][k+3] for k in range(3)):
                            continue
                        bounds = [min(a['bounds'][k], b['bounds'][k]) for k in range(3)]
                        bounds += [max(a['bounds'][k+3], b['bounds'][k+3]) for k in range(3)]
                        count = seeds.probe_count(bounds, spacing)
                        score = count/(seeds.probe_count(a['bounds'], spacing)
                                       + seeds.probe_count(b['bounds'], spacing))
                        if count <= budget and score <= 1.15:
                            choices.append((score, i, j, bounds))
                if not choices:
                    return regions
                _, i, j, bounds = min(choices)
                a, b = regions[i], regions.pop(j)
                regions[i] = {'bounds': bounds,
                              'source_mobys': sorted(set(a['source_mobys']+b['source_mobys'])),
                              'source_transitions': sorted(set(a['source_transitions']+b['source_transitions']))}

        rng = random.Random(4701)
        for spacing in ([4,4,4], [4,4,2]):
            for budget in (64, 512, 4096):
                fixtures = [[], [{'bounds': [0,0,0,8,8,8], 'source_mobys': [i],
                                 'source_transitions': []} for i in range(12)]]
                for _ in range(20):
                    boxes = []
                    for i in range(25):
                        low = [rng.randrange(-4, 5)*s for s in spacing]
                        high = [low[a]+rng.randrange(1, 7)*spacing[a] for a in range(3)]
                        boxes.append({'bounds': low+high, 'source_mobys': [i],
                                      'source_transitions': [i%3]})
                    fixtures.append(boxes)
                for boxes in fixtures:
                    original = json.dumps(boxes)
                    self.assertEqual(mobys.merge_regions(boxes, spacing, budget),
                                     exhaustive(boxes, spacing, budget))
                    self.assertEqual(json.dumps(boxes), original)

    def placement(self, cls, index, position):
        return {'class': cls, 'index': index, 'matrix': [0]*12+position+[1],
                'fields': {'pvar_index': 0}}

    def test_explicit_whitelist_and_initial_markers(self):
        allowed = mobys.whitelist(Path(mobys.__file__).with_name('moby_seed_whitelist.json'))
        self.assertTrue({13, 14, 15, 16, 500, 501, 502, 505, 511, 703, 715}.issubset(allowed))
        self.assertNotIn(734, allowed)  # Unreviewed generic platform/decorations.
        placements = {'mobys': [self.placement(703, 1, [20, 20, 20]),
                                self.placement(13, 2, [22, 22, 20]),
                                self.placement(734, 3, [24, 24, 20]),
                                self.placement(500, 4, [float('nan'), 20, 20])]}
        boxes, markers, skipped = mobys.candidates(placements, allowed, mobys.Ground([]))
        self.assertEqual([m['index'] for m in markers], [1, 2])
        self.assertEqual(skipped[0]['moby'], 4)
        self.assertEqual(boxes[0]['bounds'][5], 36)  # Room above a jumping platform.
        self.assertEqual(boxes[1]['bounds'][5], 24)  # At least one four-unit cell overhead.
        self.assertEqual(boxes[1]['bounds'][2], 16)  # And below, to avoid floor fade-out.
        self.assertIsNone(markers[0]['ground_z'])

    def test_enemy_placements_seed_combat_coverage_without_projectiles(self):
        allowed = mobys.whitelist(Path(mobys.__file__).with_name('moby_seed_whitelist.json'))
        self.assertEqual(allowed[1023]['kind'], 'enemy')
        self.assertEqual(allowed[1201]['kind'], 'enemy')
        for cls in (41, 52, 186, 295, 354, 1355):
            self.assertNotIn(cls,allowed)  # rockets, seekers, allied Doom bots and fragments
        floor = [[0,0,8],[100,0,8],[0,100,8]]
        placements = {'mobys':[self.placement(1023,1,[24,24,10]),
                               self.placement(1201,2,[60,24,9]),
                               self.placement(577,3,[24,24,28]),
                               self.placement(52,4,[20,20,10])]}
        boxes,markers,skipped = mobys.candidates(placements,allowed,mobys.Ground([floor]))
        self.assertEqual(skipped,[])
        self.assertEqual([m['index'] for m in markers],[1,2,3])
        self.assertTrue(all(m['kind']=='enemy' for m in markers))
        self.assertEqual(markers[0]['bounds'][:3],[12,12,4])
        self.assertEqual(markers[2]['bounds'][5],32)  # airborne enemy retains its spawn height
        coarse = {'origin':[0,0,0],'spacing':[64,64,64],'dimensions':[3,3,3]}
        regions,clipped = mobys.combine([],boxes,coarse,[4,4,4],4096)
        self.assertEqual(clipped,[])
        for marker in markers:
            self.assertTrue(any(all(r['bounds'][a] <= marker['position'][a] <= r['bounds'][a+3]
                                    for a in range(3)) for r in regions))
        self.assertTrue(all(r['probe_count'] <= 4096 for r in regions))

    def test_projection_ignores_winding_and_preserves_upper_floor(self):
        low = [[0, 0, 0], [16, 0, 0], [0, 16, 0]]
        high = [[p[0], p[1], 12] for p in low]
        wall = [[0, 0, 0], [0, 16, 0], [0, 0, 40]]
        for triangles in ([low, high, wall], [list(reversed(t)) for t in [low, high, wall]]):
            ground = mobys.Ground(triangles)
            self.assertEqual(ground.below([4, 4, 15]), 12)
            self.assertEqual(ground.below([4, 4, 2]), 0)
            self.assertIsNone(ground.below([20, 20, 15]))
            self.assertIsNone(ground.below([4, 4, 80]))

    def test_coarser_vertical_spacing_retains_seed_interior_support(self):
        allowed = {13: {'name':'bolt', 'radius':8}, 715: {'name':'piston','radius':10,'above':16}}
        m = self.placement(13,1,[42,42,42.5])
        boxes, markers, _ = mobys.candidates({'mobys':[m]}, allowed, mobys.Ground([]),
                                            vertical_spacing=4)
        coarse = {'origin':[0]*3,'spacing':[64]*3,'dimensions':[5]*3}
        regions, _ = mobys.combine([],boxes,coarse,[4,4,4],4096)
        self.assertLessEqual(regions[0]['bounds'][2]+4,42.5)
        self.assertGreaterEqual(regions[0]['bounds'][5]-4,42.5)
        self.assertEqual(mobys.marker_bounds([42,42,42.5],None,-8,allowed[715],4)[2::3],
                         [30.5,58.5])

    def test_piston_full_travel_and_format_guard(self):
        data = bytearray(400)
        struct.pack_into('<2I', data, 0x54, 160, 176)
        struct.pack_into('<2i', data, 160, 0, 192)
        struct.pack_into('<f', data, 176+0xa8, 4.5)
        struct.pack_into('<f', data, 176+0xb0, 0.75)
        m = self.placement(715, 7, [20, 20, 20])
        self.assertEqual(mobys.piston_travel(data, m, 'rac1-ntsc', 1), -4.5)
        data[176+0xa4] = 1
        self.assertEqual(mobys.piston_travel(data, m, 'rac1-ntsc', 1), 4.5)
        self.assertIsNone(mobys.piston_travel(data, m, 'rac1-pal', 1))
        self.assertIsNone(mobys.piston_travel(data, m, 'rac1-ntsc', 2))
        self.assertIsNone(mobys.piston_travel(data[:200], m, 'rac1-ntsc', 1))
        malformed = bytearray(data)
        struct.pack_into('<2I', malformed, 0x54, 0x100000, 0x200000)
        self.assertIsNone(mobys.piston_travel(malformed, m, 'rac1-ntsc', 1))
        allowed = {715: {'name': 'piston', 'radius': 10, 'above': 16}}
        _, markers, _ = mobys.candidates({'mobys': [m]}, allowed, mobys.Ground([]), data, 'rac1-ntsc', 1)
        self.assertEqual(markers[0]['bounds'][5], 40.5)

    def test_clustering_budget_traceability_and_coverage(self):
        coarse = {'origin': [0]*3, 'spacing': [32]*3, 'dimensions': [10]*3}
        boxes = [{'bounds': [10, 10, 10, 30, 30, 26], 'source_mobys': [1], 'source_transitions': []},
                 {'bounds': [16, 12, 10, 36, 32, 26], 'source_mobys': [2], 'source_transitions': []},
                 {'bounds': [100, 100, 10, 120, 120, 26], 'source_mobys': [3], 'source_transitions': []}]
        authored = [{'bounds': [12, 12, 12, 28, 28, 24], 'source_transition': 9}]
        regions, skipped = mobys.combine(authored, boxes, coarse, 4, 4096)
        self.assertEqual(skipped, [])
        self.assertEqual(len(regions), 2)
        self.assertEqual(regions[0]['source_mobys'], [1, 2])
        self.assertEqual(regions[0]['source_transitions'], [9])
        for box in boxes:
            self.assertTrue(any(all(r['bounds'][a] <= box['bounds'][a] and
                                    r['bounds'][a+3] >= box['bounds'][a+3] for a in range(3)) for r in regions))
        large = [{'bounds': [0, 0, 0, 256, 32, 32], 'source_mobys': [8], 'source_transitions': []}]
        regions, _ = mobys.combine([], large, coarse, 4, 1024)
        self.assertGreater(len(regions), 1)
        self.assertTrue(all(r['probe_count'] <= 1024 and r['source_mobys'] == [8] for r in regions))

    def test_anisotropic_budget_alignment_and_authored_padding(self):
        spacing = [4, 4, 2]
        self.assertEqual(seeds.probe_count([0, 0, 0, 16, 16, 16], spacing), 225)
        parts = seeds.split_bounds([0, 0, 0, 32, 32, 64], spacing, 1024)
        self.assertGreater(len(parts), 1)
        self.assertTrue(all(seeds.probe_count(p, spacing) <= 1024 for p in parts))
        ordered = sorted(parts, key=lambda p: p[2])
        self.assertTrue(all(a[5] > b[2] for a, b in zip(ordered, ordered[1:])))
        coarse = {'origin': [0]*3, 'spacing': [32]*3, 'dimensions': [10]*3}
        regions, _ = seeds.plan_regions(seeds.transitions(gameplay([1])), coarse,
                                        spacing=spacing, vertical_padding=4)
        self.assertEqual(regions[0]['bounds'], [20, 20, 54, 60, 80, 66])
        boxes = [{'bounds': [11, 11, 11, 31, 31, 27],
                  'source_mobys': [7], 'source_transitions': []}]
        combined, _ = mobys.combine(regions, boxes, coarse, spacing, 4096)
        for region in combined:
            self.assertEqual(region['spacing'], spacing)
            self.assertTrue(all(region['bounds'][a] % spacing[a % 3] == 0 for a in range(6)))
            self.assertLessEqual(region['probe_count'], 4096)
        for bad in ([4, 0, 2], [4, 2], [4, 4, float('nan')]):
            with self.assertRaises(ValueError):
                seeds.plan_regions([], coarse, spacing=bad)



class OlderExports(unittest.TestCase):
    """An export without its gameplay bake input reads the gameplay from the disc."""

    def read(self, level):
        import sys
        import types

        class Disc:
            version = types.SimpleNamespace(key='pal', gameplay='gameplay_pal')

            def __init__(self, path):
                pass

            def __enter__(self):
                return self

            def __exit__(self, *exc):
                return False

            def survey(self):
                # The disc's own levels: 0 to 18.
                return {'levels': [{'id': i, 'ranges': {'gameplay_pal': {'lba': 10 + i, 'sectors': 1}}}
                                   for i in range(19)]}

            def sectors(self, lba, count):
                return bytes([lba]) * count

        fakes = {'disc': types.SimpleNamespace(Disc=Disc), 'level': types.SimpleNamespace(decoded=lambda b: b)}
        saved = {name: sys.modules.get(name) for name in fakes}
        sys.modules.update(fakes)
        try:
            manifest = {'game': 'rac1', 'version': 'pal', 'level': level}
            return seeds.read_gameplay(Path('unused'), manifest, Path('disc.iso'))
        finally:
            for name, module in saved.items():
                if module is None:
                    sys.modules.pop(name, None)
                else:
                    sys.modules[name] = module

    def test_a_level_of_the_disc(self):
        self.assertEqual(self.read(5), bytes([15]))

    def test_the_title_and_flight_scenes_have_no_gameplay(self):
        self.assertIsNone(self.read(99))
        self.assertIsNone(self.read(98))


if __name__ == '__main__':
    unittest.main()
