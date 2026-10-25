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
    max_regions: int = 512
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
        if not isinstance(settings.max_regions, int) or not 1 <= settings.max_regions <= 512:
            raise ValueError('max_regions must be between 1 and 512')
        if settings.geometry_max_rise <= 0:
            raise ValueError('geometry_max_rise must be positive')
        return settings

    def planner_options(self):
        return SimpleNamespace(**asdict(self), spacing=None, vertical_spacing=None,
                               moby_whitelist=Path(__file__).with_name('moby_seed_whitelist.json'))


def write_json(path, value, *, replace=False):
    if replace:
        path.unlink(missing_ok=True)
    with path.open('x') as out:
        json.dump(value, out, indent=2, allow_nan=False)
        out.write('\n')


def remove_grids(directory: Path):
    """The grids a stopped bake wrote for another plan: every .orpg and its report."""
    for path in directory.glob('*.orpg*'):
        path.unlink()


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


class NativeBatch:
    """Keep one native scene alive while the Python planner chooses its grids."""

    def __init__(self, command, log_path, *, append=False, progress=None):
        self.command = command
        self.log_path = log_path
        self.append = append
        # progress(fraction): the share of the level's probes baked, from the baker's "Probes" lines.
        self.progress = progress
        self.process = None
        self.log = None
        self.inspection = ''

    def __enter__(self):
        self.log = self.log_path.open('a' if self.append else 'x')
        try:
            self.process = subprocess.Popen(self.command, stdin=subprocess.PIPE,
                                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                                            text=True, bufsize=1)
            lines = []
            for line in self.process.stdout:
                self.log.write(line)
                self.log.flush()
                if line.strip() == 'Probe batch ready':
                    self.inspection = ''.join(lines)
                    return self
                lines.append(line)
            self.process.wait()
            raise subprocess.CalledProcessError(self.process.returncode or 1, self.command)
        except BaseException:
            self.close()
            raise

    def finish(self, plan):
        self.process.stdin.write(json.dumps(plan, allow_nan=False)+'\n')
        self.process.stdin.close()
        completed = False
        for line in self.process.stdout:
            self.log.write(line)
            self.log.flush()
            if line.startswith(('Baking ', 'Verified ', 'Kept ')):
                print(line.rstrip(), flush=True)
            elif line.startswith('Probes ') and self.progress is not None:
                done, _, total = line[7:].strip().partition('/')
                if done.isdigit() and total.isdigit() and int(total) > 0:
                    self.progress(int(done) / int(total))
            completed |= line.strip() == 'Probe batch complete'
        result = self.process.wait()
        if result or not completed:
            raise subprocess.CalledProcessError(result or 1, self.command)

    def close(self):
        if self.process is not None:
            if self.process.poll() is None:
                self.process.terminate()
                try:
                    self.process.wait(timeout=10)
                except subprocess.TimeoutExpired:
                    self.process.kill()
                    self.process.wait()
            for stream in (self.process.stdin, self.process.stdout):
                if stream is not None:
                    stream.close()
        if self.log is not None:
            self.log.close()

    def __exit__(self, *_):
        self.close()


def bake_level(level_dir: Path, output_dir: Path, baker: Path, *, disc=None, options=(), resume=False,
               progress=None):
    """RESUME: OUTPUT_DIR may hold what a stopped bake of this level wrote; the grids it verified
    for the same plan are kept (the baker reads each back and checks it), the rest is baked.
    PROGRESS(fraction) hears how far the level's probes are."""
    level_dir, output_dir, baker = level_dir.resolve(), output_dir.resolve(), baker.resolve()
    if output_dir.is_relative_to(level_dir):
        raise ValueError('bake outputs must be outside the source level directory')
    if output_dir.exists() and not resume:
        raise ValueError('choose a new output directory; previous bakes are preserved')
    manifest = json.loads((level_dir/'manifest.json').read_text())
    settings = RegionSettings.load(level_dir)
    gameplay = seeds.read_gameplay(level_dir, manifest, disc) if settings.enabled else None
    common = [str(baker), '--level-dir', str(level_dir), *options]
    profile = json.loads(subprocess.check_output(common+['--print-profile'], text=True))
    output_dir.mkdir(parents=True, exist_ok=resume)

    command = common + ['--batch-output', str(output_dir)] + (['--resume'] if resume else [])
    with NativeBatch(command, output_dir/'batch.log', append=resume, progress=progress) as native:
        coarse, emission, source_hash = inspection(native.inspection)
        if settings.enabled and gameplay is not None:
            plan = seeds.plan_level(level_dir, coarse, profile, gameplay, settings.planner_options())
        elif settings.enabled:
            plan = {'regions': [], 'note': 'No gameplay to seed dense regions from (a scene, not a level of the disc).'}
        else:
            plan = {'regions': [], 'note': 'Dense regions disabled by the per-level profile.'}
        if len(plan['regions']) > settings.max_regions:
            raise ValueError('planned dense regions exceed the runtime region limit; adjust the level planning settings')
        plan.update(region_settings=asdict(settings), bake_profile=profile,
                    scene_hash=source_hash, emission=emission)
        seed_plan = output_dir/'seed-plan.json'
        if resume and seed_plan.is_file():
            try:
                same = json.loads(seed_plan.read_text()) == json.loads(json.dumps(plan, allow_nan=False))
            except ValueError:
                same = False
            if not same:
                remove_grids(output_dir)
        write_json(seed_plan, plan, replace=resume)
        print(f"Planned {len(plan['regions'])} dense regions", flush=True)

        regions = []
        for region in plan['regions']:
            spacing = seeds.axis_spacing(region['spacing'])
            regions.append({'bounds': region['bounds'], 'spacing': spacing,
                            'max_distance': 1.5*math.sqrt(sum(s*s for s in spacing))})
        native.finish({'version': 1, 'scene_hash': source_hash, 'regions': regions})

    sparse = output_dir/'sparse.orpg'
    assets = [output_dir/f'region-{i:03d}.orpg' for i in range(len(plan['regions']))]
    registry = {'regions': [path.name for path in assets]}
    write_json(Path(str(sparse)+'.regions.json'), registry, replace=resume)
    write_json(output_dir/'regions.json', registry, replace=resume)
    write_json(output_dir/'bake-result.json', replace=resume, value={
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
