"""Safety checks for the assembler's volatile call-slot model."""
import subprocess
import sys
import tempfile
from pathlib import Path
import unittest

SCRIPT = Path(__file__).resolve().parents[1] / 'fix_volatile_slot.py'


class CallSlotTests(unittest.TestCase):
    def transform(self, access, *, loads=False, between='', reorder=True, prior_noreorder=False):
        prior = (' .set noreorder\nlui $2,0x13\n.set reorder\n' if prior_noreorder
                 else 'lui $2,0x13\n')
        source = ('.set ' + ('reorder' if reorder else 'noreorder') + '\n'
                  + prior + '#.set volatile\n' + access + '\n#.set novolatile\n'
                  + between + 'jal callee\n')
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / 'input.s'
            path.write_text(source)
            subprocess.run([sys.executable, str(SCRIPT), *(['--loads'] if loads else []),
                            str(path), str(path)], check=True, capture_output=True)
            return source, path.read_text()

    def test_word_load_profile(self):
        for access in ['lw $4,8($2)', 'lw $4,%lo(symbol)($2)']:
            before, after = self.transform(access, loads=True)
            self.assertNotEqual(before, after)
            self.assertIn('jal callee\n' + access, after)

    def test_legacy_profile_does_not_move_loads(self):
        before, after = self.transform('lw $4,%lo(symbol)($2)')
        self.assertEqual(before, after)

    def test_store_behavior_preserved(self):
        _, after = self.transform('sw $4,8($2)')
        self.assertIn('jal callee\nsw $4,8($2)', after)

    def test_no_crossing_labels_or_alignment(self):
        for between in ['entry:\n', '.align 3\n']:
            before, after = self.transform('lw $4,8($2)', loads=True, between=between)
            self.assertEqual(before, after)

    def test_return_address_hazards(self):
        for access in ['lw $31,8($2)', 'lw $4,8($31)', 'sw $31,8($2)']:
            before, after = self.transform(access, loads=True)
            self.assertEqual(before, after)

    def test_load_after_explicit_noreorder_is_preserved(self):
        before, after = self.transform('lw $4,8($2)', loads=True,
                                       prior_noreorder=True)
        self.assertEqual(before, after)

    def test_noreorder_and_unverified_load_macros(self):
        before, after = self.transform('lw $4,8($2)', loads=True, reorder=False)
        self.assertEqual(before, after)
        for access in ['lw $4,0x12000000', 'ld $4,8($2)']:
            before, after = self.transform(access, loads=True)
            self.assertEqual(before, after)


if __name__ == '__main__':
    unittest.main()
