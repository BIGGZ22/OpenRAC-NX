# ps2-oracle

Run one of a PS2 game's own functions on inputs you choose, with no emulator: a small R5900 (Emotion Engine)
interpreter that loads the game's code from **your own disc image** and calls a function until it returns. A
test can then compare a re-implementation against the original over ten thousand inputs in a second or two,
bit for bit.

```rust
use ps2_oracle::{Disc, Ee};
let mut disc = Disc::iso("game.iso".as_ref())?;
disc.check_serial("SLUS-20000")?;                  // the build your addresses belong to
let (boot, _) = disc.boot2()?;
let mut ee = Ee::from_program(32 << 20, &disc.read(&boot)?, &[])?;
ee.call(0x0012_3450, &[], &[1.5, 2.0], 100_000);   // address from your notes, never from source
assert_eq!(ee.f0().to_bits(), my_port(1.5, 2.0).to_bits());
```

- **What it proves:** pure functions of their arguments and RAM — maths, table lookups, AI decisions, RNGs,
  decoders. Load a RAM image from a running game (`Ee::from_ram_file`) for functions that read globals.
- **What it cannot:** timing and frame flow, general DMA, VU1, the GS, the IOP. Record the real machine for
  those. VU0 micro programs do run (`vcallms`, with the pipeline timing their results depend on), uploaded by
  replaying the game's own VIF0 DMA chain (`Ee::vif0_dma`).
- **Floats** go through [ps2-float](https://github.com/FIZL0/ps2-float), the same model the port uses, so a
  mismatch is always a difference in logic, never in rounding.
- **Unknown instructions panic** with the word and its address: teach it to `src/ee.rs` (with a test in
  `tests/interpret.rs`), or hook the function (`Ee::hooks`).
- **No game knowledge and no game data:** it takes bytes, paths and load addresses. Which disc, which overlays
  and the function addresses stay in the project that uses it.

```toml
[dev-dependencies]
ps2-oracle = { git = "https://github.com/FIZL0/ps2-oracle", tag = "v0.1.1" }
```

Used by [opentennis](https://github.com/FIZL0/opentennis) (`hst-oracle`) and the decomp kits
([linux](https://github.com/FIZL0/ps2-decomp-kit-linux), [windows](https://github.com/FIZL0/ps2-decomp-kit-windows),
`crates/kit-oracle`), which add their game's disc layout and address list on top.

## Changing it

This library is ours: when a port needs it (a new instruction, a missing helper, a wrong edge case), fix or
extend it here with a test, tag a release (`vX.Y.Z`, bump `Cargo.toml`), and bump the tag in every dependent.
Never patch a vendored copy.

## License

MIT.
