import math
import struct
import unittest

import lighting as L


def f(x: float) -> int:
    return struct.unpack("<I", struct.pack("<f", x))[0]


def table() -> list[tuple[int, int]]:
    t = [(f(math.cos(i * math.tau / 256)), f(math.sin(i * math.tau / 256))) for i in range(256)]
    t[0], t[64], t[192] = (f(1.0), 0), (0, f(1.0)), (0, f(-1.0))
    return t


class Ps2Float(unittest.TestCase):
    # The checks ReRAC's tfrag_light.rs makes of its float model.
    def test_truncates(self):
        self.assertEqual(L.add(f(65536.0), f(0.5 / 128.0 * 1.999)), f(65536.0))
        self.assertEqual(L.add(f(65536.0), f(1.0)), f(65537.0))
        self.assertEqual(L.mul(f(1.0), f(0.3)), f(0.3))
        self.assertEqual(L.mul(0x3F800003, 0x3FC00001), 0x3FC00005)
        self.assertEqual(L.add(f(1.0), f(-1.0)), 0)
        self.assertEqual(L.add(f(1.0), f(1e-10)), f(1.0))
        self.assertEqual(L.add(f(1.0), f(-1.5 * 2.0 ** -24)), 0x3F7FFFFF)
        self.assertEqual(L.sqrt(f(4.0)), f(2.0))
        self.assertEqual(L.sqrt(f(2.0)), 0x3FB504F3)
        self.assertEqual(L.div(f(1.0), f(4.0)), f(0.25))
        self.assertEqual(L.div(f(1.0), f(3.0)), 0x3EAAAAAA)
        self.assertEqual(L.itof12(0x800), f(0.5))
        self.assertEqual(L.itof12(-4096 * 3), f(-3.0))
        self.assertEqual(L.mul(f(1e30), f(1e30)), L.MAX)


class Vertex(unittest.TestCase):
    def bank(self, colour_a, dir_a, back=0.0):
        bank = [[[0] * 4 for _ in range(4)] for _ in range(L.BANK_SETS)]
        bank[0] = [L.bits((*colour_a, back)), L.bits((*dir_a, 0.0)), [0] * 4, [0] * 4]
        return bank

    def test_base_colour_only(self):
        bank = [[[0] * 4 for _ in range(4)] for _ in range(L.BANK_SETS)]
        # 5:5:5:1 (8, 16, 24) with the alpha bit: bytes (64, 128, 192, 128).
        colour = 8 | 16 << 5 | 24 << 10 | 1 << 15
        self.assertEqual(L.light_vertex(bank, L.decode_normal(table(), 0, 64), colour, 0), (64, 128, 192, 128))

    def test_sun_from_above_lights_an_upward_normal(self):
        # Elevation 64: N = (0, 0, 1), n = -N; a sun travelling down (0, 0, -1) gives d = 1.
        bank = self.bank((0.5, 0.25, 1.0), (0.0, 0.0, -1.0))
        n = L.decode_normal(table(), 0, 64)
        self.assertEqual(L.light_vertex(bank, n, 0, 0), (64, 32, 128, 0))
        # Facing away: the back factor 0 leaves only the base.
        self.assertEqual(L.light_vertex(bank, L.decode_normal(table(), 0, 192), 0, 0), (0, 0, 0, 0))

    def test_clamped_at_255(self):
        bank = self.bank((4.0, 4.0, 4.0), (0.0, 0.0, -1.0))
        self.assertEqual(L.light_vertex(bank, L.decode_normal(table(), 0, 64), 0x7FFF, 0)[:3], (255, 255, 255))



class NormalTable(unittest.TestCase):
    def test_read_where_the_version_keeps_it(self):
        # A synthetic ELF: one load segment at 0x1000 from file offset 0x100, the table at 0x1200.
        exe = bytearray(0x100 + 0x1000)
        struct.pack_into("<I", exe, 0x1C, 0x34)
        struct.pack_into("<HH", exe, 0x2A, 0x20, 1)
        struct.pack_into("<5I", exe, 0x34, 1, 0x100, 0x1000, 0x1000, 0x1000)
        words = [w for pair in table() for w in pair]
        struct.pack_into("<512I", exe, 0x300, *words)
        self.assertEqual(L.normal_table(bytes(exe), 0x1200), table())
        with self.assertRaises(L.FormatError):
            L.normal_table(bytes(exe))      # SCES_509.16's address is outside this one
        with self.assertRaises(L.FormatError):
            L.normal_table(bytes(exe), 0x1C00)  # runs past the segment


if __name__ == "__main__":
    unittest.main()
