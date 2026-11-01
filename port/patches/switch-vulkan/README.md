# Switch Vulkan patches (2026-10-28 hardware results)

All three patches were applied to the working tree on 2026-10-28 (do not apply them again).
To undo one: `git apply -R port/patches/switch-vulkan/<name>.patch` (reverse order).

| Patch | What it does |
|---|---|
| 01-diagnostics | Reads `sdmc:/switch/openrac/env.txt` (KEY=VALUE) into the environment at startup and logs it to `env-applied.log`; adds memory types, bindless state, timestamps and present modes to `vulkan-startup.log`. |
| 02-lazy-frame-reclaim | Backport of upstream 9906aa6e: `advance()` no longer waits for the frame slot's fence at the end of present; the wait happens at the next frame's first GL use (`prepare_frame`), so simulation overlaps GPU work. |
| 03-switch-texture-safety | Switch default: bindless material arrays off (`OPENRAC_SWITCH_BINDLESS=1` re-enables); upstream's fix keeping compute-written instance buffers on their device copy. |

Test runs: copy one `env-*.txt` to `/switch/openrac/env.txt`, go to the same Novalis spot,
stay ~60 s, quit normally. Collect `vulkan-startup.log`, `env-applied.log`,
`data/openrac-rac1-pal.log` and `vulkan-runtime.log` if present. Delete `env.txt` afterwards.
