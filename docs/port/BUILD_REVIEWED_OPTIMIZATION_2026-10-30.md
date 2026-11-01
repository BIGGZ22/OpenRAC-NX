# Reviewed optimization build — 2026-10-30

Built both Deko3D targets after reading Claude's review. Both inner build logs
report exit code 0 and the completed openrac-rac1-pal_nro target. Copied NRO/ELF
hashes match build outputs; patched and stock shader packs match staged RomFS.
No shader pack regenerated, no commit/push. Hardware testing remains pending.

Performance: build/deko3d-release/openrac-rac1-pal-deko3d.nro
SHA256 54184009d5ccf59b45c9d8ba0013726f3e24c991bf7488b127f129374747acb3
ELF build ID 3a507cc6654328f3700f8a5a6f30e2a0dbfd4ffc

Diagnostic: build/deko3d-test/openrac-rac1-pal-deko3d.nro
SHA256 64be546a1a1a0b0e20f8595ee22f23255f237ffa056490169a918b1d3af63312
ELF build ID 669b04f54f2a95d5abeea28a695668e883cca5c7

Each output folder has the matching last-build.elf and updated last-build.json.
Previous artifacts preserved at build/deko3d-test/diagnostics/2026-10-30-before-reviewed-optimization-build/.

Optional measured baseline: build/deko3d-release/env-reviewed-baseline.txt.
Use as a guide for SD env.txt; remove FRAME_COST and PALETTE_COPY_REFERENCE.
First test explicit480p, stock clocks, same FPS cap; Metropolis rotation, close
ship flyby and landing. Lazy barriers remain off. Collect game and overlay-status
logs. Confirm the flight-at-game-tick startup line. Second flight run only:
OPENRAC_FLIGHT_POSE_AT_TICK=0, restart; compare attachment and smoothness.

See CLAUDE_REVIEW_OPTIMIZATION_2026-10-29.md for exact source review and limitations.
