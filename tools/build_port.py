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

--disc IMAGE names the image of GAME's disc (one game; the launcher passes the
one set up). What the decompilation still has only as assembly is then
translated from that disc's own programs (OPENRAC_<ID>_RETAIL_IMAGE,
port/tools/hostgen/retail.py) instead of staying a stub that does nothing; the
build folder keeps it too.

--clean deletes the build folder first and builds everything again (the
launcher's "Rebuild the game", and what it runs after an update); the
--source and --disc choices the folder kept are kept.

--probe-baker also builds openrac-probe-bake, the offline baker of the
enhanced rendering's probe lighting (port/enhanced/tools/probes), which the
launcher runs on the player's own extracted levels. It needs Embree 4
(Apache-2.0): a pinned release's source is downloaded once, checked against
its SHA-256 and built as a static library in port/build/embree/, so the
baker is one program with nothing beside it. A baker that cannot be built
is said and left out; the game is built all the same. The build folder
keeps the choice.

Each build that succeeds leaves openrac-build.json in the folder: the OpenRAC
version and commit it built, and the games. The launcher compares them with
the checkout to know a game is outdated after an update
(launcher/core/src/port_build.rs).

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
import platform
import shutil
import stat
import subprocess
import sys
import tarfile
import time
import urllib.request
from pathlib import Path

from openrac_common import digest, paths

ROOT = paths.repo_root()
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


# Embree, for the probe baker only (never linked into a game): this release's source, checked
# against its SHA-256, built static with Embree's own task scheduler (no TBB) and only the
# triangle geometry the baker traces (port/enhanced/lighting/bake.cpp).
EMBREE_VERSION = "4.4.1"
EMBREE_URL = f"https://github.com/RenderKit/embree/archive/refs/tags/v{EMBREE_VERSION}.tar.gz"
EMBREE_SHA256 = "dcf338cc61b636c871ccf370e673bfd380c5ecb71ce49ad50f28e1d4ec9995dc"
EMBREE_OPTIONS = {
    "CMAKE_BUILD_TYPE": "Release",
    "EMBREE_STATIC_LIB": "ON",
    "EMBREE_TASKING_SYSTEM": "INTERNAL",
    "EMBREE_TUTORIALS": "OFF",
    "EMBREE_TESTING_INSTALL_TESTS": "OFF",
    "EMBREE_ISPC_SUPPORT": "OFF",
    "EMBREE_ZIP_MODE": "OFF",
    "EMBREE_INSTALL_DEPENDENCIES": "OFF",
    "EMBREE_GEOMETRY_QUAD": "OFF",
    "EMBREE_GEOMETRY_CURVE": "OFF",
    "EMBREE_GEOMETRY_SUBDIVISION": "OFF",
    "EMBREE_GEOMETRY_USER": "OFF",
    "EMBREE_GEOMETRY_INSTANCE": "OFF",
    "EMBREE_GEOMETRY_INSTANCE_ARRAY": "OFF",
    "EMBREE_GEOMETRY_GRID": "OFF",
    "EMBREE_GEOMETRY_POINT": "OFF",
    # Embree clears CMAKE_CXX_FLAGS with Clang and GCC unless told not to, which drops the build's
    # CXXFLAGS: the launcher's Linux builds set -stdlib=libc++ there, and an Embree built against
    # libstdc++ does not link into their baker. Embree's MSVC settings do not read this option.
    "EMBREE_IGNORE_CMAKE_CXX_FLAGS": "OFF",
}
# x86-64: SSE2 for any machine, AVX and AVX2 for most; Embree picks one at run time. AVX stays on
# beside AVX2: Embree compiles the 8-wide BVH and its primitives (bvh/bvh.cpp, BVHN<8>, CurveNi<8>)
# only into its AVX library, and the AVX2 code refers to them, so without it the baker does not link
# on Windows or Linux (kernels/CMakeLists.txt, embree_files). ARM is given none of these and keeps
# Embree's own single choice (NEON, or NEON2X on Apple silicon): Embree counts a value given for an
# x86-64 ISA there too, and refuses a static library of several ISAs with AppleClang.
EMBREE_X86_ISAS = {
    "EMBREE_ISA_SSE42": "OFF",
    "EMBREE_ISA_AVX": "ON",
    "EMBREE_ISA_AVX512": "OFF",
}
EMBREE_DIR = PORT / "build" / "embree"
BAKER = "openrac-probe-bake"


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


def retail_variable(game: str) -> str:
    """The cache variable naming the disc image GAME's functions with no C are translated from."""
    return f"OPENRAC_{game.upper().replace('-', '_')}_RETAIL_IMAGE"


def default_source(game: str) -> Path:
    """The decompilation in this checkout that GAME is built from unless told otherwise (hostgen.json)."""
    config = json.loads((PORT / "game" / game / "hostgen.json").read_text(encoding="utf-8"))
    return ROOT / config["source"]


def targets(names: list[str], more: list[str] | None = None) -> list[str]:
    """What is built: each game's program, the extractor, and any target asked for besides."""
    return [f"openrac-{name}" for name in names] + ["openrac-extractor"] + list(more or [])


def embree_options(machine: str | None = None) -> dict[str, str]:
    """The options Embree is configured with on MACHINE (platform.machine(), this one's unless
    given): EMBREE_OPTIONS, and EMBREE_X86_ISAS on x86-64 only."""
    machine = (machine or platform.machine()).lower()
    x86 = machine in ("x86_64", "amd64", "x64", "i386", "i686")
    return {**EMBREE_OPTIONS, **(EMBREE_X86_ISAS if x86 else {})}


def embree_stamp(env: dict[str, str]) -> dict:
    """What an Embree build in EMBREE_DIR is of, built with ENV: a different one is built again.
    ENV's CXXFLAGS count, since Embree is compiled with them (EMBREE_OPTIONS)."""
    return {"version": EMBREE_VERSION, "sha256": EMBREE_SHA256, "options": embree_options(),
            "cxxflags": env.get("CXXFLAGS", "")}


def embree_config(install: Path) -> Path:
    """The folder of Embree's CMake package in an install (embree_DIR)."""
    for lib in ("lib", "lib64"):
        folder = install / lib / "cmake" / f"embree-{EMBREE_VERSION}"
        if (folder / "embree-config.cmake").is_file():
            return folder
    raise BuildError(f"Embree's install in {install} has no embree-config.cmake")


def fetch_embree(dest: Path) -> Path:
    """Embree's source, downloaded once into DEST and checked; the unpacked folder."""
    archive = dest / f"embree-{EMBREE_VERSION}.tar.gz"
    if not archive.is_file() or digest.sha256(archive) != EMBREE_SHA256:
        say(f"downloading Embree {EMBREE_VERSION} ({EMBREE_URL})")
        dest.mkdir(parents=True, exist_ok=True)
        part = archive.with_suffix(".part")
        with urllib.request.urlopen(EMBREE_URL, timeout=120) as response, part.open("wb") as out:
            shutil.copyfileobj(response, out)
        got = digest.sha256(part)
        if got != EMBREE_SHA256:
            part.unlink()
            raise BuildError(f"Embree's source archive has SHA-256 {got}, not {EMBREE_SHA256}")
        part.replace(archive)
    source = dest / f"embree-{EMBREE_VERSION}"
    if not (source / "CMakeLists.txt").is_file():
        if source.exists():
            remove_tree(source)
        with tarfile.open(archive) as tar:
            if hasattr(tarfile, "data_filter"):
                tar.extractall(dest, filter="data")
            else:
                tar.extractall(dest)
    return source


def build_embree(cmake: str, env: dict[str, str], compilers: tuple[str, str, str] | None,
                 cache: dict[str, str]) -> Path:
    """Embree built and installed in EMBREE_DIR (once for a version and its options); its
    embree_DIR. With the port's own compilers and Ninja, so it links with the baker."""
    install = EMBREE_DIR / "install"
    stamp = EMBREE_DIR / "openrac-embree.json"
    try:
        if json.loads(stamp.read_text(encoding="utf-8")) == embree_stamp(env):
            return embree_config(install)
    except (OSError, ValueError, BuildError):
        pass
    source = fetch_embree(EMBREE_DIR)
    work = EMBREE_DIR / "build"
    for folder in (work, install):
        if folder.exists():
            remove_tree(folder)
    tools = dict(zip(("CMAKE_C_COMPILER", "CMAKE_CXX_COMPILER", "CMAKE_MAKE_PROGRAM"), compilers or ()))
    for key in ("CMAKE_C_COMPILER", "CMAKE_CXX_COMPILER", "CMAKE_MAKE_PROGRAM"):
        tools[key] = cache.get(key) or tools.get(key) or {"CMAKE_C_COMPILER": "clang",
                                                         "CMAKE_CXX_COMPILER": "clang++"}.get(key, "")
    if WINDOWS and tools.get("CMAKE_C_COMPILER"):
        # As for the port (commands): CMake finds no resource compiler beside Clang by itself.
        rc = Path(tools["CMAKE_C_COMPILER"]).with_name("llvm-rc.exe")
        if rc.is_file():
            tools["CMAKE_RC_COMPILER"] = rc.as_posix()
    configure = [cmake, "-S", str(source), "-B", str(work), "-G", "Ninja",
                 f"-DCMAKE_INSTALL_PREFIX={install.as_posix()}",
                 *(f"-D{key}={value}" for key, value in tools.items() if value),
                 *(f"-D{key}={value}" for key, value in embree_options().items())]
    for command in (configure, [cmake, "--build", str(work)], [cmake, "--install", str(work)]):
        say("> " + " ".join(f'"{part}"' if " " in part else part for part in command))
        if subprocess.run(command, cwd=EMBREE_DIR, env=env).returncode != 0:
            raise BuildError(f"Embree did not build ({Path(command[0]).name} {command[1]})")
    # Its license and the notices of what it carries (SSE2NEON on ARM), for whoever passes the
    # baker on (licenses(), tools/bundle_standalone.py).
    notices = install / "licenses" / "embree"
    notices.mkdir(parents=True, exist_ok=True)
    for name in ("LICENSE.txt", "third-party-programs.txt"):
        shutil.copy2(source / name, notices / name)
    stamp.write_text(json.dumps(embree_stamp(env), indent=2) + "\n", encoding="utf-8")
    return embree_config(install)


def copy_licenses(build: Path) -> None:
    """Embree's license and notices beside the baker built with it: <build>/licenses/embree."""
    dest = build / "licenses" / "embree"
    for folder in (EMBREE_DIR / "install" / "licenses" / "embree", EMBREE_DIR / f"embree-{EMBREE_VERSION}"):
        if all((folder / name).is_file() for name in ("LICENSE.txt", "third-party-programs.txt")):
            dest.mkdir(parents=True, exist_ok=True)
            for name in ("LICENSE.txt", "third-party-programs.txt"):
                shutil.copy2(folder / name, dest / name)
            return
    raise BuildError("Embree's LICENSE.txt is missing: build it again (delete port/build/embree)")


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


def remove_tree(path: Path) -> None:
    """Deletes the folder PATH and everything in it. Windows refuses to delete a read-only file,
    and git makes its objects read-only: a dependency CMake cloned into the build folder
    (_deps/*-src/.git) stopped --clean with "Access is denied". Such a file is made writable and
    deleted again; one held for a moment (a virus scanner reading an object file just written)
    is tried again a few times."""
    def writable_then_again(function, name, _error):
        for attempt in range(5):
            try:
                os.chmod(name, stat.S_IWRITE)
                function(name)
                return
            except PermissionError:
                if attempt == 4:
                    raise
                time.sleep(0.5)

    if sys.version_info >= (3, 12):
        shutil.rmtree(path, onexc=writable_then_again)
    else:
        shutil.rmtree(path, onerror=writable_then_again)


def parse_environment(text: str) -> dict[str, str]:
    """What cmd.exe's `set` prints, as an environment. Names are upper-cased, as Windows ignores
    their case; the drive entries (=C:=...) are left out."""
    env = {}
    for line in text.splitlines():
        key, sep, value = line.partition("=")
        if sep and key:
            env[key.upper()] = value
    return env


def kept_choices(cache: dict[str, str]) -> dict[str, str]:
    """What a build folder keeps of --source, --disc and --probe-baker (OPENRAC_<ID>_SOURCE,
    _RETAIL_IMAGE, OPENRAC_PROBE_BAKER)."""
    return {
        key: value
        for key, value in cache.items()
        if key.startswith("OPENRAC_")
        and (key.endswith("_SOURCE") or key.endswith("_RETAIL_IMAGE") or key == "OPENRAC_PROBE_BAKER")
        and value
    }


def checkout_version() -> str | None:
    """OpenRAC's version, as the port's build reads it (port/cmake/BuildInfo.cmake)."""
    try:
        conf = json.loads((ROOT / "launcher" / "src-tauri" / "tauri.conf.json").read_text(encoding="utf-8"))
    except (OSError, ValueError):
        return None
    version = conf.get("version")
    return version if isinstance(version, str) else None


def checkout_commit() -> str | None:
    """The checkout's commit, or None outside git."""
    git = shutil.which("git")
    if git is None:
        return None
    done = subprocess.run([git, "-C", str(ROOT), "rev-parse", "HEAD"], capture_output=True, text=True)
    commit = done.stdout.strip()
    return commit if done.returncode == 0 and len(commit) == 40 else None


def write_stamp(build: Path, names: list[str]) -> None:
    """openrac-build.json: what this build is of, for the launcher (port_build.rs). Games built
    before with the same version and commit stay listed."""
    stamp = build / "openrac-build.json"
    version, commit = checkout_version(), checkout_commit()
    games = set(names)
    try:
        old = json.loads(stamp.read_text(encoding="utf-8"))
        if old.get("version") == version and old.get("commit") == commit:
            games |= {g for g in old.get("games", []) if isinstance(g, str)}
    except (OSError, ValueError, AttributeError):
        pass
    stamp.write_text(json.dumps({"version": version, "commit": commit, "games": sorted(games)}, indent=2) + "\n",
                     encoding="utf-8")


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
    disc: Path | None = None,
    kept: dict[str, str] | None = None,
    defines: dict[str, str] | None = None,
) -> list[list[str]]:
    """The command lines to run, in order. A folder not configured yet is configured from the
    preset; a configured one is only built, after a configure without the preset when `source`
    names another decompilation than it builds from. The preset names its compilers (clang,
    clang++), and a name that finds another Clang than the cached one makes CMake drop the whole
    cache, so a configure from the preset passes the folder's own compilers, else `compilers`
    (C, C++, Ninja) when given. `python` is the interpreter hostgen runs under in a new folder
    (this one: CMake's own choice may be an older Python than hostgen reads). `disc` is the
    player's image of the first game's disc, kept as its OPENRAC_<ID>_RETAIL_IMAGE: hostgen reads
    the instructions of the functions with no C from it (port/tools/hostgen/retail.py). `kept`:
    choices a deleted folder held (kept_choices), passed again where `source` and `disc` do not
    name new ones. `defines`: more cache entries to set (the probe baker's)."""
    wanted = dict(kept or {})
    wanted.update(defines or {})
    if source is not None:
        wanted[source_variable(names[0])] = Path(source).as_posix()
    if disc is not None:
        wanted[retail_variable(names[0])] = Path(disc).as_posix()
    if not cache or not (build / "build.ninja").is_file():
        configure = [cmake, "--preset", preset]
        found = dict(zip(("CMAKE_C_COMPILER", "CMAKE_CXX_COMPILER", "CMAKE_MAKE_PROGRAM"), compilers or ()))
        if python:
            found["Python3_EXECUTABLE"] = python
        c_compiler = cache.get("CMAKE_C_COMPILER") or found.get("CMAKE_C_COMPILER")
        if c_compiler and os.name == "nt":
            # Clang on Windows compiles resources with llvm-rc, beside it. CMake 4.3 finds no
            # resource compiler for its check projects without being told: SDL's configure checks
            # stopped with "CMAKE_RC_COMPILER not set".
            rc = Path(c_compiler).with_name("llvm-rc.exe")
            if rc.is_file():
                found["CMAKE_RC_COMPILER"] = rc.as_posix()
        for key in ("CMAKE_C_COMPILER", "CMAKE_CXX_COMPILER", "CMAKE_RC_COMPILER", "CMAKE_MAKE_PROGRAM",
                    "Python3_EXECUTABLE"):
            value = cache.get(key) or found.get(key)
            if value:
                configure.append(f"-D{key}={value}")
        configure += [f"-D{key}={value}" for key, value in wanted.items()]
        out = [configure]
    else:
        changed = [f"-D{key}={value}" for key, value in wanted.items() if not same_path(cache.get(key, ""), value)]
        out = [[cmake, "-S", str(PORT), "-B", str(build), *changed]] if changed else []
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
    parser.add_argument("--disc", type=Path, metavar="IMAGE",
                        help="the image of GAME's disc (one game): the functions the decompilation has no C for "
                             "are translated from its programs (kept for later builds)")
    parser.add_argument("--preset", default="release", help="the CMake preset (port/CMakePresets.json); default release")
    parser.add_argument("--target", action="append", default=[], metavar="NAME",
                        help="also build this target of the port (a test program such as runtime_test)")
    parser.add_argument("--probe-baker", action="store_true",
                        help="also build openrac-probe-bake, the enhanced lighting's baker, with a pinned Embree "
                             "built from source (kept for later builds; left out with a warning if it fails)")
    parser.add_argument("--clean", action="store_true",
                        help="delete the build folder first and build everything again (its --source and --disc kept)")
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
    disc = None
    if args.disc is not None:
        if len(names) != 1:
            parser.error("--disc is one game's disc: name that one GAME")
        disc = args.disc.resolve()
        if not disc.is_file():
            parser.error(f"--disc {args.disc}: no such file")

    build = PORT / "build" / args.preset
    cache = read_cache(build)
    kept: dict[str, str] = {}
    if args.clean and build.exists():
        kept = kept_choices(cache)
        say(f"deleting {build.relative_to(ROOT).as_posix()} to build everything again")
        remove_tree(build)
        cache = {}
    try:
        env, cmake, compilers = prepare(cache)
    except BuildError as e:
        print(f"error: {e}", file=sys.stderr, flush=True)
        return 1

    for name in names:
        decompilation = (source if name == names[0] and source
                         else cache.get(source_variable(name)) or kept.get(source_variable(name)) or default_source(name))
        say(f"{name}: from {decompilation}")
    python = Path(sys.executable).as_posix()
    more = list(args.target)
    defines: dict[str, str] = {}
    if args.probe_baker or cache.get("OPENRAC_PROBE_BAKER") == "ON" or kept.get("OPENRAC_PROBE_BAKER") == "ON":
        try:
            defines = {"OPENRAC_PROBE_BAKER": "ON", "embree_DIR": build_embree(str(cmake), env, compilers,
                                                                                 cache).as_posix()}
            more.append(BAKER)
        except (BuildError, OSError) as e:
            print(f"warning: the probe baker is left out: {e}", file=sys.stderr, flush=True)
            defines = {"OPENRAC_PROBE_BAKER": "OFF"}
        kept.pop("OPENRAC_PROBE_BAKER", None)
    for command in commands(str(cmake), args.preset, build, names, source, cache, compilers, python, more,
                            disc, kept, defines):
        say("> " + " ".join(f'"{part}"' if " " in part else part for part in command))
        status = subprocess.run(command, cwd=PORT, env=env).returncode
        if status != 0:
            print(f"error: the build stopped ({Path(command[0]).name} {command[1]} exited with {status})",
                  file=sys.stderr, flush=True)
            return 1
    write_stamp(build, names)
    suffix = ".exe" if WINDOWS else ""
    for name in names:
        # On macOS a windowed game is an application bundle (port/cmake/MacApp.cmake).
        program = next((build / "apps" / f"openrac-{name}").glob("*.app"), build / f"openrac-{name}{suffix}")
        say(f"built {program.relative_to(ROOT).as_posix()}")
    if BAKER in more:
        try:
            copy_licenses(build)
        except (BuildError, OSError) as e:
            print(f"warning: {e}", file=sys.stderr, flush=True)
        say(f"built {(build / (BAKER + suffix)).relative_to(ROOT).as_posix()}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
