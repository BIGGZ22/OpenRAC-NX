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

    def fake_bake(self, fail=()):
        def bake(level, output, baker, **kwargs):
            if level.name in fail:
                output.mkdir(parents=True)
                (output/'sparse.bake.log').write_text('failed')
                raise subprocess.CalledProcessError(1, ['fake-baker'])
            output.mkdir(parents=True)
            (output/'sparse.orpg').write_text(level.name)
            (output/'bake-result.json').write_text('{}')
            return output/'sparse.orpg'
        return bake

    def test_update_keeps_current_levels_and_bakes_changed_ones_in_place(self):
        with patch.object(batch.bake_level, 'bake_level', side_effect=self.fake_bake()) as run:
            self.assertTrue(batch.bake_all(self.levels, self.output, self.baker, update=True, jobs=0))
        self.assertEqual(run.call_count, 4)
        for name in ('level_00', 'level_02', 'level_10', 'level_99'):
            self.assertEqual((self.output/name/'sparse.orpg').read_text(), name)
            self.assertTrue((self.output/name/batch.STAMP).is_file())
        # Nothing changed: nothing is baked again.
        with patch.object(batch.bake_level, 'bake_level', side_effect=self.fake_bake()) as run:
            self.assertTrue(batch.bake_all(self.levels, self.output, self.baker, update=True, jobs=2))
        run.assert_not_called()
        # A level read again from the disc, another baker, or other options: baked again.
        (self.levels/'level_02/manifest.json').write_text('{"format": 4}')
        with patch.object(batch.bake_level, 'bake_level', side_effect=self.fake_bake()) as run:
            self.assertTrue(batch.bake_all(self.levels, self.output, self.baker, update=True))
        self.assertEqual([call.args[0].name for call in run.call_args_list], ['level_02'])
        self.assertEqual(json.loads((self.output/'level_02'/batch.STAMP).read_text())['levels_format'], 4)
        self.baker.write_text('a new baker')
        with patch.object(batch.bake_level, 'bake_level', side_effect=self.fake_bake()) as run:
            self.assertTrue(batch.bake_all(self.levels, self.output, self.baker, update=True,
                                           options=['--threads', '3']))
        self.assertEqual(run.call_count, 4)
        with patch.object(batch.bake_level, 'bake_level', side_effect=self.fake_bake()) as run:
            self.assertTrue(batch.bake_all(self.levels, self.output, self.baker, update=True,
                                           options=['--rays', '64']))
        self.assertEqual(run.call_count, 4)
        self.assertFalse(list(self.output.glob('.*')))

    def test_update_failure_keeps_the_previous_bake_and_the_logs(self):
        with patch.object(batch.bake_level, 'bake_level', side_effect=self.fake_bake()):
            self.assertTrue(batch.bake_all(self.levels, self.output, self.baker, update=True))
        (self.levels/'level_10/manifest.json').write_text('{"changed": true}')
        with patch.object(batch.bake_level, 'bake_level', side_effect=self.fake_bake(fail={'level_10'})):
            self.assertFalse(batch.bake_all(self.levels, self.output, self.baker, update=True))
        self.assertEqual((self.output/'level_10/sparse.orpg').read_text(), 'level_10')
        self.assertEqual((self.output/'.level_10.failed/sparse.bake.log').read_text(), 'failed')
        report = json.loads((self.output/'bake-all-result.json').read_text())
        self.assertEqual([level['verified'] for level in report['levels']], [True, True, False, True])

    def test_cache_stamp_tracks_planner_and_level_settings(self):
        level = self.levels/'level_02'
        initial = batch.level_stamp(level, 'native', [])
        with patch.object(batch, 'planner_hash', return_value='changed planner'):
            self.assertNotEqual(initial, batch.level_stamp(level, 'native', []))
        (level/'probe-regions.json').write_text('{"version": 1, "geometry_max_rise": 24}')
        regions = batch.level_stamp(level, 'native', [])
        self.assertNotEqual(initial, regions)
        (level/'probe-bake.json').write_text('{"version": 1, "dense_spacing": [4, 4, 2]}')
        self.assertNotEqual(regions, batch.level_stamp(level, 'native', []))

    def test_jobs_share_the_budget_unless_told(self):
        self.assertEqual(batch.with_threads(['--threads', '2'], 4), ['--threads', '2'])
        # 8 threads: 6 for the bake, 2 kept for the desktop (background.cpu_budget).
        with patch('os.cpu_count', return_value=8):
            self.assertEqual(batch.with_threads(['--rays', '8'], 1), ['--rays', '8', '--threads', '6'])
            self.assertEqual(batch.with_threads([], 3), ['--threads', '2'])
            self.assertEqual(batch.with_threads([], 16), ['--threads', '1'])
            self.assertEqual(batch.automatic_jobs(), 1)
        # A 16-core, 32-thread desktop: 28 threads, seven levels at once with four each, as
        # memory allows.
        with patch('os.cpu_count', return_value=32), \
             patch.object(batch, 'physical_memory', return_value=128 << 30):
            self.assertEqual(batch.automatic_jobs(), 7)
            self.assertEqual(batch.with_threads([], 7), ['--threads', '4'])
        with patch('os.cpu_count', return_value=32), \
             patch.object(batch, 'physical_memory', return_value=4 << 30):
            self.assertEqual(batch.automatic_jobs(), 2)

    def test_bakes_the_largest_levels_first_sharing_the_threads(self):
        (self.levels/'level_99/big.bin').write_bytes(bytes(4096))
        (self.levels/'level_02/mid.bin').write_bytes(bytes(1024))
        self.assertEqual([batch.discover_levels(self.levels)[i].name
                          for i in batch.largest_first(batch.discover_levels(self.levels))[:2]],
                         ['level_99', 'level_02'])
        calls = []

        def bake(level, work, baker, *, disc=None, options=()):
            calls.append((level.name, list(options)))
            work.mkdir(parents=True)
            (work/'sparse.orpg').write_text(level.name)
            return work/'sparse.orpg'

        with patch.object(batch.bake_level, 'bake_level', side_effect=bake), \
             patch('os.cpu_count', return_value=8):
            self.assertTrue(batch.bake_all(self.levels, self.output, self.baker, jobs=2))
        # Two at once: the two largest start together, each with half of the bake's 6 threads.
        self.assertEqual({name for name, _ in calls[:2]}, {'level_99', 'level_02'})
        self.assertTrue(all(options == ['--threads', '3'] for _, options in calls))
        # The report keeps the levels' own order.
        report = json.loads((self.output/'bake-all-result.json').read_text())
        self.assertEqual([level['level'] for level in report['levels']],
                         [level.name for level in batch.discover_levels(self.levels)])

    def test_cli_exit_status_and_overrides(self):
        with patch('sys.argv', ['bake_all.py', '--levels-dir', str(self.levels),
                               '--output-dir', str(self.output), '--threads', '0',
                               '--rays', '32', '--bounces', '2', '--dry-run']), \
             patch.object(batch.background, 'lower_priority') as lowered, \
             patch.object(batch, 'bake_all', return_value=False) as run:
            self.assertEqual(batch.main(), 1)
        lowered.assert_called_once()
        self.assertEqual(run.call_args.kwargs['options'],
                         ['--rays', '32', '--bounces', '2', '--threads', '0'])
        self.assertTrue(run.call_args.kwargs['dry_run'])
