# Original assembly classification

Some retail code was written as assembly, and some four-byte words are
leftovers from functions stripped by the linker. Matching those bytes by
compiling C would misrepresent their origin. The tracked lists are:

- `config/handwritten_asm.txt`: 212 functions marked handwritten by the
  disassembler (83,520 retail bytes).
- `config/linker_remnants.txt`: 145 entries of dead-strip remnants (3,260
  retail bytes).

The handwritten functions and the first 69 remnants were taken from the two
corresponding `tools/triage.py` buckets on 2026-09-27, and 22 remnants were
split off the functions splat had folded them into the same day. The
`fallthrough fragment` and `epilogue fragment` buckets hold both runs of
remnants that splat named as one function and pieces of functions cut in
the wrong place. `tools/overlay_remnants.py --exe` applies the rule below
(see "Level code") to the executable; the 54 runs it accepted were added on
2026-10-07, and what it rejects stays unmatched as a boundary to fix.

`tools/setup_asm.sh` regenerates `asm/` from the contributor's own baserom,
then `tools/organize_asm.py` moves the listed files into
`asm/handwritten/<segment>/` and `asm/remnants/<segment>/`. Compatibility
links remain under `asm/nonmatchings/<segment>/` for the differ and retail
inventory. No retail assembly bytes are committed to Git.

The source uses `ASM_FUNC` and `LINKER_REMNANT` to include these files
without changing the linked image. The progress report counts the listed
original assembly as finished work while the build audit continues to
count exact C matches separately. Changing this published progress policy
requires upstream review.

## Level code

Level code has the same leftovers, and `config/overlays/linker_remnants.txt`
lists them (84 catalogue entries, 1,752 bytes; an entry shared between
levels is one name with many places). The source marks each with
`LINKER_REMNANT("asm/overlays", name)` where its `INCLUDE_ASM` stub was, and
the report counts it as finished, so a file or a level whose functions are
all matched is not held open by stray words.

The level catalogue cuts functions apart more often than the executable's
listing does, so the list is not taken from a triage bucket. It is what
`tools/overlay_remnants.py` derives from the level dumps, and
`tools/overlay_remnants.py --check` compares the two. A stripped function
of size 4 mod 8 leaves its last word, then an alignment nop or the linker's
`0xCDCDCDCD` fill (`tools/strip_dead.py`), so an entry is a run of remnants
when it is not a piece of a joined function and, in every level it is
placed in:

- it starts on an 8-byte boundary, where a stripped function began; each
  of its words there is not a jump or branch (no delay slot is one), each
  word between is zero or fill, and at least one word is neither;
- the code before it is finished: going back over earlier remnants (a
  word, then zero or fill), there is a `jr $31` and its delay slot, a tail
  jump `j` to the start of a function, or the start of the code;
- nothing reaches it: no branch or jump lands on one of its words, and no
  word of code or data holds one of their addresses.

In the executable the rule picks out nine of the ten words `src/libgcc`
reproduces by stripping GCC's own source the way retail's linker did; the
tenth is a nop, which no rule can tell from alignment. The last two tests tell remnants from a piece of a function the catalogue cut
in the wrong place: of the eight 4-byte level entries that fail, three
follow code that has not returned, one is the delay slot of the return
before it, two are already joined pieces, and two are off the 8-byte
boundary. A trap block that its function branches to fails the last test.
