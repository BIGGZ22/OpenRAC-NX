#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Builds dkgl's shader pack (renderer/dkgl/dkgl.h) from programs a Vulkan build dumped with
# OPENRAC_DUMP_SHADERS: for each <key>.prog with its .vert/.frag sources, the SPIR-V the Vulkan
# backend compiled goes through SPIRV-Cross (GLSL 460) and uam (deko3d's compiler) into
# <pack key>.vert.dksh / .frag.dksh, beside a copy of the .prog (its reflection). The pack key is
# FNV-1a 64 over "vertex source\0fragment source\0", what dkgl hashes at glLinkProgram.
# Compute programs (the Vulkan backend's own culling) are skipped.
#
#   dk_pack.py DUMP_DIR... --out PACK_DIR --spirv-cross PATH --uam PATH
import argparse, os, shutil, struct, subprocess, sys

def fnv(*parts):
    h = 0xcbf29ce484222325
    for p in parts:
        for c in p + b"\0":
            h = ((h ^ c) * 0x100000001b3) & 0xFFFFFFFFFFFFFFFF
    return h

def spirv_words(blob):
    at = 0
    out = []
    for _ in range(3):  # vertex, fragment, compute
        n = struct.unpack_from("<I", blob, at)[0]
        at += 4
        out.append(blob[at:at + 4 * n])
        at += 4 * n
    return out

def run(cmd):
    r = subprocess.run(cmd, capture_output=True, text=True)
    return r.returncode, (r.stdout + r.stderr).strip()

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("dumps", nargs="+")
    ap.add_argument("--out", required=True)
    ap.add_argument("--spirv-cross", default="spirv-cross")
    ap.add_argument("--uam", default="uam")
    ap.add_argument("--work", default=None)
    a = ap.parse_args()
    os.makedirs(a.out, exist_ok=True)
    work = a.work or os.path.join(a.out, "..", "dk_pack_work")
    os.makedirs(work, exist_ok=True)
    made, failed, warnings = 0, 0, []
    seen = set()
    for dump in a.dumps:
        for name in sorted(os.listdir(dump)):
            if not name.endswith(".prog"):
                continue
            base = os.path.join(dump, name[:-5])
            if not (os.path.exists(base + ".vert") and os.path.exists(base + ".frag")):
                continue  # compute
            vert = open(base + ".vert", "rb").read()
            frag = open(base + ".frag", "rb").read()
            key = fnv(vert, frag)
            if key in seen:
                continue
            seen.add(key)
            blob = open(base + ".prog", "rb").read()
            vs, fs, _ = spirv_words(blob)
            ok = True
            for stage, spv in (("vert", vs), ("frag", fs)):
                w = os.path.join(work, f"{key:016x}.{stage}")
                open(w + ".spv", "wb").write(spv)
                flip = ["--flip-vert-y"] if stage == "vert" else []  # deko3d 0.5: row 0 at NDC y = -1 (dkgl.h)
                rc, log = run([a.spirv_cross, w + ".spv", "--version", "460", "--no-es", *flip, "--output", w + ".glsl"])
                if rc != 0:
                    print(f"FAIL {key:016x} {stage} spirv-cross: {log}", file=sys.stderr)
                    ok = False
                    continue
                out = os.path.join(a.out, f"{key:016x}.{stage}.dksh")
                rc, log = run([a.uam, "-s", stage, "-o", out, w + ".glsl"])
                if rc != 0 or not os.path.exists(out):
                    print(f"FAIL {key:016x} {stage} uam: {log}", file=sys.stderr)
                    ok = False
                elif log:
                    warnings.append(f"{key:016x} {stage}: {log}")
            if ok:
                shutil.copyfile(base + ".prog", os.path.join(a.out, f"{key:016x}.prog"))
                made += 1
            else:
                failed += 1
    for w in warnings:
        print("warning", w)
    print(f"pack: {made} programs, {failed} failed -> {a.out}")
    return 1 if failed else 0

if __name__ == "__main__":
    sys.exit(main())
