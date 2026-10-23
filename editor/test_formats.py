"""Synthetic fixtures only, never disc data: python3 -m unittest discover -s editor"""

from pathlib import Path
import struct
import tempfile
import unittest
import zlib

from disc import Disc
from formats import FormatError, Texture, check, overlay_sections, span, wad


def compressed(payload: bytes) -> bytes:
    return b"WAD" + struct.pack("<I", 16 + len(payload)) + bytes(9) + payload


def png_chunks(data: bytes) -> dict:
    chunks, pos = {}, 8
    while pos < len(data):
        size, = struct.unpack_from(">I", data, pos)
        kind, payload = data[pos + 4:pos + 8], data[pos + 8:pos + 8 + size]
        assert struct.unpack_from(">I", data, pos + 8 + size)[0] == zlib.crc32(kind + payload)
        chunks[kind] = payload
        pos += 12 + size
    return chunks


class CheckTests(unittest.TestCase):
    def test_raises_its_message_unless_the_rule_holds(self):
        check(True, "unused")
        with self.assertRaises(FormatError) as caught:
            check(False, "collision header is not the usual one")
        self.assertEqual(str(caught.exception), "collision header is not the usual one")


class WadTests(unittest.TestCase):
    def test_empty_and_initial_literal(self):
        self.assertEqual(wad(compressed(b"")), b"")
        self.assertEqual(wad(compressed(b"\x15abcd")), b"abcd")

    def test_short_overlap_and_trailing_literal(self):
        self.assertEqual(wad(compressed(b"\x01abcd\x41\0!")), b"abcdddd!")

    def test_medium_and_extended_matches(self):
        self.assertEqual(wad(compressed(b"\x01abcd\x23\x0c\0")), b"abcdabcda")
        self.assertEqual(wad(compressed(b"\x01abcd\x20\x02\0\0")), b"abcd" + b"d" * 35)

    def test_far_match(self):
        # History from literal runs separated by no-op matches, then a far copy.
        payload = (b"\0\xff" + b"x" * 273 + b"\x11\0\0") * 61
        self.assertEqual(wad(compressed(payload + b"\x11\x04\0")), b"x" * (273 * 61 + 3))

    def test_block_skip_at_8192_bytes(self):
        payload = b"\x01abcd\x12\0\0"
        payload += bytes(8192 - len(payload)) + b"\x01efgh"
        self.assertEqual(wad(compressed(payload)), b"abcdefgh")

    def test_invalid_streams(self):
        for data in (b"WAD", compressed(b"\x01abc"), compressed(b"\x40\0"),
                     compressed(b"\x01abcd\x01efgh"), compressed(b"\x12\0\0")):
            with self.subTest(data=data[:20]), self.assertRaises(FormatError):
                wad(data)
        with self.assertRaises(FormatError):
            wad(compressed(b"\x01abcd"), limit=3)


class TextureTests(unittest.TestCase):
    def test_palette_swizzle_alpha_and_png(self):
        palette = bytearray(1024)
        palette[16 * 4:16 * 4 + 4] = bytes((10, 20, 30, 0x80))  # Read as index 8 (CSM1).
        texture = Texture(2, 1, b"\x08\0", bytes(palette))
        chunks = png_chunks(texture.png())
        self.assertEqual(chunks[b"PLTE"][24:27], bytes((10, 20, 30)))
        self.assertEqual(chunks[b"tRNS"][8], 255)
        self.assertEqual(zlib.decompress(chunks[b"IDAT"]), b"\0\x08\0")
        self.assertTrue(texture.cutout)  # Index 0 is fully transparent and in use.

    def test_cutout_only_counts_pixels_in_use(self):
        palette = bytes((255, 0, 0, 0x80)) * 255 + bytes(4)
        self.assertFalse(Texture(1, 1, b"\0", palette).cutout)
        self.assertTrue(Texture(1, 1, b"\xff", palette).cutout)

    def test_bad_sizes(self):
        for args in ((0, 1, b"", bytes(1024)), (1, 1, b"", bytes(1024)), (1, 1, b"\0", bytes(4))):
            with self.subTest(args=args[:2]), self.assertRaises(FormatError):
                Texture(*args)


class ContainerTests(unittest.TestCase):
    def test_overlay_records_and_truncation(self):
        data = struct.pack("<4I", 0x100000, 4, 1, 0x100000) + bytes(4)
        self.assertEqual(overlay_sections(data)["sections"][0]["bytes"], 4)
        for bad in (data[:-1], struct.pack("<4I", 0x100000, 4, 1, 0x200000) + bytes(4)):
            with self.assertRaises(FormatError):
                overlay_sections(bad)

    def test_span_bounds(self):
        for offset, size in ((-1, 1), (0, -1), (3, 2)):
            with self.assertRaises(FormatError):
                span(b"abcd", offset, size)

    def test_iso_directory_and_bounds(self):
        data = bytearray(20 * 2048)
        data[16 * 2048:16 * 2048 + 7] = b"\x01CD001\x01"
        struct.pack_into("<H", data, 16 * 2048 + 128, 2048)
        struct.pack_into("<I", data, 16 * 2048 + 158, 18)
        struct.pack_into("<I", data, 16 * 2048 + 166, 2048)
        record = bytearray(44)
        record[0], record[32] = 44, 10
        record[33:43] = b"TEST.TXT;1"
        struct.pack_into("<I", record, 2, 19)
        struct.pack_into("<I", record, 10, 4)
        data[18 * 2048:18 * 2048 + len(record)] = record
        with tempfile.TemporaryDirectory() as temp:
            path = Path(temp) / "synthetic.iso"
            path.write_bytes(data)
            with Disc(path) as disc:
                self.assertEqual(disc.files(), [{"name": "TEST.TXT", "lba": 19, "bytes": 4}])
                with self.assertRaises(FormatError):
                    disc.read(len(data) - 2, 4)
                with self.assertRaisesRegex(FormatError, "PAL v2.00"):
                    disc.survey()


if __name__ == "__main__":
    unittest.main()
