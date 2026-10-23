"""Going Commando, Up Your Arsenal and Deadlocked discs: the table of contents at sector 1001,
the level table, the level headers and a level's sections.

Synthetic images only: a made-up version of each game, with an executable of a few bytes and
levels whose core is empty, so no game data is needed.
"""

import hashlib
from pathlib import Path
import struct
import tempfile
import unittest
from unittest import mock

from disc import SECTOR, Disc
from formats import FormatError
from level import load_wad_level
import wads
from test_disc import cnf, record

EXE = b"\x7fELF later"
GC = wads.Version("rac2", "ntsc", "NTSC-U", "test", "SCUS_000.03", len(EXE), hashlib.sha1(EXE).hexdigest(), 0x60)
DL = wads.Version("rac4", "ntsc", "NTSC-U", "test", "SCUS_000.03", len(EXE), hashlib.sha1(EXE).hexdigest(), 0xc68)
TOC = wads.TOC_LBA
FILES = 1100           # the level files' first sector
CORE = bytes(range(64))


def core_index() -> bytes:
    """A core index with every table empty and the core data's size at +0x8c."""
    index = bytearray(0xc0)
    struct.pack_into("<i", index, 0x8c, len(CORE))
    struct.pack_into("<i", index, 0xac, 0xb8)   # the stash list: ends at once
    struct.pack_into("<h", index, 0xb8, -1)
    return bytes(index)


def level_data(dl: bool, gameplay: bytes, art: bytes) -> bytes:
    """A level data WAD: its header of (offset, size) pairs, then the sections, uncompressed."""
    names = wads.DATA_SECTIONS[0xc68 if dl else 0x60]
    sections = {"core_index": core_index(), "gs_ram": bytes(256), "core_data": CORE}
    if dl:
        sections.update(gameplay_core=gameplay, art_instances=art)
    data = bytearray(0x100)
    for i, name in enumerate(names):
        if name in sections:
            struct.pack_into("<ii", data, i * 8, len(data), len(sections[name]))
            data += sections[name] + bytes(-len(sections[name]) % 64)
        else:
            struct.pack_into("<ii", data, i * 8, -1, 0)
    return bytes(data)


def image(version: wads.Version, levels: list[tuple[int, int]], swap: bool = False) -> bytes:
    """An image whose table of contents has one global header and the level table, each level
    a level header (and an audio header) in the table, its file from FILES + 10 * slot.
    LEVELS: (slot, id) pairs. SWAP: the pairs in Up Your Arsenal's order (audio first)."""
    dl = version.level_header == 0xc68
    out = bytearray((FILES + 10 * (max(s for s, _ in levels) + 1)) * SECTOR)
    out[16 * SECTOR:16 * SECTOR + 7] = b"\x01CD001\x01"
    struct.pack_into("<H", out, 16 * SECTOR + 128, SECTOR)
    struct.pack_into("<II", out, 16 * SECTOR + 158, 18, 0)
    struct.pack_into("<I", out, 16 * SECTOR + 166, SECTOR)
    directory = record(b"\0", 18, SECTOR) + record(b"\1", 18, SECTOR)
    directory += record(b"SYSTEM.CNF;1", 19, len(cnf(version.executable)))
    directory += record(version.executable.encode() + b";1", 20, len(EXE))
    folder = bytearray(record(b"G", 18, SECTOR))
    folder[25] = 2  # a directory: listed, never entered
    directory += bytes(folder)
    out[18 * SECTOR:18 * SECTOR + len(directory)] = directory
    out[19 * SECTOR:19 * SECTOR + len(cnf(version.executable))] = cnf(version.executable)
    out[20 * SECTOR:20 * SECTOR + len(EXE)] = EXE
    toc = TOC * SECTOR
    struct.pack_into("<ii8x", out, toc, 0x10, 30)         # one global header of 0x10 bytes
    table = toc + 0x10
    for n, (slot, number) in enumerate(levels):
        header_lba, audio_lba, file_lba = TOC + 2 + 2 * n, TOC + 3 + 2 * n, FILES + 10 * slot
        pairs = [(header_lba, 10), (audio_lba, 1), (0, 0)]
        if swap:
            pairs[0], pairs[1] = pairs[1], pairs[0]
        struct.pack_into("<6i", out, table + slot * 24, *(v for pair in pairs for v in pair))
        header = bytearray(version.level_header)
        struct.pack_into("<iii", header, 0, version.level_header, file_lba, number)
        gameplay, art = bytes([number]) * 32, bytes([0xa0 + number]) * 16
        data = level_data(dl, gameplay, art)
        sectors = -(-len(data) // SECTOR)
        if dl:
            struct.pack_into("<ii", header, 0x18, 1, sectors)
        else:
            struct.pack_into("<ii", header, 0x10, 1, sectors)
            struct.pack_into("<ii", header, 0x20, 1 + sectors, 1)
            out[(file_lba + 1 + sectors) * SECTOR:(file_lba + 1 + sectors) * SECTOR + 32] = gameplay
        out[(file_lba + 1) * SECTOR:(file_lba + 1) * SECTOR + len(data)] = data
        out[header_lba * SECTOR:header_lba * SECTOR + len(header)] = header
        struct.pack_into("<ii", out, audio_lba * SECTOR, 0x1018, file_lba)
    return bytes(out)


class WadDiscTests(unittest.TestCase):
    def open(self, version: wads.Version, levels, swap=False) -> Disc:
        temp = tempfile.TemporaryDirectory()
        self.addCleanup(temp.cleanup)
        path = Path(temp.name) / "later.iso"
        path.write_bytes(image(version, levels, swap))
        patch = mock.patch.object(wads, "VERSIONS", (version,))
        patch.start()
        self.addCleanup(patch.stop)
        opened = Disc(path)
        self.addCleanup(opened.file.close)
        return opened

    def test_identify_by_system_cnf(self):
        d = self.open(GC, [(0, 0)])
        self.assertEqual(wads.identify(d), GC)
        self.assertIn(("G/", 18, SECTOR), wads.walk(d))
        with mock.patch.object(wads, "VERSIONS", ()):
            self.assertIsNone(wads.identify(d))  # Another game's disc: disc.Disc reads it.
        wrong = wads.Version("rac2", "ntsc", "NTSC-U", "test", GC.executable, len(EXE), "0" * 40, 0x60)
        with mock.patch.object(wads, "VERSIONS", (wrong,)), self.assertRaisesRegex(FormatError, "SHA-1"):
            wads.identify(d)

    def test_walk_wants_iso_9660_version_1_with_2048_byte_blocks(self):
        for fmt, offset, value in (("<H", 16 * SECTOR + 128, 4096), ("B", 16 * SECTOR + 6, 2)):
            data = bytearray(image(GC, [(0, 0)]))
            struct.pack_into(fmt, data, offset, value)
            temp = tempfile.TemporaryDirectory()
            self.addCleanup(temp.cleanup)
            path = Path(temp.name) / "damaged.iso"
            path.write_bytes(bytes(data))
            with self.subTest(offset=offset), Disc(path) as d, self.assertRaisesRegex(FormatError, "2048-byte sectors"):
                wads.walk(d)

    def test_levels_are_numbered_by_their_headers(self):
        d = self.open(GC, [(0, 0), (1, 30)])
        source = wads.WadDisc(d, GC)
        self.assertEqual(source.level_table(), 0x10)
        levels = source.levels()
        self.assertEqual([(lv["id"], lv["slot"]) for lv in levels], [(0, 0), (30, 1)])
        self.assertEqual(levels[1]["level"]["lba"], FILES + 10)
        self.assertIn("audio", levels[1])
        self.assertEqual(source.level(30)["slot"], 1)
        with self.assertRaisesRegex(FormatError, "no level 2"):
            source.level(2)
        survey = source.survey()
        self.assertEqual((survey["game"], survey["version"]), ("rac2", "ntsc"))
        self.assertEqual(survey["levels"][0]["ranges"]["gameplay"]["sectors"], 1)

    def test_the_pairs_are_told_apart_by_their_headers(self):
        d = self.open(GC, [(0, 1)], swap=True)
        level, = wads.WadDisc(d, GC).levels()
        self.assertEqual((level["id"], level["level"]["header_lba"], level["audio"]["header_lba"]),
                         (1, TOC + 2, TOC + 3))

    def test_level_headers_are_checked(self):
        with self.assertRaisesRegex(FormatError, "0xc68 bytes, not 0x60"):
            wads.level_ranges(bytes(0x60), DL)
        with self.assertRaisesRegex(FormatError, "without level data"):
            wads.level_ranges(bytes(0x60), GC)
        d = self.open(GC, [(0, 0)])
        source = wads.WadDisc(d, GC)
        broken = bytearray(source.toc)
        struct.pack_into("<i", broken, (TOC + 2 - TOC) * SECTOR, 0x123)  # not a size of a known header
        source.toc = bytes(broken)
        with self.assertRaises(FormatError):
            source.levels()

    def test_a_level_loads_into_the_shared_level(self):
        d = self.open(GC, [(0, 4)])
        source = wads.WadDisc(d, GC)
        level = load_wad_level(source, source.level(4))
        self.assertEqual((level.id, level.game, level.version), (4, "rac2", "ntsc"))
        self.assertEqual((level.core, level.gameplay[:32], level.art), (CORE, bytes([4]) * 32, None))
        self.assertIs(level.instances, level.gameplay)
        self.assertEqual((level.textures, level.boundaries, level.problems), ({}, [len(CORE)], []))

    def test_deadlocked_keeps_its_instances_in_the_level_data(self):
        d = self.open(DL, [(0, 0), (2, 2)])
        source = wads.WadDisc(d, DL)
        level = load_wad_level(source, source.level(2))
        self.assertEqual((level.game, level.gameplay, level.art), ("rac4", bytes([2]) * 32, bytes([0xa2]) * 16))
        self.assertIs(level.instances, level.art)


if __name__ == "__main__":
    unittest.main()
