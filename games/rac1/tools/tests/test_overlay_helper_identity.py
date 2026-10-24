"""Keep complete-global helper fields in the overlay identity fingerprint."""
import struct
import sys
import unittest
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from overlays import identity, fingerprint

SPEED = struct.pack("<5I", 0x3c02001a, 0x24421234, 0xac400124, 0x03e00008, 0xac400120)
MAPPED = struct.pack("<5I", 0x3c02001a, 0x24421234, 0xac400284, 0x03e00008, 0xac400280)

class CompleteGlobalHelperIdentityTests(unittest.TestCase):
    def test_helpers_with_different_fields_are_not_aliases(self):
        self.assertNotEqual(fingerprint(SPEED), fingerprint(MAPPED))

    def test_global_relocation_does_not_change_helper_identity(self):
        moved = bytearray(MAPPED)
        moved[0:4] = (0x3c020016).to_bytes(4, 'little')
        moved[4:8] = (0x2442e780).to_bytes(4, 'little')
        self.assertEqual(identity(MAPPED), identity(bytes(moved)))

    def test_each_member_offset_affects_identity(self):
        for at in (8, 16):
            other = bytearray(MAPPED)
            word = int.from_bytes(other[at:at+4], 'little')
            other[at:at+4] = (word ^ 4).to_bytes(4, 'little')
            self.assertNotEqual(identity(MAPPED), identity(bytes(other)))

if __name__ == '__main__':
    unittest.main()
