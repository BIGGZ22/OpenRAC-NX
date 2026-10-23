# SPDX-License-Identifier: GPL-3.0-or-later
import json
from pathlib import Path
import subprocess
import tempfile
import unittest
from unittest.mock import patch

import bake_all as batch


class AllLevelBake(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        # bake_all resolves its paths; on macOS the temporary folder's /var is /private/var.
        self.root = Path(self.temp.name).resolve()
        self.levels = self.root/'exports'
        self.levels.mkdir()
        self.output = self.root/'bakes'
        self.baker = self.root/'fake-baker'
        self.baker.touch()
        for name in ('level_10', 'level_02', 'level_00', 'level_99'):
            level = self.levels/name
            level.mkdir()
            (level/'manifest.json').write_text('{}')
        (self.levels/'textures').mkdir()

    def test_all_levels_in_numeric_order_and_options_forwarded(self):
        def bake(level, output, baker, **kwargs):
            return output/'sparse.orpg'
        with patch.object(batch.bake_level, 'bake_level', side_effect=bake) as run:
            self.assertTrue(batch.bake_all(self.levels, self.output, self.baker,
                                          disc=self.root/'owned.iso', options=['--threads', '2']))
        self.assertEqual([call.args[0].name for call in run.call_args_list],
                         ['level_00', 'level_02', 'level_10', 'level_99'])
        for call in run.call_args_list:
            self.assertEqual(call.args[1], self.output/call.args[0].name)
            self.assertEqual(call.kwargs['options'], ['--threads', '2'])
            self.assertEqual(call.kwargs['disc'], self.root/'owned.iso')
        report = json.loads((self.output/'bake-all-result.json').read_text())
        self.assertTrue(all(level['verified'] for level in report['levels']))
        self.assertEqual(report['levels'][0]['sparse'], 'level_00/sparse.orpg')

    def test_failed_level_is_reported_and_remaining_levels_still_bake(self):
        def bake(level, output, baker, **kwargs):
            if level.name == 'level_02':
                raise subprocess.CalledProcessError(1, ['fake-baker', '--verify'])
            return output/'sparse.orpg'
        with patch.object(batch.bake_level, 'bake_level', side_effect=bake) as run:
            self.assertFalse(batch.bake_all(self.levels, self.output, self.baker))
        self.assertEqual(run.call_count, 4)
        report = json.loads((self.output/'bake-all-result.json').read_text())
        self.assertEqual([level['verified'] for level in report['levels']], [True, False, True, True])
        self.assertIn('error', report['levels'][1])

    def test_preflight_rejects_missing_manifest_before_any_bake(self):
        (self.levels/'level_99/manifest.json').unlink()
        with patch.object(batch.bake_level, 'bake_level') as run:
            with self.assertRaisesRegex(ValueError, 'missing export manifest'):
                batch.bake_all(self.levels, self.output, self.baker)
        run.assert_not_called()
        self.assertFalse(self.output.exists())

    def test_existing_output_and_source_overlap_rejected(self):
        self.output.mkdir()
        keep = self.output/'keep'
        keep.write_text('previous bake')
        with self.assertRaisesRegex(ValueError, 'previous bakes are preserved'):
            batch.bake_all(self.levels, self.output, self.baker)
        self.assertEqual(keep.read_text(), 'previous bake')
        with self.assertRaisesRegex(ValueError, 'outside the source'):
            batch.bake_all(self.levels, self.levels/'bakes', self.baker)

    def test_dry_run_writes_nothing_and_needs_no_native_baker(self):
        with patch.object(batch.bake_level, 'bake_level') as run:
            self.assertTrue(batch.bake_all(self.levels, self.output, self.root/'missing', dry_run=True))
        run.assert_not_called()
        self.assertFalse(self.output.exists())

    def test_no_levels_or_missing_baker_fail_without_outputs(self):
        empty = self.root/'empty'
        empty.mkdir()
        with self.assertRaisesRegex(ValueError, 'no exported level'):
            batch.bake_all(empty, self.output, self.baker)
        with self.assertRaisesRegex(ValueError, 'native baker not found'):
            batch.bake_all(self.levels, self.output, self.root/'missing')
        self.assertFalse(self.output.exists())

    def test_cli_exit_status_and_overrides(self):
        with patch('sys.argv', ['bake_all.py', '--levels-dir', str(self.levels),
                               '--output-dir', str(self.output), '--threads', '0',
                               '--rays', '32', '--bounces', '2', '--dry-run']), \
             patch.object(batch, 'bake_all', return_value=False) as run:
            self.assertEqual(batch.main(), 1)
        self.assertEqual(run.call_args.kwargs['options'],
                         ['--rays', '32', '--bounces', '2', '--threads', '0'])
        self.assertTrue(run.call_args.kwargs['dry_run'])
