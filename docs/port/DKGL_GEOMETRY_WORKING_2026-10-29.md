# First working-geometry report, October 29

User tested the latest Claude-change build and explicitly reports geometry now
works. Clips at D:/Downloads/2026102910224300-03DB12780BD84000.mp4,
2026102910232200-03DB12780BD84000.mp4 and
2026102910242400-03DB12780BD84000.mp4 show the title, ship, Novalis and shop.
Sampled frames show coherent ship/scenery geometry, in contrast to the previous
broken surfaces. User still reports the left Pyrocitor preview lacks its texture,
while the rest of the shop renders correctly. Landing-scene audio is delayed.
These two issues remain open; no cause or fix is established in this review.

Session log (25), driver (12), startup (14), NRO/ELF/manifest and counter summary
are saved in build/deko3d-test/diagnostics/2026-10-29-geometry-working/.
Review frames: build/deko3d-test/clip-review-working-geometry/.
No code or new game build was made for this review. Preserve this reference
before changing rendering behavior. Multiple renderer changes were enabled;
this run does not isolate which one corrected the geometry.

## Evidence

- Correct patched shader pack, texture-invalidation and sampler-rebind markers.
- All 21 GPU checks pass; six real-texture mip checks pass.
- Frame source is 1280x720. Missing program counter remains zero.
- FRAME_COST is absent: no `cost tick` reports. GPU-forcing flushes drop to
  approximately zero. There are no usable CPU/GPU millisecond figures in this
  log. Do not infer them from draw counts or the 30fps recording container.
- 44 counter windows total. Of 34 windows with >=400 draws (a rough selection,
  not a pure isolated Novalis benchmark): mean881 draws, mean1922 sampler-set
  rebinds, mean2.09 Full barriers and mean4.44 texture-cache invalidations/frame.
  Draw range484..1371; sampler-rebind range949.5..3093.5. High rebind count is a
  candidate CPU-overhead source, not proof of the limiting CPU/GPU stage.

## Next work

1. Nonblocking timings: CPU recording durations already exist in frontend
   FramePresentation; add a separate reporting mode that does not call glFinish.
   For real GPU time, use deko3d timestamp reports and read slots only after their
   existing fences signal. Avoid silently re-enabling FRAME_COST for normal FPS.
   Night100 has an implementation pattern in the locally saved backend source.
2. Sampler rebind batching: prepare_draw currently processes samplers one at a
   time and clears its read set when rebinding. This can cause >1 rebind/draw as
   samplers alternate. Consider gathering handles first, binding the set once
   if any sampler is fresh, then marking all samplers used. Retain correctness
   workaround and compare against this working build; do not just disable it.
3. Pyrocitor: trace target-moby model/material/image selection through
   frontend/scene_hud.cpp draw_target_mobys -> LevelScene::draw_mobys/draw_layer
   and TexturePool binding. Determine whether a white fallback, authored material,
   missing model texture or offscreen sampling is responsible. Other shop panels
   now work; do not assume the entire framebuffer path is broken.
4. Landing audio: distinguish delayed stream start, audio/video drift and level
   load blocking. use_level synchronously loads/uploads world assets, including
   flight world98; startup audits/readbacks may also stall. Those are hypotheses,
   not a measured cause. Capture audio-start and load/frame timestamps and compare
   stable OpenGL before changing queue depth or applying a guessed audio offset.
