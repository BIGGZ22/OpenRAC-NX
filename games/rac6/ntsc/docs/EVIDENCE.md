# Evidence: what counts, and what does not

Every number this project publishes must be traceable to a measurement that
someone else can repeat. This page defines the vocabulary of proof.

## The only proof

> The bytes produced by the qualified toolchain for an authored source are
> **identical** to the retail bytes, at the placement, and the complete
> program that contains them still hashes to the pinned retail hash.

Two halves, both required:

1. **Body level** — the compiled body equals the retail body byte for byte.
2. **Program level** — the complete program, with the body in place, equals
   the retail program.

A body-level proof alone proves the instruction sequence. It does not prove
that the body is placed correctly, that its data references resolve the same
way, or that the rest of the program is untouched.

## Accepted as evidence

| Evidence | Proves | Does not prove |
| --- | --- | --- |
| byte-identical body at a placement | the instruction sequence and its encoding | the placement, the surroundings |
| complete-program hash after replacement | the placement, the data, the surroundings | that the source is the original one |
| a negative control (a wrong body is refused) | that the harness can say no | anything about the accepted body |
| a measured cause for a compiler option | why the option is used | that the option is correct in general |
| a non-regression run | that a change broke nothing | that the change helped elsewhere |

## Not accepted as evidence

- **A file that compiles.** Compilation is not correspondence.
- **A few matching instructions.** A prefix is not a function.
- **A masked comparison.** Relocation-masked bytes can be equal while the
  programs differ.
- **An assembly fallback.** It preserves the baseline; it earns no C credit.
- **A hand-written instruction.** It needs separate evidence and an explicit
  maintainer classification; it is never automatically accepted as C.
- **A score, a percentage, a dashboard.** A percentage computed on a
  provisional denominator is a scenario, not a measurement. It is published as
  such, or not at all.
- **A shared body assumed to work elsewhere.** Every placement is proved on
  its own bytes.

## Counts

Three different numbers must never be confused:

- **placements** — how many addresses a proved body covers;
- **sources** — how many authored files carry them (one source, many
  placements);
- **bytes** — how many retail bytes the proved bodies cover.

A family of eight-byte accessors can add thousands of placements and very
few bytes. Both facts are true; report them separately.

## Scopes

Each scope (the boot, the frontend, a family of level bodies) is counted on
its own, with its own gate and its own numbers. Scopes are not added
together when they overlap, and a scope never inherits the credit of another.

## Provisional and parked

Not everything is counted, and that is fine:

- **provisional** — measured, not yet promoted: it lives in the private
  workspace, not in the published numbers;
- **parked** — measured, not matching: it is stored with its first
  difference and a reopening condition;
- **control** — a shape compiled to prove the harness can refuse, counted
  nowhere.

The published numbers must never include a provisional or a parked body.
