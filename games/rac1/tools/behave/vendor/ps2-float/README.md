# ps2-float

PlayStation 2 arithmetic, bit for bit, for re-implementations of PS2 games that have to match the original
exactly:

| Module | Model |
|---|---|
| `fpu` | EE COP1 single precision: chop rounding, the one-guard-bit adder, no Inf/NaN/denormals, `madd.s` ≠ FMA; the `F` type whose `+ − × ÷` are the FPU's |
| `vu` | VU0 macro mode: full-width adder, chop everywhere (`vsqrt` included), clamping; `Vf` vectors, `Lane` scalars |
| `libm` | fdlibm's `sinf cosf tanf asinf acosf atanf atan2f expf logf powf sqrtf` on EE arithmetic; `libm::sce` the Sony SDK build's `acosf atan2f powf` |
| `rand` | newlib's `rand()` and MT19937 |

`#![no_std]`, `#![forbid(unsafe_code)]`, no dependencies (`serde` derives for `rand` behind the optional
`serde` feature). Results are identical on x86_64, aarch64 and wasm32: integer ops and the four basic IEEE
operations only, never the host's libm.

```toml
[dependencies]
ps2-float = { git = "https://github.com/FIZL0/ps2-float", tag = "v0.2.0" }
```

The crate docs (`cargo doc --open`) explain each deviation from IEEE 754 and what it looks like in a port
that gets it wrong.

## Where it is proven

The models are properties here (`cargo test`), and proven against real code elsewhere:

- [opentennis](https://github.com/FIZL0/opentennis) (Hot Shots Tennis, SCUS-97610): the FPU model against PCSX2
  recordings (2912 shot-table results; whole matches replayed bit-exact), `libm::sce` and `rand` against the
  game's own functions over ≥10k inputs each with [ps2-oracle](https://github.com/FIZL0/ps2-oracle).
- [ps2-decomp-kit-linux](https://github.com/FIZL0/ps2-decomp-kit-linux) /
  [-windows](https://github.com/FIZL0/ps2-decomp-kit-windows): the workbench that uses both crates for any game.

## Changing it

These libraries are ours: when a port needs it (a new op, a missing function, a wrong rounding, another
SDK's libm flavour), fix or extend it here, with a test, then tag a release (`vX.Y.Z`, bump `Cargo.toml`)
and bump the tag in every dependent (opentennis, both kits, ps2-oracle). Never patch a vendored copy.

## License

MIT. `libm` is a port of fdlibm (Copyright (C) 1993 Sun Microsystems, freely redistributable with its
notice, kept in `src/libm.rs`).
