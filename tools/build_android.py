#!/usr/bin/env python3
"""
Builds OpenRAC for Android: the launcher's APK with the games in it
(docs/port/ANDROID.md).

  python3 tools/build_android.py [GAME ...] [--release] [--keep-symbols] [--no-apk]

GAME is one of the port's games (port/game/<id>/hostgen.json), rac1-pal and
rac1-ntsc when none is named. The steps:

  1. a Python for the launcher's tools: CPython for Android as Chaquopy
     packages it (MIT), from Maven Central at a fixed version, each file
     checked against its SHA-256, into build/android/
  2. the port with the NDK (port's android-arm64 preset): for each game
     libopenrac-GAME.so (the game's activity loads it) and
     libopenrac-GAME-cli.so (its windowless checks), SDL3, libadrenotools'
     hooks and libopenrac-python.so
  3. those libraries, the NDK's C++ runtime and Python's, in the launcher's
     Android project (src-tauri/gen/android/app/src/main/jniLibs), and SDL's
     Java beside the game's activity (app/src/main/sdl-java)
  4. the copy of OpenRAC the launcher carries (tools/bundle_standalone.py
     --android) as the APK's assets/openrac.zip
  5. the APK: `yarn tauri android build` (a debug build, signed with the
     debug key so that it installs as it is; --release for an unsigned
     release build to sign yourself). Its libraries carry no debug
     information unless --keep-symbols is given.

It needs the Android SDK (ANDROID_HOME) with the NDK (ANDROID_NDK_HOME or
NDK_HOME, r27 or newer), a JDK 17 or newer, Rust with the
aarch64-linux-android target, Node with Corepack, CMake, Ninja, and a host
Clang for hostgen. --no-apk stops after step 4 (the Android Studio project in
launcher/src-tauri/gen/android is then ready to open). Standard library only.
"""

import argparse
import hashlib
import os
import shutil
import subprocess
import sys
import urllib.request
import zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
PORT = ROOT / "port"
LAUNCHER = ROOT / "launcher"
APP = LAUNCHER / "src-tauri/gen/android/app"
BUILD = ROOT / "build/android"
PORT_BUILD = PORT / "build/android-arm64"
DEFAULT_GAMES = ("rac1-pal", "rac1-ntsc")

# Chaquopy's CPython for Android (https://github.com/chaquo/chaquopy, MIT): the interpreter as a
# library with its modules, and its standard library, published on Maven Central.
PYTHON_VERSION = "3.13"
PYTHON_PACKAGE = "3.13.16-0"
PYTHON_MAVEN = "https://repo1.maven.org/maven2/com/chaquo/python/target"
PYTHON_FILES = {
    "arm64-v8a": "fae8cfd4d3910b6f978b22279fe820c0c6ba49d0ea20f7b106aa6130f1c8f355",
    "stdlib": "68e3f71e6e2f2a2febb22f2d3575657787b3dd94f07d2c6f3780461babbcbf58",
}
# libadrenotools' hook libraries, which it loads by path from the app's native library folder.
ADRENOTOOLS_HOOKS = ("hook_impl", "main_hook", "file_redirect_hook", "gsl_alloc_hook")


def run(cmd: list, cwd: Path, env: dict | None = None) -> None:
    print("+", " ".join(str(c) for c in cmd), flush=True)
    subprocess.run([str(c) for c in cmd], cwd=cwd, env=env, check=True)


def ndk_home() -> Path:
    for name in ("ANDROID_NDK_HOME", "NDK_HOME", "ANDROID_NDK_ROOT", "ANDROID_NDK_LATEST_HOME"):
        value = os.environ.get(name)
        if value and (Path(value) / "build/cmake/android.toolchain.cmake").is_file():
            return Path(value)
    sdk = os.environ.get("ANDROID_HOME") or os.environ.get("ANDROID_SDK_ROOT")
    if sdk:
        found = sorted((Path(sdk) / "ndk").glob("*/build/cmake/android.toolchain.cmake"))
        if found:
            return found[-1].parents[2]
    sys.exit("The Android NDK was not found: set ANDROID_NDK_HOME (or install it with the SDK's sdkmanager).")


def download(name: str, sha256: str) -> Path:
    file = f"target-{PYTHON_PACKAGE}-{name}.zip"
    dest = BUILD / "downloads" / file
    if not dest.is_file() or hashlib.sha256(dest.read_bytes()).hexdigest() != sha256:
        dest.parent.mkdir(parents=True, exist_ok=True)
        url = f"{PYTHON_MAVEN}/{PYTHON_PACKAGE}/{file}"
        print(f"downloading {url}", flush=True)
        with urllib.request.urlopen(url) as response:
            data = response.read()
        got = hashlib.sha256(data).hexdigest()
        if got != sha256:
            sys.exit(f"{file}: its SHA-256 is {got}, not {sha256}")
        dest.write_bytes(data)
    return dest


def python() -> tuple[Path, Path]:
    """The Python package for the port's build (include/, jniLibs/) and the PYTHONHOME the launcher
    carries (lib/python3.N with its lib-dynload)."""
    target = BUILD / "python-target"
    home = BUILD / "python"
    stamp = BUILD / "python.stamp"
    if stamp.is_file() and stamp.read_text() == PYTHON_PACKAGE and target.is_dir() and home.is_dir():
        return target, home
    for path in (target, home):
        shutil.rmtree(path, ignore_errors=True)
    with zipfile.ZipFile(download("arm64-v8a", PYTHON_FILES["arm64-v8a"])) as z:
        z.extractall(target)
    lib = home / "lib" / f"python{PYTHON_VERSION}"
    with zipfile.ZipFile(download("stdlib", PYTHON_FILES["stdlib"])) as z:
        z.extractall(lib)
    shutil.copytree(target / "lib-dynload/arm64-v8a", lib / "lib-dynload")
    # What a player's tools never need, and the configuration of the other ABI.
    for name in ("test", "idlelib", "tkinter", "turtledemo", "ensurepip", "lib2to3", "pydoc_data",
                 f"config-{PYTHON_VERSION}-x86_64-linux-android",
                 "_sysconfigdata__android_x86_64-linux-android.py"):
        path = lib / name
        shutil.rmtree(path) if path.is_dir() else path.unlink(missing_ok=True)
    stamp.write_text(PYTHON_PACKAGE)
    return target, home


def build_port(games: list[str], ndk: Path, python_target: Path) -> None:
    env = dict(os.environ, ANDROID_NDK_HOME=str(ndk))
    run(["cmake", "--preset", "android-arm64", f"-DOPENRAC_ANDROID_PYTHON={python_target}"], PORT, env)
    targets = [t for g in games for t in (f"openrac-{g}", f"openrac-{g}-cli")]
    targets += ["openrac-python", "SDL3-shared", *ADRENOTOOLS_HOOKS]
    run(["cmake", "--build", PORT_BUILD, "--target", *targets], PORT, env)


def collect_libraries(games: list[str], ndk: Path, python_target: Path, keep_symbols: bool) -> list[str]:
    dest = APP / "src/main/jniLibs/arm64-v8a"
    dest.mkdir(parents=True, exist_ok=True)
    # What an earlier run put here; the launcher's own library is Tauri's to place.
    for old in dest.glob("*.so"):
        if old.name != "libopenrac_launcher_lib.so":
            old.unlink()
    wanted = [PORT_BUILD / f"libopenrac-{g}.so" for g in games]
    wanted += [PORT_BUILD / f"libopenrac-{g}-cli.so" for g in games]
    wanted.append(PORT_BUILD / "libopenrac-python.so")
    for name in ("SDL3", *ADRENOTOOLS_HOOKS):
        found = sorted(PORT_BUILD.rglob(f"lib{name}.so"))
        if not found:
            sys.exit(f"the port's build has no lib{name}.so")
        wanted.append(found[0])
    runtime = sorted(ndk.glob("toolchains/llvm/prebuilt/*/sysroot/usr/lib/aarch64-linux-android/libc++_shared.so"))
    if not runtime:
        sys.exit(f"{ndk} has no libc++_shared.so for aarch64")
    wanted.append(runtime[0])
    wanted += sorted((python_target / "jniLibs/arm64-v8a").glob("*.so"))
    strip = sorted(ndk.glob("toolchains/llvm/prebuilt/*/bin/llvm-strip"))
    names = []
    for path in wanted:
        if not path.is_file():
            sys.exit(f"{path} was not built")
        shutil.copy2(path, dest / path.name, follow_symlinks=True)
        # Gradle strips them again; this way the APK carries no debug info even when it cannot.
        if strip and not keep_symbols:
            run([strip[0], "--strip-debug", dest / path.name], ROOT)
        names.append(path.name)
    # SDL's activity and its helpers, as the SDL the port was built with has them.
    sdl_java = sorted(PORT_BUILD.rglob("android-project/app/src/main/java/org/libsdl"))
    if not sdl_java:
        sys.exit("the port's build has no SDL Java sources (android-project/app/src/main/java)")
    java = APP / "src/main/sdl-java"
    shutil.rmtree(java, ignore_errors=True)
    shutil.copytree(sdl_java[0], java / "org/libsdl")
    return names


def bundle(python_home: Path) -> Path:
    out = BUILD / "openrac"
    run([sys.executable, ROOT / "tools/bundle_standalone.py", out, "--android", "--python", python_home], ROOT)
    assets = APP / "src/main/assets"
    assets.mkdir(parents=True, exist_ok=True)
    archive = assets / "openrac.zip"
    with zipfile.ZipFile(archive, "w", zipfile.ZIP_DEFLATED, compresslevel=6) as z:
        for path in sorted(out.rglob("*")):
            if path.is_file():
                z.write(path, path.relative_to(out).as_posix())
    print(f"{archive}: {archive.stat().st_size / 1e6:.0f} MB")
    return archive


def build_apk(ndk: Path, release: bool, keep_symbols: bool) -> None:
    env = dict(os.environ, NDK_HOME=str(ndk), ANDROID_NDK_HOME=str(ndk))
    if keep_symbols:
        env["OPENRAC_ANDROID_KEEP_SYMBOLS"] = "1"  # app/build.gradle.kts
    else:
        # The launcher's own library, which a debug build compiles with debug information.
        env.setdefault("CARGO_PROFILE_DEV_DEBUG", "0")
    yarn = shutil.which("yarn") or "yarn"
    run([yarn, "install", "--immutable"], LAUNCHER, env)
    cmd = [yarn, "tauri", "android", "build", "--apk", "--target", "aarch64"]
    if not release:
        cmd.append("--debug")
    run(cmd, LAUNCHER, env)
    for apk in sorted((APP / "build/outputs/apk").rglob("*.apk")):
        print(f"APK: {apk} ({apk.stat().st_size / 1e6:.0f} MB)")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("games", nargs="*", default=list(DEFAULT_GAMES))
    parser.add_argument("--release", action="store_true", help="an unsigned release APK (default: debug)")
    parser.add_argument("--keep-symbols", action="store_true", help="keep the libraries' debug information")
    parser.add_argument("--no-apk", action="store_true", help="stop before Gradle (steps 1 to 4)")
    args = parser.parse_args()
    for game in args.games:
        if not (PORT / "game" / game / "hostgen.json").is_file():
            sys.exit(f"{game} is not one of the port's games (port/game/<id>/hostgen.json)")
    ndk = ndk_home()
    python_target, python_home = python()
    build_port(args.games, ndk, python_target)
    names = collect_libraries(args.games, ndk, python_target, args.keep_symbols)
    print("native libraries:", ", ".join(names))
    bundle(python_home)
    if not args.no_apk:
        build_apk(ndk, args.release, args.keep_symbols)
    return 0


if __name__ == "__main__":
    sys.exit(main())
