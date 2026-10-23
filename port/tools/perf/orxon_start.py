"""Repeatable RAC1 NTSC Orxon starting-view performance capture.

Uses the game's normal ship exit and camera. Outputs stay local under build/.
Run serially with other game/render workloads closed; see docs/port/DEBUGGING.md.
"""
import argparse
from datetime import datetime
import json
import os
from pathlib import Path
import statistics
import subprocess
import sys

# Share the regression suite's verified NTSC inventory layout and item IDs.
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'regress'))
import routes


def starting_inventory():
    words = routes.writes_of([{'at': 200, 'give': 'all'}], 'rac1-ntsc', routes.load())
    # Orxon chooses Ratchet/Clank while loading, before the control marker.
    # Keep O2 Mask ownership present through that initialization, with full gear.
    return ','.join(f'{at}:{address:x}:{value & 0xffffffff:x}:1200'
                    for at, address, value, _ in words)


def lines(path):
    return [json.loads(line) for line in path.read_text().splitlines() if line.strip()]


def analyse(directory, warmup, samples, gpu_driven=None, gpu_materials=None):
    trace = {t['f']: t for t in lines(directory / 'trace.jsonl')
             if t.get('c') is not None and warmup <= t['c'] < warmup + samples}
    if len(trace) != samples or any(t['lv'] != 10 or t['m'] != 0 or t['hs'] != 0
                                     for t in trace.values()):
        raise RuntimeError('Expected complete Orxon sample with Ratchet idle after the ship exit')
    anchor = next(iter(trace.values()))
    for field in ('hp', 'cam', 'fw'):
        if any(abs(t[field][axis] - anchor[field][axis]) > .01
               for t in trace.values() for axis in range(3)):
            raise RuntimeError(f'Starting view moved during sample: {field}')
    frames = [f for f in lines(directory / 'frames.jsonl')
              if f['kind'] == 'tick' and f['tick'] in trace]
    if len(frames) != samples:
        raise RuntimeError('Incomplete timing sample')
    log = (directory / 'stdout.log').read_text(errors='replace')
    if ('scene/settings verified' not in log or '[error]' in log
            or 'probe GI inactive for level 10' in log):
        raise RuntimeError('GI activation or game log check failed')
    gpu_frames = [f for f in frames if f['gpu_ms'] is not None]
    if len(gpu_frames) < samples * .9:
        raise RuntimeError('Fewer than 90% of asynchronous GPU timings are available')
    for f in frames:
        if f['gauges'].get('world depth reused') != 1 \
                or f['gauges'].get('pvs enabled') != 1:
            raise RuntimeError('Expected Vulkan GPU timings, depth reuse and PVS')
        if gpu_driven is not None and f['gauges'].get('gpu driven mesh', 0) != int(gpu_driven):
            raise RuntimeError('GPU-driven/reference path did not match the requested benchmark')
        if gpu_materials is not None and (f['gauges'].get('vk_material_draws', 0) > 0) != gpu_materials:
            raise RuntimeError('GPU material/reference path did not match the requested benchmark')
    summary = {'frames': samples, 'control_frame': anchor['f'] - anchor['c'],
               'view': {k: anchor[k] for k in ('hp', 'hr', 'hs', 'cam', 'fw')},
               'frame_ms_mean': statistics.mean(f['frame_ms'] for f in frames),
               'frame_ms_median': statistics.median(f['frame_ms'] for f in frames),
               'gpu_samples': len(gpu_frames),
               'gpu_ms_mean': statistics.mean(f['gpu_ms'] for f in gpu_frames),
               'draws_mean': statistics.mean(f['draw_calls'] for f in frames),
               'triangles_mean': statistics.mean(f['triangles'] for f in frames)}
    # Immediate renderer counters are candidate counts on the GPU path.
    # Completed GPU geometry counters lag by frame slots and are labelled separately.
    names = ('vk_draw_api_calls','vk_gpu_indirect_commands','vk_gpu_indirect_calls',
             'vk_gpu_indirect_merged','vk_gpu_instance_candidates',
             'vk_gpu_completed_draws','vk_gpu_completed_triangles','vk_draw_ms','vk_wait_ms',
             'vk_material_draws','vk_material_records','vk_material_textures',
             'vk_uniform_uploads','vk_uniform_upload_kb','vk_descriptor_sets','vk_descriptor_reuses')
    summary['gauges'] = {name: statistics.mean(f['gauges'].get(name,0) for f in frames)
                         for name in names}
    names = sorted({s['name'] for f in frames for s in f['sections']})
    summary['sections'] = {}
    for name in names:
        metrics = {}
        for metric in ('cpu_ms', 'gpu_ms', 'draw_calls', 'triangles'):
            values = [sum(s.get(metric, 0) or 0 for s in f['sections'] if s['name'] == name)
                      for f in frames if metric != 'gpu_ms'
                      or any(s['name'] == name and s.get(metric) is not None for s in f['sections'])]
            metrics[metric] = statistics.mean(values) if values else None
        summary['sections'][name] = metrics
    return summary


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('exe', 'data', 'levels', 'probes'):
        parser.add_argument('--' + name, type=Path, required=True)
    parser.add_argument('--out', type=Path, default=Path('build/perf') /
                        ('orxon-start-' + datetime.now().strftime('%Y%m%d-%H%M%S')))
    parser.add_argument('--runs', type=int, default=2)
    parser.add_argument('--warmup', type=int, default=300)
    parser.add_argument('--samples', type=int, default=300)
    parser.add_argument('--cpu-reference', action='store_true',
                        help='Keep Vulkan but use CPU culling and direct mesh draws')
    parser.add_argument('--material-reference', action='store_true',
                        help='Keep GPU visibility but use individual material uniforms/textures')
    parser.add_argument('--shadow-batch-reference', action='store_true',
                        help='Submit shadow casters in model order instead of grouping compatible draws')
    parser.add_argument('--shadow-atlas-reference', action='store_true',
                        help='Use separate shadow targets while retaining batching across layers')
    parser.add_argument('--shadow-layer-reference', action='store_true',
                        help='Use separate targets and retain the previous per-layer shadow batches')
    parser.add_argument('--opaque-colour-reference', action='store_true',
                        help='Submit opaque colour in the original per-layer order')
    parser.add_argument('--opaque-specialisation-reference', action='store_true',
                        help='Use combined opaque and general static shaders instead of specialised variants')
    parser.add_argument('--sky-intensity', type=float, default=1.5,
                        help='Must match the supplied bake; recorded in run.json')
    args = parser.parse_args()
    if min(args.runs, args.warmup, args.samples) < 1:
        parser.error('runs, warmup and samples must be positive')
    for name in ('exe', 'data', 'levels', 'probes', 'out'):
        setattr(args, name, getattr(args, name).resolve())
    args.out.mkdir(parents=True, exist_ok=False)
    record = {k: str(v) if isinstance(v, Path) else v for k, v in vars(args).items()}
    record.update({'level': 10, 'window': [2256, 1470], 'aspect': '4:3', 'runs_completed': []})
    (args.out / 'run.json').write_text(json.dumps(record, indent=2) + '\n')
    for index in range(args.runs):
        directory = args.out / f'{index + 1:02}'
        directory.mkdir()
        settings = directory / 'settings.ini'
        settings.write_text('display_mode = windowed\nwindow_size = 2256x1470\naspect = 4:3\n'
                            'render_scale = 100\nvsync = off\nframe_rate = unlimited\nvolume = 0\n'
                            'skip_cutscenes = 1\nenhanced_graphics = 1\nenhanced_shadows = 1\n'
                            'probe_lighting = 1\nprobe_gi_multiplier = 1\n'
                            'probe_normal_bias = 0.5\nprobe_view_bias = 0.5\nprobe_spheres = 0\n'
                            'gtao = 1\ngtao_strength = 1\ngtao_slices = 4\ngtao_steps = 8\n'
                            'gtao_half_resolution = 1\ngtao_radius = 1.25\nshow_fps = 0\n')
        env = {k: v for k, v in os.environ.items() if not k.startswith('OPENRAC_')}
        env.update({'OPENRAC_GL_API': 'vulkan', 'OPENRAC_VULKAN_VALIDATION': '0',
                    'OPENRAC_DIRECT': '1', 'OPENRAC_UNCAPPED': '1', 'OPENRAC_NO_BLEND': '1',
                    'OPENRAC_AUDIO_TICKED': '1', 'SDL_AUDIO_DRIVER': 'dummy',
                    'SDL_AUDIODRIVER': 'dummy',
                    'OPENRAC_PRESS': '100:4000:5,k30:8:5,c0:0:10000:128:128:128:128',
                    'OPENRAC_WRITE': starting_inventory(),
                    'OPENRAC_CONTROL_FRAMES': str(args.warmup + args.samples),
                    'OPENRAC_TRACE': str(directory / 'trace.jsonl'),
                    'OPENRAC_PROBE_GI': str(args.probes), 'OPENRAC_PROBE_VIEW': 'combined',
                    'OPENRAC_PROBE_RAYS': '1024',
                    'OPENRAC_PROBE_SKY_INTENSITY': str(args.sky_intensity),
                    'OPENRAC_SUN_SHADOWS': '1', 'OPENRAC_HERO_SHADOW': '1'})
        if args.cpu_reference:
            env['OPENRAC_GPU_DRIVEN_OFF'] = '1'
        if args.material_reference:
            env['OPENRAC_GPU_MATERIALS_OFF'] = '1'
        if args.shadow_batch_reference:
            env['OPENRAC_SHADOW_BATCH_OFF'] = '1'
        if args.shadow_atlas_reference:
            env['OPENRAC_SHADOW_ATLAS_OFF'] = '1'
        if args.shadow_layer_reference:
            env['OPENRAC_SHADOW_LAYER_REFERENCE'] = '1'
        if args.opaque_specialisation_reference:
            env['OPENRAC_OPAQUE_SPECIALISE_OFF'] = '1'
        if args.opaque_colour_reference:
            env['OPENRAC_OPAQUE_COLOUR_BATCH_OFF'] = '1'
        command = [str(args.exe), '--data', str(args.data), '--levels', str(args.levels),
                   '--settings', str(settings), '--window', '--no-card', '--skip-movies',
                   '--level', '10', '--frames', '9000', '--debug-dump', str(directory / 'frames.jsonl')]
        with (directory / 'stdout.log').open('w') as log:
            subprocess.run(command, env=env, cwd=directory, stdin=subprocess.DEVNULL,
                           stdout=log, stderr=subprocess.STDOUT, timeout=240, check=True)
        result = analyse(directory, args.warmup, args.samples, not args.cpu_reference,
                         not args.material_reference)
        if record['runs_completed']:
            previous = record['runs_completed'][0]['view']
            if any(abs(a - b) > .01 for field in ('hp', 'cam', 'fw')
                   for a, b in zip(previous[field], result['view'][field])):
                raise RuntimeError('Starting views differ between repetitions')
        record['runs_completed'].append(result)
        (args.out / 'run.json').write_text(json.dumps(record, indent=2) + '\n')
        print(json.dumps({k: v for k, v in result.items() if k != 'sections'}), flush=True)
    print(args.out, flush=True)


if __name__ == '__main__':
    main()
