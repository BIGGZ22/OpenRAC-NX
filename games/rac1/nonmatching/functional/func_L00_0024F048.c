/* functional: high. fcheck: unchecked (level/overlay function; fcheck runs boot-executable functions only; OpenRAC tools/fcheck_gate.py, 2026-10-18). Written from the level's instructions at 0x24F048-0x24F07C: the catalogue has
   only the first 12 bytes (to the branch), the function runs on to its jr at 0x24F078. 1 when
   the word at 0x13D9B4 and the bytes at 0x13D4B0 and 0x13D4B1 are all set, else 0 (the shape of
   func_L00_0024F1C8 beside it). */
extern int D_0013D9B4;
extern unsigned char D_0013D490[];

int func_L00_0024F048(void) {
    if (D_0013D9B4 == 0) {
        return 0;
    }
    if (D_0013D490[0x20] == 0) {
        return 0;
    }
    return D_0013D490[0x21] != 0;
}
