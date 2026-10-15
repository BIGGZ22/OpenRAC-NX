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


if __name__ == "__main__":
    unittest.main()
