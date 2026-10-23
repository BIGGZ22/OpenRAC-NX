# SPDX-License-Identifier: GPL-3.0-or-later
"""Extend gameplay probe regions to nearby collision walls and ceilings."""
from __future__ import annotations

import math


def clip_footprint(triangle, bounds):
    """Clip in world XY, interpolating Z; distant triangle vertices do not count."""
    polygon = list(triangle)
    for axis, edge, direction in ((0, bounds[0], 1), (0, bounds[3], -1),
                                   (1, bounds[1], 1), (1, bounds[4], -1)):
        clipped = []
        if not polygon:
            break
        previous = polygon[-1]
        before = (previous[axis] - edge) * direction
        for point in polygon:
            after = (point[axis] - edge) * direction
            if (before >= 0) != (after >= 0):
                t = before / (before - after)
                clipped.append([previous[a] + t * (point[a] - previous[a]) for a in range(3)])
            if after >= 0:
                clipped.append(point)
            previous, before = point, after
        polygon = clipped
    return polygon


def extend_regions(regions, triangles, coarse, spacing, budget=4096, max_rise=32, padding=3):
    # Imported here because seed_regions also uses this module from its CLI.
    from seed_regions import axis_spacing, probe_count, split_bounds
    spacing = axis_spacing(spacing)
    if not math.isfinite(max_rise) or max_rise <= 0 or not math.isfinite(padding) or padding < 0:
        raise ValueError('invalid geometry coverage settings')
    padding = max(padding, spacing[2])  # At least one vertical cell beyond the surface.
    ceiling = (coarse['origin'][2]
               + coarse['spacing'][2] * (coarse['dimensions'][2] - 1))
    ceiling = math.floor(ceiling / spacing[2]) * spacing[2]
    surfaces = []
    for triangle in triangles:
        if len(triangle) != 3 or any(len(p) != 3 for p in triangle):
            raise ValueError('coverage requires triangles')
        if not all(math.isfinite(v) for p in triangle for v in p):
            raise ValueError('nonfinite coverage geometry')
        u = [triangle[1][a] - triangle[0][a] for a in range(3)]
        v = [triangle[2][a] - triangle[0][a] for a in range(3)]
        normal = [u[1]*v[2]-u[2]*v[1], u[2]*v[0]-u[0]*v[2], u[0]*v[1]-u[1]*v[0]]
        length = math.sqrt(sum(n*n for n in normal))
        if length <= 1e-8:
            continue
        surfaces.append((triangle, [min(p[a] for p in triangle) for a in range(3)],
                         [max(p[a] for p in triangle) for a in range(3)],
                         abs(normal[2]) >= 0.6 * length))
    out, diagnostics = [], []
    for index, region in enumerate(regions):
        original = region['bounds']
        limit = min(ceiling, math.floor((original[5] + max_rise) / spacing[2]) * spacing[2])
        highest, hits, limited = original[5] - padding, 0, 0
        for triangle, low, high, horizontal in surfaces:
            if (high[0] < original[0] or low[0] > original[3]
                    or high[1] < original[1] or low[1] > original[4]
                    or high[2] <= original[5] - padding or low[2] > limit):
                continue
            clipped = clip_footprint(triangle, original)
            if not clipped:
                continue
            bottom, top = min(p[2] for p in clipped), max(p[2] for p in clipped)
            # Wall faces must connect to the original gameplay height band.
            # Horizontal/sloped overhead faces can supply the room's ceiling.
            if (bottom > limit or top < original[2] or top <= original[5] - padding
                    or (not horizontal and bottom > original[5])):
                continue
            hits += 1
            highest = max(highest, top)
            if top + padding > limit:
                limited += 1
        bounds = original.copy()
        bounds[5] = max(original[5], min(limit, math.ceil((highest + padding) / spacing[2]) * spacing[2]))
        diagnostic = {'seed_region': index, 'seed_bounds': original.copy(), 'bounds': bounds,
                      'nearby_surfaces': hits, 'highest_surface_z': highest if hits else None,
                      'limited_surfaces': limited,
                      'height_limit': limit, 'changed': bounds != original}
        diagnostics.append(diagnostic)
        for part in split_bounds(bounds, spacing, budget):
            out.append({**region, 'bounds': part, 'spacing': spacing,
                        'probe_count': probe_count(part, spacing), 'geometry_coverage': diagnostic})
    return out, diagnostics
