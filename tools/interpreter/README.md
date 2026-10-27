# Test-only interpreter

This is a test-only interpreter used solely by tooling (the backfill gate,
`tools/fcheck_gate.py`) to compare candidate C against retail on captured
calls. It is never built by the port's or the launcher's builds, never shipped,
never bundled, and never used to play the game. The native port does not
emulate.

It is a standalone CMake project:

```sh
cmake -S tools/interpreter -B build/interpreter -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build/interpreter -j 12
ctest --test-dir build/interpreter
```

Programs: `openrac-boot` (headless run of a retail disc, `--capture` writes
call states) and `openrac-fcheck` (runs retail and a candidate from a state and
compares). Restored from acb726d5 (the fcheck side line); see
docs/port/BACKFILL.md.
