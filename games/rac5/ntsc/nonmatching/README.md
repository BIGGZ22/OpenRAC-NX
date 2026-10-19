# Nonmatching C

Keep useful C/C++ hypotheses here until the compiler and complete-program byte
gate accept them. Some local boot slices now reproduce their reference bytes
(`docs/COMPILER-WITNESSES.md`), but their placement and acceptance reviews
are not complete. These candidates do not count toward progress.
The accepted boot-only `MEMCARD_Busy__Fv` definition now lives in `src/boot/`.
All other boot witnesses remain provisional here. Instruction assembly, raw encoded
instructions and fixed-register bindings are excluded from public C attempts and
from the compiler witness campaigns. Preserve such experiments privately; leave
their retail intervals in the assembly baseline without C credit. Symbol-only
declaration labels for retail ELF names are allowed and emit no instructions.
The two short level C controls under `nonmatching/levels/` have exact linked
bodies, but they are not accepted matches; see
[`docs/LEVEL-C-WITNESS.md`](../docs/LEVEL-C-WITNESS.md).

Name a primary attempt after its reference symbol (or a measured address when no
symbol exists). Keep distinct alternatives under the same area's `alternatives/`
directory, with the function name and a form suffix; preserve the reference symbol
in the definition or its symbol-only declaration label. Do not overwrite an older
useful form silently. Record the program/range, compiler profile, first difference,
attempted shapes and what would justify retrying. An exact but unaccepted witness
instead states the missing boundary, scope or qualification review.

After three attempts with the same mismatch class, change the hypothesis or park
the source. Raw reports, synthetic probes and scratch harnesses stay private.
