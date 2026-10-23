"""spaceships.py: the player's ships from synthetic spaceships files; no disc data. The class
decoder (moby_class.moby_class, tested in test_moby_class.py) is replaced by a recorder, so these
check what the module itself does: which entries it reads, how it cuts a file into its two classes
and two pictures, and the texture keys it gives them."""

from pathlib import Path
import struct
import sys
from types import SimpleNamespace
import unittest
from unittest import mock

sys.path.insert(0, str(Path(__file__).resolve().parent))

from disc import SECTOR  # noqa: E402
from formats import FormatError  # noqa: E402
import spaceships  # noqa: E402
from spaceships import EXTRA_CLASSES, SHIP_CLASSES, TEXTURE_BASE, add_spaceships, spaceship_texture  # noqa: E402


def picture(size: int, seed: int, *, count: int = 1, magic: bytes = b"2FIP", fmt: int = 0x13) -> bytes:
    """A texture list holding one PIF: header, CLUT at +0x30, pixels at +0x430."""
    head = struct.pack("<II", count, 0x10) + bytes(8) + magic + bytes(4) + struct.pack("<III", size, size, fmt)
    head += bytes(0x30 - len(head))
    palette = bytes((seed + i) % 256 for i in range(1024))
    pixels = bytes((seed * 3 + i) % 256 for i in range(size * size))
    return head + palette + pixels


def spaceships_file(seed: int, ship_size: int = 256, extra_size: int = 128) -> bytes:
    """A spaceships file: the word header, the ship class, the extra class, then their pictures."""
    ship, extra = bytes([seed]) * 0x40, bytes([seed + 1]) * 0x20
    ship_at = 0x10
    extra_at = ship_at + len(ship)
    ship_tex = extra_at + len(extra)
    extra_tex = ship_tex + len(picture(ship_size, seed))
    return (struct.pack("<IIII", ship_at, ship_tex, extra_at, extra_tex) + ship + extra
            + picture(ship_size, seed) + picture(extra_size, seed + 1))


class FakeDisc:
    """Sectors from a dict of files by their first sector; records what was read."""

    def __init__(self, files: dict[int, bytes]):
        self.files, self.reads = files, []

    def sectors(self, lba: int, count: int) -> bytes:
        self.reads.append((lba, count))
        data = self.files[lba]
        return (data + bytes(-len(data) % SECTOR))[:count * SECTOR].ljust(count * SECTOR, b"\0")


def reference(index: int, lba: int, size: int, group: str = "spaceships") -> dict:
    return {"group": group, "index": index, "lba": lba, "bytes": size}


class TextureTests(unittest.TestCase):
    def test_one_picture(self):
        blob = bytes(0x20) + picture(128, 5)
        texture = spaceship_texture(blob, 0x20, 128)
        self.assertEqual((texture.width, texture.height), (128, 128))
        self.assertEqual(texture.palette, blob[0x20 + 0x30:0x20 + 0x430])
        self.assertEqual(texture.pixels, blob[0x20 + 0x430:0x20 + 0x430 + 128 * 128])

    def test_refuses_anything_else(self):
        for name, blob, size in (("two pictures", picture(128, 0, count=2), 128),
                                 ("no PIF", picture(128, 0, magic=b"PIF2"), 128),
                                 ("another size", picture(128, 0), 256),
                                 ("another format", picture(128, 0, fmt=0x14), 128)):
            with self.subTest(name), self.assertRaises(FormatError) as caught:
                spaceship_texture(blob, 0, size)
            self.assertEqual(str(caught.exception), f"spaceship texture at 0x0: not one {size}x{size} PSMT8 picture")


class AddSpaceshipsTests(unittest.TestCase):
    def add(self, disc: FakeDisc, references: list[dict]):
        level = SimpleNamespace(textures={})
        calls = []

        def record(data, remap, name):
            calls.append((data, remap, name))
            return name

        with mock.patch.object(spaceships, "moby_class", side_effect=record):
            classes = add_spaceships(disc, {"global_references": references}, level)
        return classes, level.textures, calls

    def test_entries_one_to_three_are_the_three_ships(self):
        files = {100 + 10 * k: spaceships_file(k * 16) for k in range(4)}
        disc = FakeDisc(files)
        refs = [reference(k, lba, len(files[lba])) for k, lba in enumerate(sorted(files))]
        refs.append(reference(1, 999, 4, group="planets"))      # another group: not read
        classes, textures, calls = self.add(disc, refs)

        self.assertEqual(classes, {c: f"moby_{c}" for c in SHIP_CLASSES + EXTRA_CLASSES})
        self.assertEqual(list(classes), [531, 535, 532, 536, 533, 537])
        # Entry 0 (class 530) is Veldin 1's own, in its core: not read.
        self.assertEqual(disc.reads, [(lba, (len(files[lba]) + SECTOR - 1) // SECTOR) for lba in (110, 120, 130)])
        self.assertEqual(sorted(textures), [("moby", TEXTURE_BASE + n) for n in range(6)])
        for k in range(3):
            blob = files[110 + 10 * k]
            ship_at, ship_tex, extra_at, extra_tex = struct.unpack_from("<IIII", blob)
            ship_key, extra_key = TEXTURE_BASE + 2 * k, TEXTURE_BASE + 2 * k + 1
            self.assertEqual(calls[2 * k], (blob[ship_at:extra_at], [ship_key] * 16, f"moby_{SHIP_CLASSES[k]}"))
            self.assertEqual(calls[2 * k + 1], (blob[extra_at:ship_tex], [extra_key] * 16, f"moby_{EXTRA_CLASSES[k]}"))
            self.assertEqual(textures[("moby", ship_key)], spaceship_texture(blob, ship_tex, 256))
            self.assertEqual(textures[("moby", extra_key)], spaceship_texture(blob, extra_tex, 128))

    def test_missing_and_empty_entries_are_skipped(self):
        data = spaceships_file(1)
        disc = FakeDisc({200: data})
        classes, textures, _ = self.add(disc, [reference(2, 200, len(data)), reference(3, 300, 0)])
        self.assertEqual(list(classes), [532, 536])
        self.assertEqual(sorted(textures), [("moby", TEXTURE_BASE + 2), ("moby", TEXTURE_BASE + 3)])
        self.assertEqual(disc.reads, [(200, (len(data) + SECTOR - 1) // SECTOR)])

    def test_blocks_out_of_order_are_refused(self):
        data = bytearray(spaceships_file(1))
        ship_at, ship_tex, extra_at, extra_tex = struct.unpack_from("<IIII", data)
        struct.pack_into("<IIII", data, 0, ship_at, extra_at, ship_tex, extra_tex)   # the extra class after its picture
        with self.assertRaises(FormatError) as caught:
            self.add(FakeDisc({50: bytes(data)}), [reference(1, 50, len(data))])
        self.assertEqual(str(caught.exception), "spaceships 1: blocks out of order")


if __name__ == "__main__":
    unittest.main()
