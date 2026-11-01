# Session30 Status Monitor video — 2026-10-30

Reviewed compressed_25MB_18537.mp4 (21.76 s); extracted full-size frames at10s
and18s into build/deko3d-test/session30-review/.

Hardware-visible reporting now confirmed: Status Monitor1.4.1+r4 shows PFPS,
rolling FPS and resolutions1280x720 |640x480. These agree with the session30
log's640x480 internal frame and1280x720 output; the picture is4:3.

At10s: PFPS18, FPS20.2, GPU89.5% at307.2MHz; CPU1022.2MHz real /1020MHz target,
cores approximately7.06%,0%,39.87%,26.95%. RAM1329.2MHz real /1331.2MHz target.
At18s: PFPS19, FPS16.5, GPU77.8% at307.2MHz; CPU1022.2MHz real /1020MHz target,
cores approximately8.46%,39.35%,0%,26.81%. RAM1329.1MHz real /1331.2MHz target.
These are sampled frames, not a whole-clip min/max or time average. CPU work may
migrate between cores. They do not support claiming CPU saturation in this scene.

Combined with large GPU command spans and slot-fence waits in session30, this
supports a substantial graphics-side bottleneck at the displayed clocks. Command
span is not pure GPU busy time. Next work should measure draw/shader/sampler cost,
not assume CPU-only limitation. Existing logs show roughly1800–2450draws/frame in
heavy stretches and hundreds of sampler-set refreshes; do not disable correctness
workarounds blindly. Do not claim GPU load establishes a specific shader cause.

No code changes or builds from reviewing this video. Canopy behavior remains a
separate visual test; the log confirms activation, not successful attachment.
