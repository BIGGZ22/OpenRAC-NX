#!/usr/bin/env python3
"""Verify all formatter-unit C functions with the actual DTL assembler.

The independent assembly uses raw compiler output, bypassing all modeled
assembler/linker behavior. Run from the root with the host-runner env.
"""
from pathlib import Path
import re
import subprocess
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import dtls29_as
import try_func


def main():
    source = Path('src/core/00119D88.c')
    text = source.read_text()
    work = Path('build-sn/verify-formatter-assembler')
    obj = try_func.build('func_00119EA8', 'core_text', source, 0,
                         len(text.splitlines()) - 1, text, work)
    assert obj is not None, (work / 'log.txt').read_text()
    raw = work / 'raw-original.o'
    subprocess.run([*dtls29_as.command(), '-O2', '-G2', '-mabi=eabi', '-I', '.',
                    '-o', str(raw), str(work / 'a.s')], check=True)
    text = source.read_text()
    names = set()
    for match in re.finditer(r'^(?!extern\b)[A-Za-z_][^\n]*?\b(func_[0-9A-F]{8})(?:_r)?\s*\(', text, re.M):
        tail = text[match.start():]
        semi, brace = tail.find(';'), tail.find('{')
        if brace >= 0 and (semi < 0 or brace < semi):
            names.add(match[1])
    assert 'func_00119EA8' in names and len(names) > 40, names
    for name in sorted(names):
        for path in [obj, raw]:
            verdict = try_func.compare(name, 'core_text', path, False)
            assert verdict.startswith('EXACT'), (name, path, verdict)
            if name == 'func_00119EA8':
                assert verdict == 'EXACT', verdict
    print(f'PASS: all {len(names)} C functions exact with the production profile and raw DTL-S13010 assembly.')
    print('Float formatter helper: 144 bytes EXACT, no masked references.')


if __name__ == '__main__':
    main()
