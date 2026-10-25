#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""Bake and verify sparse/dense probes for every exported level in one version.

Uses each level's settings. Outputs contain game data and must stay local.
--jobs bakes that many levels at once (0: one per four of the threads the bake
may use, as many as half the memory holds). The bake uses all but a few of the
CPU's threads (openrac_common.background), shared between the levels baking,
at below-normal priority: the computer stays usable while it runs, and the
largest levels start first. Without --update a new output root
is required and previous bakes are preserved. With --update (the launcher's
way) the root is a cache: a level whose export, levels format, baker and
options are those of its last verified bake is kept, any other is baked again
in a folder of its own and put in place only once it is verified.
"""
from __future__ import annotations

import argparse
import hashlib
from concurrent.futures import ThreadPoolExecutor
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import threading

sys.path.insert(1, str(next(p for p in Path(__file__).resolve().parents if (p / "tools/openrac_common").is_dir()) / "tools"))
from openrac_common import background, digest  # noqa: E402

import bake_level  # noqa: E402
import seed_regions as seeds  # noqa: E402

# Each level's record of what its bake was made from (--update compares it).
STAMP = 'openrac-bake.json'
# Options that change the bake's bytes; --threads does not (bake_level preserves output bytes).
BAKE_OPTIONS = ('--rays', '--bounces')
# The threads each level bakes with when the levels are shared out (--jobs 0): enough that the
# largest level, which starts first, is not left on one core long after the others, few enough that
# several levels bake at once. Every baker taking every thread instead (as before 2026-10-24) ran
# one per CPU thread at once, hundreds of threads at normal priority, and left a 16-core desktop
# barely usable for the length of the bake.
THREADS_PER_LEVEL = 4


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


def bake_options(options) -> list[str]:
    """The options that change a bake, in order (for its stamp)."""
    options, out = list(options), []
    for i, option in enumerate(options):
        if option in BAKE_OPTIONS and i + 1 < len(options):
            out += [option, options[i + 1]]
    return out


def planner_hash() -> str:
    digest = hashlib.sha256()
    directory = Path(__file__).parent
    paths = list(directory.glob('*.py'))+[directory/'moby_seed_whitelist.json']
    for path in sorted(paths):
        if not path.name.startswith('test_'):
            digest.update(path.name.encode())
            digest.update(path.read_bytes())
    return digest.hexdigest()


def level_stamp(level: Path, baker_hash: str, options) -> dict:
    """What a level's bake is made from: its export's manifest (which carries the levels
    format), the native baker, planner, per-level settings and options that change the result."""
    manifest = level/'manifest.json'
    try:
        levels_format = json.loads(manifest.read_text()).get('format')
    except (ValueError, AttributeError):
        levels_format = None
    return {'version': 2, 'planner_sha256': planner_hash(),
            'region_settings_sha256': digest.sha256(level/'probe-regions.json') if (level/'probe-regions.json').exists() else None,
            'profile_sha256': digest.sha256(level/'probe-bake.json') if (level/'probe-bake.json').exists() else None,
            'manifest_sha256': digest.sha256(manifest), 'levels_format': levels_format,
            'baker_sha256': baker_hash, 'options': bake_options(options)}


def up_to_date(output: Path, stamp: dict) -> bool:
    try:
        old = json.loads((output/STAMP).read_text())
    except (OSError, ValueError):
        return False
    return old == stamp and (output/'sparse.orpg').is_file() and (output/'bake-result.json').is_file()


# A level's bake takes up to about 0.75 GB at once (a large level's scene, regions and planner).
LEVEL_MEMORY = 1 << 30


def physical_memory() -> int | None:
    """The machine's memory in bytes, or None when it cannot be told."""
    try:
        if os.name == 'nt':
            import ctypes

            class Status(ctypes.Structure):
                _fields_ = [('length', ctypes.c_ulong), ('load', ctypes.c_ulong),
                            ('total', ctypes.c_ulonglong), ('available', ctypes.c_ulonglong),
                            ('page_total', ctypes.c_ulonglong), ('page_available', ctypes.c_ulonglong),
                            ('virtual_total', ctypes.c_ulonglong), ('virtual_available', ctypes.c_ulonglong),
                            ('extended', ctypes.c_ulonglong)]
            status = Status()
            status.length = ctypes.sizeof(Status)
            return status.total if ctypes.windll.kernel32.GlobalMemoryStatusEx(ctypes.byref(status)) else None
        return os.sysconf('SC_PAGE_SIZE') * os.sysconf('SC_PHYS_PAGES')
    except (AttributeError, OSError, ValueError):
        return None


def automatic_jobs() -> int:
    """One level per THREADS_PER_LEVEL of the threads the bake may use, as many as half the
    memory holds."""
    jobs = max(1, background.cpu_budget() // THREADS_PER_LEVEL)
    memory = physical_memory()
    if memory:
        jobs = min(jobs, max(1, memory // 2 // LEVEL_MEMORY))
    return jobs


def with_threads(options, jobs: int) -> list[str]:
    """OPTIONS with --threads sharing the bake's threads (background.cpu_budget) between JOBS
    bakes at once, unless it names a count itself."""
    options = list(options)
    if '--threads' not in options:
        options += ['--threads', str(max(1, background.cpu_budget() // jobs))]
    return options


def level_size(level: Path) -> int:
    """The bytes of a level's export: roughly how long it takes to bake."""
    return sum(path.stat().st_size for path in level.rglob('*') if path.is_file())


def largest_first(levels) -> list[int]:
    """The indices of LEVELS, the largest export first: the longest bakes start first, and the
    small ones fill in around them."""
    return sorted(range(len(levels)), key=lambda i: -level_size(levels[i]))


def bake_all(levels_dir: Path, output_dir: Path, baker: Path, *, disc=None,
             options=(), dry_run=False, update=False, jobs=1):
    levels_dir, output_dir, baker = levels_dir.resolve(), output_dir.resolve(), baker.resolve()
    levels = discover_levels(levels_dir)
    # Check everything before starting the first potentially expensive bake.
    if output_dir.is_relative_to(levels_dir):
        raise ValueError('bake outputs must be outside the source levels directory')
    if output_dir.exists() and not update:
        raise ValueError('choose a new output directory; previous bakes are preserved')
    if not dry_run and not baker.is_file():
        raise ValueError(f'native baker not found: {baker}; build it or pass --baker')
    if jobs <= 0:
        jobs = automatic_jobs()
    jobs = max(1, min(jobs, len(levels)))
    if dry_run:
        for level in levels:
            print(f'{level} -> {output_dir/level.name}', flush=True)
        print(f'{len(levels)} levels, {jobs} at once; no probes baked.', flush=True)
        return True

    output_dir.mkdir(parents=True, exist_ok=update)
    options = with_threads(options, jobs)
    baker_hash = digest.sha256(baker) if update else None
    report = {'version': 1, 'levels_dir': str(levels_dir), 'levels': [
        {'level': level.name, 'verified': False} for level in levels]}
    lock = threading.Lock()
    done = [0]

    def save_report():
        path = output_dir/'bake-all-result.json'
        part = path.with_name(path.name + '.part')
        part.write_text(json.dumps(report, indent=2, allow_nan=False)+'\n', encoding='utf-8')
        part.replace(path)

    def finished(result, what):
        with lock:
            done[0] += 1
            save_report()
            # "[n/total]" is the launcher's progress (launcher/src/lib/progress.ts).
            print(f'[{done[0]}/{len(levels)}] {result["level"]}: {what}', flush=True)

    def bake(index):
        level = levels[index]
        result = report['levels'][index]
        final = output_dir/level.name
        stamp = level_stamp(level, baker_hash, options) if update else None
        if update and up_to_date(final, stamp):
            result.update(verified=True, sparse=f'{level.name}/sparse.orpg', kept=True)
            finished(result, 'up to date')
            return
        # A cache's level is baked beside the one in place, which it replaces once verified.
        work = output_dir/f'.{level.name}.partial' if update else final
        if update and work.exists():
            shutil.rmtree(work)
        try:
            sparse = bake_level.bake_level(level, work, baker, disc=disc, options=options)
        except (OSError, ValueError, KeyError, subprocess.CalledProcessError) as error:
            result['error'] = str(error)
            print(f'{level.name}: {error}', file=sys.stderr, flush=True)
            if update and work.exists():
                failed = output_dir/f'.{level.name}.failed'
                if failed.exists():
                    shutil.rmtree(failed)
                work.rename(failed)  # its logs, for the report
            finished(result, 'failed')
            return
        if update:
            (work/STAMP).write_text(json.dumps(stamp, indent=2)+'\n', encoding='utf-8')
            if final.exists():
                shutil.rmtree(final)
            work.rename(final)
            sparse = final/sparse.name
            shutil.rmtree(output_dir/f'.{level.name}.failed', ignore_errors=True)
        result.update(verified=True, sparse=sparse.relative_to(output_dir).as_posix())
        finished(result, 'baked')

    save_report()
    order = largest_first(levels) if jobs > 1 else range(len(levels))
    if jobs == 1:
        for index in order:
            bake(index)
    else:
        with ThreadPoolExecutor(max_workers=jobs) as pool:
            list(pool.map(bake, order))

    passed = sum(result['verified'] for result in report['levels'])
    print(f'Verified {passed}/{len(levels)} levels. Summary: '
          f'{output_dir/"bake-all-result.json"}', flush=True)
    print(f'openrac-output: {output_dir}', flush=True)
    return passed == len(levels)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--levels-dir', type=Path, required=True,
                        help='one exported version containing level_XX directories')
    parser.add_argument('--output-dir', type=Path, required=True,
                        help='new output root (or, with --update, the cache); each level gets its own directory')
    parser.add_argument('--baker', type=Path, default=seeds.ROOT/'port/build/release'/
                        ('openrac-probe-bake.exe' if os.name == 'nt' else 'openrac-probe-bake'))
    parser.add_argument('--disc', type=Path, help='fallback for older exports without gameplay')
    parser.add_argument('--rays', type=int, help='quality override for every level')
    parser.add_argument('--bounces', type=int, help='bounce override for every level')
    parser.add_argument('--threads', type=int, help='CPU bake threads; 0 uses all, 1 is serial')
    parser.add_argument('--jobs', type=int, default=1,
                        help='levels baked at once; 0 is one per four threads of the bake, as memory allows (default 1)')
    parser.add_argument('--update', action='store_true',
                        help='keep up-to-date levels of an existing output root and bake the others again')
    parser.add_argument('--dry-run', action='store_true', help='list levels without baking or writing')
    args = parser.parse_args()
    # Whatever the player does comes first; the bake has the rest of the CPU.
    background.lower_priority()
    options = []
    for key in ('rays', 'bounces', 'threads'):
        value = getattr(args, key)
        if value is not None:
            options.extend(('--'+key, str(value)))
    try:
        success = bake_all(args.levels_dir, args.output_dir, args.baker,
                           disc=args.disc, options=options, dry_run=args.dry_run,
                           update=args.update, jobs=args.jobs)
    except (OSError, ValueError) as error:
        parser.exit(1, f'{error}\n')
    return 0 if success else 1


if __name__ == '__main__':
    sys.exit(main())
