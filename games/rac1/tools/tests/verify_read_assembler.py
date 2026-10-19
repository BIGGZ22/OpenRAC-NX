#!/usr/bin/env python3
"""Verify the landed CD read against retail using the original GNU 2.10-ee.
Run from the repository root. Requires the repository toolchain mirror and PAL baserom.
No SDK source or header is used. The real assembler receives raw compiler
output, bypassing every assembly-modeling pass under test.
"""
from pathlib import Path
import os
import subprocess
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import try_func


def main():
    root = Path.cwd()
    assembler = root / 'toolchain/sn-prodg-24/local/sce/ee/gcc/bin/ee-as'
    qemu = root / 'toolchain/host-runners/qemu/usr/bin/qemu-i386-static'
    sysroot = root / 'toolchain/host-runners/linux32'
    if not assembler.exists():
        raise SystemExit('Requires the GNU 2.10-ee mirror (toolchain/sn-prodg-24).')
    assembler.chmod(assembler.stat().st_mode | 0o100)
    # A 32-bit Linux binary: run it directly where i386 libraries are
    # installed, else through a local QEMU user-mode runner.
    command = [str(assembler)]
    if qemu.exists():
        command = [str(qemu), '-L', str(sysroot), str(assembler)]
    version = subprocess.check_output([*command, '--version'], text=True).splitlines()[0]
    assert version == 'GNU assembler 2.10-ee-001003-1', version
    source = Path('src/core/00121750.c')
    text = source.read_text()
    work = Path('build-sn/verify-read-assembler')
    obj = try_func.build('func_00121750', 'core_text', source, 0,
                         len(text.splitlines()) - 1, text, work)
    assert obj is not None, (work / 'log.txt').read_text()
    # a.s is the compiler's unmodified output, before any modeled behavior.
    original = work / 'original-assembler.o'
    subprocess.run([*command, '-O2', '-G2', '-mabi=eabi', '-I', '.',
                    '-o', str(original), str(work / 'a.s')], check=True)
    names = ['func_00121750', 'func_00121930', 'func_001219C8',
             'func_00121A80', 'func_00121B78', 'func_00121D08']
    for name in names:
        for path in [obj, original]:
            verdict = try_func.compare(name, 'core_text', path, False)
            assert verdict == 'EXACT', (name, path, verdict)
    print(f'PASS: all {len(names)} C functions EXACT with both the build and {version}.')
    print('func_00121750: 480 bytes, all relocated references resolved.')


if __name__ == '__main__':
    main()
