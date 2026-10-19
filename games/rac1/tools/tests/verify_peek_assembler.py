#!/usr/bin/env python3
"""Check the whole bitstream object using the original assembler binary.

The independent assembly takes raw compiler output, bypassing all modeled
assembler/linker behavior. Requires the SDK binary mirror, PAL executable,
and host runners (source toolchain/host-runners/env.sh on headless Linux).
"""
from pathlib import Path
import subprocess
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import gas210
import try_func
from toolchain import sn


def main():
    source = Path('src/core/0012A2F0.c')
    text = source.read_text()
    work = Path('build-sn/verify-peek-assembler')
    obj = try_func.build('func_0012AAA8', 'core_text', source, 0,
                         len(text.splitlines()) - 1, text, work)
    assert obj is not None, (work / 'log.txt').read_text()
    original = work / 'raw-original.o'
    subprocess.run([*gas210.command(), '-O2', '-G2', '-mabi=eabi', '-I', '.',
                    '-o', str(original), str(work / 'a.s')], check=True)
    names = ['func_0012A2F0', 'func_0012A558', 'func_0012AA70',
             'func_0012AAA8', 'func_0012AAC8', 'func_0012AB60',
             'func_0012ABB0', 'func_0012ABF8', 'func_0012AC50']
    for name in names:
        for path in [obj, original]:
            verdict = try_func.compare(name, 'core_text', path, False)
            assert verdict == 'EXACT', (name, path, verdict)
    old = work / 'old-assembler.o'
    subprocess.run(sn(try_func.CC, *try_func.CFLAGS, '-c',
                      str(work / 'a.s'), '-o', str(old)), check=True)
    verdict = try_func.compare('func_0012AAA8', 'core_text', old, False)
    assert verdict == 'SIZE ours 32 / retail 28', verdict
    print('PASS: all 9 C functions EXACT with the production profile and raw GNU 2.10-ee assembly.')
    print('Peek: 28 bytes, no masked references; older assembler produces 32 bytes from the same input.')


if __name__ == '__main__':
    main()
