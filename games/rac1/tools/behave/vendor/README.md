# Vendored crates

[ps2-oracle](https://github.com/FIZL0/ps2-oracle) v0.1.3 (3bd5938) and
[ps2-float](https://github.com/FIZL0/ps2-float) v0.3.0 (647eefb), both MIT
(their LICENSE files), by FIZL0. Two changes, both in ps2-oracle:

- `Cargo.toml` takes ps2-float from `../ps2-float` instead of its git tag;
- `src/ee.rs`: `Ee::step` and `Ee::npc` are public, so behave can run its own
  call loop and record which instructions ran (its coverage figure).
- `src/ee.rs`: `Ee::strict`, empty by default: when set, the only RAM ranges
  a program may touch, and every access must be aligned to its size (the
  quadword loads and stores too, which the console rounds down instead).
  behave makes up its state at random; this turns the states the game never
  has (a float used as a pointer) into faults that skip the trial.
- `src/ee.rs`: the MMI lane instructions hand-written VU0 code uses (the
  parallel adds, compares, min/max, extends, packs and shuffles, and the
  parallel shifts, saturating adds), and the VU0 macro ops vmax, vmini,
  vmaddq, vmsubq, vitof, vftoi, vabs and the q accumulator forms, which
  ps2-oracle panics on.
