# Reuse: what to take from the sibling projects

The three OpenRAC projects decompile the same engine family. This page says
what is worth carrying over, what is target-specific, and where the authentic
source forms live.

| Project | Target | What it is good for |
| --- | --- | --- |
| `OpenRAC/rac1-decomp` | Ratchet & Clank (PS2) | the oldest, the closest to the engine's original C sources; the richest "authentic form" reference |
| `OpenRAC/rac2-gc-decomp` | Ratchet & Clank 2 (GameCube) | the most complete campaign: gates, promotion protocol, shared-code audits |
| `OpenRAC/rac5-sm-decomp` | Ratchet & Clank: Size Matters (PS2) | the SNR2 module format, the level-body gate, the wave protocol this project starts from |

All three are GPL-3.0. Reusing their code here means keeping the copyright,
the license and the exception notices, and describing the local adaptation —
see `LICENSE` and `THIRD_PARTY_NOTICES.md`.

## Method: reusable as is

- the **evidence standard** (`docs/EVIDENCE.md` here) and the ten working
  rules (`AGENTS.md` here);
- the **wave protocol** (`docs/METHOD.md` here): foundation lot, shortlist
  ranked by leverage, one agent per lot, proof by byte comparison, root
  integration with complete-program gates;
- the **promotion protocol**: a body becomes a match only when it is linked at
  every reviewed placement and the complete-program gates pass;
- the **parking rule**: three attempts in the same mismatch class, then park
  with the first difference and a reopening condition.

## Tooling: reusable with adaptation

| Tool | From | Adaptation needed |
| --- | --- | --- |
| per-body harness (`levels_try.py` shape) | rac5 | the module reader, the symbol resolution rules and the linker script are target-specific; the *shape* — compile, link at the offset, compare every byte — is not |
| wave lot runner | rac5 | the path of the lots, the result table columns |
| level body gate | rac5 | the module format and the relocation model |
| inventory / boundary audit | rac5, rac2 | the format of the program |
| ByteMatch-style estimate | rac2, rac5 | the denominator; never publish it as a rate |

## Authentic source forms: the most valuable thing to reuse

The engine's maths and utility routines are shared between the games, and
their **original source form** is what reproduces the retail register
allocation. The sibling projects have already recovered several of them, and
their files are the reference to read before reconstructing a routine by hand.

The pattern to look for, when a body will not compile to the retail bytes:

- the original keeps vectors in **arrays in memory** and calls helpers,
  instead of juggling floating-point temporaries;
- the original names the **intermediate products explicitly**, in an order
  that fixes the register assignment;
- the original isolates a **block of its own** where the retail code keeps a
  value alive longer than a naive translation would.

Read the sibling's authored C (not its assembly stubs) and reproduce the
structure, not just the arithmetic.

## What must not be reused

- retail bytes, extracted modules, instruction listings, disassembly;
- SDK sources, headers and samples, and any leaked or NDA source;
- another project's private workspace, its gate receipts and its raw reports.

The sibling repositories are public and their **written** source is reusable
with attribution. Their local workspaces are private and stay private.
