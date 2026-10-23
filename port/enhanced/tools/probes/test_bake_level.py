# SPDX-License-Identifier: GPL-3.0-or-later
import json
import io
from pathlib import Path
import subprocess
import tempfile
import unittest
from unittest.mock import patch

import bake_level as batch
import seed_regions as seeds
from test_seed_regions import gameplay


INSPECTION = '''game_version rac1-ntsc
level 1
probe_origin 0 0 0
probe_spacing 32 32 32
probe_dimensions 5 5 5
emissive_triangles 20
automatic_liquid_triangles 0
point_emitters 2
emission_masks none 15 green 5
scene_hash 42
'''


class LevelBake(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.level = self.root/'level'
        self.level.mkdir()
        self.manifest = {'game': 'rac1', 'version': 'ntsc', 'level': 1,
                         'bake_inputs': {'gameplay': 'rac1-v1'}}
        (self.level/'manifest.json').write_text(json.dumps(self.manifest))
        (self.level/'bake-inputs').mkdir()
        (self.level/'bake-inputs/gameplay.bin').write_bytes(gameplay([1]))
        (self.level/'collision.glb').write_bytes(b'synthetic collision placeholder')
        (self.level/'placements.json').write_text(json.dumps({'mobys': [
            {'index': 7, 'class': 500, 'matrix': [0]*12+[40, 40, 60, 1]}]}))
        self.profile = {'dense_spacing': [4, 4, 4], 'probe_format_version': 3, 'bake_version': 1}
        self.output = self.root/'bake'
        self.calls = []
        self.failure = None
        self.changed = False

    def run_native(self, command, **kwargs):
        self.calls.append(command)
        owner = self

        class Input(io.StringIO):
            def close(self):
                if not self.closed and process.returncode is None:
                    owner.sent_plan = json.loads(self.getvalue())
                    output = Path(command[command.index('--batch-output')+1])
                    names = ['sparse'] + [f'region-{i:03d}' for i in range(len(owner.sent_plan['regions']))]
                    for name in names:
                        (output/(name+'.orpg')).write_bytes(b'fake native output')
                    process.returncode = int(bool(owner.failure or owner.changed))
                super().close()

        class Process:
            def __init__(self):
                self.returncode = None
                self.terminated = False
                result = ('source level changed\n' if owner.changed else 'verification failed\n') \
                    if owner.failure or owner.changed else 'Probe batch complete\n'
                self.stdout = io.StringIO(INSPECTION+'Probe batch ready\n'+result)
                self.stdin = Input()

            def wait(self, **kwargs):
                return self.returncode

            def poll(self):
                return self.returncode

            def terminate(self):
                self.terminated = True
                self.returncode = -15

            kill = terminate

        process = Process()
        self.native = process
        return process

    def bake(self):
        # Exercise real planning/registration and command propagation; only native
        # transport and collision decoding are replaced with synthetic fixtures.
        with patch.object(batch.subprocess, 'check_output', return_value=json.dumps(self.profile)), \
             patch.object(batch.subprocess, 'Popen', side_effect=self.run_native), \
             patch('moby_seeds.Ground.read_triangles', return_value=[
                 [[30, 30, 75], [50, 30, 75], [30, 70, 75]]]):
            return batch.bake_level(self.level, self.output, Path('fake-baker'), options=['--rays', '16'])

    def test_complete_bake_plans_geometry_and_registers_verified_regions(self):
        sparse = self.bake()
        plan = json.loads((self.output/'seed-plan.json').read_text())
        self.assertTrue(plan['regions'])
        self.assertEqual(plan['authored_count'], 1)
        self.assertEqual(plan['moby_seeds'][0]['class'], 500)
        self.assertTrue(any(item['changed'] for item in plan['geometry_coverage']))
        self.assertEqual(plan['emission']['emission_masks'], 'none 15 green 5')
        result = json.loads((self.output/'bake-result.json').read_text())
        self.assertTrue(result['verified'])
        self.assertEqual(result['probe_format_version'], 3)
        self.assertEqual(result['bake_version'], 1)
        registry = json.loads(Path(str(sparse)+'.regions.json').read_text())
        self.assertEqual(registry['regions'], result['regions'])
        self.assertEqual(len(registry['regions']), len(plan['regions']))
        self.assertEqual(len(self.calls), 1)
        self.assertIn('--batch-output', self.calls[0])
        self.assertEqual(len(self.sent_plan['regions']), len(plan['regions']))
        self.assertEqual(self.sent_plan['scene_hash'], '42')
        self.assertTrue(all(len(region['spacing']) == 3 and region['max_distance'] > 0
                            for region in self.sent_plan['regions']))
        self.assertTrue(all('--rays' in call for call in self.calls))
        self.assertTrue(self.native.stdin.closed and self.native.stdout.closed)

    def test_failed_dense_verification_never_registers_partial_batch(self):
        self.failure = True
        with self.assertRaises(subprocess.CalledProcessError):
            self.bake()
        self.assertTrue((self.output/'seed-plan.json').exists())
        self.assertFalse((self.output/'regions.json').exists())
        self.assertFalse((self.output/'sparse.orpg.regions.json').exists())
        self.assertFalse((self.output/'bake-result.json').exists())

    def test_planning_failure_terminates_the_prepared_native_process(self):
        with patch.object(seeds, 'plan_level', side_effect=ValueError('invalid seed plan')):
            with self.assertRaisesRegex(ValueError, 'invalid seed plan'):
                self.bake()
        self.assertTrue(self.native.terminated)
        self.assertTrue(self.native.stdin.closed and self.native.stdout.closed)
        self.assertFalse((self.output/'sparse.orpg.regions.json').exists())

    def test_cli_forwards_thread_limit(self):
        with patch('sys.argv', ['bake_level.py', '--level-dir', str(self.level),
                               '--output-dir', str(self.output), '--threads', '2']), \
             patch.object(batch, 'bake_level') as bake:
            batch.main()
        self.assertEqual(bake.call_args.kwargs['options'], ['--threads', '2'])

    def test_source_change_never_registers(self):
        self.changed = True
        with self.assertRaises(subprocess.CalledProcessError):
            self.bake()
        self.assertFalse((self.output/'sparse.orpg.regions.json').exists())

    def test_sparse_only_level_does_not_require_gameplay(self):
        (self.level/'probe-regions.json').write_text('{"version":1,"enabled":false}')
        (self.level/'bake-inputs/gameplay.bin').unlink()
        sparse = self.bake()
        self.assertEqual(json.loads(Path(str(sparse)+'.regions.json').read_text()), {'regions': []})
        self.assertEqual(self.sent_plan['regions'], [])

    def test_existing_outputs_and_recursive_sources_rejected(self):
        self.output.mkdir()
        (self.output/'keep').write_text('previous experiment')
        with self.assertRaisesRegex(ValueError, 'new output directory'):
            self.bake()
        self.assertEqual((self.output/'keep').read_text(), 'previous experiment')
        self.output = self.level/'recursive-output'
        with self.assertRaisesRegex(ValueError, 'outside the source'):
            self.bake()

    def test_missing_gameplay_fails_before_baking(self):
        manifest = {key: value for key, value in self.manifest.items() if key != 'bake_inputs'}
        with self.assertRaisesRegex(ValueError, 're-extract'):
            seeds.read_gameplay(self.level, manifest)
        manifest['bake_inputs'] = {'gameplay': 'unknown'}
        with self.assertRaisesRegex(ValueError, 'unsupported'):
            seeds.read_gameplay(self.level, manifest)

    def test_invalid_level_planning_settings(self):
        for fields in ({'max_regions': 129}, {'max_probes': 32}, {'enabled': 1},
                       {'geometry_max_rise': 0}, {'padding': float('nan')}, {'typo': 1}):
            (self.level/'probe-regions.json').write_text(json.dumps({'version': 1, **fields}))
            with self.assertRaises(ValueError):
                batch.RegionSettings.load(self.level)
