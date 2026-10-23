# SPDX-License-Identifier: GPL-3.0-or-later
import unittest

from surface_coverage import clip_footprint, extend_regions


class SurfaceCoverage(unittest.TestCase):
    coarse = {'origin': [0, 0, 0], 'spacing': [64]*3, 'dimensions': [5]*3}
    region = {'bounds': [52,168,38,96,220,50], 'source_mobys': [1],
              'source_transitions': [2]}
    ceiling = [[60.5625,174.5625,54.15625], [75.0625,174.5625,54.15625],
               [60.5625,190.625,54.15625]]

    def extend(self, triangles, **kwargs):
        return extend_regions([self.region], triangles, self.coarse, [4,4,2], **kwargs)

    def test_measured_novalis_ceiling_and_winding(self):
        for triangle in (self.ceiling, list(reversed(self.ceiling))):
            regions, diagnostics = self.extend([triangle])
            self.assertEqual(regions[0]['bounds'], [52,168,38,96,220,58])
            self.assertLessEqual(self.ceiling[0][2], regions[0]['bounds'][5]-2)
            self.assertEqual(regions[0]['source_mobys'], [1])
            self.assertEqual(regions[0]['source_transitions'], [2])
            self.assertTrue(diagnostics[0]['changed'])
            self.assertEqual(self.region['bounds'][5], 50)  # input never mutated

    def test_wall_connected_to_gameplay_band(self):
        wall = [[60,180,44], [60,190,44], [60,180,62]]
        regions, _ = self.extend([wall])
        self.assertEqual(regions[0]['bounds'][5], 66)
        floating_wall = [[p[0],p[1],p[2]+20] for p in wall]
        self.assertEqual(self.extend([floating_wall])[0][0]['bounds'][5], 50)

    def test_unrelated_and_open_geometry_preserve_clearance(self):
        for triangles in ([], [[[0,0,70],[10,0,70],[0,10,70]]],
                          [[[60,180,70]]*3],
                          [[[60,180,100],[70,180,100],[60,190,100]]],
                          [[[60,180,44],[70,180,44],[60,190,44]]]):
            self.assertEqual(self.extend(triangles)[0][0]['bounds'], self.region['bounds'])
        low_floor = [[60,180,30],[70,180,30],[60,190,30]]
        self.assertEqual(self.extend([low_floor], padding=30)[0][0]['bounds'], self.region['bounds'])

    def test_clips_remote_vertices_before_measuring_height(self):
        triangle = [[60,180,52],[70,180,52],[60,1000,200]]
        clipped = clip_footprint(triangle, self.region['bounds'])
        self.assertLess(max(p[2] for p in clipped), 60)
        regions, diagnostics = self.extend([triangle])
        self.assertLess(regions[0]['bounds'][5], 66)
        self.assertEqual(diagnostics[0]['limited_surfaces'], 0)

    def test_budget_splits_preserve_overlap_and_sources(self):
        regions, _ = self.extend([self.ceiling], budget=512)
        self.assertGreater(len(regions), 1)
        self.assertTrue(all(r['probe_count'] <= 512 for r in regions))
        self.assertTrue(all(r['source_mobys'] == [1] for r in regions))
        # Every point on the ceiling retains a cell of interior support in
        # at least one split, so splitting cannot create a coarse-light seam.
        for x in (61,64,68,72,75):
            for y in (175,180,185,190):
                p = [x,y,54.15625]
                self.assertTrue(any(all(r['bounds'][a]+[4,4,2][a] <= p[a]
                                        <= r['bounds'][a+3]-[4,4,2][a] for a in range(3))
                                    for r in regions))

    def test_limits_report_incomplete_coverage(self):
        wall = [[60,180,44],[60,190,44],[60,180,100]]
        regions, diagnostics = self.extend([wall], max_rise=12)
        self.assertEqual(max(r['bounds'][5] for r in regions), 62)
        self.assertEqual(diagnostics[0]['limited_surfaces'], 1)
        coarse = {**self.coarse, 'spacing': [64,64,2], 'dimensions': [5,5,29]}
        regions, diagnostics = extend_regions([self.region], [self.ceiling], coarse, [4,4,2])
        self.assertEqual(regions[0]['bounds'][5], 56)
        self.assertEqual(diagnostics[0]['limited_surfaces'], 1)

    def test_padding_and_piston_clearance_not_reduced(self):
        regions, _ = self.extend([self.ceiling], padding=0)
        self.assertEqual(regions[0]['bounds'][5], 58)
        region = {**self.region, 'bounds': [52,168,20,96,220,76]}
        result, _ = extend_regions([region], [self.ceiling], self.coarse, [4,4,2])
        self.assertEqual(min(r['bounds'][2] for r in result), 20)
        self.assertEqual(max(r['bounds'][5] for r in result), 76)

    def test_invalid_inputs(self):
        for kwargs in ({'max_rise':0}, {'max_rise':float('nan')}, {'padding':-1}):
            with self.assertRaises(ValueError):
                self.extend([], **kwargs)
        with self.assertRaises(ValueError):
            self.extend([[[float('nan'),0,0]]*3])


if __name__ == '__main__':
    unittest.main()
