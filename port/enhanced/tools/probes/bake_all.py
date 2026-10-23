#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""Bake and verify sparse/dense probes for every exported level in one version.

Uses each level's settings and preserves previous bakes. Outputs contain game
data and must stay local. Levels run sequentially; each bake can use all CPUs.
"""
from __future__ import annotations

import argparse
import json
import os
from pathlib import Path
import re
import subprocess
import sys

import bake_level
import seed_regions as seeds


def discover_levels(levels_dir: Path):
    levels = sorted((path for path in levels_dir.iterdir()
                     if path.is_dir() and re.fullmatch(r'level_\d+', path.name)),
                    key=lambda path: (int(path.name[6:]), path.name))
    if not levels:
        raise ValueError(f'no exported level directories found in {levels_dir}')
    for level in levels:
        if not (level/'manifest.json').is_file():
            raise ValueError(f'missing export manifest: {level}/manifest.json')
    return levels


def bake_all(levels_dir: Path, output_dir: Path, baker: Path, *, disc=None,
             options=(), dry_run=False):
    levels_dir, output_dir, baker = levels_dir.resolve(), output_dir.resolve(), baker.resolve()
    levels = discover_levels(levels_dir)
    # Check everything before starting the first potentially expensive bake.
    if output_dir.is_relative_to(levels_dir):
        raise ValueError('bake outputs must be outside the source levels directory')
    if output_dir.exists():
        raise ValueError('choose a new output directory; previous bakes are preserved')
    if not dry_run and not baker.is_file():
        raise ValueError(f'native baker not found: {baker}; build it or pass --baker')
    if dry_run:
        for level in levels:
            print(f'{level} -> {output_dir/level.name}', flush=True)
        print(f'{len(levels)} levels; no probes baked.', flush=True)
        return True

    output_dir.mkdir(parents=True)
    report = {'version': 1, 'levels_dir': str(levels_dir), 'levels': []}

    def save_report():
        (output_dir/'bake-all-result.json').write_text(
            json.dumps(report, indent=2, allow_nan=False)+'\n', encoding='utf-8')

    save_report()
    for index, level in enumerate(levels, 1):
        print(f'[{index}/{len(levels)}] {level.name}', flush=True)
        result = {'level': level.name, 'verified': False}
        report['levels'].append(result)
        save_report()
        try:
            sparse = bake_level.bake_level(level, output_dir/level.name, baker,
                                          disc=disc, options=options)
        except (OSError, ValueError, KeyError, subprocess.CalledProcessError) as error:
            result['error'] = str(error)
            print(f'{level.name}: {error}', file=sys.stderr, flush=True)
        else:
            result.update(verified=True, sparse=sparse.relative_to(output_dir).as_posix())
        save_report()

    passed = sum(result['verified'] for result in report['levels'])
    print(f'Verified {passed}/{len(levels)} levels. Summary: '
          f'{output_dir/"bake-all-result.json"}', flush=True)
    return passed == len(levels)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--levels-dir', type=Path, required=True,
                        help='one exported version containing level_XX directories')
    parser.add_argument('--output-dir', type=Path, required=True,
                        help='new output root; each level gets its own directory')
    parser.add_argument('--baker', type=Path, default=seeds.ROOT/'port/build/release'/
                        ('openrac-probe-bake.exe' if os.name == 'nt' else 'openrac-probe-bake'))
    parser.add_argument('--disc', type=Path, help='fallback for older exports without gameplay')
    parser.add_argument('--rays', type=int, help='quality override for every level')
    parser.add_argument('--bounces', type=int, help='bounce override for every level')
    parser.add_argument('--threads', type=int, help='CPU bake threads; 0 uses all, 1 is serial')
    parser.add_argument('--dry-run', action='store_true', help='list levels without baking or writing')
    args = parser.parse_args()
    options = []
    for key in ('rays', 'bounces', 'threads'):
        value = getattr(args, key)
        if value is not None:
            options.extend(('--'+key, str(value)))
    try:
        success = bake_all(args.levels_dir, args.output_dir, args.baker,
                           disc=args.disc, options=options, dry_run=args.dry_run)
    except (OSError, ValueError) as error:
        parser.exit(1, f'{error}\n')
    return 0 if success else 1


if __name__ == '__main__':
    sys.exit(main())
