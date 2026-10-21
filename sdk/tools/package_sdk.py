#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""The SDK as one archive, for writing a mod without a copy of OpenRAC.

    python3 sdk/tools/package_sdk.py [-o DIR]

writes DIR/openrac-mod-sdk-<version>.zip (default DIR: dist/) and its .sha256: the headers, the
CMake package, the template, the example, the packaging tool, the README, MODS.md and the license,
in one top folder openrac-mod-sdk-<version>/. <version> is sdk/VERSION, MAJOR.MINOR.PATCH, whose
MAJOR.MINOR must be the interface's (openrac_mod.h, OPENRAC_MOD_API_MAJOR and _MINOR) and the CMake
package's. The archive is the same bytes wherever it is made (stored, sorted, fixed times), so a
pinned SHA-256 holds for every build of one version. The launcher workflow's sdk job publishes it on
a tag sdk-v<version> (.github/workflows/sdk.yml).
"""
from __future__ import annotations

import argparse
import hashlib
import re
import sys
import zipfile
from pathlib import Path

SDK = Path(__file__).resolve().parent.parent
ROOT = SDK.parent
# What goes in, relative to sdk/ (folders whole), and the files from elsewhere in OpenRAC.
PARTS = ["README.md", "VERSION", "include", "cmake", "template", "examples", "tools/package_mod.py"]
EXTRA = {"docs/MODS.md": ROOT / "docs" / "port" / "MODS.md", "LICENSE": ROOT / "COPYING"}
SKIP = {"__pycache__", ".DS_Store", "build", "dist"}
DATE = (2026, 1, 1, 0, 0, 0)


def version(sdk: Path = SDK) -> str:
    text = (sdk / "VERSION").read_text().strip()
    m = re.fullmatch(r"(\d+)\.(\d+)\.(\d+)", text)
    if not m:
        raise SystemExit(f"sdk/VERSION: {text!r} is not MAJOR.MINOR.PATCH")
    header = (sdk / "include" / "openrac" / "openrac_mod.h").read_text()
    api = tuple(re.search(rf"#define OPENRAC_MOD_API_{k} (\d+)", header).group(1) for k in ("MAJOR", "MINOR"))
    if (m.group(1), m.group(2)) != api:
        raise SystemExit(f"sdk/VERSION {text} is not the interface's {'.'.join(api)} (openrac_mod.h)")
    cmake = (sdk / "cmake" / "OpenRACModConfigVersion.cmake").read_text()
    if f'set(PACKAGE_VERSION "{api[0]}.{api[1]}")' not in cmake:
        raise SystemExit(f"cmake/OpenRACModConfigVersion.cmake does not say {'.'.join(api)}")
    return text


def files(sdk: Path = SDK) -> dict[str, Path]:
    out: dict[str, Path] = {}
    for part in PARTS:
        p = sdk / part
        if p.is_file():
            out[part] = p
            continue
        for f in sorted(p.rglob("*")):
            if f.is_file() and not SKIP.intersection(f.relative_to(sdk).parts):
                out[f.relative_to(sdk).as_posix()] = f
    for name, path in EXTRA.items():
        if path.is_file():
            out[name] = path
    return dict(sorted(out.items()))


def write(out_dir: Path, sdk: Path = SDK) -> Path:
    v = version(sdk)
    top = f"openrac-mod-sdk-{v}"
    out_dir.mkdir(parents=True, exist_ok=True)
    archive = out_dir / f"{top}.zip"
    with zipfile.ZipFile(archive, "w", zipfile.ZIP_STORED) as z:
        for name, path in files(sdk).items():
            info = zipfile.ZipInfo(f"{top}/{name}", DATE)
            info.external_attr = (0o755 if name.endswith(".py") else 0o644) << 16
            info.create_system = 3
            z.writestr(info, path.read_bytes())
    digest = hashlib.sha256(archive.read_bytes()).hexdigest()
    (out_dir / f"{top}.zip.sha256").write_text(f"{digest}  {archive.name}\n")
    return archive


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("-o", "--out", type=Path, default=Path("dist"))
    args = ap.parse_args(argv)
    archive = write(args.out)
    print(archive, (args.out / (archive.name + ".sha256")).read_text().split()[0])
    return 0


if __name__ == "__main__":
    sys.exit(main())
