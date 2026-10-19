#!/usr/bin/env python3
"""Run the original GNU 2.10-ee assembler without rewriting its input.

EE_GAS210 may specify a command (including a host runner). On Linux, the
default is the SDK mirror's ELF assembler, with our local QEMU runner when
installed. Other hosts can use the Linux build container or EE_GAS210.
"""
import hashlib
import os
from pathlib import Path
import shlex
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
VERSION = 'GNU assembler 2.10-ee-001003-1'
SHA256 = 'b8cfdb6ecb6931642914020f92a62e653378f57230eed4895f69afa050d47b5e'


def command():
    override = os.environ.get('EE_GAS210')
    if override:
        result = shlex.split(override)
    else:
        assembler = ROOT / 'toolchain/sn-prodg-24/local/sce/ee/gcc/bin/ee-as'
        if not assembler.is_file() or hashlib.sha256(assembler.read_bytes()).hexdigest() != SHA256:
            raise RuntimeError('Missing original GNU 2.10-ee assembler in the SDK binary mirror.')
        if sys.platform != 'linux':
            raise RuntimeError('GNU 2.10-ee requires Linux/the build container, or EE_GAS210 with a compatible runner.')
        assembler.chmod(assembler.stat().st_mode | 0o100)
        qemu = ROOT / 'toolchain/host-runners/qemu/usr/bin/qemu-i386-static'
        sysroot = ROOT / 'toolchain/host-runners/linux32'
        result = ([str(qemu), '-L', str(sysroot)] if qemu.is_file() else []) + [str(assembler)]
    version = subprocess.check_output([*result, '--version'], text=True).splitlines()[0]
    if version != VERSION:
        raise RuntimeError('Expected ' + VERSION + '; got ' + version)
    return result


def main():
    if len(sys.argv) != 3:
        raise SystemExit('usage: python tools/gas210.py INPUT.s OUTPUT.o')
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
