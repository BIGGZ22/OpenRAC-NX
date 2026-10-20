/* 0 of 1 places match - last line: 12_todano: candidate is 108 bytes, expected 84
   The body is recovered: the two mode gates (0x001A8F00 == 2 and 0x001BDB30 == 5),
   the tracked pointer 0x001B2EA8 through $gp, the cleared word at +0x98, the two
   flag bits (| 0x41 & ~0x1000) at +0x34 and the walk of the chain at +0x28.
   What is left is the chain walk: retail enters the loop at the `lw $v1,0x28($v1)`
   and keeps `lhu $v0,0x34($v1)` in the delay slot of its `bnel` (84 bytes, no
   padding); this cc1 peels that load out of the loop, pads the loop head with
   alignment nops and leaves the load in the delay slot of the peeled copy, so the
   candidate is 24 bytes (6 instructions) longer. The `$gp` global needs the -g8
   sibling and the two absolute ones the `lui`/`lw` form, so the unit needs
   `-G8 -mno-split-addresses`; the load of 0x001B2EA8 also lands in the first
   branch's delay slot here and in the second one's there.
   Measured with nine spellings (the loop written as while / do-while / for, the
   pointer in a local or read inline, the function returning void or the loaded
   halfword): all 108, all peeling the same load. */

extern int D_L12_001A8F00 __attribute__((nosda));
extern int D_L12_001BDB30 __attribute__((nosda));
extern char *D_L12_001B2EA8 __attribute__((sda));

unsigned short func_L12_00388038(void) {
    char *v1 = D_L12_001B2EA8;
    unsigned short v0;

    if (D_L12_001A8F00 != 2) {
        return 0;
    }
    if (D_L12_001BDB30 != 5) {
        return 0;
    }
    if (v1 == 0) {
        return 0;
    }
    v0 = *(unsigned short *)(v1 + 0x34);
    *(int *)(v1 + 0x98) = 0;
    *(unsigned short *)(v1 + 0x34) = (v0 | 0x41) & 0xEFFF;
    while (v1 != 0) {
        v0 = *(unsigned short *)(v1 + 0x34);
        v1 = *(char **)(v1 + 0x28);
    }
    return v0;
}
