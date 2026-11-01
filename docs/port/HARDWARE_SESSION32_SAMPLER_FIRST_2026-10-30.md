# Session32: first sampler experiment confirmed

Input video compressed_25MB_18546.mp4 is the NEW test. The earlier18541video
belongs to session31 baseline, per user correction. Do not mislabel it as first.
Session32 logs archived at build/deko3d-test/sampler-first-actual-review/logs/.

Env and game startup both explicitly confirm SAMPLER_REBIND=first. Internal640x480,
output1280x720. FRAME_COST absent, lazy barriers0, other baseline switches retained.
Overlay attaches and publishes. Phone-video samples show normal geometry and no
obvious sampler corruption; sampled images are not exhaustive visual validation.
Selftest lines report PASS. Startup log lists SDK0.5 without a new fatal error.

Mechanism works: sampler refreshes hold near7/frame in gameplay instead of hundreds.
Representative heavy samples:
 baseline31:2164draws/frame,633.4refreshes/frame, GPU span62.009ms,
            present66.648ms, record14.834ms.
 first32:   2214draws/frame,7.0refreshes/frame, GPU span50.569ms,
            present64.112ms (about15.6FPS).
These are approximately similar workloads, not identical-camera A/B. Refreshes
are98.9% lower in those samples, command span18.4% lower, presentation interval
only3.8% lower. Do NOT claim18% FPS improvement. Command span is not GPU busy time.
Other session32 views span~28–44ms with presentation~33–54ms; heavy rotation still
runs slowly. Cannot infer all residual cost is shader execution rather than
submission, synchronization or pacing. Need per-pass timing/copy attribution next.

Keep first an opt-in experiment until a longer route, reloads, vendor panels,
wrap/filter transitions and other levels are checked. Do not promote to default
from this short visual sample. No code changes/build in this review.
