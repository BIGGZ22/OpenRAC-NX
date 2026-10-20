/* Not attempted to the end: it contains a `sync` too.
   +0xEC is 0x0000000F, in the same "save the interrupt state, write the DMA
   registers, barrier" shape as func_0011B0B0. No C construct in this
   toolchain emits `sync` (`__sync_synchronize()` becomes a call, and no
   function already matched in src/ contains one), so the same decision is
   needed before anyone spends compiles on it. The rest of the body -- a loop
   over 0x140-byte stride entries writing 0x20-byte packet pairs -- looks like
   ordinary C. */
