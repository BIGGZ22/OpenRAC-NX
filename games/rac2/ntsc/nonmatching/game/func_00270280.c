#include "include_asm.h"

/* The resident-table walk: the entries are 0xA0 bytes apart and the id word sits at
   +0x86 of each; the table ends 0x1E00 bytes after its base.  Retail branches likely
   ("bnel") on the id test and keeps the increment in the annulled delay slot; this cc1
   emits a plain "beq" with the not-found path as a separate block.  The bound test is a
   signed "slt" in retail: an explicit (int) cast on the pointers produces it, a plain
   pointer comparison gives "sltu". */
extern char D_001B5750[];

void *func_00270280(int id)
{
    char *entry = D_001B5750;
    char *end = entry + 0x1E00;

    for (;;) {
        if (*(short *)(entry + 0x86) == id)
            return entry;
        entry += 0xA0;
        if ((int)entry < (int)end)
            continue;
        return 0;
    }
}
