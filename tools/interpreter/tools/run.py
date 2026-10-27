#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""Builds the runtime and starts a game in it. One command each, for the launcher and for people.

    run.py build                        configure if needed, then build everything
    run.py test                         build, then run the runtime's tests
    run.py play SERIAL DISC [OPTIONS]   build, then run a disc image in a window
    run.py port GAME/VERSION            build the game's decompiled C as a host library
    run.py play-port GAME/VERSION SERIAL DISC [OPTIONS]
                                        both builds, then play with the decompiled
                                        functions running as host code

SERIAL is the disc's serial as the game folders write it (SCES_509.16). The
runtime needs a table for the game, runtime/games/SERIAL.hooks; without one
this says so and stops. OPTIONS go to openrac-boot unchanged (--card DIR,
--no-card, --frames N and so on; see runtime/README.md).

The build goes to build/runtime in the checkout. It needs CMake, a C++20
compiler and SDL3; Ninja is used when it is installed. The host library goes
to build/port/GAME-VERSION and needs LLVM with the wasm32 target and WABT as
well (runtime/port/README.md).
"""
import os
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
BUILD = ROOT / "build" / "runtime"

# Where package managers put programs that a desktop app's PATH often lacks.
EXTRA_PATH = ["/opt/homebrew/bin", "/usr/local/bin"]


def find(program):
    """Returns the path of a program, looking beyond PATH in the usual places, or None."""
    path = os.pathsep.join([os.environ.get("PATH", "")] + EXTRA_PATH)
    return shutil.which(program, path=path)


def run(command):
    """Runs a command with its output passed through; stops this script if it fails."""
    print("+ " + " ".join(str(part) for part in command), flush=True)
    result = subprocess.run(command)
    if result.returncode != 0:
        raise SystemExit(result.returncode)


def build(targets=()):
    """Configures the build directory once, then builds the targets (all of them when none)."""
    cmake = find("cmake")
    if not cmake:
        raise SystemExit("run.py: CMake is not installed (https://cmake.org); the runtime needs it")
    generator_file = BUILD / ("build.ninja" if find("ninja") else "Makefile")
    if not generator_file.exists():
        if BUILD.exists():
            shutil.rmtree(BUILD)
        configure = [cmake, "-S", str(ROOT / "runtime"), "-B", str(BUILD)]
        ninja = find("ninja")
        if ninja:
            configure += ["-G", "Ninja", f"-DCMAKE_MAKE_PROGRAM={ninja}"]
        run(configure)
    command = [cmake, "--build", str(BUILD)]
    for target in targets:
        command += ["--target", target]
    run(command)


def test():
    """Builds everything and runs the tests."""
    build()
    ctest = find("ctest")
    if not ctest:
        raise SystemExit("run.py: ctest was not found beside CMake")
    run([ctest, "--test-dir", str(BUILD), "--output-on-failure"])


def play(serial, disc, options):
    """Builds openrac-boot and runs a disc image in a window."""
    hooks = ROOT / "runtime" / "games" / f"{serial}.hooks"
    if not hooks.exists():
        known = ", ".join(sorted(path.stem for path in hooks.parent.glob("*.hooks")))
        raise SystemExit(f"run.py: the runtime has no table for {serial} yet (it has: {known})")
    if not Path(disc).exists():
        raise SystemExit(f"run.py: no disc image at {disc}")
    build(["openrac-boot"])
    command = [str(BUILD / "openrac-boot"), str(disc), "--hooks", str(hooks), "--window"]
    # European serials are 50 fields a second, the others 60.
    if not serial.startswith(("SCES", "SLES")):
        command.append("--ntsc")
    run(command + list(options))


def port(key):
    """Builds a game version's decompiled C as a host library and returns the library's path."""
    run([sys.executable, str(ROOT / "runtime" / "port" / "port.py"), "build", key])
    folder = ROOT / "build" / "port" / key.replace("/", "-")
    return folder / ("libopenrac-native.dylib" if sys.platform == "darwin" else "libopenrac-native.so")


def main():
    arguments = sys.argv[1:]
    if arguments == ["build"]:
        build()
    elif arguments == ["test"]:
        test()
    elif len(arguments) >= 3 and arguments[0] == "play":
        play(arguments[1], arguments[2], arguments[3:])
    elif len(arguments) == 2 and arguments[0] == "port":
        print(port(arguments[1]))
    elif len(arguments) >= 4 and arguments[0] == "play-port":
        play(arguments[2], arguments[3], ["--native", str(port(arguments[1]))] + arguments[4:])
    else:
        raise SystemExit(__doc__)


if __name__ == "__main__":
    main()
