#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""Bake one exported level: emission, region planning, transport and validation.

Python standard library only; ray tracing uses the native Embree baker.
Generated plans, logs and probe assets contain game data and must stay local.
"""
from __future__ import annotations

import argparse
from dataclasses import asdict, dataclass
import json
import math
import os
from pathlib import Path
import subprocess
from types import SimpleNamespace

import seed_regions as seeds


@dataclass
class RegionSettings:
    enabled: bool = True
    padding: float = 8
    vertical_padding: float = 4
    max_probes: int = 4096
    max_regions: int = 128
    no_geometry_coverage: bool = False
    geometry_max_rise: float = 32
    geometry_padding: float = 3
    no_moby_seeds: bool = False

    @classmethod
    def load(cls, level_dir: Path):
        path = level_dir/'probe-regions.json'
        data = json.loads(path.read_text()) if path.exists() else {'version': 1}
        if not isinstance(data, dict) or data.get('version') != 1:
            raise ValueError('probe-regions.json requires version 1')
        values = {k: v for k, v in data.items() if k != 'version'}
        if values.keys() - cls.__dataclass_fields__.keys():
            raise ValueError('unknown region planning setting')
        settings = cls(**values)
        defaults = asdict(cls())
        for key, value in asdict(settings).items():
            if isinstance(defaults[key], bool):
                if not isinstance(value, bool):
                    raise ValueError(f'{key} requires a boolean')
            elif isinstance(value, bool) or not isinstance(value, (int, float)) or not math.isfinite(value):
                raise ValueError(f'{key} requires a finite number')
            elif value < 0:
                raise ValueError(f'{key} must not be negative')
        if not isinstance(settings.max_probes, int) or settings.max_probes < 64:
            raise ValueError('max_probes must be an integer of at least 64')
        if not isinstance(settings.max_regions, int) or not 1 <= settings.max_regions <= 128:
            raise ValueError('max_regions must be between 1 and 128')
        if settings.geometry_max_rise <= 0:
            raise ValueError('geometry_max_rise must be positive')
        return settings

    def planner_options(self):
        return SimpleNamespace(**asdict(self), spacing=None, vertical_spacing=None,
                               moby_whitelist=Path(__file__).with_name('moby_seed_whitelist.json'))


def write_json(path, value):
    with path.open('x') as out:
        json.dump(value, out, indent=2, allow_nan=False)
        out.write('\n')


def inspection(text):
    fields = {line.split(' ', 1)[0]: line.split(' ', 1)[1]
              for line in text.splitlines() if ' ' in line}
    grid = {'game_version': fields['game_version'], 'level': int(fields['level']),
            'origin': list(map(float, fields['probe_origin'].split())),
            'spacing': list(map(float, fields['probe_spacing'].split())),
            'dimensions': list(map(int, fields['probe_dimensions'].split()))}
    emission = {key: fields[key] for key in (
        'emissive_triangles', 'automatic_liquid_triangles', 'point_emitters',
        'bake_surfaces', 'emission', 'emission_masks', 'emission_vertex_colour_triangles') if key in fields}
    return grid, emission, fields['scene_hash']


def bake_level(level_dir: Path, output_dir: Path, baker: Path, *, disc=None, options=()):
    level_dir, output_dir, baker = level_dir.resolve(), output_dir.resolve(), baker.resolve()
    if output_dir.is_relative_to(level_dir):
        raise ValueError('bake outputs must be outside the source level directory')
    if output_dir.exists():
        raise ValueError('choose a new output directory; previous bakes are preserved')
    manifest = json.loads((level_dir/'manifest.json').read_text())
    settings = RegionSettings.load(level_dir)
    gameplay = seeds.read_gameplay(level_dir, manifest, disc) if settings.enabled else None
    common = [str(baker), '--level-dir', str(level_dir), *options]
    profile = json.loads(subprocess.check_output(common+['--print-profile'], text=True))
    output_dir.mkdir(parents=True)

    def run(name, arguments):
        print(name, flush=True)
        log_path = output_dir/(name+'.log')
        with log_path.open('x') as log:
            subprocess.run(common+arguments, stdout=log, stderr=subprocess.STDOUT, check=True)
        return log_path.read_text()

    # Scene conversion resolves shared emission colours/masks and liquid analysis
    # here, before any grid traces. The same native conversion feeds every bake.
    coarse, emission, source_hash = inspection(run('scene-emission', ['--inspect']))
    if settings.enabled:
        plan = seeds.plan_level(level_dir, coarse, profile, gameplay, settings.planner_options())
    else:
        plan = {'regions': [], 'note': 'Dense regions disabled by the per-level profile.'}
    if len(plan['regions']) > settings.max_regions:
        raise ValueError('planned dense regions exceed the runtime region limit; adjust the level planning settings')
    plan.update(region_settings=asdict(settings), bake_profile=profile,
                scene_hash=source_hash, emission=emission)
    write_json(output_dir/'seed-plan.json', plan)
    print(f"Planned {len(plan['regions'])} dense regions", flush=True)

    sparse = output_dir/'sparse.orpg'
    run('sparse.bake', ['--output', str(sparse)])
    run('sparse.verify', ['--verify', str(sparse), '--game-rounding'])
    assets = []
    for i, region in enumerate(plan['regions']):
        name = f'region-{i:03d}'
        asset = output_dir/(name+'.orpg')
        spacing = seeds.axis_spacing(region['spacing'])
        local = ['--spacing', ','.join(map(str, spacing)),
                 '--bounds', ','.join(map(str, region['bounds'])),
                 '--max-distance', str(1.5*math.sqrt(sum(s*s for s in spacing)))]
        run(name+'.bake', local+['--output', str(asset)])
        run(name+'.verify', local+['--verify', str(asset), '--game-rounding', '--compare-coarse', str(sparse)])
        assets.append(asset)

    # Recheck source identity before writing the runtime registration. A failed
    # bake/verification leaves diagnostic output, never a partially active batch.
    _, _, final_hash = inspection(run('scene-final', ['--inspect']))
    if final_hash != source_hash:
        raise ValueError('source level changed during the bake; no manifest was registered')
    registry = {'regions': [path.name for path in assets]}
    write_json(Path(str(sparse)+'.regions.json'), registry)
    write_json(output_dir/'regions.json', registry)
    write_json(output_dir/'bake-result.json', {
        'version': 1, 'game_version': coarse['game_version'], 'level': coarse['level'],
        'scene_hash': source_hash, 'sparse': sparse.name, 'regions': registry['regions'],
        'probe_format_version': profile['probe_format_version'],
        'bake_version': profile['bake_version'], 'verified': True, 'emission': emission})
    print('Verified level bake:', sparse, flush=True)
    return sparse


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--level-dir', type=Path, required=True)
    parser.add_argument('--output-dir', type=Path, required=True)
    parser.add_argument('--disc', type=Path, help='fallback for exports made before gameplay retention')
    parser.add_argument('--baker', type=Path, default=seeds.ROOT/'port/build/release'/
                        ('openrac-probe-bake.exe' if os.name == 'nt' else 'openrac-probe-bake'))
    parser.add_argument('--rays', type=int, help='quality override for every grid and verification')
    parser.add_argument('--bounces', type=int, help='bounce override for every grid and verification')
    parser.add_argument('--threads', type=int, help='CPU bake threads; 0 uses all, 1 is serial')
    args = parser.parse_args()
    options = []
    for key in ('rays', 'bounces', 'threads'):
        value = getattr(args, key)
        if value is not None:
            options.extend(('--' + key, str(value)))
    bake_level(args.level_dir, args.output_dir, args.baker, disc=args.disc, options=options)


if __name__ == '__main__':
    main()
