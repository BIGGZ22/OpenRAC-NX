#!/usr/bin/env python3
"""
Full build with the SN ProDG toolchain, followed by the match audit.
Cross-platform Python replacement for tools/build_sn.sh (works on Windows, Linux, macOS).

Usage:
    python tools/build_sn.py
"""
import os
import re
import shutil
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
    """Ensure PATH contains Git usr/bin tools (grep, cut, sed, bash) on Windows."""
    env = os.environ.copy()
    current_path = env.get("PATH", "")
    additions = [p for p in GIT_PATHS if os.path.isdir(p) and p not in current_path]
    if additions:
        env["PATH"] = ";".join(additions) + ";" + current_path
    
    # Force WINE empty on Windows to prevent toolchain.sh from guessing wrong
    if os.name == 'nt':
        env["WINE"] = ""
        
    return env

def build_sn_data(tc_as, env):
    """Assembles the standalone data/rodata .s files."""
    data_dir = REPO_ROOT / "asm" / "data"
    build_dir = REPO_ROOT / "build-sn"
    build_dir.mkdir(parents=True, exist_ok=True)

    for s_file in sorted(data_dir.glob("*.s")):
        out = build_dir / f"{s_file.stem}.o"
        cmd = [str(tc_as), "-I", "include-sn", "-I", "include", "-o", str(out), str(s_file)]
        res = subprocess.run(cmd, cwd=REPO_ROOT, env=env, capture_output=True, text=True)
        if res.returncode != 0:
            print(f"*** Failed assembling {s_file}: {res.stderr}")
            sys.exit(1)

    # core_rdata split
    res = subprocess.run([
        sys.executable, "tools/split_data_s.py",
        "asm/data/core_rdata.rodata.s", "build-sn/core_rdata", "D_00152B18"
    ], cwd=REPO_ROOT, env=env, capture_output=True, text=True)
    if res.returncode != 0:
        print(f"*** split_data_s failed: {res.stderr}")
        sys.exit(1)

    for n in (1, 2):
        out = build_dir / f"core_rdata_{n}.o"
        src = build_dir / f"core_rdata_{n}.s"
        subprocess.run([str(tc_as), "-I", "include-sn", "-I", "include", "-o", str(out), str(src)],
                       cwd=REPO_ROOT, env=env, check=True)

    # bss padding objects
    bash_exe = shutil.which("bash", path=env.get("PATH"))
    subprocess.run([bash_exe, "tools/build_sn_data.sh"], cwd=REPO_ROOT, env=env, check=True)

def make_bss_equs(tc_ld, tc_as, env):
    """Collect undefined symbols from linker complaints and equate them."""
    ld_undef_log = REPO_ROOT / "build-sn" / "ld_undef.log"
    cmd = [str(tc_ld), "-T", "build-sn/rac1.ld", "-o", "build-sn/rac1.elf"]
    r = subprocess.run(cmd, cwd=REPO_ROOT, env=env, capture_output=True, text=True)
    all_output = (r.stdout or "") + (r.stderr or "")
    ld_undef_log.write_text(all_output, encoding="utf-8")

    matches = re.findall(r"undefined reference to [`']([^']+)'", all_output)
    undef_syms = sorted(set(matches))
    undef_file = REPO_ROOT / "build-sn" / "undefined_syms.txt"
    undef_file.write_text("\n".join(undef_syms) + ("\n" if undef_syms else ""), encoding="utf-8")

    subprocess.run([sys.executable, "tools/gen_bss_equs.py"], cwd=REPO_ROOT, env=env, check=True)
    subprocess.run([str(tc_as), "-o", "build-sn/bss_equs.o", "build-sn/bss_equs.s"],
                   cwd=REPO_ROOT, env=env, check=True)

def main():
    tc_bin = REPO_ROOT / "toolchain" / "sn-prodg-3.01" / "usr" / "local" / "sce" / "ee" / "gcc" / "bin"
    tc_make = tc_bin / "make.exe"
    tc_ld = tc_bin / "ee-ld.exe"
    tc_as = tc_bin / "ee-as.exe"

    if not (REPO_ROOT / "asm" / "nonmatchings").is_dir():
        sys.exit("asm/ missing: run setup_asm")
    if not tc_make.is_file():
        sys.exit("toolchain/sn-prodg-3.01 missing (see README.md)")
    if not (REPO_ROOT / "toolchain" / "sn-prodg-24").is_dir():
        sys.exit("toolchain/sn-prodg-24 missing (see README.md)")

    env = ensure_env()

    # 1. Build standalone data objects if needed
    build_sn_data(tc_as, env)

    # 2. Always clean object dirs to prevent stale matches
    for d in ["core", "libgcc", "game"]:
        p = REPO_ROOT / "build-sn" / d
        if p.exists():
            shutil.rmtree(p, ignore_errors=True)

    # 3. Run make
    print("Compiling translation units (Makefile.sn)...")
    make_res = subprocess.run([str(tc_make), "-f", "Makefile.sn"], cwd=REPO_ROOT, env=env, capture_output=True, text=True)
    (REPO_ROOT / "build-sn" / "make.log").write_text((make_res.stdout or "") + (make_res.stderr or ""), encoding="utf-8")

    if make_res.returncode != 0:
        err = (make_res.stderr or "") + (make_res.stdout or "")
        tail = "\n".join(err.splitlines()[-20:])
        print(tail)
        sys.exit(f"*** make failed (exit {make_res.returncode})")

    # 4. Generate rac1.ld
    bash_exe = shutil.which("bash", path=env.get("PATH"))
    subprocess.run([bash_exe, "rac1.ld.sh"], cwd=REPO_ROOT, env=env, check=True)
    print("wrote build-sn/rac1.ld")

    # 5. Handle bss equates
    bss_equs = REPO_ROOT / "build-sn" / "bss_equs.o"
    if not bss_equs.exists():
        make_bss_equs(tc_ld, tc_as, env)

    # 6. Link
    ld_cmd = [str(tc_ld), "-T", "build-sn/rac1.ld", "build-sn/bss_equs.o", "-o", "build-sn/rac1.elf"]
    ld_res = subprocess.run(ld_cmd, cwd=REPO_ROOT, env=env, capture_output=True, text=True)
    if ld_res.returncode != 0:
        print("link failed -- regenerating build-sn/bss_equs.o and retrying once")
        make_bss_equs(tc_ld, tc_as, env)
        ld_res = subprocess.run(ld_cmd, cwd=REPO_ROOT, env=env, capture_output=True, text=True)
        if ld_res.returncode != 0:
            err = (ld_res.stderr or "") + (ld_res.stdout or "")
            print("\n".join(err.splitlines()[-10:]))
            sys.exit("*** link failed")

    # 7. Audit results
    print("\nAuditing matches...")
    sweep_res = subprocess.run([sys.executable, "tools/sweep_matches.py"], cwd=REPO_ROOT, env=env, capture_output=True, text=True)
    out = sweep_res.stdout or ""
    # Filter summary block
    summary_match = re.search(r"(=== \d+ decompiled functions audited ===.*?byte mismatch:\s+\d+)", out, re.DOTALL)
    if summary_match:
        print(summary_match.group(1))
    else:
        print("\n".join(out.splitlines()[-10:]))

    layout_res = subprocess.run([sys.executable, "tools/check_layout.py"], cwd=REPO_ROOT, env=env, capture_output=True, text=True)
    layout_out = (layout_res.stdout or "").strip().splitlines()
    if layout_out:
        print(layout_out[-1])

if __name__ == "__main__":
    main()
