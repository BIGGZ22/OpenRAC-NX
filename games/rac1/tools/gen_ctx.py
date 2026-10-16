#!/usr/bin/env python3
"""
Generate ctx.c: our recovered types, preprocessed into plain C that m2c's
pycparser front end can read. Cross-platform replacement for gen_ctx.sh.
"""
import os
import subprocess
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent

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

def generate_ctx():
    cpp = REPO_ROOT / "toolchain" / "sn-prodg-24" / "local" / "sce" / "ee" / "gcc" / "bin" / "ee-gcc2953.exe"
    build_dir = REPO_ROOT / "build-sn"
    build_dir.mkdir(parents=True, exist_ok=True)
    
    ctx_in = build_dir / "ctx_in.c"
    ctx_out = REPO_ROOT / "ctx.c"

    ctx_in.write_text('#include "common.h"\n#include "structs.h"\n', encoding="utf-8")

    cmd = [
        str(cpp),
        "-E", "-P",
        "-Iinclude",
        "-D__attribute__(x)=",
        "-D__asm__(x)=",
        str(ctx_in)
    ]

    res = subprocess.run(cmd, cwd=REPO_ROOT, env=ensure_env(), capture_output=True, text=True)
    if res.returncode != 0:
        sys.exit(f"Preprocessing ctx failed: {res.stderr}")

    ctx_out.write_text(res.stdout, encoding="utf-8")
    if ctx_in.exists():
        ctx_in.unlink()

    lines = len(res.stdout.splitlines())
    print(f"wrote ctx.c ({lines} lines)")

if __name__ == "__main__":
    generate_ctx()
