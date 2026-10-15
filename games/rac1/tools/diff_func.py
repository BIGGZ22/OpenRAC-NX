#!/usr/bin/env python3
"""
Cross-platform diff helper for Ratchet & Clank decompilation.
Works directly on Windows PowerShell / Command Prompt / Git Bash.

Usage:
    python tools/diff_func.py func_XXXXXXXX [extra asm-differ options]
"""
import sys
import subprocess
from pathlib import Path

import os

REPO_ROOT = Path(__file__).resolve().parent.parent
TC_BIN = REPO_ROOT / "toolchain" / "sn-prodg-3.01" / "usr" / "local" / "sce" / "ee" / "gcc" / "bin"
TC_MAKE = TC_BIN / "make.exe"
TC_LD = TC_BIN / "ee-ld.exe"

GIT_PATHS = [
    r"C:\Program Files\Git\usr\bin",
    r"C:\Program Files\Git\bin",
    r"C:\Program Files (x86)\Git\usr\bin",
    r"C:\Program Files (x86)\Git\bin",
]

def ensure_env():
    env = os.environ.copy()
    current_path = env.get("PATH", "")
    additions = [p for p in GIT_PATHS if os.path.isdir(p) and p not in current_path]
    if additions:
        env["PATH"] = ";".join(additions) + ";" + current_path
    return env

def main():
    if len(sys.argv) < 2:
        print("Usage: python tools/diff_func.py func_XXXXXXXX [args]")
        sys.exit(1)

    func_name = sys.argv[1]
    extra_args = sys.argv[2:]
    env = ensure_env()

    # 1. Clean previous build artifacts
    for d in ["core", "libgcc", "game"]:
        p = REPO_ROOT / "build-sn" / d
        if p.exists():
            import shutil
            shutil.rmtree(p, ignore_errors=True)

    # 2. Make
    print(f"Building {func_name}...")
    make_res = subprocess.run([str(TC_MAKE), "-f", "Makefile.sn"], cwd=REPO_ROOT, env=env, capture_output=True, text=True)
    if make_res.returncode != 0:
        err = make_res.stderr or make_res.stdout
        print("\n".join(err.splitlines()[-20:]))
        print(f"*** make failed (exit {make_res.returncode}) -- NOT diffing ***")
        sys.exit(1)

    # 3. Link script
    subprocess.run([sys.executable, "tools/gen_ld.py"], cwd=REPO_ROOT, env=env, capture_output=True)

    # 4. Link
    ld_res = subprocess.run([str(TC_LD), "-T", "build-sn/rac1.ld", "build-sn/bss_equs.o", "-o", "build-sn/rac1.elf"], cwd=REPO_ROOT, env=env, capture_output=True, text=True)
    if ld_res.returncode != 0:
        print(ld_res.stderr or ld_res.stdout)
        print("*** link failed -- NOT diffing ***")
        sys.exit(1)

    # 5. Differ files
    subprocess.run([sys.executable, "tools/gen_differ_files.py"], cwd=REPO_ROOT, env=env, capture_output=True)

    # 6. asm-differ
    cmd = [sys.executable, "tools/ext/asm-differ/diff.py", "--no-pager", "-s", func_name] + extra_args
    subprocess.run(cmd, cwd=REPO_ROOT, env=env)

if __name__ == "__main__":
    main()
