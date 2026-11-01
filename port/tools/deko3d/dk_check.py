#!/usr/bin/env python3
# Checks that every OpenRAC program dumped by OPENRAC_DUMP_SHADERS converts to deko3d:
# SPDX-License-Identifier: GPL-3.0-or-later
# .prog (encode_program bytes) -> SPIR-V -> SPIRV-Cross (GLSL 460) -> uam (DKSH).
import os, struct, subprocess, sys, json

import argparse
SPVC = "spirv-cross"
UAM = "uam"

class R:
    def __init__(s, b): s.b, s.p = b, 0
    def u32(s):
        v = struct.unpack_from("<I", s.b, s.p)[0]; s.p += 4; return v
    def i32(s):
        v = struct.unpack_from("<i", s.b, s.p)[0]; s.p += 4; return v
    def words(s):
        n = s.u32(); w = s.b[s.p:s.p + 4 * n]; s.p += 4 * n; return w
    def text(s):
        n = s.u32(); t = s.b[s.p:s.p + n].decode("utf-8", "replace"); s.p += n; return t

def decode(b):
    r = R(b)
    p = {"vertex": r.words(), "fragment": r.words(), "compute": r.words()}
    p["default_binding"], p["default_size"] = r.i32(), r.i32()
    p["uniforms"] = []
    for _ in range(r.u32()):
        p["uniforms"].append(dict(name=r.text(), gl_type=r.u32(), offset=r.i32(), array_size=r.i32(),
                                  array_stride=r.i32(), sampler=r.u32(), binding=r.i32(), kind=r.u32()))
    p["blocks"] = [dict(name=r.text(), binding=r.i32(), size=r.i32()) for _ in range(r.u32())]
    p["storage"] = [dict(name=r.text(), binding=r.i32(), size=r.i32()) for _ in range(r.u32())]
    p["attributes"] = [dict(name=r.text(), location=r.i32(), gl_type=r.u32()) for _ in range(r.u32())]
    return p

def run(cmd):
    r = subprocess.run(cmd, capture_output=True, text=True)
    return r.returncode, (r.stdout + r.stderr).strip()

def main(src, out):
    os.makedirs(out, exist_ok=True)
    progs = sorted(f for f in os.listdir(src) if f.endswith(".prog"))
    summary = {"programs": len(progs), "ok": 0, "fail": []}
    for f in progs:
        key = f[:-5]
        p = decode(open(os.path.join(src, f), "rb").read())
        stages = [(s, p[n]) for s, n in (("vert", "vertex"), ("frag", "fragment"), ("comp", "compute")) if p[n]]
        ok = True
        for stage, spv in stages:
            base = os.path.join(out, f"{key}.{stage}")
            open(base + ".spv", "wb").write(spv)
            rc, log = run([SPVC, base + ".spv", "--version", "460", "--no-es", "--output", base + ".glsl"])
            if rc != 0:
                ok = False; summary["fail"].append({"key": key, "stage": stage, "step": "spirv-cross", "log": log[:2000]}); continue
            rc, log = run([UAM, "-s", stage, "-o", base + ".dksh", base + ".glsl"])
            if rc != 0 or not os.path.exists(base + ".dksh"):
                ok = False; summary["fail"].append({"key": key, "stage": stage, "step": "uam", "log": log[:2000]})
        summary["ok"] += ok
    json.dump(summary, open(os.path.join(out, "summary.json"), "w"), indent=1)
    print(f"{summary['ok']}/{summary['programs']} programs converted; {len(summary['fail'])} stage failures")
    for e in summary["fail"][:40]:
        print(f"- {e['key']} {e['stage']} {e['step']}: {e['log'].splitlines()[0] if e['log'] else ''}")

if __name__ == "__main__":
    ap = argparse.ArgumentParser(description="Convert OPENRAC_DUMP_SHADERS programs to deko3d DKSH")
    ap.add_argument("dump"); ap.add_argument("out")
    ap.add_argument("--spirv-cross", default="spirv-cross"); ap.add_argument("--uam", default="uam")
    a = ap.parse_args()
    SPVC, UAM = a.spirv_cross, a.uam
    main(a.dump, a.out)
