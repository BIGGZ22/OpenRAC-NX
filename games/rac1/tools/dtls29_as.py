#!/usr/bin/env python3
"""Launch the original DTL-S13010 GNU assembler with its bundled runtime.

No assembler input is rewritten. The 2.9-ee version label is shared by
other builds, so the default launcher checks the actual binary checksum.
"""
import hashlib
import os
from pathlib import Path
import shlex
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
VERSION = 'GNU assembler 2.9-ee-991111'
FILES = {
    'as.bin': 'cb909465e826106e8ad11f11bb201476a0b64a5012829cd2a440ca3eaf732a03',
    'lib/ld-2.3.6.so': 'd5a16aee4a04db7fa27a5c071a0bef62002ee679bf5ecce51e383003cf840fd8',
    'lib/libc-2.3.6.so': '7b091aee7173e3a7cbddc6630fa6b0d0106944b5618b0b27c2c058ea9c3d92eb',
}


def command():
    override = os.environ.get('EE_DTLS29_AS')
    if override:
        result = shlex.split(override)
    else:
        base = ROOT / 'toolchain/ee-as-dtls13010'
        for name, digest in FILES.items():
            path = base / name
            if not path.is_file() or hashlib.sha256(path.read_bytes()).hexdigest() != digest:
                raise RuntimeError('Missing or unexpected DTL-S13010 assembler/runtime; see docs/TOOLCHAIN.md.')
            path.chmod(path.stat().st_mode | 0o100)
        if sys.platform != 'linux':
            raise RuntimeError('Use a Linux host or the build container, or EE_DTLS29_AS with a compatible runner for the same binary.')
        # A 32-bit Linux binary, run with its own bundled loader and libc:
        # directly on an x86 host, through a local QEMU user-mode runner
        # if one is installed.
        result = [str(base / 'lib/ld-2.3.6.so'), '--library-path',
                  str(base / 'lib'), str(base / 'as.bin')]
        qemu = ROOT / 'toolchain/host-runners/qemu/usr/bin/qemu-i386-static'
        if qemu.is_file():
            result = [str(qemu)] + result
    version = subprocess.check_output([*result, '--version'], text=True).splitlines()[0]
    if version != VERSION:
        raise RuntimeError('Expected ' + VERSION + '; got ' + version)
    return result


def main():
    if len(sys.argv) != 3:
        raise SystemExit('usage: python tools/dtls29_as.py INPUT.s OUTPUT.o')
    output = Path(sys.argv[2])
    output.unlink(missing_ok=True)
    try:
        subprocess.run([*command(), '-O2', '-G2', '-mabi=eabi', '-I', '.',
                        '-o', str(output), sys.argv[1]], check=True)
    except (RuntimeError, OSError, subprocess.SubprocessError) as error:
        output.unlink(missing_ok=True)
        raise SystemExit(str(error))


if __name__ == '__main__':
    main()
