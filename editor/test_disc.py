"""Telling the two discs apart, and reading the table of contents where a version keeps it.

Synthetic images only: the versions under test are made up here, with executables of a few
bytes, so no game data is needed.
"""

import hashlib
from pathlib import Path
import struct
import tempfile
import unittest
from unittest import mock

import disc
from disc import LEVEL_COUNT, LEVEL_HEADER_SIZE, NTSC, PAL, SECTOR, VERSIONS, Disc, Version
from formats import FormatError

GROUPS = ((0x08, 2, "pictures", "sectors"), (0x18, 2, "audio", "lba"))
LEVEL_TABLE = 0x20
TOC_SIZE = LEVEL_TABLE + LEVEL_COUNT * 8
TOC_LBA = 23
HEADERS = 24                                  # the first level header's sector
HEADER_SECTORS = (LEVEL_HEADER_SIZE + SECTOR - 1) // SECTOR


def record(name: bytes, lba: int, size: int) -> bytes:
    """An ISO 9660 directory record."""
    length = 33 + len(name) + (len(name) + 1) % 2
    body = struct.pack("<BBIIIIxxxxxxxBBBHHB", length, 0, lba, 0, size, 0, 0, 0, 0, 1, 1, len(name)) + name
    return body + bytes(length - len(body))


def image(files: dict[str, bytes], toc: bytes = b"", headers: list[bytes] = ()) -> bytes:
    """The volume descriptor, a root directory at sector 18, FILES from sector 19 (one sector
    each), the table of contents at TOC_LBA and the level headers from HEADERS."""
    out = bytearray((HEADERS + LEVEL_COUNT * HEADER_SECTORS) * SECTOR)
    out[16 * SECTOR:16 * SECTOR + 7] = b"\x01CD001\x01"
    struct.pack_into("<H", out, 16 * SECTOR + 128, SECTOR)
    struct.pack_into("<I", out, 16 * SECTOR + 158, 18)           # the root directory's sector
    struct.pack_into("<I", out, 16 * SECTOR + 166, SECTOR)       # and size
    directory = record(b"\0", 18, SECTOR) + record(b"\1", 18, SECTOR)
    for i, (name, data) in enumerate(files.items()):
        assert len(data) <= SECTOR and 19 + i < TOC_LBA
        directory += record(name.encode() + b";1", 19 + i, len(data))
        out[(19 + i) * SECTOR:(19 + i) * SECTOR + len(data)] = data
    out[18 * SECTOR:18 * SECTOR + len(directory)] = directory
    out[TOC_LBA * SECTOR:TOC_LBA * SECTOR + len(toc)] = toc
    for i, header in enumerate(headers):
        at = (HEADERS + i * HEADER_SECTORS) * SECTOR
        out[at:at + len(header)] = header
    return bytes(out)


def toc_and_headers() -> tuple[bytes, list[bytes]]:
    """A table of contents with one picture, one audio sector and 19 levels, each level's
    gameplay copies at sectors of their own (NTSC 10 + level, PAL 20 + level)."""
    toc = bytearray(TOC_SIZE)
    struct.pack_into("<II", toc, 0, 1, TOC_SIZE)
    struct.pack_into("<II", toc, 0x08, 19, 1)          # pictures 0: sector 19, one sector
    struct.pack_into("<I", toc, 0x1c, 20)              # audio 1: sector 20
    headers = []
    for level in range(LEVEL_COUNT):
        lba = HEADERS + level * HEADER_SECTORS
        struct.pack_into("<II", toc, LEVEL_TABLE + level * 8, lba, HEADER_SECTORS)
        header = bytearray(LEVEL_HEADER_SIZE)
        struct.pack_into("<II", header, 0, level, LEVEL_HEADER_SIZE)
        struct.pack_into("<8I", header, 8, lba, 1, 10 + level, 1, 20 + level, 1, lba, 0)
        headers.append(bytes(header))
    return bytes(toc), headers


def version(key: str, executable: str, data: bytes, gameplay: str, **kw) -> Version:
    fields = dict(key=key, region=key.upper(), label=f"{key} test", executable=executable,
                  executable_size=len(data), executable_sha1=hashlib.sha1(data).hexdigest(), gameplay=gameplay,
                  normal_table=0x1000, toc_lba=TOC_LBA, toc_size=TOC_SIZE, level_table=LEVEL_TABLE, groups=GROUPS)
    fields.update(kw)
    return Version(**fields)


EXE_A, EXE_B = b"\x7fELF first", b"\x7fELF second one"
A = version("a", "SCES_000.01", EXE_A, "gameplay_pal")
B = version("b", "SCUS_000.02", EXE_B, "gameplay_ntsc")


def cnf(name: str) -> bytes:
    return f"BOOT2 = cdrom0:\\{name};1\r\nVER = 1.00\r\nVMODE = NTSC\r\n".encode()


class VersionTests(unittest.TestCase):
    def open(self, files: dict[str, bytes], versions=(A, B)) -> Disc:
        toc, headers = toc_and_headers()
        temp = tempfile.TemporaryDirectory()
        self.addCleanup(temp.cleanup)
        path = Path(temp.name) / "synthetic.iso"
        path.write_bytes(image(files, toc, headers))
        patch = mock.patch.object(disc, "VERSIONS", versions)
        patch.start()
        self.addCleanup(patch.stop)
        opened = Disc(path)
        self.addCleanup(opened.file.close)
        return opened

    def test_system_cnf_names_the_version(self):
        # Both executables are on it; the one SYSTEM.CNF boots decides.
        d = self.open({"SYSTEM.CNF": cnf(B.executable), A.executable: EXE_A, B.executable: EXE_B})
        self.assertEqual(d.version, B)
        self.assertEqual(d.executable(), EXE_B)
        survey = d.survey()
        self.assertEqual((survey["version"], survey["executable_sha1"]), ("b", B.executable_sha1))
        self.assertEqual([(r["group"], r["index"], r["lba"], r["bytes"]) for r in survey["global_references"]],
                         [("pictures", 0, 19, SECTOR), ("audio", 1, 20, None)])
        self.assertEqual(len(survey["levels"]), LEVEL_COUNT)
        ranges = survey["levels"][5]["ranges"]
        self.assertEqual((ranges["gameplay_ntsc"]["lba"], ranges["gameplay_pal"]["lba"]), (15, 25))
        self.assertEqual(ranges[d.version.gameplay]["lba"], 15)

    def test_without_system_cnf_the_executable_on_the_disc_decides(self):
        d = self.open({A.executable: EXE_A})
        self.assertEqual(d.version, A)
        self.assertEqual(d.survey()["version"], "a")

    def test_unknown_discs_are_refused(self):
        cases = ({"SYSTEM.CNF": cnf("SLUS_999.99"), A.executable: EXE_A},      # boots something else
                 {"OTHER.ELF": EXE_A},                                         # no known executable
                 {"SYSTEM.CNF": cnf(A.executable), A.executable: EXE_A + b"!"})  # another size
        for files in cases:
            with self.subTest(files=list(files)), self.assertRaisesRegex(FormatError, "SCES_000.01 \\(a test\\) and"):
                self.open(files).survey()

    def test_the_executable_must_be_that_version_s(self):
        d = self.open({"SYSTEM.CNF": cnf(A.executable), A.executable: b"\x7fELF fiRst"})
        with self.assertRaisesRegex(FormatError, "SHA-1 does not match a test"):
            d.survey()

    def test_the_table_of_contents_is_checked(self):
        d = self.open({A.executable: EXE_A}, (version("a", A.executable, EXE_A, "gameplay_pal", toc_size=TOC_SIZE + 8),))
        with self.assertRaisesRegex(FormatError, "table of contents"):
            d.survey()


class KnownVersionTests(unittest.TestCase):
    def test_the_two_discs(self):
        self.assertEqual(VERSIONS, (PAL, NTSC))
        self.assertEqual([(v.key, v.executable, v.gameplay) for v in VERSIONS],
                         [("pal", "SCES_509.16", "gameplay_pal"), ("ntsc", "SCUS_971.99", "gameplay_ntsc")])
        # games/rac1/game.json holds the same executables' sizes and SHA-1s.
        self.assertEqual((PAL.executable_size, PAL.executable_sha1), (1388100, "79956931bd62fafd8d20fa2eae796dbaf2e15e83"))
        self.assertEqual((NTSC.executable_size, NTSC.executable_sha1), (1383028, "72fd1de379dafd8f243dceeb3be50980f3fd3cd5"))
        self.assertEqual((PAL.normal_table, NTSC.normal_table), (0x165600, 0x165500))
        for v in VERSIONS:
            self.assertEqual((v.toc_lba, v.toc_size, v.level_table), (1500, 0x2960, 0x28c8))
            self.assertEqual(v.level_table + LEVEL_COUNT * 8, v.toc_size)
            self.assertIn(v.gameplay, disc.LEVEL_RANGES)

    def test_game_json_agrees(self):
        import json
        catalogue = json.loads((Path(__file__).resolve().parents[1] / "games/rac1/game.json").read_text())
        text = json.dumps(catalogue)
        for v in VERSIONS:
            self.assertIn(v.executable_sha1, text)
            self.assertIn(f'"serial": "{v.executable}"', text)


if __name__ == "__main__":
    unittest.main()
