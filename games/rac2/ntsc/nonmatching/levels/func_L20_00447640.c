/* near-miss: func_L20_00447640 (20_yeedil, 264 bytes).
   The body and the loop condition are retail's, byte for byte; only the entry block
   differs, the same way as func_L20_00442E68's (see that file: the table's %hi lands in
   $v1 instead of $v1/$v0, the table's address is materialised in place instead of into
   $a0, and the index lands in $a0 instead of $v0).
   Everything from 0x00447688 on is retail's: the object lookup
   (D_L20_001B2F9C + ((*p & 0x7FFF) << 8)), the 0xB69 class test with the `bnel` that
   skips to the condition, the two 0xFE/0xFD tests, the two calls and the `p++` that
   falls into the condition block.
   Measured: the class byte must be read as UNSIGNED (`*(unsigned char *)(e + 0x20)`);
   a signed char makes the 0xFE/0xFD test impossible and gcc drops the whole block
   (248 bytes). The two 0xFE/0xFD tests must be two separate `if`s: gcc merges
   `c == 0xFE || c == 0xFD` into a range check (`addiu`/`andi`/`sltiu`, 268 bytes) and
   a `switch` with the two cases merges them the same way.
   Per-file flags measured on this unit: -fno-schedule-insns2 gives 260 bytes (four
   short) and -fno-schedule-insns 256; -g and -mno-split-addresses change nothing. All
   of them break the five functions already matched in L20_0043ECB0-g8.c, so no flag can
   be used for that unit.
   The body needs the -G8 unit: it reaches D_L20_001B2FB8 through $gp. */
#include "common.h"

extern char *D_L20_001FBB20[] __attribute__((nosda));

extern char *D_L20_001B2F9C __attribute__((nosda));

extern int D_L20_001B2FB8 __attribute__((sda));

extern void func_0032EAB8(char *, int, int);

extern void func_003FBEA8(char *, int);

void func_L20_00447640(char *arg0) {
    unsigned char *self = arg0;
    unsigned short *p;
    int i;

    i = self[0x21];
    if (i == 0xFF) {
        goto out;
    }
    p = (unsigned short *)D_L20_001FBB20[i];
    goto cond;
loop:
    {
        char *e = D_L20_001B2F9C + ((*p & 0x7FFF) << 8);

        if (e == 0) {
            goto out;
        }
        if (*(short *)(e + 0xAA) != 0xB69) {
            p++;
            goto cond;
        }
        if (*(unsigned char *)(e + 0x20) == 0xFE) {
            goto skip;
        }
        if (*(unsigned char *)(e + 0x20) == 0xFD) {
            goto skip;
        }
        func_0032EAB8(e, 0, 1);
        func_003FBEA8(e, 0);
skip:
        p++;
    }
    goto cond;
cond:
    i = self[0x21];
    if (i == 0xFF) {
        goto out;
    }
    if (D_L20_001B2FB8 < i) {
        goto out;
    }
    if (p == 0) {
        goto out;
    }
    if (*(short *)(p - 1) >= 0 || p == (unsigned short *)D_L20_001FBB20[i]) {
        goto loop;
    }
    goto out;
out:
    ;
}
