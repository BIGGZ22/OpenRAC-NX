# SPDX-License-Identifier: GPL-3.0-or-later
"""Offline gameplay markers; these never add movable objects to bake geometry."""
from __future__ import annotations

import json
import math
from pathlib import Path
import struct
import sys


def whitelist(path: Path) -> dict[int, dict]:
    data = json.loads(path.read_text())
    if data['version'] != 1 or data['game'] != 'rac1':
        raise ValueError('unsupported moby seed whitelist')
    out = {}
    for entry in data['classes']:
        cls = entry['class']
        if cls in out or not isinstance(cls, int) or cls < 0:
            raise ValueError('duplicate or invalid moby class')
        if 'radius' not in entry or any(not math.isfinite(entry.get(key, default)) or entry.get(key, default) <= 0
               for key, default in [('radius', 8), ('above', 3)]):
            raise ValueError('invalid moby seed extent')
        out[cls] = entry
    return out


class Ground:
    """XY hash of geometric walkable triangles, independent of normals/winding."""
    cell = 16

    def __init__(self, triangles):
        self.tiles = {}
        for triangle in triangles:
            a, b, c = triangle
            if not all(math.isfinite(x) for p in triangle for x in p):
                continue
            u, v = [b[i]-a[i] for i in range(3)], [c[i]-a[i] for i in range(3)]
            n = [u[1]*v[2]-u[2]*v[1], u[2]*v[0]-u[0]*v[2], u[0]*v[1]-u[1]*v[0]]
            length = math.sqrt(sum(x*x for x in n))
            if length < 1e-8 or abs(n[2]) < 0.6 * length:
                continue
            for x in range(math.floor(min(p[0] for p in triangle)/self.cell),
                           math.floor(max(p[0] for p in triangle)/self.cell)+1):
                for y in range(math.floor(min(p[1] for p in triangle)/self.cell),
                               math.floor(max(p[1] for p in triangle)/self.cell)+1):
                    self.tiles.setdefault((x, y), []).append((a, u, v, n[2]))

    def below(self, position):
        x, y, z = position
        hits = []
        for a, u, v, determinant in self.tiles.get((math.floor(x/self.cell), math.floor(y/self.cell)), []):
            dx, dy = x-a[0], y-a[1]
            s = (dx*v[1]-dy*v[0])/determinant
            t = (u[0]*dy-u[1]*dx)/determinant
            if s >= -1e-6 and t >= -1e-6 and s+t <= 1+1e-6:
                height = a[2]+s*u[2]+t*v[2]
                if z-32 <= height <= z+2:
                    hits.append(height)
        return max(hits) if hits else None

    @classmethod
    def read(cls, path: Path):
        return cls(cls.read_triangles(path))

    @staticmethod
    def read_triangles(path: Path):
        sys.path.insert(0, str(Path(__file__).resolve().parents[4]/'editor'))
        import gltf_read
        doc = gltf_read.read(path)
        triangles = []
        for index, node in enumerate(doc.list('nodes')):
            if 'mesh' not in node:
                continue
            world = doc.node_world(index)
            for primitive in doc.list('meshes')[node['mesh']]['primitives']:
                if primitive.get('mode', 4) != 4:
                    raise ValueError('collision seed projection requires triangles')
                vertices = [gltf_read.point(world, p) for p in doc.accessor(primitive['attributes']['POSITION'])]
                indices = ([p[0] for p in doc.accessor(primitive['indices'])]
                           if 'indices' in primitive else list(range(len(vertices))))
                if len(indices) % 3:
                    raise ValueError('truncated collision triangle')
                triangles.extend([vertices[i] for i in indices[a:a+3]] for a in range(0, len(indices), 3))
        return triangles


def piston_travel(gameplay: bytes, moby: dict, game_version: str, level: int):
    """Known Novalis LiftVars layout, checked against the level-01 update C.

    Do not interpret this layout in unrelated levels or versions. Initial mode
    zero subtracts travel; mode one adds it. Both Novalis piston classes share
    FUN_L01_002f95c0 (config/overlays/us/names/level-01.json).
    """
    if game_version != 'rac1-ntsc' or level != 1 or moby['class'] not in (703, 715):
        return None
    index = moby.get('fields', {}).get('pvar_index', -1)
    if index < 0 or len(gameplay) < 0x94:
        return None
    table, data = struct.unpack_from('<2I', gameplay, 0x54)
    later = [p for p in struct.unpack_from('<37I', gameplay) if p > table]
    end = min([len(gameplay)]+later)
    if table < 0x94 or data < 0x94 or table+8*(index+1) > end:
        return None
    offset, size = struct.unpack_from('<2i', gameplay, table+8*index)
    data_end = min([len(gameplay)]+[p for p in struct.unpack_from('<37I', gameplay) if p > data])
    if offset < 0 or size < 0xb8 or data+offset+size > data_end:
        return None
    at = data+offset
    mode = gameplay[at+0xa4]
    travel, = struct.unpack_from('<f', gameplay, at+0xa8)
    period, = struct.unpack_from('<f', gameplay, at+0xb0)
    if mode not in (0, 1) or not math.isfinite(travel) or not 0 <= travel <= 128 or not math.isfinite(period) or period <= 0:
        return None
    return -travel if mode == 0 else travel


def marker_bounds(position, floor, travel, entry, vertical_spacing=4):
    if not math.isfinite(vertical_spacing) or vertical_spacing <= 0:
        raise ValueError('invalid vertical seed spacing')
    z = position[2]
    low = min(z, floor if floor is not None else z, z+(travel or 0))-max(2, vertical_spacing)
    high = max(z, z+(travel or 0))+max(entry.get('above', 3), vertical_spacing)
    radius = entry['radius']
    return [position[0]-radius, position[1]-radius, low,
            position[0]+radius, position[1]+radius, high]


def candidates(placements, allowed, ground, gameplay=b'', game_version='', level=-1, vertical_spacing=4):
    """Only initial exported placements; runtime crate drops cannot become seeds."""
    seeds, boxes, skipped = [], [], []
    for moby in placements['mobys']:
        entry = allowed.get(moby['class'])
        if entry is None:
            continue
        position = moby['matrix'][12:15]
        if len(position) != 3 or not all(math.isfinite(x) for x in position):
            skipped.append({'moby': moby['index'], 'reason': 'nonfinite placement'})
            continue
        floor = ground.below(position)
        travel = piston_travel(gameplay, moby, game_version, level)
        bounds = marker_bounds(position, floor, travel, entry, vertical_spacing)
        source = {'index': moby['index'], 'class': moby['class'], 'name': entry['name'],
                  'kind': entry.get('kind', 'gameplay'),
                  'position': position, 'ground_z': floor, 'piston_travel_z': travel,
                  'bounds': bounds}
        seeds.append(source)
        boxes.append({'bounds': bounds, 'source_mobys': [moby['index']], 'source_transitions': []})
    return boxes, seeds, skipped


def combine(authored, mobys, coarse, spacing, budget):
    """Greedy compact overlap merging; no one-volume-per-crate explosion.

    Every original box remains covered. Merge growth is bounded to limit empty
    space and avoid connecting entire levels via collectible trails.
    """
    from seed_regions import axis_spacing, probe_count, split_bounds
    spacing = axis_spacing(spacing)
    lower = coarse['origin']
    upper = [lower[a]+coarse['spacing'][a]*(coarse['dimensions'][a]-1) for a in range(3)]
    boxes = [{'bounds': r['bounds'], 'source_mobys': [],
              'source_transitions': [r['source_transition']]} for r in authored]
    boxes += mobys
    regions, skipped = [], []
    for box in boxes:
        bounds = [max(math.floor(box['bounds'][a]/spacing[a])*spacing[a], math.ceil(lower[a]/spacing[a])*spacing[a])
                  for a in range(3)]
        bounds += [min(math.ceil(box['bounds'][a+3]/spacing[a])*spacing[a], math.floor(upper[a]/spacing[a])*spacing[a])
                   for a in range(3)]
        if any(bounds[a+3]-bounds[a] < spacing[a] for a in range(3)):
            skipped.append({**box, 'reason': 'outside sparse grid or no usable extent'})
            continue
        regions.append({**box, 'bounds': bounds})
    # Best-first merging is deterministic and favours tight clusters first.
    while True:
        best = None
        for i, a in enumerate(regions):
            for j in range(i+1, len(regions)):
                b = regions[j]
                if any(a['bounds'][k] > b['bounds'][k+3] or b['bounds'][k] > a['bounds'][k+3] for k in range(3)):
                    continue
                merged = [min(a['bounds'][k], b['bounds'][k]) for k in range(3)]
                merged += [max(a['bounds'][k+3], b['bounds'][k+3]) for k in range(3)]
                count = probe_count(merged, spacing)
                score = count/(probe_count(a['bounds'], spacing)+probe_count(b['bounds'], spacing))
                if count > budget or score > 1.15:
                    continue
                candidate = (score, i, j, merged)
                if best is None or candidate[:3] < best[:3]:
                    best = candidate
        if best is None:
            break
        _, i, j, bounds = best
        a, b = regions[i], regions.pop(j)
        regions[i] = {'bounds': bounds,
                      'source_mobys': sorted(set(a['source_mobys']+b['source_mobys'])),
                      'source_transitions': sorted(set(a['source_transitions']+b['source_transitions']))}
    out = []
    for r in regions:
        for bounds in split_bounds(r['bounds'], spacing, budget):
            out.append({**r, 'bounds': bounds, 'spacing': spacing,
                        'probe_count': probe_count(bounds, spacing)})
    out.sort(key=lambda r: r['bounds'])
    return out, skipped
