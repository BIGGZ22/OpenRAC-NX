# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""The prepared memory card for the routes that load a game (docs/port/REGRESSION.md).

A copy of a card (the player's by default, only read), with its five slots rewritten from the
first saved game on it: every weapon and gadget owned with full ammo, Ultra Nanotech, the save
notice seen (SAVE_NOTICE_SEEN), every planet the ship reaches unlocked, the bolt counter full,
and each slot resuming on another planet (SLOT_PLANETS). The format and what each chunk means
are the launcher's save editor's (launcher/core/src/save_edit.rs, which documents them); this is
the same encoding in Python, so the harness needs no Rust build. The copy lives in build/regress/cards/<game>/card, and each
run plays on a fresh copy of it (runner.py), never on the player's card.
"""

from __future__ import annotations

import json
import os
import shutil
import struct
from pathlib import Path

import runner

SERIALS = {"rac1-pal": "SCES_509.16", "rac1-ntsc": "SCUS_971.99"}

# The planet each slot resumes on: only the ship's destinations (1 to 16), as the save editor
# allows (Veldin and Drek's Fleet are reached by the story). Rilgar (the hoverboard race),
# Orxon (Clank's part), Gemlik Base (the boss), Hoven, Kalebo III (the hoverboard and grind rails).
SLOT_PLANETS = (5, 10, 13, 12, 16)

# Global chunk ids (save_edit.rs, mod id).
PLANET, BOLTS, FLAGS, AMMO, OWNED, ACQUIRED, VENDOR, QUICK, PLANETS, MAX_HEALTH, MAP_ORDER = (
    0, 1, 5, 9, 10, 11, 12, 13, 14, 19, 20)

# Items (save_edit.rs ITEMS): id -> the item record's ammo limit (0 for none).
ITEMS = {2: 0, 3: 0, 4: 0, 5: 0, 6: 0, 7: 0, 9: 0, 10: 40, 11: 20, 12: 0, 13: 20, 14: 0, 15: 200, 16: 240,
         17: 50, 18: 0, 19: 240, 20: 10, 21: 0, 22: 0, 23: 50, 24: 10, 25: 20, 26: 0, 27: 0, 28: 0, 29: 0,
         30: 0, 31: 0, 32: 0, 33: 0, 34: 0, 35: 0}
VENDOR_ITEMS = (10, 11, 13, 14, 15, 16, 17, 18, 19, 20, 24, 25)
QUICK_SELECT = (15, 10, 16, 19, 11, 17, 25, 23)   # Blaster, Bomb Glove, Pyrocitor, Tesla, ...
# Story flags (chunk 5, gSpecialItems: PAL 0x13D490, US 0x13D388): [4] and [5] the nanotech, and
# [0x10] the save notice seen. The level's frame update (PAL LevelFrameUpdate, func_L00_00299250;
# US FUN_L00_00297f78) shows that notice once, on any level but 0, while the flag is clear and the
# word before the flags (PAL 0x13D48C, US 0x13D384) is set, as it is after Load Game: the freeze
# screen's kind 5 (help text 0x4E2B, game mode 4) holds the game until Cross, and the flag is set.
# From a base save made before that notice (a first save on Veldin), every loaded route would wait
# there, before control.
PREMIUM, ULTRA, SAVE_NOTICE_SEEN = 4, 5, 0x10
SHIP_PLANETS = range(1, 17)
MAX_BOLTS = 9_999_999


def crc16(data: bytes) -> int:
    """memcard_Checksum (save_edit.rs crc16): 0 above 0x1800 bytes."""
    if len(data) > 0x1800:
        return 0
    crc = 0xEDB88320
    for b in data:
        crc ^= b << 8
        for _ in range(8):
            crc = ((crc << 1) ^ 0x1F45) if crc & 0x8000 else (crc << 1)
            crc &= 0xFFFFFFFF
    return crc & 0xFFFF


def decode_section(b: bytes) -> list[list]:
    """[id, data bytearray, pad] per chunk; the checksum must hold, as the game's load checks."""
    size, stored = struct.unpack_from("<II", b, 0)
    data = b[8:8 + size]
    if stored == 0 or crc16(data) != stored:
        raise ValueError("a section's checksum does not match")
    out, at = [], 0
    while True:
        cid, n = struct.unpack_from("<iI", data, at)
        if cid == -1:
            return out
        padded = (n + 3) & ~3
        out.append([cid, bytearray(data[at + 8:at + 8 + n]), bytes(data[at + 8 + n:at + 8 + padded])])
        at += 8 + padded


def encode_section(chunks: list[list]) -> bytes:
    def raw(cs):
        v = bytearray(8)
        for cid, d, pad in cs:
            v += struct.pack("<iI", cid, len(d)) + d
            v += bytes(pad[k] if k < len(pad) else 0 for k in range(((len(d) + 3) & ~3) - len(d)))
        v += struct.pack("<iI", -1, 0)
        struct.pack_into("<II", v, 0, len(v) - 8, crc16(bytes(v[8:])))
        return bytes(v)
    out = raw(chunks)
    if struct.unpack_from("<I", out, 4)[0] == 0:  # a checksum of 0 fails: change a padding byte
        for c in chunks:
            if c[2]:
                for x in range(1, 256):
                    c[2] = bytes([x]) + c[2][1:]
                    out = raw(chunks)
                    if struct.unpack_from("<I", out, 4)[0] != 0:
                        return out
    return out


def parse(b: bytes) -> tuple[list[list], list[list[list]], int]:
    gs, ls = struct.unpack_from("<II", b, 0)
    if len(b) != 8 + gs + 20 * ls:
        raise ValueError("not a save of the size the game writes")
    glob = decode_section(b[8:8 + gs])
    levels = [decode_section(b[8 + gs + i * ls:8 + gs + (i + 1) * ls]) for i in range(20)]
    return glob, levels, ls


def to_bytes(glob: list[list], levels: list[list[list]], ls: int) -> bytes:
    g = encode_section(glob)
    return struct.pack("<II", len(g), ls) + g + b"".join(encode_section(lv) for lv in levels)


def chunk(section: list[list], cid: int) -> bytearray:
    return next(c[1] for c in section if c[0] == cid)


def set_i32(b: bytearray, i: int, v: int) -> None:
    struct.pack_into("<i", b, 4 * i, v)


def prepared(save: bytes, planet: int) -> bytes:
    """save with everything the routes use, resuming on planet."""
    glob, levels, ls = parse(save)
    if struct.unpack("<i", chunk(glob, PLANET))[0] < 0:
        raise ValueError("an empty slot")
    set_i32(chunk(glob, PLANET), 0, planet)
    set_i32(chunk(glob, BOLTS), 0, MAX_BOLTS)
    flags = chunk(glob, FLAGS)
    flags[PREMIUM] = flags[ULTRA] = 1  # Premium and Ultra Nanotech
    flags[SAVE_NOTICE_SEEN] = 1      # or the notice waits for Cross before control
    set_i32(chunk(glob, MAX_HEALTH), 0, 8)
    owned, acquired, ammo, stock = chunk(glob, OWNED), chunk(glob, ACQUIRED), chunk(glob, AMMO), chunk(glob, VENDOR)
    for item, limit in ITEMS.items():
        owned[item] = acquired[item] = 1
        if limit:
            set_i32(ammo, item, limit)
        if item in VENDOR_ITEMS:
            at = next((i for i, b in enumerate(stock) if b != 0xFF and (b & 0x3F) == item), None)
            if at is None:
                at = next((i for i, b in enumerate(stock) if b == 0xFF), None)
                if at is not None:
                    stock[at] = item
            if at is not None:
                stock[at] |= 0x40
    quick = chunk(glob, QUICK)
    for i, item in enumerate(QUICK_SELECT):
        set_i32(quick, i, item)
    unlocked, order = chunk(glob, PLANETS), chunk(glob, MAP_ORDER)
    for p in list(SHIP_PLANETS):
        if not unlocked[p]:
            n = sum(1 for u in unlocked if u)
            unlocked[p] = 1
            if n < len(order) // 4:
                set_i32(order, n, p)
    return to_bytes(glob, levels, ls)


def player_card(game: str) -> Path:
    home = Path(os.environ.get("HOME", "."))
    base = (home / "Library/Application Support/OpenRAC" if os.uname().sysname == "Darwin"
            else Path(os.environ.get("XDG_DATA_HOME", home / ".local/share")) / "openrac")
    return base / "memcard" / SERIALS[game]


def game_folder(card: Path) -> Path:
    found = sorted(p for p in card.iterdir() if p.is_dir() and not p.name.startswith("."))
    if not found:
        raise SystemExit(f"{card}: no game folder on this card")
    return found[0]


def prepare(game: str, source: Path | None = None) -> Path:
    """Copies source (the player's card by default; it is only read) to build/regress/cards/<game>
    and rewrites the copy's five slots from its first saved game."""
    source = source or player_card(game)
    if not source.is_dir():
        raise SystemExit(f"no card at {source}: play once and save, or pass --from")
    folder = game_folder(source)
    saves = sorted(folder.glob("save[0-9].bin"))
    base = None
    for s in saves:
        try:
            glob, _, _ = parse(s.read_bytes())
            if struct.unpack("<i", chunk(glob, PLANET))[0] >= 0:
                base = s.read_bytes()
                break
        except (ValueError, StopIteration):
            continue
    if base is None:
        raise SystemExit(f"{folder}: no saved game to start from")
    dest = runner.cards_dir() / game
    if (dest / "card").exists():
        shutil.rmtree(dest / "card")  # this tool's own earlier copy
    dest.mkdir(parents=True, exist_ok=True)
    shutil.copytree(folder, dest / "card" / folder.name,
                    ignore=shutil.ignore_patterns(".*"))  # the game's folder only, no backups
    for slot, planet in enumerate(SLOT_PLANETS):
        (dest / "card" / folder.name / f"save{slot}.bin").write_bytes(prepared(base, planet))
    (dest / "card.json").write_text(json.dumps({
        "game": game, "from": str(source), "slots": {str(i): p for i, p in enumerate(SLOT_PLANETS)},
        "items": sorted(ITEMS), "quick_select": list(QUICK_SELECT)}, indent=1) + "\n")
    return dest / "card"
