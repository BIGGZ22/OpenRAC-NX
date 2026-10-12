# Credits

This file records every outside project or person whose work this repository
uses, and what was used. Credit here is attribution only: it never adds
matching credit, and nothing listed is evidence for a match. The only evidence
is compiler output that compares byte for byte against the pinned executable.

Add a row for every item taken from an outside source, at the time it is taken,
with the source commit and the repository file it informed.

## Promises — RC2-Going-Decompiled

- Project: [Promises/RC2-Going-Decompiled](https://github.com/Promises/RC2-Going-Decompiled)
  (commits are authored under the name Henning Berge)
- Reviewed revision: `855deaff0eed040190198f83ed5fa7b871f52efb`
- Target of that project: Going Commando USA v2.00 and EU v1.00, not this
  repository's USA v1.01 target. Its addresses, sources and proofs are leads
  here until re-measured against `config/target.json`.
- Licence: the repository carries no `LICENSE` file at the reviewed revision.
  The initial gap-report PR copied no source. Subsequent tooling contributions
  import the reference helpers below under their upstream GPL-2.0 licence.
- Permission: the maintainer reports that the author agreed to reuse. The
  author's own approval on the pull request that introduced this file is the
  record; the scope and licence of any later adoption are to be stated in the
  row that adopts it.

### What was taken

| Item | Used for | Where |
| --- | --- | --- |
| Reference shell/AWK/sed helpers, once in `tools/ee/` | Experimental reference only; removed when the build was rewritten (2026-10-11) | contributor import history in PR #135; upstream licence revision `f134130e8e979843da090f89109a480b773bc2e3` |
| v2.00/EU symbol maps and derived name hints | Exploratory naming with region-specific addresses | `symbol_addrs/` and `config/datamine/`; naming adds no match credit |
| Gap comparison of their public tree against this repository | Listing leads we do not yet have | docs/PROMISES-GAP-REPORT.md |

Anything adopted later from this project is added to the table above, with the
source file and commit, in the same change that adopts it.

## Made in Slovakia — Ratchet & Clank PCSX2 cheat codes

[Made-in-Slovakia/rac](https://github.com/Made-in-Slovakia/rac) is credited by
the Promises project for its first global-naming anchors (v2.00 addresses).
It is listed here only because the Promises leads below depend on it. Nothing
from it is used in this repository.

## Lombyte (the NTSC decomp)

- The patch stack for the game compiler (`tools/compiler/patches/`, `sce-991111b`, MIT) and the
  recipe for the SDK compiler (`tools/compiler/sdk/build.py`, adapted from
  `scripts/build-patched-toolchain.py`, MIT, Mateusz Kłysz).
