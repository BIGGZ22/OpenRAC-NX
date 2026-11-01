# Switch effects and render resolution

## Effect rendering

The OpenGL particle-extraction path now uses the same per-pass texture-address
reuse as Vulkan. Its cache is discarded each extraction so animation at the same
RAM address is still detected next tick. Rotation uses a 256-entry sin/cos lookup
for the particle's byte-sized angle, and depth-sort scratch capacity is reused.
All rotations are tested against the previous formula; a 10,000-particle shared
texture test resolves the texture once. This is work-count evidence, not a measured
Switch FPS improvement. Save diagnostics are now opt-in (`OPENRAC_MC_TRACE=1`)
since the user confirmed saves work with the correct test NRO.

The existing EffectRenderer already expands strips/quads into a shared vertex buffer and batches consecutive draws with identical texture and blend state. Keep that ordering: sorting transparent particles by texture can change their appearance. Collectible simulation, rewards and collision are unchanged.

The changes extend texture-upload reuse to OpenGL. Calling upload_cached(const ImageUpload&) before the by-value upload avoids copying unchanged image and palette vectors on cache hits. The cache compares both the texture metadata and bytes, and retains upload ordering.

CPU vertex storage grows geometrically and survives between frames. The GPU stream keeps a high-water capacity and orphans its old store before uploading the current frame:

```cpp
glBufferData(GL_ARRAY_BUFFER, capacity, nullptr, GL_STREAM_DRAW);
glBufferSubData(GL_ARRAY_BUFFER, 0, bytes, vertices.data());
```

This lets a driver provide fresh storage while previous draws use the old store. It is a candidate optimization, not a guaranteed speedup on every driver. Existing compatible-state draw batching remains intact. The Vulkan reference upload probe still disables upload reuse for comparisons on either backend.

Reference: [Khronos OpenGL buffer streaming](https://wikis.khronos.org/opengl/Buffer_Object_Streaming).

Track `effects vertices`, `effects vertex upload bytes`, `effects uploads reused` and `effects upload bytes reused` alongside existing draw-call/frame statistics. Test the same crate burst and enemy-heavy scene at 480p and 720p, including frame-time spikes. Lower resolution mainly helps GPU fill cost; game simulation and skinning can still dominate bolt-heavy scenes. Do not claim a frame-rate improvement until hardware measurements confirm it.

Full instancing for arbitrary effect quads would require carrying four independent corners, UVs and colours, and respecting texture-upload boundaries. The shared triangle stream already batches these. Pooling gameplay entities requires separate lifetime/reward validation and is not part of this renderer change.

## Display settings

The Switch Display page replaces Render scale with Render resolution. Presets are Auto, 640x480 (4:3), 854x480 (approximately 16:9), 960x540 (16:9), 1280x720 (16:9), 1024x768 (4:3) and 1920x1080 (16:9). Pixels are square; the ratio describes the image shape, not pixel aspect ratio.

1. Persist the preset in `switch_resolution` (0 means Auto). Old settings without the key remain Auto.
2. A fixed preset supplies both target dimensions and picture aspect, so 4:3 presets are pillarboxed rather than stretched on a wide screen.
3. Choosing Picture shape returns to Auto. Choosing a resolution resets the legacy render scale to 100%.
4. Clamp Auto to at least 480 pixels high and 640 wide after legacy scaling. Fixed presets all meet the floor.
5. Feed the resulting dimensions into the existing framebuffer resize path. Its existing multisample fallback remains active.

These are internal game render targets. They do not change the Switch display/HDMI mode, and rendering 1080p does not make a handheld panel display 1080p. The 480p floor is a project requirement, not a claim that every original PAL framebuffer used those exact dimensions.

## Validation before release

The user has authorized a test NRO build. Independent resolution/profile and overlay-timing tests pass. Verify controller/touch preset selection, saved settings across restart, 4:3 letterboxing, HUD alignment, dock/handheld transitions, invalid settings and effect-heavy scenes on hardware. Compare the new stream path against the previous one for frame time and GPU memory behavior.
