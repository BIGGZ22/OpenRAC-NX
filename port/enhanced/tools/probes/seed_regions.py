#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""Seed bounded GI bakes from authored lighting cuboids and gameplay mobys.

Offline only: reads exported gameplay, with an optional older-export disc
fallback through the editor's readers. No game runs or images. Outputs stay local.
The transition format is documented by assets/world/gameplay.h/.cpp.
"""
from __future__ import annotations

import argparse
import hashlib
import itertools
import json
import math
import os
from pathlib import Path
import struct
import subprocess
import sys
import tempfile

sys.path.insert(1, str(next(p for p in Path(__file__).resolve().parents if (p / "tools/openrac_common").is_dir()) / "tools"))
from openrac_common import digest, paths  # noqa: E402

ROOT = paths.repo_root(__file__)


def transitions(gameplay: bytes) -> list[dict]:
    if len(gameplay) < 37 * 4:
        raise ValueError("truncated gameplay section table")
    offset, = struct.unpack_from('<I', gameplay, 0x80)
    if not offset:
        return []
    if offset + 16 > len(gameplay):
        raise ValueError("truncated transition header")
    count, = struct.unpack_from('<i', gameplay, offset)
    if count < 0 or offset + 16 + count * (16 + 128) > len(gameplay):
        raise ValueError("truncated transition records")
    records = offset + 16 + count * 16
    out = []
    for i in range(count):
        at = records + 128 * i
        flags, = struct.unpack_from('<I', gameplay, at + 0x50)
        lights = struct.unpack_from('<2i', gameplay, at + 0x48)
        out.append({'index': i, 'flags': flags,
                    'inverse': struct.unpack_from('<16f', gameplay, at),
                    'light_sets': [11 if n < 0 else n for n in lights],
                    'colours': list(struct.unpack_from('<2I', gameplay, at + 0x40))})
    return out


def cuboid_bounds(matrix: list[float]) -> tuple[list[float], list[float]]:
    """Invert world-to-local row-vector axes, then transform all eight corners."""
    if len(matrix) != 16 or not all(math.isfinite(x) for x in matrix):
        raise ValueError("nonfinite transition transform")
    # l[j] = sum(world[k]*matrix[4*k+j]) + matrix[12+j].
    rows = [[matrix[4 * k + j] for k in range(3)]
            + [float(k == j) for k in range(3)] for j in range(3)]
    for col in range(3):
        pivot = max(range(col, 3), key=lambda r: abs(rows[r][col]))
        if abs(rows[pivot][col]) < 1e-12:
            raise ValueError("singular transition transform")
        rows[col], rows[pivot] = rows[pivot], rows[col]
        scale = rows[col][col]
        rows[col] = [v / scale for v in rows[col]]
        for r in range(3):
            if r != col:
                scale = rows[r][col]
                rows[r] = [a - scale * b for a, b in zip(rows[r], rows[col])]
    corners = [[sum(rows[r][3 + k] * (p[k] - matrix[12 + k]) for k in range(3))
                for r in range(3)] for p in itertools.product((-1, 1), repeat=3)]
    return ([min(p[a] for p in corners) for a in range(3)],
            [max(p[a] for p in corners) for a in range(3)])


def axis_spacing(spacing) -> list[float]:
    values = [spacing]*3 if isinstance(spacing, (int, float)) else list(spacing)
    if len(values) != 3 or any(not math.isfinite(s) or s <= 0 for s in values):
        raise ValueError('spacing needs three positive finite axis steps')
    return values


def probe_count(bounds: list[float], spacing) -> int:
    steps = axis_spacing(spacing)
    return math.prod(round((bounds[a + 3] - bounds[a]) / steps[a]) + 1 for a in range(3))


def split_bounds(bounds: list[float], spacing: float, budget: int) -> list[list[float]]:
    """Split large AABBs with a cell of overlap on each side of each cut."""
    if probe_count(bounds, spacing) <= budget:
        return [bounds]
    steps = axis_spacing(spacing)
    cells = [round((bounds[a + 3] - bounds[a]) / steps[a]) for a in range(3)]
    axis = max(range(3), key=lambda a: cells[a])
    if cells[axis] < 4:
        raise ValueError("region probe budget is too small")
    middle = bounds[axis] + (cells[axis] // 2) * steps[axis]
    left, right = bounds.copy(), bounds.copy()
    left[axis + 3], right[axis] = middle + steps[axis], middle - steps[axis]
    return split_bounds(left, spacing, budget) + split_bounds(right, spacing, budget)


def plan_regions(records: list[dict], coarse: dict, spacing: float = 4, padding: float = 8,
                 budget: int = 4096, vertical_padding: float | None = None) -> tuple[list[dict], list[dict]]:
    spacing = axis_spacing(spacing)
    padding = [padding, padding, padding if vertical_padding is None else vertical_padding]
    if any(not math.isfinite(p) or p < 0 for p in padding) or budget < 64:
        raise ValueError("invalid dense region settings")
    if any(spacing[a] >= coarse['spacing'][a] for a in range(3)):
        raise ValueError("region spacing must be finer than the level grid")
    lower = coarse['origin']
    upper = [lower[a] + coarse['spacing'][a] * (coarse['dimensions'][a] - 1) for a in range(3)]
    regions, skipped = [], []
    for record in records:
        if not record['flags'] & 1:
            skipped.append({'source': record['index'], 'reason': 'no hero-light transition flag'})
            continue
        try:
            lo, hi = cuboid_bounds(record['inverse'])
            # Padding includes surfaces on both sides of the local-X transition.
            # Align inward to coarse bounds so every dense region passes pair validation.
            lo = [max(math.floor((lo[a] - padding[a]) / spacing[a]) * spacing[a],
                      math.ceil(lower[a] / spacing[a]) * spacing[a]) for a in range(3)]
            hi = [min(math.ceil((hi[a] + padding[a]) / spacing[a]) * spacing[a],
                      math.floor(upper[a] / spacing[a]) * spacing[a]) for a in range(3)]
            if any(hi[a] - lo[a] < spacing[a] for a in range(3)):
                raise ValueError("outside sparse grid or no usable extent")
            for bounds in split_bounds(lo + hi, spacing, budget):
                regions.append({'source_transition': record['index'], 'bounds': bounds,
                                'spacing': spacing, 'probe_count': probe_count(bounds, spacing),
                                'light_sets': record['light_sets'], 'colours': record['colours']})
        except ValueError as why:
            skipped.append({'source': record['index'], 'reason': str(why)})
    return regions, skipped


def coarse_header(path: Path) -> dict:
    # Only metadata is read here. The baker's --verify performs full asset,
    # scene/settings and payload validation before the optional batch bake.
    with path.open('rb') as source:
        header = source.read(256)
    magic, version = struct.unpack_from('<2I', header)
    if magic != 0x5047524f or version != 3:
        raise ValueError("unsupported sparse asset header")
    bake_version = struct.unpack_from('<I', header, 8)[0]
    name_at = 16
    size, = struct.unpack_from('<I', header, name_at - 4)
    if bake_version != 1 or not 1 <= size <= 64:
        raise ValueError("unsupported sparse asset header")
    at = name_at + size
    level, = struct.unpack_from('<I', header, at)
    origin = list(struct.unpack_from('<3f', header, at + 20))
    spacing = list(struct.unpack_from('<3f', header, at + 32))
    dimensions = list(struct.unpack_from('<3I', header, at + 44))
    maximum, = struct.unpack_from('<f', header, at + 60)
    return {
        'game_version': header[name_at:name_at + size].decode('ascii'),
        'level': level,
        'origin': origin,
        'spacing': spacing,
        'dimensions': dimensions,
        'max_distance': maximum,
        'probe_format_version': version,
        'bake_version': bake_version,
    }


def register_regions(coarse: Path, assets: list[Path], backup_dir: Path | None = None) -> Path | None:
    if not assets:
        return None  # No seeds: preserve every existing region choice.
    sidecar = Path(str(coarse) + '.regions.json')
    payload = json.dumps({'regions': [os.path.relpath(file.resolve(), sidecar.parent.resolve())
                                     for file in assets]}, indent=2)
    if backup_dir is None:
        with sidecar.open('x') as out:
            out.write(payload)
        return sidecar
    # Explicit regeneration only, called after every bake and pair check succeeds.
    # Keep the old registration alongside the new experiment; old assets survive.
    if sidecar.exists():
        with (backup_dir/'previous-regions.json').open('xb') as out:
            out.write(sidecar.read_bytes())
    with tempfile.NamedTemporaryFile(mode='w', dir=sidecar.parent, prefix=sidecar.name+'.',
                                     suffix='.tmp', delete=False) as out:
        temporary = Path(out.name)
        out.write(payload)
        out.flush()
        os.fsync(out.fileno())
    try:
        os.replace(temporary, sidecar)
    finally:
        temporary.unlink(missing_ok=True)
    return sidecar


def profile_spacing(profile: dict, horizontal=None, vertical=None) -> list[float]:
    """Resolved C++ profile defaults, then explicit planner overrides."""
    spacing = axis_spacing(profile['dense_spacing'])
    if horizontal is not None:
        spacing[0] = spacing[1] = horizontal
    if vertical is not None:
        spacing[2] = vertical
    return axis_spacing(spacing)


def read_gameplay(level_dir: Path, manifest: dict, disc_path: Path | None = None) -> bytes | None:
    """Prefer normal exported inputs; disc access is only for older exports. None for a scene
    that is no level of the disc's own (it has no gameplay)."""
    tag = manifest.get('bake_inputs', {}).get('gameplay')
    if tag is not None:
        if tag != 'rac1-v1':
            raise ValueError('unsupported gameplay bake input')
        return (level_dir/'bake-inputs/gameplay.bin').read_bytes()
    if disc_path is None:
        raise ValueError('missing gameplay bake input; re-extract the level or pass --disc for an older export')
    sys.path.insert(0, str(ROOT/'editor'))
    from disc import Disc
    from level import decoded
    with Disc(disc_path) as disc:
        if manifest['game'] != 'rac1' or disc.version.key != manifest['version']:
            raise ValueError('disc and exported level differ')
        level = next((entry for entry in disc.survey()['levels'] if entry['id'] == manifest['level']), None)
        if level is None:
            # The title screen (99) and the flight between planets (98) are scenes the extractor
            # builds from other parts of the disc: they have no gameplay file to seed from.
            return None
        span = level['ranges'][disc.version.gameplay]
        return decoded(disc.sectors(span['lba'], span['sectors']))


def plan_level(level_dir: Path, coarse: dict, profile: dict, gameplay: bytes, options) -> dict:
    """Shared bake-time planner: authored volumes, gameplay seeds and surfaces."""
    manifest = json.loads((level_dir/'manifest.json').read_text())
    if manifest['game'] != 'rac1' or manifest['version'] not in ('pal', 'ntsc'):
        raise ValueError('authored seeding currently supports RAC1 PAL/NTSC only')
    if coarse['level'] != manifest['level'] or coarse['game_version'] != f"{manifest['game']}-{manifest['version']}":
        raise ValueError('sparse asset and level identity differ')
    records = transitions(gameplay)
    spacing = profile_spacing(profile, options.spacing, options.vertical_spacing)
    regions, skipped = plan_regions(records, coarse, spacing, options.padding, options.max_probes,
                                    options.vertical_padding)
    markers = []
    whitelist_hash = None
    geometry = None
    geometry_hash = None
    coverage = []
    import moby_seeds
    if not options.no_moby_seeds or (regions and not options.no_geometry_coverage):
        geometry = moby_seeds.Ground.read_triangles(level_dir/'collision.glb')
        geometry_hash = digest.sha256(level_dir/'collision.glb')
    if not options.no_moby_seeds:
        allowed = moby_seeds.whitelist(options.moby_whitelist)
        whitelist_hash = digest.sha256(options.moby_whitelist)
        placements = json.loads((level_dir/'placements.json').read_text())
        ground = moby_seeds.Ground(geometry)
        boxes, markers, bad_mobys = moby_seeds.candidates(placements, allowed, ground, gameplay,
                                                       coarse['game_version'], manifest['level'], spacing[2])
        regions, clipped = moby_seeds.combine(regions, boxes, coarse, spacing, options.max_probes)
        skipped += bad_mobys+clipped
    if regions and not options.no_geometry_coverage:
        from surface_coverage import extend_regions
        regions, coverage = extend_regions(regions, geometry, coarse, spacing, options.max_probes,
                                            options.geometry_max_rise, options.geometry_padding)
    report = {'level': manifest['level'], 'game_version': coarse['game_version'],
              'source': 'authored environment transitions (lighting bit 0) and whitelisted initial moby placements',
              'gameplay_sha256': hashlib.sha256(gameplay).hexdigest(),
              'bake_profile': profile, 'authored_count': len(records), 'moby_whitelist_sha256': whitelist_hash,
              'moby_seeds': markers, 'regions': regions, 'skipped': skipped,
              'geometry_coverage': coverage,
              'geometry_sha256': geometry_hash,
              'note': 'Light-bank IDs are seed evidence; bake transport uses the sparse bake lighting inputs.'}
    return report


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--disc', type=Path, help='legacy export fallback; normal exports retain gameplay input')
    parser.add_argument('--level-dir', type=Path, required=True)
    parser.add_argument('--coarse', type=Path, required=True)
    parser.add_argument('--output-dir', type=Path, required=True)
    parser.add_argument('--spacing', type=float, help='horizontal spacing override; otherwise level profile')
    parser.add_argument('--vertical-spacing', type=float, help='Z spacing override; otherwise level profile')
    parser.add_argument('--padding', type=float, default=8)
    parser.add_argument('--vertical-padding', type=float, default=4, help='authored cuboid Z padding')
    parser.add_argument('--max-probes', type=int, default=4096)
    parser.add_argument('--no-geometry-coverage', action='store_true',
                        help='disable automatic upper-wall/ceiling height coverage')
    parser.add_argument('--geometry-max-rise', type=float, default=32,
                        help='maximum added region height from nearby geometry')
    parser.add_argument('--geometry-padding', type=float, default=3,
                        help='height beyond nearby surfaces; at least one vertical cell')
    parser.add_argument('--no-moby-seeds', action='store_true')
    parser.add_argument('--moby-whitelist', type=Path, default=Path(__file__).with_name('moby_seed_whitelist.json'))
    parser.add_argument('--replace-registration', action='store_true',
                        help='activate verified replacement bakes, saving the previous registry in the new output directory')
    parser.add_argument('--bake', action='store_true')
    parser.add_argument('--threads', type=int, default=0, help='CPU bake threads; 0 uses all, 1 is serial')
    parser.add_argument('--baker', type=Path,
                        default=ROOT/'port/build/release'/('openrac-probe-bake.exe' if os.name == 'nt' else 'openrac-probe-bake'))
    args = parser.parse_args()
    manifest = json.loads((args.level_dir/'manifest.json').read_text())
    coarse = coarse_header(args.coarse)
    gameplay = read_gameplay(args.level_dir, manifest, args.disc)
    profile = json.loads(subprocess.check_output(
        [str(args.baker.resolve()), '--level-dir', str(args.level_dir.resolve()), '--print-profile'], text=True))
    report = plan_level(args.level_dir, coarse, profile, gameplay, args)
    regions, markers, coverage = report['regions'], report['moby_seeds'], report['geometry_coverage']
    spacing = profile_spacing(profile, args.spacing, args.vertical_spacing)
    if args.bake and regions and not args.replace_registration and Path(str(args.coarse) + '.regions.json').exists():
        raise ValueError("region sidecar already exists; choose a new coarse asset or explicit manifest")
    # New folders only: generated data never overwrites an earlier experiment.
    args.output_dir.mkdir(parents=True, exist_ok=False)
    (args.output_dir/'seed-plan.json').write_text(json.dumps(report, indent=2))
    print(f"Authored transitions: {report['authored_count']}; moby seeds: {len(markers)}; "
          f"dense regions: {len(regions)}; probes: {sum(r['probe_count'] for r in regions)}", flush=True)
    if coverage:
        print(f"Geometry coverage: {sum(r['changed'] for r in coverage)} seed regions extended; "
              f"{sum(r['limited_surfaces'] for r in coverage)} surface candidates hit height limits "
              '(inspect seed-plan.json)', flush=True)
    if not args.bake:
        return
    common = [str(args.baker.resolve()), '--level-dir', str(args.level_dir.resolve())]
    common += ['--threads', str(args.threads)]
    with (args.output_dir/'coarse.verify.log').open('x') as log:
        subprocess.run(common+['--verify', str(args.coarse.resolve()), '--game-rounding'],
                       stdout=log, stderr=subprocess.STDOUT, check=True)
    files = []
    for i, region in enumerate(regions):
        file = args.output_dir/f'region-{i:03d}.orpg'
        options = ['--spacing', ','.join(map(str, spacing)), '--bounds', ','.join(map(str, region['bounds'])),
                   '--max-distance', str(1.5 * math.sqrt(sum(s*s for s in spacing)))]
        with (args.output_dir/f'region-{i:03d}.bake.log').open('x') as log:
            subprocess.run(common+options+['--output', str(file.resolve())],
                           stdout=log, stderr=subprocess.STDOUT, check=True)
        with (args.output_dir/f'region-{i:03d}.verify.log').open('x') as log:
            subprocess.run(common+options+['--verify', str(file.resolve()), '--game-rounding',
                                           '--compare-coarse', str(args.coarse.resolve())],
                           stdout=log, stderr=subprocess.STDOUT, check=True)
        files.append(file.name)
        print(f"Baked and verified {i+1}/{len(regions)}", flush=True)
    (args.output_dir/'regions.json').write_text(json.dumps({'regions': files}, indent=2))
    registered = register_regions(args.coarse, [args.output_dir/file for file in files],
                                  args.output_dir if args.replace_registration else None)
    if registered:
        print('Automatic region sidecar:', registered, flush=True)
    print('Region manifest:', args.output_dir/'regions.json', flush=True)


if __name__ == '__main__':
    main()
