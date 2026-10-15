import struct
import unittest

import versions

GP = 0x00170000


def words(*ws: int) -> bytes:
    return struct.pack(f"<{len(ws)}I", *ws)


class Derive(unittest.TestCase):
    def test_values_from_the_other_versions_bytes(self):
        # jal sym; lui $2, %hi(D); lw $2, %lo(D)($2); lw $3, %gprel(G)($gp)
        code = words(0x0C000000, 0x3C020000, 0x8C420000, 0x8F830000)
        relocs = [(0, versions.R_MIPS_26, "func"), (4, versions.R_MIPS_HI16, "D"),
                  (8, versions.R_MIPS_LO16, "D"), (12, versions.R_MIPS_GPREL16, "G")]
        # There: jal 0x00123450; D = 0x0015FFF0 (%hi 0x0016, %lo -0x10); G = gp - 8
        retail = words(0x0C000000 | 0x00123450 >> 2, 0x3C020016, 0x8C42FFF0, 0x8F83FFF8)
        values, why = versions.derive(code, relocs, 0x00200000, retail, GP)
        self.assertIsNone(why)
        self.assertEqual(values, {"func": 0x00123450, "D": 0x0015FFF0, "G": GP - 8})

    def test_an_unrelocated_difference_is_not_the_same_function(self):
        code = words(0x24020001, 0x03E00008)
        _, why = versions.derive(code, [], 0x00200000, words(0x24020002, 0x03E00008), GP)
        self.assertEqual(why, "instruction differs")

    def test_one_symbol_one_value(self):
        code = words(0x0C000000, 0x0C000000)
        relocs = [(0, versions.R_MIPS_26, "f"), (4, versions.R_MIPS_26, "f")]
        retail = words(0x0C000000 | 0x100 >> 2, 0x0C000000 | 0x200 >> 2)
        _, why = versions.derive(code, relocs, 0, retail, GP)
        self.assertEqual(why, "a symbol needs two values")


class Paired(unittest.TestCase):
    HI, LO, J = versions.R_MIPS_HI16, versions.R_MIPS_LO16, versions.R_MIPS_26

    def test_each_hi_goes_before_its_lo(self):
        relocs = [(0x44, self.LO, "D"), (0x14, self.HI, "D"), (0x1C, self.J, "f")]
        self.assertEqual(versions.paired(relocs), [(0x14, self.HI, "D"), (0x44, self.LO, "D"), (0x1C, self.J, "f")])

    def test_two_his_share_one_lo(self):
        relocs = [(0xDC, self.HI, "D"), (0xE4, self.LO, "D"), (0x120, self.HI, "D")]
        self.assertEqual(versions.paired(relocs),
                         [(0xDC, self.HI, "D"), (0x120, self.HI, "D"), (0xE4, self.LO, "D")])


class WriteObject(unittest.TestCase):
    def test_round_trip(self):
        import tempfile
        from pathlib import Path
        with tempfile.TemporaryDirectory() as d:
            path = Path(d) / "p.o"
            versions.write_object(path, words(0x0C000000, 0x03E00008), {"f": 0},
                                  [(0, versions.R_MIPS_26, "g")], 4)
            obj = versions.read_object(path)
        text = next(i for i, s in enumerate(obj["sections"]) if s["name"] == ".text")
        self.assertEqual(obj["sections"][text]["data"], words(0x0C000000, 0x03E00008))
        names = [s["name"] for s in obj["symbols"]]
        self.assertEqual(names, ["", "f", "g"])
        self.assertEqual(obj["relocations"][text], [(0, versions.R_MIPS_26, 2)])


if __name__ == "__main__":
    unittest.main()
