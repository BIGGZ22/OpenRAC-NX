#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""
Build and place a candidate for fcheck as it would sit in a level (test-only).

  python3 build-sn/fcheck/gate/fcheck_level_build.py NAME CANDIDATE.c OUT.json LEVEL|exe

LEVEL is the level (0-18) whose call states the candidate runs from; `exe` keeps
fcheck's own addresses (the boot executable) and only adds the scratch source
below.

tools/fcheck_gate.py copies this file into rac1-decomp's git-ignored
build-sn/fcheck/gate/ and runs it there in rac1-decomp's container (where the
retail compiler is), from rac1-decomp's root. It is rac1-decomp's own
`tools/fcheck.py --build`, unchanged, with one thing swapped: the address a
symbol has. fcheck resolves every symbol to its address in the boot executable;
in a level the game's code above 0x15F000 is the level's overlay, where a
function of the executable sits somewhere else (config/overlays/functions.tsv
gives each function its place in each level) and the level's own data and
functions (D_LNN_/func_LNN_) are where their names say. So the symbols resolve
the way rac1-decomp's tools/overlay_check.py resolves them for level LEVEL:

  - _gp, the libgcc helpers, func_/D_ below 0x15F000: as fcheck has them;
  - func_LNN_Y and D_LNN_Y of this level: Y; func_LMM_Y of another level and a
    func_Y of the executable above 0x15F000: its place in this level;
  - a D_Y above 0x15F000 in the C of a level's own function (func_LNN_ checked
    in level NN): Y, the address the level's code uses; in an executable
    function that data is the executable's, which a level does not have, so it
    stays unresolved (fcheck says LINK);
  - names with a suffix after the address (D_0013E650_DD68, an alias a file
    keeps for its own declaration) resolve as the bare name.

Nothing in rac1-decomp is changed: the module is imported and one function
replaced in memory for this run.
"""
from __future__ import annotations

import importlib.util
import re
import sys
from pathlib import Path

# rac1-decomp's top folder, not OpenRAC's, so not openrac_common's repo_root: fcheck_gate.py
# copies this file alone into rac1-decomp and runs it from rac1-decomp's root in that project's
# container, where no OpenRAC checkout is. rac1-decomp's tools and baserom are read from here.
DECOMP = Path.cwd()
sys.path.insert(0, str(DECOMP / "tools"))

# The game's libc routines (port/game/rac1-pal/libraries.tsv), resident.
LIBC = {"memcpy": 0x00115248, "memset": 0x001153FC}
ADDRESSED = re.compile(r"^(D|func|jtbl)_(?:L(\d{2})_)?([0-9A-Fa-f]{8})(?:_\w+)?$")


def load_fcheck():
    spec = importlib.util.spec_from_file_location("fcheck", DECOMP / "tools/fcheck.py")
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


def resolver(name: str, level: int, original):
    """symbol_address for LEVEL: a symbol's address there, or None."""
    import overlay_check as oc

    own = (m := re.fullmatch(r"func_L(\d{2})_[0-9A-Fa-f]{8}", name)) is not None and int(m.group(1)) == level
    me = re.fullmatch(r"func_(?:L\d{2}_)?([0-9A-Fa-f]{8})", name)
    near = int(me.group(1), 16) if me else 0x200000

    def address(sym: str):
        # The compiler calls these by name for a block copy or clear: the game's own.
        if sym in LIBC:
            return LIBC[sym]
        m = re.fullmatch(r"func_([0-9A-Fa-f]{8})(?:_\w+)?", sym)
        if m and int(m.group(1), 16) >= oc.RESIDENT_MAX:
            at = exe_place(oc, f"func_{m.group(1).upper()}", level, near)
            if at is not None:
                return at
        try:
            return oc.resolve_symbol(sym, level, near)
        except oc.Unresolved:
            pass
        m = ADDRESSED.match(sym)
        if m:
            kind, lv, y = m.group(1), m.group(2), int(m.group(3), 16)
            bare = f"{kind}_" + (f"L{lv}_" if lv else "") + m.group(3).upper()
            if bare != sym:
                try:
                    return oc.resolve_symbol(bare, level, near)
                except oc.Unresolved:
                    pass
                try:
                    return oc.resolve_symbol(f"{kind}_" + (f"L{lv}_" if lv else "") + m.group(3), level, near)
                except oc.Unresolved:
                    pass
            if kind in ("D", "jtbl") and lv is None and own:
                return y
            if kind == "func" and lv is None and y >= oc.RESIDENT_MAX:
                return inside(oc, y, level)
            return None
        # A named symbol (config/symbol_addrs.txt): resident as is, otherwise by its func_ name.
        a = original(sym)
        if a is None:
            return None
        if a < oc.RESIDENT_MAX:
            return a
        try:
            return oc.resolve_symbol(f"func_{a:08X}", level, near)
        except oc.Unresolved:
            return a if own else None

    return address


_exe = None


def exe_bytes(address: int, n: int) -> bytes:
    global _exe
    if _exe is None:
        import struct
        raw = (DECOMP / "baserom/SCES_509.16").read_bytes()
        phoff = struct.unpack_from("<I", raw, 0x1C)[0]
        _t, poff, pvaddr = struct.unpack_from("<III", raw, phoff)
        _exe = (pvaddr - poff, raw)
    delta, raw = _exe
    return raw[address - delta:address - delta + n]


def exe_place(oc, name: str, level: int, near: int):
    """Of an executable function's places in LEVEL, the nearest whose code is the executable's
    but for call and jump targets, or None (then overlay_check's nearest place). The catalogue
    folds routines that differ in a constant or an offset under one name (func_001F9878 has five
    places in some levels, one of them its own code); the C means the executable's."""
    places = oc.places_in_level(name, level) if name in oc.load_catalogue() else []
    if len(places) < 2:
        return None
    size = oc.load_catalogue()[name][1]
    ours = exe_bytes(int(name[5:], 16), size)
    text = oc.text_record(oc.level_manifest(level))
    blob = (oc.DUMP / f"level_{level:02d}/text.bin").read_bytes()
    same = []
    for at in places:
        theirs = blob[at - text["address"]:at - text["address"] + size]
        if len(theirs) == len(ours) and all(
                a == b or (a >> 26 == b >> 26 and a >> 26 in (2, 3))
                for a, b in ((int.from_bytes(ours[i:i + 4], "little"), int.from_bytes(theirs[i:i + 4], "little"))
                             for i in range(0, size - size % 4, 4))):
            same.append(at)
    return min(same, key=lambda a: abs(a - near)) if same else None


def inside(oc, y: int, level: int):
    """An executable function the catalogue has no entry for (it took it as part of the one
    before it, func_00218930 after func_00218928): the place of the nearest catalogued
    executable function before it, plus the distance, when the level's code there is the same
    instructions (func_0020EA70 is level 0's func_L00_00251B58)."""
    best = None
    for n, (kind, _size, places) in oc.load_catalogue().items():
        m = re.fullmatch(r"func_([0-9A-Fa-f]{8})", n)
        if kind == "exe" and m and 0 < y - int(m.group(1), 16) <= 0x400:
            x = int(m.group(1), 16)
            if best is None or x > best[0]:
                best = (x, [a for lv, a in places if lv == level])
    if not best or not best[1]:
        return None
    if len(best[1]) == 1:
        at = best[1][0] + (y - best[0])
    else:
        # A folded function: the place that is its own code (func_001F98B0 follows
        # func_001F9878's own copy, not the copies of its neighbours folded into it).
        own = exe_place(oc, f"func_{best[0]:08X}", level, y)
        if own is None:
            return None
        at = own + (y - best[0])
    text = oc.text_record(oc.level_manifest(level))
    blob = (oc.DUMP / f"level_{level:02d}/text.bin").read_bytes()
    # The same instructions (by opcode): the level's copy reaches the level's data, so its
    # addresses differ from the executable's.
    ours, theirs = exe_bytes(y, 32), blob[at - text["address"]:at - text["address"] + 32]
    for i in range(0, 32, 4):
        a, b = int.from_bytes(ours[i:i + 4], "little"), int.from_bytes(theirs[i:i + 4], "little")
        if a >> 26 != b >> 26:
            return None
    return at


def scratch_source(name: str):
    """find_stub for a function no source file of rac1-decomp has (a libgcc routine built from
    its own object, a second entry inside another function's assembly): a file of its own with
    the decompilation's common header, the candidate going where its stub would be."""
    src = DECOMP / "build-sn/fcheck/gate/scratch" / f"{name}.c"
    src.parent.mkdir(parents=True, exist_ok=True)
    src.write_text('#include "common.h"\n/* the candidate */\n')
    return "core", src.relative_to(DECOMP), 1, 1


def main() -> None:
    if len(sys.argv) != 5:
        sys.exit(__doc__)
    name, cand, out, level = sys.argv[1], Path(sys.argv[2]), Path(sys.argv[3]), sys.argv[4]
    fcheck = load_fcheck()
    import try_func

    find_stub = try_func.find_stub

    def find(n):
        try:
            seg, src, first, last = find_stub(n)
        except SystemExit:
            return scratch_source(n)
        # The "text" pipeline's steps after the compiler serve the byte match (the retail
        # assembler's nops and its jump-table sections, which fcheck does not place, and it
        # refuses read-only data that is not a jump table); for a behaviour check the
        # compiler's own output is the candidate, so it goes the way other segments go.
        return ("core" if seg == "text" else seg), src, first, last

    try_func.find_stub = find
    if level != "exe":
        try_func.symbol_address = resolver(name, int(level), try_func.symbol_address)
    fcheck.build(name, cand, out)


if __name__ == "__main__":
    main()
