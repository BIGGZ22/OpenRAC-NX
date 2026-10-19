# Boot C promotion protocol

`nonmatching/boot/` contains useful authored C trials, including bodies in a
byte-exact combined boot build. A passing combined build **does not** promote
every body: the rest of the boot may come from the retail assembly baseline,
and a linked symbol's compiled extent alone is not a reviewed retail function
boundary. `src/boot/` and the accepted progress count are reserved for reviewed
placements.

Run the [complete boot gate](BOOT-TRIAL-GATE.md) with the pinned lawful input and
private tools. It recompiles every C source, rejects instruction assembly even
after preprocessing, verifies each pinned object, and requires the entire boot
ELF to match. A successful private receipt now includes
`promotion_input_sha256`, a fingerprint of the source files, replacement owners,
compiler profile and generated body catalogue. A changed input makes the old
receipt unusable for promotion triage.

Then run:

```sh
python tools/boot_promotion_audit.py --write --check --require-gate
python tools/boot_promotion_audit.py --candidate memcard_get_widescreen
```

The metadata-only result is ignored at `build/boot-promotion-audit/report.json`.
It lists every campaign owner, its linked symbols, byte extent and reasons for
its status:

- `blocked`: no usable catalogued body, a broken source pin/policy, or invalid
  ownership evidence. Fix the cause and rerun the complete gate.
- `needs_review_or_gate`: the full gate is missing/stale, or unverified padding,
  multiple bodies or control flow require closer boundary review.
- `ready_for_semantic_review`: current exact gate, pinned pure C source, one
  catalogued body starting at its owner, a normal return, and either no trailing
  gap or one verified zero word supplied by the linker's `ALIGN(8)`. The
  generated catalogue checks that this gap is zero in retail and has no direct
  branch, jump or named code-label target. This is a **shortlist**, not accepted
  progress; indirect targets, aliases, behavior and the real retail function
  boundary still need review.
- `accepted_needs_gate` / `accepted`: a source already in the explicit accepted
  set; the former means this checkout lacks a current exact gate receipt.

For each shortlisted body, review its complete retail function boundary,
branches into and out of that range, aliases, linker symbol, callers, prototype,
data sizes and original behavior. A `jr $ra` and an exact compiled span do not
prove these facts. Record the evidence in the PR. If it survives review, move
the one source definition to `src/boot/`, add it to the explicit accepted set and
gate/profile scope, regenerate progress, then rerun the complete private gate at
the **final commit**. Count only the compiled `body_bytes`, never an alignment
word. The entire owner interval must still be rebuilt exactly without assembly
fallback. CI checks public metadata and source
policy, but cannot replace the private gate or semantic review.

The audit never moves sources, changes accepted counts or publishes private
bytes. Any span without a global-function extent needs a separate boundary
investigation before it can become a review candidate.
