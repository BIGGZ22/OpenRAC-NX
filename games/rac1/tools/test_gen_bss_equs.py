"""Address-only aliases emitted by the original SN assembler remain linkable."""
import contextlib
import io
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

import gen_bss_equs


class SdataEquatesTests(unittest.TestCase):
    def test_known_sdata_aliases_use_the_base_address(self):
        with tempfile.TemporaryDirectory() as directory:
            source = Path(directory) / 'undefined.txt'
            output = Path(directory) / 'equs.s'
            source.write_text('D_0015EDB8__gp\nD_0015EDA8__gp\nD_0015EDC0__gp\nD_0015EDB8\nfunc_001211B0\n')
            with patch('sys.argv', ['gen_bss_equs.py', str(source), str(output)]), contextlib.redirect_stdout(io.StringIO()):
                gen_bss_equs.main()
            actual = output.read_text()
            for name, address in [('D_0015EDB8__gp', '0015EDB8'),
                                  ('D_0015EDA8__gp', '0015EDA8'),
                                  ('D_0015EDC0__gp', '0015EDC0'),
                                  ('D_0015EDB8', '0015EDB8'),
                                  ('func_001211B0', '001211B0')]:
                self.assertIn(f'.global {name}\n.equ {name}, 0x{address}\n', actual)

    def test_unknown_suffixes_and_malformed_aliases_stay_unresolved(self):
        names = ['D_0015EDB8_typo', 'D_1234__gp', 'func_001211B0__gp',
                 'D_0015EDB8__gp_extra', 'unknown__gp']
        with tempfile.TemporaryDirectory() as directory:
            source = Path(directory) / 'undefined.txt'
            output = Path(directory) / 'equs.s'
            source.write_text('\n'.join(names))
            capture = io.StringIO()
            with patch('sys.argv', ['gen_bss_equs.py', str(source), str(output)]), contextlib.redirect_stdout(capture):
                gen_bss_equs.main()
            self.assertNotIn('.equ ', output.read_text())
            for name in names:
                self.assertIn(name, capture.getvalue())


if __name__ == '__main__':
    unittest.main()
