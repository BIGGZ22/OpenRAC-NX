"""Regression tests for metadata-only preparation for the real assembler."""
from pathlib import Path
import hashlib
import tempfile
import unittest
from unittest.mock import patch

import ps2eeas


class PrepareTests(unittest.TestCase):
    def test_compiler_instructions_are_unchanged(self):
        source = ('.include "include/labels.inc"\n.text\n'
                  'func_x:\n\tlui $2,%hi(D_00133380)\n'
                  '\taddiu $2,$2,%lo(D_00133380)\n'
                  '\tsw $2,D_0015EDA0+4\n\tjr $31\n\tnop\n'
                  '\t.extern D_0015EDA0, 2\n')
        result = ps2eeas.prepare(source)
        expected = '\t.extern D_0015EDA0, 2\n' + source.replace('.include "include/labels.inc"', '')
        self.assertEqual(result, expected)

    def test_remnant_metadata_expands_without_changing_instructions(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            remnant = root / 'asm/remnants/core_text/example_remnant.s'
            remnant.parent.mkdir(parents=True)
            instructions = ('    /* synthetic instruction */  addiu $29,$29,0x20\n'
                            '    /* synthetic padding */  nop\n')
            remnant.write_text('.align 3\nnonmatching example_remnant, 0x8\n'
                               'glabel example_remnant\n' + instructions +
                               'endlabel example_remnant\n    nop\n')
            with patch.object(ps2eeas, 'ROOT', root):
                result = ps2eeas.prepare('.include "asm/remnants/core_text/example_remnant.s"\n')
            self.assertIn(instructions, result)
            self.assertEqual(result.count('    nop\n'), 1)
            self.assertIn('.size example_remnant.NON_MATCHING, 0x8', result)
            self.assertIn('.ent example_remnant\nexample_remnant:', result)
            self.assertIn('.end example_remnant', result)
            self.assertNotIn('glabel', result)
            self.assertNotIn('endlabel', result)

    def test_unknown_or_escaping_include_is_rejected(self):
        for name in ('src/example.s', 'asm/handwritten/core_text/example.s',
                     'asm/remnants/../../src/example.s', 'asm/remnants/file.h'):
            with self.subTest(name=name), self.assertRaises(ValueError):
                ps2eeas.prepare(f'.include "{name}"\n')

    def test_preparation_errors_remove_old_objects(self):
        for failure in ('missing_binary', 'missing_source', 'unsupported_include'):
            with self.subTest(failure=failure), tempfile.TemporaryDirectory() as directory:
                root = Path(directory)
                source, output, assembler = (root / name for name in ('input.s', 'old.o', 'as.exe'))
                output.write_bytes(b'old object must not survive')
                assembler.write_bytes(b'test assembler identity')
                source.write_text('.include "src/unsupported.s"\n')
                digest = hashlib.sha256(assembler.read_bytes()).hexdigest()
                if failure == 'missing_binary':
                    assembler.unlink()
                elif failure == 'missing_source':
                    source.unlink()
                with patch.object(ps2eeas, 'ASSEMBLER', assembler), \
                     patch.object(ps2eeas, 'SHA256', digest), \
                     patch.object(ps2eeas.sys, 'argv', ['ps2eeas.py', str(source), str(output)]), \
                     patch.object(ps2eeas.subprocess, 'run') as run:
                    with self.assertRaises(SystemExit):
                        ps2eeas.main()
                    run.assert_not_called()
                self.assertFalse(output.exists())


if __name__ == '__main__':
    unittest.main()
