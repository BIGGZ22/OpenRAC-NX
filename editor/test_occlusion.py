"""Synthetic PVS object mappings: no disc data or external tools."""
import json
from pathlib import Path
import struct
import tempfile
from types import SimpleNamespace
import unittest

from occlusion import STAMP, export_pvs, write_stamp


class OcclusionTest(unittest.TestCase):
    def level(self):
        index, gameplay, terrain = bytearray(256), bytearray(768), bytearray(128)
        struct.pack_into('<II', index, 8, 16, 32)
        struct.pack_into('<I', gameplay, 0x8c, 0x90)
        struct.pack_into('<3I', gameplay, 0x90, 1, 2, 1)
        for i, record in enumerate(((9, 7), (10, 41), (11, 41), (12, 93))):
            struct.pack_into('<ii', gameplay, 0xa0 + 8 * i, *record)
        struct.pack_into('<2I', terrain, 0, 16, 1)
        terrain[16 + 0x3d] = 7
        struct.pack_into('<I', gameplay, 0x34, 0x100)
        struct.pack_into('<I', gameplay, 0x100, 2)
        struct.pack_into('<i', gameplay, 0x110 + 12, 41)
        struct.pack_into('<i', gameplay, 0x110 + 0xe0 + 12, 41)
        return SimpleNamespace(index=index, gameplay=gameplay, id=9, version='ntsc',
                               block=lambda start: terrain if start == 16 else bytes(160))

    def test_mappings_and_staleness(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory)
            level = self.level()
            export_pvs(level, path)
            data = json.loads((path / 'pvs.json').read_text())
            self.assertEqual(data['terrain'], [0x102])
            self.assertEqual(data['ties'], {'41': 0x104}) # first duplicate wins
            self.assertEqual(data['tie_words'], [0x104, 0x108]) # positional fast path
            self.assertEqual(data['mobys'], {'93': 0x110})
            level.block(16)[16 + 0x3d] = 8
            export_pvs(level, path)
            self.assertEqual(json.loads((path / 'pvs.json').read_text())['terrain'], [0x7f80])

    def test_stamp(self):
        # The launcher waits for this file (actions.json, port-levels-visibility makes it).
        with tempfile.TemporaryDirectory() as directory:
            write_stamp(Path(directory))
            self.assertEqual(json.loads((Path(directory) / STAMP).read_text()), {'format': 1})
            actions = json.loads((Path(__file__).parent.parent / 'launcher/actions.json').read_text())
            makes = {a.get('makes') for version in actions['versions'].values() for a in version
                     if a.get('id') == 'port-levels-visibility'}
            self.assertTrue(makes and all(m and m.endswith('/' + STAMP) for m in makes), makes)

    def test_unsupported_and_missing(self):
        with tempfile.TemporaryDirectory() as directory:
            level = self.level()
            level.game = 'rac2'
            export_pvs(level, Path(directory))
            self.assertEqual(list(Path(directory).iterdir()), [])
            level.game = 'rac1'
            level.gameplay = b''
            export_pvs(level, Path(directory))
            self.assertEqual(list(Path(directory).iterdir()), [])
