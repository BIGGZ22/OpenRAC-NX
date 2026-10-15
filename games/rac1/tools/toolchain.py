"""
Where the SN toolchain lives and how to run it -- the Python side of
tools/toolchain.sh, for the tools that start toolchain programs themselves.

The toolchain is 32-bit Windows programs. On Windows they run directly and
the build uses SN's own make.exe. Anywhere else they run through Wine (see
tools/docker/) and the build uses the host's GNU make. WINE=... and
MAKE_SN=... in the environment override either choice.
"""
import os
import shlex
import shutil
import subprocess
import sys
from pathlib import Path

# Absolute: Windows' CreateProcess won't resolve a relative forward-slash
# path to an .exe.
TC = str(Path("toolchain/sn-prodg-3.01/usr/local/sce/ee/gcc/bin").resolve())
ON_WINDOWS = os.name == "nt" or sys.platform in ("cygwin", "msys")
WINE = shlex.split(os.environ.get("WINE", "" if ON_WINDOWS else "wine"))


def sn(program: str, *args: str) -> list[str]:
    """Command line that runs one toolchain program."""
    return [*WINE, str(program), *args]


def start_wineserver() -> None:
    """Start a wineserver before a parallel make: many Wine processes starting
    at once otherwise race to start one, and some fail. See toolchain.sh."""
    if WINE and shutil.which("wineserver"):
        subprocess.run(["wineserver", "-p60"], stderr=subprocess.DEVNULL)


def make_sn(*args: str) -> list[str]:
    """Command line that runs Makefile.sn with the right make and Wine."""
    default = f"{TC}/make.exe" if ON_WINDOWS else f"make -j{os.cpu_count() or 4}"
    make = shlex.split(os.environ.get("MAKE_SN", default))
    return [*make, "-f", "Makefile.sn", "WINE=" + " ".join(WINE), *args]
