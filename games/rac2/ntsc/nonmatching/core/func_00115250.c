/* from src/core/00115200.c: the SDK's hand-written byte-scan routine - builds the
   0x8080...8080 mask, scans 16 bytes at a time with multimedia ops the disassembler
   prints as .word, then finishes with an lbu loop that returns the matching address or 0
   (memchr-shaped; the named memcmp is the next row, 0x115340). Plain C cannot emit those
   MMI instructions. AGENTS.md's handwritten-assembly rule applies. */
