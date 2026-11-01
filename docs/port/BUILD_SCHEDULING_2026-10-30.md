# Scheduling candidate build — 2026-10-30

Both release and diagnostic Switch builds completed openrac-rac1-pal_nro with inner exit code 0. Copies match build output SHA256; manifests contain matching readelf build IDs and hardware_tested=false. Both patched and stock source packs match staged RomFS. No shader regeneration was run. Prior builds preserved in build/deko3d-test/diagnostics/2026-10-30-before-scheduling-build/.

Release NRO SHA256: a23e6d182748fa466272ab4975385a9ceaa0919ef429aee3c21bf939805eda3d
Release ELF build ID: 66cc4f1b3efc1a918af70f9fd83aee8e73981470
Diagnostic NRO SHA256: f35610a60473179226cb5ec7fa9e5a8591f4df1b4aa13aca38a83e7c5bacdac6
Diagnostic ELF build ID: a08333cf445da20b025f7d7194a939485230013e

Full Switch builds compile the changed main, worker, interpolation, frame, vsync and sound translation units. Reviewed handoff, worker ownership/placement, finish-before-present flow, preparation path and incremental audio publication. Experiments remain opt-in and require hardware validation. Expected environment and timing strings found in release ELF.

PC checks: direct image_sends test passed. Direct tick_phases default and render_worker tests passed using test-only logging and crash-thread setup stand-ins (build/deko3d-test/scheduling-test-support.cpp). Timing-on test passed accounting/report checks but failed six tight sleep-duration bounds on Windows; do not claim a complete timing-test pass. Full CMake host suite could not compile common/system_info.cpp on this host (os() branch syntax errors); no unrelated source changes made. Claude's sanitizer results were not reproduced here.

Three-run configs are in build/deko3d-release/three-run-tests/. 01 baseline; 02 combines render worker and repeated-image skip; 03 adds early stream start. All inherit session39 terrain/PVS/pacing settings and diagnostics. Combining run02 options sacrifices attribution; split them only if needed. Use same performance NRO for all runs. Preserve session39 reference env.

Direct sound_stream_early test passed off and on with AUDIO_TRACE enabled for the on run, real audio/ADPCM/bank sources plus test-only logging support. Both produced pinned hash 2bd2a9fa9628097b and 497426 sounding samples. No sanitizer run locally.
