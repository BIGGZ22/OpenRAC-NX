#!/usr/bin/env python3
"""
Decompile a level's code in Ghidra, with this repository's function names.

The Ghidra MCP project (docs/CONTAINERS.md) holds only the executable, so a
level's own code (the part that replaces `main` when the level loads,
docs/OVERLAYS.md) is not there, and the headless server cannot import files.
This builds a flat image of the level's memory (the executable's resident
code below 0x15F000 from baserom/SCES_509.16, then the level's records at
their load addresses from baserom/overlays/level_NN/), imports it into a
separate scratch project inside the running ghidra-ps2-mcp container with
analyzeHeadless (the container's own project is not touched), creates a
function at every address in config/overlays/functions.tsv (level places) and
at every func_XXXXXXXX of the executable, named as here, and writes
the decompiler's C for the requested functions.

  python3 tools/ghidra_level.py 18                    # every function still INCLUDE_ASM in src/overlays/lNN_*/
  python3 tools/ghidra_level.py 18 func_L18_002DD8A8  # just these
  # -> build-sn/ghidra/lNN/out/<name>.c

The output is a reference for structure only (stack layout, which address a
value is hoisted from, call order); it is never pasted into src/. Needs
`podman start ghidra-ps2-mcp` first. Nothing generated here is tracked.
"""
import glob
import json
import os
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
CONTAINER = "ghidra-ps2-mcp"
EXE_VADDR_DELTA = 0xFF080      # vaddr - file offset in baserom/SCES_509.16
BASE = 0x100000


def sh(*cmd, check=True):
    return subprocess.run(cmd, check=check, capture_output=True, text=True)


def main():
    if len(sys.argv) < 2:
        sys.exit(__doc__)
    level = int(sys.argv[1])
    wanted = sys.argv[2:]
    out = ROOT / "build-sn" / "ghidra" / f"l{level:02d}"
    out.mkdir(parents=True, exist_ok=True)
    ldir = ROOT / "baserom" / "overlays" / f"level_{level:02d}"
    manifest = json.loads((ldir / "manifest.json").read_text())
    text = next(r for r in manifest["records"] if r["name"] == "text")
    end = text["address"] + text["bytes"]

    img = bytearray(end - BASE)
    exe = (ROOT / "baserom" / "SCES_509.16").read_bytes()
    for v in range(0x100080, 0x15F000, 0x10000):
        n = min(0x10000, 0x15F000 - v)
        img[v - BASE:v - BASE + n] = exe[v - EXE_VADDR_DELTA:v - EXE_VADDR_DELTA + n]
    for r in manifest["records"]:
        if r["name"] == "bss":
            continue
        b = (ldir / f"{r['name']}.bin").read_bytes()
        img[r["address"] - BASE:r["address"] - BASE + len(b)] = b
    (out / "image.bin").write_bytes(img)

    names = {}
    for line in (ROOT / "config/overlays/functions.tsv").read_text().splitlines():
        if line.startswith("#"):
            continue
        f = line.split("\t")
        for place in f[5].split(","):
            lv, addr = place.split(":")
            if int(lv) == level:
                names[int(addr, 16)] = f[0]
    for p in glob.glob(str(ROOT / "asm/nonmatchings/*/func_*.s")):
        n = os.path.basename(p)[:-2]
        names.setdefault(int(n[5:], 16), n)
    (out / "names.tsv").write_text("".join(f"{a:08x}\t{n}\n" for a, n in sorted(names.items())))

    if not wanted:
        for p in sorted(glob.glob(str(ROOT / "src/overlays" / f"l{level:02d}_*" / "*.c"))):
            wanted += re.findall(r'INCLUDE_ASM\("asm/overlays", (func_L\d+_[0-9A-F]+)\)', Path(p).read_text())
    (out / "request.txt").write_text("\n".join(wanted))

    work = "/tmp/ghidra_level"
    sh("podman", "exec", CONTAINER, "sh", "-c", f"rm -rf {work} {work}proj; mkdir -p {work} {work}proj")
    for name, src in (("image.bin", out / "image.bin"), ("names.tsv", out / "names.tsv"),
                      ("request.txt", out / "request.txt"),
                      ("L18Decomp.java", ROOT / "tools/ghidra_level_decomp.java")):
        sh("podman", "cp", str(src), f"{CONTAINER}:{work}/{name}")
    r = sh("podman", "exec", CONTAINER, "sh", "-c",
           f"cd /tmp && timeout 1800 /opt/ghidra/support/analyzeHeadless {work}proj L -import {work}/image.bin "
           f"-processor r5900:LE:32:default -loader BinaryLoader -loader-baseAddr {BASE:#x} -noanalysis "
           f"-scriptPath {work} -postScript L18Decomp.java {work}/names.tsv {work}/request.txt {work}/out 2>&1 | tail -4",
           check=False)
    print(r.stdout)
    shutil_out = out / "out"
    sh("rm", "-rf", str(shutil_out))
    sh("podman", "cp", f"{CONTAINER}:{work}/out", str(shutil_out))
    print(f"{len(list(shutil_out.glob('*.c')))} functions decompiled into {shutil_out}")


if __name__ == "__main__":
    main()
