# SPDX-License-Identifier: GPL-3.0-or-later
"""Extend dense probes through enclosed playable collision cells to their first ceiling."""
from __future__ import annotations

import math
from collections import deque, defaultdict
from functools import lru_cache

WALL_DISTANCE = 32
MIN_WALL_DIRECTIONS = 5
MAX_ROOM_SAMPLES = 4096


class Surfaces:
    """Two-sided collision queries shared by every region, indexed in world XY."""
    cell = 16

    def __init__(self, triangles):
        """Index finite nondegenerate triangles; winding never changes enclosure."""
        self.ray = lru_cache(maxsize=65536)(self.ray)
        self.room = lru_cache(maxsize=32768)(self.room)
        self.triangles = []
        self.tiles = defaultdict(list)
        for triangle in triangles:
            if len(triangle) != 3 or any(len(p) != 3 for p in triangle):
                raise ValueError('coverage requires triangles')
            if not all(math.isfinite(v) for p in triangle for v in p):
                raise ValueError('nonfinite coverage geometry')
            a, b, c = triangle
            u = [b[i]-a[i] for i in range(3)]
            v = [c[i]-a[i] for i in range(3)]
            normal = self.cross(u, v)
            if sum(n*n for n in normal) <= 1e-16:
                continue
            index = len(self.triangles)
            self.triangles.append((a, u, v, abs(normal[2]) >= 0.6*math.sqrt(sum(n*n for n in normal))))
            for x in range(math.floor(min(p[0] for p in triangle)/self.cell),
                           math.floor(max(p[0] for p in triangle)/self.cell)+1):
                for y in range(math.floor(min(p[1] for p in triangle)/self.cell),
                               math.floor(max(p[1] for p in triangle)/self.cell)+1):
                    self.tiles[x, y].append(index)

    @staticmethod
    def cross(a, b):
        """Three-dimensional cross product."""
        return (a[1]*b[2]-a[2]*b[1], a[2]*b[0]-a[0]*b[2], a[0]*b[1]-a[1]*b[0])

    @staticmethod
    def dot(a, b):
        """Three-dimensional dot product."""
        return sum(x*y for x, y in zip(a, b))

    def ray(self, origin, direction, distance):
        """Nearest positive hit along a unit ray, or None; down rays require walkable slope."""
        # Bound the XY search; vertical rays only visit one tile.
        end = [origin[i]+direction[i]*distance for i in range(3)]
        candidates = set()
        for x in range(math.floor(min(origin[0], end[0])/self.cell),
                       math.floor(max(origin[0], end[0])/self.cell)+1):
            for y in range(math.floor(min(origin[1], end[1])/self.cell),
                           math.floor(max(origin[1], end[1])/self.cell)+1):
                candidates.update(self.tiles.get((x, y), ()))
        closest = None
        for index in sorted(candidates):
            a, u, v, walkable = self.triangles[index]
            if direction[0] == 0 and direction[1] == 0:
                if direction[2] < 0 and not walkable:
                    continue
                determinant = u[0]*v[1]-u[1]*v[0]
                if abs(determinant) < 1e-9:
                    continue
                dx, dy = origin[0]-a[0], origin[1]-a[1]
                s = (dx*v[1]-dy*v[0])/determinant
                t = (u[0]*dy-u[1]*dx)/determinant
                hit = (a[2]+s*u[2]+t*v[2]-origin[2])/direction[2]
                if s >= -1e-6 and t >= -1e-6 and s+t <= 1+1e-6 and 0.05 < hit <= distance:
                    closest = hit if closest is None else min(closest, hit)
                continue
            h = self.cross(direction, v)
            determinant = self.dot(u, h)
            if abs(determinant) < 1e-9:
                continue
            offset = [origin[i]-a[i] for i in range(3)]
            s = self.dot(offset, h)/determinant
            q = self.cross(offset, u)
            t = self.dot(direction, q)/determinant
            hit = self.dot(v, q)/determinant
            if s >= -1e-6 and t >= -1e-6 and s+t <= 1+1e-6 and 0.05 < hit <= distance:
                closest = hit if closest is None else min(closest, hit)
        return closest

    def room(self, x, y, reference, max_rise, cell):
        """Return local floor/first-ceiling heights only for sufficiently enclosed space."""
        down = self.ray((x, y, reference), (0, 0, -1), 32)
        if down is None:
            return None
        floor = reference-down
        eye = floor+1.5
        roof = self.ray((x, y, eye), (0, 0, 1), max(64, max_rise+32))
        if roof is None or roof < 1.5:
            return None
        # A roof must cover most of the local patch, not just one triangle edge.
        covered = 1
        for dx, dy in ((-cell/4, 0), (cell/4, 0), (0, -cell/4), (0, cell/4)):
            hit = self.ray((x+dx, y+dy, eye), (0, 0, 1), max(64, max_rise+32))
            covered += hit is not None and abs(hit-roof) <= max(4, cell)
        if covered < 4:
            return None
        height = floor+min(3, (roof+1.5)*0.5)
        walls = 0
        for i in range(8):
            angle = i*math.pi/4
            walls += self.ray((x, y, height), (math.cos(angle), math.sin(angle), 0), WALL_DISTANCE) is not None
        if walls < MIN_WALL_DIRECTIONS:
            return None
        return floor, eye+roof


def rectangles(cells):
    """Deterministic rectangles of identically bounded adjacent room cells."""
    remaining = dict(cells)
    while remaining:
        x, y = min(remaining)
        value = remaining[x, y]
        right = x+1
        while remaining.get((right, y)) == value:
            right += 1
        top = y+1
        while all(remaining.get((i, top)) == value for i in range(x, right)):
            top += 1
        for i in range(x, right):
            for j in range(y, top):
                del remaining[i, j]
        yield x, y, right, top, value


def extend_regions(regions, triangles, coarse, spacing, budget=4096, max_rise=32, padding=3):
    """Grow dense coverage through enclosed playable cells, preserving outdoor seeds."""
    from seed_regions import axis_spacing, probe_count, split_bounds
    spacing = axis_spacing(spacing)
    if not math.isfinite(max_rise) or max_rise <= 0 or not math.isfinite(padding) or padding < 0:
        raise ValueError('invalid geometry coverage settings')
    padding = max(padding, spacing[2])
    surfaces = Surfaces(triangles)
    lower = [math.ceil(coarse['origin'][i]/spacing[i])*spacing[i] for i in range(3)]
    upper = [math.floor((coarse['origin'][i]+coarse['spacing'][i]*(coarse['dimensions'][i]-1))/spacing[i])*spacing[i] for i in range(3)]
    cell = [spacing[0]*2, spacing[1]*2]
    out, diagnostics = [], []
    for index, region in enumerate(regions):
        original = region['bounds']
        limit = min(upper[2], original[5]+max_rise)
        cells, queue, seen = {}, deque(), set()
        indoor, limited, discovered = 0, 0, 0
        sample_limit_reached = False
        for x in range(math.floor(original[0]/cell[0]), math.ceil(original[3]/cell[0])):
            for y in range(math.floor(original[1]/cell[1]), math.ceil(original[4]/cell[1])):
                cells[x, y] = (original[2], original[5])
                # Sample occupied height bands, so a tall seed cannot jump over a room.
                reference = original[2]+max(spacing[2], 2)+0.25
                while reference <= original[5]:
                    queue.append((x, y, reference, False))
                    reference += max(spacing[2]*2, 4)
        while queue:
            x, y, reference, grown = queue.popleft()
            key = (x, y, round(reference/spacing[2]))
            if key in seen:
                continue
            if len(seen) >= MAX_ROOM_SAMPLES:
                sample_limit_reached = True
                break
            seen.add(key)
            px, py = (x+.5)*cell[0], (y+.5)*cell[1]
            if not (lower[0]+spacing[0] <= px <= upper[0]-spacing[0]
                    and lower[1]+spacing[1] <= py <= upper[1]-spacing[1]):
                continue
            room = surfaces.room(px, py, reference, max_rise, min(cell))
            if room is None:
                continue
            floor, roof = room
            if floor < original[2]-spacing[2] or floor > original[5]:
                continue
            # Flood only to nearby walkable heights, never directly into another storey.
            if grown and abs(floor-(reference-1)) > max(2, spacing[2]):
                continue
            indoor += 1
            limited += roof+padding > limit
            lo = max(lower[2], math.floor((floor-spacing[2])/spacing[2])*spacing[2])
            hi = max(original[5], min(math.floor(limit/spacing[2])*spacing[2],
                                     math.ceil((roof+padding)/spacing[2])*spacing[2]))
            old = cells.get((x, y))
            if old is not None:
                lo, hi = min(lo, old[0]), max(hi, old[1])
            else:
                discovered += 1
            cells[x, y] = (lo, hi)
            for dx, dy in ((-1,0),(1,0),(0,-1),(0,1)):
                distance = cell[0] if dx else cell[1]
                # Neighbouring enclosed cells need a traversable connection;
                # proximity alone must not flood through a wall into another room.
                if surfaces.ray((px, py, floor+1.5), (dx, dy, 0), distance) is None:
                    queue.append((x+dx, y+dy, floor+1, True))
        diagnostic = {'seed_region': index, 'seed_bounds': original.copy(),
                      'indoor_samples': indoor, 'connected_cells_added': discovered,
                      'limited_surfaces': limited, 'height_limit': limit,
                      'sample_limit_reached': sample_limit_reached,
                      'unresolved_or_outdoor_samples': len(seen)-indoor, 'changed': False}
        if indoor == 0:
            parts = [original.copy()]
        else:
            parts = []
            for x, y, right, top, (lo, hi) in rectangles(cells):
                # One cell of support beyond walls/ceilings avoids fading to sparse there.
                bounds = [max(lower[0], x*cell[0]-spacing[0]),
                          max(lower[1], y*cell[1]-spacing[1]), lo,
                          min(upper[0], right*cell[0]+spacing[0]),
                          min(upper[1], top*cell[1]+spacing[1]), hi]
                if all(bounds[a+3]>bounds[a] for a in range(3)):
                    parts.append(bounds)
            diagnostic['changed'] = parts != [original]
        diagnostic['bounds'] = parts
        diagnostics.append(diagnostic)
        for bounds in parts:
            for part in split_bounds(bounds, spacing, budget):
                out.append({**region, 'bounds': part, 'spacing': spacing,
                            'probe_count': probe_count(part, spacing), 'geometry_coverage': diagnostic})
    # Exact duplicate room pieces need only one bake; preserve their source evidence.
    unique = {}
    for region in out:
        key = tuple(region['bounds'])
        if key not in unique:
            unique[key] = region
        else:
            previous = unique[key]
            for name in ('source_mobys', 'source_transitions'):
                previous[name] = sorted(set(previous.get(name, [])+region.get(name, [])))
    # Reuse the existing bounded overlap merge: no volume exceeds the probe
    # budget, and a union may add at most 15% beyond the pair's probe count.
    from moby_seeds import merge_regions
    pieces = list(unique.values())
    for piece in pieces:
        piece.setdefault('source_mobys', [])
        piece.setdefault('source_transitions', [piece['source_transition']] if 'source_transition' in piece else [])
    merged = merge_regions(pieces, spacing, budget, preserve_height=True)
    for piece in merged:
        piece.update(spacing=spacing, probe_count=probe_count(piece['bounds'], spacing))
    surfaces.ray.cache_clear()
    surfaces.room.cache_clear()
    return sorted(merged, key=lambda piece: piece['bounds']), diagnostics
