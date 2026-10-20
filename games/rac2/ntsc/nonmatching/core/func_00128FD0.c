/* Reads a register it never sets.
   $s4 is saved at +0x8 and restored at +0xE0, but the first thing that
   touches it is `and $s4,$s4,0xFFFFC000` at +0x10 -- before any instruction
   writes it. The value it accumulates keeps bits 32..63 of that incoming
   register, so the function's result depends on the caller's $s4. That is
   BACKFILL.md's "reads a register it never sets" sign: either a piece of a
   neighbour or a catalogue problem, not something C can express. */
