# Provisional RAC5 short-loop compiler trial

The existing reconstructed GNU EE `cc1` produces two `nop` instructions
before the backward `bnel` in `FOG_PreUpdate__Fv` at `0x01E7D4A8`. Its linked
body is 68 bytes, against the pinned retail 64-byte interval. Removing the
compiler's short-loop padding pass in an isolated build produced **zero**
`nop` instructions and a 60-byte body. The retail has **one** there.

[`0001-rac5-short-loop-six.patch`](../tools/compiler/patches/0001-rac5-short-loop-six.patch)
changes both branches of `mips_r5900_pad_loops` from `loop < 7` to
`loop < 6`. It is a **RAC5-only trial**, applied to an isolated copy of the
locally reconstructed source after its existing patch stack. The source
still invokes this pass despite a nearby older comment that says the
assembler handles loop padding; the observed FOG output is the reason to
measure the pass instead of relying on that comment. The source
`gcc/config/mips/mips.c` SHA-256 before this trial was
`c76c0bec5b56c198381ab2a4fc60c161a4287e8312d7d1fdea3d1e6a0e1af614`;
`mips.h` remained unchanged at
`52f470a043ffbba535582e2f08a8353d23bbf6521b1b63e241847f464891d9e6`.
The patch passed a dry run against that source. It was never applied to RAC2's
shared runtime or compiler.

The isolated patched `cc1` SHA-256 was
`9614eceecbcb1f7a64cf3a4ad3c827b3e257ab6b1f934e42fcbc4ded0f0584d6`.
With the same measured `cpp` and assembler, `-O2 -G0` now emits **one**
`nop`. The C source in
[`nonmatching/boot/fog_preupdate.c`](../nonmatching/boot/fog_preupdate.c)
produces object SHA-256
`c2cb485df80e2e548499a984b4d9aeb753393dd8a958069255e42190631fc0f1`;
the isolated linked interval matches **64/64 bytes**, SHA-256
`f8a6560b8960c8d17085570344896f2e51ffea6285580cd0390579b61c6f7abe`.

A second, independent loop shape provides another control.
[`MCPFONT_FontStringLength__FPCc`](../nonmatching/boot/mcpfont_string_length.c)
at `0x01E67978` and
[`MCP_FontStringLength__FPCc`](../nonmatching/boot/mcp_string_length.c)
at `0x01E67EE8` have identical retail 64-byte bodies. With the same authored
indexed-string loop, the original threshold-seven `cc1` yields **48/64**
equal bytes at either placement; threshold six yields **64/64** at each,
slice SHA-256
`7bc5cb5ef40e15ad48e97d0f83d1e10ad38599e84c99f45d235e4563355cd973`.
The private compiled objects are distinct because they define different
symbols; their SHA-256 values are
`91227c38a9efa7c0e60f52c04006b1b7cce81750bd2931983d083034c54c309a`
and `8b227d0835ceb0a800ad6d444d10a72b57fa5b7e40ef83e0030841094b949159`.
Alternative pointer and while-loop C forms did not match, so this result also
depends on the authored source shape.

A third control exercises the other padding path: the backward branch in
[`GLOBALWAD_ReleaseTransitionBank__Fv`](../nonmatching/boot/globalwad_release_transition_bank.c)
at `0x01E8FE80` has an unfilled (`nop`) delay slot in retail. The original
compiler yields **69/96** equal bytes and a 100-byte body. Threshold six
yields **96/96**, SHA-256
`daf80f2a29103e4a0a6dfffea346a53543864441d3002f2f4c4cdc87aff26ccd`,
with a 96-byte object SHA-256
`ca6040aee485385ffeab6bb2efd967b638c6168de5b49f0c96d4f5e74b773508`.
The authored C unloads a transition sound bank, flushes pending commands and
delays while the flush reports busy. This validates one unfilled-delay case;
it does not establish behavior for every such branch.

As a regression control, all thirteen prior C source files were rebuilt by
the patched `cc1` without `RAC5_WITNESS_PADDING`. Their object SHA-256 values
were **identical** to the previous pinned objects, preserving the other
sixteen function witnesses. The full boot was then linked with all seventeen
objects: `.text` **498,900/498,900** and raw `PT_LOAD`
**809,872/809,872** remain exact, as do the ELF entry, program header and
BSS placement. The provisional linked bodies total **604 bytes**, with
**44 zero alignment bytes** across 648 measured placement bytes.

This trial matches three distinct loop shapes at four placements, including
one unfilled-delay case, but it does not qualify an original RAC5 compiler
profile. Other loop lengths, branch forms and source families need independent
controls. The GNU source, local compiler binaries,
objects, generated assembly and linked ELF are not distributed here. Keep
the three loop shapes under `nonmatching/`; they remain unaccepted. One
different boot function, `MEMCARD_Busy__Fv`, has since passed the limited
[boot-only gate](BOOT-MATCH-SCOPE.md). The later
[ByteMatch estimate](../progress/README.md) uses only accepted C bytes and a
provisional code denominator; it does not count these loop trials.
