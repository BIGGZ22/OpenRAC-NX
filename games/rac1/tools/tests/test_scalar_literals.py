"""Section placement for compiler-generated scalar constants; no instruction rewrites."""
import json
import sys
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch
sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import fix_jump_tables as f

FUNC='func_L00_00232EF0'
SYMBOL='D_L00_0015F7E8'
SOURCE=['\t.rdata\n','\t.align\t2\n','$LC17:\n','\t.word\t0x7f800000\t# inf\n','\t.text\n', '\t.ent '+FUNC+'\n','\tlui\t$2,%hi($LC17)\n','\tl.s\t$f23,%lo($LC17)($2)\n','\t.end '+FUNC+'\n']

class ScalarPlacementTests(unittest.TestCase):
    def setUp(self):
        self.tmp=tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.root=Path(self.tmp.name)
        self.cfg=self.root/'scalar_literals.tsv'
        self.cfg.write_text(f'{FUNC}\t{SYMBOL}\t7f800000\n')
        folder=self.root/'baserom/overlays/level_00'
        folder.mkdir(parents=True)
        (folder/'manifest.json').write_text(json.dumps({'records':[{'name':'lit','address':0x15F7E8,'bytes':8,'type':1}]}))
        (folder/'lit.bin').write_bytes(bytes.fromhex('0000807f00000000'))
        asm=self.root/'asm/overlays';asm.mkdir(parents=True)
        (asm/(FUNC+'.s')).write_text(f'lui $2,%hi({SYMBOL})\nlwc1 $f23,%lo({SYMBOL})($2)\n')
        self.a=patch.object(f,'ROOT',self.root);self.a.start();self.addCleanup(self.a.stop)
        self.b=patch.object(f,'SCALAR_CONFIG',self.cfg);self.b.start();self.addCleanup(self.b.stop)

    def test_known_scalar_only_changes_section_directive(self):
        rows=f.scalar_literals();lines=SOURCE.copy()
        replacement=f.scalar_section(lines,0,f.scalar_owners(lines),rows)
        self.assertEqual(replacement,'.section\t.rodata.'+SYMBOL+',"a"\n')
        self.assertEqual(lines,SOURCE)

    def test_wrong_configured_value_rejected(self):
        self.cfg.write_text(f'{FUNC} {SYMBOL} 3f800000\n')
        with self.assertRaisesRegex(ValueError,'value differs'): f.scalar_literals()

    def test_wrong_target_address_rejected(self):
        self.cfg.write_text(f'{FUNC} D_L00_0015F7EC 7f800000\n')
        with self.assertRaisesRegex(ValueError,'does not reference'): f.scalar_literals()

    def test_unaligned_target_rejected(self):
        self.cfg.write_text(f'{FUNC} D_L00_0015F7E9 7f800000\n')
        with self.assertRaisesRegex(ValueError,'not aligned'): f.scalar_literals()

    def test_ambiguous_config_rejected(self):
        self.cfg.write_text(f'{FUNC} {SYMBOL} 7f800000\n'*2)
        with self.assertRaisesRegex(ValueError,'ambiguous'): f.scalar_literals()

    def test_generated_wrong_value_stays_unplaced(self):
        lines=SOURCE.copy();lines[3]='\t.word\t0x3f800000\n'
        self.assertIsNone(f.scalar_section(lines,0,f.scalar_owners(lines),f.scalar_literals()))

    def test_unconfigured_shared_owner_stays_unplaced(self):
        owners=f.scalar_owners(SOURCE);owners['$LC17'].add('func_L00_00232A00')
        self.assertIsNone(f.scalar_section(SOURCE,0,owners,f.scalar_literals()))

    def test_conflicting_owners_rejected(self):
        owners={'$LC17':{FUNC,'func_L00_00232A00'}}
        configured={(FUNC,0x7f800000):SYMBOL,('func_L00_00232A00',0x7f800000):'D_L00_0015F7EC'}
        with self.assertRaisesRegex(ValueError,'ambiguous'): f.scalar_section(SOURCE,0,owners,configured)

    def test_strings_and_wide_pools_stay_unplaced(self):
        for content in ['\t.ascii "abc"\n','\t.word 0x7f800000,0x00000000\n']:
            lines=SOURCE.copy();lines[3]=content
            self.assertIsNone(f.scalar_section(lines,0,f.scalar_owners(lines),f.scalar_literals()))

    def test_compiled_section_independently_checked(self):
        class Section(dict):
            name='.rodata.'+SYMBOL
            value=bytes.fromhex('0000807f')
            def data(self): return self.value
        sec=Section(sh_addralign=4)
        self.assertEqual(f.validate_scalar_section(FUNC,sec),SYMBOL)
        sec.value=bytes.fromhex('0000803f')
        with self.assertRaisesRegex(ValueError,'value or alignment'): f.validate_scalar_section(FUNC,sec)
        sec.value=bytes.fromhex('0000807f');sec['sh_addralign']=16
        with self.assertRaisesRegex(ValueError,'value or alignment'): f.validate_scalar_section(FUNC,sec)
        sec['sh_addralign']=4
        with self.assertRaisesRegex(ValueError,'unconfigured'): f.validate_scalar_section('func_L00_00232A00',sec)

if __name__=='__main__': unittest.main()
