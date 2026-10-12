"""wadpack.compress against formats.wad, on made-up data."""

import random
import unittest

from formats import FormatError, wad
from wadpack import BLOCK, HEADER, ROOM, compress


def packets(stream: bytes):
    """(start, end, is_skip) of each packet after the header: the walk formats.wad does."""
    end = int.from_bytes(stream[3:7], "little")
    pos = HEADER
    if pos < end and stream[pos] > 0x11:
        pos += 1 + stream[pos] - 0x11
        yield HEADER, pos, False
    while pos < end:
        start, tag = pos, stream[pos]
        pos += 1
        if tag < 0x10:
            count = tag + 3 if tag else stream[pos] + 18
            pos += (0 if tag else 1) + count
            yield start, pos, False
            continue
        if tag >= 0x40:
            pos += 1
            trailing = tag & 3
        elif tag >= 0x20:
            pos += (0 if tag & 31 else 1) + 2
            trailing = stream[pos - 2] & 3
        else:
            count = tag & 7
            if not count:
                count = stream[pos] + 7
                pos += 1
            lo, hi = stream[pos], stream[pos + 1]
            pos += 2
            if (tag & 8) * 2048 + (lo >> 2) + hi * 64 == 0 and count != 1:
                yield start, pos, True
                pos = HEADER + (pos - HEADER + BLOCK - 1) // BLOCK * BLOCK
                continue
            trailing = lo & 3
        pos += trailing
        yield start, pos, False


class Compress(unittest.TestCase):
    def samples(self):
        rng = random.Random(1)
        words = [bytes(rng.randrange(256) for _ in range(rng.randrange(1, 40))) for _ in range(60)]
        yield b""
        yield b"a"
        yield b"abc" * 5
        yield bytes(100_000)                                            # one long run
        yield bytes(rng.randrange(256) for _ in range(40_000))          # nothing repeats
        yield b"".join(rng.choice(words) for _ in range(6000))          # like level data: repeats near and far
        yield b"".join(rng.choice(words) for _ in range(300)) + bytes(70_000) + b"".join(words)

    def test_reads_back(self):
        for data in self.samples():
            self.assertEqual(wad(compress(data)), data)

    def test_blocks_as_the_game_reads_them(self):
        """No packet crosses a block's end; a block that is followed by another ends with a skip
        and room after it."""
        for data in self.samples():
            stream = compress(data)
            skips = set()
            for start, end, skip in packets(stream):
                self.assertEqual((start - HEADER) // BLOCK, (end - 1 - HEADER) // BLOCK, "a packet crosses a block")
                if skip:
                    self.assertGreaterEqual(BLOCK - (end - HEADER) % BLOCK, ROOM)
                    skips.add((start - HEADER) // BLOCK)
            blocks = (len(stream) - HEADER + BLOCK - 1) // BLOCK
            self.assertEqual(skips, set(range(blocks - 1)))

    def test_keeps_the_header_it_is_given(self):
        header = b"WAD" + bytes(4) + bytes(range(9))
        stream = compress(b"hello hello hello", header)
        self.assertEqual(stream[7:16], header[7:16])
        self.assertEqual(int.from_bytes(stream[3:7], "little"), len(stream))
        with self.assertRaises(FormatError):
            compress(b"x", b"not a wad header")

    def test_repeats_shrink(self):
        data = b"the same forty bytes, over and over: 40." * 2000
        self.assertLess(len(compress(data)), len(data) // 20)


if __name__ == "__main__":
    unittest.main()
