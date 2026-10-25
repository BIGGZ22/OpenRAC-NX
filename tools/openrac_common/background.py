# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""
Long CPU work (a build, the lighting bake) that leaves the computer usable while it runs.

  from openrac_common import background
  background.lower_priority()       # this process and every program it starts, below normal
  threads = background.cpu_budget()  # the CPU threads to use: all but a few, kept for the desktop

A build of the port or a bake of every level otherwise takes every thread at normal priority, and
on a 16-core desktop that left the mouse, the music and the launcher's window stalling for minutes
(and at full power for as long). Below normal priority, anything the player does comes first and the
work still has all the CPU nobody else wants, so it is no slower on an idle machine. Windows passes a
below-normal priority class on to the programs a process starts; POSIX passes on its niceness.
"""
import os
import sys

# The threads left free by cpu_budget: an eighth of the CPU, at least two.
_RESERVE_SHARE = 8
_RESERVE_MIN = 2


def lower_priority() -> bool:
    """Puts this process (and so the programs it starts) below normal priority. False when the
    system did not allow it; the work then runs as before."""
    try:
        if sys.platform == "win32":
            import ctypes

            below_normal = 0x00004000  # BELOW_NORMAL_PRIORITY_CLASS
            kernel32 = ctypes.WinDLL("kernel32", use_last_error=True)
            # The process's pseudo-handle is -1 as a pointer: as ctypes' default 32-bit int it is
            # no handle at all, and the call fails without a word.
            kernel32.GetCurrentProcess.restype = ctypes.c_void_p
            kernel32.SetPriorityClass.argtypes = (ctypes.c_void_p, ctypes.c_uint32)
            kernel32.SetPriorityClass.restype = ctypes.c_int
            return bool(kernel32.SetPriorityClass(kernel32.GetCurrentProcess(), below_normal))
        os.nice(10)
        return True
    except (AttributeError, OSError):
        return False


def cpu_budget(cpus: int | None = None) -> int:
    """The CPU threads long work should use: all but an eighth (at least two kept free), and at
    least one. 32 threads give 28; 8 give 6; 2 give 1."""
    cpus = cpus if cpus is not None else (os.cpu_count() or 1)
    return max(1, cpus - max(_RESERVE_MIN, cpus // _RESERVE_SHARE))
