#!/usr/bin/env python3
"""
Builds the native port (port/): what the launcher's "Build the game" runs, so
that nobody has to put a compiler's environment together by hand.

  python3 tools/build_port.py [GAME ...] [--source DIR] [--preset NAME] [--target NAME ...]

GAME is one of the port's games (port/game/<id>/hostgen.json: rac1-pal,
rac1-ntsc, ...), rac1-pal when none is named. The build makes openrac-GAME
for each, and the C++ extractor (openrac-extractor), in port/build/PRESET
(a CMake preset, "release" by default). The first run configures that
folder; later runs rebuild only what changed, hostgen included when the
decompilation or hostgen changed (port/cmake/Games.cmake).

--target NAME builds another of the port's targets as well (a test program
such as runtime_test), in the same environment.

--source DIR builds GAME (one game) from another copy of its decompilation:
your own checkout, further along than OpenRAC's (OPENRAC_<ID>_SOURCE). The
build folder keeps the choice, so later runs build from it again until
another --source.

On Windows the build runs in Visual Studio's x64 environment (the MSVC
libraries and the Windows SDK, from any edition with the C++ tools, found
with vswhere), with Clang (OPENRAC_CLANG, else LLVM's installer, Scoop or
Visual Studio's own, else clang on PATH) and Ninja (PATH, or the one Visual
Studio ships). On Linux and macOS, CMake, Ninja and Clang come from PATH, as
the preset says. Standard library only.
"""

import argparse
import json
import os
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
PORT = ROOT / "port"
WINDOWS = os.name == "nt"
DEFAULT_GAME = "rac1-pal"

NO_VISUAL_STUDIO = (
    "Visual Studio's C++ tools were not found. Install Visual Studio (Community is free) or its "
    'Build Tools with the "Desktop development with C++" workload, then build again.'
)
NO_CLANG = (
    "Clang was not found. Install LLVM (winget install LLVM.LLVM) or Visual Studio's "
    '"C++ Clang tools for Windows", then build again.'
    if WINDOWS
    else "Clang was not found. Install it (apt install clang, or Xcode's command line tools), then build again."
)
NO_NINJA = (
    'Ninja was not found. Install it (winget install Ninja-build.Ninja) or Visual Studio\'s "C++ CMake '
    'tools for Windows", then build again.'
    if WINDOWS
    else "Ninja was not found. Install it (apt install ninja-build, or brew install ninja), then build again."
)
NO_CMAKE = "CMake 3.25 or newer was not found. Install it (https://cmake.org/download/), then build again."


class BuildError(Exception):
    """Why the build cannot start, worded for whoever reads the launcher's Tasks."""


def say(text: str) -> None:
    print(text, flush=True)


def games() -> list[str]:
    """The port's games: the folders of port/game that hold a hostgen.json."""
    return sorted(p.parent.name for p in (PORT / "game").glob("*/hostgen.json"))


def source_variable(game: str) -> str:
    """The cache variable naming GAME's decompilation (Games.cmake): rac1-pal is OPENRAC_RAC1_PAL_SOURCE."""
    return f"OPENRAC_{game.upper().replace('-', '_')}_SOURCE"


def default_source(game: str) -> Path:
    """The decompilation in this checkout that GAME is built from unless told otherwise (hostgen.json)."""
    config = json.loads((PORT / "game" / game / "hostgen.json").read_text(encoding="utf-8"))
    return ROOT / config["source"]


def targets(names: list[str], more: list[str] | None = None) -> list[str]:
    """What is built: each game's program, the extractor, and any target asked for besides."""
    return [f"openrac-{name}" for name in names] + ["openrac-extractor"] + list(more or [])


def parse_cache(text: str) -> dict[str, str]:
    """CMakeCache.txt's entries (NAME:TYPE=VALUE) as NAME -> VALUE."""
    cache = {}
    for line in text.splitlines():
        if not line or line.startswith(("#", "//")):
            continue
        key, sep, value = line.partition("=")
        if sep:
            cache[key.split(":", 1)[0]] = value
    return cache


def read_cache(build: Path) -> dict[str, str]:
    """A build folder's cache, or {} when the folder was never configured."""
    try:
        return parse_cache((build / "CMakeCache.txt").read_text(encoding="utf-8", errors="replace"))
    except OSError:
        return {}


def parse_environment(text: str) -> dict[str, str]:
    """What cmd.exe's `set` prints, as an environment. Names are upper-cased, as Windows ignores
    their case; the drive entries (=C:=...) are left out."""
    env = {}
    for line in text.splitlines():
        key, sep, value = line.partition("=")
        if sep and key:
            env[key.upper()] = value
    return env


def same_path(a: str | Path, b: str | Path) -> bool:
    def norm(p: str | Path) -> str:
        return os.path.normcase(os.path.normpath(str(p)))

    return norm(a) == norm(b)


def commands(
    cmake: str,
    preset: str,
    build: Path,
    names: list[str],
    source: Path | None,
    cache: dict[str, str],
    compilers: tuple[str, str, str] | None = None,
    python: str | None = None,
    more: list[str] | None = None,
) -> list[list[str]]:
    """The command lines to run, in order. A folder not configured yet is configured from the
    preset; a configured one is only built, after a configure without the preset when `source`
    names another decompilation than it builds from. The preset names its compilers (clang,
    clang++), and a name that finds another Clang than the cached one makes CMake drop the whole
    cache, so a configure from the preset passes the folder's own compilers, else `compilers`
    (C, C++, Ninja) when given. `python` is the interpreter hostgen runs under in a new folder
    (this one: CMake's own choice may be an older Python than hostgen reads)."""
    variable = source_variable(names[0]) if source is not None else None
    if not cache or not (build / "build.ninja").is_file():
        configure = [cmake, "--preset", preset]
        found = dict(zip(("CMAKE_C_COMPILER", "CMAKE_CXX_COMPILER", "CMAKE_MAKE_PROGRAM"), compilers or ()))
        if python:
            found["Python3_EXECUTABLE"] = python
        for key in ("CMAKE_C_COMPILER", "CMAKE_CXX_COMPILER", "CMAKE_MAKE_PROGRAM", "Python3_EXECUTABLE"):
            value = cache.get(key) or found.get(key)
            if value:
                configure.append(f"-D{key}={value}")
        if variable:
            configure.append(f"-D{variable}={Path(source).as_posix()}")
        out = [configure]
    elif variable and not same_path(cache.get(variable, ""), source):
        out = [[cmake, "-S", str(PORT), "-B", str(build), f"-D{variable}={Path(source).as_posix()}"]]
    else:
        out = []
    out.append([cmake, "--build", "--preset", preset, "--target", *targets(names, more)])
    return out


# ---- the tools, found ----------------------------------------------------------------------


def find_visual_studio() -> Path:
    """The newest Visual Studio (any edition, Build Tools too) with the C++ x64 tools."""
    base = os.environ.get("PROGRAMFILES(X86)") or r"C:\Program Files (x86)"
    vswhere = Path(base) / "Microsoft Visual Studio" / "Installer" / "vswhere.exe"
    if not vswhere.is_file():
        raise BuildError(NO_VISUAL_STUDIO)
    found = subprocess.run(
        [str(vswhere), "-latest", "-products", "*", "-requires",
         "Microsoft.VisualStudio.Component.VC.Tools.x86.x64", "-property", "installationPath"],
        capture_output=True, text=True,
    )
    lines = found.stdout.strip().splitlines()
    if found.returncode != 0 or not lines:
        raise BuildError(NO_VISUAL_STUDIO)
    return Path(lines[0])


def msvc_environment(visual_studio: Path) -> dict[str, str]:
    """The environment vcvars64.bat sets up, read back with `set` (UTF-16, cmd's /u, so any
    character in a path comes through)."""
    bat = visual_studio / "VC" / "Auxiliary" / "Build" / "vcvars64.bat"
    if not bat.is_file():
        raise BuildError(f"{bat} is missing: repair Visual Studio's C++ tools, then build again.")
    done = subprocess.run(f'cmd /u /s /c ""{bat}" >nul 2>&1 && set"', capture_output=True)
    env = parse_environment(done.stdout.decode("utf-16-le", errors="replace"))
    if done.returncode != 0 or "INCLUDE" not in env:
        raise BuildError(f"{bat} did not set up Visual Studio's environment.")
    return env


def clang_candidates(env: dict[str, str], visual_studio: Path | None) -> list[Path]:
    """Where Clang is looked for on Windows, in order."""
    out = []
    if env.get("OPENRAC_CLANG"):
        out.append(Path(env["OPENRAC_CLANG"]))
    for base in (env.get("PROGRAMFILES"), env.get("PROGRAMW6432")):
        if base:
            out.append(Path(base) / "LLVM" / "bin" / "clang.exe")
    if env.get("USERPROFILE"):
        out.append(Path(env["USERPROFILE"]) / "scoop" / "apps" / "llvm" / "current" / "bin" / "clang.exe")
    if visual_studio is not None:
        out.append(visual_studio / "VC" / "Tools" / "Llvm" / "x64" / "bin" / "clang.exe")
    on_path = shutil.which("clang", path=env.get("PATH"))
    if on_path:
        out.append(Path(on_path))
    return out


def find_clang(env: dict[str, str], visual_studio: Path | None) -> tuple[Path, Path]:
    """Clang and Clang++ (beside it), for a new build folder on Windows."""
    for clang in clang_candidates(env, visual_studio):
        if clang.is_file():
            cxx = clang.with_name("clang++.exe")
            return clang, cxx if cxx.is_file() else clang
    raise BuildError(NO_CLANG)


def find_program(name: str, *paths: str | None) -> Path | None:
    for path in paths:
        if path:
            found = shutil.which(name, path=path)
            if found:
                return Path(found)
    return None


def find_cmake(cache: dict[str, str], *paths: str | None) -> Path:
    """The CMake that configured the build folder, else the first on these PATHs."""
    cached = cache.get("CMAKE_COMMAND")
    if cached and Path(cached).is_file():
        return Path(cached)
    found = find_program("cmake", *paths)
    if found is None:
        raise BuildError(NO_CMAKE)
    return found


# ---- the build ---------------------------------------------------------------------------


def prepare(cache: dict[str, str]) -> tuple[dict[str, str], Path, tuple[str, str, str] | None]:
    """The environment to build in, CMake, and the compilers to configure a new folder with."""
    user_path = os.environ.get("PATH")
    if not WINDOWS:
        if not cache:
            if find_program("ninja", user_path) is None:
                raise BuildError(NO_NINJA)
            if find_program("clang", user_path) is None:
                raise BuildError(NO_CLANG)
        return dict(os.environ), find_cmake(cache, user_path), None

    visual_studio = find_visual_studio()
    say(f"Visual Studio: {visual_studio}")
    env = msvc_environment(visual_studio)
    compilers = None
    if "CMAKE_C_COMPILER" in cache:
        clang = Path(cache["CMAKE_C_COMPILER"])
    else:
        clang, cxx = find_clang(env, visual_studio)
        ninja = find_program("ninja", env.get("PATH"))
        if ninja is None:
            raise BuildError(NO_NINJA)
        compilers = (clang.as_posix(), cxx.as_posix(), ninja.as_posix())
    say(f"Clang: {clang}")
    # hostgen reads the decompilation with the same Clang (port/tools/hostgen/clangast.py).
    env["OPENRAC_CLANG"] = str(clang)
    # The user's own CMake before the one Visual Studio puts first on its PATH.
    return env, find_cmake(cache, user_path, env.get("PATH")), compilers


def main(argv: list[str] | None = None) -> int:
    known = games()
    parser = argparse.ArgumentParser(description="Builds the native port (port/) for this computer.")
    parser.add_argument("games", nargs="*", metavar="GAME", help=f"a port game ({', '.join(known)}); default {DEFAULT_GAME}")
    parser.add_argument("--source", type=Path, help="build GAME from this copy of its decompilation (kept for later builds)")
    parser.add_argument("--preset", default="release", help="the CMake preset (port/CMakePresets.json); default release")
    parser.add_argument("--target", action="append", default=[], metavar="NAME",
                        help="also build this target of the port (a test program such as runtime_test)")
    args = parser.parse_args(argv)

    names = args.games or [DEFAULT_GAME]
    unknown = [name for name in names if name not in known]
    if unknown:
        parser.error(f"no such game in port/game: {', '.join(unknown)} (the games: {', '.join(known)})")
    source = None
    if args.source is not None:
        if len(names) != 1:
            parser.error("--source is one game's decompilation: name that one GAME")
        source = args.source.resolve()
        if not source.is_dir():
            parser.error(f"--source {args.source}: no such folder")

    build = PORT / "build" / args.preset
    cache = read_cache(build)
    try:
        env, cmake, compilers = prepare(cache)
    except BuildError as e:
        print(f"error: {e}", file=sys.stderr, flush=True)
        return 1

    for name in names:
        decompilation = source if name == names[0] and source else cache.get(source_variable(name)) or default_source(name)
        say(f"{name}: from {decompilation}")
    python = Path(sys.executable).as_posix()
    for command in commands(str(cmake), args.preset, build, names, source, cache, compilers, python, args.target):
        say("> " + " ".join(f'"{part}"' if " " in part else part for part in command))
        status = subprocess.run(command, cwd=PORT, env=env).returncode
        if status != 0:
            print(f"error: the build stopped ({Path(command[0]).name} {command[1]} exited with {status})",
                  file=sys.stderr, flush=True)
            return 1
    suffix = ".exe" if WINDOWS else ""
    for name in names:
        # On macOS a windowed game is an application bundle (port/cmake/MacApp.cmake).
        program = next((build / "apps" / f"openrac-{name}").glob("*.app"), build / f"openrac-{name}{suffix}")
        say(f"built {program.relative_to(ROOT).as_posix()}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
