/* from src/core/00115200.c: memcpy (symbol_addrs: size 0xB0, "libc.a, verbatim"). It is
   the SDK's hand-written r5900 memcpy: 128-bit lq/sq moves (the .word entries the
   disassembler cannot decode), an alignment prologue built from ldl/ldr/sdl/sdr and a
   tail loop of ld/sd plus lbu/sb. Plain C emits no lq/sq/ldl/ldr, so no C is written;
   AGENTS.md's handwritten-assembly rule applies (evidence: the instruction set). */
