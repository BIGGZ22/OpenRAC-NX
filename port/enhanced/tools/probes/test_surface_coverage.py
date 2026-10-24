# SPDX-License-Identifier: GPL-3.0-or-later
import copy
import unittest

from surface_coverage import Surfaces, extend_regions


def quad(a, b, c, d):
    return [[a,b,c],[a,c,d]]


def room(x0=32, y0=32, x1=64, y1=64, floor=0, roof=12, walls=True):
    triangles = quad([x0,y0,floor],[x1,y0,floor],[x1,y1,floor],[x0,y1,floor])
    triangles += quad([x0,y0,roof],[x1,y0,roof],[x1,y1,roof],[x0,y1,roof])
    if walls:
        for a,b in (((x0,y0),(x1,y0)),((x1,y0),(x1,y1)),
                    ((x1,y1),(x0,y1)),((x0,y1),(x0,y0))):
            triangles += quad([*a,floor],[*b,floor],[*b,roof],[*a,roof])
    return triangles


class SurfaceCoverage(unittest.TestCase):
    coarse = {'origin': [0,0,-16], 'spacing': [64]*3, 'dimensions': [5]*3}
    region = {'bounds': [40,40,-4,56,56,8], 'source_mobys': [1], 'source_transitions': [2]}

    def extend(self, geometry, **kwargs):
        return extend_regions([self.region], geometry, self.coarse, [4,4,4], **kwargs)

    def test_room_reaches_first_ceiling_and_preserves_sources(self):
        before = copy.deepcopy(self.region)
        for triangles in (room(), [list(reversed(t)) for t in room()]):
            regions, report = self.extend(triangles)
            self.assertTrue(report[0]['changed'])
            self.assertTrue(report[0]['indoor_samples'])
            self.assertEqual(max(r['bounds'][5] for r in regions),16)
            self.assertTrue(all(r['source_mobys']==[1] for r in regions))
            self.assertEqual(self.region,before)
            # Connected interior beyond the marker footprint gets dense coverage.
            self.assertGreater(report[0]['connected_cells_added'],0)
            self.assertLessEqual(min(r['bounds'][0] for r in regions),32)

    def test_outdoor_wall_or_overhang_does_not_extend(self):
        for triangles in ([],room(walls=False),room()[4:6]):
            result,report=self.extend(triangles)
            self.assertEqual(result[0]['bounds'],self.region['bounds'])
            self.assertFalse(report[0]['changed'])

    def test_stacked_floors_choose_first_ceiling(self):
        geometry=room()+room(floor=12,roof=32)
        result,_=self.extend(geometry)
        self.assertEqual(max(r['bounds'][5] for r in result),16)

    def test_open_corridor_qualifies(self):
        geometry=room(16,32,112,48,walls=False)
        for y in (32,48):
            geometry+=quad([16,y,0],[112,y,0],[112,y,12],[16,y,12])
        region={**self.region,'bounds':[40,36,-4,56,44,8]}
        result,report=extend_regions([region],geometry,self.coarse,[4]*3)
        self.assertTrue(report[0]['indoor_samples'])
        self.assertEqual(max(r['bounds'][5] for r in result),16)

    def test_doorway_region_only_extends_enclosed_portion(self):
        region={**self.region,'bounds':[40,40,-4,96,56,8]}
        result,_=extend_regions([region],room(),self.coarse,[4]*3)
        self.assertTrue(any(r['bounds'][5]==8 and r['bounds'][3]>=96 for r in result))
        self.assertTrue(any(r['bounds'][5]==16 and r['bounds'][3]<=72 for r in result))

    def test_height_and_probe_limits_report_incomplete_coverage(self):
        result,report=self.extend(room(roof=48),max_rise=12,budget=512)
        self.assertEqual(max(r['bounds'][5] for r in result),20)
        self.assertGreater(report[0]['limited_surfaces'],0)
        self.assertTrue(all(r['probe_count']<=512 for r in result))

    def test_existing_clearance_never_shrinks(self):
        region={**self.region,'bounds':[40,40,-4,56,56,24]}
        result,_=extend_regions([region],room(),self.coarse,[4]*3)
        self.assertGreaterEqual(max(r['bounds'][5] for r in result),24)
        self.assertLessEqual(min(r['bounds'][2] for r in result),-4)

    def test_queries_are_two_sided_and_use_nearest_surface(self):
        query=Surfaces(room()+room(floor=12,roof=32))
        self.assertAlmostEqual(query.ray((48,48,2),(0,0,1),64),10)
        self.assertAlmostEqual(query.ray((48,48,2),(0,0,-1),64),2)

    def test_invalid_inputs(self):
        for kwargs in ({'max_rise':0},{'max_rise':float('nan')},{'padding':-1}):
            with self.assertRaises(ValueError): self.extend([],**kwargs)
        with self.assertRaises(ValueError): self.extend([[[float('nan'),0,0]]*3])

    def test_airborne_marker_does_not_fill_empty_outdoor_column(self):
        from moby_seeds import candidates
        from types import SimpleNamespace
        from unittest.mock import patch
        placements={'mobys':[{'index':1,'class':459,'matrix':[0]*12+[48,48,30,1]}]}
        allowed={459:{'name':'flying fixture','radius':8,'above':3}}
        ground=SimpleNamespace(below=lambda position: 0)
        boxes,markers,_=candidates(placements,allowed,ground)
        self.assertEqual(len(boxes),2)
        self.assertTrue(markers[0]['split_airborne_column'])
        self.assertTrue(all(box['bounds'][5]-box['bounds'][2]==8 for box in boxes))
        with patch('moby_seeds.piston_travel',return_value=-30):
            boxes,markers,_=candidates(placements,allowed,ground)
        self.assertEqual(len(boxes),1)
        self.assertFalse(markers[0]['split_airborne_column'])
        self.assertGreater(boxes[0]['bounds'][5]-boxes[0]['bounds'][2],30)

    def test_outdoor_height_cannot_spread_from_a_higher_neighbour(self):
        low={**self.region,'bounds':[40,40,-4,56,56,8]}
        high={**self.region,'bounds':[44,40,4,60,56,16]}
        result,_=extend_regions([low,high],[],self.coarse,[4]*3)
        at_low=[r for r in result if r['bounds'][0]<=42<=r['bounds'][3]]
        self.assertEqual(max(r['bounds'][5] for r in at_low),8)
        self.assertEqual(len(result),2)

    def test_seed_merge_preserves_local_height_before_classification(self):
        from moby_seeds import combine
        boxes=[{'bounds':[40,40,-4,56,56,8],'source_mobys':[1],'source_transitions':[]},
               {'bounds':[44,40,4,60,56,16],'source_mobys':[2],'source_transitions':[]}]
        merged,_=combine([],boxes,self.coarse,[4]*3,4096,preserve_height=True)
        self.assertEqual(len(merged),2)
        self.assertEqual(max(r['bounds'][5] for r in merged if r['bounds'][0]<=42),8)

    def test_growth_does_not_cross_a_sealed_wall(self):
        result,report=self.extend(room()+room(64,32,96,64))
        self.assertTrue(report[0]['indoor_samples'])
        self.assertLessEqual(max(r['bounds'][3] for r in result),68)

    def test_shallow_floor_band_and_anisotropic_spacing(self):
        region={**self.region,'bounds':[39,40,-4,57,56,4]}
        result,report=extend_regions([region],room(),self.coarse,[3,4,4])
        self.assertTrue(report[0]['indoor_samples'])
        self.assertEqual(max(r['bounds'][5] for r in result),16)
        for piece in result:
            for a,step in enumerate([3,4,4]):
                self.assertAlmostEqual(piece['bounds'][a]/step,round(piece['bounds'][a]/step))
                self.assertAlmostEqual(piece['bounds'][a+3]/step,round(piece['bounds'][a+3]/step))


if __name__ == '__main__':
    unittest.main()
