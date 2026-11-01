#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""Rebuild a dkgl pack from its embedded SPIR-V with recorded tool provenance.

Writes to a separate directory. Install only after all stages compile. The .prog
reflection bytes and program names remain unchanged, matching runtime source keys.
"""
import argparse
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
from dk_check import decode


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('pack', type=Path)
    parser.add_argument('--out', type=Path, required=True)
    parser.add_argument('--spirv-cross', type=Path, required=True)
    parser.add_argument('--uam', type=Path, required=True)
    args = parser.parse_args()
    if args.pack.resolve() == args.out.resolve():
        parser.error('output must be separate from the input pack')
    args.out.mkdir(parents=True, exist_ok=True)
    work = args.out / 'sources'
    work.mkdir(exist_ok=True)
    report = {'tools': {str(p.resolve()): sha(p) for p in (args.spirv_cross, args.uam)},
              'vertex_options': ['--version', '460', '--no-es', '--flip-vert-y'], 'stages': []}
    programs = sorted(args.pack.glob('*.prog'))
    if not programs:
        parser.error('no programs found')
    for prog in programs:
        program = decode(prog.read_bytes())
        for stage, field in (('vert', 'vertex'), ('frag', 'fragment')):
            stem = prog.stem + '.' + stage
            spv, glsl = work / (stem + '.spv'), work / (stem + '.glsl')
            output = args.out / (stem + '.dksh')
            spv.write_bytes(program[field])
            commands = [
                [str(args.spirv_cross), str(spv), '--version', '460', '--no-es',
                 *(['--flip-vert-y'] if stage == 'vert' else []), '--output', str(glsl)],
                [str(args.uam), '-s', stage, '-o', str(output), str(glsl)],
            ]
            for command in commands:
                result = subprocess.run(command, capture_output=True, text=True)
                (work / (stem + ('.cross.log' if command == commands[0] else '.uam.log'))).write_text(
                    result.stdout + result.stderr, encoding='utf-8')
                if result.returncode:
                    raise RuntimeError(f'{stem}: failed with {result.returncode}: {result.stderr}')
            if output.read_bytes()[:4] != b'DKSH':
                raise RuntimeError(f'{stem}: invalid DKSH magic')
            previous = args.pack / output.name
            report['stages'].append({'name': output.name, 'spirv_sha256': sha(spv),
                'glsl_sha256': sha(glsl), 'dksh_sha256': sha(output),
                'previous_sha256': sha(previous), 'changed': sha(previous) != sha(output)})
        shutil.copyfile(prog, args.out / prog.name)
    (args.out / 'provenance.json').write_text(json.dumps(report, indent=2), encoding='utf-8')
    print(f"Compiled {len(report['stages'])} stages; "
          f"{sum(s['changed'] for s in report['stages'])} differ from input")


if __name__ == '__main__':
    main()
